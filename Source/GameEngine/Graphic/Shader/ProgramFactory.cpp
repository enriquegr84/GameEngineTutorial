// David Eberly, Geometric Tools, Redmond WA 98052
// Copyright (c) 1998-2017
// Distributed under the Boost Software License, Version 1.0.
// http://www.boost.org/LICENSE_1_0.txt
// http://www.geometrictools.com/License/Boost/LICENSE_1_0.txt
// File Version: 3.0.0 (2016/06/19)


#include "ProgramFactory.h"
#include <fstream>

eastl::shared_ptr<ProgramFactory> ProgramFactory::mProgramFactory = NULL;

eastl::shared_ptr<ProgramFactory> ProgramFactory::Get(void)
{
	LogAssert(ProgramFactory::mProgramFactory, "ProgramFactory doesn't exist");
	return ProgramFactory::mProgramFactory;
}

ProgramFactory::ProgramFactory()
    :
    version(""),
    vsEntry(""),
    psEntry(""),
    gsEntry(""),
    csEntry(""),
    defines(),
    flags(0)
{
	if (ProgramFactory::mProgramFactory)
	{
		LogError("Attempting to create two global program factory! \
					The old one will be destroyed and overwritten with this one.");
	}

	ProgramFactory::mProgramFactory.reset(this);
}

ProgramFactory::~ProgramFactory()
{
	if (ProgramFactory::mProgramFactory.get() == this)
		ProgramFactory::mProgramFactory = nullptr;
}

eastl::shared_ptr<VisualProgram> ProgramFactory::CreateFromFiles(
	eastl::string const& vsFile, eastl::string const& psFile, eastl::string const& gsFile)
{
    return CreateFromNamedFiles("vs", vsFile, "ps", psFile, "gs", gsFile);
}

eastl::shared_ptr<VisualProgram> ProgramFactory::CreateFromSources(
	eastl::string const& vsSource, eastl::string const& psSource, eastl::string const& gsSource)
{
    return CreateFromNamedSources("vs", vsSource, "ps", psSource, "gs", gsSource);
}

eastl::shared_ptr<ComputeProgram> ProgramFactory::CreateFromFile(eastl::string const& csFile)
{
    return CreateFromNamedFile("cs", csFile);
}

eastl::shared_ptr<ComputeProgram> ProgramFactory::CreateFromSource(eastl::string const& csSource)
{
    return CreateFromNamedSource("cs", csSource);
}

void ProgramFactory::PushDefines()
{
    mDefinesStack.push(defines);
    defines.Clear();
}

void ProgramFactory::PopDefines()
{
    if (mDefinesStack.size() > 0)
    {
        defines = mDefinesStack.top();
        mDefinesStack.pop();
    }
}

void ProgramFactory::PushFlags()
{
    mFlagsStack.push(flags);
    flags = 0;
}

void ProgramFactory::PopFlags()
{
    if (mFlagsStack.size() > 0)
    {
        flags = mFlagsStack.top();
        mFlagsStack.pop();
    }
}

