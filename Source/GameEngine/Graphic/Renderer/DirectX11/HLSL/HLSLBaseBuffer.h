// David Eberly, Geometric Tools, Redmond WA 98052
// Copyright (c) 1998-2017
// Distributed under the Boost Software License, Version 1.0.
// http://www.boost.org/LICENSE_1_0.txt
// http://www.geometrictools.com/License/Boost/LICENSE_1_0.txt
// File Version: 3.0.1 (2016/09/12)

#ifndef HLSLBASEBUFFER_H
#define HLSLBASEBUFFER_H

#include "Graphic/Resource/MemberLayout.h"

#include "HLSLResource.h"
#include "HLSLShaderType.h"
#include "HLSLShaderVariable.h"
#include <fstream>


class GRAPHIC_ITEM HLSLBaseBuffer : public HLSLResource
{
public:
    typedef eastl::pair<HLSLShaderVariable, HLSLShaderType> Member;

    // Construction and destruction.
    virtual ~HLSLBaseBuffer();

    HLSLBaseBuffer(D3D_SHADER_INPUT_BIND_DESC const& desc,
        unsigned int numBytes, eastl::vector<Member> const& members);

    HLSLBaseBuffer(D3D_SHADER_INPUT_BIND_DESC const& desc,
        unsigned int index, unsigned int numBytes,
		eastl::vector<Member> const& members);

    // Member access.
	eastl::vector<Member> const& GetMembers() const;

    // Print to a text file for human readability.
    virtual void Print(std::ofstream& output) const;

    // Generation of lookup tables for member layout.
    void GenerateLayout(eastl::vector<MemberLayout>& layout) const;

private:
    void GenerateLayout(HLSLShaderType const& type, unsigned int parentOffset,
		eastl::string const& parentName,
		eastl::vector<MemberLayout>& layout) const;

	eastl::vector<Member> mMembers;
};

#endif