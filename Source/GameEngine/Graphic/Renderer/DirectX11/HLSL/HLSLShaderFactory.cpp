// David Eberly, Geometric Tools, Redmond WA 98052
// Copyright (c) 1998-2017
// Distributed under the Boost Software License, Version 1.0.
// http://www.boost.org/LICENSE_1_0.txt
// http://www.geometrictools.com/License/Boost/LICENSE_1_0.txt
// File Version: 3.0.1 (2016/09/12)

#include "Core/Logger/Logger.h"
#include "Core/Utility/StringUtil.h"

#include "HLSLShaderFactory.h"

#include <string>

HLSLShader HLSLShaderFactory::CreateFromFile(eastl::string const& name, 
	eastl::string const& filepath, eastl::string const& entry,
	eastl::string const& target, ProgramDefines const& defines, 
	unsigned int compileFlags)
{
	eastl::string type = target.substr(0, 3);
	if (type == "vs_" || type == "gs_" || type == "ps_" || type == "cs_")
	{
		ID3DBlob* compiledCode = CompileShader(
			filepath, entry, target, compileFlags, defines);
		if (!compiledCode)
		{
			// Errors are recorded to a logfile by CompileShader.
			return HLSLShader();
		}

		HLSLShader shader;
		if (!ReflectShader(name, entry, target, compiledCode, shader))
		{
			// Errors are recorded to a logfile by ReflectShader.
			shader = HLSLShader();
		}
		compiledCode->Release();
		return shader;
	}
	else
	{
		LogError("Invalid target " + target);
		return HLSLShader();
	}
}

HLSLShader HLSLShaderFactory::CreateFromString(eastl::string const& name,
	eastl::string const& source, eastl::string const& entry,
	eastl::string const& target, ProgramDefines const& defines,
	unsigned int compileFlags)
{
	eastl::string type = target.substr(0, 3);
	if (type == "vs_" || type == "gs_" || type == "ps_" || type == "cs_")
	{
		ID3DBlob* compiledCode = CompileShader(
			name, source, entry, target, compileFlags, defines);
		if (!compiledCode)
		{
			// Errors are recorded to a logfile by CompileShader.
			return HLSLShader();
		}

		HLSLShader shader;
		if (!ReflectShader(name, entry, target, compiledCode, shader))
		{
			// Errors are recorded to a logfile by ReflectShader.
			shader = HLSLShader();
		}
		compiledCode->Release();
		return shader;
	}
	else
	{
		LogError("Invalid target " + target);
		return HLSLShader();
	}
}

HLSLShader HLSLShaderFactory::CreateFromByteCode(eastl::string const& name,
	eastl::string const& entry, eastl::string const& target,
	size_t numBytes, unsigned char const* bytecode)
{
	ID3DBlob* compiledCode = nullptr;
	HRESULT hr = D3DCreateBlob(numBytes, &compiledCode);
	if (FAILED(hr))
	{
		LogError("CreateFromByteCode failed to create blob.");
		return HLSLShader();
	}

	std::memcpy(compiledCode->GetBufferPointer(), bytecode, numBytes);

	HLSLShader shader;
	if (!ReflectShader(name, entry, target, compiledCode, shader))
	{
		// Errors are recorded to a logfile by ReflectShader.
		shader = HLSLShader();
	}
	compiledCode->Release();
	return shader;
}

ID3DBlob* HLSLShaderFactory::CompileShader(eastl::string const& filepath,
	eastl::string const& entry, eastl::string const& target, 
	unsigned int compileFlags, ProgramDefines const& defines)
{
	ID3DInclude* include = D3D_COMPILE_STANDARD_FILE_INCLUDE;
	ID3DBlob* compiledCode = nullptr;
	ID3DBlob* errors = nullptr;

	// Repackage the 'defines' for D3DCompile.  Prepend a definition based
	// on the engine's current matrix-vector multiplication convention.
	auto const& definitions = defines.Get();
	eastl::vector<D3D_SHADER_MACRO> localDefinitions(definitions.size() + 2);
	eastl::string umvName = "GE_USE_MAT_VEC";
	eastl::string umvValue;
#if defined(GE_USE_MAT_VEC)
	umvValue = "1";
#else
	umvValue = "0";
#endif
	localDefinitions.front().Name = umvName.c_str();
	localDefinitions.front().Definition = umvValue.c_str();
	for (size_t i = 0; i < definitions.size(); ++i)
	{
		localDefinitions[i + 1].Name = definitions[i].first.c_str();
		localDefinitions[i + 1].Definition = definitions[i].second.c_str();
	}
	localDefinitions.back().Name = nullptr;
	localDefinitions.back().Definition = nullptr;

	if (!(compileFlags & D3DCOMPILE_PACK_MATRIX_ROW_MAJOR)
		&& !(compileFlags & D3DCOMPILE_PACK_MATRIX_COLUMN_MAJOR))
	{
		// The matrix storage was not explicitly set.  Use the storage
		// consistent with the CPU storage.
#if defined(GE_USE_ROW_MAJOR)
		compileFlags |= D3DCOMPILE_PACK_MATRIX_ROW_MAJOR;
#else
		compileFlags |= D3DCOMPILE_PACK_MATRIX_COLUMN_MAJOR;
#endif
	}

	HRESULT hr = D3DCompileFromFile(ToWideString(filepath.c_str()).c_str(), 
		localDefinitions.data(), include, entry.c_str(), target.c_str(), compileFlags, 
		0, &compiledCode, &errors);
	if (SUCCEEDED(hr))
	{
		if (errors)
		{
			// The message is a 'warning' from the HLSL compiler.
			char const* message =
				static_cast<char const*>(errors->GetBufferPointer());
			LogError("D3DCompile warning: " + eastl::string(message));
		}
	}
	else  // FAILED(hr)
	{
		if (errors)
		{
			// The message is an 'error' from the HLSL compiler.
			char const* message =
				static_cast<char const*>(errors->GetBufferPointer());
			LogError("D3DCompile error: " + eastl::string(message));
		}
		else
		{
			// Unknown reason for error.
			LogError("D3DCompile error, hr = " + GetErrorMessage(hr));
		}
	}

	if (errors)
	{
		errors->Release();
	}

	return compiledCode;
}

ID3DBlob* HLSLShaderFactory::CompileShader(eastl::string const& name,
	eastl::string const& source, eastl::string const& entry,
	eastl::string const& target, unsigned int compileFlags,
	ProgramDefines const& defines)
{
	ID3DInclude* include = D3D_COMPILE_STANDARD_FILE_INCLUDE;
	ID3DBlob* compiledCode = nullptr;
	ID3DBlob* errors = nullptr;

	// Repackage the 'defines' for D3DCompile.  Prepend a definition based
	// on the engine's current matrix-vector multiplication convention.
	auto const& definitions = defines.Get();
	eastl::vector<D3D_SHADER_MACRO> localDefinitions(definitions.size() + 2);
	eastl::string umvName = "GE_USE_MAT_VEC";
	eastl::string umvValue;
#if defined(GE_USE_MAT_VEC)
	umvValue = "1";
#else
	umvValue = "0";
#endif
	localDefinitions.front().Name = umvName.c_str();
	localDefinitions.front().Definition = umvValue.c_str();
	for (size_t i = 0; i < definitions.size(); ++i)
	{
		localDefinitions[i + 1].Name = definitions[i].first.c_str();
		localDefinitions[i + 1].Definition = definitions[i].second.c_str();
	}
	localDefinitions.back().Name = nullptr;
	localDefinitions.back().Definition = nullptr;

	if (!(compileFlags & D3DCOMPILE_PACK_MATRIX_ROW_MAJOR)
		&& !(compileFlags & D3DCOMPILE_PACK_MATRIX_COLUMN_MAJOR))
	{
		// The matrix storage was not explicitly set.  Use the storage
		// consistent with the CPU storage.
#if defined(GE_USE_ROW_MAJOR)
		compileFlags |= D3DCOMPILE_PACK_MATRIX_ROW_MAJOR;
#else
		compileFlags |= D3DCOMPILE_PACK_MATRIX_COLUMN_MAJOR;
#endif
	}

	HRESULT hr = D3DCompile(source.c_str(), source.length(), name.c_str(),
		localDefinitions.data(), include, entry.c_str(), target.c_str(),
		compileFlags, 0, &compiledCode, &errors);

	if (SUCCEEDED(hr))
	{
		if (errors)
		{
			// The message is a 'warning' from the HLSL compiler.
			char const* message =
				static_cast<char const*>(errors->GetBufferPointer());
			LogError("D3DCompile warning: " + eastl::string(message));
		}
	}
	else  // FAILED(hr)
	{
		if (errors)
		{
			// The message is an 'error' from the HLSL compiler.
			char const* message =
				static_cast<char const*>(errors->GetBufferPointer());
			LogError("D3DCompile error: " + eastl::string(message));
		}
		else
		{
			// Unknown reason for error.
			LogError("D3DCompile error, hr = " + GetErrorMessage(hr));
		}
	}

	if (errors)
	{
		errors->Release();
	}

	return compiledCode;
}

bool HLSLShaderFactory::ReflectShader(eastl::string const& name,
	eastl::string const& entry, eastl::string const& target,
    ID3DBlob* compiledCode, HLSLShader& shader)
{
	void const* buffer = compiledCode->GetBufferPointer();
    size_t numBytes = compiledCode->GetBufferSize();
    ID3DShaderReflection* reflector = nullptr;
    HRESULT hr = D3DReflect(buffer, numBytes, IID_ID3DShaderReflection, (void**)&reflector);
    if (FAILED(hr))
    {
        LogError("D3DReflect error, hr = " + GetErrorMessage(hr));
        return false;
    }

	bool success =
		GetDescription(reflector, shader) &&
        GetInputs(reflector, shader) &&
        GetOutputs(reflector, shader) &&
        GetCBuffers(reflector, shader) &&
        GetBoundResources(reflector, shader);

    if (success)
    {
        shader.SetName(eastl::string(name));
        shader.SetEntry(eastl::string(entry));
        shader.SetTarget(eastl::string(target));
        shader.SetCompiledCode(numBytes, buffer);

        if (shader.GetDescription().shaderType == D3D11_SHVER_COMPUTE_SHADER)
        {
            // The return value of GetThreadGroupSize is numX*numY*numZ, so
            // it is safe to ignore.
            UINT numX, numY, numZ;
            reflector->GetThreadGroupSize(&numX, &numY, &numZ);
            shader.SetNumThreads(numX, numY, numZ);
        }
    }

	reflector->Release();
    return success;
}

bool HLSLShaderFactory::GetDescription(ID3DShaderReflection* reflector, HLSLShader& shader)
{
    D3D_SHADER_DESC desc;
    HRESULT hr = reflector->GetDesc(&desc);
    if (FAILED(hr))
    {
        LogError("reflector->GetDesc error, hr = " + GetErrorMessage(hr));
        return false;
    }
    shader.SetDescription(desc);
    return true;
}

bool HLSLShaderFactory::GetInputs(ID3DShaderReflection* reflector, HLSLShader& shader)
{
    UINT const numInputs = shader.GetDescription().numInputParameters;
    for (UINT i = 0; i < numInputs; ++i)
    {
        D3D_SIGNATURE_PARAMETER_DESC spDesc;
        HRESULT hr = reflector->GetInputParameterDesc(i, &spDesc);
        if (FAILED(hr))
        {
            LogError("reflector->GetInputParameterDesc error, hr = " + GetErrorMessage(hr));
            return false;
        }
        shader.InsertInput(HLSLParameter(spDesc));
    }
    return true;
}

bool HLSLShaderFactory::GetOutputs(ID3DShaderReflection* reflector, HLSLShader& shader)
{
    UINT const numOutputs = shader.GetDescription().numOutputParameters;
    for (UINT i = 0; i < numOutputs; ++i)
    {
        D3D_SIGNATURE_PARAMETER_DESC spDesc;
        HRESULT hr = reflector->GetOutputParameterDesc(i, &spDesc);
        if (FAILED(hr))
        {
            LogError("reflector->GetOutputParameterDesc error, hr = " + GetErrorMessage(hr));
            return false;
        }
        shader.InsertOutput(HLSLParameter(spDesc));
    }
    return true;
}

bool HLSLShaderFactory::GetCBuffers(ID3DShaderReflection* reflector, HLSLShader& shader)
{
    UINT const numCBuffers = shader.GetDescription().numConstantBuffers;
    for (UINT i = 0; i < numCBuffers; ++i)
    {
        ID3DShaderReflectionConstantBuffer* cbuffer = reflector->GetConstantBufferByIndex(i);

        D3D_SHADER_BUFFER_DESC cbDesc;
        HRESULT hr = cbuffer->GetDesc(&cbDesc);
        if (FAILED(hr))
        {
            LogError("reflector->GetConstantBufferByIndex error, hr = " + GetErrorMessage(hr));
            return false;
        }

        D3D_SHADER_INPUT_BIND_DESC resDesc;
        hr = reflector->GetResourceBindingDescByName(cbDesc.Name, &resDesc);
        if (FAILED(hr))
        {
            LogError("reflector->GetResourceBindingDescByName error, hr = " + GetErrorMessage(hr));
            return false;
        }

        if (cbDesc.Type == D3D_CT_CBUFFER)
        {
			eastl::vector<HLSLBaseBuffer::Member> members;
            if (!GetVariables(cbuffer, cbDesc.Variables, members))
            {
                return false;
            }

            if (resDesc.BindCount == 1)
            {
                shader.Insert(HLSLConstantBuffer(resDesc, cbDesc.Size, members));
            }
            else
            {
                for (UINT j = 0; j < resDesc.BindCount; ++j)
                {
                    shader.Insert(HLSLConstantBuffer(resDesc, j, cbDesc.Size, members));
                }
            }
        }
        else if (cbDesc.Type == D3D_CT_TBUFFER)
        {
			eastl::vector<HLSLBaseBuffer::Member> members;
            if (!GetVariables(cbuffer, cbDesc.Variables, members))
            {
                return false;
            }

            if (resDesc.BindCount == 1)
            {
                shader.Insert(HLSLTextureBuffer(resDesc, cbDesc.Size, members));
            }
            else
            {
                for (UINT j = 0; j < resDesc.BindCount; ++j)
                {
                    shader.Insert(HLSLTextureBuffer(resDesc, j,
                        cbDesc.Size, members));
                }
            }
        }
        else if (cbDesc.Type == D3D_CT_RESOURCE_BIND_INFO)
        {
			eastl::vector<HLSLBaseBuffer::Member> members;
            if (!GetVariables(cbuffer, cbDesc.Variables, members))
            {
                return false;
            }

            if (resDesc.BindCount == 1)
            {
                shader.Insert(HLSLResourceBindInfo(resDesc, cbDesc.Size, members));
            }
            else
            {
                for (UINT j = 0; j < resDesc.BindCount; ++j)
                {
                    shader.Insert(HLSLResourceBindInfo(resDesc, j, cbDesc.Size, members));
                }
            }
        }
        else  // cbDesc.Type == D3D_CT_INTERFACE_POINTERS
        {
            LogError("Interface pointers are not yet supported in GameEngine.");
            return false;
        }
    }
    return true;
}

bool HLSLShaderFactory::GetBoundResources(ID3DShaderReflection* reflector, HLSLShader& shader)
{
    // TODO: It appears that D3DCompile never produces a resource with a bind
    // count larger than 1.  For example, "Texture2D texture[2];" comes
    // through with names "texture[0]" and "texture[1]", each having a bind
    // count of 1 (with consecutive bind points).  So it appears that passing
    // a bind count in D3D interfaces that is larger than 1 is something a
    // programmer can do only if he manually combines the texture resources
    // into his own data structure.  If the statement about D3DCompiler is
    // true, we do not need the loops with upper bound resDesc.BindCount.

    UINT const numResources = shader.GetDescription().numBoundResources;
    for (UINT i = 0; i < numResources; ++i)
    {
        D3D_SHADER_INPUT_BIND_DESC resDesc;
        HRESULT hr = reflector->GetResourceBindingDesc(i, &resDesc);
        if (FAILED(hr))
        {
            LogError("reflector->GetResourceBindingDesc error, hr = " + GetErrorMessage(hr));
            return false;
        }

        if (resDesc.Type == D3D_SIT_CBUFFER     // cbuffer
        ||  resDesc.Type == D3D_SIT_TBUFFER)    // tbuffer
        {
            // These were processed in the previous loop.
        }
        else if (
            resDesc.Type == D3D_SIT_TEXTURE        // Texture*
        ||  resDesc.Type == D3D_SIT_UAV_RWTYPED)   // RWTexture*
        {
            if (IsTextureArray(resDesc.Dimension))
            {
                if (resDesc.BindCount == 1)
                {
                    shader.Insert(HLSLTextureArray(resDesc));
                }
                else
                {
                    for (UINT j = 0; j < resDesc.BindCount; ++j)
                    {
                        shader.Insert(HLSLTextureArray(resDesc, j));
                    }
                }
            }
            else
            {
                if (resDesc.BindCount == 1)
                {
                    shader.Insert(HLSLTexture(resDesc));
                }
                else
                {
                    for (UINT j = 0; j < resDesc.BindCount; ++j)
                    {
                        shader.Insert(HLSLTexture(resDesc, j));
                    }
                }
            }
        }
        else if (resDesc.Type == D3D_SIT_SAMPLER)   // SamplerState
        {
            if (resDesc.BindCount == 1)
            {
                shader.Insert(HLSLSamplerState(resDesc));
            }
            else
            {
                for (UINT j = 0; j < resDesc.BindCount; ++j)
                {
                    shader.Insert(HLSLSamplerState(resDesc, j));
                }
            }
        }
        else if (
            resDesc.Type == D3D_SIT_BYTEADDRESS         // ByteAddressBuffer
        ||  resDesc.Type == D3D_SIT_UAV_RWBYTEADDRESS)  // RWByteAddressBuffer
        {
            if (resDesc.BindCount == 1)
            {
                shader.Insert(HLSLByteAddressBuffer(resDesc));
            }
            else
            {
                for (UINT j = 0; j < resDesc.BindCount; ++j)
                {
                    shader.Insert(HLSLByteAddressBuffer(resDesc, j));
                }
            }
        }
        else
        {
            // D3D_SIT_STRUCTURED:  StructuredBuffer
            // D3D_SIT_UAV_RWSTRUCTURED:  RWStructuredBuffer
            // D3D_SIT_UAV_APPEND_STRUCTURED:  AppendStructuredBuffer
            // D3D_SIT_UAV_CONSUME_STRUCTURED:  ConsumeStructuredBuffer
            // D3D_SIT_UAV_RWSTRUCTURED_WITH_COUNTER:  RWStructuredBuffer

            if (resDesc.BindCount == 1)
            {
                shader.Insert(HLSLStructuredBuffer(resDesc));
            }
            else
            {
                for (UINT j = 0; j < resDesc.BindCount; ++j)
                {
                    shader.Insert(HLSLStructuredBuffer(resDesc, j));
                }
            }
        }
    }

    return true;
}

bool HLSLShaderFactory::GetVariables(ID3DShaderReflectionConstantBuffer* cbuffer,
    unsigned int numVariables, eastl::vector<HLSLBaseBuffer::Member>& members)
{
    for (UINT i = 0; i < numVariables; ++i)
    {
        ID3DShaderReflectionVariable* var = cbuffer->GetVariableByIndex(i);
        ID3DShaderReflectionType* varType = var->GetType();

        D3D_SHADER_VARIABLE_DESC varDesc;
        HRESULT hr = var->GetDesc(&varDesc);
        if (FAILED(hr))
        {
            LogError("var->GetDesc error, hr = " + GetErrorMessage(hr));
            return false;
        }

        D3D_SHADER_TYPE_DESC varTypeDesc;
        hr = varType->GetDesc(&varTypeDesc);
        if (FAILED(hr))
        {
            LogError("varType->GetDesc error, hr = " + GetErrorMessage(hr));
            return false;
        }

        // Get the top-level information about the shader variable.
        HLSLShaderVariable svar;
        svar.SetDescription(varDesc);

        // Get the type of the variable.  If this is a struct type, the
        // call recurses to build the type tree implied by the struct.
        HLSLShaderType stype;
        stype.SetName(svar.GetName());
        stype.SetDescription(varTypeDesc);
        if (!GetTypes(varType, varTypeDesc.Members, stype))
        {
            return false;
        }

        members.push_back(eastl::make_pair(svar, stype));
    }
    return true;
}

bool HLSLShaderFactory::GetTypes(ID3DShaderReflectionType* rtype,
    unsigned int numMembers, HLSLShaderType& stype)
{
    for (UINT i = 0; i < numMembers; ++i)
    {
        ID3DShaderReflectionType* memType = rtype->GetMemberTypeByIndex(i);
        char const* memTypeName = rtype->GetMemberTypeName(i);
        eastl::string memName(memTypeName ? memTypeName : "");
        D3D_SHADER_TYPE_DESC memTypeDesc;
        HRESULT hr = memType->GetDesc(&memTypeDesc);
        if (FAILED(hr))
        {
            LogError("memType->GetDesc error, hr = " + GetErrorMessage(hr));
            return false;
        }
        HLSLShaderType& child = stype.GetChild(i);
        child.SetName(memName);
        child.SetDescription(memTypeDesc);
        GetTypes(memType, memTypeDesc.Members, child);
    }
    return true;
}

bool HLSLShaderFactory::IsTextureArray(D3D_SRV_DIMENSION dim)
{
    return dim == D3D_SRV_DIMENSION_TEXTURE1DARRAY
        || dim == D3D_SRV_DIMENSION_TEXTURE2DARRAY
        || dim == D3D_SRV_DIMENSION_TEXTURE2DMSARRAY
        || dim == D3D_SRV_DIMENSION_TEXTURECUBE
        || dim == D3D_SRV_DIMENSION_TEXTURECUBEARRAY;
}

