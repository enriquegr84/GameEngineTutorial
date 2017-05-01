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

	//! Get the current working directory.
	/** \return Current working directory as a string. */
    static const eastl::wstring GetWorkingDirectory ();

	//! Converts a relative path to an absolute (unique) path,
	/** \param filename Possibly relative file or directory name to query.
	\result Absolute filename which points to the same file. */
	static eastl::string GetAbsolutePath(const eastl::string& filename);
};

#endif
