// David Eberly, Geometric Tools, Redmond WA 98052
// Copyright (c) 1998-2017
// Distributed under the Boost Software License, Version 1.0.
// http://www.boost.org/LICENSE_1_0.txt
// http://www.geometrictools.com/License/Boost/LICENSE_1_0.txt
// File Version: 3.0.1 (2016/11/13)

#include "MultiTexture2Effect.h"

MultiTexture2Effect::MultiTexture2Effect(
	eastl::shared_ptr<ProgramFactory> const& factory, eastl::vector<eastl::string> path,
	eastl::vector<eastl::shared_ptr<Texture2>> const& texture,
	SamplerState::Filter filter, SamplerState::Mode mode0, SamplerState::Mode mode1)
    :
    mPVWMatrix(nullptr)
{
	eastl::string vsPath = path.front();
	eastl::string psPath = path.size() > 1 ? path[1] : path.front();
	eastl::string gsPath = path.size() > 2 ? path[2] : "";
	mProgram = factory->CreateFromFiles(vsPath, psPath, gsPath);
    if (mProgram)
    {
        mPVWMatrixConstant = eastl::make_shared<ConstantBuffer>(sizeof(Matrix4x4<float>), true);
        mPVWMatrix = mPVWMatrixConstant->Get<Matrix4x4<float>>();
        *mPVWMatrix = Matrix4x4<float>::Identity();

		mSampler = eastl::make_shared<SamplerState>();
		mSampler->mFilter = filter;
		mSampler->mMode[0] = mode0;
		mSampler->mMode[1] = mode1;

		for (unsigned int i = 0; i < texture.size(); i++)
		{
			mTextures.push_back(texture[i]);

#if defined(_OPENGL_)
			mProgram->GetPShader()->Set("baseSampler" + eastl::to_string(i + 1), mTextures[i]);
			mProgram->GetPShader()->Set("baseSampler" + eastl::to_string(i + 1), mSampler);
#else
			mProgram->GetPShader()->Set("baseTexture" + eastl::to_string(i+1), mTextures[i]);
#endif
		}

#if defined(_OPENGL_)
		mProgram->GetVShader()->Set("PVWMatrix", mPVWMatrixConstant);
#else
		mProgram->GetPShader()->Set("baseSampler", mSampler);
		mProgram->GetVShader()->Set("PVWMatrix", mPVWMatrixConstant);
#endif
    }
}

void MultiTexture2Effect::SetPVWMatrixConstant(eastl::shared_ptr<ConstantBuffer> const& pvwMatrix)
{
    mPVWMatrixConstant = pvwMatrix;
    mProgram->GetVShader()->Set("PVWMatrix", mPVWMatrixConstant);
}

void MultiTexture2Effect::SetTextures(eastl::vector<eastl::shared_ptr<Texture2>> const& textures)
{
	mTextures.clear();
	for (unsigned int i = 0; i < textures.size(); i++)
	{
		mTextures.push_back(textures[i]);

#if defined(_OPENGL_)
		mProgram->GetPShader()->Set("baseSampler" + eastl::to_string(i + 1), mTextures[i]);
		mProgram->GetPShader()->Set("baseSampler" + eastl::to_string(i + 1), mSampler);
#else
		mProgram->GetPShader()->Set("baseTexture" + eastl::to_string(i+1), mTextures[i]);
#endif
	}

}