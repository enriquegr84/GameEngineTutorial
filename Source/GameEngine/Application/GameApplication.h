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
#include "Graphic/Renderer/Renderer.h"

//	Window abstracts the platform-dependent implementations
class GameApplication : public Application, public EventListener
{
protected:
    // Abstract base class.
    GameApplication (const char* windowTitle, int xPosition,
        int yPosition, int width, int height, const eastl::array<float, 4>& clearColor);
public:
    virtual ~GameApplication ();
	
	inline float GetAspectRatio() const;

	// Main loop processing
	virtual bool OnInitialize();
	virtual void OnTerminate();
	virtual void OnRun();

	virtual void OnPreidle();
	virtual void OnIdle();

	void AbortGame() { mQuitting = true; }
	bool IsRunning() { return mIsRunning; }
	void SetQuitting(bool quitting) { mQuitting = quitting; }
	
	// Event callbacks.
	virtual bool OnEvent(const Event& ev);

	// Game specific.

	// Game Application Data
	// You must define these in an inherited
	// class - see TeapotWarsApp for an example
	virtual eastl::wstring GetGameTitle() = 0;
	virtual eastl::wstring GetGameAppDirectory() = 0;
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

protected:

	/*
	Function called from the application in order to start the timer. The
	child class use the timer depending on the platform.
	*/
	void InitTime();

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

	eastl::shared_ptr<BaseGameLogic> GetGameLogic(void) const { return mGame; }

	HumanView* GetHumanView();	// it was convenient to grab the HumanView attached to the game.

	void InitHumanViews(XMLElement* pRoot);
	const eastl::list<eastl::shared_ptr<BaseGameView>>& GetGameViews() { return mGameViews; }
	void AddView(const eastl::shared_ptr<BaseGameView>& pView, ActorId actorId = INVALID_ACTOR_ID);
	void RemoveView(const eastl::shared_ptr<BaseGameView>& pView);
	void RemoveViews();
	void RemoveView();

	// You must define these functions to initialize your game.
	virtual eastl::shared_ptr<BaseGameLogic> CreateGameAndView() = 0;
	virtual bool LoadGame(void);

	bool IsEditorRunning() { return mIsEditorRunning; }

	bool AttachAsClient();

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
	eastl::list<eastl::shared_ptr<BaseGameView>> mGameViews;

	//main services

	// File and Resource System
	eastl::shared_ptr<ResCache> mResCache;

	// Event manager
	eastl::shared_ptr<EventManager> mEventManager;

	// Socket manager - could be server or client
	eastl::shared_ptr<BaseSocketManager> mBaseSocketManager;
	eastl::shared_ptr<NetworkEventForwarder> mNetworkEventForwarder;

	eastl::shared_ptr<ProgramFactory> mProgramFactory;
	eastl::shared_ptr<Renderer> mRenderer;
	eastl::shared_ptr<System> mSystem;

	// Game specific
	eastl::shared_ptr<BaseGameLogic> mGame;
	struct GameOption mOption;

private:

	void RegisterEngineEvents(void);
};

inline float GameApplication::GetAspectRatio() const
{
	return (float)mWidth / (float)mHeight;
}

#endif
