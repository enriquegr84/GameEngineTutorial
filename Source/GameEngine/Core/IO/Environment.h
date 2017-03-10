// Geometric Tools, LLC
// Copyright (c) 1998-2014
// Distributed under the Boost Software License, Version 1.0.
// http://www.boost.org/LICENSE_1_0.txt
// http://www.geometrictools.com/License/Boost/LICENSE_1_0.txt
//
// File Version: 5.0.1 (2010/10/01)

#ifndef ENVIRONMENT_H
#define ENVIRONMENT_H

#include "Core/CoreStd.h"

class CORE_ITEM Environment
{

public:

	// Event callbacks.
	static void OnInitialize();
	static void OnTerminate();

	//! Get the current working directory.
	/** \return Current working directory as a string. */
    static const eastl::wstring GetWorkingDirectory ();

	//! Converts a relative path to an absolute (unique) path,
	/** \param filename Possibly relative file or directory name to query.
	\result Absolute filename which points to the same file. */
	static eastl::string GetAbsolutePath(const eastl::string& filename);

    // Support for paths to locate files.  For platform independence, use
    // "/" for the path separator.  Please terminate the input 'directory'
    // values with "/".  The Insert/Remove functions return 'true' iff the
    // operation was successful.
    static bool InsertDirectory (const eastl::string& directory);
    static bool RemoveDirectory (const eastl::string& directory);
    static void RemoveAllDirectories ();

    // The GetPath* function searches the list of directories and returns the
    // fully decorated file name, assuming it satisfies the required
    // conditions, or returns "" if conditions are not met.  GetPathR succeeds
    // when the file can be opened for reading.  GetPathW succeeds when the
    // file can be opened for writing.  GetPathRW succedds when the file can
    // be opened for reading and/or writing.
    static eastl::string GetPathR (const eastl::string& fileName);
    static eastl::string GetPathW (const eastl::string& fileName);
    static eastl::string GetPathRW (const eastl::string& fileName);

private:

    static eastl::string GetPath (const eastl::string& filename, const char* attributes);

    // The list of directories for GetPath to search.  The list is
    // allocated during InitTerm::ExecuteInitializers.  As with other
    // classes in GameEngine, no dynamic allocation occurs pre-main to
    // assist in tracking memory problems.
    static eastl::vector<eastl::string>* msDirectories;
};

#endif
