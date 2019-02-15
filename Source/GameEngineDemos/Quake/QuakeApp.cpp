//========================================================================
// Quake.cpp : source file for the sample game
//
// Part of the GameEngine Application
//
// GameEngine is the sample application that encapsulates much of the source code
// discussed in "Game Coding Complete - 4th Edition" by Mike McShaffry and David
// "Rez" Graham, published by Charles River Media. 
// ISBN-10: 1133776574 | ISBN-13: 978-1133776574
//
// If this source code has found it's way to you, and you think it has helped you
// in any way, do the authors a favor and buy a new copy of the book - there are 
// detailed explanations in it that compliment this code well. Buy a copy at Amazon.com
// by clicking here: 
//    http://www.amazon.com/gp/product/1133776574/ref=olp_product_details?ie=UTF8&me=&seller=
//
// There's a companion web site at http://www.mcshaffry.com/GameCode/
// 
// The source code is managed and maintained through Google Code: 
//    http://code.google.com/p/GameEngine/
//
// (c) Copyright 2012 Michael L. McShaffry and David Graham
//
// This program is free software; you can redistribute it and/or
// modify it under the terms of the GNU Lesser GPL v3
// as published by the Free Software Foundation.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See 
// http://www.gnu.org/licenses/lgpl-3.0.txt for more details.
//
// You should have received a copy of the GNU Lesser GPL v3
// along with this program; if not, write to the Free Software
// Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
//
//========================================================================

#include "QuakeStd.h"

#include "Game/Game.h"

#include "Physic/Physic.h"
#include "Physic/PhysicEventListener.h"

#include "Core/Logger/LogReporter.h"
#include "Core/IO/Environment.h"

#include "Core/Event/Event.h"
#include "Core/Event/EventManager.h"

#include "Quake.h"
#include "QuakeView.h"
#include "QuakeNetwork.h"
#include "QuakeEvents.h"
#include "QuakeResources.h"

#include "QuakeApp.h"

//========================================================================
// main - Defines the entry point to the application, the GameApplication handles the 
// initialization. This allows the GameEngine function to live in a library, 
// separating the game engine from game specific code, in this case Quake.
//========================================================================

int main()
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
	// The application layer depends on the directory structure that ships with the 
	// libraries. You need to create the APP_PATH environment variable in order for 
	// the applications to find various data files.
#ifndef __APPLE__
	Application::ApplicationPath = Environment::GetAbsolutePath("../../../");
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
		LogError("Please set the APP_PATH environment variable.\n");
		return INT_MAX;
	}
	Application::ApplicationPath += "/";

	// Initialization
	QuakeApp* quakeApp = new QuakeApp();
	Application::App = quakeApp;

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

//========================================================================
//
// QuakeApp Implementation     - Chapter 21, page 722
//
//========================================================================

//----------------------------------------------------------------------------
QuakeApp::QuakeApp()
	: GameApplication("Quake", 0, 0, 800, 600, { 0.392f, 0.584f, 0.929f, 1.0f })
{
}

//----------------------------------------------------------------------------
QuakeApp::~QuakeApp()
{
}

//
// QuakeApp::CreateGameAndView
//
void QuakeApp::CreateGameAndView()
{
	QuakeLogic* game = new QuakeLogic();
	game->Init();
}

//
// QuakeLogic::AddView
//
void QuakeApp::AddView(const eastl::shared_ptr<BaseGameView>& pView, ActorId actor)
{
	GameApplication::AddView(pView, actor);
	GameLogic::Get()->UpdateViewType(pView, true);
}

//
// QuakeLogic::RemoveView
//
void QuakeApp::RemoveView(const eastl::shared_ptr<BaseGameView>& pView)
{
	GameApplication::RemoveView(pView);
	GameLogic::Get()->UpdateViewType(pView, false);
}

//remove game view
void QuakeApp::RemoveView()
{
	GameLogic::Get()->UpdateViewType(mGameViews.front(), false);
	GameApplication::RemoveView();
}

// Added this to explicitly remove views from the game logic list.
void QuakeApp::RemoveViews()
{
	GameLogic::Get()->ResetViewType();
	while (!mGameViews.empty())
		mGameViews.pop_front();
}

/*
HICON QuakeApp::VGetIcon()
{
	return LoadIcon(GetInstance(), MAKEINTRESOURCE(IDI_ICON1));
}
*/

void QuakeApp::RegisterGameEvents(void)
{
	REGISTER_EVENT(QuakeEventDataSplashDamage);
	REGISTER_EVENT(QuakeEventDataSpawnActor);
	REGISTER_EVENT(QuakeEventDataPushActor);
	REGISTER_EVENT(QuakeEventDataJumpActor);
	REGISTER_EVENT(QuakeEventDataMoveActor);
	REGISTER_EVENT(QuakeEventDataFallActor);
	REGISTER_EVENT(QuakeEventDataRotateActor);
    REGISTER_EVENT(QuakeEventDataStartThrust);
    REGISTER_EVENT(QuakeEventDataEndThrust);
    REGISTER_EVENT(QuakeEventDataStartSteer);
    REGISTER_EVENT(QuakeEventDataEndSteer);
}

void QuakeApp::CreateNetworkEventForwarder(void)
{
	GameApplication::CreateNetworkEventForwarder();
    if (mNetworkEventForwarder != NULL)
    {
	    BaseEventManager* pGlobalEventManager = BaseEventManager::Get();
		//	FUTURE WORK - Events should have a "classification" that signals if 
		//	they are sent from client to server, from server to client, or both.
		//	Then as events are created, they are automatically added to the right 
		//	network forwarders. This could also detect a 

		pGlobalEventManager->AddListener(
			MakeDelegate(mNetworkEventForwarder.get(), &NetworkEventForwarder::ForwardEvent),
			QuakeEventDataSplashDamage::skEventType);
		pGlobalEventManager->AddListener(
			MakeDelegate(mNetworkEventForwarder.get(), &NetworkEventForwarder::ForwardEvent),
			QuakeEventDataSpawnActor::skEventType);
		pGlobalEventManager->AddListener(
			MakeDelegate(mNetworkEventForwarder.get(), &NetworkEventForwarder::ForwardEvent),
			QuakeEventDataPushActor::skEventType);
		pGlobalEventManager->AddListener(
			MakeDelegate(mNetworkEventForwarder.get(), &NetworkEventForwarder::ForwardEvent),
			QuakeEventDataJumpActor::skEventType);
		pGlobalEventManager->AddListener(
			MakeDelegate(mNetworkEventForwarder.get(), &NetworkEventForwarder::ForwardEvent),
			QuakeEventDataMoveActor::skEventType);
		pGlobalEventManager->AddListener(
			MakeDelegate(mNetworkEventForwarder.get(), &NetworkEventForwarder::ForwardEvent),
			QuakeEventDataFallActor::skEventType);
		pGlobalEventManager->AddListener(
			MakeDelegate(mNetworkEventForwarder.get(), &NetworkEventForwarder::ForwardEvent),
			QuakeEventDataRotateActor::skEventType);
        pGlobalEventManager->AddListener(
			MakeDelegate(mNetworkEventForwarder.get(), &NetworkEventForwarder::ForwardEvent), 
			QuakeEventDataStartThrust::skEventType);
        pGlobalEventManager->AddListener(
			MakeDelegate(mNetworkEventForwarder.get(), &NetworkEventForwarder::ForwardEvent),
			QuakeEventDataEndThrust::skEventType);
        pGlobalEventManager->AddListener(
			MakeDelegate(mNetworkEventForwarder.get(), &NetworkEventForwarder::ForwardEvent),
			QuakeEventDataStartSteer::skEventType);
        pGlobalEventManager->AddListener(
			MakeDelegate(mNetworkEventForwarder.get(), &NetworkEventForwarder::ForwardEvent),
			QuakeEventDataEndSteer::skEventType);

	}
}

void QuakeApp::DestroyNetworkEventForwarder(void)
{
	GameApplication::DestroyNetworkEventForwarder();
    if (mNetworkEventForwarder)
    {
        BaseEventManager* eventManager = BaseEventManager::Get();

		eventManager->RemoveListener(
			MakeDelegate(mNetworkEventForwarder.get(), &NetworkEventForwarder::ForwardEvent),
			EventDataEnvironmentLoaded::skEventType);

		eventManager->RemoveListener(
			MakeDelegate(mNetworkEventForwarder.get(), &NetworkEventForwarder::ForwardEvent),
			QuakeEventDataSplashDamage::skEventType);
		eventManager->RemoveListener(
			MakeDelegate(mNetworkEventForwarder.get(), &NetworkEventForwarder::ForwardEvent),
			QuakeEventDataSpawnActor::skEventType);
		eventManager->RemoveListener(
			MakeDelegate(mNetworkEventForwarder.get(), &NetworkEventForwarder::ForwardEvent),
			QuakeEventDataPushActor::skEventType);
		eventManager->RemoveListener(
			MakeDelegate(mNetworkEventForwarder.get(), &NetworkEventForwarder::ForwardEvent),
			QuakeEventDataJumpActor::skEventType);
		eventManager->RemoveListener(
			MakeDelegate(mNetworkEventForwarder.get(), &NetworkEventForwarder::ForwardEvent),
			QuakeEventDataMoveActor::skEventType);
		eventManager->RemoveListener(
			MakeDelegate(mNetworkEventForwarder.get(), &NetworkEventForwarder::ForwardEvent),
			QuakeEventDataFallActor::skEventType);
		eventManager->RemoveListener(
			MakeDelegate(mNetworkEventForwarder.get(), &NetworkEventForwarder::ForwardEvent),
			QuakeEventDataRotateActor::skEventType);
        eventManager->RemoveListener(
			MakeDelegate(mNetworkEventForwarder.get(), &NetworkEventForwarder::ForwardEvent),
			QuakeEventDataStartThrust::skEventType);
        eventManager->RemoveListener(
			MakeDelegate(mNetworkEventForwarder.get(), &NetworkEventForwarder::ForwardEvent),
			QuakeEventDataEndThrust::skEventType);
        eventManager->RemoveListener(
			MakeDelegate(mNetworkEventForwarder.get(), &NetworkEventForwarder::ForwardEvent),
			QuakeEventDataStartSteer::skEventType);
        eventManager->RemoveListener(
			MakeDelegate(mNetworkEventForwarder.get(), &NetworkEventForwarder::ForwardEvent),
			QuakeEventDataEndSteer::skEventType);

        delete mNetworkEventForwarder.get();
    }
}