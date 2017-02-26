// David Eberly, Geometric Tools, Redmond WA 98052
// Copyright (c) 1998-2017
// Distributed under the Boost Software License, Version 1.0.
// http://www.boost.org/LICENSE_1_0.txt
// http://www.geometrictools.com/License/Boost/LICENSE_1_0.txt
// File Version: 3.0.0 (2016/06/19)

#include "TextureBuffer.h"

TextureBuffer::TextureBuffer(DFType format, unsigned int numElements,
    bool allowDynamicUpdate)
    :
    Buffer(numElements, DataFormat::GetNumBytesPerStruct(format), true),
    mFormat(format)
{
    mType = GE_TEXTURE_BUFFER;
    mUsage = (allowDynamicUpdate ? DYNAMIC_UPDATE : IMMUTABLE);
    memset(mData, 0, mNumBytes);
}

DFType TextureBuffer::GetFormat() const
{
    return mFormat;
}

bool TextureBuffer::HasMember(eastl::string const& name) const
{
    auto iter = eastl::find_if(mLayout.begin(), mLayout.end(),
        [&name](MemberLayout const& item){ return name == item.name; });
    return iter != mLayout.end();
}
