// David Eberly, Geometric Tools, Redmond WA 98052
// Copyright (c) 1998-2017
// Distributed under the Boost Software License, Version 1.0.
// http://www.boost.org/LICENSE_1_0.txt
// http://www.geometrictools.com/License/Boost/LICENSE_1_0.txt
// File Version: 3.0.0 (2016/06/19)

#ifndef DX11PIXELSHADER_H
#define DX11PIXELSHADER_H

#include "Graphic/Shader/Shader.h"
#include "DX11Shader.h"

/*
	Pixels Shader Stage (PS) executes shader code against each pixel input from
	the rasterizer stage and ouputs a color. This gives the programmer control over
	every pixel that's rendered to the screen. The pixel shader uses interpolated
	per-vertex data, global variables, and texture data to produce its output.
*/
class GRAPHIC_ITEM DX11PixelShader : public DX11Shader
{
public:
    // Construction.
    DX11PixelShader(ID3D11Device* device, Shader const* shader);
    static eastl::shared_ptr<GraphicObject> Create(void* device, GraphicObject const* object);

    // Overrides to hide DX11 functions that have "PS" embedded in their names.

    // Calls to ID3D11DeviceContext::PSSetShader.
    virtual void Enable(ID3D11DeviceContext* context) override;
    virtual void Disable(ID3D11DeviceContext* context) override;

    // Calls to ID3D11DeviceContext::PSSetConstantBuffers.
    virtual void EnableCBuffer(ID3D11DeviceContext* context,
        unsigned int bindPoint, ID3D11Buffer* buffer) override;
    virtual void DisableCBuffer(ID3D11DeviceContext* context,
        unsigned int bindPoint) override;

    // Calls to ID3D11DeviceContext::PSSetShaderResources.
    virtual void EnableSRView(ID3D11DeviceContext* context,
        unsigned int bindPoint, ID3D11ShaderResourceView* srView) override;
    virtual void DisableSRView(ID3D11DeviceContext* context,
        unsigned int bindPoint) override;

    // Calls to ID3D11DeviceContext::PSSetUnorderedAccessViews.
    virtual void EnableUAView(ID3D11DeviceContext* context,
        unsigned int bindPoint, ID3D11UnorderedAccessView* uaView,
        unsigned int initialCount) override;
    virtual void DisableUAView(ID3D11DeviceContext* context,
        unsigned int bindPoint) override;

    // Calls to ID3D11DeviceContext::PSSetSamplers.
    virtual void EnableSampler(ID3D11DeviceContext* context,
        unsigned int bindPoint, ID3D11SamplerState* state) override;
    virtual void DisableSampler(ID3D11DeviceContext* context,
        unsigned int bindPoint) override;
};

#endif