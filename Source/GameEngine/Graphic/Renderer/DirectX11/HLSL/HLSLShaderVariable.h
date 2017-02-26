// David Eberly, Geometric Tools, Redmond WA 98052
// Copyright (c) 1998-2017
// Distributed under the Boost Software License, Version 1.0.
// http://www.boost.org/LICENSE_1_0.txt
// http://www.geometrictools.com/License/Boost/LICENSE_1_0.txt
// File Version: 3.0.1 (2016/09/12)

#ifndef HLSLSHADERVARIABLE_H
#define HLSLSHADERVARIABLE_H

#include "HLSLResource.h"

#include "EASTL/vector.h"

#include <fstream>

class GRAPHIC_ITEM HLSLShaderVariable
{
public:
    struct Description
    {
        eastl::string name;
        unsigned int offset;
        unsigned int numBytes;
        unsigned int flags;
        unsigned int textureStart;
        unsigned int textureNumSlots;
        unsigned int samplerStart;
        unsigned int samplerNumSlots;
		eastl::vector<unsigned char> defaultValue;
    };

    // Construction.  Shader variables are reported for constant buffers,
    // texture buffers, and structs defined in the shaders (resource
    // binding information).
    HLSLShaderVariable();

    // Deferred construction for shader reflection.  This function is
    // intended to be write-once.
    void SetDescription(D3D_SHADER_VARIABLE_DESC const& desc);

    // Member access.
	eastl::string const& GetName() const;
    unsigned int GetOffset() const;
    unsigned int GetNumBytes() const;
    unsigned int GetFlags() const;
    unsigned int GetTextureStart() const;
    unsigned int GetTextureNumSlots() const;
    unsigned int GetSamplerStart() const;
    unsigned int GetSamplerNumSlots() const;
	eastl::vector<unsigned char> const& GetDefaultValue() const;

    // Print to a text file for human readability.
    void Print(std::ofstream& output) const;

private:
    Description mDesc;
};

#endif