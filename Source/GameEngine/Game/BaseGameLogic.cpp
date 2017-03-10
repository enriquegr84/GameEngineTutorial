//========================================================================
// BaseGameLogic.cpp : defines game logic class
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

#include "BaseGameLogic.h"

//#include "AI/Pathing.h"
#include "Events/Events.h"				// only for EvtData_Game_State
#include "Process/Process.h"
#include "Network/Network.h"
#include "ResourceCache/XmlResource.h"
#include "Physics/Physics.h"
#include "Actors/Actor.h"
#include "Actors/ActorFactory.h"

#include "GameEngine.h"
#include "GameOptions.h"

#include "LevelManager.h"


//========================================================================
//
// BaseGameLogic implementation
//
//========================================================================


BaseGameLogic::BaseGameLogic()
{
	m_LastActorId = 0;
	m_Lifetime = 0;
	m_pProcessManager = new ProcessManager;
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

	//m_pLevelManager = new LevelManager;
	//GE_ASSERT(m_pProcessManager && m_pLevelManager);
	//m_pLevelManager->Initialize(g_pGameApp->mResCache->Match("world\\*.xml"));

    //	register script events from the engine
	//  [mrmike] this was moved to the constructor post-press, since this function 
	//	can be called when new levels are loaded by the game or editor
    RegisterEngineScriptEvents();
}

BaseGameLogic::~BaseGameLogic()
{
	// Added this to explicitly remove views from the game logic list.
	g_pGameApp->RemoveViews();

	//SAFE_DELETE(m_pLevelManager);
	SAFE_DELETE(m_pProcessManager);
    SAFE_DELETE(m_pActorFactory);

    // destroy all actors
    for (auto it = m_actors.begin(); it != m_actors.end(); ++it)
        it->second->Destroy();
    m_actors.clear();

   BaseEventManager::Get()->RemoveListener(
	   MakeDelegate(this, &BaseGameLogic::RequestDestroyActorDelegate), EvtData_Request_Destroy_Actor::sk_EventType);
}

bool BaseGameLogic::Init(void)
{
    m_pActorFactory = CreateActorFactory();
    //m_pPathingGraph.reset(CreatePathingGraph());

    BaseEventManager::Get()->AddListener(
		MakeDelegate(this, &BaseGameLogic::RequestDestroyActorDelegate), EvtData_Request_Destroy_Actor::sk_EventType);
    return true;
}

eastl::string BaseGameLogic::GetActorXml(const ActorId id)
{
    StrongActorPtr pActor = MakeStrongPtr(GetActor(id));
    if (pActor)
            return pActor->ToXML();
    else
        GE_ERROR(eastl::string("Couldn't find actor: ") + eastl::string(id));

    return eastl::string();
}

bool BaseGameLogic::LoadGame(const wchar_t* levelResource)
{
    // Grab the root XML node
    XMLElement* pRoot = XmlResourceLoader::LoadAndReturnRootXMLElement(levelResource);
    if (!pRoot)
    {
        GE_ERROR(eastl::string("Failed to find level resource file: ") + eastl::string(levelResource));
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
    if (preLoadScript)
    {
        BaseResource resource(preLoadScript);
		// this actually loads the XML file from the zip file
        shared_ptr<ResHandle> pResourceHandle = 
			g_pGameApp->m_ResCache->GetHandle(&resource);  
    }

    // load all initial actors
    XMLElement* pActorsNode = pRoot->FirstChildElement("StaticActors");
    if (pActorsNode)
    {
		XMLElement* pNode = pActorsNode->FirstChildElement();
        for (; pNode; pNode = pNode->NextSiblingElement())
        {
            const char* actorResource = pNode->Attribute("resource");

			StrongActorPtr pActor = CreateActor(actorResource, pNode);
			if (pActor)
			{
				// fire an event letting everyone else know that we created a new actor
				shared_ptr<EvtData_New_Actor> pNewActorEvent(
					new EvtData_New_Actor(pActor->GetId()));
				BaseEventManager::Get()->QueueEvent(pNewActorEvent);
			}
        }
    }

    // initialize all human views
	g_pGameApp->InitHumanViews(pRoot);

    //	register script events from the engine
	//	[mrmike] this was moved to the constructor post-press, since this function can be 
	//	called when new levels are loaded by the game or editor
    //	RegisterEngineScriptEvents();

    // call the delegate load function
    if (!LoadGameDelegate(pRoot))
        return false;  // no error message, it's assumed LoadGameDelegate() kicked out the error

    // load the post-load script if there is one
    if (postLoadScript)
    {
        BaseResource resource(postLoadScript);
		// this actually loads the XML file from the zip file
        const shared_ptr<ResHandle>& pResourceHandle = 
			g_pGameApp->m_ResCache->GetHandle(&resource);
    }

	//	trigger the Environment Loaded Game event - 
	//	only then can player actors and AI be spawned!
	if (m_bProxy)
	{
		shared_ptr<EvtData_Remote_Environment_Loaded> pNewGameEvent(
			new EvtData_Remote_Environment_Loaded);
		BaseEventManager::Get()->TriggerEvent(pNewGameEvent);
	}
	else
	{
		shared_ptr<EvtData_Environment_Loaded> pNewGameEvent(
			new EvtData_Environment_Loaded);
		BaseEventManager::Get()->TriggerEvent(pNewGameEvent);
	}

    return true;
}



void BaseGameLogic::SetProxy() 
{
	m_bProxy = true; 

    BaseEventManager::Get()->AddListener(
		MakeDelegate(this, &BaseGameLogic::RequestNewActorDelegate), 
		EvtData_Request_New_Actor::sk_EventType);

	m_pPhysics.reset(CreateNullPhysics());
}

StrongActorPtr BaseGameLogic::CreateActor(const eastl::string &actorResource, XMLElement *overrides, 
	const matrix4 *initialTransform, const ActorId serversActorId)
{
    GE_ASSERT(m_pActorFactory);
	if (!m_bProxy && serversActorId != INVALID_ACTOR_ID)
		return StrongActorPtr();

	if (m_bProxy && serversActorId == INVALID_ACTOR_ID)
		return StrongActorPtr();

    StrongActorPtr pActor = m_pActorFactory->CreateActor(
		eastl::wstring(actorResource.c_str()).c_str(), overrides, initialTransform, serversActorId);
    if (pActor)
    {
        m_actors.insert(eastl::make_pair(pActor->GetId(), pActor));
		if (!m_bProxy && (m_State==BGS_SpawningPlayersActors || m_State==BGS_Running))
		{
			shared_ptr<EvtData_Request_New_Actor> pNewActor(
				new EvtData_Request_New_Actor(actorResource, initialTransform, pActor->GetId()));
			BaseEventManager::Get()->TriggerEvent(pNewActor);
		}
        return pActor;
    }
    else
    {
        // FUTURE WORK: Log error: couldn't create actor
        return StrongActorPtr();
    }
}

void BaseGameLogic::DestroyActor(const ActorId actorId)
{
    //	We need to trigger a synchronous event to ensure that any systems responding to this 
	//	event can still access a valid actor if need be. The actor will be destroyed after this.
    shared_ptr<EvtData_Destroy_Actor> pEvent(new EvtData_Destroy_Actor(actorId));
    BaseEventManager::Get()->TriggerEvent(pEvent);

    auto findIt = m_actors.find(actorId);
    if (findIt != m_actors.end())
    {
        findIt->second->Destroy();
        m_actors.erase(findIt);
    }
}

WeakActorPtr BaseGameLogic::GetActor(const ActorId actorId)
{
    ActorMap::iterator findIt = m_actors.find(actorId);
    if (findIt != m_actors.end())
        return findIt->second;

    return WeakActorPtr();
}

void BaseGameLogic::ModifyActor(const ActorId actorId, XMLElement* overrides)
{
    GE_ASSERT(m_pActorFactory);
	if (!m_pActorFactory)
		return;

	auto findIt = m_actors.find(actorId);
    if (findIt != m_actors.end())
    {
		m_pActorFactory->ModifyActor(findIt->second, overrides);
	}
}

void BaseGameLogic::OnUpdate(float time, float elapsedTime)
{
	int deltaMilliseconds = int(elapsedTime * 1000.0f);
	m_Lifetime += elapsedTime;

	switch(m_State)
	{
		case BGS_Initializing:
			// If we get to here we're ready to attach players
			ChangeState(BGS_MainMenu);
			break;

		case BGS_MainMenu:
			break;

		case BGS_LoadingGameEnvironment:
/***
			// [mrmike] This was modified a little from what you see in the book - 
			//	LoadGame() is now called from BaseGameLogic::ChangeState()
			if (!g_pGameApp->LoadGame())
			{
				GE_ERROR("The game failed to load.");
				g_pGameApp->AbortGame();
			}
***/
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
				if (!g_pGameApp->m_Options.m_Level.empty())
				{
					ChangeState(BGS_LoadingGameEnvironment);
				}
			}
			break;

		case BGS_Running:
			m_pProcessManager->UpdateProcesses(deltaMilliseconds);

            if(m_pPhysics && !m_bProxy)
            {
                m_pPhysics->OnUpdate(elapsedTime);
                m_pPhysics->SyncVisibleScene();
            }

			break;

		default:
			GE_ERROR("Unrecognized state.");
	}

    // update game actors
    for (ActorMap::const_iterator it = m_actors.begin(); it != m_actors.end(); ++it)
    {
        it->second->Update(deltaMilliseconds);
    }

}

//
// BaseGameLogic::ChangeState				- Chapter 19, page 710
//
void BaseGameLogic::ChangeState(BaseGameState newState)
{
	if (newState==BGS_WaitingForPlayers)
	{
		// Get rid of the Main Menu...
		g_pGameApp->RemoveView();

		// Note: Split screen support would require this to change!
		m_ExpectedPlayers = 1;
		m_ExpectedRemotePlayers = g_pGameApp->m_Options.m_expectedPlayers - 1;
		m_ExpectedAI = g_pGameApp->m_Options.m_numAIs;

		if (!g_pGameApp->m_Options.m_gameHost.empty())
		{
			SetProxy();					
			m_ExpectedAI = 0;				// the server will create these
			m_ExpectedRemotePlayers = 0;	// the server will create these
			
			if (!g_pGameApp->AttachAsClient())
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
				new GameServerListenSocket(g_pGameApp->m_Options.m_listenPort));
			g_pGameApp->m_pBaseSocketManager = pServer;
		}
	}
	else if (newState == BGS_LoadingGameEnvironment)
	{
		m_State = newState;

		if (!g_pGameApp->LoadGame())
		{
			GE_ERROR("The game failed to load.");
			g_pGameApp->AbortGame();
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
//   - move physics pointer into BaseGameLogic
//   - but make the inherited logic choose the implementation
void BaseGameLogic::RenderDiagnostics() 
{ 
	if (m_RenderDiagnostics)
	{
		m_pPhysics->RenderDiagnostics();
	}
}


ActorFactory* BaseGameLogic::CreateActorFactory(void)
{
    return new ActorFactory;
}


void BaseGameLogic::RequestDestroyActorDelegate(BaseEventDataPtr pEventData)
{
    shared_ptr<EvtData_Request_Destroy_Actor> pCastEventData = 
		static_pointer_cast<EvtData_Request_Destroy_Actor>(pEventData);
    DestroyActor(pCastEventData->GetActorId());
}


// [mrmike] -	These were moved here after the chapter for BaseGameLogic was written. 
//				These were originally in TeapotWarsLogic class, but should really be 
//				in the BaseLogic class.

void BaseGameLogic::MoveActorDelegate(BaseEventDataPtr pEventData)
{
    shared_ptr<EvtData_Move_Actor> pCastEventData = 
		static_pointer_cast<EvtData_Move_Actor>(pEventData);
    MoveActor(pCastEventData->GetId(), pCastEventData->GetMatrix());
}

void BaseGameLogic::RequestNewActorDelegate(BaseEventDataPtr pEventData)
{
	//	This should only happen if the game logic is a proxy, and there's a server 
	//	asking us to create an actor.
	GE_ASSERT(m_bProxy);
	if (!m_bProxy)
		return;

    shared_ptr<EvtData_Request_New_Actor> pCastEventData = 
		static_pointer_cast<EvtData_Request_New_Actor>(pEventData);

    // create the actor
	StrongActorPtr pActor = CreateActor(pCastEventData->GetActorResource(), 
		NULL, pCastEventData->GetInitialTransform(), pCastEventData->GetServerActorId());
	if (pActor)
	{
		shared_ptr<EvtData_New_Actor> pNewActorEvent(
			new EvtData_New_Actor(pActor->GetId(), pCastEventData->GetViewId()));
        BaseEventManager::Get()->QueueEvent(pNewActorEvent);
	}
}