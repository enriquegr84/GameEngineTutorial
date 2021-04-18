// Geometric Tools, LLC
// Copyright (c) 1998-2014
// Distributed under the Boost Software License, Version 1.0.
// http://www.boost.org/LICENSE_1_0.txt
// http://www.geometrictools.com/License/Boost/LICENSE_1_0.txt
//
// File Version: 5.0.10 (2012/11/29)

#ifndef CORESTD_H
#define CORESTD_H

//----------------------------------------------------------------------------
// Platform-specific information.  The defines to control which platform is
// included are listed below.  Add others as needed.
//
// _Windows/32 or Windows/64    :  Microsoft Windows
// __APPLE__                :  Macintosh OS X
// __linux__ or __LINUX__   :  Linux
//----------------------------------------------------------------------------
// Microsoft Windows platform
//----------------------------------------------------------------------------
#if defined(_WIN32) || defined(_WIN64) || defined(WIN32)  || defined(WIN64)

#define _WINDOWS_API_

#define WIN32_LEAN_AND_MEAN

#ifndef __MSXML_LIBRARY_DEFINED__
#define __MSXML_LIBRARY_DEFINED__
#endif

#ifndef __XMLDOCUMENT_FWD_DEFINED__
#define __XMLDOCUMENT_FWD_DEFINED__
#endif

#include <windows.h>
#include <windowsx.h>

#define LITTLE_ENDIAN

#pragma warning(disable : 4091)

// Disable the Microsoft warnings about not using the secure functions.
#pragma warning(disable : 4996)

// The use of <libname>_ITEM to export an entire class generates warnings
// when member data and functions involving templates or inlines occur. To
// avoid the warning, <libname>_ITEM can be applied only to those items
// that really need to be exported.
#pragma warning(disable : 4251) 

// Support for standard integer types. This is only a small part of what
// stdint.h provides on Unix platforms.
#include <climits>

#endif
//----------------------------------------------------------------------------
// Macintosh OS X platform
//----------------------------------------------------------------------------
#if defined(__APPLE__)

#if defined(__BIG_ENDIAN__)
#define BIG_ENDIAN
#else
#define LITTLE_ENDIAN
#endif

#endif
//----------------------------------------------------------------------------
// PC Linux platform
//----------------------------------------------------------------------------
#if !defined(__LINUX__) && defined(__linux__)
// Apparently, many PC Linux flavors define __linux__, but we have used
// __LINUX__.  To avoid breaking code by replacing __LINUX__ by __linux__,
// we will just define __LINUX__.
#define __LINUX__
#endif
#if defined(__LINUX__)

// Support for standard integer types.
#include <inttypes.h>

#define LITTLE_ENDIAN

#endif
//----------------------------------------------------------------------------

// Begin Microsoft Windows DLL support.
#if defined(CORE_DLL_EXPORT)
    // For the DLL library.
    #define CORE_ITEM __declspec(dllexport)
#elif defined(CORE_DLL_IMPORT)
    // For a client of the DLL library.
    #define CORE_ITEM __declspec(dllimport)
#else
    // For the static library and for Apple/Linux.
    #define CORE_ITEM
#endif
// End Microsoft Windows DLL support.


// User-defined keywords for syntax highlighting of special class sections.
#define public_internal public
#define protected_internal protected
#define private_internal private

// Avoid warnings about unused variables.  This is designed for variables
// that are exposed in debug configurations but are hidden in release
// configurations.
#define UNUSED(variable) (void)variable

// Macros to enable or disable various Assert subsystems.  TODO:  Currently,
// the extended assert system is implemented only for Microsoft Visual Studio.
#ifdef _DEBUG

    #if defined(_WINDOWS_) && defined(_MSC_VER)

		#include <crtdbg.h>

        #define USE_ASSERT
        #ifdef USE_ASSERT
            #define USE_ASSERT_WRITE_TO_OUTPUT_WINDOW
            #define USE_ASSERT_WRITE_TO_MESSAGE_BOX
        #endif
    #endif
    //#define USE_ASSERT_LOG_TO_FILE
#else
	#include <assert.h> 
#endif

// Common standard library headers.
#include <cstdarg>
#include <cstddef>
#include <cstdio>
#include <iostream>
#include <istream>
#include <ostream>
#include <sstream>
#include <fstream>
#include <memory>

//Third parties libraries stl-containers
#include "EASTL/algorithm.h"
#include "EASTL/list.h"
#include "EASTL/vector.h"
#include "EASTL/allocator.h"
#include "EASTL/queue.h"
#include "EASTL/hash_map.h"
#include "EASTL/map.h"
#include "EASTL/set.h"
#include "EASTL/sort.h"
#include "EASTL/array.h"
#include "EASTL/string.h"
#include "EASTL/unique_ptr.h"
#include "EASTL/shared_ptr.h"

// fast delegate
#include "FastDelegate.h"
using fastdelegate::MakeDelegate;

#include "tinyxml2.h"

#ifdef _WINDOWS_

#include <direct.h>
#include <io.h>
#include <tchar.h>

// Create a string with last error message
inline eastl::string GetErrorMessage(HRESULT hr)
{
	LPWSTR lpMsgBuf;
	DWORD bufLen = FormatMessage(
		FORMAT_MESSAGE_ALLOCATE_BUFFER |
		FORMAT_MESSAGE_FROM_SYSTEM |
		FORMAT_MESSAGE_IGNORE_INSERTS,
		NULL,
		hr,
		MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
		(LPWSTR)&lpMsgBuf,
		0, NULL);
	if (bufLen)
	{
		std::wstring error(lpMsgBuf);
		std::string errorMessage(error.begin(), error.end());

		LocalFree(lpMsgBuf);
		return eastl::string(errorMessage.c_str());
	}
	else
	{
		char buf[256];
		sprintf(buf, "error message failed with %d", GetLastError());
		return eastl::string(buf);
	}
}

#endif

template <class BaseType, class SubType>
BaseType* GenericObjectCreationFunction(void) { return new SubType; }

template <class BaseClass, class IdType>
class GenericObjectFactory
{
	typedef BaseClass* (*ObjectCreationFunction)(void);
	eastl::map<IdType, ObjectCreationFunction> mCreationFunctions;

public:
	template <class SubClass>
	bool Register(IdType id)
	{
		auto findIt = mCreationFunctions.find(id);
		if (findIt == mCreationFunctions.end())
		{
			mCreationFunctions[id] =
				&GenericObjectCreationFunction<BaseClass, SubClass>;
			return true;
		}

		return false;
	}

	BaseClass* Create(IdType id)
	{
		auto findIt = mCreationFunctions.find(id);
		if (findIt != mCreationFunctions.end())
		{
			ObjectCreationFunction func = findIt->second;
			return func();
		}

		return NULL;
	}
};

#endif
