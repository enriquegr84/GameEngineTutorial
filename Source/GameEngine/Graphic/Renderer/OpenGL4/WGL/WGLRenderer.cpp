// David Eberly, Geometric Tools, Redmond WA 98052
// Copyright (c) 1998-2018
// Distributed under the Boost Software License, Version 1.0.
// http://www.boost.org/LICENSE_1_0.txt
// http://www.geometrictools.com/License/Boost/LICENSE_1_0.txt
// File Version: 3.0.4 (2018/06/18)


#include "WGLRenderer.h"

extern "C"
{
    extern int __stdcall wglSwapIntervalEXT(int interval);
    extern int __stdcall wglGetSwapIntervalEXT(void);
}
extern void InitializeWGL();

WGLRenderer::~WGLRenderer()
{
    Terminate();
}

WGLRenderer::WGLRenderer(HWND handle, int requiredMajor, int requiredMinor)
    :
    GL4Renderer(),
    mHandle(handle),
    mDevice(nullptr),
    mImmediate(nullptr),
    mComputeWindowAtom(0)
{
    Initialize(requiredMajor, requiredMinor);
}

bool WGLRenderer::IsActive() const
{
    return mImmediate == wglGetCurrentContext();
}

void WGLRenderer::MakeActive()
{
    if (mImmediate != wglGetCurrentContext())
    {
        wglMakeCurrent(mDevice, mImmediate);
    }
}

void WGLRenderer::DisplayColorBuffer(unsigned int syncInterval)
{
    wglSwapIntervalEXT(syncInterval > 0 ? 1 : 0);
    SwapBuffers(mDevice);
}

bool WGLRenderer::Initialize(int requiredMajor, int requiredMinor)
{
    if (!mHandle)
    {
        LogError("Invalid window handle.");
        return false;
    }

    mDevice = GetDC(mHandle);
    if (!mDevice)
    {
        LogError("Invalid device context.");
        mHandle = nullptr;
        return false;
    }

    RECT rect;
    BOOL result = GetClientRect(mHandle, &rect); (void)result;
    mScreenSize[0] = static_cast<unsigned int>(rect.right - rect.left);
    mScreenSize[1] = static_cast<unsigned int>(rect.bottom - rect.top);

    // Select the format for the drawing surface.
    PIXELFORMATDESCRIPTOR pfd;
    memset(&pfd, 0, sizeof(PIXELFORMATDESCRIPTOR));
    pfd.nSize = sizeof(PIXELFORMATDESCRIPTOR);
    pfd.nVersion = 1;
    pfd.dwFlags =
        PFD_DRAW_TO_WINDOW |
        PFD_SUPPORT_OPENGL |
        PFD_GENERIC_ACCELERATED |
        PFD_DOUBLEBUFFER;

    // Create an R8G8B8A8 buffer.
    pfd.iPixelType = PFD_TYPE_RGBA;
    pfd.cColorBits = 32;

    // Create a depth-stencil buffer.
	pfd.cDepthBits = 24;
	pfd.cStencilBits = 8;

    // Set the pixel format for the rendering context.
    int pixelFormat = ChoosePixelFormat(mDevice, &pfd);
    if (pixelFormat == 0)
    {
        LogError("ChoosePixelFormat failed.");
        ReleaseDC(mHandle, mDevice);
		mScreenSize[0] = 0;
		mScreenSize[1] = 0;
        mDevice = nullptr;
        mHandle = nullptr;
        return false;
    }

    if (!SetPixelFormat(mDevice, pixelFormat, &pfd))
    {
        LogError("SetPixelFormat failed.");
        ReleaseDC(mHandle, mDevice);
		mScreenSize[0] = 0;
		mScreenSize[1] = 0;
        mDevice = nullptr;
        mHandle = nullptr;
        return false;
    }

    // Create an OpenGL context.
    mImmediate = wglCreateContext(mDevice);
    if (!mImmediate)
    {
        LogError("wglCreateContext failed.");
        ReleaseDC(mHandle, mDevice);
		mScreenSize[0] = 0;
		mScreenSize[1] = 0;
        mDevice = nullptr;
        mHandle = nullptr;
        return false;
    }

    // Activate the context.
    if (!wglMakeCurrent(mDevice, mImmediate))
    {
        LogError("wglMakeCurrent failed.");
        wglDeleteContext(mImmediate);
        ReleaseDC(mHandle, mDevice);
		mScreenSize[0] = 0;
		mScreenSize[1] = 0;
        mImmediate = nullptr;
        mDevice = nullptr;
        mHandle = nullptr;
        return false;
    }

    // Get the function pointers for WGL.
    InitializeWGL();

    // Get the function pointers for OpenGL; initialize the viewport,
    // default global state, and default font.
    return GL4Renderer::Initialize(requiredMajor, requiredMinor);
}

void WGLRenderer::Terminate()
{
    GL4Renderer::Terminate();

    if (mHandle && mDevice && mImmediate)
    {
        wglMakeCurrent(mDevice, nullptr);
        wglDeleteContext(mImmediate);
        ReleaseDC(mHandle, mDevice);
    }

    if (mComputeWindowAtom)
    {
        DestroyWindow(mHandle);
        UnregisterClass(mComputeWindowClass.c_str(), 0);
    }
}
