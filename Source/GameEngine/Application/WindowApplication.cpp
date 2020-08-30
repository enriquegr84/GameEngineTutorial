// Geometric Tools, LLC
// Copyright (c) 1998-2014
// Distributed under the Boost Software License, Version 1.0.
// http://www.boost.org/LICENSE_1_0.txt
// http://www.geometrictools.com/License/Boost/LICENSE_1_0.txt
//
// File Version: 5.0.2 (2011/08/13)

#include "WindowApplication.h"

#include "Core/Core.h"

#include "Game/GameLogic.h"

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
	mFramesPerSecond(0), mTimer(0), mMaxTimer(30), mSystem(0), mRenderer(0)
{
	mEventManager = NULL;
	mResCache = NULL;

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

//----------------------------------------------------------------------------
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
	mFileSystem->InsertDirectory(ApplicationPath + "/../../Assets/");

	/*
		ResCache is created and initialized to 100MB and assocaited to a concreted mount point where
		files will be taken. A resource cache can contain many different types of resources, such as
		sounds, music, textures, and more. The resource cache needs to know how each one of these
		files types is read and converted into something the game engine can use directly. The process
		of registering a loader associates a specific loader class with a file type.
	*/

	BaseResourceFile *mountPointFile = new ResourceMountPointFile(L"/../../Assets");
	mResCache = eastl::shared_ptr<ResCache>(new ResCache(200, mountPointFile));

	if (!mResCache->Init())
	{
		LogError("Failed to initialize resource cache!\
				 Are your paths set up correctly?");
		return false;
	}

	extern eastl::shared_ptr<BaseResourceLoader> CreateMeshResourceLoader();
	extern eastl::shared_ptr<BaseResourceLoader> CreateXmlResourceLoader();
	extern eastl::shared_ptr<BaseResourceLoader> CreateImageResourceLoader();

#ifdef USE_DX11
	extern eastl::shared_ptr<BaseResourceLoader> CreateHLSLShaderResourceLoader();
#elif _OPENGL_
	extern eastl::shared_ptr<BaseResourceLoader> CreateGLSLShaderResourceLoader();
#endif

	//	Note - register these in order from least specific to most specific! 
	//	They get pushed onto a list.
	mResCache->RegisterLoader(CreateMeshResourceLoader());
	mResCache->RegisterLoader(CreateXmlResourceLoader());
	mResCache->RegisterLoader(CreateImageResourceLoader());


#ifdef USE_DX11
	mResCache->RegisterLoader(CreateHLSLShaderResourceLoader());
#elif _OPENGL_
	mResCache->RegisterLoader(CreateGLSLShaderResourceLoader());
#endif

	// The event manager should be created next so that subsystems can hook in as desired.
	mEventManager = eastl::shared_ptr<EventManager>(
		new EventManager("GameEngine EventMgr", true));
	if (!mEventManager)
	{
		LogError("Failed to create EventManager.");
		return false;
	}

	// Create the game logic
	GameLogic* game = new GameLogic();
	game->Init();

	if (!GameLogic::mGame)
		return false;

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