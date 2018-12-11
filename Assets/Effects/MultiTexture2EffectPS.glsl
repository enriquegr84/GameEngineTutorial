// David Eberly, Geometric Tools, Redmond WA 98052
// Copyright (c) 1998-2017
// Distributed under the Boost Software License, Version 1.0.
// http://www.boost.org/LICENSE_1_0.txt
// http://www.geometrictools.com/License/Boost/LICENSE_1_0.txt
// File Version: 3.0.0 (2016/06/19)

uniform sampler2D baseSampler1, baseSampler2;

layout(location = 0) in vec2 vertexTCoord;
layout(location = 0) out vec4 pixelColor;

void main()
{
    pixelColor0 = vec4(0, 0, 0, 0);

    vec4 tcd;

    // Sample first 2D texture array.
    tcd.xyz = vec3(vertexTCoord, 0);
    pixelColor0 += texture(baseSampler1, tcd.xyz);
    tcd.xyz = vec3(vertexTCoord, 1);
    pixelColor0 += texture(baseSampler1, tcd.xyz);

    // Sample second 2D texture array.
    tcd.xyz = vec3(vertexTCoord, 0);
    pixelColor0 += texture(baseSampler2, tcd.xyz);
    tcd.xyz = vec3(vertexTCoord, 1);
    pixelColor0 += texture(baseSampler2, tcd.xyz);

    pixelColor0 *= 0.25f;
}
