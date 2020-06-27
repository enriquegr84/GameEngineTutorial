// David Eberly, Geometric Tools, Redmond WA 98052
// Copyright (c) 1998-2017
// Distributed under the Boost Software License, Version 1.0.
// http://www.boost.org/LICENSE_1_0.txt
// http://www.geometrictools.com/License/Boost/LICENSE_1_0.txt
// File Version: 3.0.1 (2016/11/13)

#ifndef TEXTURE2EFFECT_H
#define TEXTURE2EFFECT_H

#include "Mathematic/Algebra/Matrix4x4.h"

#include "Graphic/Resource/Texture/Texture2.h"
#include "Graphic/Effect/VisualEffect.h"


class GRAPHIC_ITEM Texture2Effect : public VisualEffect
{
public:
    // Constructionn.
    Texture2Effect(eastl::shared_ptr<ProgramFactory> const& factory,
		eastl::vector<eastl::string> path, eastl::shared_ptr<Texture2> const& texture,
		SamplerState::Filter filter, SamplerState::Mode mode0, SamplerState::Mode mode1);

    inline eastl::shared_ptr<Texture2> const& GetTexture() const;
    inline eastl::shared_ptr<SamplerState> const& GetSampler() const;

    void SetPVWMatrixConstant(eastl::shared_ptr<ConstantBuffer> const& pvwMatrix);
	void SetTexture(eastl::shared_ptr<Texture2> const& texture);

private:

    // Pixel shader parameters.
	eastl::shared_ptr<Texture2> mTexture;
	eastl::shared_ptr<SamplerState> mSampler;

};

inline eastl::shared_ptr<Texture2> const& Texture2Effect::GetTexture() const
{
    return mTexture;
}

inline eastl::shared_ptr<SamplerState> const& Texture2Effect::GetSampler() const
{
    return mSampler;
}

#endif