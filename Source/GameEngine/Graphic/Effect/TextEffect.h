// David Eberly, Geometric Tools, Redmond WA 98052
// Copyright (c) 1998-2017
// Distributed under the Boost Software License, Version 1.0.
// http://www.boost.org/LICENSE_1_0.txt
// http://www.geometrictools.com/License/Boost/LICENSE_1_0.txt
// File Version: 3.0.0 (2016/06/19)

#ifndef TEXTEFFECT_H
#define TEXTEFFECT_H

#include "Mathematic/Algebra/Vector4.h"

#include "Graphic/Shader/ProgramFactory.h"
#include "Graphic/State/SamplerState.h"
#include "Graphic/Effect/VisualEffect.h"
#include "Graphic/Resource/Texture/Texture2.h"
#include "Graphic/Resource/Buffer/ConstantBuffer.h"

class GRAPHIC_ITEM TextEffect : public VisualEffect
{
public:
    // Construction.
    TextEffect(eastl::shared_ptr<ProgramFactory> const& factory,
		eastl::string path, eastl::shared_ptr<Texture2> const& texture);

    // Support for typesetting.
	eastl::shared_ptr<ConstantBuffer> const& GetTranslate() const;
	eastl::shared_ptr<ConstantBuffer> const& GetColor() const;
    void SetTranslate(float x, float y);
    void SetColor(Vector4<float> const& color);

private:
	eastl::shared_ptr<ConstantBuffer> mTranslate;
	eastl::shared_ptr<ConstantBuffer> mColor;
	eastl::shared_ptr<SamplerState> mSamplerState;
};

#endif