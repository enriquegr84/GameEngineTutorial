// Geometric Tools, LLC
// Copyright (c) 1998-2014
// Distributed under the Boost Software License, Version 1.0.
// http://www.boost.org/LICENSE_1_0.txt
// http://www.geometrictools.com/License/Boost/LICENSE_1_0.txt
//
// File Version: 5.0.1 (2010/04/30)

#ifndef CONSOLEAPPLICATION_H
#define CONSOLEAPPLICATION_H

#include "Application.h"

class ConsoleApplication : public Application
{
public:
    // Construction and destruction.
    ConsoleApplication (const char* consoleTitle);
    virtual ~ConsoleApplication ();

	// Event callbacks.
	virtual bool OnInitialize() { return true; }
	virtual void OnTerminate() { }

	// The hookup to the 'main' entry point into the executable.
	static int Run(int numArguments, char** arguments);

protected:

    // Console parameters (from the constructor).
    eastl::string mConsoleTitle;
};
//----------------------------------------------------------------------------

#endif
