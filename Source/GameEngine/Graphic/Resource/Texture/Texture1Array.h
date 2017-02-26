// David Eberly, Geometric Tools, Redmond WA 98052
// Copyright (c) 1998-2017
// Distributed under the Boost Software License, Version 1.0.
// http://www.boost.org/LICENSE_1_0.txt
// http://www.geometrictools.com/License/Boost/LICENSE_1_0.txt
// File Version: 3.0.0 (2016/06/19)

#ifndef TEXTURE1ARRAY_H
#define TEXTURE1ARRAY_H

#include "TextureArray.h"

class GRAPHIC_ITEM Texture1Array : public TextureArray
{
public:
    // Construction.
    Texture1Array(unsigned int numItems, DFType format, unsigned int length,
        bool hasMipmaps = false, bool createStorage = true);

    // Texture dimensions.
    unsigned int GetLength() const;
};

#endif
