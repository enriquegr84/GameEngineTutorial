//========================================================================
// GameLogic.cpp : defines game logic class
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

//#include <mmsystem.h>

#include "GameLogic.h"

//#include "AI/Pathing.h"
#include "Core/Event/Event.h"				// only for EvtData_Game_State
#include "Core/Process/Process.h"
#include "Core/IO/XmlResource.h"

#include "Network/Network.h"
#include "Physic/Physic.h"

#include "Game/GameOption.h"
#include "Game/Actor/Actor.h"
#include "Game/Actor/ActorFactory.h"
#include "Game/Level/LevelManager.h"

#include "Application/GameApplication.h"


//========================================================================
//
// GameLogic implementation
//
//========================================================================


GameLogic::GameLogic()
{
	m_LastActorId = 0;
	m_Lifetime = 0;

	//m_Random.Randomize();
	m_State = BGS_Initializing;
	m_bProxy = false;
	m_RenderDiagnostics = false;
	m_ExpectedPlayers = 0;
	m_ExpectedRemotePlayers = 0;
	m_ExpectedAI = 0;
	m_HumanPlayersAttached = 0;
	m_AIPlayersAttached = 0;
	m_HumanGamesLoaded = 0;
	m_pActorFactory = NULL;
	//m_pPathingGraph = NULL;

	m_pProcessManager = new ProcessManager;
	//mLevelManager = new LevelManager;
	//GE_ASSERT(m_pProcessManager && mLevelManager);
	//mLevelManager->Initialize(g_pGameApp->mResCache->Match(L"world\\*.xml"));

    //	register script events from the engine
	//  [mrmike] this was moved to the constructor post-press, since this function 
	//	can be called when new levels are loaded by the game or editor
    //RegisterEngineScriptEvents();
}

GameLogic::~GameLogic()
{
	// Added this to explicitly remove views from the game logic list.
	GameApplication* gameApp = (GameApplication*)Application::App;
	gameApp->RemoveViews();

	delete mLevelManager;
	delete m_pProcessManager;
	delete m_pActorFactory;

    // destroy all actors
    for (auto it = m_actors.begin(); it != m_actors.end(); ++it)
        it->second->Destroy();
    m_actors.clear();

   BaseEventManager::Get()->RemoveListener(
	   MakeDelegate(this, &GameLogic::RequestDestroyActorDelegate), 
	   EvtData_Request_Destroy_Actor::sk_EventType);
}

bool GameLogic::Init(void)
{
    m_pActorFactory = CreateActorFactory();
    //m_pPathingGraph.reset(CreatePathingGraph());

    BaseEventManager::Get()->AddListener(
		MakeDelegate(this, &GameLogic::RequestDestroyActorDelegate), 
		EvtData_Request_Destroy_Actor::sk_EventType);
    return true;
}

eastl::string GameLogic::GetActorXml(const ActorId id)
{
    eastl::shared_ptr<Actor> pActor(GetActor(id));
    if (pActor) 
		return pActor->ToXML();
    else
        LogError("Couldn't find actor: " + eastl::to_string(id));

    return eastl::string();
}

bool GameLogic::LoadGame(const char* levelResource)
{
	eastl::wstring levelName(levelResource);

    // Grab the root XML node
    XMLElement* pRoot = XmlResourceLoader::LoadAndReturnRootXMLElement(levelName.c_str());
    if (!pRoot)
    {
        LogError(L"Failed to find level resource file: " + levelName);
        return false;
    }

    // pre and post load scripts
    const char* preLoadScript = NULL;
    const char* postLoadScript = NULL;

    // parse the pre & post script attributes
    XMLElement* pScriptElement = pRoot->FirstChildElement("Script");
    if (pScriptElement)
    {
        preLoadScript = pScriptElement->Attribute("preLoad");
        postLoadScript = pScriptElement->Attribute("postLoad");
    }

    // load the pre-load script if there is one
		GameApplication* gameApp = (GameApplication*)Application::App;
	/*
    if (preLoadScript)
    {
        BaseResource resource(preLoadScript);
		// this actually loads the XML file from the zip file
        eastl::shared_ptr<ResHandle> pResourceHandle = 
			gameApp->mResCache->GetHandle(&resource);
    }
	*/

    // load all initial actors
    XMLElement* pActorsNode = pRoot->FirstChildElement("StaticActors");
    if (pActorsNode)
    {
		XMLElement* pNode = pActorsNode->FirstChildElement();
        for (; pNode; pNode = pNode->NextSiblingElement())
        {
            const char* actorResource = pNode->Attribute("resource");

			eastl::shared_ptr<Actor> pActor = CreateActor(actorResource, pNode);
			if (pActor)
			{
				// fire an event letting everyone else know that we created a new actor
				eastl::shared_ptr<EvtData_New_Actor> pNewActorEvent(
					new EvtData_New_Actor(pActor->GetId()));
				BaseEventManager::Get()->QueueEvent(pNewActorEvent);
			}
        }
    }

    // initialize all human views
	gameApp->InitHumanViews(pRoot);

    //	register script events from the engine
	//	[mrmike] this was moved to the constructor post-press, since this function can be 
	//	called when new levels are loaded by the game or editor
    //	RegisterEngineScriptEvents();

    // call the delegate load function
    if (!LoadGameDelegate(pRoot))
        return false;  // no error message, it's assumed LoadGameDelegate() kicked out the error

    // load the post-load script if there is one
	/*
    if (postLoadScript)
    {
        BaseResource resource(postLoadScript);
		// this actually loads the XML file from the zip file
        const eastl::shared_ptr<ResHandle>& pResourceHandle = 
			gameApp->mResCache->GetHandle(&resource);
    }
	*/

	//	trigger the Environment Loaded Game event - 
	//	only then can player actors and AI be spawned!
	if (m_bProxy)
	{
		eastl::shared_ptr<EvtData_Remote_Environment_Loaded> pNewGameEvent(
			new EvtData_Remote_Environment_Loaded);
		BaseEventManager::Get()->TriggerEvent(pNewGameEvent);
	}
	else
	{
		eastl::shared_ptr<EvtData_Environment_Loaded> pNewGameEvent(
			new EvtData_Environment_Loaded);
		BaseEventManager::Get()->TriggerEvent(pNewGameEvent);
	}

    return true;
}



void GameLogic::SetProxy() 
{
	m_bProxy = true; 

    BaseEventManager::Get()->AddListener(
		MakeDelegate(this, &GameLogic::RequestNewActorDelegate), 
		EvtData_Request_New_Actor::sk_EventType);

	mPhysics.reset(CreateNullPhysics());
}

eastl::shared_ptr<Actor> GameLogic::CreateActor(const eastl::string &actorResource, XMLElement *overrides, 
	const Transform *initialTransform, const ActorId serversActorId)
{
    LogAssert(m_pActorFactory, "actor factory is not initialized");
	if (!m_bProxy && serversActorId != INVALID_ACTOR_ID)
		return eastl::shared_ptr<Actor>();

	if (m_bProxy && serversActorId == INVALID_ACTOR_ID)
		return eastl::shared_ptr<Actor>();

    eastl::shared_ptr<Actor> pActor = m_pActorFactory->CreateActor(
		eastl::wstring(actorResource.c_str()).c_str(), overrides, initialTransform, serversActorId);
    if (pActor)
    {
        m_actors.insert(eastl::make_pair(pActor->GetId(), pActor));
		if (!m_bProxy && (m_State==BGS_SpawningPlayersActors || m_State==BGS_Running))
		{
			eastl::shared_ptr<EvtData_Request_New_Actor> pNewActor(
				new EvtData_Request_New_Actor(actorResource, initialTransform, pActor->GetId()));
			BaseEventManager::Get()->TriggerEvent(pNewActor);
		}
        return pActor;
    }
    else
    {
        // FUTURE WORK: Log error: couldn't create actor
        return eastl::shared_ptr<Actor>();
    }
}

void GameLogic::DestroyActor(const ActorId actorId)
{
    //	We need to trigger a synchronous event to ensure that any systems responding to this 
	//	event can still access a valid actor if need be. The actor will be destroyed after this.
    eastl::shared_ptr<EvtData_Destroy_Actor> pEvent(new EvtData_Destroy_Actor(actorId));
    BaseEventManager::Get()->TriggerEvent(pEvent);

    auto findIt = m_actors.find(actorId);
    if (findIt != m_actors.end())
    {
        findIt->second->Destroy();
        m_actors.erase(findIt);
    }
}

eastl::weak_ptr<Actor> GameLogic::GetActor(const ActorId actorId)
{
    ActorMap::iterator findIt = m_actors.find(actorId);
    if (findIt != m_actors.end())
        return findIt->second;

    return eastl::weak_ptr<Actor>();
}

void GameLogic::ModifyActor(const ActorId actorId, XMLElement* overrides)
{
    LogAssert(m_pActorFactory, "actor factory is not initialized");
	if (!m_pActorFactory)
		return;

	auto findIt = m_actors.find(actorId);
    if (findIt != m_actors.end())
    {
		m_pActorFactory->ModifyActor(findIt->second, overrides);
	}
}

void GameLogic::OnUpdate(float time, float elapsedTime)
{
	int deltaMilliseconds = int(elapsedTime * 1000.0f);
	m_Lifetime += elapsedTime;

	GameApplication* gameApp = (GameApplication*)Application::App;

	switch(m_State)
	{
		case BGS_Initializing:
			// If we get to here we're ready to attach players
			ChangeState(BGS_MainMenu);
			break;

		case BGS_MainMenu:
			break;

		case BGS_LoadingGameEnvironment:
/*
			// [mrmike] This was modified a little from what you see in the book - 
			//	LoadGame() is now called from GameLogic::ChangeState()
			if (!gameApp->LoadGame())
			{
				LogError("The game failed to load.");
				gameApp->AbortGame();
			}
*/
			break;

		case BGS_WaitingForPlayersToLoadEnvironment:
			if (m_ExpectedPlayers + m_ExpectedRemotePlayers <= m_HumanGamesLoaded)
			{
				ChangeState(BGS_SpawningPlayersActors);
			}
			break;

		case BGS_SpawningPlayersActors:
			ChangeState(BGS_Running);
			break;

		case BGS_WaitingForPlayers:
			if (m_ExpectedPlayers + m_ExpectedRemotePlayers == m_HumanPlayersAttached ) 
			{
				// The server sends us the level name as a part of the login message. 
				// We have to wait until it arrives before loading the level
				if (!gameApp->mOption.m_Level.empty())
				{
					ChangeState(BGS_LoadingGameEnvironment);
				}
			}
			break;

		case BGS_Running:
			m_pProcessManager->UpdateProcesses(deltaMilliseconds);

            if(mPhysics && !m_bProxy)
            {
                mPhysics->OnUpdate(elapsedTime);
                mPhysics->SyncVisibleScene();
            }

			break;

		default:
			LogError("Unrecognized state.");
	}

    // update game actors
    for (ActorMap::const_iterator it = m_actors.begin(); it != m_actors.end(); ++it)
    {
        it->second->Update(deltaMilliseconds);
    }

}

//
// GameLogic::ChangeState				- Chapter 19, page 710
//
void GameLogic::ChangeState(BaseGameState newState)
{
	GameApplication* gameApp = (GameApplication*)Application::App;

	if (newState==BGS_WaitingForPlayers)
	{
		// Get rid of the Main Menu...
		gameApp->RemoveView();

		// Note: Split screen support would require this to change!
		m_ExpectedPlayers = 1;
		m_ExpectedRemotePlayers = gameApp->mOption.m_expectedPlayers - 1;
		m_ExpectedAI = gameApp->mOption.m_numAIs;

		if (!gameApp->mOption.m_gameHost.empty())
		{
			SetProxy();					
			m_ExpectedAI = 0;				// the server will create these
			m_ExpectedRemotePlayers = 0;	// the server will create these
			
			if (!gameApp->AttachAsClient())
			{
				// Throw up a main menu
				ChangeState(BGS_MainMenu);
				return;
			}
		}
		else if (m_ExpectedRemotePlayers > 0)
		{
			BaseSocketManager *pServer = new BaseSocketManager();
			if (!pServer->Init())
			{
				// Throw up a main menu
				ChangeState(BGS_MainMenu);	
				return;
			}

			pServer->AddSocket(
				new GameServerListenSocket(gameApp->mOption.m_listenPort));
			gameApp->mBaseSocketManager.reset(pServer);
		}
	}
	else if (newState == BGS_LoadingGameEnvironment)
	{
		m_State = newState;

		if (!gameApp->LoadGame())
		{
			LogError("The game failed to load.");
			gameApp->AbortGame();
		}
		else
		{
			// we must wait for all human player to report their environments are loaded.
			ChangeState(BGS_WaitingForPlayersToLoadEnvironment);
			return;
		}
	}

	m_State = newState;
}


// Chapter 19/20 refactor work
//   - move physics pointer into GameLogic
//   - but make the inherited logic choose the implementation
void GameLogic::RenderDiagnostics() 
{ 
	if (m_RenderDiagnostics)
	{
		mPhysics->RenderDiagnostics();
	}
}


ActorFactory* GameLogic::CreateActorFactory(void)
{
    return new ActorFactory;
}


void GameLogic::RequestDestroyActorDelegate(BaseEventDataPtr pEventData)
{
    eastl::shared_ptr<EvtData_Request_Destroy_Actor> pCastEventData = 
		eastl::static_pointer_cast<EvtData_Request_Destroy_Actor>(pEventData);
    DestroyActor(pCastEventData->GetActorId());
}


// [mrmike] -	These were moved here after the chapter for GameLogic was written. 
//				These were originally in TeapotWarsLogic class, but should really be 
//				in the BaseLogic class.

void GameLogic::MoveActorDelegate(BaseEventDataPtr pEventData)
{
    eastl::shared_ptr<EvtData_Move_Actor> pCastEventData = 
		eastl::static_pointer_cast<EvtData_Move_Actor>(pEventData);
    MoveActor(pCastEventData->GetId(), pCastEventData->GetMatrix());
}

void GameLogic::RequestNewActorDelegate(BaseEventDataPtr pEventData)
{
	//	This should only happen if the game logic is a proxy, and there's a server 
	//	asking us to create an actor.
	LogAssert(m_bProxy, "Proxy not initialized");
	if (!m_bProxy)
		return;

    eastl::shared_ptr<EvtData_Request_New_Actor> pCastEventData = 
		eastl::static_pointer_cast<EvtData_Request_New_Actor>(pEventData);

    // create the actor
	eastl::shared_ptr<Actor> pActor = CreateActor(pCastEventData->GetActorResource(), 
		NULL, pCastEventData->GetInitialTransform(), pCastEventData->GetServerActorId());
	if (pActor)
	{
		eastl::shared_ptr<EvtData_New_Actor> pNewActorEvent(
			new EvtData_New_Actor(pActor->GetId(), pCastEventData->GetViewId()));
        BaseEventManager::Get()->QueueEvent(pNewActorEvent);
	}
}