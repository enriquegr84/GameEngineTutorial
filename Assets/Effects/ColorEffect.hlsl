// David Eberly, Geometric Tools, Redmond WA 98052
// Copyright (c) 1998-2017
// Distributed under the Boost Software License, Version 1.0.
// http://www.boost.org/LICENSE_1_0.txt
// http://www.geometrictools.com/License/Boost/LICENSE_1_0.txt
// File Version: 3.0.0 (2016/06/19)

cbuffer PVWMatrix
{
    float4x4 pvwMatrix; 
}

/************* Data Structures *************/

struct VS_INPUT
{
    float4 modelPosition: POSITION;
    float4 color : COLOR;
};

struct VS_OUTPUT 
{
    float4 position: SV_Position;
    float4 color : COLOR;
};


/************* Vertex Shader *************/

VS_OUTPUT VSMain(VS_INPUT input)
{
    VS_OUTPUT output = (VS_OUTPUT)0;
#if GE_USE_MAT_VEC
	output.position = mul(pvwMatrix, input.modelPosition);
#else
	output.position = mul(input.modelPosition, pvwMatrix);
#endif
    output.color = input.color;
    
    return output;
}

/************* Pixel Shader *************/

float4 PSMain(VS_OUTPUT input) : SV_Target
{
    return input.color;
}