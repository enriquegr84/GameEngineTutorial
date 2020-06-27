// David Eberly, Geometric Tools, Redmond WA 98052
// Copyright (c) 1998-2017
// Distributed under the Boost Software License, Version 1.0.
// http://www.boost.org/LICENSE_1_0.txt
// http://www.geometrictools.com/License/Boost/LICENSE_1_0.txt
// File Version: 3.0.1 (2016/11/13)

#include "Texture2ArrayEffect.h"

Texture2ArrayEffect::Texture2ArrayEffect(eastl::shared_ptr<ProgramFactory> const& factory, 
	eastl::vector<eastl::string> path, eastl::shared_ptr<Texture2Array> const& textures,
	SamplerState::Filter filter, SamplerState::Mode mode0, SamplerState::Mode mode1)
{
	eastl::string vsPath = path[0];
	eastl::string psPath = path[1];
	eastl::string gsPath = "";
	mProgram = factory->CreateFromFiles(vsPath, psPath, gsPath);
    if (mProgram)
    {
		mSampler = eastl::make_shared<SamplerState>();
		mSampler->mFilter = filter;
		mSampler->mMode[0] = mode0;
		mSampler->mMode[1] = mode1;

		mTextures = textures;
#if defined(_OPENGL_)
		mProgram->GetPShader()->Set("baseSampler", mTextures);
		mProgram->GetPShader()->Set("baseSampler", mSampler);
#else
		mProgram->GetPShader()->Set("baseTextureArray", mTextures);
#endif

#if defined(_OPENGL_)
		mProgram->GetVShader()->Set("PVWMatrix", mPVWMatrixConstant);
#else
		mProgram->GetPShader()->Set("baseSampler", mSampler);
		mProgram->GetVShader()->Set("PVWMatrix", mPVWMatrixConstant);
#endif
    }
}

void Texture2ArrayEffect::SetPVWMatrixConstant(eastl::shared_ptr<ConstantBuffer> const& pvwMatrix)
{
    mPVWMatrixConstant = pvwMatrix;
    mProgram->GetVShader()->Set("PVWMatrix", mPVWMatrixConstant);
}

void Texture2ArrayEffect::SetTextures(eastl::shared_ptr<Texture2Array> const& textures)
{
	mTextures = textures;
#if defined(_OPENGL_)
	mProgram->GetPShader()->Set("baseSampler", mTextures);
	mProgram->GetPShader()->Set("baseSampler", mSampler);
#else
	mProgram->GetPShader()->Set("baseTextureArray", mTextures);
#endif

}
