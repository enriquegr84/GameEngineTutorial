// David Eberly, Geometric Tools, Redmond WA 98052
// Copyright (c) 1998-2017
// Distributed under the Boost Software License, Version 1.0.
// http://www.boost.org/LICENSE_1_0.txt
// http://www.geometrictools.com/License/Boost/LICENSE_1_0.txt
// File Version: 3.0.0 (2016/06/19)

#include "DX11SamplerState.h"

DX11SamplerState::DX11SamplerState(ID3D11Device* device, SamplerState const* samplerState)
    :
    DX11DrawingState(samplerState)
{
    // Specify the sampler state description.
    D3D11_SAMPLER_DESC desc;
    desc.Filter = msFilter[samplerState->mFilter];
    desc.AddressU = msMode[samplerState->mMode[0]];
    desc.AddressV = msMode[samplerState->mMode[1]];
    desc.AddressW = msMode[samplerState->mMode[2]];
    desc.MipLODBias = samplerState->mMipLODBias;
    desc.MaxAnisotropy = samplerState->mMaxAnisotropy;
    desc.ComparisonFunc = msComparison[samplerState->mComparison];
    desc.BorderColor[0] = samplerState->mBorderColor[0];
    desc.BorderColor[1] = samplerState->mBorderColor[1];
    desc.BorderColor[2] = samplerState->mBorderColor[2];
    desc.BorderColor[3] = samplerState->mBorderColor[3];
    desc.MinLOD = samplerState->mMinLOD;
    desc.MaxLOD = samplerState->mMaxLOD;

    // Create the sampler state.
    ID3D11SamplerState* state = nullptr;
    HRESULT hr = device->CreateSamplerState(&desc, &state);
	if (FAILED(hr))
	{
		LogError("Failed to create sampler state, hr =" +
			GetErrorMessage(hr) + ".");
	}
    mDXObject = state;
}

eastl::shared_ptr<GraphicObject> DX11SamplerState::Create(
	void* device, GraphicObject const* object)
{
    if (object->GetType() == GE_SAMPLER_STATE)
    {
        return eastl::make_shared<DX11SamplerState>(
            reinterpret_cast<ID3D11Device*>(device),
            static_cast<SamplerState const*>(object));
    }

    LogError("Invalid object type.");
    return nullptr;
}


D3D11_FILTER const DX11SamplerState::msFilter[] =
{
    D3D11_FILTER_MIN_MAG_MIP_POINT,
    D3D11_FILTER_MIN_MAG_POINT_MIP_LINEAR,
    D3D11_FILTER_MIN_POINT_MAG_LINEAR_MIP_POINT,
    D3D11_FILTER_MIN_POINT_MAG_MIP_LINEAR,
    D3D11_FILTER_MIN_LINEAR_MAG_MIP_POINT,
    D3D11_FILTER_MIN_LINEAR_MAG_POINT_MIP_LINEAR,
    D3D11_FILTER_MIN_MAG_LINEAR_MIP_POINT,
    D3D11_FILTER_MIN_MAG_MIP_LINEAR,
    D3D11_FILTER_ANISOTROPIC,
    D3D11_FILTER_COMPARISON_MIN_MAG_MIP_POINT,
    D3D11_FILTER_COMPARISON_MIN_MAG_POINT_MIP_LINEAR,
    D3D11_FILTER_COMPARISON_MIN_POINT_MAG_LINEAR_MIP_POINT,
    D3D11_FILTER_COMPARISON_MIN_POINT_MAG_MIP_LINEAR,
    D3D11_FILTER_COMPARISON_MIN_LINEAR_MAG_MIP_POINT,
    D3D11_FILTER_COMPARISON_MIN_LINEAR_MAG_POINT_MIP_LINEAR,
    D3D11_FILTER_COMPARISON_MIN_MAG_LINEAR_MIP_POINT,
    D3D11_FILTER_COMPARISON_MIN_MAG_MIP_LINEAR,
    D3D11_FILTER_COMPARISON_ANISOTROPIC
};

D3D11_TEXTURE_ADDRESS_MODE const DX11SamplerState::msMode[] =
{
    D3D11_TEXTURE_ADDRESS_WRAP,
    D3D11_TEXTURE_ADDRESS_MIRROR,
    D3D11_TEXTURE_ADDRESS_CLAMP,
    D3D11_TEXTURE_ADDRESS_BORDER,
    D3D11_TEXTURE_ADDRESS_MIRROR_ONCE
};

D3D11_COMPARISON_FUNC const DX11SamplerState::msComparison[] =
{
    D3D11_COMPARISON_NEVER,
    D3D11_COMPARISON_LESS,
    D3D11_COMPARISON_EQUAL,
    D3D11_COMPARISON_LESS_EQUAL,
    D3D11_COMPARISON_GREATER,
    D3D11_COMPARISON_NOT_EQUAL,
    D3D11_COMPARISON_GREATER_EQUAL,
    D3D11_COMPARISON_ALWAYS
};
