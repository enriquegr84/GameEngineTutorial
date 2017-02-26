// David Eberly, Geometric Tools, Redmond WA 98052
// Copyright (c) 1998-2017
// Distributed under the Boost Software License, Version 1.0.
// http://www.boost.org/LICENSE_1_0.txt
// http://www.geometrictools.com/License/Boost/LICENSE_1_0.txt
// File Version: 3.0.0 (2016/06/19)

#ifndef GEOMETRYSHADER_H
#define GEOMETRYSHADER_H

#include "Shader.h"

class GRAPHIC_ITEM GeometryShader : public Shader
{
public:
    // Construction.
#if defined(_OPENGL_)
    GeometryShader(GLSLReflection const& reflector);
#else
    GeometryShader(HLSLShader const& program);
#endif
};

#endif
