// David Eberly, Geometric Tools, Redmond WA 98052
// Copyright (c) 1998-2017
// Distributed under the Boost Software License, Version 1.0.
// http://www.boost.org/LICENSE_1_0.txt
// http://www.geometrictools.com/License/Boost/LICENSE_1_0.txt
// File Version: 3.0.0 (2016/06/19)

#include "TextEffect.h"

#include "Mathematic/Algebra/Vector2.h"

TextEffect::TextEffect(eastl::shared_ptr<ProgramFactory> const& factory,
	eastl::vector<eastl::string> path, eastl::shared_ptr<Texture2> const& texture)
{

	eastl::string vsPath = path.front();
	eastl::string psPath = path.size() > 1 ? path[1] : path.front();
	eastl::string gsPath = path.size() > 2 ? path[2] : "";
	mProgram = factory->CreateFromFiles(vsPath, psPath, gsPath);
    if (mProgram)
    {
        mTranslate = eastl::make_shared<ConstantBuffer>(sizeof(Vector2<float>), true);
        mColor = eastl::make_shared<ConstantBuffer>(sizeof(Vector4<float>), true);
        mSamplerState = eastl::make_shared<SamplerState>();

        SetTranslate(0.0f, 0.0f);
        mProgram->GetVShader()->Set("Translate", mTranslate);

        SetColor({ 0.0f, 0.0f, 0.0f, 0.0f });
        mProgram->GetPShader()->Set("TextColor", mColor);
#if defined(_OPENGL_)
        mProgram->GetPShader()->Set("baseSampler", texture);
#else
        mProgram->GetPShader()->Set("baseTexture", texture);
#endif
        mProgram->GetPShader()->Set("baseSampler", mSamplerState);
    }
}

eastl::shared_ptr<ConstantBuffer> const& TextEffect::GetTranslate() const
{
    return mTranslate;
}

eastl::shared_ptr<ConstantBuffer> const& TextEffect::GetColor() const
{
    return mColor;
}

void TextEffect::SetTranslate(float x, float  y)
{
    float* data = mTranslate->Get<float>();
    data[0] = x;
    data[1] = y;
}

void TextEffect::SetColor(Vector4<float> const& color)
{
    Vector4<float>* data = mColor->Get<Vector4<float>>();
    *data = color;
}