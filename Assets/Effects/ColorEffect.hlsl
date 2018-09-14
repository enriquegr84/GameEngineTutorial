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
    float3 modelPosition : POSITION;
    float4 color : COLOR0;
};

struct VS_OUTPUT 
{
    float4 position: SV_Position;
    float4 color : COLOR0;
};


/************* Vertex Shader *************/

VS_OUTPUT VSMain(VS_INPUT input)
{
    VS_OUTPUT output = (VS_OUTPUT)0;
#if GE_USE_MAT_VEC
	output.position = mul(pvwMatrix, float4(input.modelPosition, 1.0f));
#else
	output.position = mul(float4(input.modelPosition, 1.0f), pvwMatrix);
#endif
    output.color = input.color;
    
    return output;
}

/************* Pixel Shader *************/

float4 PSMain(VS_OUTPUT input) : SV_Target
{
    return input.color;
}