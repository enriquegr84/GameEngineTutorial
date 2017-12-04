// Geometric Tools, LLC
// Copyright (c) 1998-2014
// Distributed under the Boost Software License, Version 1.0.
// http://www.boost.org/LICENSE_1_0.txt
// http://www.geometrictools.com/License/Boost/LICENSE_1_0.txt
//
// File Version: 5.0.2 (2010/05/06)

#ifndef GAMEAPPLICATION_H
#define GAMEAPPLICATION_H

#include "Application.h"

#include "Game/Game.h"
#include "Game/GameOption.h"

#include "System/System.h"
#include "Core/IO/FileSystem.h"
#include "Core/IO/ResourceCache.h"

#include "Graphic/Renderer/Renderer.h"

class BaseSocketManager;
class NetworkEventForwarder;

/*
	Game application layer handles operating system-specific tasks, including interfacing
	with the hardware and operating system, handling the application life cycle including
	initialization, managing acces to localized strings, and initializing the game logic.
	This class is meant to be inherited by a game-specific application class that will
	extend it and define some game specifics, also implementations for creating the game
	logic, game views and loading the initial state of the game.
*/
class GameApplication : public Application, public EventListener
{
protected:
    // Abstract base class.
    GameApplication (const char* windowTitle, int xPosition, int yPosition, 
		int width, int height, const eastl::array<float, 4>& clearColor);
public:
    virtual ~GameApplication ();
	
	inline float GetAspectRatio() const;

	/*
	Initializing games involves performing setup tasks in a particular order.
	Every platform will be different but they follow the same steps:
	- Check system resources: HDD, memory, input/output devices.
	- Check CPU speed.
	- Initialize memory cache
	- Create window
	- Initialize aduio system
	- Load player's game options and saved game files
	- Create drawing surface
	- Perform game system initializations: Physics, AI and so on
	*/
	virtual bool OnInitialize();
	virtual void OnTerminate();
	virtual void OnRun();

	virtual void OnPreidle();
	virtual void OnIdle();

	// Game Application Data
	// You must define these in an inherited
	// class - see TeapotWarsApp for an example
	virtual eastl::wstring GetGameTitle() = 0;
	virtual eastl::wstring GetGameAppDirectory() = 0;

	int GetFPS() { return mFramesPerSecond; }
	void AbortGame() { mQuitting = true; }
	bool IsRunning() { return mIsRunning; }
	void SetQuitting(bool quitting) { mQuitting = quitting; }

	HumanView* GetHumanView();	// it was convenient to grab the HumanView attached to the game.

	void InitHumanViews(XMLElement* pRoot);
	const eastl::list<eastl::shared_ptr<BaseGameView>>& GetGameViews() { return mGameViews; }
	
	virtual void AddView(const eastl::shared_ptr<BaseGameView>& pView, ActorId actorId = INVALID_ACTOR_ID);
	virtual void RemoveView(const eastl::shared_ptr<BaseGameView>& pView);
	virtual void RemoveViews();
	virtual void RemoveView();

	// You must define these functions to initialize your game.
	virtual void CreateGameAndView() = 0;
	virtual bool LoadGame(void);

	bool IsEditorRunning() { return mIsEditorRunning; }

	bool AttachAsClient();

	/*
		The class actas as a container for other important members that manage
		the application layer:
		- Game logic implementation
		- Data structure that holds game options (usually XML file)
		- Resource cache, responsible for loading textures, meshes, sounds...
		- Event manager, which allows different game subsystems to communicate
		- Network communications manager
	*/

	// File and Resource System
	eastl::shared_ptr<ResCache> mResCache;

	// Event manager
	eastl::shared_ptr<EventManager> mEventManager;

	// Socket manager - could be server or client
	eastl::shared_ptr<BaseSocketManager> mBaseSocketManager;
	eastl::shared_ptr<NetworkEventForwarder> mNetworkEventForwarder;

	eastl::shared_ptr<ProgramFactory> mProgramFactory;
	eastl::shared_ptr<FileSystem> mFileSystem;
	eastl::shared_ptr<Renderer> mRenderer;
	eastl::shared_ptr<System> mSystem;

	// Game specific
	struct GameOption mOption;

protected:

	/*
	Function called from the application in order to start the timer. The
	child class use the timer depending on the platform.
	*/
	void InitTime();

	// process message
	void ProcessMessage(int* hWndPtrAddress, int msg, int wParam, int lParam);

	/*
	Function called in each tick. It must update the deltaTime and time
	attributes, so it will reflects the pass of the time. Particularly, the
	variable accumulated time has in milliseconds the difference time since 
	updateTime has been called last time, and time has the live time of the 
	application.
	*/
	void UpdateTime();
	void UpdateFrameCount();
	float GetLimitedDt();

	// Event callbacks.
	virtual bool OnEvent(const Event& ev);

	/*
	Reimplemented method by the child class and performs the logic associated
	to this state. It is called before drawing the scene to update game logic.
	@param elapsedTime Running time in milliseconds since the last updating of
	the logic
	*/
	void OnUpdateGame(unsigned int elapsedTime);
	void OnUpdateView(unsigned int elapsedTime);
	void OnAnimateView(unsigned int time);

	/*
	Reimplemented method by the child classes which draws the scene
	@param elapsedTime Running time in milliseconds since the last drawing
	*/
	void OnRender(unsigned int elapsedTime);

	virtual void RegisterGameEvents(void) {}
	virtual void CreateNetworkEventForwarder(void);
	virtual void DestroyNetworkEventForwarder(void);

	double mLastTime, mAccumulatedTime, mFrameRate;
	int mFrameCount, mFramesPerSecond, mTimer, mMaxTimer;

    // Window parameters (from the constructor).
    eastl::wstring mTitle;
    int mXOrigin, mYOrigin, mWidth, mHeight;

	eastl::array<float, 4> mClearColor;
    bool mAllowResize;

	//HINSTANCE m_hInstance;	//	the module instance
	bool mWindowedMode;			//	true if the app is windowed, false if fullscreen
	bool mIsRunning;			//	true if everything is initialized and the game is in the main loop
	bool mQuitRequested;		//	true if the app should run the exit sequence
	bool mQuitting;				//	true if the app is running the exit sequence

	bool mIsEditorRunning;		// true if the game editor is running

    // The window ID is platform-specific but hidden by an 'int' opaque handle.
    int mWindowID;

	// views that are attached to our game
	GameViewList mGameViews;

private:

	void RegisterEngineEvents(void);
};

inline float GameApplication::GetAspectRatio() const
{
	return (float)mWidth / (float)mHeight;
}

#endif
