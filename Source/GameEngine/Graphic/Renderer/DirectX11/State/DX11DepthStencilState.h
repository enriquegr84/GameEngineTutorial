// David Eberly, Geometric Tools, Redmond WA 98052
// Copyright (c) 1998-2017
// Distributed under the Boost Software License, Version 1.0.
// http://www.boost.org/LICENSE_1_0.txt
// http://www.geometrictools.com/License/Boost/LICENSE_1_0.txt
// File Version: 3.0.0 (2016/06/19)

#ifndef DX11DEPTHSTENCILSTATE_H
#define DX11DEPTHSTENCILSTATE_H

#include "Graphic/State/DepthStencilState.h"
#include "DX11DrawingState.h"

/*
	The OM stage determines which pixels are visible in the final render
	through processes known as depth testing and stencil testing. Depth testing
	uses data that has previously been written to the render target to determine
	whether a pixel should be drawn. Depth testing makes use of the distance
	between the object and the camera for each corresponding pixel written to the
	render target. If the pixel already in the render target is closer to the camera
	than the pixel being considered, the new pixel is discarded (this is the case
	when one object occlude the ones behind them).
*/
class GRAPHIC_ITEM DX11DepthStencilState : public DX11DrawingState
{
public:
    // Construction.
    DX11DepthStencilState(ID3D11Device* device, DepthStencilState const* depthStencilState);
    static eastl::shared_ptr<GraphicObject> Create(void* device, GraphicObject const* object);

    // Member access.
    inline DepthStencilState* GetDepthStencilState();
    inline ID3D11DepthStencilState* GetDXDepthStencilState();

    // Enable the depth-stencil state.
    void Enable(ID3D11DeviceContext* context);

private:
    // Conversions from GTEngine values to DX11 values.
    static D3D11_DEPTH_WRITE_MASK const msWriteMask[];
    static D3D11_COMPARISON_FUNC const msComparison[];
    static D3D11_STENCIL_OP const msOperation[];
};

inline DepthStencilState* DX11DepthStencilState::GetDepthStencilState()
{
    return static_cast<DepthStencilState*>(mGObject);
}

inline ID3D11DepthStencilState* DX11DepthStencilState::GetDXDepthStencilState()
{
    return static_cast<ID3D11DepthStencilState*>(mDXObject);
}

#endif