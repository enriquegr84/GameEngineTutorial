// David Eberly, Geometric Tools, Redmond WA 98052
// Copyright (c) 1998-2017
// Distributed under the Boost Software License, Version 1.0.
// http://www.boost.org/LICENSE_1_0.txt
// http://www.geometrictools.com/License/Boost/LICENSE_1_0.txt
// File Version: 3.0.0 (2016/06/19)

uniform sampler2DArray baseSampler;

layout(location = 0) in vec3 vertexTCoord;
layout(location = 0) out vec4 pixelColor;

void main()
{
	// Sample 2D texture array.
    pixelColor = texture(baseSampler, vertexTCoord);
}