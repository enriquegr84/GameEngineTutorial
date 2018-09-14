// Geometric Tools, LLC
// Copyright (c) 1998-2014
// Distributed under the Boost Software License, Version 1.0.
// http://www.boost.org/LICENSE_1_0.txt
// http://www.geometrictools.com/License/Boost/LICENSE_1_0.txt
//
// File Version: 5.0.3 (2011/07/29)

#ifndef GRAPHICSTD_H
#define GRAPHICSTD_H

//! Define and _COMPILE_WITH_DIRECT3D11_ to
//! compile the game engine with DIRECT3D11.
/** If you only want to use the software device or opengl you can disable those defines.
This switch is mostly disabled because people do not get the g++ compiler compile
directX header files, and directX is only available on Windows platforms. If you
are using Dev-Cpp, and want to compile this using a DX dev pack, you can define
_COMPILE_WITH_DX11_DEV_PACK_. So you simply need to add something like this
to the compiler settings: -DIRR_COMPILE_WITH_DX11_DEV_PACK
and this to the linker settings: -ld3dx11
*/
#if defined(_WIN32) || defined(_WIN64) || defined(WIN32) || defined(WIN64)

#define USE_DX11

#include <D3D11.h>
#include <D3Dcompiler.h>
#include <DXGI.h>

// Fake enumerations to have human-readable names that D3D11 did not provide.

// D3D11_BIND_FLAG
#define D3D11_BIND_NONE 0

// D3D11_RESOURCE_MISC_FLAG
#define D3D11_RESOURCE_MISC_NONE 0

// D3D11_BUFFER_UAV_FLAG
#define D3D11_BUFFER_UAV_FLAG_BASIC 0

// D3D11_CPU_ACCESS_FLAG
#define D3D11_CPU_ACCESS_NONE 0
#define D3D11_CPU_ACCESS_READ_WRITE (D3D11_CPU_ACCESS_READ | D3D11_CPU_ACCESS_WRITE)

// D3D11_QUERY_MISC_FLAG
#define D3D11_QUERY_MISC_NONE 0

#ifdef NO_USE_DX11
#undef USE_DX11
#endif

#endif

// Begin Microsoft Windows DLL support.
#if defined(GRAPHICS_DLL_EXPORT)
    // For the DLL library.
    #define GRAPHIC_ITEM __declspec(dllexport)
#elif defined(GRAPHICS_DLL_IMPORT)
    // For a client of the DLL library.
    #define GRAPHIC_ITEM __declspec(dllimport)
#else
    // For the static library and for Apple/Linux.
    #define GRAPHIC_ITEM
#endif
// End Microsoft Windows DLL support.

// Enable this define to verify that the camera axis directions form a
// right-handed orthonormal set.  The test is done only once, because
// numerical round-off errors during rotations of the frame over time
// may require a renormalization of the frame.
#ifdef _DEBUG
    #define VALIDATE_CAMERA_FRAME_ONCE
#endif

// Enable this define to test whether Shader::MAX_PROFILES has changed
// and affects the streamed files.
#ifdef _DEBUG
    #define ASSERT_ON_CHANGED_MAX_PROFILES
#endif

// Expose this define to allow resetting of render state and other state in
// the Renderer::Draw (const Visual*, const VisualEffectInstance*) call.
//#define RESET_STATE_AFTER_DRAW

// Enable this to allow counting the number of pixels drawn in
// Renderer::DrawPrimitive.
//#define QUERY_PIXEL_COUNT

// Enable this to draw text using display lists in OpenGL; otherwise, text is
// drawn manually using bitmapped fonts.
#ifdef _OPENGL_
    #if defined(WIN32) && !defined(USE_GLUT)
        #define TEXT_DISPLAY_LIST
    #endif

    // Some OpenGL 2.x drivers are not handling normal attributes correctly.
    // This is a problem should you want to use the normal vector to pass a
    // 4-tuple of information to the shader.  The OpenGL 1.x glNormalPointer
    // assumes the normals are 3-tuples.  If you know that your target
    // machines correctly support OpenGL 2.x normal attributes, expose the
    // following #define. Otherwise, the renderer will use the OpenGL 1.x
    // glNormalPointer.
    //
    // On the Macintosh, whether PowerPC or Intel, with NVIDIA graphics cards,
    // the generic attributes for normals does not work.  This happens because
    // we are using OpenGL 1.x extensions for shader programming.  We need to
    // update to OpenGL 2.x and later.
    #ifndef __APPLE__
        #define _OPENGL_2_NORMAL_ATTRIBUTES
    #endif
#endif

// Enables checking of various data in the DX11 renderer.
#ifdef USE_DX11
    #ifdef _DEBUG
        #define PDR_DEBUG
    #endif
#endif


#define MAXIMUM_LIGHTS_SUPPORTED (8)
#define MATERIAL_MAX_TEXTURES (8)

#endif