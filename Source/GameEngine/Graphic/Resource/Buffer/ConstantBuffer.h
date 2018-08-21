// David Eberly, Geometric Tools, Redmond WA 98052
// Copyright (c) 1998-2017
// Distributed under the Boost Software License, Version 1.0.
// http://www.boost.org/LICENSE_1_0.txt
// http://www.geometrictools.com/License/Boost/LICENSE_1_0.txt
// File Version: 3.0.1 (2016/08/29)

#ifndef CONSTANTBUFFER_H
#define CONSTANTBUFFER_H

#include "Buffer.h"
#include "Graphic/Resource/MemberLayout.h"

/*
	Constant Buffers. Its purpose is to organize one or more shader constants.
	A shader constant is input the CPU sends to a hader, which remains constant for
	all the primitives processed by a single draw call. cbuffers hold variables,
	"constant variables". They are constant from the perspective of the GPU while
	processing the primitives of a draw call yet variable from the perspective of
	the CPU from one draw call to the next
	A per object buffer indicates that the CPU should update the data within that 
	buffer for each object associated with the effect. In contrast, a per frame 
	buffer implies that the data within the buffer can be updated just once per 
	frame, allowing multiple objects to be rendered with the same shared shader
	constants. For performance considerations, constant buffers are grouped 
	according to their update frequency.
*/
class GRAPHIC_ITEM ConstantBuffer : public Buffer
{
public:
    // Construction.
    ConstantBuffer(size_t numBytes, bool allowDynamicUpdate);

    // Access to constant buffer members using the layout of a shader
    // program itself is allowed as long as you have attached the constant
    // buffer to a shader first.
    //   eastl::shared_ptr<VertexShader> vshader = <some shader>;
    //   eastl::shared_ptr<ConstantBuffer> cbuffer = <buffer for the shader>;
    //   vshader->Set("MyCBuffer", cbuffer);
    // Now you can use SetMember/GetMember calls successfully.  In these
    // calls, you are required to specify the correct type T for the member.
    // No checking is performed for the size of the input; i.e., too large a
    // 'value' will cause a memory overwrite within the buffer.  The code
    // does test to ensure that no overwrite occurs outside the buffer.

    inline void SetLayout(eastl::vector<MemberLayout> const& layout);
    inline eastl::vector<MemberLayout> const& GetLayout() const;

    // Test for existence of a member with the specified name.
    bool HasMember(eastl::string const& name) const;

    // Set or get a non-array member.
    template <typename T>
    bool SetMember(eastl::string const& name, T const& value);

    template <typename T>
    bool GetMember(eastl::string const& name, T& value) const;

    // Set or get an array member.
    template <typename T>
    bool SetMember(eastl::string const& name, unsigned int index,
        T const& value);

    template <typename T>
    bool GetMember(eastl::string const& name, unsigned int index, T& value) const;

protected:
    // Direct3D 11 requires the storage to be a multiple of 16 bytes.
    // Direct3D 12 requires the storage to be a multiple of 256 bytes.
    // This function rounds to the nearest multiple when allocating
    // buffer memory.
    static size_t GetRoundedNumBytes(size_t numBytes);

	eastl::vector<MemberLayout> mLayout;

public:
    // For use by the Shader class for storing reflection information.
    static int const mShaderDataLookup = 0;
};


inline void ConstantBuffer::SetLayout(eastl::vector<MemberLayout> const& layout)
{
    mLayout = layout;
}

inline eastl::vector<MemberLayout> const& ConstantBuffer::GetLayout() const
{
    return mLayout;
}

template <typename T>
bool ConstantBuffer::SetMember(eastl::string const& name, T const& value)
{
    auto iter = eastl::find_if(mLayout.begin(), mLayout.end(),
        [&name](MemberLayout const& item){ return name == item.name; });

    if (iter == mLayout.end())
    {
        LogError("Failed to find member name " + name + ".");
        return false;
    }

    if (iter->numElements > 0)
    {
        LogError("Member is an array, use SetMember(name,index,value).");
        return false;
    }

    if (iter->offset + sizeof(T) > mNumBytes)
    {
        LogError("Writing will access memory outside the buffer.");
        return false;
    }

    T* target = reinterpret_cast<T*>(mData + iter->offset);
    *target = value;
    return true;
}

template <typename T>
bool ConstantBuffer::GetMember(eastl::string const& name, T& value) const
{
    auto iter = eastl::find_if(mLayout.begin(), mLayout.end(),
        [&name](MemberLayout const& item){ return name == item.name; });

    if (iter == mLayout.end())
    {
        LogError("Failed to find member name " + name + ".");
        return false;
    }

    if (iter->numElements > 0)
    {
        LogError("Member is an array, use GetMember(name,index,value).");
        return false;
    }

    if (iter->offset + sizeof(T) > mNumBytes)
    {
        LogError("Reading will access memory outside the buffer.");
        return false;
    }

    T* target = reinterpret_cast<T*>(mData + iter->offset);
    value = *target;
    return true;
}

template <typename T>
bool ConstantBuffer::SetMember(eastl::string const& name, unsigned int index,
    T const& value)
{
    auto iter = eastl::find_if(mLayout.begin(), mLayout.end(),
        [&name](MemberLayout const& item){ return name == item.name; });

    if (iter == mLayout.end())
    {
        LogError("Failed to find member name " + name + ".");
        return false;
    }

    if (iter->numElements == 0)
    {
        LogError("Member is a singleton, use SetMember(name,value).");
        return false;
    }

    if (index >= iter->numElements)
    {
        LogError("Index is out of range for the member array.");
        return false;
    }

    if (iter->offset + (index + 1)*sizeof(T) > mNumBytes)
    {
        LogError("Writing will access memory outside the buffer.");
        return false;
    }

    T* target = reinterpret_cast<T*>(mData + iter->offset + index*sizeof(T));
    *target = value;
    return true;
}

template <typename T>
bool ConstantBuffer::GetMember(eastl::string const& name, unsigned int index, T& value) const
{
    auto iter = eastl::find_if(mLayout.begin(), mLayout.end(),
        [&name](MemberLayout const& item){ return name == item.name; });

    if (iter == mLayout.end())
    {
        LogError("Failed to find member name " + name + ".");
        return false;
    }

    if (iter->numElements == 0)
    {
        LogError("Member is a singleton, use GetMember(name,value).");
        return false;
    }

    if (index >= iter->numElements)
    {
        LogError("Index is out of range for the member array.");
        return false;
    }

    if (iter->offset + (index + 1)*sizeof(T) > mNumBytes)
    {
        LogError("Reading will access memory outside the buffer.");
        return false;
    }

    T* target = reinterpret_cast<T*>(mData + iter->offset + index*sizeof(T));
    value = *target;
    return true;
}


#endif