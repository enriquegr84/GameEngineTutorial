// Geometric Tools, LLC
// Copyright (c) 1998-2014
// Distributed under the Boost Software License, Version 1.0.
// http://www.boost.org/LICENSE_1_0.txt
// http://www.geometrictools.com/License/Boost/LICENSE_1_0.txt
//
// File Version: 5.0.1 (2010/04/30)

#include "ConsoleApplication.h"

//----------------------------------------------------------------------------
ConsoleApplication::ConsoleApplication (const char* consoleTitle)
    :
    mConsoleTitle(consoleTitle)
{
	ProjectApplicationPath = ApplicationPath + mConsoleTitle + "/";
}
//----------------------------------------------------------------------------
ConsoleApplication::~ConsoleApplication ()
{
}
//----------------------------------------------------------------------------
int ConsoleApplication::Run (int numArguments, char** arguments)
{
    ConsoleApplication* consoleApp = (ConsoleApplication*)App;
    return 0;
}
//----------------------------------------------------------------------------
