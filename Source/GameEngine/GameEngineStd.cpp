// GameEngineStd.cpp : source file that includes just the standard includes

#include "GameEngineStd.h" 

#include <malloc.h>
#include <EAStdC/EASprintf.h>
#include <EASTL/internal/config.h>

#if defined(_M_IX86)
	#if defined(_DEBUG)
		#pragma comment(lib, "bulletcollision_debug.lib")
		#pragma comment(lib, "bulletdynamics_debug.lib")
		#pragma comment(lib, "linearmath_debug.lib")
	#else
		#pragma comment(lib, "bulletcollision.lib")
		#pragma comment(lib, "bulletdynamics.lib")
		#pragma comment(lib, "linearmath.lib")
	#endif
#elif defined(_M_X64)
	#if defined(_DEBUG)
		#pragma comment(lib, "bulletcollision_x64__debug.lib")
		#pragma comment(lib, "bulletdynamics_x64__debug.lib")
		#pragma comment(lib, "linearmath_x64_debug.lib")
	#else
		#pragma comment(lib, "bulletcollision_x64.lib")
		#pragma comment(lib, "bulletdynamics_x64.lib")
		#pragma comment(lib, "linearmath_x64.lib")
	#endif
#else
	#error Preprocessor defines can't figure out which Bullet library to use.
#endif

#pragma comment(lib, "zlibstat.lib")

#pragma comment(lib, "libogg_static.lib")
#pragma comment(lib, "libvorbis_static.lib")
#pragma comment(lib, "libvorbisfile_static.lib")

#pragma comment(lib, "tinyxml.lib")

///////////////////////////////////////////////////////////////////////////////
// Required by EASTL.
//
#if !EASTL_EASTDC_VSNPRINTF
int Vsnprintf8(char8_t* pDestination, size_t n, const char8_t*  pFormat, va_list arguments)
{
	return EA::StdC::Vsnprintf(pDestination, n, pFormat, arguments);
}

int Vsnprintf16(char16_t* pDestination, size_t n, const char16_t* pFormat, va_list arguments)
{
	return EA::StdC::Vsnprintf(pDestination, n, pFormat, arguments);
}

#if (EASTDC_VERSION_N >= 10600)
int Vsnprintf32(char32_t* pDestination, size_t n, const char32_t* pFormat, va_list arguments)
{
	return EA::StdC::Vsnprintf(pDestination, n, pFormat, arguments);
}
#endif

#if defined(EA_WCHAR_UNIQUE) && EA_WCHAR_UNIQUE
int VsnprintfW(wchar_t* pDestination, size_t n, const wchar_t* pFormat, va_list arguments)
{
	return EA::StdC::Vsnprintf(pDestination, n, pFormat, arguments);
}
#endif
#endif


///////////////////////////////////////////////////////////////////////////////

// EASTL expects us to define these, see allocator.h line 194
void* operator new[](size_t size, const char* pName, int flags,
		     unsigned debugFlags, const char* file, int line)
{
  return malloc(size);
}
void* operator new[](size_t size, size_t alignment, size_t alignmentOffset,
    const char* pName, int flags, unsigned debugFlags, const char* file, int line)
{
  // this allocator doesn't support alignment
  EASTL_ASSERT(alignment <= 8);
  return malloc(size);
}
