// David Eberly, Geometric Tools, Redmond WA 98052
// Copyright (c) 1998-2017
// Distributed under the Boost Software License, Version 1.0.
// http://www.boost.org/LICENSE_1_0.txt
// http://www.geometrictools.com/License/Boost/LICENSE_1_0.txt
// File Version: 3.0.1 (2016/09/12)

#ifndef HLSLSHADERFACTORY_H
#define HLSLSHADERFACTORY_H

#include "Graphic/Shader/ProgramDefines.h"

#include "HLSLShader.h"

class GRAPHIC_ITEM HLSLShaderFactory
{
public:
	// Create a shader from an HLSL program in a file.
	static HLSLShader CreateFromFile(
		eastl::string const& name,
		eastl::string const& filepath,
		eastl::string const& entry,
		eastl::string const& target,
		ProgramDefines const& defines,
		unsigned int compileFlags);

	// Create a shader from an HLSL represented as a string.
	static HLSLShader CreateFromString(
		eastl::string const& name,
		eastl::string const& source,
		eastl::string const& entry,
		eastl::string const& target,
		ProgramDefines const& defines,
		unsigned int compileFlags);

	// Create a shader from an HLSL bytecode blob.
	static HLSLShader CreateFromByteCode(
		eastl::string const& name,
		eastl::string const& entry,
		eastl::string const& target,
		size_t numBytes,
		unsigned char const* bytecode);

private:
	// Wrapper for the D3DCompile call.
	static ID3DBlob* CompileShader(
		eastl::string const& filepath,
		eastl::string const& entry,
		eastl::string const& target,
		unsigned int compileFlags,
		ProgramDefines const& defines);

	// Wrapper for the D3DCompile call.
	static ID3DBlob* CompileShader(
		eastl::string const& name,
		eastl::string const& source,
		eastl::string const& entry,
		eastl::string const& target,
		unsigned int compileFlags,
		ProgramDefines const& defines);

	// Support for shader reflection to obtain information about the HLSL
	// program.
	static bool ReflectShader(
		eastl::string const& name,
		eastl::string const& entry,
		eastl::string const& target,
		ID3DBlob* compiledCode,
		HLSLShader& shader);

    static bool GetDescription(ID3DShaderReflection* reflector,  HLSLShader& shader);
	static bool GetInputs(ID3DShaderReflection* reflector,  HLSLShader& shader);
    static bool GetOutputs(ID3DShaderReflection* reflector, HLSLShader& shader);
    static bool GetCBuffers(ID3DShaderReflection* reflector, HLSLShader& shader);
    static bool GetBoundResources(ID3DShaderReflection* reflector, HLSLShader& shader);

    static bool GetVariables(ID3DShaderReflectionConstantBuffer* cbuffer,
        unsigned int numVariables, eastl::vector<HLSLBaseBuffer::Member>& members);

    static bool GetTypes(ID3DShaderReflectionType* rtype,
        unsigned int numMembers, HLSLShaderType& stype);

    static bool IsTextureArray(D3D_SRV_DIMENSION dim);
};

#endif