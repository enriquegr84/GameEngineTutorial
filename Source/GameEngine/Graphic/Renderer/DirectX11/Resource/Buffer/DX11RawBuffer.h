// David Eberly, Geometric Tools, Redmond WA 98052
// Copyright (c) 1998-2017
// Distributed under the Boost Software License, Version 1.0.
// http://www.boost.org/LICENSE_1_0.txt
// http://www.geometrictools.com/License/Boost/LICENSE_1_0.txt
// File Version: 3.0.0 (2016/06/19)

#ifndef DX11RAWBUFFER_H
#define DX11RAWBUFFER_H

#include "Graphic/Resource/Buffer/RawBuffer.h"
#include "DX11Buffer.h"

class GRAPHIC_ITEM DX11RawBuffer : public DX11Buffer
{
public:
    // Construction and destruction.
    virtual ~DX11RawBuffer();
    DX11RawBuffer(ID3D11Device* device, RawBuffer const* rbuffer);
    static eastl::shared_ptr<GraphicObject> Create(void* device, GraphicObject const* object);

    // Member access.
    inline RawBuffer* GetRawBuffer() const;
    inline ID3D11ShaderResourceView* GetSRView() const;
    inline ID3D11UnorderedAccessView* GetUAView() const;

private:
    // Support for construction.
    void CreateSRView(ID3D11Device* device);
    void CreateUAView(ID3D11Device* device);

    ID3D11ShaderResourceView* mSRView;
    ID3D11UnorderedAccessView* mUAView;
};

inline RawBuffer* DX11RawBuffer::GetRawBuffer() const
{
    return static_cast<RawBuffer*>(mGObject);
}

inline ID3D11ShaderResourceView* DX11RawBuffer::GetSRView() const
{
    return mSRView;
}

inline ID3D11UnorderedAccessView* DX11RawBuffer::GetUAView() const
{
    return mUAView;
}

#endif