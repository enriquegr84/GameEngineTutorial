// Geometric Tools, LLC
// Copyright (c) 1998-2014
// Distributed under the Boost Software License, Version 1.0.
// http://www.boost.org/LICENSE_1_0.txt
// http://www.geometrictools.com/License/Boost/LICENSE_1_0.txt
//
// File Version: 5.0.2 (2011/08/13)

#include "WindowApplication.h"

#include "Core/Core.h"

#ifdef _WINDOWS_API_

#include "System/WindowsSystem.h"

#ifdef USE_DX11

#include "Graphic/Renderer/DirectX11/Dx11Renderer.h"
#include "Graphic/Renderer/DirectX11/HLSL/HLSLProgramFactory.h"

#elif _OPENGL_

#include "Graphic/Renderer/OpenGL4/WGL/WGLRenderer.h"
#include "Graphic/Renderer/OpenGL4/Shader/GLSLProgramFactory.h"

#endif

#endif

//----------------------------------------------------------------------------
WindowApplication::WindowApplication(const char* windowTitle, int xPosition,
    int yPosition, int width, int height, const eastl::array<float, 4>& clearColor)
:	mTitle(windowTitle), mXOrigin(xPosition), mYOrigin(yPosition), mWidth(width), 
	mHeight(height), mClearColor(clearColor), mAllowResize(true), mWindowID(0), 
	mLastTime(-1000.0), mAccumulatedTime(0.0), mFrameRate(0.0), mFramesPerSecond(0), 
	mTimer(0), mMaxTimer(30), mSystem(0), mRenderer(0)
{
	ProjectApplicationPath = Environment::GetAbsolutePath("") + '/';

	mQuitRequested = false;
	mQuitting = false;
}

//----------------------------------------------------------------------------
WindowApplication::~WindowApplication()
{

}

//-----------------------------------------------------------------------------
/*
	Time is a key element in the context of game engine update throughout the
	main modules (physics, graphics, game logic...)
*/
void WindowApplication::InitTime() 
{
	// Called to get the real time
	Timer::InitTimer();
	mTimer = (unsigned int)Timer::GetRealTime();
}

//----------------------------------------------------------------------------
/*
	Update time related information: the time elpased since the clock started and the 
	time elpased since the last iteration of the game loop. The latter applies to what
	is known as game's frame rate. An iteration of the game loop counts as one frame,
	and the frame rate is the number of frames executed in a second
*/
unsigned int WindowApplication::UpdateTime()
{
	const unsigned int currentTime = Timer::GetRealTime();
	const unsigned int elapsedTime = currentTime - mTimer;
	mTimer = currentTime;

	if (elapsedTime > 0)
		mFramesPerSecond = 1000 / elapsedTime;

	return elapsedTime;
}
//----------------------------------------------------------------------------


//-----------------------------------------------------------------------------
/*
	Returns the current dt, which guarantees a limited frame rate. If dt is
	too low (the frame rate too high), the process will sleep to reach the
	maxium frame rate.
 */
float WindowApplication::GetLimitedDt()
{
	unsigned int currTime = mTimer;
	unsigned int prevTime = currTime;

	float dt;  // needed outside of the while loop
	while (1)
	{
		currTime = Timer::GetRealTime();
		dt = (float)(currTime - prevTime);

		// don't allow the game to run slower than a certain amount.
		// when the computer can't keep it up, slow down the shown time instead
		static const float maxElapsedTime = 3.0f*1.0f / 60.0f*1000.0f; /* time 3 internal substeps take */
		if (dt > maxElapsedTime) dt = maxElapsedTime;

		// Throttle fps if more than maximum, which can reduce
		// the noise the fan on a graphics card makes.
		// When in menus, reduce FPS much, it's not necessary to push to the maximum for plain menus
		const int maxFPS = 60;//(StateManager::get()->throttleFPS() ? 35 : UserConfigParams::mMaxFPS);
		const int currentFPS = (int)(1000.0f / dt);
		if (currentFPS > maxFPS)// && !ProfileWorld::isProfileMode())
		{
			int waitTime = 1000 / maxFPS - 1000 / currentFPS;
			if (waitTime < 1) waitTime = 1;

			mSystem->OnPause(waitTime);
		}
		else break;
	}
	dt *= 0.001f;
	return dt;
}   // GetLimitedDt

	//----------------------------------------------------------------------------
	/*
	This is the base class which provides general purpose initialization for platform-dependent
	specifications. It creates a window for the application to draw to, set up graphics drivers,
	and perform generic component initialization. It is intended to be inherited so this method
	can be customized by derived class.
	*/
bool WindowApplication::OnInitialize()
{

#ifdef _WINDOWS_API_

	mSystem = eastl::shared_ptr<System>(new WindowsSystem(mWidth, mHeight));
	mSystem->SetEventListener(this);
	HWND handle = reinterpret_cast<HWND>(mSystem->GetID());

#ifdef USE_DX11

	mRenderer = eastl::shared_ptr<Renderer>(new Dx11Renderer(
		handle, mWidth, mHeight, D3D_FEATURE_LEVEL_11_0));
	mProgramFactory = eastl::make_shared<HLSLProgramFactory>();

#elif _OPENGL_

	mRenderer = eastl::shared_ptr<Renderer>(new WGLRenderer(handle));
	mProgramFactory = eastl::make_shared<GLSLProgramFactory>();

#endif

	if (mSystem == 0) return false; // initialization failed

	// get windows version and create OS operator
	eastl::string winversion;
	mSystem->GetSystemVersion(winversion);
	//mSystem->SetWindowCaption(mTitle.c_str());
	//mOperatingSystem = new OperatingSystem(winversion);

#endif

	// The device is created in the platform-dependent code in the
	// WindowApplication::Main function before OnInitialize is called.  Thus,
	// at this point the device state may be modified.
	mRenderer->SetClearColor(mClearColor);

	mFileSystem = eastl::shared_ptr<FileSystem>(new FileSystem());
	// Always check the application directory.
	mFileSystem->InsertDirectory(Application::ApplicationPath);
	mFileSystem->InsertDirectory(ProjectApplicationPath + "../../../Assets/");

	InitTime();

	mIsRunning = true;

	return true;
}

//----------------------------------------------------------------------------
void WindowApplication::OnTerminate()
{
	//Destroy the logging system at the last possible moment
	//Logger::Destroy();
	mFileSystem->RemoveAllDirectories();
}

//----------------------------------------------------------------------------
bool WindowApplication::OnEvent(const Event& event)
{
	bool result = 0;

	return result;
}

//----------------------------------------------------------------------------
/*
	The game loop is the system that makes calls to update the objects in the scene
	and draw them to the screen. It is initialized and executed right after program
	startup and runs continuously until the program is terminated.
	Game loop illustration page 195
*/
//----------------------------------------------------------------------------
void WindowApplication::OnRun()
{
	if (OnInitialize())
	{
		// The default OnPreidle() clears the buffers.  Allow the application
		// to fill them before the window is shown and before the event loop
		// starts.
		OnPreidle();

		// performs the main loop
		while (IsRunning())
		{
			/*
			Handles the low level operations of the platform, process system events
			such as keyboard and mouse input
			*/
			if (mQuitting)
				mSystem->OnClose();

			mSystem->OnRun();
			/*
			int fps = mRenderer->GetFPS();

			eastl::wstring str = L"Game Engine - Demos [";
			str += mRenderer->GetName();
			str += L"] FPS: ";
			str += (signed int)mRenderer->GetFPS();

			mSystem->SetWindowCaption(str.c_str());
			*/
			UpdateTime();

			OnIdle();
		}

		OnTerminate();
	}
}
//----------------------------------------------------------------------------
void WindowApplication::OnPreidle()
{
	// The default behavior is to clear the buffers before the window is
	// displayed for the first time.
	mRenderer->ClearBuffers();
}
//----------------------------------------------------------------------------
void WindowApplication::OnIdle()
{
	// stub for derived classes
}