// David Eberly, Geometric Tools, Redmond WA 98052
// Copyright (c) 1998-2017
// Distributed under the Boost Software License, Version 1.0.
// http://www.boost.org/LICENSE_1_0.txt
// http://www.geometrictools.com/License/Boost/LICENSE_1_0.txt
// File Version: 3.0.0 (2016/06/19)

#include "Core/Logger/Logger.h"

#include "DX11Texture1.h"

DX11Texture1::DX11Texture1(ID3D11Device* device, Texture1 const* texture)
    :
    DX11TextureSingle(texture)
{
    // Specify the texture description.
    D3D11_TEXTURE1D_DESC desc;
    desc.Width = texture->GetLength();
    desc.MipLevels = texture->GetNumLevels();
    desc.ArraySize = 1;
    desc.Format = static_cast<DXGI_FORMAT>(texture->GetFormat());
    desc.BindFlags = D3D11_BIND_SHADER_RESOURCE;
    desc.MiscFlags = D3D11_RESOURCE_MISC_NONE;
    Resource::Usage usage = texture->GetUsage();
    if (usage == Resource::IMMUTABLE)
    {
        desc.Usage = D3D11_USAGE_IMMUTABLE;
        desc.CPUAccessFlags = D3D11_CPU_ACCESS_NONE;
    }
    else if (usage == Resource::DYNAMIC_UPDATE)
    {
        desc.Usage = D3D11_USAGE_DYNAMIC;
        desc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
    }
    else  // usage == Resource::SHADER_OUTPUT
    {
        desc.Usage = D3D11_USAGE_DEFAULT;
        desc.BindFlags |= D3D11_BIND_UNORDERED_ACCESS;
        desc.CPUAccessFlags = D3D11_CPU_ACCESS_NONE;
    }

    if (texture->WantAutogenerateMipmaps())
    {
        desc.Usage = D3D11_USAGE_DEFAULT;
        desc.BindFlags |= D3D11_BIND_RENDER_TARGET;
        desc.CPUAccessFlags = D3D11_CPU_ACCESS_NONE;
        desc.MiscFlags |= D3D11_RESOURCE_MISC_GENERATE_MIPS;
    }

    // Create the texture.
    ID3D11Texture1D* dxTexture = nullptr;
    HRESULT hr;
    if (texture->GetData())
    {
        unsigned int const numSubresources = texture->GetNumSubresources();
        eastl::vector<D3D11_SUBRESOURCE_DATA> data(numSubresources);
        for (unsigned int index = 0; index < numSubresources; ++index)
        {
            auto sr = texture->GetSubresource(index);
            data[index].pSysMem = sr.data;
            data[index].SysMemPitch = 0;
            data[index].SysMemSlicePitch = 0;
        }
        hr = device->CreateTexture1D(&desc, &data[0], &dxTexture);
    }
    else
    {
        hr = device->CreateTexture1D(&desc, nullptr, &dxTexture);
    }
	if (FAILED(hr))
	{
		LogError("Failed to create texture, hr = " + GetErrorMessage(hr));
	}
    mDXObject = dxTexture;

    // Create views of the texture.
    CreateSRView(device, desc);
    if (usage == Resource::SHADER_OUTPUT)
    {
        CreateUAView(device, desc);
    }

    // Create a staging texture if requested.
    if (texture->GetCopyType() != Resource::COPY_NONE)
    {
        CreateStaging(device, desc);
    }

    // Generate mipmaps if requested.
    if (texture->WantAutogenerateMipmaps() && mSRView)
    {
        ID3D11DeviceContext* context;
        device->GetImmediateContext(&context);
        context->GenerateMips(mSRView);
        context->Release();
    }
}

eastl::shared_ptr<GraphicObject> DX11Texture1::Create(void* device, GraphicObject const* object)
{
    if (object->GetType() == GE_TEXTURE1)
    {
        return eastl::make_shared<DX11Texture1>(
            reinterpret_cast<ID3D11Device*>(device),
            static_cast<Texture1 const*>(object));
    }

    LogError("Invalid object type.");
    return nullptr;
}

void DX11Texture1::CreateStaging(ID3D11Device* device, D3D11_TEXTURE1D_DESC const& tx)
{
    D3D11_TEXTURE1D_DESC desc;
    desc.Width = tx.Width;
    desc.MipLevels = tx.MipLevels;
    desc.ArraySize = tx.ArraySize;
    desc.Format = tx.Format;
    desc.Usage = D3D11_USAGE_STAGING;
    desc.BindFlags = D3D11_BIND_NONE;
    desc.CPUAccessFlags = msStagingAccess[GetTexture()->GetCopyType()];
    desc.MiscFlags = D3D11_RESOURCE_MISC_NONE;

    HRESULT hr = device->CreateTexture1D(&desc, nullptr,
        reinterpret_cast<ID3D11Texture1D**>(&mStaging));
	if (FAILED(hr))
	{
		LogError("Failed to create staging texture, hr = " + GetErrorMessage(hr));
	}
}

void DX11Texture1::CreateSRView(ID3D11Device* device, D3D11_TEXTURE1D_DESC const& tx)
{
    D3D11_SHADER_RESOURCE_VIEW_DESC desc;
    desc.Format = tx.Format;
    desc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE1D;
    desc.Texture1D.MostDetailedMip = 0;
    desc.Texture1D.MipLevels = tx.MipLevels;
    HRESULT hr = device->CreateShaderResourceView(GetDXTexture(), &desc, &mSRView);
	if (FAILED(hr))
	{
		LogError("Failed to create shader resource view, hr = " + GetErrorMessage(hr));
	}
}

void DX11Texture1::CreateUAView(ID3D11Device* device, D3D11_TEXTURE1D_DESC const& tx)
{
    D3D11_UNORDERED_ACCESS_VIEW_DESC desc;
    desc.Format = tx.Format;
    desc.ViewDimension = D3D11_UAV_DIMENSION_TEXTURE1D;
    desc.Texture1D.MipSlice = 0;
    HRESULT hr = device->CreateUnorderedAccessView(GetDXTexture(), &desc, &mUAView);
	if (FAILED(hr))
	{
		LogError("Failed to create unordered access view, hr = " + GetErrorMessage(hr));
	}
}
