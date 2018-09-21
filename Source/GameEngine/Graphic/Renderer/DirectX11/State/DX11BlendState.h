// David Eberly, Geometric Tools, Redmond WA 98052
// Copyright (c) 1998-2017
// Distributed under the Boost Software License, Version 1.0.
// http://www.boost.org/LICENSE_1_0.txt
// http://www.geometrictools.com/License/Boost/LICENSE_1_0.txt
// File Version: 3.0.0 (2016/06/19)

#ifndef DX11BLENDSTATE_H
#define DX11BLENDSTATE_H

#include "Graphic/State/BlendState.h"
#include "DX11DrawingState.h"

class GRAPHIC_ITEM DX11BlendState : public DX11DrawingState
{
public:
    // Construction.
    DX11BlendState(ID3D11Device* device, BlendState const* blendState);
    static eastl::shared_ptr<GraphicObject> Create(void* device, GraphicObject const* object);

    // Member access.
    inline BlendState* GetBlendState();
    inline ID3D11BlendState* GetDXBlendState();

    // Enable the blend state.
    void Enable(ID3D11DeviceContext* context);

private:
    // Conversions from GEngine values to DX11 values.
    static D3D11_BLEND const msMode[];
    static D3D11_BLEND_OP const msOperation[];
};

inline BlendState* DX11BlendState::GetBlendState()
{
    return static_cast<BlendState*>(mGObject);
}

inline ID3D11BlendState* DX11BlendState::GetDXBlendState()
{
    return static_cast<ID3D11BlendState*>(mDXObject);
}

#endif