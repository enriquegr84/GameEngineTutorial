// David Eberly, Geometric Tools, Redmond WA 98052
// Copyright (c) 1998-2017
// Distributed under the Boost Software License, Version 1.0.
// http://www.boost.org/LICENSE_1_0.txt
// http://www.geometrictools.com/License/Boost/LICENSE_1_0.txt
// File Version: 3.0.1 (2016/11/13)

#ifndef TEXTURE2ARRAYEFFECT_H
#define TEXTURE2ARRAYEFFECT_H

#include "Mathematic/Algebra/Matrix4x4.h"

#include "Graphic/Resource/Texture/Texture2Array.h"
#include "Graphic/Effect/VisualEffect.h"


class GRAPHIC_ITEM Texture2ArrayEffect : public VisualEffect
{
public:
    // Constructionn.
	Texture2ArrayEffect(eastl::shared_ptr<ProgramFactory> const& factory, 
		eastl::vector<eastl::string> path, eastl::shared_ptr<Texture2Array> const& textures,
		SamplerState::Filter filter, SamplerState::Mode mode0, SamplerState::Mode mode1);

    // Member access.
    inline void SetPVWMatrix(Matrix4x4<float> const& pvwMatrix);
    inline Matrix4x4<float> const& GetPVWMatrix() const;

    // Required to bind and update resources.
    inline eastl::shared_ptr<ConstantBuffer> const& GetPVWMatrixConstant() const;
    inline eastl::shared_ptr<Texture2Array> const& GetTextures() const;
    inline eastl::shared_ptr<SamplerState> const& GetSampler() const;

    void SetPVWMatrixConstant(eastl::shared_ptr<ConstantBuffer> const& pvwMatrix);
	void SetTextures(eastl::shared_ptr<Texture2Array> const& textures);

private:
    // Vertex shader parameters.
	eastl::shared_ptr<ConstantBuffer> mPVWMatrixConstant;

    // Pixel shader parameters.
	eastl::shared_ptr<Texture2Array> mTextures;
	eastl::shared_ptr<SamplerState> mSampler;

    // Convenience pointer.
    Matrix4x4<float>* mPVWMatrix;
};


inline void Texture2ArrayEffect::SetPVWMatrix(Matrix4x4<float> const& pvwMatrix)
{
    *mPVWMatrix = pvwMatrix;
}

inline Matrix4x4<float> const& Texture2ArrayEffect::GetPVWMatrix() const
{
    return *mPVWMatrix;
}

inline eastl::shared_ptr<ConstantBuffer> const& Texture2ArrayEffect::GetPVWMatrixConstant() const
{
    return mPVWMatrixConstant;
}

inline eastl::shared_ptr<Texture2Array> const& Texture2ArrayEffect::GetTextures() const
{
    return mTextures;
}

inline eastl::shared_ptr<SamplerState> const& Texture2ArrayEffect::GetSampler() const
{
    return mSampler;
}

#endif
