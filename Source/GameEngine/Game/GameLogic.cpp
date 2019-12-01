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

#include "GameLogic.h"

//#include "AI/Pathing.h"
#include "Core/Event/Event.h"				// only for EventDataGame_State
#include "Core/Process/Process.h"
#include "Core/IO/XmlResource.h"

#include "Network/Network.h"
#include "Physic/Physic.h"

#include "Game/GameOption.h"
#include "Game/Actor/Actor.h"
#include "Game/Actor/ActorFactory.h"
#include "Game/Level/LevelManager.h"

#include "AI/AIManager.h"

#include "Application/GameApplication.h"


//========================================================================
//
// GameLogic implementation
//
//========================================================================

GameLogic* GameLogic::mGame = NULL;

GameLogic* GameLogic::Get(void)
{
	LogAssert(GameLogic::mGame, "Game doesn't exist");
	return GameLogic::mGame;
}

GameLogic::GameLogic()
{
	mLastActorId = 0;
	mLifetime = 0;

	//mRandom.Randomize();
	mGameState = BGS_INITIALIZING;
	mIsProxy = false;
	mIsRenderDiagnostics = false;
	mExpectedPlayers = 0;
	mExpectedRemotePlayers = 0;
	mExpectedAI = 0;
	mHumanPlayersAttached = 0;
	mAIPlayersAttached = 0;
	mHumanGamesLoaded = 0;
	mActorFactory = NULL;
	mLevelManager = NULL;
	mAIManager = NULL;

	mProcessManager = new ProcessManager();
	LogAssert(mProcessManager, "Uninitialized process mngr");

    //	register script events from the engine
	//  [mrmike] this was moved to the constructor post-press, since this function 
	//	can be called when new levels are loaded by the game or editor
    //RegisterEngineScriptEvents();

	if (GameLogic::mGame)
	{
		LogError("Attempting to create two global game! \
					The old one will be destroyed and overwritten with this one.");
		delete GameLogic::mGame;
	}

	GameLogic::mGame = this;
}

GameLogic::~GameLogic()
{
	// Added this to explicitly remove views from the game logic list.
	GameApplication* gameApp = (GameApplication*)Application::App;
	gameApp->RemoveViews();

	delete mProcessManager;
	delete mActorFactory;
	delete mLevelManager;
	delete mAIManager;

	mProcessManager = nullptr;
	mActorFactory = nullptr;
	mLevelManager = nullptr;
	mAIManager = nullptr;

    // destroy all actors
    for (auto it = mActors.begin(); it != mActors.end(); ++it)
        it->second->Destroy();
    mActors.clear();

   BaseEventManager::Get()->RemoveListener(
	   MakeDelegate(this, &GameLogic::RequestDestroyActorDelegate), 
	   EventDataRequestDestroyActor::skEventType);

   if (GameLogic::mGame == this)
	   GameLogic::mGame = nullptr;
}

bool GameLogic::Init(void)
{
    mActorFactory = CreateActorFactory();
	mLevelManager = CreateLevelManager();
	mAIManager = CreateAIManager();

    BaseEventManager::Get()->AddListener(
		MakeDelegate(this, &GameLogic::RequestDestroyActorDelegate), 
		EventDataRequestDestroyActor::skEventType);
    return true;
}

eastl::string GameLogic::GetActorXml(const ActorId id)
{
    eastl::shared_ptr<Actor> pActor(GetActor(id).lock());
    if (pActor) 
		return pActor->ToXML();
    else
        LogError("Couldn't find actor: " + eastl::to_string(id));

    return eastl::string();
}

bool GameLogic::LoadGame(const char* levelResource)
{
	eastl::wstring levelName(ToWideString(levelResource));

    // Grab the root XML node
	tinyxml2::XMLElement* pRoot = XmlResourceLoader::LoadAndReturnRootXMLElement(levelName.c_str());
    if (!pRoot)
    {
        LogError(L"Failed to find level resource file: " + levelName);
        return false;
    }

    // pre and post load scripts
    const char* preLoadScript = NULL;
    const char* postLoadScript = NULL;

    // parse the pre & post script attributes
	tinyxml2::XMLElement* pScriptElement = pRoot->FirstChildElement("Script");
    if (pScriptElement)
    {
        preLoadScript = pScriptElement->Attribute("preLoad");
        postLoadScript = pScriptElement->Attribute("postLoad");
    }

    // load all initial actors
	tinyxml2::XMLElement* pActorsNode = pRoot->FirstChildElement("StaticActors");
    if (pActorsNode)
    {
		tinyxml2::XMLElement* pNode = pActorsNode->FirstChildElement();
        for (; pNode; pNode = pNode->NextSiblingElement())
        {
            const char* actorResource = pNode->Attribute("resource");

			eastl::shared_ptr<Actor> pActor = CreateActor(actorResource, pNode);
			if (pActor)
			{
				// fire an event letting everyone else know that we created a new actor
				eastl::shared_ptr<EventDataNewActor> pNewActorEvent(
					new EventDataNewActor(pActor->GetId()));
				BaseEventManager::Get()->QueueEvent(pNewActorEvent);
			}
        }
    }

    // initialize all human views
	GameApplication* gameApp = (GameApplication*)Application::App;
	gameApp->InitHumanViews(pRoot);

    //	register script events from the engine
	//	[mrmike] this was moved to the constructor post-press, since this function can be 
	//	called when new levels are loaded by the game or editor
    //	RegisterEngineScriptEvents();

    // call the delegate load function
    if (!LoadGameDelegate(pRoot))
        return false;  // no error message, it's assumed LoadGameDelegate() kicked out the error

	//	trigger the Environment Loaded Game event - 
	//	only then can player actors and AI be spawned!
	if (mIsProxy)
	{
		eastl::shared_ptr<EventDataRemoteEnvironmentLoaded> pNewGameEvent(
			new EventDataRemoteEnvironmentLoaded);
		BaseEventManager::Get()->TriggerEvent(pNewGameEvent);
	}
	else
	{
		eastl::shared_ptr<EventDataEnvironmentLoaded> pNewGameEvent(
			new EventDataEnvironmentLoaded);
		BaseEventManager::Get()->TriggerEvent(pNewGameEvent);
	}

    return true;
}

void GameLogic::SetProxy() 
{
	mIsProxy = true; 

    BaseEventManager::Get()->AddListener(
		MakeDelegate(this, &GameLogic::RequestNewActorDelegate), 
		EventDataRequestNewActor::skEventType);

	mPhysics.reset(CreateNullPhysics());
}

eastl::shared_ptr<Actor> GameLogic::CreateActor(const eastl::string &actorResource, 
	tinyxml2::XMLElement *overrides, const Transform *initialTransform, const ActorId serversActorId)
{
    LogAssert(mActorFactory, "actor factory is not initialized");
	if (!mIsProxy && serversActorId != INVALID_ACTOR_ID)
		return eastl::shared_ptr<Actor>();

	if (mIsProxy && serversActorId == INVALID_ACTOR_ID)
		return eastl::shared_ptr<Actor>();

    eastl::shared_ptr<Actor> pActor = mActorFactory->CreateActor(
		ToWideString(actorResource.c_str()).c_str(), overrides, initialTransform, serversActorId);
    if (pActor)
    {
        mActors.insert(eastl::make_pair(pActor->GetId(), pActor));
		if (!mIsProxy && (mGameState==BGS_SPAWNINGPLAYERACTORS || mGameState==BGS_RUNNING))
		{
			eastl::shared_ptr<EventDataRequestNewActor> pNewActor(
				new EventDataRequestNewActor(actorResource, initialTransform, pActor->GetId()));
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
	//	event can still access a valid actor if need to be. The actor will be destroyed after this.
    eastl::shared_ptr<EventDataDestroyActor> pEvent(new EventDataDestroyActor(actorId));
    BaseEventManager::Get()->TriggerEvent(pEvent);

    auto findIt = mActors.find(actorId);
    if (findIt != mActors.end())
    {
        findIt->second->Destroy();
        mActors.erase(findIt);
    }
}

eastl::weak_ptr<Actor> GameLogic::GetActor(const ActorId actorId)
{
    ActorMap::iterator findIt = mActors.find(actorId);
    if (findIt != mActors.end())
        return findIt->second;

    return eastl::weak_ptr<Actor>();
}

void GameLogic::ModifyActor(const ActorId actorId, tinyxml2::XMLElement* overrides)
{
    LogAssert(mActorFactory, "actor factory is not initialized");
	if (!mActorFactory)
		return;

	auto findIt = mActors.find(actorId);
    if (findIt != mActors.end())
    {
		mActorFactory->ModifyActor(findIt->second, overrides);
	}
}

void GameLogic::OnUpdate(float time, float elapsedTime)
{
	mLifetime += elapsedTime;

	GameApplication* gameApp = (GameApplication*)Application::App;

	switch(mGameState)
	{
		case BGS_INITIALIZING:
			// If we get to here we're ready to attach players
			ChangeState(BGS_MAINMENU);
			break;

		case BGS_MAINMENU:
			break;

		case BGS_LOADINGGAMEENVIRONMENT:
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

		case BGS_WAITINGFORPLAYERSTOLOADENVIRONMENT:
			if (mExpectedPlayers + mExpectedRemotePlayers <= mHumanGamesLoaded)
			{
				ChangeState(BGS_SPAWNINGPLAYERACTORS);
			}
			break;

		case BGS_SPAWNINGPLAYERACTORS:
			ChangeState(BGS_RUNNING);
			break;

		case BGS_WAITINGFORPLAYERS:
			if (mExpectedPlayers + mExpectedRemotePlayers == mHumanPlayersAttached ) 
			{
				// The server sends us the level name as a part of the login message. 
				// We have to wait until it arrives before loading the level
				if (!gameApp->mOption.mLevel.empty())
				{
					ChangeState(BGS_LOADINGGAMEENVIRONMENT);
				}
			}
			break;

		case BGS_RUNNING:
			mProcessManager->UpdateProcesses((unsigned int)elapsedTime);

			if (mAIManager)
			{
				mAIManager->OnUpdate((unsigned int)elapsedTime);
			}

            if(mPhysics && !mIsProxy)
            {
                mPhysics->OnUpdate(elapsedTime / 1000.f);
                mPhysics->SyncVisibleScene();
            }

			break;

		default:
			LogError("Unrecognized state.");
	}

    // update game actors
    for (ActorMap::const_iterator it = mActors.begin(); it != mActors.end(); ++it)
    {
        it->second->Update(elapsedTime);
    }

}

//
// GameLogic::ChangeState				- Chapter 19, page 710
//
void GameLogic::ChangeState(BaseGameState newState)
{
	GameApplication* gameApp = (GameApplication*)Application::App;
	if (newState==BGS_WAITINGFORPLAYERS)
	{
		// Get rid of the Main Menu...
		gameApp->RemoveView();

		// Note: Split screen support would require this to change!
		mExpectedPlayers = 1;
		mExpectedRemotePlayers = gameApp->mOption.mExpectedPlayers - 1;
		mExpectedAI = gameApp->mOption.mNumAIs;

		mGameState = newState;
		return;

		if (!gameApp->mOption.mGameHost.empty())
		{
			SetProxy();					
			mExpectedAI = 0;				// the server will create these
			mExpectedRemotePlayers = 0;	// the server will create these
			
			if (!gameApp->AttachAsClient())
			{
				// Throw up a main menu
				//ChangeState(BGS_MAINMENU);
				return;
			}
		}
		else if (mExpectedRemotePlayers > 0)
		{
			BaseSocketManager *pServer = new BaseSocketManager();
			if (!pServer->Init())
			{
				// Throw up a main menu
				//ChangeState(BGS_MAINMENU);	
				return;
			}

			pServer->AddSocket(
				new GameServerListenSocket(gameApp->mOption.mListenPort));
			gameApp->mBaseSocketManager.reset(pServer);
		}
	}
	else if (newState == BGS_LOADINGGAMEENVIRONMENT)
	{
		mGameState = newState;

		if (!gameApp->LoadGame())
		{
			LogError("The game failed to load.");
			gameApp->AbortGame();
		}
		else
		{
			// we must wait for all human player to report their environments are loaded.
			ChangeState(BGS_WAITINGFORPLAYERSTOLOADENVIRONMENT);
			return;
		}
	}

	mGameState = newState;
}


// Chapter 19/20 refactor work
//   - move physics pointer into GameLogic
//   - but make the inherited logic choose the implementation
void GameLogic::RenderDiagnostics() 
{ 
	if (mIsRenderDiagnostics)
		mPhysics->RenderDiagnostics();
}


ActorFactory* GameLogic::CreateActorFactory(void)
{
    return new ActorFactory();
}

AIManager* GameLogic::CreateAIManager(void)
{
	return new AIManager();
}

LevelManager* GameLogic::CreateLevelManager(void)
{
	return new LevelManager();
}

void GameLogic::RequestDestroyActorDelegate(BaseEventDataPtr pEventData)
{
    eastl::shared_ptr<EventDataRequestDestroyActor> pCastEventData = 
		eastl::static_pointer_cast<EventDataRequestDestroyActor>(pEventData);
    DestroyActor(pCastEventData->GetActorId());
}

void GameLogic::SyncActorDelegate(BaseEventDataPtr pEventData)
{
	eastl::shared_ptr<EventDataSyncActor> pCastEventData =
		eastl::static_pointer_cast<EventDataSyncActor>(pEventData);
	SyncActor(pCastEventData->GetId(), pCastEventData->GetTransform());
}

void GameLogic::RequestNewActorDelegate(BaseEventDataPtr pEventData)
{
	//	This should only happen if the game logic is a proxy, and there's a server 
	//	asking us to create an actor.
	LogAssert(mIsProxy, "Proxy not initialized");
	if (!mIsProxy)
		return;

    eastl::shared_ptr<EventDataRequestNewActor> pCastEventData = 
		eastl::static_pointer_cast<EventDataRequestNewActor>(pEventData);

    // create the actor
	eastl::shared_ptr<Actor> pActor = CreateActor(pCastEventData->GetActorResource(), 
		NULL, pCastEventData->GetInitialTransform(), pCastEventData->GetServerActorId());
	if (pActor)
	{
		eastl::shared_ptr<EventDataNewActor> pNewActorEvent(
			new EventDataNewActor(pActor->GetId(), pCastEventData->GetViewId()));
        BaseEventManager::Get()->TriggerEvent(pNewActorEvent);
	}
}