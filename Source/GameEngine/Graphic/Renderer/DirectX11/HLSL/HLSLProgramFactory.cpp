// David Eberly, Geometric Tools, Redmond WA 98052
// Copyright (c) 1998-2017
// Distributed under the Boost Software License, Version 1.0.
// http://www.boost.org/LICENSE_1_0.txt
// http://www.geometrictools.com/License/Boost/LICENSE_1_0.txt
// File Version: 3.0.1 (2016/09/12)

#include "Core/Logger/Logger.h"

#include "HLSLComputeProgram.h"
#include "HLSLProgramFactory.h"
#include "HLSLShaderFactory.h"
#include "HLSLVisualProgram.h"

eastl::string HLSLProgramFactory::defaultVersion = "5_0";
eastl::string HLSLProgramFactory::defaultVSEntry = "VSMain";
eastl::string HLSLProgramFactory::defaultPSEntry = "PSMain";
eastl::string HLSLProgramFactory::defaultGSEntry = "GSMain";
eastl::string HLSLProgramFactory::defaultCSEntry = "CSMain";
unsigned int HLSLProgramFactory::defaultFlags = (
	D3DCOMPILE_DEBUG |
    D3DCOMPILE_ENABLE_STRICTNESS |
    D3DCOMPILE_IEEE_STRICTNESS |
    D3DCOMPILE_OPTIMIZATION_LEVEL3);

HLSLProgramFactory::~HLSLProgramFactory()
{
}

HLSLProgramFactory::HLSLProgramFactory()
{
    version = defaultVersion;
    vsEntry = defaultVSEntry;
    psEntry = defaultPSEntry;
    gsEntry = defaultGSEntry;
    csEntry = defaultCSEntry;
    flags = defaultFlags;
}

int HLSLProgramFactory::GetAPI() const
{
    return PF_HLSL;
}

eastl::shared_ptr<VisualProgram> HLSLProgramFactory::CreateFromByteCode(
	eastl::vector<unsigned char> const& vsBytecode,
	eastl::vector<unsigned char> const& psBytecode,
	eastl::vector<unsigned char> const& gsBytecode)
{
    if (vsBytecode.size() == 0 || psBytecode.size() == 0)
    {
        LogError("A program must have a vertex shader and a pixel shader.");
        return nullptr;
    }

	eastl::shared_ptr<VertexShader> vshader;
	eastl::shared_ptr<PixelShader> pshader;
	eastl::shared_ptr<GeometryShader> gshader;

    HLSLShader hlslVShader = HLSLShaderFactory::CreateFromByteCode("vs",
        vsEntry, eastl::string("vs_") + version, vsBytecode.size(), vsBytecode.data());
    if (hlslVShader.IsValid())
    {
        vshader =
			eastl::static_pointer_cast<VertexShader>(eastl::make_shared<Shader>(hlslVShader));
    }
    else
    {
        return nullptr;
    }

    HLSLShader hlslPShader = HLSLShaderFactory::CreateFromByteCode("ps",
        psEntry, eastl::string("ps_") + version, psBytecode.size(), psBytecode.data());
    if (hlslPShader.IsValid())
    {
        pshader = eastl::static_pointer_cast<PixelShader>(
			eastl::make_shared<Shader>(hlslPShader));
    }
    else
    {
        return nullptr;
    }

    HLSLShader hlslGShader;
    if (gsBytecode.size() > 0)
    {
        hlslGShader = HLSLShaderFactory::CreateFromByteCode("gs",
            gsEntry, eastl::string("gs_") + version, gsBytecode.size(), gsBytecode.data());
        if (hlslGShader.IsValid())
        {
            gshader = eastl::static_pointer_cast<GeometryShader>(
				eastl::make_shared<Shader>(hlslGShader));
        }
        else
        {
            return nullptr;
        }
    }

	eastl::shared_ptr<HLSLVisualProgram> program = eastl::make_shared<HLSLVisualProgram>();
    program->SetVShader(vshader);
    program->SetPShader(pshader);
    program->SetGShader(gshader);
    return program;
}

eastl::shared_ptr<ComputeProgram> HLSLProgramFactory::CreateFromNamedFile(
	eastl::string const& csName, eastl::string const& csFile)
{
	if (csFile == "")
	{
		LogError("A program must have a compute shader.");
		return nullptr;
	}

	HLSLShader hlslCShader = HLSLShaderFactory::CreateFromFile(csName,
		csFile, csEntry, eastl::string("cs_") + version, defines, flags);
	if (hlslCShader.IsValid())
	{
		eastl::shared_ptr<ComputeShader> cshader =
			eastl::static_pointer_cast<ComputeShader>(eastl::make_shared<Shader>(hlslCShader));

		eastl::shared_ptr<HLSLComputeProgram> program = eastl::make_shared<HLSLComputeProgram>();
		program->SetCShader(cshader);
		return program;
	}
	else
	{
		return nullptr;
	}
}

eastl::shared_ptr<ComputeProgram> HLSLProgramFactory::CreateFromNamedSource(
	eastl::string const& csName, eastl::string const& csSource)
{
    if (csSource == "")
    {
        LogError("A program must have a compute shader.");
        return nullptr;
    }

    HLSLShader hlslCShader = HLSLShaderFactory::CreateFromString(csName,
        csSource, csEntry, eastl::string("cs_") + version, defines, flags);
    if (hlslCShader.IsValid())
    {
		eastl::shared_ptr<ComputeShader> cshader = 
			eastl::static_pointer_cast<ComputeShader>(eastl::make_shared<Shader>(hlslCShader));

		eastl::shared_ptr<HLSLComputeProgram> program = eastl::make_shared<HLSLComputeProgram>();
        program->SetCShader(cshader);
        return program;
    }
    else
    {
        return nullptr;
    }
}

eastl::shared_ptr<VisualProgram> HLSLProgramFactory::CreateFromNamedFiles(
	eastl::string const& vsName, eastl::string const& vsFile,
	eastl::string const& psName, eastl::string const& psFile,
	eastl::string const& gsName, eastl::string const& gsFile)
{
	if (vsFile == "" || psFile == "")
	{
		LogError("A program must have a vertex shader and a pixel shader.");
		return nullptr;
	}

	eastl::shared_ptr<VertexShader> vshader;
	eastl::shared_ptr<PixelShader> pshader;
	eastl::shared_ptr<GeometryShader> gshader;

	HLSLShader hlslVShader = HLSLShaderFactory::CreateFromFile(vsName,
		vsFile, vsEntry, eastl::string("vs_") + version, defines, flags);
	if (hlslVShader.IsValid())
	{
		vshader = eastl::static_pointer_cast<VertexShader>(
			eastl::make_shared<Shader>(hlslVShader));
	}
	else
	{
		return nullptr;
	}

	HLSLShader hlslPShader = HLSLShaderFactory::CreateFromFile(psName,
		psFile, psEntry, eastl::string("ps_") + version, defines, flags);
	if (hlslPShader.IsValid())
	{
		pshader = eastl::static_pointer_cast<PixelShader>(
			eastl::make_shared<Shader>(hlslPShader));
	}
	else
	{
		return nullptr;
	}

	HLSLShader hlslGShader;
	if (gsFile != "")
	{
		hlslGShader = HLSLShaderFactory::CreateFromFile(gsName,
			gsFile, gsEntry, eastl::string("gs_") + version, defines, flags);
		if (hlslGShader.IsValid())
		{
			gshader = eastl::static_pointer_cast<GeometryShader>(
				eastl::make_shared<Shader>(hlslGShader));
		}
		else
		{
			return nullptr;
		}
	}

	eastl::shared_ptr<HLSLVisualProgram> program = eastl::make_shared<HLSLVisualProgram>();
	program->SetVShader(vshader);
	program->SetPShader(pshader);
	program->SetGShader(gshader);
	return program;
}

eastl::shared_ptr<VisualProgram> HLSLProgramFactory::CreateFromNamedSources(
	eastl::string const& vsName, eastl::string const& vsSource,
	eastl::string const& psName, eastl::string const& psSource,
	eastl::string const& gsName, eastl::string const& gsSource)
{
    if (vsSource == "" || psSource == "")
    {
        LogError("A program must have a vertex shader and a pixel shader.");
        return nullptr;
    }

	eastl::shared_ptr<VertexShader> vshader;
	eastl::shared_ptr<PixelShader> pshader;
	eastl::shared_ptr<GeometryShader> gshader;

    HLSLShader hlslVShader = HLSLShaderFactory::CreateFromString(vsName,
        vsSource, vsEntry, eastl::string("vs_") + version, defines, flags);
    if (hlslVShader.IsValid())
    {
        vshader = eastl::static_pointer_cast<VertexShader>(
			eastl::make_shared<Shader>(hlslVShader));
    }
    else
    {
        return nullptr;
    }

    HLSLShader hlslPShader = HLSLShaderFactory::CreateFromString(psName,
        psSource, psEntry, eastl::string("ps_") + version, defines, flags);
    if (hlslPShader.IsValid())
    {
        pshader = eastl::static_pointer_cast<PixelShader>(
			eastl::make_shared<Shader>(hlslPShader));
    }
    else
    {
        return nullptr;
    }

    HLSLShader hlslGShader;
    if (gsSource != "")
    {
        hlslGShader = HLSLShaderFactory::CreateFromString(gsName,
            gsSource, gsEntry, eastl::string("gs_") + version, defines, flags);
        if (hlslGShader.IsValid())
        {
            gshader = eastl::static_pointer_cast<GeometryShader>(
				eastl::make_shared<Shader>(hlslGShader));
        }
        else
        {
            return nullptr;
        }
    }

	eastl::shared_ptr<HLSLVisualProgram> program = eastl::make_shared<HLSLVisualProgram>();
    program->SetVShader(vshader);
    program->SetPShader(pshader);
    program->SetGShader(gshader);
    return program;
}

eastl::shared_ptr<ComputeProgram> HLSLProgramFactory::CreateFromByteCode(
	eastl::vector<unsigned char> const& csBytecode)
{
    if (csBytecode.size() == 0)
    {
        LogError("A program must have a compute shader.");
        return nullptr;
    }

    HLSLShader hlslCShader = HLSLShaderFactory::CreateFromByteCode("cs",
        csEntry, eastl::string("cs_") + version, csBytecode.size(), csBytecode.data());
    if (hlslCShader.IsValid())
    {
		eastl::shared_ptr<ComputeShader> cshader = eastl::static_pointer_cast<ComputeShader>(
			eastl::make_shared<Shader>(hlslCShader));

		eastl::shared_ptr<HLSLComputeProgram> program = eastl::make_shared<HLSLComputeProgram>();
        program->SetCShader(cshader);
        return program;
    }
    else
    {
        return nullptr;
    }
}
