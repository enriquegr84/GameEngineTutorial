// David Eberly, Geometric Tools, Redmond WA 98052
// Copyright (c) 1998-2017
// Distributed under the Boost Software License, Version 1.0.
// http://www.boost.org/LICENSE_1_0.txt
// http://www.geometrictools.com/License/Boost/LICENSE_1_0.txt
// File Version: 3.0.0 (2016/06/19)

#ifndef MEMBERLAYOUT_H
#define MEMBERLAYOUT_H

#include "Graphic/GraphicStd.h"

#include <EASTL/vector.h>
#include <EASTL/string.h>

// Support for generation of lookup tables for constant buffers and texture
// buffers.  Given the name of a member of a buffer, get the offset into the
// buffer memory where the member lives.  The layout is specific to the
// shading language (HLSL, GLSL).
struct GRAPHIC_ITEM MemberLayout
{
    eastl::string name;
    unsigned int offset;
    unsigned int numElements;
};

typedef eastl::vector<MemberLayout> BufferLayout;

#endif
