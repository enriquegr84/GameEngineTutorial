// David Eberly, Geometric Tools, Redmond WA 98052
// Copyright (c) 1998-2017
// Distributed under the Boost Software License, Version 1.0.
// http://www.boost.org/LICENSE_1_0.txt
// http://www.geometrictools.com/License/Boost/LICENSE_1_0.txt
// File Version: 3.0.0 (2016/06/19)

#ifndef BASICEFFECT_H
#define BASICEFFECT_H

#include "Mathematic/Algebra/Matrix4x4.h"

#include "Graphic/Effect/VisualEffect.h"

class GRAPHIC_ITEM BasicEffect : public VisualEffect
{
public:
    // Construction.
	BasicEffect(eastl::shared_ptr<ProgramFactory> const& factory, eastl::string path);

	// Member access.
	inline void SetPVWMatrix(Matrix4x4<float> const& pvwMatrix);
	inline Matrix4x4<float> const& GetPVWMatrix() const;

	// Required to bind and update resources.
	inline eastl::shared_ptr<ConstantBuffer> const& GetPVWMatrixConstant() const;

	void SetPVWMatrixConstant(eastl::shared_ptr<ConstantBuffer> const& pvwMatrix);

private:
	// Vertex shader parameters.
	eastl::shared_ptr<ConstantBuffer> mPVWMatrixConstant;

	// Convenience pointers.
	Matrix4x4<float>* mPVWMatrix;
};


inline void BasicEffect::SetPVWMatrix(Matrix4x4<float> const& pvwMatrix)
{
	*mPVWMatrix = pvwMatrix;
}

inline Matrix4x4<float> const& BasicEffect::GetPVWMatrix() const
{
	return *mPVWMatrix;
}

inline eastl::shared_ptr<ConstantBuffer> const& BasicEffect::GetPVWMatrixConstant() const
{
	return mPVWMatrixConstant;
}

#endif