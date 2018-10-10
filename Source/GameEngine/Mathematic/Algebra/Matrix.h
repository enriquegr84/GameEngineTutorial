// David Eberly, Geometric Tools, Redmond WA 98052
// Copyright (c) 1998-2017
// Distributed under the Boost Software License, Version 1.0.
// http://www.boost.org/LICENSE_1_0.txt
// http://www.geometrictools.com/License/Boost/LICENSE_1_0.txt
// File Version: 3.0.1 (2016/08/07)

#ifndef MATRIX_H
#define MATRIX_H

#include "Vector.h"

#include "Mathematic/NumericalMethod/GaussianElimination.h"

template <int NumRows, int NumCols, typename Real>
class Matrix;

template <int NumRows, int NumCols, typename Real>
Matrix<NumRows, NumCols, Real>
operator+(Matrix<NumRows, NumCols, Real> const& M);

template <int NumRows, int NumCols, typename Real>
Matrix<NumRows, NumCols, Real>
operator-(Matrix<NumRows, NumCols, Real> const& M);

template <int NumRows, int NumCols, typename Real>
Matrix<NumRows, NumCols, Real> operator+(
	Matrix<NumRows, NumCols, Real> const& M0,
	Matrix<NumRows, NumCols, Real> const& M1);

template <int NumRows, int NumCols, typename Real>
Matrix<NumRows, NumCols, Real> operator-(
	Matrix<NumRows, NumCols, Real> const& M0,
	Matrix<NumRows, NumCols, Real> const& M1);

template <int NumRows, int NumCols, typename Real>
Matrix<NumRows, NumCols, Real>
operator*(Matrix<NumRows, NumCols, Real> const& M, Real scalar);

template <int NumRows, int NumCols, typename Real>
Matrix<NumRows, NumCols, Real>
operator*(Real scalar, Matrix<NumRows, NumCols, Real> const& M);

template <int NumRows, int NumCols, typename Real>
Matrix<NumRows, NumCols, Real>
operator/(Matrix<NumRows, NumCols, Real> const& M, Real scalar);

template <int NumRows, int NumCols, typename Real>
Matrix<NumRows, NumCols, Real>& operator+=(
	Matrix<NumRows, NumCols, Real>& M0,
	Matrix<NumRows, NumCols, Real> const& M1);

template <int NumRows, int NumCols, typename Real>
Matrix<NumRows, NumCols, Real>& operator-=(
	Matrix<NumRows, NumCols, Real>& M0,
	Matrix<NumRows, NumCols, Real> const& M1);

template <int NumRows, int NumCols, typename Real>
Matrix<NumRows, NumCols, Real>& operator*=(
	Matrix<NumRows, NumCols, Real>& M, Real scalar);

template <int NumRows, int NumCols, typename Real>
Matrix<NumRows, NumCols, Real>& operator/=(
	Matrix<NumRows, NumCols, Real>& M, Real scalar);

template <int NumRows, int NumCols, typename Real>
Real L1Norm(Matrix<NumRows, NumCols, Real> const& M);

template <int NumRows, int NumCols, typename Real>
Real L2Norm(Matrix<NumRows, NumCols, Real> const& M);

template <int NumRows, int NumCols, typename Real>
Real LInfinityNorm(Matrix<NumRows, NumCols, Real> const& M);

template <int NumRows, int NumCols, typename Real>
Matrix<NumRows, NumCols, Real> Inverse(
	Matrix<NumRows, NumCols, Real> const& M,
	bool* reportInvertibility = nullptr);

template <int NumRows, int NumCols, typename Real>
Real Determinant(Matrix<NumRows, NumCols, Real> const& M);

template <int NumRows, int NumCols, typename Real>
Matrix<NumCols, NumRows, Real>
Transpose(Matrix<NumRows, NumCols, Real> const& M);

template <int NumRows, int NumCols, typename Real>
Vector<NumRows, Real> operator*(
	Matrix<NumRows, NumCols, Real> const& M,
	Vector<NumCols, Real> const& V);

template <int NumRows, int NumCols, typename Real>
Vector<NumCols, Real> operator*(
	Vector<NumRows, Real> const& V,
	Matrix<NumRows, NumCols, Real> const& M);

template <int NumRows, int NumCols, int NumCommon, typename Real>
Matrix<NumRows, NumCols, Real> operator*(
	Matrix<NumRows, NumCommon, Real> const& A,
	Matrix<NumCommon, NumCols, Real> const& B);

template <int NumRows, int NumCols, int NumCommon, typename Real>
Matrix<NumRows, NumCols, Real> MultiplyAB(
	Matrix<NumRows, NumCommon, Real> const& A,
	Matrix<NumCommon, NumCols, Real> const& B);

template <int NumRows, int NumCols, int NumCommon, typename Real>
Matrix<NumRows, NumCols, Real> MultiplyABT(
	Matrix<NumRows, NumCommon, Real> const& A,
	Matrix<NumCols, NumCommon, Real> const& B);

template <int NumRows, int NumCols, int NumCommon, typename Real>
Matrix<NumRows, NumCols, Real> MultiplyATB(
	Matrix<NumCommon, NumRows, Real> const& A,
	Matrix<NumCommon, NumCols, Real> const& B);

template <int NumRows, int NumCols, int NumCommon, typename Real>
Matrix<NumRows, NumCols, Real> MultiplyATBT(
	Matrix<NumCommon, NumRows, Real> const& A,
	Matrix<NumCols, NumCommon, Real> const& B);

template <int NumRows, int NumCols, typename Real>
Matrix<NumRows, NumCols, Real> MultiplyMD(
	Matrix<NumRows, NumCols, Real> const& M,
	Vector<NumCols, Real> const& D);

template <int NumRows, int NumCols, typename Real>
Matrix<NumRows, NumCols, Real> MultiplyDM(
	Vector<NumRows, Real> const& D,
	Matrix<NumRows, NumCols, Real> const& M);

template <int NumRows, int NumCols, typename Real>
Matrix<NumRows, NumCols, Real> OuterProduct(
	Vector<NumRows, Real> const& U, Vector<NumCols, Real> const& V);

template <int NumRows, int NumCols, typename Real>
void MakeDiagonal(Vector<NumRows, Real> const& D, Matrix<NumRows, NumCols, Real>& M);

template <int NumRows, int NumCols, typename Real>
Matrix<NumRows + 1, NumCols + 1, Real> HLift(Matrix<NumRows, NumCols, Real> const& M);

// Extract the upper (N-1)-by-(N-1) block of the input N-by-N matrix.
template <int NumRows, int NumCols, typename Real>
Matrix<NumRows - 1, NumCols - 1, Real> HProject(Matrix<NumRows, NumCols, Real> const& M);

template <int NumRows, int NumCols, typename Real>
class Matrix
{
public:
    // The table is initialized to zero.
    Matrix();

    // The table is fully initialized by the inputs.  The 'values' must be
    // specified in row-major order, regardless of the active storage scheme
    // (GE_USE_ROW_MAJOR or GE_USE_COL_MAJOR).
    Matrix(eastl::array<Real, NumRows*NumCols> const& values);

    // At most NumRows*NumCols are copied from the initializer list, setting
    // any remaining elements to zero.  The 'values' must be specified in
    // row-major order, regardless of the active storage scheme
    // (GE_USE_ROW_MAJOR or GE_USE_COL_MAJOR).  Create the zero matrix using
    // the syntax
    //   Matrix<NumRows,NumCols,Real> zero{(Real)0};
    // WARNING: The C++ 11 specification states that
    //   Matrix<NumRows,NumCols,Real> zero{};
    // will lead to a call of the default constructor, not the initializer
    // constructor!
    Matrix(std::initializer_list<Real> values);

    // For 0 <= r < NumRows and 0 <= c < NumCols, element (r,c) is 1 and all
    // others are 0.  If either of r or c is invalid, the zero matrix is
    // created.  This is a convenience for creating the standard Euclidean
    // basis matrices; see also MakeUnit(int,int) and Unit(int,int).
    Matrix(int r, int c);

    // The copy constructor, destructor, and assignment operator are generated
    // by the compiler.

    // Member access for which the storage representation is transparent.  The
    // matrix entry in row r and column c is A(r,c).  The first operator()
    // returns a const reference rather than a Real value.  This supports
    // writing via standard file operations that require a const pointer to
    // data.
    inline Real const& operator()(int r, int c) const;
    inline Real& operator()(int r, int c);

	// Member access by 1-dimensional index.  NOTE: These accessors are
	// useful for the manipulation of matrix entries when it does not
	// matter whether storage is row-major or column-major.  Do not use
	// constructs such as M[c+NumCols*r] or M[r+NumRows*c] that expose the
	// storage convention.
	inline Real const& operator[](int i) const;
	inline Real& operator[](int i);

    // Member access by rows or by columns.
    void SetRow(int r, Vector<NumCols,Real> const& vec);
    void SetCol(int c, Vector<NumRows,Real> const& vec);
    Vector<NumCols,Real> GetRow(int r) const;
    Vector<NumRows,Real> GetCol(int c) const;

	// Transformation
	void Transformation(
		Vector<NumRows, Real> const& in, Vector<NumRows, Real>& out) const;

    // Comparisons for sorted containers and geometric ordering.
    inline bool operator==(Matrix const& mat) const;
    inline bool operator!=(Matrix const& mat) const;

    // Special matrices.
    void MakeZero();  // All components are 0.
    void MakeUnit(int r, int c);  // Component (r,c) is 1, all others zero.
    void MakeIdentity();  // Diagonal entries 1, others 0, even when nonsquare
    static Matrix Zero();
    static Matrix Unit(int r, int c);
    static Matrix Identity();

	// Unary operations.
	friend Matrix operator+<NumRows, NumCols, Real>(Matrix const& M);
	friend Matrix operator-<NumRows, NumCols, Real>(Matrix const& M);

	// Linear-algebraic operations.
	friend Matrix operator+<NumRows, NumCols, Real>(Matrix const& M0, Matrix const& M1);
	friend Matrix operator-<NumRows, NumCols, Real>(Matrix const& M0, Matrix const& M1);
	friend Matrix operator*<NumRows, NumCols, Real>(Matrix const& M, Real scalar);
	friend Matrix operator*<NumRows, NumCols, Real>(Real scalar, Matrix const& M);
	friend Matrix operator/<NumRows, NumCols, Real>(Matrix const& M, Real scalar);
	friend Matrix& operator+=<NumRows, NumCols, Real>(Matrix& M0, Matrix const& M1);
	friend Matrix& operator-=<NumRows, NumCols, Real>(Matrix& M0, Matrix const& M1);
	friend Matrix& operator*=<NumRows, NumCols, Real>(Matrix& M, Real scalar);
	friend Matrix& operator/=<NumRows, NumCols, Real>(Matrix& M, Real scalar);

	// Geometric operations.
	friend Real L1Norm<NumRows, NumCols, Real>(Matrix const& M);
	friend Real L2Norm<NumRows, NumCols, Real>(Matrix const& M);
	friend Real LInfinityNorm<NumRows, NumCols, Real>(Matrix const& M);

	friend Matrix Inverse<NumRows, NumCols, Real>(Matrix const& M, bool* reportInvertibility);
	friend Real Determinant<NumRows, NumCols, Real>(Matrix const& M);

	// M^T
	friend Matrix<NumCols, NumRows, Real> Transpose<NumRows, NumCols, Real>(Matrix const& M);

	// M*V
	friend Vector<NumRows, Real> operator*<NumRows, NumCols, Real>
		(Matrix const& M, Vector<NumCols, Real> const& V);

	// V^T*M
	friend Vector<NumCols, Real> operator*<NumRows, NumCols, Real>
		(Vector<NumRows, Real> const& V, Matrix const& M);

	// A*B
	template <int NumRows, int NumCols, int NumCommon, typename Real>
	friend Matrix<NumRows, NumCols, Real> operator*<NumRows, NumCols, Real>(
			Matrix<NumRows, NumCommon, Real> const& A,
			Matrix<NumCommon, NumCols, Real> const& B);

	template <int NumRows, int NumCols, int NumCommon, typename Real>
	friend Matrix<NumRows, NumCols, Real> MultiplyAB<NumRows, NumCols, Real>(
			Matrix<NumRows, NumCommon, Real> const& A,
			Matrix<NumCommon, NumCols, Real> const& B);

	// A*B^T
	template <int NumRows, int NumCols, int NumCommon, typename Real>
	friend Matrix<NumRows, NumCols, Real> MultiplyABT<NumRows, NumCols, Real>(
			Matrix<NumRows, NumCommon, Real> const& A,
			Matrix<NumCols, NumCommon, Real> const& B);

	// A^T*B
	template <int NumRows, int NumCols, int NumCommon, typename Real>
	friend Matrix<NumRows, NumCols, Real> MultiplyATB<NumRows, NumCols, Real>(
			Matrix<NumCommon, NumRows, Real> const& A,
			Matrix<NumCommon, NumCols, Real> const& B);

	// A^T*B^T
	template <int NumRows, int NumCols, int NumCommon, typename Real>
	friend Matrix<NumRows, NumCols, Real> MultiplyATBT<NumRows, NumCols, Real>(
			Matrix<NumCommon, NumRows, Real> const& A,
			Matrix<NumCols, NumCommon, Real> const& B);

	// M*D, D is diagonal NumCols-by-NumCols
	friend Matrix MultiplyMD<NumRows, NumCols, Real>(Matrix const& M, Vector<NumCols, Real> const& D);

	// D*M, D is diagonal NumRows-by-NumRows
	friend Matrix MultiplyDM<NumRows, NumCols, Real>(Vector<NumRows, Real> const& D, Matrix const& M);

	// U*V^T, U is NumRows-by-1, V is Num-Cols-by-1, result is NumRows-by-NumCols.
	friend Matrix OuterProduct<NumRows, NumCols, Real>(
		Vector<NumRows, Real> const& U, Vector<NumCols, Real> const& V);

	// Initialization to a diagonal matrix whose diagonal entries are the
	// components of D.
	friend void MakeDiagonal<NumRows, NumCols, Real>(Vector<NumRows, Real> const& D, Matrix& M);

	// Create an (N+1)-by-(N+1) matrix H by setting the upper N-by-N block to the
	// input N-by-N matrix and all other entries to 0 except for the last row
	// and last column entry which is set to 1.
	friend Matrix<NumRows + 1, NumCols + 1, Real> HLift<NumRows, NumCols, Real>(Matrix const& M);

	// Extract the upper (N-1)-by-(N-1) block of the input N-by-N matrix.
	friend Matrix<NumRows - 1, NumCols - 1, Real> HProject<NumRows, NumCols, Real>(Matrix const& M);

protected:

	Real mTable[NumCols*NumRows];
};

template <int NumRows, int NumCols, typename Real>
Matrix<NumRows, NumCols, Real>::Matrix()
{

}

template <int NumRows, int NumCols, typename Real>
Matrix<NumRows, NumCols, Real>::Matrix(
	eastl::array<Real, NumRows*NumCols> const& values)
{
	Real const* in = reinterpret_cast<Real const*>(&values);
    for (int r = 0, i = 0; r < NumRows; ++r)
    {
        for (int c = 0; c < NumCols; ++c, ++i)
        {
            mTable[r * NumCols + c] = in[i];
        }
    }
}

template <int NumRows, int NumCols, typename Real>
Matrix<NumRows, NumCols, Real>::Matrix(std::initializer_list<Real> values)
{
    int const numValues = static_cast<int>(values.size());
    auto iter = values.begin();
    int r, c, i;
    for (r = 0, i = 0; r < NumRows; ++r)
    {
        for (c = 0; c < NumCols; ++c, ++i)
        {
            if (i < numValues)
            {
                mTable[r * NumCols + c] = *iter++;
            }
            else
            {
                break;
            }
        }

        if (c < NumCols)
        {
            // Fill in the remaining columns of the current row with zeros.
            for (/**/; c < NumCols; ++c)
            {
                mTable[r * NumCols + c] = (Real)0;
            }
            ++r;
            break;
        }
    }

    if (r < NumRows)
    {
        // Fill in the remain rows with zeros.
        for (/**/; r < NumRows; ++r)
        {
            for (c = 0; c < NumCols; ++c)
            {
                mTable[r * NumCols + c] = (Real)0;
            }
        }
    }
}

template <int NumRows, int NumCols, typename Real>
Matrix<NumRows, NumCols, Real>::Matrix(int r, int c)
{
    MakeUnit(r, c);
}

template <int NumRows, int NumCols, typename Real> inline
Real const& Matrix<NumRows, NumCols, Real>::operator()(int r, int c) const
{
	return mTable[r * NumCols + c];
}

template <int NumRows, int NumCols, typename Real> inline
Real& Matrix<NumRows, NumCols, Real>::operator()(int r, int c)
{
	return mTable[r * NumCols + c];
}

template <int NumRows, int NumCols, typename Real> inline
Real const& Matrix<NumRows, NumCols, Real>::operator[](int i) const
{
	return mTable[i];
}

template <int NumRows, int NumCols, typename Real> inline
Real& Matrix<NumRows, NumCols, Real>::operator[](int i)
{
	return mTable[i];
}

template <int NumRows, int NumCols, typename Real>
void Matrix<NumRows, NumCols, Real>::SetRow(int r,
    Vector<NumCols, Real> const& vec)
{
	Real const* in = reinterpret_cast<Real const*>(&vec);
    for (int c = 0; c < NumCols; ++c)
    {
        mTable[r * NumCols + c] = in[c];
    }
}

template <int NumRows, int NumCols, typename Real>
void Matrix<NumRows, NumCols, Real>::SetCol(int c,
    Vector<NumRows, Real> const& vec)
{
	Real const* in = reinterpret_cast<Real const*>(&vec);
    for (int r = 0; r < NumRows; ++r)
    {
        mTable[r * NumCols + c] = in[r];
    }
}

template <int NumRows, int NumCols, typename Real>
Vector<NumCols, Real> Matrix<NumRows, NumCols, Real>::GetRow(int r) const
{
    Vector<NumCols, Real> vec;
	Real* in = reinterpret_cast<Real*>(&vec);
    for (int c = 0; c < NumCols; ++c)
    {
        in[c] = mTable[r * NumCols + c];
    }
    return vec;
}

template <int NumRows, int NumCols, typename Real>
Vector<NumRows, Real> Matrix<NumRows, NumCols, Real>::GetCol(int c) const
{
    Vector<NumRows, Real> vec;
	Real* in = reinterpret_cast<Real*>(&vec);
    for (int r = 0; r < NumRows; ++r)
    {
        in[r] = mTable[r * NumCols + c];
    }
    return vec;
}

template <int NumRows, int NumCols, typename Real>
void Matrix<NumRows, NumCols, Real>::Transformation(
	Vector<NumRows, Real> const& in, Vector<NumRows, Real>& out) const
{
	Real* vOut = reinterpret_cast<Real*>(&out);
	Real const* vIn = reinterpret_cast<Real const*>(&in);
	for (int r = 0; r < NumRows; ++r)
	{
		vOut[r] = (Real)0;
		for (int c = 0; c < NumCols; ++c)
		{
			vOut[r] += vIn[c] * mTable[c * NumCols + r];
		}
	}
}

template <int NumRows, int NumCols, typename Real> inline
bool Matrix<NumRows, NumCols, Real>::operator==(Matrix const& mat) const
{
	for (int i = 0; i < NumRows*NumCols; ++i)
		if (mTable[i] != mat.mTable[i])
			return false;

	return true;
}

template <int NumRows, int NumCols, typename Real> inline
bool Matrix<NumRows, NumCols, Real>::operator!=(Matrix const& mat) const
{
	return !(*this == mat);
}

template <int NumRows, int NumCols, typename Real>
void Matrix<NumRows, NumCols, Real>::MakeZero()
{
    Real const zero = (Real)0;
    for (int i = 0; i < NumRows * NumCols; ++i)
    {
        mTable[i] = zero;
    }
}

template <int NumRows, int NumCols, typename Real>
void Matrix<NumRows, NumCols, Real>::MakeUnit(int r, int c)
{
    MakeZero();
    if (0 <= r && r < NumRows && 0 <= c && c < NumCols)
    {
        mTable[r * NumCols + c] = (Real)1;
    }
}

template <int NumRows, int NumCols, typename Real>
void Matrix<NumRows, NumCols, Real>::MakeIdentity()
{
    MakeZero();
    int const numDiagonal = (NumRows <= NumCols ? NumRows : NumCols);
    for (int i = 0; i < numDiagonal; ++i)
    {
        mTable[i * NumCols + i] = (Real)1;
    }
}

template <int NumRows, int NumCols, typename Real>
Matrix<NumRows, NumCols, Real> Matrix<NumRows, NumCols, Real>::Zero()
{
    Matrix M;
    M.MakeZero();
    return M;
}

template <int NumRows, int NumCols, typename Real>
Matrix<NumRows, NumCols, Real> Matrix<NumRows, NumCols, Real>::Unit(int r,
    int c)
{
    Matrix M;
    M.MakeUnit(r, c);
    return M;
}

template <int NumRows, int NumCols, typename Real>
Matrix<NumRows, NumCols, Real> Matrix<NumRows, NumCols, Real>::Identity()
{
    Matrix M;
    M.MakeIdentity();
    return M;
}

template <int NumRows, int NumCols, typename Real> inline
Matrix<NumRows, NumCols, Real> operator+(
	Matrix<NumRows, NumCols, Real> const& M)
{
    return M;
}

template <int NumRows, int NumCols, typename Real> inline
Matrix<NumRows, NumCols, Real> operator-(
	Matrix<NumRows, NumCols, Real> const& M)
{
    Matrix<NumRows, NumCols, Real> result;
    for (int i = 0; i < NumRows*NumCols; ++i)
    {
        result.mTable[i] = -M.mTable[i];
    }
    return result;
}

template <int NumRows, int NumCols, typename Real> inline
Matrix<NumRows, NumCols, Real> operator+(
    Matrix<NumRows, NumCols, Real> const& M0,
    Matrix<NumRows, NumCols, Real> const& M1)
{
    Matrix<NumRows, NumCols, Real> result = M0;
    return result += M1;
}

template <int NumRows, int NumCols, typename Real> inline
Matrix<NumRows, NumCols, Real> operator-(
    Matrix<NumRows, NumCols, Real> const& M0,
    Matrix<NumRows, NumCols, Real> const& M1)
{
    Matrix<NumRows, NumCols, Real> result = M0;
    return result -= M1;
}

template <int NumRows, int NumCols, typename Real> inline
Matrix<NumRows, NumCols, Real> operator*(
	Matrix<NumRows, NumCols, Real> const& M, Real scalar)
{
    Matrix<NumRows, NumCols, Real> result = M;
    return result *= scalar;
}

template <int NumRows, int NumCols, typename Real> inline
Matrix<NumRows, NumCols, Real> operator*(
	Real scalar, Matrix<NumRows, NumCols, Real> const& M)
{
    Matrix<NumRows, NumCols, Real> result = M;
    return result *= scalar;
}

template <int NumRows, int NumCols, typename Real> inline
Matrix<NumRows, NumCols, Real> operator/(
	Matrix<NumRows, NumCols, Real> const& M, Real scalar)
{
    Matrix<NumRows, NumCols, Real> result = M;
    return result /= scalar;
}

template <int NumRows, int NumCols, typename Real> inline
Matrix<NumRows, NumCols, Real>& operator+=(
    Matrix<NumRows, NumCols, Real>& M0,
    Matrix<NumRows, NumCols, Real> const& M1)
{
    for (int i = 0; i < NumRows*NumCols; ++i)
    {
        M0.mTable[i] += M1.mTable[i];
    }
    return M0;
}

template <int NumRows, int NumCols, typename Real> inline
Matrix<NumRows, NumCols, Real>& operator-=(
    Matrix<NumRows, NumCols, Real>& M0,
    Matrix<NumRows, NumCols, Real> const& M1)
{
    for (int i = 0; i < NumRows*NumCols; ++i)
    {
        M0.mTable[i] -= M1.mTable[i];
    }
    return M0;
}

template <int NumRows, int NumCols, typename Real> inline
Matrix<NumRows, NumCols, Real>& operator*=(
	Matrix<NumRows, NumCols, Real>& M, Real scalar)
{
    for (int i = 0; i < NumRows*NumCols; ++i)
    {
        M.mTable[i] *= scalar;
    }
    return M;
}

template <int NumRows, int NumCols, typename Real> inline
Matrix<NumRows, NumCols, Real>& operator/=(
	Matrix<NumRows, NumCols, Real>& M, Real scalar)
{
    if (scalar != (Real)0)
    {
        Real invScalar = ((Real)1) / scalar;
        for (int i = 0; i < NumRows*NumCols; ++i)
        {
            M.mTable[i] *= invScalar;
        }
    }
    else
    {
        for (int i = 0; i < NumRows*NumCols; ++i)
        {
            M.mTable[i] = (Real)0;
        }
    }
    return M;
}

template <int NumRows, int NumCols, typename Real> inline
Real L1Norm(Matrix<NumRows, NumCols, Real> const& M)
{
    Real sum = fabs(M.mTable[0]);
    for (int i = 1; i < NumRows*NumCols; ++i)
    {
        sum += fabs(M.mTable[i]);
    }
    return sum;
}

template <int NumRows, int NumCols, typename Real> inline
Real L2Norm(Matrix<NumRows, NumCols, Real> const& M)
{
    Real sum = M.mTable[0] * M.mTable[0];
    for (int i = 1; i < NumRows*NumCols; ++i)
    {
        sum += M.mTable[i] * M.mTable[i];
    }
    return sqrt(sum);
}

template <int NumRows, int NumCols, typename Real> inline
Real LInfinityNorm(Matrix<NumRows, NumCols, Real> const& M)
{
    Real maxAbsElement = M.mTable[0];
    for (int i = 1; i < NumRows*NumCols; ++i)
    {
        Real absElement = fabs(M.mTable[i]);
        if (absElement > maxAbsElement)
        {
            maxAbsElement = absElement;
        }
    }
    return maxAbsElement;
}

template <int NumRows, int NumCols, typename Real> inline
Matrix<NumRows, NumCols, Real> Inverse(
	Matrix<NumRows, NumCols, Real> const& M, bool* reportInvertibility)
{
    Matrix<NumRows, NumCols, Real> invM;
    Real determinant;
    bool invertible = GaussianElimination<Real>()(
		NumRows, &M.mTable[0], &invM.mTable[0],
        determinant, nullptr, nullptr, nullptr, 0, nullptr);
    if (reportInvertibility)
    {
        *reportInvertibility = invertible;
    }
    return invM;
}

template <int NumRows, int NumCols, typename Real> inline
Real Determinant(Matrix<NumRows, NumCols, Real> const& M)
{
    Real determinant;
    GaussianElimination<Real>()(NumRows, &M.mTable[0], nullptr, determinant, nullptr,
        nullptr, nullptr, 0, nullptr);
    return determinant;
}

template <int NumRows, int NumCols, typename Real> inline
Matrix<NumCols, NumRows, Real>
Transpose(Matrix<NumRows, NumCols, Real> const& M)
{
    Matrix<NumCols, NumRows, Real> result;
    for (int r = 0; r < NumRows; ++r)
    {
        for (int c = 0; c < NumCols; ++c)
        {
            result.mTable[c * NumCols + r] = M.mTable[r * NumCols + c];
        }
    }
    return result;
}

template <int NumRows, int NumCols, typename Real> inline
Vector<NumRows, Real> operator*(
    Matrix<NumRows, NumCols, Real> const& M,
    Vector<NumCols, Real> const& V)
{
    Vector<NumRows, Real> result;
	Real* out = reinterpret_cast<Real*>(&result);
	Real const* in = reinterpret_cast<Real const*>(&V);
	for (int r = 0; r < NumRows; ++r)
	{
		out[r] = (Real)0;
		for (int c = 0; c < NumCols; ++c)
		{
			out[r] += M.mTable[r * NumCols + c] * in[c];
		}
	}
    return result;
}

template <int NumRows, int NumCols, typename Real> inline
Vector<NumCols, Real> operator*(
	Vector<NumRows, Real> const& V,
    Matrix<NumRows, NumCols, Real> const& M)
{
    Vector<NumCols, Real> result;
	Real* out = reinterpret_cast<Real*>(&result);
	Real const* in = reinterpret_cast<Real const*>(&V);
    for (int c = 0; c < NumCols; ++c)
    {
		out[c] = (Real)0;
        for (int r = 0; r < NumRows; ++r)
        {
			out[c] += in[r] * M.mTable[r * NumCols + c];
        }
    }
    return result;
}

template <int NumRows, int NumCols, int NumCommon, typename Real> inline
Matrix<NumRows, NumCols, Real> operator*(
    Matrix<NumRows, NumCommon, Real> const& A,
    Matrix<NumCommon, NumCols, Real> const& B)
{
	Matrix<NumRows, NumCols, Real> result;
	for (int r = 0; r < NumRows; ++r)
	{
		for (int c = 0; c < NumCols; ++c)
		{
			result.mTable[r * NumCols + c] = (Real)0;
			for (int i = 0; i < NumCommon; ++i)
			{
				result.mTable[r * NumCols + c] +=
					A.mTable[r * NumCols + i] * B.mTable[i * NumCols + c];
			}
		}
	}
	return result;
}

template <int NumRows, int NumCols, int NumCommon, typename Real> inline
Matrix<NumRows, NumCols, Real> MultiplyAB(
    Matrix<NumRows, NumCommon, Real> const& A,
    Matrix<NumCommon, NumCols, Real> const& B)
{
    Matrix<NumRows, NumCols, Real> result;
	for (int r = 0; r < NumRows; ++r)
	{
		for (int c = 0; c < NumCols; ++c)
		{
			result.mTable[r * NumCols + c] = (Real)0;
			for (int i = 0; i < NumCommon; ++i)
			{
				result.mTable[r * NumCols + c] +=
					A.mTable[i * NumCols + c] * B.mTable[r * NumCols + i];
			}
		}
	}
    return result;
}

template <int NumRows, int NumCols, int NumCommon, typename Real> inline
Matrix<NumRows, NumCols, Real> MultiplyABT(
    Matrix<NumRows, NumCommon, Real> const& A,
    Matrix<NumCols, NumCommon, Real> const& B)
{
    Matrix<NumRows, NumCols, Real> result;
    for (int r = 0; r < NumRows; ++r)
    {
        for (int c = 0; c < NumCols; ++c)
        {
            result.mTable[r * NumCols + c] = (Real)0;
            for (int i = 0; i < NumCommon; ++i)
            {
                result.mTable[r * NumCols + c] +=
					A.mTable[i * NumCols + c] * B.mTable[i * NumCols + r];
            }
        }
    }
    return result;
}

template <int NumRows, int NumCols, int NumCommon, typename Real> inline
Matrix<NumRows, NumCols, Real> MultiplyATB(
    Matrix<NumCommon, NumRows, Real> const& A,
    Matrix<NumCommon, NumCols, Real> const& B)
{
    Matrix<NumRows, NumCols, Real> result;
    for (int r = 0; r < NumRows; ++r)
    {
        for (int c = 0; c < NumCols; ++c)
        {
            result.mTable[r * NumCols + c] = (Real)0;
            for (int i = 0; i < NumCommon; ++i)
            {
                result.mTable[r * NumCols + c] +=
					A.mTable[c * NumCols + i] * B.mTable[r * NumCols + i];
            }
        }
    }
    return result;
}

template <int NumRows, int NumCols, int NumCommon, typename Real> inline
Matrix<NumRows, NumCols, Real> MultiplyATBT(
    Matrix<NumCommon, NumRows, Real> const& A,
    Matrix<NumCols, NumCommon, Real> const& B)
{
    Matrix<NumRows, NumCols, Real> result;
    for (int r = 0; r < NumRows; ++r)
    {
        for (int c = 0; c < NumCols; ++c)
        {
            result.mTable[r * NumCols + c] = (Real)0;
            for (int i = 0; i < NumCommon; ++i)
            {
                result.mTable[r * NumCols + c] +=
					A.mTable[c * NumCols + i] * B.mTable[i * NumCols + r];
            }
        }
    }
    return result;
}

template <int NumRows, int NumCols, typename Real> inline
Matrix<NumRows, NumCols, Real> MultiplyMD(
    Matrix<NumRows, NumCols, Real> const& M,
    Vector<NumCols, Real> const& D)
{
    Matrix<NumRows, NumCols, Real> result;
	Real const* in = reinterpret_cast<Real const*>(&D);
    for (int r = 0; r < NumRows; ++r)
    {
        for (int c = 0; c < NumCols; ++c)
        {
            result.mTable[r * NumCols + c] = M.mTable[r * NumCols + c] * in[c];
        }
    }
    return result;
}

template <int NumRows, int NumCols, typename Real> inline
Matrix<NumRows, NumCols, Real> MultiplyDM(
    Vector<NumRows, Real> const& D,
    Matrix<NumRows, NumCols, Real> const& M)
{
    Matrix<NumRows, NumCols, Real> result;
	Real const* in = reinterpret_cast<Real const*>(&D);
    for (int r = 0; r < NumRows; ++r)
    {
        for (int c = 0; c < NumCols; ++c)
        {
            result.mTable[r * NumCols + c] = in[r] * M.mTable[r * NumCols + c];
        }
    }
    return result;
}

template <int NumRows, int NumCols, typename Real> inline
Matrix<NumRows, NumCols, Real> OuterProduct(
	Vector<NumRows, Real> const& U, Vector<NumCols, Real> const& V)
{
    Matrix<NumRows, NumCols, Real> result;
	Real const* inU = reinterpret_cast<Real const*>(&U);
	Real const* inV = reinterpret_cast<Real const*>(&V);
    for (int r = 0; r < NumRows; ++r)
    {
        for (int c = 0; c < NumCols; ++c)
        {
            result.mTable[r * NumCols + c] = inU[r] * inV[c];
        }
    }
    return result;
}

template <int NumRows, int NumCols, typename Real> inline
void MakeDiagonal(Vector<NumRows, Real> const& D, Matrix<NumRows, NumCols, Real>& M)
{
	Real const* in = reinterpret_cast<Real const*>(&D);
    for (int i = 0; i < NumRows*NumCols; ++i)
    {
        M.mTable[i] = (Real)0;
    }

    for (int i = 0; i < NumRows; ++i)
    {
        M.mTable[i * NumCols + i] = in[i];
    }
}

template <int NumRows, int NumCols, typename Real> inline
Matrix<NumRows + 1, NumCols + 1, Real> HLift(Matrix<NumRows, NumCols, Real> const& M)
{
    Matrix<NumRows + 1, NumCols + 1, Real> result;
	Real* out = reinterpret_cast<Real*>(&result);
    for (int r = 0; r < NumRows; ++r)
    {
        for (int c = 0; c < NumCols; ++c)
        {
			out[r * NumCols + c] = M.mTable[r * NumCols + c];
        }
    }
    return result;
}

// Extract the upper (N-1)-by-(N-1) block of the input N-by-N matrix.
template <int NumRows, int NumCols, typename Real> inline
Matrix<NumRows - 1, NumCols - 1, Real> HProject(Matrix<NumRows, NumCols, Real> const& M)
{
    static_assert(NumRows >= 2, "Invalid matrix dimension.");
    Matrix<NumRows - 1, NumCols - 1, Real> result;
	Real* out = reinterpret_cast<Real*>(&result);
    for (int r = 0; r < NumRows - 1; ++r)
    {
        for (int c = 0; c < NumCols - 1; ++c)
        {
			out[r * NumCols + c] = M.mTable[r * NumCols + c];
        }
    }
    return out;
}


#endif