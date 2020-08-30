// David Eberly, Geometric Tools, Redmond WA 98052
// Copyright (c) 1998-2020
// Distributed under the Boost Software License, Version 1.0.
// https://www.boost.org/LICENSE_1_0.txt
// https://www.geometrictools.com/License/Boost/LICENSE_1_0.txt
// Version: 4.0.2019.08.13

#include "BumpMapEffect.h"

#include "Graphic/Graphic.h"
#include "Mathematic/Algebra/Matrix4x4.h"

BumpMapEffect::BumpMapEffect(
	eastl::shared_ptr<ProgramFactory> const& factory, eastl::vector<eastl::string> const& path,
	eastl::shared_ptr<Texture2> const& baseTexture, eastl::shared_ptr<Texture2> const& normalTexture, 
	SamplerState::Filter filter, SamplerState::Mode mode0, SamplerState::Mode mode1)
    :
    mBaseTexture(baseTexture),
    mNormalTexture(normalTexture)
{
	eastl::string vsPath = path[0];
	eastl::string psPath = path[1];
	eastl::string gsPath = "";
	mProgram = factory->CreateFromFiles(vsPath, psPath, gsPath);
    if (mProgram)
    {
        mCommonSampler = eastl::make_shared<SamplerState>();
        mCommonSampler->mFilter = filter;
        mCommonSampler->mMode[0] = mode0;
        mCommonSampler->mMode[1] = mode1;

		mProgram->GetVShader()->Set("PVWMatrix", mPVWMatrixConstant);
#if defined(_OPENGL_)
		mProgram->GetPShader()->Set("baseSampler", mBaseTexture);
		mProgram->GetPShader()->Set("normalSampler", mNormalTexture);
#else
		mProgram->GetPShader()->Set("baseTexture", mBaseTexture);
		mProgram->GetPShader()->Set("normalTexture", mNormalTexture);
#endif
		mProgram->GetPShader()->Set("baseSampler", mCommonSampler);
		mProgram->GetPShader()->Set("normalSampler", mCommonSampler);

    }
    else
    {
        LogError("Failed to compile shader programs.");
    }
}

void BumpMapEffect::SetPVWMatrixConstant(eastl::shared_ptr<ConstantBuffer> const& buffer)
{
    VisualEffect::SetPVWMatrixConstant(buffer);
    mProgram->GetVShader()->Set("PVWMatrix", mPVWMatrixConstant);
}

void BumpMapEffect::ComputeLightVectors(eastl::shared_ptr<Node> const& node,
    Vector4<float> const& worldLightDirection)
{
    // The tangent-space coordinates for the light direction vector at each
    // vertex is stored in the color0 channel.  The computations use the
    // vertex normals and the texture coordinates for the base mesh, which
    // are stored in the tcoord0 channel.  Thus, the mesh must have positions,
    // normals, colors (unit 0), and texture coordinates (unit 0).  The struct
    // shown next is consistent with mesh->GetVertexFormat().
    struct Vertex
    {
        Vector3<float> position;
        Vector3<float> normal;
        Vector3<float> lightDirection;
        Vector2<float> baseTCoord;
        Vector2<float> normalTCoord;
    };

    // The light direction D is in world-space coordinates.  Negate it,
    // transform it to model-space coordinates, and then normalize it.  The
    // world-space direction is unit-length, but the geometric primitive
    // might have non-unit scaling in its model-to-world transformation, in
    // which case the normalization is necessary.
    Matrix4x4<float> invWMatrix = node->GetAbsoluteTransform().GetHInverse();
    Vector4<float> tempDirection = -DoTransform(invWMatrix, worldLightDirection);
    Vector3<float> modelLightDirection = HProject(tempDirection);

    // Set the light vectors to (0,0,0) as a flag that the quantity has not
    // yet been computed.  The probability that a light vector is actually
    // (0,0,0) should be small, so the flag system should save computation
    // time overall.
	for (unsigned int v = 0; v < node->GetVisualCount(); v++)
	{
		auto vbuffer = node->GetVisual(v)->GetVertexBuffer();
		unsigned int const numVertices = vbuffer->GetNumElements();
		auto* vertices = vbuffer->Get<Vertex>();
		Vector3<float> const zero{ 0.0f, 0.0f, 0.0f };
		for (unsigned int i = 0; i < numVertices; ++i)
		{
			vertices[i].lightDirection = zero;
		}

		auto ibuffer = node->GetVisual(v)->GetIndexBuffer();
		unsigned int numTriangles = ibuffer->GetNumPrimitives();
		auto* indices = ibuffer->Get<unsigned int>();
		for (unsigned int t = 0; t < numTriangles; ++t)
		{
			// Get the triangle vertices and attributes.
			unsigned int v[3];
			v[0] = *indices++;
			v[1] = *indices++;
			v[2] = *indices++;

			for (int i = 0; i < 3; ++i)
			{
				int v0 = v[i];
				if (vertices[v0].lightDirection != zero)
				{
					continue;
				}

				int iP = (i == 0) ? 2 : i - 1;
				int iN = (i + 1) % 3;
				int v1 = v[iN], v2 = v[iP];

				Vector3<float> const& pos0 = vertices[v0].position;
				Vector2<float> const& tcd0 = vertices[v0].baseTCoord;
				Vector3<float> const& pos1 = vertices[v1].position;
				Vector2<float> const& tcd1 = vertices[v1].baseTCoord;
				Vector3<float> const& pos2 = vertices[v2].position;
				Vector2<float> const& tcd2 = vertices[v2].baseTCoord;
				Vector3<float> const& normal = vertices[v0].normal;

				Vector3<float> tangent;
				if (!ComputeTangent(pos0, tcd0, pos1, tcd1, pos2, tcd2, tangent))
				{
					// The texture coordinate mapping is not properly defined for
					// this.  Just say that the tangent space light vector points
					// in the same direction as the surface normal.
					vertices[v0].lightDirection = normal;
					continue;
				}

				// Project T into the tangent plane by projecting out the surface
				// normal N, and then make it unit length.
				tangent -= Dot(normal, tangent) * normal;
				Normalize(tangent);

				// Compute the bitangent B, another tangent perpendicular to T.
				Vector3<float> bitangent = UnitCross(normal, tangent);

				// The set {T,B,N} is a right-handed orthonormal set.  The
				// negated light direction U = -D is represented in this
				// coordinate system as
				//   U = Dot(U,T)*T + Dot(U,B)*B + Dot(U,N)*N
				float dotUT = Dot(modelLightDirection, tangent);
				float dotUB = Dot(modelLightDirection, bitangent);
				float dotUN = Dot(modelLightDirection, normal);

				// Transform the light vector into [0,1]^3 to make it a valid
				// Vector3<float> object.
				vertices[v0].lightDirection =
				{
					0.5f * (dotUT + 1.0f),
					0.5f * (dotUB + 1.0f),
					0.5f * (dotUN + 1.0f)
				};
			}
		}
	}
}

bool BumpMapEffect::ComputeTangent(
    Vector3<float> const& position0, Vector2<float> const& tcoord0,
    Vector3<float> const& position1, Vector2<float> const& tcoord1,
    Vector3<float> const& position2, Vector2<float> const& tcoord2,
    Vector3<float>& tangent)
{
    // Compute the change in positions at the vertex P0.
    Vector3<float> deltaPos1 = position1 - position0;
    Vector3<float> deltaPos2 = position2 - position0;

    float const epsilon = 1e-08f;
    if (Length(deltaPos1) <= epsilon || Length(deltaPos1) <= epsilon )
    {
        // The triangle is degenerate.
        return false;
    }

    // Compute the change in texture coordinates at the vertex P0 in the
    // direction of edge P1-P0.
    float du1 = tcoord1[0] - tcoord0[0];
    float dv1 = tcoord1[1] - tcoord0[1];
    if (std::fabs(dv1) <= epsilon)
    {
        // The triangle effectively has no variation in the v texture
        // coordinate.
        if (std::fabs(du1) <= epsilon)
        {
            // The triangle effectively has no variation in the u coordinate.
            // Since the texture coordinates do not vary on this triangle,
            // treat it as a degenerate parametric surface.
            return false;
        }

        // The variation is effectively all in u, so set the tangent vector
        // to be T = dP/du.
        tangent = deltaPos1 / du1;
        return true;
    }

    // Compute the change in texture coordinates at the vertex P0 in the
    // direction of edge P2-P0.
    float du2 = tcoord2[0] - tcoord0[0];
    float dv2 = tcoord2[1] - tcoord0[1];
    float det = dv1 * du2 - dv2 * du1;
    if (std::fabs(det) <= epsilon)
    {
        // The triangle vertices are collinear in parameter space, so treat
        // this as a degenerate parametric surface.
        return false;
    }

    // The triangle vertices are not collinear in parameter space, so choose
    // the tangent to be dP/du = (dv1*dP2-dv2*dP1)/(dv1*du2-dv2*du1)
    tangent = (dv1 * deltaPos2 - dv2 * deltaPos1) / det;
    return true;
}