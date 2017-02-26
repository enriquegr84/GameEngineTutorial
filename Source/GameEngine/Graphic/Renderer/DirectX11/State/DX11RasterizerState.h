// David Eberly, Geometric Tools, Redmond WA 98052
// Copyright (c) 1998-2017
// Distributed under the Boost Software License, Version 1.0.
// http://www.boost.org/LICENSE_1_0.txt
// http://www.geometrictools.com/License/Boost/LICENSE_1_0.txt
// File Version: 3.0.0 (2016/06/19)

#ifndef DX11RASTERIZERSTATE_H
#define DX11RASTERIZERSTATE_H

#include "Graphic/State/RasterizerState.h"
#include "Graphic/Renderer/DirectX11/State/DX11DrawingState.h"

/*
	Rasterizer Stage (RS) converts primitives into a raster image. A raster
	image is represented as 2-dimensional array of pixels (colors) and generally
	makes up some area of your computer screen. The rasterizer stage determines what,
	if any, pizels should be rendered and passes those pixels to the pixel shader
	stage. Along with the pixels to render, the rasterizer passes per-vertex values
	interpolated across each primitive.
*/
class GRAPHIC_ITEM DX11RasterizerState : public DX11DrawingState
{
public:
    // Construction.
    DX11RasterizerState(ID3D11Device* device, RasterizerState const* rasterizerState);
    static eastl::shared_ptr<GraphicObject> Create(void* device, GraphicObject const* object);

    // Member access.
    inline RasterizerState* GetRasterizerState();
    inline ID3D11RasterizerState* GetDXRasterizerState();

    // Enable the rasterizer state.
    void Enable(ID3D11DeviceContext* context);

private:
    // Conversions from GTEngine values to DX11 values.
    static D3D11_FILL_MODE const msFillMode[];
    static D3D11_CULL_MODE const msCullMode[];
};

inline RasterizerState* DX11RasterizerState::GetRasterizerState()
{
    return static_cast<RasterizerState*>(mGObject);
}

inline ID3D11RasterizerState* DX11RasterizerState::GetDXRasterizerState()
{
    return static_cast<ID3D11RasterizerState*>(mDXObject);
}

#endif