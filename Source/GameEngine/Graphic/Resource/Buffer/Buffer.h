// David Eberly, Geometric Tools, Redmond WA 98052
// Copyright (c) 1998-2017
// Distributed under the Boost Software License, Version 1.0.
// http://www.boost.org/LICENSE_1_0.txt
// http://www.geometrictools.com/License/Boost/LICENSE_1_0.txt
// File Version: 3.0.0 (2016/06/19)

#ifndef BUFFER_H
#define BUFFER_H

#include "Graphic/Resource/Resource.h"

enum GRAPHIC_ITEM HardwareMapping
{
	//! Don't store on the hardware
	HM_NEVER = 0,

	//! Rarely changed, usually stored completely on the hardware
	HM_STATIC,

	//! Sometimes changed, driver optimized placement
	HM_DYNAMIC,

	//! Always changed, cache optimizing on the GPU
	HM_STREAM
};

enum GRAPHIC_ITEM BufferType
{
	//! Does not change anything
	BT_NONE = 0,
	//! Change the vertex mapping
	BT_VERTEX,
	//! Change the index mapping
	BT_INDEX,
	//! Change both vertex and index mapping to the same value
	BT_VERTEX_AND_INDEX
};

class GRAPHIC_ITEM Buffer : public Resource
{
protected:
    // Abstract base class.
    Buffer(unsigned int numElements, size_t elementSize, bool createStorage = true);
};

typedef eastl::function<void(eastl::shared_ptr<Buffer> const&)> BufferUpdater;

#endif