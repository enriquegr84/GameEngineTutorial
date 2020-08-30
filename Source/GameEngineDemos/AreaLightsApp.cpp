// Geometric Tools, LLC
// Copyright (c) 1998-2014
// Distributed under the Boost Software License, Version 1.0.
// http://www.boost.org/LICENSE_1_0.txt
// http://www.geometrictools.com/License/Boost/LICENSE_1_0.txt
//
// File Version: 5.0.2 (2011/08/13)

#include "Core/Logger/LogReporter.h"
#include "Core/IO/Environment.h"
#include "Core/OS/OS.h"

#include "Graphic/GraphicStd.h"

#include "AreaLightsApp.h"
#include "AreaLightsView.h"

//----------------------------------------------------------------------------
int main(int numArguments, char* arguments[])
{
#if defined(_DEBUG)
	LogReporter reporter(
		"",
		Logger::Listener::LISTEN_FOR_NOTHING,
		Logger::Listener::LISTEN_FOR_NOTHING,
		Logger::Listener::LISTEN_FOR_NOTHING,
		Logger::Listener::LISTEN_FOR_ALL);
#endif

	// Application entry point. It is the startup function used for initialization
#ifndef __APPLE__
	Application::ApplicationPath = Environment::GetAbsolutePath("");
#else
	// Mac OS X Lion returns NULL on any getenv call (such as the one in
	// Environment::GetVariable).  This hack works around the problem.
	if (system("cp ~/.MacOSX/apppath.txt tempapppath.txt") == 0)
	{
		std::ifstream inFile("tempapppath.txt");
		if (inFile)
		{
			getline(inFile, Application::AppPath);
			inFile.close();
			system("rm tempapppath.txt");
		}
	}
#endif
	if (Application::ApplicationPath == "")
	{
		LogError("Unknown application path");
		return INT_MAX;
	}
	Application::ApplicationPath += "/";

	// Initialization
	AreaLightsApp* areaLightsApp = new AreaLightsApp();
	Application::App = areaLightsApp;

	int exitCode = -1;
	try
	{
		Application::App->OnRun();
		exitCode = 0;
	}
	catch (...)
	{
		// Catch all exceptions – dangerous!!!  
		// Respond (perhaps only partially) to the exception, then  
		// re-throw to pass the exception to some other handler  
		// throw;
		LogError("An error happend during execution.\n");
	}
	//delete0(Application::TheCommand);

	// Termination
	delete Application::App;

	return exitCode;
}

//----------------------------------------------------------------------------
AreaLightsApp::AreaLightsApp()
:	WindowApplication("AreaLights", 0, 0, 800, 600, { 0.392f, 0.584f, 0.929f, 1.0f })
{

}

//----------------------------------------------------------------------------
AreaLightsApp::~AreaLightsApp()
{

}

//----------------------------------------------------------------------------
bool AreaLightsApp::CreateScene()
{
	eastl::shared_ptr<HumanView> pHumanView(new AreaLightsHumanView());
	AddView(pHumanView);

	tinyxml2::XMLElement* pRoot = NULL;
	pHumanView->LoadGame(pRoot);

	return true;
}

//----------------------------------------------------------------------------
bool AreaLightsApp::OnInitialize()
{
    if (!WindowApplication::OnInitialize())
    {
        return false;
    }
	
	//Initialize timer
	InitTime();

	CreateScene();

    return true;
}

//----------------------------------------------------------------------------
void AreaLightsApp::OnIdle()
{

}

//----------------------------------------------------------------------------
void AreaLightsApp::OnTerminate()
{
	WindowApplication::OnTerminate();
}

//----------------------------------------------------------------------------
void AreaLightsApp::OnRun()
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
void AreaLightsApp::OnUpdateView(unsigned int timeMs, unsigned int elapsedTime)
{
	eastl::list<eastl::shared_ptr<BaseGameView>>::iterator it = mGameViews.begin();
	for (; it != mGameViews.end(); ++it)
	{
		(*it)->OnUpdate(timeMs, elapsedTime);
	}
}

void AreaLightsApp::OnRender(unsigned int elapsedTime)
{
	// clear the buffers before rendering
	mRenderer->ClearBuffers();

	eastl::list<eastl::shared_ptr<BaseGameView>>::iterator it = mGameViews.begin();
	for (; it != mGameViews.end(); ++it)
	{
		(*it)->OnRender(Timer::GetTime(), (float)elapsedTime);
	}

	mRenderer->DisplayColorBuffer(0);
}

//----------------------------------------------------------------------------
bool AreaLightsApp::OnResize(int width, int height)
{
	return true;
}

//----------------------------------------------------------------------------
bool AreaLightsApp::OnEvent(const Event& event)
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

HumanView* AreaLightsApp::GetHumanView()
{
	HumanView *pView = NULL;
	for (auto it = mGameViews.begin(); it != mGameViews.end(); ++it)
	{
		if ((*it)->GetType() == GV_HUMAN)
		{
			eastl::shared_ptr<BaseGameView> pBaseGameView(*it);
			pView = static_cast<HumanView *>(&*pBaseGameView);
			break;
		}
	}
	return pView;
}

void AreaLightsApp::AddView(const eastl::shared_ptr<BaseGameView>& pView)
{
	// This makes sure that all views have a non-zero view id.
	int viewId = static_cast<int>(mGameViews.size());
	mGameViews.push_back(pView);
	pView->OnRestore();
}

//remove game view
void AreaLightsApp::RemoveView(const eastl::shared_ptr<BaseGameView>& pView)
{
	mGameViews.remove(pView);
}

//remove game view
void AreaLightsApp::RemoveView()
{
	mGameViews.pop_front();
}

// Added this to explicitly remove views from the game logic list.
void AreaLightsApp::RemoveViews()
{
	while (!mGameViews.empty())
		mGameViews.pop_front();
}
