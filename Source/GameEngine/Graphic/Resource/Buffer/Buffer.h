// David Eberly, Geometric Tools, Redmond WA 98052
// Copyright (c) 1998-2017
// Distributed under the Boost Software License, Version 1.0.
// http://www.boost.org/LICENSE_1_0.txt
// http://www.geometrictools.com/License/Boost/LICENSE_1_0.txt
// File Version: 3.0.0 (2016/06/19)

#ifndef BUFFER_H
#define BUFFER_H

#include "Graphic/Resource/Resource.h"

class GRAPHIC_ITEM Buffer : public Resource
{
protected:
    // Abstract base class.
    Buffer(unsigned int numElements, size_t elementSize, bool createStorage = true);
};

typedef eastl::function<void(eastl::shared_ptr<Buffer> const&)> BufferUpdater;

#endif