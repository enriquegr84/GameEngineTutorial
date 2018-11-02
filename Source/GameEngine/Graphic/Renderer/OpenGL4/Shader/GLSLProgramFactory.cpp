// David Eberly, Geometric Tools, Redmond WA 98052
// Copyright (c) 1998-2018
// Distributed under the Boost Software License, Version 1.0.
// http://www.boost.org/LICENSE_1_0.txt
// http://www.geometrictools.com/License/Boost/LICENSE_1_0.txt
// File Version: 3.0.0 (2016/06/19)


#include "Core/Logger/Logger.h"
#include "Core/IO/FileSystem.h"
#include "Core/Utility/StringUtil.h"

#include "GLSLComputeProgram.h"
#include "GLSLProgramFactory.h"
#include "GLSLVisualProgram.h"


eastl::string GLSLProgramFactory::defaultVersion = "#version 430";
eastl::string GLSLProgramFactory::defaultVSEntry = "main";
eastl::string GLSLProgramFactory::defaultPSEntry = "main";
eastl::string GLSLProgramFactory::defaultGSEntry = "main";
eastl::string GLSLProgramFactory::defaultCSEntry = "main";
unsigned int GLSLProgramFactory::defaultFlags = 0;  // unused in GLSL for now

GLSLProgramFactory::GLSLProgramFactory()
{
    version = defaultVersion;
    vsEntry = defaultVSEntry;
    psEntry = defaultPSEntry;
    gsEntry = defaultGSEntry;
    csEntry = defaultCSEntry;
    flags = defaultFlags;
}

eastl::shared_ptr<VisualProgram> GLSLProgramFactory::CreateFromNamedFiles(
	eastl::string const&, eastl::string const& vsFile,
	eastl::string const&, eastl::string const& psFile,
	eastl::string const&, eastl::string const& gsFile)
{
	BaseReadFile* vertexFile = FileSystem::Get()->CreateReadFile(ToWideString(vsFile.c_str()));
	BaseReadFile* pixelFile = FileSystem::Get()->CreateReadFile(ToWideString(psFile.c_str()));
	if (vertexFile == nullptr || pixelFile == nullptr)
	{
		LogError("A program must have a vertex shader and a pixel shader.");
		delete vertexFile;
		delete pixelFile;
		return nullptr;
	}

	char* vsSource = new char[vertexFile->GetSize() + 1];
	memset(vsSource, 0, vertexFile->GetSize() + 1);
	vertexFile->Read(vsSource, vertexFile->GetSize());
	GLuint vsHandle = Compile(GL_VERTEX_SHADER, vsSource);
	delete vertexFile;
	if (vsHandle == 0)
	{
		return nullptr;
	}

	char* psSource = new char[pixelFile->GetSize() + 1];
	memset(psSource, 0, pixelFile->GetSize() + 1);
	pixelFile->Read(psSource, pixelFile->GetSize());
	GLuint psHandle = Compile(GL_FRAGMENT_SHADER, psSource);
	delete pixelFile;
	if (psHandle == 0)
	{
		return nullptr;
	}

	GLuint gsHandle = 0;
	BaseReadFile* geometryFile = FileSystem::Get()->CreateReadFile(ToWideString(gsFile.c_str()));
	delete geometryFile;
	if (geometryFile != nullptr)
	{
		char* gsSource = new char[geometryFile->GetSize() + 1];
		memset(gsSource, 0, geometryFile->GetSize() + 1);
		geometryFile->Read(gsSource, geometryFile->GetSize());
		gsHandle = Compile(GL_GEOMETRY_SHADER, gsSource);
		if (gsHandle == 0)
		{
			return nullptr;
		}
	}

	GLuint programHandle = glCreateProgram();
	if (programHandle == 0)
	{
		LogError("Program creation failed.");
		return nullptr;
	}

	glAttachShader(programHandle, vsHandle);
	glAttachShader(programHandle, psHandle);
	if (gsHandle > 0)
	{
		glAttachShader(programHandle, gsHandle);
	}

	if (!Link(programHandle))
	{
		glDetachShader(programHandle, vsHandle);
		glDeleteShader(vsHandle);
		glDetachShader(programHandle, psHandle);
		glDeleteShader(psHandle);
		if (gsHandle)
		{
			glDetachShader(programHandle, gsHandle);
			glDeleteShader(gsHandle);
		}
		glDeleteProgram(programHandle);
		return nullptr;
	}

	eastl::shared_ptr<GLSLVisualProgram> program =
		eastl::make_shared<GLSLVisualProgram>(programHandle, vsHandle, psHandle, gsHandle);

	GLSLReflection const& reflector = program->GetReflector();
	program->SetVShader(eastl::make_shared<VertexShader>(reflector));
	program->SetPShader(eastl::make_shared<PixelShader>(reflector));
	if (gsHandle > 0)
	{
		program->SetGShader(eastl::make_shared<GeometryShader>(reflector));
	}
	return program;
}

eastl::shared_ptr<VisualProgram> GLSLProgramFactory::CreateFromNamedSources(
    eastl::string const&, eastl::string const& vsSource,
    eastl::string const&, eastl::string const& psSource,
    eastl::string const&, eastl::string const& gsSource)
{
    if (vsSource == "" || psSource == "")
    {
        LogError("A program must have a vertex shader and a pixel shader.");
        return nullptr;
    }

    GLuint vsHandle = Compile(GL_VERTEX_SHADER, vsSource);
    if (vsHandle == 0)
    {
        return nullptr;
    }

    GLuint psHandle = Compile(GL_FRAGMENT_SHADER, psSource);
    if (psHandle == 0)
    {
        return nullptr;
    }

    GLuint gsHandle = 0;
    if (gsSource != "")
    {
        gsHandle = Compile(GL_GEOMETRY_SHADER, gsSource);
        if (gsHandle == 0)
        {
            return nullptr;
        }
    }

    GLuint programHandle = glCreateProgram();
    if (programHandle == 0)
    {
        LogError("Program creation failed.");
        return nullptr;
    }

    glAttachShader(programHandle, vsHandle);
    glAttachShader(programHandle, psHandle);
    if (gsHandle > 0)
    {
        glAttachShader(programHandle, gsHandle);
    }

    if (!Link(programHandle))
    {
        glDetachShader(programHandle, vsHandle);
        glDeleteShader(vsHandle);
        glDetachShader(programHandle, psHandle);
        glDeleteShader(psHandle);
        if (gsHandle)
        {
            glDetachShader(programHandle, gsHandle);
            glDeleteShader(gsHandle);
        }
        glDeleteProgram(programHandle);
        return nullptr;
    }

    eastl::shared_ptr<GLSLVisualProgram> program =
        eastl::make_shared<GLSLVisualProgram>(programHandle, vsHandle, psHandle, gsHandle);

    GLSLReflection const& reflector = program->GetReflector();
    program->SetVShader(eastl::make_shared<VertexShader>(reflector));
    program->SetPShader(eastl::make_shared<PixelShader>(reflector));
    if (gsHandle > 0)
    {
        program->SetGShader(eastl::make_shared<GeometryShader>(reflector));
    }
    return program;
}

eastl::shared_ptr<ComputeProgram> GLSLProgramFactory::CreateFromNamedFile(
	eastl::string const&, eastl::string const& csFile)
{
	BaseReadFile* shaderFile = FileSystem::Get()->CreateReadFile(ToWideString(csFile.c_str()));
	if (shaderFile == nullptr)
	{
		LogError("A program must have a compute shader.");
		return nullptr;
	}

	char* csSource = new char[shaderFile->GetSize() + 1];
	memset(csSource, 0, shaderFile->GetSize() + 1);
	shaderFile->Read(csSource, shaderFile->GetSize());
	GLuint csHandle = Compile(GL_COMPUTE_SHADER, csSource);
	if (csHandle == 0)
	{
		return nullptr;
	}

	GLuint programHandle = glCreateProgram();
	if (programHandle == 0)
	{
		LogError("Program creation failed.");
		return nullptr;
	}

	glAttachShader(programHandle, csHandle);

	if (!Link(programHandle))
	{
		glDetachShader(programHandle, csHandle);
		glDeleteShader(csHandle);
		glDeleteProgram(programHandle);
		return nullptr;
	}

	eastl::shared_ptr<GLSLComputeProgram> program =
		eastl::make_shared<GLSLComputeProgram>(programHandle, csHandle);

	GLSLReflection const& reflector = program->GetReflector();
	program->SetCShader(eastl::make_shared<ComputeShader>(reflector));
	return program;
}

eastl::shared_ptr<ComputeProgram> GLSLProgramFactory::CreateFromNamedSource(
    eastl::string const&, eastl::string const& csSource)
{
    if (csSource == "")
    {
        LogError("A program must have a compute shader.");
        return nullptr;
    }

    GLuint csHandle = Compile(GL_COMPUTE_SHADER, csSource);
    if (csHandle == 0)
    {
        return nullptr;
    }

    GLuint programHandle = glCreateProgram();
    if (programHandle == 0)
    {
        LogError("Program creation failed.");
        return nullptr;
    }

    glAttachShader(programHandle, csHandle);

    if (!Link(programHandle))
    {
        glDetachShader(programHandle, csHandle);
        glDeleteShader(csHandle);
        glDeleteProgram(programHandle);
        return nullptr;
    }

    eastl::shared_ptr<GLSLComputeProgram> program =
        eastl::make_shared<GLSLComputeProgram>(programHandle, csHandle);

    GLSLReflection const& reflector = program->GetReflector();
    program->SetCShader(eastl::make_shared<ComputeShader>(reflector));
    return program;
}

GLuint GLSLProgramFactory::Compile(GLenum shaderType, eastl::string const& source)
{
    GLuint handle = glCreateShader(shaderType);
    if (handle > 0)
    {
        // Prepend to the definitions
        // 1. The version of the GLSL program; for example, "#version 400".
        // 2. A define for the matrix-vector multiplication convention if
        //    it is selected as GTE_USE_MAT_VEC: "define GTE_USE_MAT_VEC 1"
        //    else "define GTE_USE_MAT_VEC 0".
        // 3. "layout(std140, *_major) uniform;" for either row_major or column_major
        //    to select default for all uniform matrices and select std140 layout.
        // 4. "layout(std430, *_major) buffer;" for either row_major or column_major
        //    to select default for all buffer matrices and select std430 layout.
        // Append to the definitions the source-code string.
        auto const& definitions = defines.Get();
        eastl::vector<eastl::string> glslDefines;
        glslDefines.reserve(definitions.size() + 5);
        glslDefines.push_back(version + "\n");
#if defined(GTE_USE_MAT_VEC)
        glslDefines.push_back("#define GTE_USE_MAT_VEC 1\n");
#else
        glslDefines.push_back("#define GTE_USE_MAT_VEC 0\n");
#endif
#if defined(GTE_USE_ROW_MAJOR)
        glslDefines.push_back("layout(std140, row_major) uniform;\n");
        glslDefines.push_back("layout(std430, row_major) buffer;\n");
#else
        glslDefines.push_back("layout(std140, column_major) uniform;\n");
        glslDefines.push_back("layout(std430, column_major) buffer;\n");
#endif
        for (auto d : definitions)
        {
            glslDefines.push_back("#define " + d.first + " " + d.second + "\n");
        }
        glslDefines.push_back(source);

        // Repackage the definitions for glShaderSource.
        eastl::vector<GLchar const*> code;
        code.reserve(glslDefines.size());
        for (auto const& d : glslDefines)
        {
            code.push_back(d.c_str());
        }

        glShaderSource(handle, static_cast<GLsizei>(code.size()), &code[0],
            nullptr);

        glCompileShader(handle);
        GLint status;
        glGetShaderiv(handle, GL_COMPILE_STATUS, &status);
        if (status == GL_FALSE)
        {
            GLint logLength;
            glGetShaderiv(handle, GL_INFO_LOG_LENGTH, &logLength);
            if (logLength > 0)
            {
                eastl::vector<GLchar> log(logLength);
                GLsizei numWritten;
                glGetShaderInfoLog(handle, static_cast<GLsizei>(logLength), &numWritten, log.data());
                LogError("Compile failed:\n" + eastl::string(log.data()));
            }
            else
            {
                LogError("Invalid info log length.");
            }
            glDeleteShader(handle);
            handle = 0;
        }
    }
    else
    {
        LogError("Cannot create shader.");
    }
    return handle;
}

bool GLSLProgramFactory::Link(GLuint programHandle)
{
    glLinkProgram(programHandle);
    int status;
    glGetProgramiv(programHandle, GL_LINK_STATUS, &status);
    if (status == GL_FALSE)
    {
        int logLength;
        glGetProgramiv(programHandle, GL_INFO_LOG_LENGTH, &logLength);
        if (logLength > 0)
        {
            eastl::vector<GLchar> log(logLength);
            int numWritten;
            glGetProgramInfoLog(programHandle, logLength, &numWritten, log.data());
            LogError("Link failed:\n" + eastl::string(log.data()));
        }
        else
        {
            LogError("Invalid info log length.");
        }
        return false;
    }
    else
    {
        return true;
    }
}
