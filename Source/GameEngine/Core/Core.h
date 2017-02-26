// Geometric Tools, LLC
// Copyright (c) 1998-2014
// Distributed under the Boost Software License, Version 1.0.
// http://www.boost.org/LICENSE_1_0.txt
// http://www.geometrictools.com/License/Boost/LICENSE_1_0.txt
//
// File Version: 5.0.0 (2010/01/01)

#ifndef CORE_H
#define CORE_H

// Assert
#include "Logger/Logger.h"
#include "Logger/LogReporter.h"
#include "Logger/LogToFile.h"
#include "Logger/LogToStdout.h"
#include "Logger/LogToStringArray.h"

#if defined(USE_WINDOWS)
#include "Logger/Windows/LogToMessageBox.h"
#include "Logger/Windows/LogToOutputWindow.h"
#endif

// InputOutput
#include "IO/Environment.h"
#include "IO/FileIO.h"

//OS
#include "OS/OS.h"

#include <mutex>

#endif
