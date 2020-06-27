// David Eberly, Geometric Tools, Redmond WA 98052
// Copyright (c) 1998-2017
// Distributed under the Boost Software License, Version 1.0.
// http://www.boost.org/LICENSE_1_0.txt
// http://www.geometrictools.com/License/Boost/LICENSE_1_0.txt
// File Version: 3.0.0 (2016/06/19)

#include "VisualEffect.h"

VisualEffect::~VisualEffect()
{
}

VisualEffect::VisualEffect()
{
	mPVWMatrixConstant = eastl::make_shared<ConstantBuffer>(sizeof(Matrix4x4<float>), true);
	SetPVWMatrix(Matrix4x4<float>::Identity());
}

VisualEffect::VisualEffect(eastl::shared_ptr<VisualProgram> const& program)
    :
    mProgram(program)
{
	mPVWMatrixConstant = eastl::make_shared<ConstantBuffer>(sizeof(Matrix4x4<float>), true);
	SetPVWMatrix(Matrix4x4<float>::Identity());
}

void VisualEffect::SetPVWMatrixConstant(eastl::shared_ptr<ConstantBuffer> const& buffer)
{
	mPVWMatrixConstant = buffer;
	SetPVWMatrix(Matrix4x4<float>::Identity());
}

