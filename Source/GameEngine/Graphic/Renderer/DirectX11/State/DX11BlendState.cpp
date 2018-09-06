// David Eberly, Geometric Tools, Redmond WA 98052
// Copyright (c) 1998-2017
// Distributed under the Boost Software License, Version 1.0.
// http://www.boost.org/LICENSE_1_0.txt
// http://www.geometrictools.com/License/Boost/LICENSE_1_0.txt
// File Version: 3.0.0 (2016/06/19)

#include "DX11BlendState.h"

DX11BlendState::DX11BlendState (ID3D11Device* device, BlendState const* blendState)
    :
    DX11DrawingState(blendState)
{
    // Specify the blend state description.
    D3D11_BLEND_DESC desc;
    desc.AlphaToCoverageEnable = (blendState->mEnableAlphaToCoverage ? TRUE : FALSE);
    desc.IndependentBlendEnable = (blendState->mEnableIndependentBlend ? TRUE : FALSE);
    for (unsigned int i = 0; i < BlendState::NUM_TARGETS; ++i)
    {
        D3D11_RENDER_TARGET_BLEND_DESC& out = desc.RenderTarget[i];
        BlendState::Target const& in = blendState->mTarget[i];
        out.BlendEnable = (in.enable ? TRUE : FALSE);
        out.SrcBlend = msMode[in.srcColor];
        out.DestBlend = msMode[in.dstColor];
        out.BlendOp = msOperation[in.opColor];
        out.SrcBlendAlpha = msMode[in.srcAlpha];
        out.DestBlendAlpha = msMode[in.dstAlpha];
        out.BlendOpAlpha = msOperation[in.opAlpha];
        out.RenderTargetWriteMask = in.mask;
    }

    // Create the blend state.
    ID3D11BlendState* state = nullptr;
    HRESULT hr = device->CreateBlendState(&desc, &state);
	if (FAILED(hr))
	{
		LogError("Failed to create blend state, hr =" + GetErrorMessage(hr));
	}
    mDXObject = state;
}

eastl::shared_ptr<GraphicObject> DX11BlendState::Create(
	void* device, GraphicObject const* object)
{
    if (object->GetType() == GE_BLEND_STATE)
    {
        return eastl::make_shared<DX11BlendState>(
            reinterpret_cast<ID3D11Device*>(device),
            static_cast<BlendState const*>(object));
    }

    LogError("Invalid object type.");
    return nullptr;
}

void DX11BlendState::Enable(ID3D11DeviceContext* context)
{
    BlendState* bState = GetBlendState();
    context->OMSetBlendState(GetDXBlendState(), &bState->mBlendColor[0], bState->mSampleMask);
}


D3D11_BLEND const DX11BlendState::msMode[] =
{
    D3D11_BLEND_ZERO,
    D3D11_BLEND_ONE,
    D3D11_BLEND_SRC_COLOR,
    D3D11_BLEND_INV_SRC_COLOR,
    D3D11_BLEND_SRC_ALPHA,
    D3D11_BLEND_INV_SRC_ALPHA,
    D3D11_BLEND_DEST_ALPHA,
    D3D11_BLEND_INV_DEST_ALPHA,
    D3D11_BLEND_DEST_COLOR,
    D3D11_BLEND_INV_DEST_COLOR,
    D3D11_BLEND_SRC_ALPHA_SAT,
    D3D11_BLEND_BLEND_FACTOR,
    D3D11_BLEND_INV_BLEND_FACTOR,
    D3D11_BLEND_SRC1_COLOR,
    D3D11_BLEND_INV_SRC1_COLOR,
    D3D11_BLEND_SRC1_ALPHA,
    D3D11_BLEND_INV_SRC1_ALPHA
};

D3D11_BLEND_OP const DX11BlendState::msOperation[] =
{
    D3D11_BLEND_OP_ADD,
    D3D11_BLEND_OP_SUBTRACT,
    D3D11_BLEND_OP_REV_SUBTRACT,
    D3D11_BLEND_OP_MIN,
    D3D11_BLEND_OP_MAX
};
