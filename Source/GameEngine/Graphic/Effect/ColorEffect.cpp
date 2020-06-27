// David Eberly, Geometric Tools, Redmond WA 98052
// Copyright (c) 1998-2017
// Distributed under the Boost Software License, Version 1.0.
// http://www.boost.org/LICENSE_1_0.txt
// http://www.geometrictools.com/License/Boost/LICENSE_1_0.txt
// File Version: 3.0.0 (2016/06/19)

#include "ColorEffect.h"

#include "Mathematic/Algebra/Vector2.h"

ColorEffect::ColorEffect(
	eastl::shared_ptr<ProgramFactory> const& factory, 
	eastl::vector<eastl::string> const& path)
{
	eastl::string vsPath = path[0];
	eastl::string psPath = path[1];
	eastl::string gsPath = "";
	mProgram = factory->CreateFromFiles(vsPath, psPath, gsPath);
	if (mProgram)
		mProgram->GetVShader()->Set("PVWMatrix", mPVWMatrixConstant);
}

void ColorEffect::SetPVWMatrixConstant(eastl::shared_ptr<ConstantBuffer> const& pvwMatrix)
{
	mPVWMatrixConstant = pvwMatrix;
	mProgram->GetVShader()->Set("PVWMatrix", mPVWMatrixConstant);
}