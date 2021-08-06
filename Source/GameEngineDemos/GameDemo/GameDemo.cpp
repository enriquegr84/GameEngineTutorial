//========================================================================
//
// GameDemoLogic Implementation       - Chapter 21, page 725
//
//========================================================================

#include "GameDemoView.h"

#include "Physic/Physic.h"
#include "Physic/PhysicEventListener.h"

#include "GameDemo.h"
#include "GameDemoApp.h"
#include "GameDemoNetwork.h"
#include "GameDemoEvents.h"
#include "GameDemoManager.h"
#include "GameDemoAIManager.h"

//
// GameDemoLogic::GameDemoLogic
//
GameDemoLogic::GameDemoLogic()
	: GameLogic()
{
	mPhysics.reset(CreateGamePhysics());
	RegisterAllDelegates();
}


//
// GameDemoLogic::~GameDemoLogic
//
GameDemoLogic::~GameDemoLogic()
{
	RemoveAllDelegates();
	DestroyAllNetworkEventForwarders();
}


void GameDemoLogic::UpdateViewType(const eastl::shared_ptr<BaseGameView>& pView, bool add)
{
	GameLogic::UpdateViewType(pView, add);

	//  This is commented out because while the view is created and waiting, the player has NOT attached yet. 
	/*
	if (pView->GetType() == GV_REMOTE)
	{
		mHumanPlayersAttached += add ? 1 : -1;
	}
	*/
	if (pView->GetType() == GV_HUMAN)
	{
		mHumanPlayersAttached += add ? 1 : -1;
	}
	else if (pView->GetType() == GV_AI)
	{
		mAIPlayersAttached += add ? 1 : -1;
	}
}

void GameDemoLogic::ResetViewType()
{
	GameLogic::ResetViewType();

	mHumanPlayersAttached = 0;
	mAIPlayersAttached = 0;
}

void GameDemoLogic::SetProxy()
{
	GameLogic::SetProxy();
}


//
// GameDemoLogic::ChangeState
//
void GameDemoLogic::ChangeState(BaseGameState newState)
{
	GameLogic::ChangeState(newState);

	GameApplication* gameApp = (GameApplication*)Application::App;
	switch (newState)
	{
		case BGS_MAINMENU:
		{
			eastl::shared_ptr<BaseGameView> menuView(new MainMenuView());
			gameApp->AddView(menuView);

			break;
		}

		case BGS_WAITINGFORPLAYERS:
		{
			// spawn all local players (should only be one, though we might support more in the future)
			LogAssert(mExpectedPlayers == 1, "needs only one player");
			for (int i = 0; i < mExpectedPlayers; ++i)
			{
				eastl::shared_ptr<BaseGameView> playersView(new GameDemoHumanView());
				gameApp->AddView(playersView);

				if (mIsProxy)
				{
					// if we are a remote player, all we have to do is spawn our view - the server will do the rest.
					return;
				}
			}
			// spawn all remote player's views on the game
			for (int i = 0; i < mExpectedRemotePlayers; ++i)
			{
				eastl::shared_ptr<BaseGameView> remoteGameView(new NetworkGameView());
				gameApp->AddView(remoteGameView);
			}

			// spawn all AI's views on the game
			for (int i = 0; i < mExpectedAI; ++i)
			{
				eastl::shared_ptr<BaseGameView> aiView(new AIPlayerView(eastl::shared_ptr<PathingGraph>()));
				gameApp->AddView(aiView);
			}

			break;
		}

		case BGS_SPAWNINGPLAYERACTORS:
		{
			if (mIsProxy)
			{
				// only the server needs to do this.
				return;
			}

            const GameViewList& gameViews = gameApp->GetGameViews();
            for (auto it = gameViews.begin(); it != gameViews.end(); ++it)
            {
                eastl::shared_ptr<BaseGameView> pView = *it;
                if (pView->GetType() == GV_HUMAN)
                {
                    eastl::shared_ptr<Actor> pActor = CreateActor("actors\\player.xml", NULL);
                    if (pActor)
                    {
                        pView->OnAttach(pView->GetId(), pActor->GetId());

                        eastl::shared_ptr<EventDataNewActor> pNewActorEvent(
                            new EventDataNewActor(pActor->GetId(), pView->GetId()));

                        // [rez] This needs to happen asap because the constructor function for Lua 
                        // (which is called in through CreateActor()) queues an event that expects 
                        // this event to have been handled
                        BaseEventManager::Get()->TriggerEvent(pNewActorEvent);
                    }
                }
                else if (pView->GetType() == GV_REMOTE)
                {
                    eastl::shared_ptr<NetworkGameView> pNetworkGameView =
                        eastl::static_pointer_cast<NetworkGameView, BaseGameView>(pView);
                    eastl::shared_ptr<Actor> pActor = CreateActor("actors\\remote_player.xml", NULL);
                    if (pActor)
                    {
                        pView->OnAttach(pView->GetId(), pActor->GetId());

                        eastl::shared_ptr<EventDataNewActor> pNewActorEvent(
                            new EventDataNewActor(pActor->GetId(), pNetworkGameView->GetId()));
                        BaseEventManager::Get()->QueueEvent(pNewActorEvent);
                    }
                }
                else if (pView->GetType() == GV_AI)
                {
                    eastl::shared_ptr<AIPlayerView> pAiView =
                        eastl::static_pointer_cast<AIPlayerView, BaseGameView>(pView);
                    eastl::shared_ptr<Actor> pActor = CreateActor("actors\\ai_player.xml", NULL);
                    if (pActor)
                    {
                        pView->OnAttach(pView->GetId(), pActor->GetId());

                        eastl::shared_ptr<EventDataNewActor> pNewActorEvent(
                            new EventDataNewActor(pActor->GetId(), pAiView->GetId()));
                        BaseEventManager::Get()->QueueEvent(pNewActorEvent);
                    }
                }
            }

			break;
		}
	}
}

void GameDemoLogic::SyncActor(const ActorId id, Transform const &transform)
{
	GameLogic::SyncActor(id, transform);
}

void GameDemoLogic::RequestStartGameDelegate(BaseEventDataPtr pEventData)
{
	ChangeState(BGS_WAITINGFORPLAYERS);
}

void GameDemoLogic::EnvironmentLoadedDelegate(BaseEventDataPtr pEventData)
{
	++mHumanGamesLoaded;
}


// FUTURE WORK - this isn't Demos specific so it can go into the game agnostic base class
void GameDemoLogic::RemoteClientDelegate(BaseEventDataPtr pEventData)
{
	// This event is always sent from clients to the game server.

	eastl::shared_ptr<EventDataRemoteClient> pCastEventData = 
		eastl::static_pointer_cast<EventDataRemoteClient>(pEventData);
	const int sockID = pCastEventData->GetSocketId();
	const int ipAddress = pCastEventData->GetIpAddress();

	// go find a NetworkGameView that doesn't have a socket ID, and attach this client to that view.
	GameApplication* gameApp = (GameApplication*)Application::App;
	const GameViewList& gameViews = gameApp->GetGameViews();
	for (auto it = gameViews.begin(); it != gameViews.end(); ++it)
	{
		eastl::shared_ptr<BaseGameView> pView = *it;
		if (pView->GetType() == GV_REMOTE)
		{
			eastl::shared_ptr<NetworkGameView> pNetworkGameView = 
				eastl::static_pointer_cast<NetworkGameView, BaseGameView>(pView);
			if (!pNetworkGameView->HasRemotePlayerAttached())
			{
				pNetworkGameView->AttachRemotePlayer(sockID);
				CreateNetworkEventForwarder(sockID);
				mHumanPlayersAttached++;

				return;
			}
		}
	}

}

void GameDemoLogic::NetworkPlayerActorAssignmentDelegate(BaseEventDataPtr pEventData)
{
	if (!mIsProxy)
		return;

	// we're a remote client getting an actor assignment.
	// the server assigned us a playerId when we first attached (the server's socketId, actually)
	eastl::shared_ptr<EventDataNetworkPlayerActorAssignment> pCastEventData =
		eastl::static_pointer_cast<EventDataNetworkPlayerActorAssignment>(pEventData);

	if (pCastEventData->GetActorId() == INVALID_ACTOR_ID)
	{
		mRemotePlayerId = pCastEventData->GetSocketId();
		return;
	}

	GameApplication* gameApp = (GameApplication*)Application::App;
	const GameViewList& gameViews = gameApp->GetGameViews();
	for (auto it = gameViews.begin(); it != gameViews.end(); ++it)
	{
		eastl::shared_ptr<BaseGameView> pView = *it;
		if (pView->GetType() == GV_HUMAN)
		{
			eastl::shared_ptr<GameDemoHumanView> pHumanView =
				eastl::static_pointer_cast<GameDemoHumanView, BaseGameView>(pView);
			if (mRemotePlayerId == pCastEventData->GetSocketId())
				pHumanView->SetControlledActor(pCastEventData->GetActorId());

			return;
		}
	}

	LogError("Could not find HumanView to attach actor to!");
}

void GameDemoLogic::JumpActorDelegate(BaseEventDataPtr pEventData)
{
	eastl::shared_ptr<EventDataJumpActor> pCastEventData =
		eastl::static_pointer_cast<EventDataJumpActor>(pEventData);

	eastl::shared_ptr<Actor> pGameActor(
		GameLogic::Get()->GetActor(pCastEventData->GetId()).lock());
	if (pGameActor)
	{
		eastl::shared_ptr<PhysicComponent> pPhysicalComponent =
			pGameActor->GetComponent<PhysicComponent>(PhysicComponent::Name).lock();
		if (pPhysicalComponent)
			pPhysicalComponent->KinematicJump(pCastEventData->GetDirection());
	}
}

void GameDemoLogic::MoveActorDelegate(BaseEventDataPtr pEventData)
{
	eastl::shared_ptr<EventDataMoveActor> pCastEventData =
		eastl::static_pointer_cast<EventDataMoveActor>(pEventData);

	eastl::shared_ptr<Actor> pGameActor(
		GameLogic::Get()->GetActor(pCastEventData->GetId()).lock());
	if (pGameActor)
	{
		eastl::shared_ptr<PhysicComponent> pPhysicalComponent =
			pGameActor->GetComponent<PhysicComponent>(PhysicComponent::Name).lock();
		if (pPhysicalComponent)
			pPhysicalComponent->KinematicMove(pCastEventData->GetDirection());
	}
}

void GameDemoLogic::RotateActorDelegate(BaseEventDataPtr pEventData)
{
	eastl::shared_ptr<EventDataRotateActor> pCastEventData =
		eastl::static_pointer_cast<EventDataRotateActor>(pEventData);

	eastl::shared_ptr<Actor> pGameActor(
		GameLogic::Get()->GetActor(pCastEventData->GetId()).lock());
	if (pGameActor)
	{
		eastl::shared_ptr<PhysicComponent> pPhysicalComponent =
			pGameActor->GetComponent<PhysicComponent>(PhysicComponent::Name).lock();
		if (pPhysicalComponent)
			pPhysicalComponent->SetRotation(pCastEventData->GetTransform());
	}
}

void GameDemoLogic::StartThrustDelegate(BaseEventDataPtr pEventData)
{
	eastl::shared_ptr<EventDataStartThrust> pCastEventData =
		eastl::static_pointer_cast<EventDataStartThrust>(pEventData);
	eastl::shared_ptr<Actor> pActor = GetActor(pCastEventData->GetActorId()).lock();
	if (pActor)
	{
		eastl::shared_ptr<PhysicComponent> pPhysicalComponent = 
			pActor->GetComponent<PhysicComponent>(PhysicComponent::Name).lock();
		if (pPhysicalComponent)
			pPhysicalComponent->ApplyAcceleration(pCastEventData->GetAcceleration());
	}
}

void GameDemoLogic::EndThrustDelegate(BaseEventDataPtr pEventData)
{
	eastl::shared_ptr<EventDataStartThrust> pCastEventData =
		eastl::static_pointer_cast<EventDataStartThrust>(pEventData);
	eastl::shared_ptr<Actor> pActor = GetActor(pCastEventData->GetActorId()).lock();
	if (pActor)
	{
		eastl::shared_ptr<PhysicComponent> pPhysicalComponent =
			pActor->GetComponent<PhysicComponent>(PhysicComponent::Name).lock();
		if (pPhysicalComponent)
			pPhysicalComponent->RemoveAcceleration();
	}
}

void GameDemoLogic::StartSteerDelegate(BaseEventDataPtr pEventData)
{
	eastl::shared_ptr<EventDataStartSteer> pCastEventData =
		eastl::static_pointer_cast<EventDataStartSteer>(pEventData);
	eastl::shared_ptr<Actor> pActor = GetActor(pCastEventData->GetActorId()).lock();
	if (pActor)
	{
		eastl::shared_ptr<PhysicComponent> pPhysicalComponent = 
			pActor->GetComponent<PhysicComponent>(PhysicComponent::Name).lock();
		if (pPhysicalComponent)
			pPhysicalComponent->ApplyAngularAcceleration(pCastEventData->GetAcceleration());
	}
}

void GameDemoLogic::EndSteerDelegate(BaseEventDataPtr pEventData)
{
	eastl::shared_ptr<EventDataStartSteer> pCastEventData =
		eastl::static_pointer_cast<EventDataStartSteer>(pEventData);
	eastl::shared_ptr<Actor> pActor = GetActor(pCastEventData->GetActorId()).lock();
	if (pActor)
	{
		eastl::shared_ptr<PhysicComponent> pPhysicalComponent =
			pActor->GetComponent<PhysicComponent>(PhysicComponent::Name).lock();
		if (pPhysicalComponent)
			pPhysicalComponent->RemoveAngularAcceleration();
	}
}


void GameDemoLogic::RegisterAllDelegates(void)
{
	// FUTURE WORK: Lots of these functions are ok to go into the base game logic!
	BaseEventManager* pGlobalEventManager = BaseEventManager::Get();
	pGlobalEventManager->AddListener(
		MakeDelegate(this, &GameDemoLogic::RemoteClientDelegate), 
		EventDataRemoteClient::skEventType);
	pGlobalEventManager->AddListener(
		MakeDelegate(this, &GameDemoLogic::SyncActorDelegate),
		EventDataSyncActor::skEventType);
	pGlobalEventManager->AddListener(
		MakeDelegate(this, &GameDemoLogic::JumpActorDelegate),
		EventDataJumpActor::skEventType);
	pGlobalEventManager->AddListener(
		MakeDelegate(this, &GameDemoLogic::MoveActorDelegate), 
		EventDataMoveActor::skEventType);
	pGlobalEventManager->AddListener(
		MakeDelegate(this, &GameDemoLogic::RotateActorDelegate),
		EventDataRotateActor::skEventType);
	pGlobalEventManager->AddListener(
		MakeDelegate(this, &GameDemoLogic::RequestStartGameDelegate), 
		EventDataRequestStartGame::skEventType);
	pGlobalEventManager->AddListener(
		MakeDelegate(this, &GameDemoLogic::NetworkPlayerActorAssignmentDelegate), 
		EventDataNetworkPlayerActorAssignment::skEventType);
	pGlobalEventManager->AddListener(
		MakeDelegate(this, &GameDemoLogic::EnvironmentLoadedDelegate), 
		EventDataEnvironmentLoaded::skEventType);
	pGlobalEventManager->AddListener(
		MakeDelegate(this, &GameDemoLogic::EnvironmentLoadedDelegate), 
		EventDataRemoteEnvironmentLoaded::skEventType);

	pGlobalEventManager->AddListener(
		MakeDelegate(this, &GameDemoLogic::StartThrustDelegate), 
		EventDataStartThrust::skEventType);
	pGlobalEventManager->AddListener(
		MakeDelegate(this, &GameDemoLogic::EndThrustDelegate), 
		EventDataEndThrust::skEventType);
	pGlobalEventManager->AddListener(
		MakeDelegate(this, &GameDemoLogic::StartSteerDelegate), 
		EventDataStartSteer::skEventType);
	pGlobalEventManager->AddListener(
		MakeDelegate(this, &GameDemoLogic::EndSteerDelegate), 
		EventDataEndSteer::skEventType);
}

void GameDemoLogic::RemoveAllDelegates(void)
{
	// FUTURE WORK: See the note in RegisterDelegates above....
	BaseEventManager* pGlobalEventManager = BaseEventManager::Get();
	pGlobalEventManager->RemoveListener(
		MakeDelegate(this, &GameDemoLogic::RemoteClientDelegate), 
		EventDataRemoteClient::skEventType);
	pGlobalEventManager->RemoveListener(
		MakeDelegate(this, &GameDemoLogic::SyncActorDelegate),
		EventDataSyncActor::skEventType);
	pGlobalEventManager->RemoveListener(
		MakeDelegate(this, &GameDemoLogic::JumpActorDelegate),
		EventDataJumpActor::skEventType);
	pGlobalEventManager->RemoveListener(
		MakeDelegate(this, &GameDemoLogic::MoveActorDelegate), 
		EventDataMoveActor::skEventType);
	pGlobalEventManager->RemoveListener(
		MakeDelegate(this, &GameDemoLogic::RotateActorDelegate),
		EventDataRotateActor::skEventType);
	pGlobalEventManager->RemoveListener(
		MakeDelegate(this, &GameDemoLogic::RequestStartGameDelegate), 
		EventDataRequestStartGame::skEventType);
	pGlobalEventManager->RemoveListener(
		MakeDelegate(this, &GameDemoLogic::NetworkPlayerActorAssignmentDelegate), 
		EventDataNetworkPlayerActorAssignment::skEventType);
	pGlobalEventManager->RemoveListener(
		MakeDelegate(this, &GameDemoLogic::EnvironmentLoadedDelegate), 
		EventDataEnvironmentLoaded::skEventType);
	pGlobalEventManager->RemoveListener(
		MakeDelegate(this, &GameDemoLogic::EnvironmentLoadedDelegate), 
		EventDataRemoteEnvironmentLoaded::skEventType);
	if (mIsProxy)
	{
		pGlobalEventManager->RemoveListener(
			MakeDelegate(this, &GameDemoLogic::RequestNewActorDelegate), 
			EventDataRequestNewActor::skEventType);
	}

	pGlobalEventManager->RemoveListener(
		MakeDelegate(this, &GameDemoLogic::StartThrustDelegate), 
		EventDataStartThrust::skEventType);
	pGlobalEventManager->RemoveListener(
		MakeDelegate(this, &GameDemoLogic::EndThrustDelegate), 
		EventDataEndThrust::skEventType);
	pGlobalEventManager->RemoveListener(
		MakeDelegate(this, &GameDemoLogic::StartSteerDelegate), 
		EventDataStartSteer::skEventType);
	pGlobalEventManager->RemoveListener(
		MakeDelegate(this, &GameDemoLogic::EndSteerDelegate), 
		EventDataEndSteer::skEventType);
}

void GameDemoLogic::CreateNetworkEventForwarder(const int socketId)
{
	NetworkEventForwarder* pNetworkEventForwarder = new NetworkEventForwarder(socketId);

	BaseEventManager* pGlobalEventManager = BaseEventManager::Get();

	// then add those events that need to be sent along to amy attached clients
	pGlobalEventManager->AddListener(
		MakeDelegate(pNetworkEventForwarder, &NetworkEventForwarder::ForwardEvent),
		EventDataPhysTriggerEnter::skEventType);
	pGlobalEventManager->AddListener(
		MakeDelegate(pNetworkEventForwarder, &NetworkEventForwarder::ForwardEvent),
		EventDataPhysTriggerLeave::skEventType);
	pGlobalEventManager->AddListener(
		MakeDelegate(pNetworkEventForwarder, &NetworkEventForwarder::ForwardEvent), 
		EventDataPhysCollision::skEventType);
	pGlobalEventManager->AddListener(
		MakeDelegate(pNetworkEventForwarder, &NetworkEventForwarder::ForwardEvent),
		EventDataPhysSeparation::skEventType);
	pGlobalEventManager->AddListener(
		MakeDelegate(pNetworkEventForwarder, &NetworkEventForwarder::ForwardEvent), 
		EventDataDestroyActor::skEventType);
	pGlobalEventManager->AddListener(
		MakeDelegate(pNetworkEventForwarder, &NetworkEventForwarder::ForwardEvent), 
		EventDataEnvironmentLoaded::skEventType);
	pGlobalEventManager->AddListener(
		MakeDelegate(pNetworkEventForwarder, &NetworkEventForwarder::ForwardEvent), 
		EventDataNewActor::skEventType);
	pGlobalEventManager->AddListener(
		MakeDelegate(pNetworkEventForwarder, &NetworkEventForwarder::ForwardEvent),
		EventDataSyncActor::skEventType);
	pGlobalEventManager->AddListener(
		MakeDelegate(pNetworkEventForwarder, &NetworkEventForwarder::ForwardEvent),
		EventDataJumpActor::skEventType);
	pGlobalEventManager->AddListener(
		MakeDelegate(pNetworkEventForwarder, &NetworkEventForwarder::ForwardEvent), 
		EventDataMoveActor::skEventType);
	pGlobalEventManager->AddListener(
		MakeDelegate(pNetworkEventForwarder, &NetworkEventForwarder::ForwardEvent),
		EventDataRotateActor::skEventType);
	pGlobalEventManager->AddListener(
		MakeDelegate(pNetworkEventForwarder, &NetworkEventForwarder::ForwardEvent), 
		EventDataRequestNewActor::skEventType);
	pGlobalEventManager->AddListener(
		MakeDelegate(pNetworkEventForwarder, &NetworkEventForwarder::ForwardEvent), 
		EventDataNetworkPlayerActorAssignment::skEventType);

	mNetworkEventForwarders.push_back(pNetworkEventForwarder);
}

void GameDemoLogic::DestroyAllNetworkEventForwarders(void)
{
	for (auto it = mNetworkEventForwarders.begin(); it != mNetworkEventForwarders.end(); ++it)
	{
		NetworkEventForwarder* networkEventForwarder = (*it);

		BaseEventManager* eventManager = BaseEventManager::Get();
		eventManager->RemoveListener(
			MakeDelegate(networkEventForwarder, &NetworkEventForwarder::ForwardEvent),
			EventDataPhysTriggerEnter::skEventType);
		eventManager->RemoveListener(
			MakeDelegate(networkEventForwarder, &NetworkEventForwarder::ForwardEvent),
			EventDataPhysTriggerLeave::skEventType);
		eventManager->RemoveListener(
			MakeDelegate(networkEventForwarder, &NetworkEventForwarder::ForwardEvent), 
			EventDataPhysCollision::skEventType);
		eventManager->RemoveListener(
			MakeDelegate(networkEventForwarder, &NetworkEventForwarder::ForwardEvent),
			EventDataPhysSeparation::skEventType);
		eventManager->RemoveListener(
			MakeDelegate(networkEventForwarder, &NetworkEventForwarder::ForwardEvent), 
			EventDataDestroyActor::skEventType);
		eventManager->RemoveListener(
			MakeDelegate(networkEventForwarder, &NetworkEventForwarder::ForwardEvent), 
			EventDataEnvironmentLoaded::skEventType);
		eventManager->RemoveListener(
			MakeDelegate(networkEventForwarder, &NetworkEventForwarder::ForwardEvent), 
			EventDataNewActor::skEventType);
		eventManager->RemoveListener(
			MakeDelegate(networkEventForwarder, &NetworkEventForwarder::ForwardEvent),
			EventDataSyncActor::skEventType);
		eventManager->RemoveListener(
			MakeDelegate(networkEventForwarder, &NetworkEventForwarder::ForwardEvent),
			EventDataJumpActor::skEventType);
		eventManager->RemoveListener(
			MakeDelegate(networkEventForwarder, &NetworkEventForwarder::ForwardEvent), 
			EventDataMoveActor::skEventType);
		eventManager->RemoveListener(
			MakeDelegate(networkEventForwarder, &NetworkEventForwarder::ForwardEvent),
			EventDataRotateActor::skEventType);
		eventManager->RemoveListener(
			MakeDelegate(networkEventForwarder, &NetworkEventForwarder::ForwardEvent), 
			EventDataRequestNewActor::skEventType);
		eventManager->RemoveListener(
			MakeDelegate(networkEventForwarder, &NetworkEventForwarder::ForwardEvent), 
			EventDataNetworkPlayerActorAssignment::skEventType);

		delete networkEventForwarder;
	}

	mNetworkEventForwarders.clear();
}

LevelManager* GameDemoLogic::CreateLevelManager(void)
{
	GameDemoManager* demoManager = new GameDemoManager();
	demoManager->AddLevelSearchDir(L"world/demo/");
	demoManager->LoadLevelList(L"*.xml");
	return demoManager;
}

AIManager* GameDemoLogic::CreateAIManager(void)
{
	GameDemoAIManager* demoAIManager = new GameDemoAIManager();
	return demoAIManager;
}

bool GameDemoLogic::LoadGameDelegate(tinyxml2::XMLElement* pLevelData)
{
	return true;
}