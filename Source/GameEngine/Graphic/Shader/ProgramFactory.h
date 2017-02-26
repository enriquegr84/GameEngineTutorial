// David Eberly, Geometric Tools, Redmond WA 98052
// Copyright (c) 1998-2017
// Distributed under the Boost Software License, Version 1.0.
// http://www.boost.org/LICENSE_1_0.txt
// http://www.geometrictools.com/License/Boost/LICENSE_1_0.txt
// File Version: 3.0.0 (2016/06/19)

#ifndef PROGRAMFACTORY_H
#define PROGRAMFACTORY_H

#include "Graphic/Shader/ComputeProgram.h"
#include "Graphic/Shader/ProgramDefines.h"
#include "Graphic/Shader/VisualProgram.h"

#include <EASTL/stack.h>

class GRAPHIC_ITEM ProgramFactory
{
public:
    // Abstract base class.
    virtual ~ProgramFactory();
    ProgramFactory();

    // All members are in public scope, because there are no side effects
    // when the values are modified.  The current values are used in the
    // the Create(...) functions.
    eastl::string version;
	eastl::string vsEntry, psEntry, gsEntry, csEntry;
    ProgramDefines defines;
    unsigned int flags;

    // The returned value is used as a lookup index into arrays of strings
    // corresponding to shader programs.  Currently, GLSLProgramFactory
    // returns PF_GLSL and HLSLProgramFactory returns PF_HLSL.
    enum
    {
        PF_GLSL,
        PF_HLSL,
        PF_NUM_API
    };

    virtual int GetAPI() const = 0;

	// Create a program for GPU display. The filenames are passed as parameters 
	// in case the shader compiler needs this for #include path searches.
	eastl::shared_ptr<VisualProgram> CreateFromFiles(
		eastl::string const& vsFile, eastl::string const& psFile, eastl::string const& gsFile);

	eastl::shared_ptr<VisualProgram> CreateFromSources(
		eastl::string const& vsSource, eastl::string const& psSource, eastl::string const& gsSource);

    // Create a program for GPU computing. The filename is passed the 'csName' 
	// parameters in case the shader compiler needs this for #include path searches.
	eastl::shared_ptr<ComputeProgram> CreateFromFile(eastl::string const& csFile);

	eastl::shared_ptr<ComputeProgram> CreateFromSource(eastl::string const& csSource);

    // Support for passing ProgramFactory objects to a function that
    // potentially modifies 'defines' or 'flags' but then needs to resstore
    // the previous state on return.  The PushDefines() function saves the
    // current 'defines' on a stack and then clears 'defines'.  The
    // PushFlags() function saves the current 'flags' on a stack and then
    // sets 'flags' to zero.  If you need to modify subelements of either
    // member, you will have to manage that on your own.
    void PushDefines();
    void PopDefines();
    void PushFlags();
    void PopFlags();

protected:
	virtual eastl::shared_ptr<VisualProgram> CreateFromNamedFiles(
		eastl::string const& vsName, eastl::string const& vsFile,
		eastl::string const& psName, eastl::string const& psFile,
		eastl::string const& gsName, eastl::string const& gsFile) = 0;

    virtual eastl::shared_ptr<VisualProgram> CreateFromNamedSources(
		eastl::string const& vsName, eastl::string const& vsSource,
		eastl::string const& psName, eastl::string const& psSource,
		eastl::string const& gsName, eastl::string const& gsSource) = 0;

    virtual eastl::shared_ptr<ComputeProgram> CreateFromNamedSource(
		eastl::string const& csName, eastl::string const& csSource) = 0;

	virtual eastl::shared_ptr<ComputeProgram> CreateFromNamedFile(
		eastl::string const& csName, eastl::string const& csFile) = 0;

private:
	eastl::stack<ProgramDefines> mDefinesStack;
	eastl::stack<unsigned int> mFlagsStack;

    // Prevent assignment.
    ProgramFactory& operator=(ProgramFactory const&) { return *this; }
};

#endif
