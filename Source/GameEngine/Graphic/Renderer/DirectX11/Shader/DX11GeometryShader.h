// David Eberly, Geometric Tools, Redmond WA 98052
// Copyright (c) 1998-2017
// Distributed under the Boost Software License, Version 1.0.
// http://www.boost.org/LICENSE_1_0.txt
// http://www.geometrictools.com/License/Boost/LICENSE_1_0.txt
// File Version: 3.0.0 (2016/06/19)

#ifndef DX11GEOMETRYSHADER_H
#define DX11GEOMETRYSHADER_H

#include "Graphic/Shader/Shader.h"
#include "DX11Shader.h"

/*
	Geometry Shader Stage (GS) operates on complete primitives (such as points,
	lines and triangles). Moreover, geometry shaders have the capability to add or
	remove geometry from the pipeline. Connected to the GS stage is the stream-output
	stage (SO). This stage streams in the vertices output from the geometry shader
	and stores them in memory. For multipass rendering, this data can be read back
	into the pipeline in a subsequent pass, or the CPU can read the data. As with
	the tessellation stages, the geometry shader stage is optional.
*/
class GRAPHIC_ITEM DX11GeometryShader : public DX11Shader
{
public:
    // Construction.
    DX11GeometryShader(ID3D11Device* device, Shader const* shader);
    static eastl::shared_ptr<GraphicObject> Create(void* device, GraphicObject const* object);

    // Overrides to hide DX11 functions that have "GS" embedded in their names.

    // Calls to ID3D11DeviceContext::GSSetShader.
    virtual void Enable(ID3D11DeviceContext* context) override;
    virtual void Disable(ID3D11DeviceContext* context) override;

    // Calls to ID3D11DeviceContext::GSSetConstantBuffers.
    virtual void EnableCBuffer(ID3D11DeviceContext* context,
        unsigned int bindPoint, ID3D11Buffer* buffer) override;
    virtual void DisableCBuffer(ID3D11DeviceContext* context,
        unsigned int bindPoint) override;

    // Calls to ID3D11DeviceContext::GSSetShaderResources.
    virtual void EnableSRView(ID3D11DeviceContext* context,
        unsigned int bindPoint, ID3D11ShaderResourceView* srView) override;
    virtual void DisableSRView(ID3D11DeviceContext* context,
        unsigned int bindPoint) override;

    // Unordered access views are supported in geometry shaders starting
    // with D3D11.1.
    virtual void EnableUAView(ID3D11DeviceContext* context,
        unsigned int bindPoint, ID3D11UnorderedAccessView* uaView,
        unsigned int initialCount) override;
    virtual void DisableUAView(ID3D11DeviceContext* context,
        unsigned int bindPoint) override;

    // Calls to ID3D11DeviceContext::GSSetSamplers.
    virtual void EnableSampler(ID3D11DeviceContext* context,
        unsigned int bindPoint, ID3D11SamplerState* state) override;
    virtual void DisableSampler(ID3D11DeviceContext* context,
        unsigned int bindPoint) override;
};

#endif