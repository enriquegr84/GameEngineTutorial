// David Eberly, Geometric Tools, Redmond WA 98052
// Copyright (c) 1998-2017
// Distributed under the Boost Software License, Version 1.0.
// http://www.boost.org/LICENSE_1_0.txt
// http://www.geometrictools.com/License/Boost/LICENSE_1_0.txt
// File Version: 3.0.0 (2016/06/19)

#ifndef HLSLPROGRAMFACTORY_H
#define HLSLPROGRAMFACTORY_H

#include "Graphic/Shader/ProgramFactory.h"

class GRAPHIC_ITEM HLSLProgramFactory : public ProgramFactory
{
public:
    // The 'defaultVersion' can be set once on application initialization if
    // you want an HLSL version different from our default when constructing a
    // program factory.
    static eastl::string defaultVersion;  // "5_0" (Shader Model 5)
    static eastl::string defaultVSEntry;  // "VSMain"
    static eastl::string defaultPSEntry;  // "PSMain"
    static eastl::string defaultGSEntry;  // "GSMain"
    static eastl::string defaultCSEntry;  // "CSMain"
    static unsigned int defaultFlags;
        // enable strictness, ieee strictness, optimization level 3

    // Construction.  The 'version' member is set to 'defaultVersion'.  The
    // 'defines' are empty.
    virtual ~HLSLProgramFactory();
    HLSLProgramFactory();

    // The returned value is used as a lookup index into arrays of strings
    // corresponding to shader programs.
    virtual int GetAPI() const;

    // Create a program for GPU display.
	eastl::shared_ptr<VisualProgram> CreateFromByteCode(
		eastl::vector<unsigned char> const& vsBytecode,
		eastl::vector<unsigned char> const& psBytecode,
		eastl::vector<unsigned char> const& gsBytecode);

    // Create a program for GPU computing.
	eastl::shared_ptr<ComputeProgram> CreateFromByteCode(
		eastl::vector<unsigned char> const& csBytecode);

private:
	// Create a program for GPU display. The files are
	// passed as parameters in case the shader compiler needs
	// this for #include path searches.
	virtual eastl::shared_ptr<VisualProgram> CreateFromNamedFiles(
		eastl::string const& vsName, eastl::string const& vsFile,
		eastl::string const& psName, eastl::string const& psFile,
		eastl::string const& gsName, eastl::string const& gsFile);

    // Create a program for GPU display.  The files are loaded, converted to
    // strings, and passed to CreateFromNamedSources.  The filenames are
    // passed as the 'xsName' parameters in case the shader compiler needs
    // this for #include path searches.
    virtual eastl::shared_ptr<VisualProgram> CreateFromNamedSources(
		eastl::string const& vsName, eastl::string const& vsSource,
		eastl::string const& psName, eastl::string const& psSource,
		eastl::string const& gsName, eastl::string const& gsSource);

    // Create a program for GPU computing. The filename is passed as parameters 
	// in case the shader compiler needs this for #include path searches.
    virtual eastl::shared_ptr<ComputeProgram> CreateFromNamedFile(
		eastl::string const& csName, eastl::string const& csFile);

	// Create a program for GPU computing. The file is loaded, converted to
	// a string, and passed to CreateFromNamedSource. The filename is passed
	// as the 'csName' parameters in case the shader compiler needs this for
	// #include path searches.
	virtual eastl::shared_ptr<ComputeProgram> CreateFromNamedSource(
		eastl::string const& csName, eastl::string const& csSource);
};

#endif