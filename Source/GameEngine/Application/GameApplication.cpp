// Geometric Tools, LLC
// Copyright (c) 1998-2014
// Distributed under the Boost Software License, Version 1.0.
// http://www.boost.org/LICENSE_1_0.txt
// http://www.geometrictools.com/License/Boost/LICENSE_1_0.txt
//
// File Version: 5.0.2 (2011/08/13)

#include "GameApplication.h"

#include "Core/Core.h"

#include "Game/GameLogic.h"
#include "Network/Network.h"

#include "Core/IO/ResourceCache.h"
#include "Core/IO/XmlResource.h"

#include "Graphic/UI/UserInterface.h"
#include "Game/View/HumanView.h"

// All event type headers
#include "Physic/PhysicEventListener.h"
#include "Core/Event/Event.h"
#include "Core/Event/EventManager.h"

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
GameApplication::GameApplication(const char* windowTitle, int xPosition,
    int yPosition, int width, int height, const eastl::array<float, 4>& clearColor)
:	mTitle(windowTitle), mXOrigin(xPosition), mYOrigin(yPosition), mWidth(width), 
	mHeight(height), mClearColor(clearColor), mAllowResize(true), mWindowID(0), 
	mFramesPerSecond(0), mTimer(0), mSystem(0), mRenderer(0)
{
	mIsRunning = false;
	mIsEditorRunning = false;

	mEventManager = NULL;
	mResCache = NULL;

	mNetworkEventForwarder = NULL;
	mBaseSocketManager = NULL;

	mQuitRequested = false;
	mQuitting = false;
}

//----------------------------------------------------------------------------
GameApplication::~GameApplication()
{

}

//-----------------------------------------------------------------------------
/*
	Time is a key element in the context of game engine update throughout the
	main modules (physics, graphics, game logic...)
*/
void GameApplication::InitTime() 
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
unsigned int GameApplication::UpdateTime()
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
/*
	This is the base class which provides general purpose initialization for platform-independent
	specifications. It creates a window for the application to draw to, set up graphics drivers,
	and perform generic component initialization. It is intended to be inherited so this method
	can be customized by derived class.

	The initialization is particularly sensitive to order, it is necessary to keep shutdown code
	in sync, or rather reverse sync, with the order of initialization. The method does:
	- Detects multiple instances of the application
	- Checks secondary storage space and memory
	- Calculates CPU speed
	- Loads game's resource cache
	- Loads strings which will be presented to the player
	- Creates game event manager
	- Initializes application's window
	- Creates game logic and game views
	- Sets the directory for save games and other temporary files
	- Preloads selected resources from the resource cache.
*/
bool GameApplication::OnInitialize()
{

#ifdef _WINDOWS_API_

	mSystem = eastl::shared_ptr<System>(new WindowsSystem(mWidth, mHeight));
	mSystem->SetEventListener(this);
	HWND handle = reinterpret_cast<HWND>(mSystem->GetID());

#ifdef USE_DX11

	mRenderer = eastl::shared_ptr<Renderer>(new Dx11Renderer(
		handle, mWidth, mHeight, D3D_FEATURE_LEVEL_11_0));
	mProgramFactory = eastl::make_shared<HLSLProgramFactory>();

	if (mRenderer == 0) return false; // initialization failed

#elif _OPENGL_

	mRenderer = eastl::shared_ptr<Renderer>(new WGLRenderer(handle));
	mProgramFactory = eastl::make_shared<GLSLProgramFactory>();

	if (mRenderer == 0) return false; // initialization failed

#endif

	if (mSystem == 0) return false; // initialization failed

	// Check for existing instance of the same window

	// Note - it can be really useful to debug network code to have
	// more than one instance of the game up at one time - so
	// feel free to comment these lines in or out as you wish!
	if (!mSystem->IsOnlyInstance(GetGameTitle().c_str()))
		return false;

	// get windows version and create OS operator
	eastl::string winversion;
	mSystem->GetSystemVersion(winversion);

#endif

	mRenderer->SetClearColor(mClearColor);

	eastl::wstring gametitle(GetGameTitle());
	//mSystem->SetWindowCaption(gametitle.c_str());
	//mRenderer->SetBackgroundColor(255, 20, 20, 200);
	//mRenderer->OnRestore();

	// register all events
	RegisterEngineEvents();
	RegisterGameEvents();

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
	
	extern eastl::shared_ptr<BaseResourceLoader> CreateWAVResourceLoader();
	extern eastl::shared_ptr<BaseResourceLoader> CreateOGGResourceLoader();
	extern eastl::shared_ptr<BaseResourceLoader> CreateMeshResourceLoader();
	extern eastl::shared_ptr<BaseResourceLoader> CreateXmlResourceLoader();
	extern eastl::shared_ptr<BaseResourceLoader> CreateImageResourceLoader();
	extern eastl::shared_ptr<BaseResourceLoader> CreatePhysicResourceLoader();

#ifdef USE_DX11
	extern eastl::shared_ptr<BaseResourceLoader> CreateHLSLShaderResourceLoader();
#elif _OPENGL_
	extern eastl::shared_ptr<BaseResourceLoader> CreateGLSLShaderResourceLoader();
#endif

	//	Note - register these in order from least specific to most specific! 
	//	They get pushed onto a list.
	mResCache->RegisterLoader(CreateWAVResourceLoader());
	mResCache->RegisterLoader(CreateOGGResourceLoader());
	mResCache->RegisterLoader(CreateMeshResourceLoader());
	mResCache->RegisterLoader(CreateXmlResourceLoader());
	mResCache->RegisterLoader(CreateImageResourceLoader());
	mResCache->RegisterLoader(CreatePhysicResourceLoader());


#ifdef USE_DX11
	mResCache->RegisterLoader(CreateHLSLShaderResourceLoader());
#elif _OPENGL_
	mResCache->RegisterLoader(CreateGLSLShaderResourceLoader());
#endif

	mOption.Init(L"Config/PlayerOptions.xml");

	// The event manager should be created next so that subsystems can hook in as desired.
	// Discussed in Chapter 5, page 144
	mEventManager = eastl::shared_ptr<EventManager>(
		new EventManager("GameEngine EventMgr", true));
	if (!mEventManager)
	{
		LogError("Failed to create EventManager.");
		return false;
	}

	// Create the game logic
	CreateGame();

	if (!GameLogic::mGame)
		return false;

	InitTime();

	mIsRunning = true;
	return true;
}

/*
	The game system should shut down or deallocate in the reverse order of which they were created.
	Each data structure should be traversed and freed, taking special care of dependencies between
	resources.
*/
void GameApplication::OnTerminate()
{
	//Destroy the logging system at the last possible moment
	//Logger::Destroy();
	mFileSystem->RemoveAllDirectories();

	// release all the game systems in reverse order from which they were created
	delete GameLogic::mGame;

	GameLogic::mGame = nullptr;

	DestroyNetworkEventForwarder();
}

//----------------------------------------------------------------------------
bool GameApplication::OnEvent(const Event& event)
{
	bool absorbed = false;

	eastl::list<eastl::shared_ptr<BaseGameView>>::reverse_iterator i = mGameViews.rbegin();
	for (; i != mGameViews.rend(); ++i)
	{
		if ((*i)->OnMsgProc(event))
		{
			absorbed = true;
			break;				// WARNING! This breaks out of the for loop.
		}
	}

	return absorbed;
}

//----------------------------------------------------------------------------
/*
	The game loop is the system that makes calls to update the objects in the scene
	and draw them to the screen. It is initialized and executed right after program
	startup and runs continuously until the program is terminated.

	Our main loop is hard-coded update in which every system updates once per frame,
	but it is inflexible since doesn't allow to update in different frequency. 
	Multithreaded architecture is the alternative to separate system in their own 
	thread, and resolve communications issues between them.
	Another solution is a hybrid technique called cooperative multitasking which
	consists on putting multiple systems in their own discrete execution modules but 
	throw away all the problems with concurrent execution. It keeps all of different 
	systems decoupled from each other whilst running "simultaneously".
*/
//----------------------------------------------------------------------------
void GameApplication::OnRun()
{
	if (OnInitialize())
	{
		OnPreidle();

		// performs the main loop
		while (IsRunning())
		{
			if (mQuitting)
				mSystem->OnClose();

			/*
				Handles the low level operations of the platform, process system events
				such as keyboard and mouse input
			*/
			mSystem->OnRun();

			const unsigned int elapsedTime = UpdateTime();

			// game logic execution
			OnUpdateGame(elapsedTime);

			// update all game views
			OnUpdateView(Timer::GetTime(), elapsedTime);

			// Render the scene
			OnRender(elapsedTime);

			OnIdle();
		}

		OnTerminate();
	}
}

// update all game views
void GameApplication::OnUpdateView(unsigned int timeMs, unsigned int elapsedTime)
{
	eastl::list<eastl::shared_ptr<BaseGameView>>::iterator it = mGameViews.begin();
	for (; it != mGameViews.end(); ++it)
	{
		(*it)->OnUpdate(timeMs, elapsedTime);
	}
}

/*
	Render function retrieves and calls the application's frame render, which will call
	the OnRender() methods of the views attached to the game every frame to perform 
	all the rendering calls for the scene, and it will also be called if the window needs 
	to be repainted. After the rendering is complete, the screen must be presented onto the
	monitor.
*/
void GameApplication::OnRender(unsigned int elapsedTime)
{
	// clear the buffers before rendering
	mRenderer->ClearBuffers();

	eastl::list<eastl::shared_ptr<BaseGameView>>::iterator it = mGameViews.begin();
	for (; it != mGameViews.end(); ++it)
	{
		(*it)->OnRender(Timer::GetTime(), (float)elapsedTime);
	}

	//Rendering for debug purpose
	GameLogic::Get()->RenderDiagnostics();

	mRenderer->DisplayColorBuffer(1);
}


/*
	This function controls the game logic and how the game state changes over each pass of the main loop.
	Control passes to the game logic's onupdate() method, which will update all the running game processes
	and send updates to all the game views attached to the game logic. This is the best location for
	application to handle updates to the scene, but is not intended to contain actual rendering calls, 
	which should instead be placed in the OnFrameRender callback.
*/
void GameApplication::OnUpdateGame(unsigned int elapsedTime)
{
	if (GameLogic::mGame)
	{
		BaseEventManager::Get()->Update(20); // allow event queue to process for up to 20 ms

		if (mBaseSocketManager)
			mBaseSocketManager->DoSelect(0);	// pause 0 microseconds

		GameLogic::mGame->OnUpdate((float)Timer::GetTime(), (float)elapsedTime);
	}
}

//----------------------------------------------------------
void GameApplication::ProcessMessage(int* hWndPtrAddress, int msg, int wParam, int lParam)
{
	mSystem->ProcessMessage(hWndPtrAddress, msg, wParam, lParam);
}

//----------------------------------------------------------------------------
void GameApplication::OnPreidle()
{
	// The default behavior is to clear the buffers before the window is
	// displayed for the first time.
	mRenderer->ClearBuffers();
}
//----------------------------------------------------------------------------
void GameApplication::OnIdle()
{
	// stub for derived classes
	// Temporarily pause execution and let other processes run.
	//mSystem->OnPause(100);
}

bool GameApplication::LoadGame(void)
{
    // Read the game options and see what the current game
    // needs to be - all of the game graphics are initialized by now, too...
	return GameLogic::mGame->LoadGame(mOption.mLevel.c_str());
}

void GameApplication::RegisterEngineEvents(void)
{
    REGISTER_EVENT(EventDataEnvironmentLoaded);
    REGISTER_EVENT(EventDataNewActor);
	REGISTER_EVENT(EventDataSyncActor);
    REGISTER_EVENT(EventDataDestroyActor);
	REGISTER_EVENT(EventDataRequestNewActor);
	REGISTER_EVENT(EventDataNetworkPlayerActorAssignment);
}

void GameApplication::AddView(const eastl::shared_ptr<BaseGameView>& pView)
{
	// This makes sure that all views have a non-zero view id.
	int viewId = static_cast<int>(mGameViews.size());
	mGameViews.push_back(pView);
	pView->OnRestore();
}

//
// GameEngineApp::GetHumanView()					- not described in the book
//
//   This function should accept a player number for split screen games
//
HumanView* GameApplication::GetHumanView()
{
	HumanView *pView = NULL;
	for (auto it = mGameViews.begin(); it != mGameViews.end(); ++it)
	{
		if ((*it)->GetType()==GV_HUMAN)
		{
			eastl::shared_ptr<BaseGameView> pBaseGameView(*it);
			pView = static_cast<HumanView *>(&*pBaseGameView);
			break;
		}
	}
	return pView;
}

void GameApplication::InitHumanViews(tinyxml2::XMLElement* pRoot)
{
	for (auto it = mGameViews.begin(); it != mGameViews.end(); ++it)
	{
		eastl::shared_ptr<BaseGameView> pView = *it;
		if (pView->GetType() == GV_HUMAN)
		{
			eastl::shared_ptr<HumanView> pHumanView =
				eastl::static_pointer_cast<HumanView, BaseGameView>(pView);
			pHumanView->LoadGame(pRoot);
		}
	}
}

//remove game view
void GameApplication::RemoveView(const eastl::shared_ptr<BaseGameView>& pView)
{
	mGameViews.remove(pView);
}

//remove game view
void GameApplication::RemoveView()
{
	mGameViews.pop_front();
}

// Added this to explicitly remove views from the game logic list.
void GameApplication::RemoveViews( )
{
	while (!mGameViews.empty())
		mGameViews.pop_front();
}

bool GameApplication::AttachAsClient()
{
	ClientSocketManager *pClient = 
		new ClientSocketManager(mOption.mGameHost, mOption.mListenPort);
	if (!pClient->Connect())
	{
		return false;
	}
	mBaseSocketManager.reset(pClient);
	CreateNetworkEventForwarder();

	return true;
}

// Any events that will be received from the server logic should be here!
void GameApplication::CreateNetworkEventForwarder(void)
{
    if (mNetworkEventForwarder != NULL)
    {
        LogError("Overwriting network event forwarder in App!");
        //delete mNetworkEventForwarder;
    }

    mNetworkEventForwarder.reset(new NetworkEventForwarder(0));

    BaseEventManager* pGlobalEventManager = EventManager::Get();
	pGlobalEventManager->AddListener(MakeDelegate(
		mNetworkEventForwarder.get(), &NetworkEventForwarder::ForwardEvent), EventDataRequestNewActor::skEventType);
	pGlobalEventManager->AddListener(MakeDelegate(
		mNetworkEventForwarder.get(), &NetworkEventForwarder::ForwardEvent), EventDataEnvironmentLoaded::skEventType);
	pGlobalEventManager->AddListener(MakeDelegate(
		mNetworkEventForwarder.get(), &NetworkEventForwarder::ForwardEvent), EventDataPhysTriggerEnter::skEventType);
	pGlobalEventManager->AddListener(MakeDelegate(
		mNetworkEventForwarder.get(), &NetworkEventForwarder::ForwardEvent), EventDataPhysTriggerLeave::skEventType);
	pGlobalEventManager->AddListener(MakeDelegate(
		mNetworkEventForwarder.get(), &NetworkEventForwarder::ForwardEvent), EventDataPhysCollision::skEventType);
	pGlobalEventManager->AddListener(MakeDelegate(
		mNetworkEventForwarder.get(), &NetworkEventForwarder::ForwardEvent), EventDataPhysSeparation::skEventType);
}

void GameApplication::DestroyNetworkEventForwarder(void)
{
    if (mNetworkEventForwarder)
    {
        BaseEventManager* pGlobalEventManager = EventManager::Get();
		pGlobalEventManager->RemoveListener(MakeDelegate(
			mNetworkEventForwarder.get(), &NetworkEventForwarder::ForwardEvent), EventDataRequestNewActor::skEventType);
		pGlobalEventManager->RemoveListener(MakeDelegate(
			mNetworkEventForwarder.get(), &NetworkEventForwarder::ForwardEvent), EventDataEnvironmentLoaded::skEventType);
		pGlobalEventManager->RemoveListener(MakeDelegate(
			mNetworkEventForwarder.get(), &NetworkEventForwarder::ForwardEvent), EventDataPhysTriggerEnter::skEventType);
		pGlobalEventManager->RemoveListener(MakeDelegate(
			mNetworkEventForwarder.get(), &NetworkEventForwarder::ForwardEvent), EventDataPhysTriggerLeave::skEventType);
		pGlobalEventManager->RemoveListener(MakeDelegate(
			mNetworkEventForwarder.get(), &NetworkEventForwarder::ForwardEvent), EventDataPhysCollision::skEventType);
		pGlobalEventManager->RemoveListener(MakeDelegate(
			mNetworkEventForwarder.get(), &NetworkEventForwarder::ForwardEvent), EventDataPhysSeparation::skEventType);

		//delete mNetworkEventForwarder;
    }
}
