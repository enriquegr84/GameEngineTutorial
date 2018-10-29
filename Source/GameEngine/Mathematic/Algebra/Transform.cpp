// David Eberly, Geometric Tools, Redmond WA 98052
// Copyright (c) 1998-2017
// Distributed under the Boost Software License, Version 1.0.
// http://www.boost.org/LICENSE_1_0.txt
// http://www.geometrictools.com/License/Boost/LICENSE_1_0.txt
// File Version: 3.0.0 (2016/06/19)

#include "Transform.h"
#include <algorithm>

Transform const Transform::Identity;

Transform::Transform()
    :
    mTranslate({ 0.0f, 0.0f, 0.0f, 1.0f }),
    mScale({ 1.0f, 1.0f, 1.0f, 1.0f }),
    mIsIdentity(true),
    mIsRSMatrix(true),
    mIsUniformScale(true),
    mInverseNeedsUpdate(false)
{
	mHMatrix.MakeIdentity();
	mInvHMatrix.MakeIdentity();
	mMatrix.MakeIdentity();
}

void Transform::MakeIdentity()
{
	mMatrix.MakeIdentity();

    mTranslate = { 0.0f, 0.0f, 0.0f, 1.0f };
    mScale = { 1.0f, 1.0f, 1.0f, 1.0f };
    mIsIdentity = true;
    mIsRSMatrix = true;
    mIsUniformScale = true;
    UpdateHMatrix();
}

void Transform::MakeUnitScale()
{
    LogAssert(mIsRSMatrix, "Transform is not rotation-scale.");
    mScale = { 1.0f, 1.0f, 1.0f, 1.0f };
    mIsUniformScale = true;
    UpdateHMatrix();
}

void Transform::SetRotation(Matrix4x4<float> const& rotate)
{
    mMatrix = rotate;
    mIsIdentity = false;
    mIsRSMatrix = true;
    UpdateHMatrix();
}

void Transform::SetMatrix(Matrix4x4<float> const& matrix)
{
    mMatrix = matrix;
    mIsIdentity = false;
    mIsRSMatrix = false;
    mIsUniformScale = false;
    UpdateHMatrix();
}

void Transform::SetTranslation(float x0, float x1, float x2)
{
    mTranslate = { x0, x1, x2, 1.0f };
    mIsIdentity = false;
    UpdateHMatrix();
}

void Transform::SetTranslation(Vector3<float> const& translate)
{
    SetTranslation(translate[0], translate[1], translate[2]);
}

void Transform::SetTranslation(Vector4<float> const& translate)
{
    SetTranslation(translate[0], translate[1], translate[2]);
}

void Transform::SetScale(float s0, float s1, float s2)
{
    LogAssert(mIsRSMatrix, "Transform is not rotation-scale.");
    LogAssert(s0 != 0.0f && s1 != 0.0f && s2 != 0.0f,
        "Scales must be nonzero.");

    mScale = { s0, s1, s2, 1.0f };
    mIsIdentity = false;
    mIsUniformScale = false;
    UpdateHMatrix();
}

void Transform::SetScale(Vector3<float> const& scale)
{
    SetScale(scale[0], scale[1], scale[2]);
}

void Transform::SetScale(Vector4<float> const& scale)
{
    SetScale(scale[0], scale[1], scale[2]);
}

void Transform::SetUniformScale(float scale)
{
    LogAssert(mIsRSMatrix, "Transform is not rotation-scale.");
    LogAssert(scale != 0.0f, "Scale must be nonzero.");

    mScale = { scale, scale, scale, 1.0f };
    mIsIdentity = false;
    mIsUniformScale = true;
    UpdateHMatrix();
}

void Transform::SetRotation(Matrix3x3<float> const& rotate)
{
    mMatrix.MakeIdentity();

	float* matrix = reinterpret_cast<float*>(&mMatrix);
	float const* in = reinterpret_cast<float const*>(&rotate);
    for (int r = 0; r < 3; ++r)
    {
        for (int c = 0; c < 3; ++c)
        {
			matrix[r * 4 + c] = in[r * 4 + c];
        }
    }

    mIsIdentity = false;
    mIsRSMatrix = true;
    UpdateHMatrix();
}

void Transform::GetRotation(Matrix3x3<float>& rotate) const
{
    LogAssert(mIsRSMatrix, "Transform is not rotation-scale.");

	float const* matrix = reinterpret_cast<float const*>(&mMatrix);
	float* out = reinterpret_cast<float*>(&rotate);
    for (int r = 0; r < 3; ++r)
    {
        for (int c = 0; c < 3; ++c)
        {
			out[r * 4 + c] = matrix[r * 4 + c];
        }
    }
}

void Transform::SetRotation(Quaternion<float> const& q)
{
    mMatrix = Rotation<4, float>(q);
    mIsIdentity = false;
    mIsRSMatrix = true;
    UpdateHMatrix();
}

void Transform::GetRotation(Quaternion<float>& q) const
{
    LogAssert(mIsRSMatrix, "Transform is not rotation-scale.");
    q = Rotation<4, float>(mMatrix);
}

void Transform::SetRotation(AxisAngle<4, float> const& axisAngle)
{
    mMatrix = Rotation<4, float>(axisAngle);
    mIsIdentity = false;
    mIsRSMatrix = true;
    UpdateHMatrix();
}

void Transform::GetRotation(AxisAngle<4, float>& axisAngle) const
{
    LogAssert(mIsRSMatrix, "Transform is not rotation-scale.");
    axisAngle = Rotation<4, float>(mMatrix);
}

void Transform::SetRotation(EulerAngles<float> const& eulerAngles)
{
    mMatrix = Rotation<4, float>(eulerAngles);
    mIsIdentity = false;
    mIsRSMatrix = true;
    UpdateHMatrix();
}

void Transform::GetRotation(EulerAngles<float>& eulerAngles) const
{
    LogAssert(mIsRSMatrix, "Transform is not rotation-scale.");
    eulerAngles = Rotation<4, float>(mMatrix)(eulerAngles.mAxis[0],
        eulerAngles.mAxis[1], eulerAngles.mAxis[2]);
}

float Transform::GetNorm() const
{
    float r0, r1, r2;

    if (mIsRSMatrix)
    {
		float const* scale = reinterpret_cast<float const*>(&mScale);

        // A RS matrix (GE_USE_MAT_VEC) or an SR matrix (GE_USE_VEC_MAT).
        r0 = fabs(scale[0]);
        r1 = fabs(scale[1]);
        r2 = fabs(scale[2]);
    }
    else
    {
        // The spectral norm (the maximum absolute value of the eigenvalues)
        // is smaller or equal to this norm.  Therefore, this function returns
        // an approximation to the maximum scale.
		float const* matrix = reinterpret_cast<float const*>(&mMatrix);

#if defined(GE_USE_MAT_VEC)
        // Use the max-row-sum matrix norm.
        r0 = fabs(matrix[0 * 4 + 0]) + fabs(matrix[0 * 4 + 1]) + fabs(matrix[0 * 4 + 2]);
        r1 = fabs(matrix[1 * 4 + 0]) + fabs(matrix[1 * 4 + 1]) + fabs(matrix[1 * 4 + 2]);
        r2 = fabs(matrix[2 * 4 + 0]) + fabs(matrix[2 * 4 + 1]) + fabs(matrix[2 * 4 + 2]);
#else
        // Use the max-col-sum matrix norm.
        r0 = fabs(matrix[0 * 4 + 0]) + fabs(matrix[1 * 4 + 0]) + fabs(matrix[2 * 4 + 0]);
        r1 = fabs(matrix[0 * 4 + 1]) + fabs(matrix[1 * 4 + 1]) + fabs(matrix[2 * 4 + 1]);
        r2 = fabs(matrix[0 * 4 + 2]) + fabs(matrix[1 * 4 + 2]) + fabs(matrix[2 * 4 + 2]);
#endif
    }

    return eastl::max(eastl::max(r0, r1), r2);
}

Matrix4x4<float> const& Transform::GetHInverse() const
{
    if (mInverseNeedsUpdate)
    {
        if (mIsIdentity)
        {
            mInvHMatrix.MakeIdentity();
        }
        else
        {
			float const* matrix = reinterpret_cast<float const*>(&mMatrix);
			float* invHMatrix = reinterpret_cast<float*>(&mInvHMatrix);

			float const* scale = reinterpret_cast<float const*>(&mScale);
			float const* translate = reinterpret_cast<float const*>(&mTranslate);

            if (mIsRSMatrix)
            {
                if (mIsUniformScale)
                {
                    float invScale = 1.0f / scale[0];
#if defined(GE_USE_MAT_VEC)
					invHMatrix[0 * 4 + 0] = invScale * matrix[0 * 4 + 0];
					invHMatrix[0 * 4 + 1] = invScale * matrix[1 * 4 + 0];
					invHMatrix[0 * 4 + 2] = invScale * matrix[2 * 4 + 0];
					invHMatrix[1 * 4 + 0] = invScale * matrix[0 * 4 + 1];
					invHMatrix[1 * 4 + 1] = invScale * matrix[1 * 4 + 1];
					invHMatrix[1 * 4 + 2] = invScale * matrix[2 * 4 + 1];
					invHMatrix[2 * 4 + 0] = invScale * matrix[0 * 4 + 2];
					invHMatrix[2 * 4 + 1] = invScale * matrix[1 * 4 + 2];
					invHMatrix[2 * 4 + 2] = invScale * matrix[2 * 4 + 2];
#else
					invHMatrix[0 * 4 + 0] = matrix[0 * 4 + 0] * invScale;
					invHMatrix[0 * 4 + 1] = matrix[1 * 4 + 0] * invScale;
					invHMatrix[0 * 4 + 2] = matrix[2 * 4 + 0] * invScale;
					invHMatrix[1 * 4 + 0] = matrix[0 * 4 + 1] * invScale;
					invHMatrix[1 * 4 + 1] = matrix[1 * 4 + 1] * invScale;
					invHMatrix[1 * 4 + 2] = matrix[2 * 4 + 1] * invScale;
					invHMatrix[2 * 4 + 0] = matrix[0 * 4 + 2] * invScale;
					invHMatrix[2 * 4 + 1] = matrix[1 * 4 + 2] * invScale;
					invHMatrix[2 * 4 + 2] = matrix[2 * 4 + 2] * invScale;
#endif
                }
                else
                {
                    // Replace 3 reciprocals by 6 multiplies and 1 reciprocal.
                    float s01 = scale[0] * scale[1];
                    float s02 = scale[0] * scale[2];
                    float s12 = scale[1] * scale[2];
                    float invs012 = 1.0f / (s01 * scale[2]);
                    float invS0 = s12 * invs012;
                    float invS1 = s02 * invs012;
                    float invS2 = s01 * invs012;
#if defined(GE_USE_MAT_VEC)
					invHMatrix[0 * 4 + 0] = invS0 * matrix[0 * 4 + 0];
					invHMatrix[0 * 4 + 1] = invS0 * matrix[1 * 4 + 0];
					invHMatrix[0 * 4 + 2] = invS0 * matrix[2 * 4 + 0];
					invHMatrix[1 * 4 + 0] = invS1 * matrix[0 * 4 + 1];
					invHMatrix[1 * 4 + 1] = invS1 * matrix[1 * 4 + 1];
					invHMatrix[1 * 4 + 2] = invS1 * matrix[2 * 4 + 1];
					invHMatrix[2 * 4 + 0] = invS2 * matrix[0 * 4 + 2];
					invHMatrix[2 * 4 + 1] = invS2 * matrix[1 * 4 + 2];
					invHMatrix[2 * 4 + 2] = invS2 * matrix[2 * 4 + 2];
#else
					invHMatrix[0 * 4 + 0] = matrix[0 * 4 + 0] * invS0;
					invHMatrix[0 * 4 + 1] = matrix[1 * 4 + 0] * invS1;
					invHMatrix[0 * 4 + 2] = matrix[2 * 4 + 0] * invS2;
					invHMatrix[1 * 4 + 0] = matrix[0 * 4 + 1] * invS0;
					invHMatrix[1 * 4 + 1] = matrix[1 * 4 + 1] * invS1;
					invHMatrix[1 * 4 + 2] = matrix[2 * 4 + 1] * invS2;
					invHMatrix[2 * 4 + 0] = matrix[0 * 4 + 2] * invS0;
					invHMatrix[2 * 4 + 1] = matrix[1 * 4 + 2] * invS1;
					invHMatrix[2 * 4 + 2] = matrix[2 * 4 + 2] * invS2;
#endif
                }
            }
            else
            {
                Invert3x3(mHMatrix, mInvHMatrix);
            }

#if defined(GE_USE_MAT_VEC)
			invHMatrix[0 * 4 + 3] = -(
				invHMatrix[0 * 4 + 0] * translate[0] +
				invHMatrix[0 * 4 + 1] * translate[1] +
				invHMatrix[0 * 4 + 2] * translate[2]
                );

			invHMatrix[1 * 4 + 3] = -(
				invHMatrix[1 * 4 + 0] * translate[0] +
				invHMatrix[1 * 4 + 1] * translate[1] +
				invHMatrix[1 * 4 + 2] * translate[2]
                );

			invHMatrix[2 * 4 + 3] = -(
				invHMatrix[2 * 4 + 0] * translate[0] +
				invHMatrix[2 * 4 + 1] * translate[1] +
				invHMatrix[2 * 4 + 2] * translate[2]
                );

            // The last row of mHMatrix is always (0,0,0,1) for an affine
            // transformation, so it is set once in the constructor.  It is
            // not necessary to reset it here.
#else
			invHMatrix[3 * 4 + 0] = -(
				invHMatrix[0 * 4 + 0] * translate[0] +
				invHMatrix[1 * 4 + 0] * translate[1] +
				invHMatrix[2 * 4 + 0] * translate[2]
                );

			invHMatrix[3 * 4 + 1] = -(
				invHMatrix[0 * 4 + 1] * translate[0] +
				invHMatrix[1 * 4 + 1] * translate[1] +
				invHMatrix[2 * 4 + 1] * translate[2]
                );

			invHMatrix[3 * 4 + 2] = -(
				invHMatrix[0 * 4 + 2] * translate[0] +
				invHMatrix[1 * 4 + 2] * translate[1] +
				invHMatrix[2 * 4 + 2] * translate[2]
                );

            // The last column of mHMatrix is always (0,0,0,1) for an affine
            // transformation, so it is set once in the constructor.  It is
            // not necessary to reset it here.
#endif
        }

        mInverseNeedsUpdate = false;
    }

    return mInvHMatrix;
}

Transform Transform::Inverse() const
{
    Transform inverse;

    if (mIsIdentity)
    {
        inverse.MakeIdentity();
    }
    else
    {
        if (mIsRSMatrix && mIsUniformScale)
        {
            inverse.SetRotation(Transpose(GetRotation()));
            inverse.SetUniformScale(1.0f / GetUniformScale());
        }
        else
        {
            Matrix4x4<float> invUpper;
            Invert3x3(GetMatrix(), invUpper);
            inverse.SetMatrix(invUpper);
        }
        Vector4<float> trn = -GetTranslationW0();
        inverse.SetTranslation(inverse.GetMatrix() * trn);
    }

    mInverseNeedsUpdate = true;
    return inverse;
}

void Transform::UpdateHMatrix()
{
    if (mIsIdentity)
    {
        mHMatrix.MakeIdentity();
    }
    else
    {
		float* matrix = reinterpret_cast<float*>(&mMatrix);
		float* hmatrix = reinterpret_cast<float*>(&mHMatrix);
		float* translate = reinterpret_cast<float*>(&mTranslate);
		float* scale = reinterpret_cast<float*>(&mScale);

        if (mIsRSMatrix)
        {
#if defined(GE_USE_MAT_VEC)
			hmatrix[0 * 4 + 0] = matrix[0 * 4 + 0] * scale[0];
			hmatrix[0 * 4 + 1] = matrix[0 * 4 + 1] * scale[1];
			hmatrix[0 * 4 + 2] = matrix[0 * 4 + 2] * scale[2];
			hmatrix[1 * 4 + 0] = matrix[1 * 4 + 0] * scale[0];
			hmatrix[1 * 4 + 1] = matrix[1 * 4 + 1] * scale[1];
			hmatrix[1 * 4 + 2] = matrix[1 * 4 + 2] * scale[2];
			hmatrix[2 * 4 + 0] = matrix[2 * 4 + 0] * scale[0];
			hmatrix[2 * 4 + 1] = matrix[2 * 4 + 1] * scale[1];
			hmatrix[2 * 4 + 2] = matrix[2 * 4 + 2] * scale[2];
#else
			hmatrix[0 * 4 + 0] = scale[0] * matrix[0 * 4 + 0];
			hmatrix[0 * 4 + 1] = scale[0] * matrix[0 * 4 + 1];
			hmatrix[0 * 4 + 2] = scale[0] * matrix[0 * 4 + 2];
			hmatrix[1 * 4 + 0] = scale[1] * matrix[1 * 4 + 0];
			hmatrix[1 * 4 + 1] = scale[1] * matrix[1 * 4 + 1];
			hmatrix[1 * 4 + 2] = scale[1] * matrix[1 * 4 + 2];
			hmatrix[2 * 4 + 0] = scale[2] * matrix[2 * 4 + 0];
			hmatrix[2 * 4 + 1] = scale[2] * matrix[2 * 4 + 1];
			hmatrix[2 * 4 + 2] = scale[2] * matrix[2 * 4 + 2];
#endif
        }
        else
        {
			hmatrix[0 * 4 + 0] = matrix[0 * 4 + 0];
			hmatrix[0 * 4 + 1] = matrix[0 * 4 + 1];
			hmatrix[0 * 4 + 2] = matrix[0 * 4 + 2];
			hmatrix[1 * 4 + 0] = matrix[1 * 4 + 0];
			hmatrix[1 * 4 + 1] = matrix[1 * 4 + 1];
			hmatrix[1 * 4 + 2] = matrix[1 * 4 + 2];
			hmatrix[2 * 4 + 0] = matrix[2 * 4 + 0];
			hmatrix[2 * 4 + 1] = matrix[2 * 4 + 1];
			hmatrix[2 * 4 + 2] = matrix[2 * 4 + 2];
        }

#if defined(GE_USE_MAT_VEC)
		hmatrix[0 * 4 + 3] = translate[0];
		hmatrix[1 * 4 + 3] = translate[1];
		hmatrix[2 * 4 + 3] = translate[2];

        // The last row of mHMatrix is always (0,0,0,1) for an affine
        // transformation, so it is set once in the constructor.  It is not
        // necessary to reset it here.
#else
		hmatrix[3 * 4 + 0] = translate[0];
		hmatrix[3 * 4 + 1] = translate[1];
		hmatrix[3 * 4 + 2] = translate[2];

        // The last column of mHMatrix is always (0,0,0,1) for an affine
        // transformation, so it is set once in the constructor.  It is not
        // necessary to reset it here.
#endif
    }

    mInverseNeedsUpdate = true;
}

void Transform::Invert3x3(Matrix4x4<float> const& matrix,
    Matrix4x4<float>& invMatrix)
{
	float* invMat = reinterpret_cast<float*>(&invMatrix);
	float const* mat = reinterpret_cast<float const*>(&matrix);

    // Compute the adjoint of M (3x3).
    invMat[0 * 4 + 0] = mat[1 * 4 + 1] * mat[2 * 4 + 2] - mat[1 * 4 + 2] * mat[2 * 4 + 1];
    invMat[0 * 4 + 1] = mat[0 * 4 + 2] * mat[2 * 4 + 1] - mat[0 * 4 + 1] * mat[2 * 4 + 2];
    invMat[0 * 4 + 2] = mat[0 * 4 + 1] * mat[1 * 4 + 2] - mat[0 * 4 + 2] * mat[1 * 4 + 1];
    invMat[0 * 4 + 3] = 0.0f;
    invMat[1 * 4 + 0] = mat[1 * 4 + 2] * mat[2 * 4 + 0] - mat[1 * 4 + 0] * mat[2 * 4 + 2];
    invMat[1 * 4 + 1] = mat[0 * 4 + 0] * mat[2 * 4 + 2] - mat[0 * 4 + 2] * mat[2 * 4 + 0];
    invMat[1 * 4 + 2] = mat[0 * 4 + 2] * mat[1 * 4 + 0] - mat[0 * 4 + 0] * mat[1 * 4 + 2];
    invMat[1 * 4 + 3] = 0.0f;
    invMat[2 * 4 + 0] = mat[1 * 4 + 0] * mat[2 * 4 + 1] - mat[1 * 4 + 1] * mat[2 * 4 + 0];
    invMat[2 * 4 + 1] = mat[0 * 4 + 1] * mat[2 * 4 + 0] - mat[0 * 4 + 0] * mat[2 * 4 + 1];
    invMat[2 * 4 + 2] = mat[0 * 4 + 0] * mat[1 * 4 + 1] - mat[0 * 4 + 1] * mat[1 * 4 + 0];
    invMat[2 * 4 + 3] = 0.0f;
    invMat[3 * 4 + 0] = 0.0f;
    invMat[3 * 4 + 1] = 0.0f;
    invMat[3 * 4 + 2] = 0.0f;
    invMat[3 * 4 + 3] = 1.0f;

    // Compute the reciprocal of the determinant of M.
    float invDet = 1.0f / (
        mat[0 * 4 + 0] * invMat[0 * 4 + 0] +
        mat[0 * 4 + 1] * invMat[1 * 4 + 0] +
        mat[0 * 4 + 2] * invMat[2 * 4 + 0]
        );

    // inverse(M) = adjoint(M)/determinant(M).
    invMat[0 * 4 + 0] *= invDet;
    invMat[0 * 4 + 1] *= invDet;
    invMat[0 * 4 + 2] *= invDet;
    invMat[1 * 4 + 0] *= invDet;
    invMat[1 * 4 + 1] *= invDet;
    invMat[1 * 4 + 2] *= invDet;
    invMat[2 * 4 + 0] *= invDet;
    invMat[2 * 4 + 1] *= invDet;
    invMat[2 * 4 + 2] *= invDet;
}

Vector4<float> operator*(Transform const& M, Vector4<float> const& V)
{
    return M.GetHMatrix() * V;
}

Vector4<float> operator*(Vector4<float> const& V, Transform const& M)
{
    return V * M.GetHMatrix();
}

Transform operator*(Transform const& A, Transform const& B)
{
    if (A.IsIdentity())
    {
        return B;
    }

    if (B.IsIdentity())
    {
        return A;
    }

    Transform product;

    if (A.IsRSMatrix() && B.IsRSMatrix())
    {
#if defined(GE_USE_MAT_VEC)
        if (A.IsUniformScale())
        {
            product.SetRotation(MultiplyAB(A.GetRotation(), B.GetRotation()));

			Vector4<float> transform;
			A.GetRotation().Transformation(B.GetTranslationW0(), transform);
            product.SetTranslation(A.GetUniformScale()*transform + A.GetTranslationW1());

            if (B.IsUniformScale())
            {
                product.SetUniformScale(
                    A.GetUniformScale() * B.GetUniformScale());
            }
            else
            {
                product.SetScale(A.GetUniformScale() * B.GetScale());
            }

            return product;
        }
#else
        if (B.IsUniformScale())
        {
            product.SetRotation(MultiplyAB(A.GetRotation(), B.GetRotation()));

			Vector4<float> transform;
			B.GetRotation().Transformation(A.GetTranslationW0(), transform);
            product.SetTranslation(B.GetUniformScale()*transform + B.GetTranslationW1());

            if (A.IsUniformScale())
            {
                product.SetUniformScale(
                    A.GetUniformScale() * B.GetUniformScale());
            }
            else
            {
                product.SetScale(A.GetScale() * B.GetUniformScale());
            }

            return product;
        }
#endif
    }

    // In all remaining cases, the matrix cannot be written as R*S*X+T.
    Matrix4x4<float> matMA;
    if (A.IsRSMatrix())
    {
#if defined(GE_USE_MAT_VEC)
        matMA = MultiplyMD(A.GetRotation(), A.GetScaleW1());
#else
        matMA = MultiplyDM(A.GetScaleW1(), A.GetRotation());
#endif
    }
    else
    {
        matMA = A.GetMatrix();
    }

    Matrix4x4<float> matMB;
    if (B.IsRSMatrix())
    {
#if defined(GE_USE_MAT_VEC)
        matMB = MultiplyMD(B.GetRotation(), B.GetScaleW1());
#else
        matMB = MultiplyDM(B.GetScaleW1(), B.GetRotation());
#endif
    }
    else
    {
        matMB = B.GetMatrix();
    }

    product.SetMatrix(MultiplyAB(matMA, matMB));
#if defined(GE_USE_MAT_VEC)
	Vector4<float> transform;
	matMA.Transformation(B.GetTranslationW0(), transform);
	product.SetTranslation(transform + A.GetTranslationW1());
#else
	Vector4<float> transform;
	matMB.Transformation(A.GetTranslationW0(), transform);
	product.SetTranslation(transform + B.GetTranslationW1());
#endif
    return product;
}