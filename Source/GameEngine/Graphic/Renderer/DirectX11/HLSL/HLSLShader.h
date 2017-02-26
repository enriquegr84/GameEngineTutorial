// David Eberly, Geometric Tools, Redmond WA 98052
// Copyright (c) 1998-2017
// Distributed under the Boost Software License, Version 1.0.
// http://www.boost.org/LICENSE_1_0.txt
// http://www.geometrictools.com/License/Boost/LICENSE_1_0.txt
// File Version: 3.0.1 (2016/09/12)

#ifndef HLSLSHADER_H
#define HLSLSHADER_H

#include "Graphic/GraphicStd.h"

#include "HLSLByteAddressBuffer.h"
#include "HLSLConstantBuffer.h"
#include "HLSLParameter.h"
#include "HLSLResourceBindInfo.h"
#include "HLSLSamplerState.h"
#include "HLSLStructuredBuffer.h"
#include "HLSLTexture.h"
#include "HLSLTextureArray.h"
#include "HLSLTextureBuffer.h"
#include <fstream>

class GRAPHIC_ITEM HLSLShader
{
public:
    struct Description
    {
        struct InstructionCount
        {
            unsigned int numInstructions;
            unsigned int numTemporaryRegisters;
            unsigned int numTemporaryArrays;
            unsigned int numDefines;
            unsigned int numDeclarations;
            unsigned int numTextureNormal;
            unsigned int numTextureLoad;
            unsigned int numTextureComparison;
            unsigned int numTextureBias;
            unsigned int numTextureGradient;
            unsigned int numFloatArithmetic;
            unsigned int numSIntArithmetic;
            unsigned int numUIntArithmetic;
            unsigned int numStaticFlowControl;
            unsigned int numDynamicFlowControl;
            unsigned int numMacro;
            unsigned int numArray;
        };


        struct GSParameters
        {
            unsigned int numCutInstructions;
            unsigned int numEmitInstructions;
            D3D_PRIMITIVE inputPrimitive;
            D3D_PRIMITIVE_TOPOLOGY outputTopology;
            unsigned int maxOutputVertices;
        };

        struct TSParameters
        {
            unsigned int numPatchConstants;
            unsigned int numGSInstances;
            unsigned int numControlPoints;
            D3D_PRIMITIVE inputPrimitive;
            D3D_TESSELLATOR_OUTPUT_PRIMITIVE outputPrimitive;
            D3D_TESSELLATOR_PARTITIONING partitioning;
            D3D_TESSELLATOR_DOMAIN domain;
        };

        struct CSParameters
        {
            unsigned int numBarrierInstructions;
            unsigned int numInterlockedInstructions;
            unsigned int numTextureStoreInstructions;
        };

        eastl::string creator;
        D3D_SHADER_VERSION_TYPE shaderType;
        unsigned int majorVersion;
        unsigned int minorVersion;
        unsigned int flags;
        unsigned int numConstantBuffers;
        unsigned int numBoundResources;
        unsigned int numInputParameters;
        unsigned int numOutputParameters;
        InstructionCount instructions;
        GSParameters gs;
        TSParameters ts;
        CSParameters cs;
    };

    // Construction.
    HLSLShader();

    // Test whether the shader was constructed properly.  The function tests
    // solely whether the name, entry, and target are nonempty strings and
    // that the compiled code array is nonempty; this is the common case when
    // HLSLShaderFactory is used to create the shader.
    bool IsValid() const;

    // Deferred construction for shader reflection.  These functions are
    // intended to be write-once.
    void SetDescription(D3D_SHADER_DESC const& desc);
    void SetName(eastl::string const& name);
    void SetEntry(eastl::string const& entry);
    void SetTarget(eastl::string const& target);
	void InsertInput(HLSLParameter const& parameter);
	void InsertOutput(HLSLParameter const& parameter);
	void Insert(HLSLConstantBuffer const& cbuffer);
	void Insert(HLSLTextureBuffer const& tbuffer);
	void Insert(HLSLStructuredBuffer const& sbuffer);
	void Insert(HLSLByteAddressBuffer const& rbuffer);
	void Insert(HLSLTexture const& texture);
	void Insert(HLSLTextureArray const& textureArray);
	void Insert(HLSLSamplerState const& samplerState);
	void Insert(HLSLResourceBindInfo const& rbinfo);
	void SetCompiledCode(size_t numBytes, void const* buffer);

    // Member access.
    Description const& GetDescription() const;
    eastl::string const& GetName() const;
    eastl::string const& GetEntry() const;
    eastl::string const& GetTarget() const;
    int GetShaderTypeIndex() const;
	eastl::vector<HLSLParameter> const& GetInputs() const;
	eastl::vector<HLSLParameter> const& GetOutputs() const;
	eastl::vector<HLSLConstantBuffer> const& GetCBuffers() const;
	eastl::vector<HLSLTextureBuffer> const& GetTBuffers() const;
	eastl::vector<HLSLStructuredBuffer> const& GetSBuffers() const;
	eastl::vector<HLSLByteAddressBuffer> const& GetRBuffers() const;
	eastl::vector<HLSLTexture> const& GetTextures() const;
	eastl::vector<HLSLTextureArray> const& GetTextureArrays() const;
	eastl::vector<HLSLSamplerState> const& GetSamplerStates() const;
	eastl::vector<HLSLResourceBindInfo> const& GetResourceBindInfos() const;
    eastl::vector<unsigned char> const& GetCompiledCode() const;

    // Compute shaders only.
    void SetNumThreads(unsigned int numXThreads, unsigned int numYThreads,
        unsigned int numZThreads);
    unsigned int GetNumXThreads() const;
    unsigned int GetNumYThreads() const;
    unsigned int GetNumZThreads() const;

    // Print to a text file for human readability.
    void Print(std::ofstream& output) const;

private:
    Description mDesc;
    eastl::string mName;
    eastl::string mEntry;
    eastl::string mTarget;
	eastl::vector<HLSLParameter> mInputs;
	eastl::vector<HLSLParameter> mOutputs;
	eastl::vector<HLSLConstantBuffer> mCBuffers;
	eastl::vector<HLSLTextureBuffer> mTBuffers;
	eastl::vector<HLSLStructuredBuffer> mSBuffers;
	eastl::vector<HLSLByteAddressBuffer> mRBuffers;
	eastl::vector<HLSLTexture> mTextures;
	eastl::vector<HLSLTextureArray> mTextureArrays;
	eastl::vector<HLSLSamplerState> mSamplerStates;
	eastl::vector<HLSLResourceBindInfo> mRBInfos;
	eastl::vector<unsigned char> mCompiledCode;
    unsigned int mNumXThreads;
    unsigned int mNumYThreads;
    unsigned int mNumZThreads;

    // Support for Print.
    static eastl::string const msShaderType[];
    static eastl::string const msCompileFlags[];
    static eastl::string const msPrimitive[];
    static eastl::string const msPrimitiveTopology[];
    static eastl::string const msOutputPrimitive[];
    static eastl::string const msPartitioning[];
    static eastl::string const msDomain[];
};

#endif