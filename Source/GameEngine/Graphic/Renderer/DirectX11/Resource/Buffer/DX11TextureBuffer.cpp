// David Eberly, Geometric Tools, Redmond WA 98052
// Copyright (c) 1998-2017
// Distributed under the Boost Software License, Version 1.0.
// http://www.boost.org/LICENSE_1_0.txt
// http://www.geometrictools.com/License/Boost/LICENSE_1_0.txt
// File Version: 3.0.0 (2016/06/19)

#include "Core/Logger/Logger.h"

#include "DX11TextureBuffer.h"

DX11TextureBuffer::~DX11TextureBuffer()
{
	FinalRelease(mSRView);
}

DX11TextureBuffer::DX11TextureBuffer(ID3D11Device* device, TextureBuffer const* tbuffer)
    :
    DX11Buffer(tbuffer),
    mSRView(nullptr)
{
    // Specify the buffer description.
    D3D11_BUFFER_DESC desc;
    desc.ByteWidth = tbuffer->GetNumBytes();
    desc.BindFlags = D3D11_BIND_SHADER_RESOURCE;
    desc.MiscFlags = D3D11_RESOURCE_MISC_NONE;
    desc.StructureByteStride = 0;

    Resource::Usage usage = tbuffer->GetUsage();
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
    else
    {
        desc.Usage = D3D11_USAGE_DEFAULT;
        desc.CPUAccessFlags = D3D11_CPU_ACCESS_NONE;
    }

    // Create the buffer.
    ID3D11Buffer* buffer = nullptr;
    HRESULT hr;
    if (tbuffer->GetData())
    {
        D3D11_SUBRESOURCE_DATA data;
        data.pSysMem = tbuffer->GetData();
        data.SysMemPitch = 0;
        data.SysMemSlicePitch = 0;
        hr = device->CreateBuffer(&desc, &data, &buffer);
    }
    else
    {
        hr = device->CreateBuffer(&desc, nullptr, &buffer);
    }
	if (FAILED(hr))
	{
		LogError("Failed to create texture buffer, hr = " + GetErrorMessage(hr));
	}
	mDXObject = buffer;

    // Create a SRV for the texture to be readable.
    CreateSRView(device);

    // Create a staging buffer if requested.
    if (tbuffer->GetCopyType() != Resource::COPY_NONE)
    {
        CreateStaging(device, desc);
    }
}

eastl::shared_ptr<GraphicObject> DX11TextureBuffer::Create(
	void* device, GraphicObject const* object)
{
    if (object->GetType() == GE_TEXTURE_BUFFER)
    {
        return eastl::make_shared<DX11TextureBuffer>(
            reinterpret_cast<ID3D11Device*>(device),
            static_cast<TextureBuffer const*>(object));
    }

    LogError("Invalid object type.");
    return nullptr;
}

void DX11TextureBuffer::CreateSRView(ID3D11Device* device)
{
    TextureBuffer* tbuffer = GetTextureBuffer();
    ID3D11Buffer* dxBuffer = GetDXBuffer();

    D3D11_SHADER_RESOURCE_VIEW_DESC desc;
    desc.Format = static_cast<DXGI_FORMAT>(tbuffer->GetFormat());
    desc.ViewDimension = D3D11_SRV_DIMENSION_BUFFER;
    desc.Buffer.FirstElement = 0;
    desc.Buffer.NumElements = tbuffer->GetNumElements();
    HRESULT hr = device->CreateShaderResourceView(dxBuffer, &desc, &mSRView);
	if (FAILED(hr))
	{
		LogError("Failed to create shader resource view, hr = " + GetErrorMessage(hr));
	}
}
