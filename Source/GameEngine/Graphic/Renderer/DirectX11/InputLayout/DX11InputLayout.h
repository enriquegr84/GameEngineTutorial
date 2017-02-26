// David Eberly, Geometric Tools, Redmond WA 98052
// Copyright (c) 1998-2017
// Distributed under the Boost Software License, Version 1.0.
// http://www.boost.org/LICENSE_1_0.txt
// http://www.geometrictools.com/License/Boost/LICENSE_1_0.txt
// File Version: 3.0.0 (2016/06/19)

#ifndef DX11INPUTLAYOUT_H
#define DX11INPUTLAYOUT_H

#include "Graphic/Shader/Shader.h"
#include "Graphic/Resource/Buffer/VertexBuffer.h"

#include "Graphic/Renderer/DirectX11/Resource/DX11GraphicObject.h"

class GRAPHIC_ITEM DX11InputLayout
{
public:
    // Construction and destruction.
    ~DX11InputLayout();
    DX11InputLayout(ID3D11Device* device, VertexBuffer const* vbuffer,
        Shader const* vshader);

    // Support for drawing geometric primitives.
    void Enable(ID3D11DeviceContext* context);
    void Disable(ID3D11DeviceContext* context);

private:
    ID3D11InputLayout* mLayout;
    int mNumElements;
    D3D11_INPUT_ELEMENT_DESC mElements[VA_MAX_ATTRIBUTES];

    // Conversions from GameEngine values to DX11 values.
    static char const* msSemantic[VA_NUM_SEMANTICS];
};

#endif