// Geometric Tools, LLC
// Copyright (c) 1998-2014
// Distributed under the Boost Software License, Version 1.0.
// http://www.boost.org/LICENSE_1_0.txt
// http://www.geometrictools.com/License/Boost/LICENSE_1_0.txt
//
// File Version: 5.0.5 (2012/03/09)

#ifndef MATHSTD_H
#define MATHSTD_H

// Common standard library headers.
#include <cctype>
#include <cfloat>
#include <climits>
#include <cmath>
#include <cstdlib>

// Begin Microsoft Windows DLL support.
#if defined(MATH_DLL_EXPORT)
    // For the DLL library.
    #define MATH_ITEM __declspec(dllexport)
#elif defined(MATH_DLL_IMPORT)
    // For a client of the DLL library.
    #define MATH_ITEM __declspec(dllimport)
#else
    // For the static library and for Apple/Linux.
    #define MATH_ITEM
#endif
// End Microsoft Windows DLL support.

// Expose exactly one of these.
#define GE_USE_ROW_MAJOR
//#define GE_USE_COL_MAJOR

// Expose exactly one of these.
#define GE_USE_MAT_VEC
//#define GE_USE_VEC_MAT

#if (defined(GE_USE_ROW_MAJOR) && defined(GE_USE_COL_MAJOR)) || (!defined(GE_USE_ROW_MAJOR) && !defined(GE_USE_COL_MAJOR))
#error Exactly one storage order must be specified.
#endif

#if (defined(GE_USE_MAT_VEC) && defined(GE_USE_VEC_MAT)) || (!defined(GE_USE_MAT_VEC) && !defined(GE_USE_VEC_MAT))
#error Exactly one multiplication convention must be specified.
#endif

#endif
