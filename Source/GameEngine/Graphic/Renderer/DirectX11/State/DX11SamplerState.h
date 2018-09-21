// David Eberly, Geometric Tools, Redmond WA 98052
// Copyright (c) 1998-2017
// Distributed under the Boost Software License, Version 1.0.
// http://www.boost.org/LICENSE_1_0.txt
// http://www.geometrictools.com/License/Boost/LICENSE_1_0.txt
// File Version: 3.0.0 (2016/06/19)

#ifndef DX11SAMPLERSTATE_H
#define DX11SAMPLERSTATE_H

#include "Graphic/State/SamplerState.h"
#include "DX11DrawingState.h"

class GRAPHIC_ITEM DX11SamplerState : public DX11DrawingState
{
public:
    // Construction.
    DX11SamplerState(ID3D11Device* device, SamplerState const* samplerState);
    static eastl::shared_ptr<GraphicObject> Create(void* device, GraphicObject const* object);

    // Member access.
    inline SamplerState* GetSamplerState();
    inline ID3D11SamplerState* GetDXSamplerState();

private:
    // Conversions from GEngine values to DX11 values.
    static D3D11_FILTER const msFilter[];
    static D3D11_TEXTURE_ADDRESS_MODE const msMode[];
    static D3D11_COMPARISON_FUNC const msComparison[];
};

inline SamplerState* DX11SamplerState::GetSamplerState()
{
    return static_cast<SamplerState*>(mGObject);
}

inline ID3D11SamplerState* DX11SamplerState::GetDXSamplerState()
{
    return static_cast<ID3D11SamplerState*>(mDXObject);
}

#endif