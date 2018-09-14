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

#endif

#endif

//----------------------------------------------------------------------------
GameApplication::GameApplication(const char* windowTitle, int xPosition,
    int yPosition, int width, int height, const eastl::array<float, 4>& clearColor)
:	mTitle(windowTitle), mXOrigin(xPosition), mYOrigin(yPosition), mWidth(width), 
	mHeight(height), mClearColor(clearColor), mAllowResize(true), mWindowID(0), 
	mLastTime(-1000.0), mAccumulatedTime(0.0), mFrameRate(0.0), mFramesPerSecond(0), 
	mTimer(0), mMaxTimer(30), mSystem(0), mRenderer(0)
{
	ProjectApplicationPath = Environment::GetAbsolutePath("") + '/';

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

	// note: max frame time to avoid spiral of death
	if (elapsedTime > 250)
	{
		mFramesPerSecond = 1000 / 250;
		return 250;
	}
	else if (elapsedTime > 0)
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
float GameApplication::GetLimitedDt()
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
	return dt;
}   // GetLimitedDt

//----------------------------------------------------------------------------
/*
	This is the base class which provides general purpose initialization for platform-dependent
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

	mProgramFactory = eastl::make_shared<HLSLProgramFactory>();

	mRenderer = eastl::shared_ptr<Renderer>(new Dx11Renderer(
		handle, mWidth, mHeight, D3D_FEATURE_LEVEL_11_0));

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
	//mSystem->SetWindowCaption(mTitle.c_str());
	//mOperatingSystem = new OperatingSystem(winversion);

#endif

	// The device is created in the platform-dependent code in the
	// GameApplication::Main function before OnInitialize is called.  Thus,
	// at this point the device state may be modified.
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
	mFileSystem->InsertDirectory(ProjectApplicationPath + "../../../Assets/");

	/*
		ResCache is created and initialized to 50MB and assocaited to a concreted mount point where 
		files will be taken. A resource cache can contain many different types of resources, such as 
		sounds, music, textures, and more. The resource cache needs to know how each one of these 
		files types is read and converted into something the game engine can use directly. The process
		of registering a loader associates a specific loader class with a file type.
	*/
	BaseResourceFile *mountPointFile = new ResourceMountPointFile(L"../../../Assets");
	mResCache = eastl::shared_ptr<ResCache>(new ResCache(50, mountPointFile));

	if (!mResCache->Init())
	{
		LogError("Failed to initialize resource cache!\
				 Are your paths set up correctly?");
		return false;
	}
	
	extern eastl::shared_ptr<BaseResourceLoader> CreateWAVResourceLoader();
	extern eastl::shared_ptr<BaseResourceLoader> CreateOGGResourceLoader();
	/*
	extern eastl::shared_ptr<BaseResourceLoader> CreateBMPResourceLoader();
	extern eastl::shared_ptr<BaseResourceLoader> CreateDDSResourceLoader();
	extern eastl::shared_ptr<BaseResourceLoader> CreateJPGResourceLoader();
	extern eastl::shared_ptr<BaseResourceLoader> CreatePCXResourceLoader();
	extern eastl::shared_ptr<BaseResourceLoader> CreatePNGResourceLoader();
	extern eastl::shared_ptr<BaseResourceLoader> CreatePPMResourceLoader();
	extern eastl::shared_ptr<BaseResourceLoader> CreatePSDResourceLoader();
	extern eastl::shared_ptr<BaseResourceLoader> CreateRGBResourceLoader();
	extern eastl::shared_ptr<BaseResourceLoader> CreateTGAResourceLoader();
	extern eastl::shared_ptr<BaseResourceLoader> CreateLMPResourceLoader();
	extern eastl::shared_ptr<BaseResourceLoader> CreateWALResourceLoader();
	extern eastl::shared_ptr<BaseResourceLoader> CreateWAL2ResourceLoader();

	extern eastl::shared_ptr<BaseResourceLoader> CreateB3DMeshResourceLoader();
	extern eastl::shared_ptr<BaseResourceLoader> Create3DSMeshResourceLoader();
	extern eastl::shared_ptr<BaseResourceLoader> CreateBSPMeshResourceLoader();
	extern eastl::shared_ptr<BaseResourceLoader> CreateLWOMeshResourceLoader();
	extern eastl::shared_ptr<BaseResourceLoader> CreateMD2MeshResourceLoader();
	extern eastl::shared_ptr<BaseResourceLoader> CreateMD3MeshResourceLoader();
	extern eastl::shared_ptr<BaseResourceLoader> CreateMS3DMeshResourceLoader();
	extern eastl::shared_ptr<BaseResourceLoader> CreateOCTResourceLoader();
	extern eastl::shared_ptr<BaseResourceLoader> CreateOgreMeshResourceLoader();
	extern eastl::shared_ptr<BaseResourceLoader> CreatePLYMeshResourceLoader();
	extern eastl::shared_ptr<BaseResourceLoader> CreateXMeshResourceLoader();
	*/
	extern eastl::shared_ptr<BaseResourceLoader> CreateXmlResourceLoader();
	extern eastl::shared_ptr<BaseResourceLoader> CreateImageResourceLoader();
	//extern eastl::shared_ptr<BaseResourceLoader> CreateScriptResourceLoader();

	//	Note - register these in order from least specific to most specific! 
	//	They get pushed onto a list.
	mResCache->RegisterLoader(CreateWAVResourceLoader());
	mResCache->RegisterLoader(CreateOGGResourceLoader());
	/*
	mResCache->RegisterLoader(CreateDDSResourceLoader());
	mResCache->RegisterLoader(CreateBMPResourceLoader());
	mResCache->RegisterLoader(CreateJPGResourceLoader());
	mResCache->RegisterLoader(CreatePCXResourceLoader());
	mResCache->RegisterLoader(CreatePNGResourceLoader());
	mResCache->RegisterLoader(CreatePPMResourceLoader());
	mResCache->RegisterLoader(CreatePSDResourceLoader());
	mResCache->RegisterLoader(CreateRGBResourceLoader());
	mResCache->RegisterLoader(CreateTGAResourceLoader());
	mResCache->RegisterLoader(CreateLMPResourceLoader());
	mResCache->RegisterLoader(CreateWALResourceLoader());
	mResCache->RegisterLoader(CreateWAL2ResourceLoader());

	mResCache->RegisterLoader(CreateB3DMeshResourceLoader());
	mResCache->RegisterLoader(Create3DSMeshResourceLoader());
	mResCache->RegisterLoader(CreateBSPMeshResourceLoader());
	mResCache->RegisterLoader(CreateLWOMeshResourceLoader());
	mResCache->RegisterLoader(CreateMD2MeshResourceLoader());
	mResCache->RegisterLoader(CreateMD3MeshResourceLoader());
	mResCache->RegisterLoader(CreateMS3DMeshResourceLoader());
	mResCache->RegisterLoader(CreateOCTResourceLoader());
	mResCache->RegisterLoader(CreateOgreMeshResourceLoader());
	mResCache->RegisterLoader(CreatePLYMeshResourceLoader());
	mResCache->RegisterLoader(CreateXMeshResourceLoader());
	*/
	mResCache->RegisterLoader(CreateXmlResourceLoader());
	mResCache->RegisterLoader(CreateImageResourceLoader());
	//mResCache->RegisterLoader(CreateScriptResourceLoader());

	mOption.Init(L"Config/PlayerOptions.xml");

	// Init the minimum managers so that user config exists, then
	// handle all command line options that do not need (or must
	// not have) other managers initialised:
	//InitUserConfig();

	//	Load the preinit file. This is within braces to create a scope and destroy 
	//	the resource once it's loaded.  We don't need to do anything with it, 
	//	we just need to load it.
	{
		//BaseResource resource(SCRIPT_PREINIT_FILE);
		// this actually loads the XML file from the zip file
		//eastl::shared_ptr<ResHandle> pResourceHandle = mResCache->GetHandle(&resource);
	}

	// The event manager should be created next so that subsystems can hook in as desired.
	// Discussed in Chapter 5, page 144
	mEventManager = eastl::shared_ptr<EventManager>(
		new EventManager("GameEngine EventMgr", true));
	if (!mEventManager)
	{
		LogError("Failed to create EventManager.");
		return false;
	}

	// Create the game logic and all the views that attach to the game logic
	CreateGameAndView();

	if (!GameLogic::mGame)
		return false;

	/*
		Preload most commonly used files. The resource cache provides the methods to
		preload those resources, based on file type.
	*/
	//mResCache->Preload(L"*.ogg", NULL);
	//mResCache->Preload(L"*.dds", NULL);
	//mResCache->Preload(L"*.jpg", NULL);

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

	//delete user_config;
	//delete unlock_manager;
	//delete level_manager;

	//DestroyWindow((HWND)mSystem->GetID());

	DestroyNetworkEventForwarder();

	//delete mBaseSocketManager;

	//delete mEventManager;
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

	Our main loop is hard-coded update in which every system updates onceper frame,
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
		// The default OnPreidle() clears the buffers. Allow the application
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

			const unsigned int elapsedTime = UpdateTime();

			// game logic execution
			OnUpdateGame(elapsedTime);

			// update all game views
			OnUpdateView(elapsedTime);

			// animate all game views
			OnAnimateView(Timer::GetTime());

			// Render the scene
			OnRender(elapsedTime);

			OnIdle();
		}

		OnTerminate();
	}
}

// animate all game views
void GameApplication::OnAnimateView(unsigned int time)
{
	eastl::list<eastl::shared_ptr<BaseGameView>>::iterator it = mGameViews.begin();
	for (; it != mGameViews.end(); ++it)
	{
		(*it)->OnAnimate(time);
	}
}

// update all game views
void GameApplication::OnUpdateView(unsigned int elapsedTime)
{
	eastl::list<eastl::shared_ptr<BaseGameView>>::iterator it = mGameViews.begin();
	for (; it != mGameViews.end(); ++it)
	{
		(*it)->OnUpdate(elapsedTime);
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
	// lear the buffers before rendering
	mRenderer->ClearBuffers();

	eastl::list<eastl::shared_ptr<BaseGameView>>::iterator it = mGameViews.begin();
	for (; it != mGameViews.end(); ++it)
	{
		(*it)->OnRender(Timer::GetTime(), (float)elapsedTime);
	}

	//Rendering for debug purpose
	GameLogic::Get()->RenderDiagnostics();

	mRenderer->DisplayColorBuffer(0);
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
// WndProc - the main message handler for the window class
//
// OnNcCreate - this is where you can set window data before it is created
// OnMove - called whenever the window moves; used to update members of g_App
// OnDeviceChange - called whenever you eject the CD-ROM.
// OnDisplayChange - called whenever the user changes the desktop settings
// OnPowerBroadcast - called whenever a power message forces a shutdown
// OnActivate - called whenever windows on the desktop change focus.
//
// Note: pUserContext added to comply with DirectX 9c - June 2005 Update
//
void GameApplication::ProcessMessage(int* hWndPtrAddress, int msg, int wParam, int lParam)
{
	mSystem->ProcessMessage(hWndPtrAddress, msg, wParam, lParam);
}

//=============================================================================
//	Initialises the minimum number of managers to get access to user_config.
//=============================================================================
/*
void GameApplication::InitUserConfig()
{
	UserConfig* userConfig = new UserConfig();     // needs file_manager
	const bool config_ok = user_config->LoadConfig();
	if (UserConfigParams::mLanguage.ToString() != L"system")
	{
#ifdef WIN32
		eastl::string s = eastl::string("LANGUAGE=") + UserConfigParams::mLanguage.c_str();
		_putenv(s.c_str());
#else
		setenv("LANGUAGE", UserConfigParams::mLanguage.c_str(), 1);
#endif
	}

	if (!LoadStrings("English"))
		GE_ERROR("Failed to load strings");

	//stk_config              = new STKConfig();      // in case of --stk-config
	// command line parameters
	user_config->PostLoadInit();
	if (!configOk || UserConfigParams::mAllPlayers.size() == 0)
	{
		userConfig->AddDefaultPlayer();
		userConfig->SaveConfig();
	}

}   // InitUserConfig
*/
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
    REGISTER_EVENT(EventDataMoveActor);
    REGISTER_EVENT(EventDataDestroyActor);
	REGISTER_EVENT(EventDataRequestNewActor);
	REGISTER_EVENT(EventDataNetworkPlayerActorAssignment);
}

void GameApplication::AddView(const eastl::shared_ptr<BaseGameView>& pView, ActorId actorId)
{
	// This makes sure that all views have a non-zero view id.
	int viewId = static_cast<int>(mGameViews.size());
	mGameViews.push_back(pView);
	pView->OnAttach(viewId, actorId);
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
		mNetworkEventForwarder.get(), &NetworkEventForwarder::ForwardEvent), EventDataPhysCollision::skEventType);

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
			mNetworkEventForwarder.get(), &NetworkEventForwarder::ForwardEvent), EventDataPhysCollision::skEventType);
        
		//delete mNetworkEventForwarder;
    }
}
