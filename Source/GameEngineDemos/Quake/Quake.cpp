//========================================================================
//
// QuakeLogic Implementation       - Chapter 21, page 725
//
//========================================================================

#include "QuakeView.h"

#include "Physic/Physic.h"
#include "Physic/PhysicEventListener.h"
#include "Physic/Importer/PhysicResource.h"

#include "Quake.h"
#include "QuakeApp.h"
#include "QuakeNetwork.h"
#include "QuakeEvents.h"
#include "QuakeActorFactory.h"
#include "QuakeLevelManager.h"

//
// QuakeLogic::QuakeLogic
//
QuakeLogic::QuakeLogic() : GameLogic()
{
	mPhysics.reset(CreateGamePhysics());
	RegisterAllDelegates();
}


//
// QuakeLogic::~QuakeLogic
//
QuakeLogic::~QuakeLogic()
{
	RemoveAllDelegates();
	DestroyAllNetworkEventForwarders();
}


void QuakeLogic::UpdateViewType(const eastl::shared_ptr<BaseGameView>& pView, bool add)
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

void QuakeLogic::ResetViewType()
{
	GameLogic::ResetViewType();

	mHumanPlayersAttached = 0;
	mAIPlayersAttached = 0;
}

void QuakeLogic::SetProxy()
{
	GameLogic::SetProxy();
}


//
// QuakeLogic::ChangeState
//
void QuakeLogic::ChangeState(BaseGameState newState)
{
	GameLogic::ChangeState(newState);

	GameApplication* gameApp = (GameApplication*)Application::App;
	switch (newState)
	{
		case BGS_MAINMENU:
		{
			eastl::shared_ptr<BaseGameView> menuView(new QuakeMainMenuView());
			gameApp->AddView(menuView);

			break;
		}

		case BGS_WAITINGFORPLAYERS:
		{
			// spawn all local players (should only be one, though we might support more in the future)
			LogAssert(mExpectedPlayers == 1, "needs only one player");
			for (int i = 0; i < mExpectedPlayers; ++i)
			{
				eastl::shared_ptr<BaseGameView> playersView(new QuakeHumanView());
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
				eastl::shared_ptr<BaseGameView> aiView(new QuakeAIPlayerView(eastl::shared_ptr<PathingGraph>()));
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
					eastl::shared_ptr<PlayerActor> pPlayerActor = 
						CreatePlayerActor("actors\\quake\\players\\player.xml", NULL);
					if (pPlayerActor)
					{
						PlayerSpawn(pPlayerActor);
						pView->OnAttach(pView->GetId(), pPlayerActor->GetId());

						eastl::shared_ptr<EventDataNewActor> pNewActorEvent(
							new EventDataNewActor(pPlayerActor->GetId(), pView->GetId()));
						BaseEventManager::Get()->TriggerEvent(pNewActorEvent);
					}
				}
				else if (pView->GetType() == GV_REMOTE)
				{
					eastl::shared_ptr<NetworkGameView> pNetworkGameView =
						eastl::static_pointer_cast<NetworkGameView, BaseGameView>(pView);
					eastl::shared_ptr<PlayerActor> pPlayerActor =
						CreatePlayerActor("actors\\quake\\players\\remote_player.xml", NULL);
					if (pPlayerActor)
					{
						PlayerSpawn(pPlayerActor);
						pView->OnAttach(pView->GetId(), pPlayerActor->GetId());

						eastl::shared_ptr<EventDataNewActor> pNewActorEvent(
							new EventDataNewActor(pPlayerActor->GetId(), pNetworkGameView->GetId()));
						BaseEventManager::Get()->QueueEvent(pNewActorEvent);
					}
				}
				else if (pView->GetType() == GV_AI)
				{
					eastl::shared_ptr<QuakeAIPlayerView> pAiView = 
						eastl::static_pointer_cast<QuakeAIPlayerView, BaseGameView>(pView);
					eastl::shared_ptr<PlayerActor> pPlayerActor =
						CreatePlayerActor("actors\\quake\\players\\ai_player.xml", NULL);
					if (pPlayerActor)
					{
						PlayerSpawn(pPlayerActor);
						pView->OnAttach(pView->GetId(), pPlayerActor->GetId());

						eastl::shared_ptr<EventDataNewActor> pNewActorEvent(
							new EventDataNewActor(pPlayerActor->GetId(), pAiView->GetId()));
						BaseEventManager::Get()->QueueEvent(pNewActorEvent);
					}
				}
			}

			break;
		}
	}
}

void QuakeLogic::SyncActor(const ActorId id, Transform const &transform)
{
	GameLogic::SyncActor(id, transform);
}

void QuakeLogic::RequestStartGameDelegate(BaseEventDataPtr pEventData)
{
	ChangeState(BGS_WAITINGFORPLAYERS);
}

void QuakeLogic::EnvironmentLoadedDelegate(BaseEventDataPtr pEventData)
{
	++mHumanGamesLoaded;
}

// FUTURE WORK - this isn't Quake specific so it can go into the game agnostic base class
void QuakeLogic::RemoteClientDelegate(BaseEventDataPtr pEventData)
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

void QuakeLogic::NetworkPlayerActorAssignmentDelegate(BaseEventDataPtr pEventData)
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
			eastl::shared_ptr<QuakeHumanView> pHumanView =
				eastl::static_pointer_cast<QuakeHumanView, BaseGameView>(pView);
			if (mRemotePlayerId == pCastEventData->GetSocketId())
				pHumanView->SetControlledActor(pCastEventData->GetActorId());

			return;
		}
	}

	LogError("Could not find HumanView to attach actor to!");
}

void QuakeLogic::PushActorDelegate(BaseEventDataPtr pEventData)
{
	eastl::shared_ptr<QuakeEventDataPushActor> pCastEventData =
		eastl::static_pointer_cast<QuakeEventDataPushActor>(pEventData);

	eastl::shared_ptr<Actor> pGameActor(
		GameLogic::Get()->GetActor(pCastEventData->GetId()).lock());
	if (pGameActor)
	{
		eastl::shared_ptr<PhysicComponent> pPhysicalComponent =
			pGameActor->GetComponent<PhysicComponent>(PhysicComponent::Name).lock();
		if (pPhysicalComponent)
			pPhysicalComponent->ApplyForce(pCastEventData->GetDirection());
	}
}

void QuakeLogic::JumpActorDelegate(BaseEventDataPtr pEventData)
{
	eastl::shared_ptr<QuakeEventDataJumpActor> pCastEventData =
		eastl::static_pointer_cast<QuakeEventDataJumpActor>(pEventData);

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

void QuakeLogic::SpawnActorDelegate(BaseEventDataPtr pEventData)
{
	eastl::shared_ptr<QuakeEventDataSpawnActor> pCastEventData =
		eastl::static_pointer_cast<QuakeEventDataSpawnActor>(pEventData);

	eastl::shared_ptr<Actor> pGameActor(
		GameLogic::Get()->GetActor(pCastEventData->GetId()).lock());
	if (pGameActor)
	{
		eastl::shared_ptr<PhysicComponent> pPhysicalComponent =
			pGameActor->GetComponent<PhysicComponent>(PhysicComponent::Name).lock();
		if (pPhysicalComponent)
			pPhysicalComponent->SetTransform(pCastEventData->GetTransform());
	}
}

void QuakeLogic::MoveActorDelegate(BaseEventDataPtr pEventData)
{
	eastl::shared_ptr<QuakeEventDataMoveActor> pCastEventData =
		eastl::static_pointer_cast<QuakeEventDataMoveActor>(pEventData);

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

void QuakeLogic::FallActorDelegate(BaseEventDataPtr pEventData)
{
	eastl::shared_ptr<QuakeEventDataFallActor> pCastEventData =
		eastl::static_pointer_cast<QuakeEventDataFallActor>(pEventData);

	eastl::shared_ptr<Actor> pGameActor(
		GameLogic::Get()->GetActor(pCastEventData->GetId()).lock());
	if (pGameActor)
	{
		eastl::shared_ptr<PhysicComponent> pPhysicalComponent =
			pGameActor->GetComponent<PhysicComponent>(PhysicComponent::Name).lock();
		if (pPhysicalComponent)
			pPhysicalComponent->KinematicFall(pCastEventData->GetDirection());
	}
}

void QuakeLogic::RotateActorDelegate(BaseEventDataPtr pEventData)
{
	eastl::shared_ptr<QuakeEventDataRotateActor> pCastEventData =
		eastl::static_pointer_cast<QuakeEventDataRotateActor>(pEventData);

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

void QuakeLogic::StartThrustDelegate(BaseEventDataPtr pEventData)
{
	eastl::shared_ptr<QuakeEventDataStartThrust> pCastEventData =
		eastl::static_pointer_cast<QuakeEventDataStartThrust>(pEventData);
	eastl::shared_ptr<Actor> pActor = GetActor(pCastEventData->GetActorId()).lock();
	if (pActor)
	{
		eastl::shared_ptr<PhysicComponent> pPhysicalComponent = 
			pActor->GetComponent<PhysicComponent>(PhysicComponent::Name).lock();
		if (pPhysicalComponent)
			pPhysicalComponent->ApplyAcceleration(pCastEventData->GetAcceleration());
	}
}

void QuakeLogic::EndThrustDelegate(BaseEventDataPtr pEventData)
{
	eastl::shared_ptr<QuakeEventDataStartThrust> pCastEventData =
		eastl::static_pointer_cast<QuakeEventDataStartThrust>(pEventData);
	eastl::shared_ptr<Actor> pActor = GetActor(pCastEventData->GetActorId()).lock();
	if (pActor)
	{
		eastl::shared_ptr<PhysicComponent> pPhysicalComponent =
			pActor->GetComponent<PhysicComponent>(PhysicComponent::Name).lock();
		if (pPhysicalComponent)
			pPhysicalComponent->RemoveAcceleration();
	}
}

void QuakeLogic::StartSteerDelegate(BaseEventDataPtr pEventData)
{
	eastl::shared_ptr<QuakeEventDataStartSteer> pCastEventData =
		eastl::static_pointer_cast<QuakeEventDataStartSteer>(pEventData);
	eastl::shared_ptr<Actor> pActor = GetActor(pCastEventData->GetActorId()).lock();
	if (pActor)
	{
		eastl::shared_ptr<PhysicComponent> pPhysicalComponent = 
			pActor->GetComponent<PhysicComponent>(PhysicComponent::Name).lock();
		if (pPhysicalComponent)
			pPhysicalComponent->ApplyAngularAcceleration(pCastEventData->GetAcceleration());
	}
}

void QuakeLogic::EndSteerDelegate(BaseEventDataPtr pEventData)
{
	eastl::shared_ptr<QuakeEventDataStartSteer> pCastEventData =
		eastl::static_pointer_cast<QuakeEventDataStartSteer>(pEventData);
	eastl::shared_ptr<Actor> pActor = GetActor(pCastEventData->GetActorId()).lock();
	if (pActor)
	{
		eastl::shared_ptr<PhysicComponent> pPhysicalComponent =
			pActor->GetComponent<PhysicComponent>(PhysicComponent::Name).lock();
		if (pPhysicalComponent)
			pPhysicalComponent->RemoveAngularAcceleration();
	}
}


void QuakeLogic::RegisterAllDelegates(void)
{
	// FUTURE WORK: Lots of these functions are ok to go into the base game logic!
	BaseEventManager* pGlobalEventManager = BaseEventManager::Get();
	pGlobalEventManager->AddListener(
		MakeDelegate(this, &QuakeLogic::RemoteClientDelegate), 
		EventDataRemoteClient::skEventType);
	pGlobalEventManager->AddListener(
		MakeDelegate(this, &QuakeLogic::SyncActorDelegate),
		EventDataSyncActor::skEventType);
	pGlobalEventManager->AddListener(
		MakeDelegate(this, &QuakeLogic::RequestStartGameDelegate), 
		EventDataRequestStartGame::skEventType);
	pGlobalEventManager->AddListener(
		MakeDelegate(this, &QuakeLogic::NetworkPlayerActorAssignmentDelegate), 
		EventDataNetworkPlayerActorAssignment::skEventType);
	pGlobalEventManager->AddListener(
		MakeDelegate(this, &QuakeLogic::EnvironmentLoadedDelegate), 
		EventDataEnvironmentLoaded::skEventType);
	pGlobalEventManager->AddListener(
		MakeDelegate(this, &QuakeLogic::EnvironmentLoadedDelegate), 
		EventDataRemoteEnvironmentLoaded::skEventType);
	pGlobalEventManager->AddListener(
		MakeDelegate(this, &QuakeLogic::PhysicsTriggerEnterDelegate),
		EventDataPhysTriggerEnter::skEventType);
	pGlobalEventManager->AddListener(
		MakeDelegate(this, &QuakeLogic::PhysicsTriggerLeaveDelegate),
		EventDataPhysTriggerLeave::skEventType);
	pGlobalEventManager->AddListener(
		MakeDelegate(this, &QuakeLogic::PhysicsCollisionDelegate),
		EventDataPhysCollision::skEventType);
	pGlobalEventManager->AddListener(
		MakeDelegate(this, &QuakeLogic::PhysicsSeparationDelegate),
		EventDataPhysSeparation::skEventType);

	pGlobalEventManager->AddListener(
		MakeDelegate(this, &QuakeLogic::FireWeaponDelegate),
		QuakeEventDataFireWeapon::skEventType);
	pGlobalEventManager->AddListener(
		MakeDelegate(this, &QuakeLogic::SpawnActorDelegate),
		QuakeEventDataSpawnActor::skEventType);
	pGlobalEventManager->AddListener(
		MakeDelegate(this, &QuakeLogic::PushActorDelegate),
		QuakeEventDataPushActor::skEventType);
	pGlobalEventManager->AddListener(
		MakeDelegate(this, &QuakeLogic::JumpActorDelegate),
		QuakeEventDataJumpActor::skEventType);
	pGlobalEventManager->AddListener(
		MakeDelegate(this, &QuakeLogic::MoveActorDelegate),
		QuakeEventDataMoveActor::skEventType);
	pGlobalEventManager->AddListener(
		MakeDelegate(this, &QuakeLogic::FallActorDelegate),
		QuakeEventDataFallActor::skEventType);
	pGlobalEventManager->AddListener(
		MakeDelegate(this, &QuakeLogic::RotateActorDelegate),
		QuakeEventDataRotateActor::skEventType);
	pGlobalEventManager->AddListener(
		MakeDelegate(this, &QuakeLogic::StartThrustDelegate), 
		QuakeEventDataStartThrust::skEventType);
	pGlobalEventManager->AddListener(
		MakeDelegate(this, &QuakeLogic::EndThrustDelegate), 
		QuakeEventDataEndThrust::skEventType);
	pGlobalEventManager->AddListener(
		MakeDelegate(this, &QuakeLogic::StartSteerDelegate), 
		QuakeEventDataStartSteer::skEventType);
	pGlobalEventManager->AddListener(
		MakeDelegate(this, &QuakeLogic::EndSteerDelegate), 
		QuakeEventDataEndSteer::skEventType);
}

void QuakeLogic::RemoveAllDelegates(void)
{
	// FUTURE WORK: See the note in RegisterDelegates above....
	BaseEventManager* pGlobalEventManager = BaseEventManager::Get();
	pGlobalEventManager->RemoveListener(
		MakeDelegate(this, &QuakeLogic::RemoteClientDelegate), 
		EventDataRemoteClient::skEventType);
	pGlobalEventManager->RemoveListener(
		MakeDelegate(this, &QuakeLogic::SyncActorDelegate),
		EventDataSyncActor::skEventType);
	pGlobalEventManager->RemoveListener(
		MakeDelegate(this, &QuakeLogic::RequestStartGameDelegate), 
		EventDataRequestStartGame::skEventType);
	pGlobalEventManager->RemoveListener(
		MakeDelegate(this, &QuakeLogic::NetworkPlayerActorAssignmentDelegate), 
		EventDataNetworkPlayerActorAssignment::skEventType);
	pGlobalEventManager->RemoveListener(
		MakeDelegate(this, &QuakeLogic::EnvironmentLoadedDelegate), 
		EventDataEnvironmentLoaded::skEventType);
	pGlobalEventManager->RemoveListener(
		MakeDelegate(this, &QuakeLogic::EnvironmentLoadedDelegate), 
		EventDataRemoteEnvironmentLoaded::skEventType);
	pGlobalEventManager->RemoveListener(
		MakeDelegate(this, &QuakeLogic::PhysicsTriggerEnterDelegate),
		EventDataPhysTriggerEnter::skEventType);
	pGlobalEventManager->RemoveListener(
		MakeDelegate(this, &QuakeLogic::PhysicsTriggerLeaveDelegate),
		EventDataPhysTriggerLeave::skEventType);
	pGlobalEventManager->RemoveListener(
		MakeDelegate(this, &QuakeLogic::PhysicsCollisionDelegate),
		EventDataPhysCollision::skEventType);
	pGlobalEventManager->RemoveListener(
		MakeDelegate(this, &QuakeLogic::PhysicsSeparationDelegate),
		EventDataPhysSeparation::skEventType);
	if (mIsProxy)
	{
		pGlobalEventManager->RemoveListener(
			MakeDelegate(this, &QuakeLogic::RequestNewActorDelegate), 
			EventDataRequestNewActor::skEventType);
	}

	pGlobalEventManager->RemoveListener(
		MakeDelegate(this, &QuakeLogic::FireWeaponDelegate),
		QuakeEventDataFireWeapon::skEventType);
	pGlobalEventManager->RemoveListener(
		MakeDelegate(this, &QuakeLogic::SpawnActorDelegate),
		QuakeEventDataSpawnActor::skEventType);
	pGlobalEventManager->RemoveListener(
		MakeDelegate(this, &QuakeLogic::PushActorDelegate),
		QuakeEventDataPushActor::skEventType);
	pGlobalEventManager->RemoveListener(
		MakeDelegate(this, &QuakeLogic::JumpActorDelegate),
		QuakeEventDataJumpActor::skEventType);
	pGlobalEventManager->RemoveListener(
		MakeDelegate(this, &QuakeLogic::MoveActorDelegate),
		QuakeEventDataMoveActor::skEventType);
	pGlobalEventManager->RemoveListener(
		MakeDelegate(this, &QuakeLogic::FallActorDelegate),
		QuakeEventDataFallActor::skEventType);
	pGlobalEventManager->RemoveListener(
		MakeDelegate(this, &QuakeLogic::RotateActorDelegate),
		QuakeEventDataRotateActor::skEventType);
	pGlobalEventManager->RemoveListener(
		MakeDelegate(this, &QuakeLogic::StartThrustDelegate), 
		QuakeEventDataStartThrust::skEventType);
	pGlobalEventManager->RemoveListener(
		MakeDelegate(this, &QuakeLogic::EndThrustDelegate), 
		QuakeEventDataEndThrust::skEventType);
	pGlobalEventManager->RemoveListener(
		MakeDelegate(this, &QuakeLogic::StartSteerDelegate), 
		QuakeEventDataStartSteer::skEventType);
	pGlobalEventManager->RemoveListener(
		MakeDelegate(this, &QuakeLogic::EndSteerDelegate), 
		QuakeEventDataEndSteer::skEventType);
}

void QuakeLogic::CreateNetworkEventForwarder(const int socketId)
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
		EventDataRequestNewActor::skEventType);
	pGlobalEventManager->AddListener(
		MakeDelegate(pNetworkEventForwarder, &NetworkEventForwarder::ForwardEvent), 
		EventDataNetworkPlayerActorAssignment::skEventType);

	pGlobalEventManager->AddListener(
		MakeDelegate(pNetworkEventForwarder, &NetworkEventForwarder::ForwardEvent),
		QuakeEventDataFireWeapon::skEventType);
	pGlobalEventManager->AddListener(
		MakeDelegate(pNetworkEventForwarder, &NetworkEventForwarder::ForwardEvent),
		QuakeEventDataChangeWeapon::skEventType);
	pGlobalEventManager->AddListener(
		MakeDelegate(pNetworkEventForwarder, &NetworkEventForwarder::ForwardEvent),
		QuakeEventDataDeadActor::skEventType);
	pGlobalEventManager->AddListener(
		MakeDelegate(pNetworkEventForwarder, &NetworkEventForwarder::ForwardEvent),
		QuakeEventDataSpawnActor::skEventType);
	pGlobalEventManager->AddListener(
		MakeDelegate(pNetworkEventForwarder, &NetworkEventForwarder::ForwardEvent),
		QuakeEventDataPushActor::skEventType);
	pGlobalEventManager->AddListener(
		MakeDelegate(pNetworkEventForwarder, &NetworkEventForwarder::ForwardEvent),
		QuakeEventDataJumpActor::skEventType);
	pGlobalEventManager->AddListener(
		MakeDelegate(pNetworkEventForwarder, &NetworkEventForwarder::ForwardEvent),
		QuakeEventDataMoveActor::skEventType);
	pGlobalEventManager->AddListener(
		MakeDelegate(pNetworkEventForwarder, &NetworkEventForwarder::ForwardEvent),
		QuakeEventDataFallActor::skEventType);
	pGlobalEventManager->AddListener(
		MakeDelegate(pNetworkEventForwarder, &NetworkEventForwarder::ForwardEvent),
		QuakeEventDataRotateActor::skEventType);

	mNetworkEventForwarders.push_back(pNetworkEventForwarder);
}

void QuakeLogic::DestroyAllNetworkEventForwarders(void)
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
			EventDataRequestNewActor::skEventType);
		eventManager->RemoveListener(
			MakeDelegate(networkEventForwarder, &NetworkEventForwarder::ForwardEvent), 
			EventDataNetworkPlayerActorAssignment::skEventType);

		eventManager->RemoveListener(
			MakeDelegate(networkEventForwarder, &NetworkEventForwarder::ForwardEvent),
			QuakeEventDataFireWeapon::skEventType);
		eventManager->RemoveListener(
			MakeDelegate(networkEventForwarder, &NetworkEventForwarder::ForwardEvent),
			QuakeEventDataChangeWeapon::skEventType);
		eventManager->RemoveListener(
			MakeDelegate(networkEventForwarder, &NetworkEventForwarder::ForwardEvent),
			QuakeEventDataDeadActor::skEventType);
		eventManager->RemoveListener(
			MakeDelegate(networkEventForwarder, &NetworkEventForwarder::ForwardEvent),
			QuakeEventDataSpawnActor::skEventType);
		eventManager->RemoveListener(
			MakeDelegate(networkEventForwarder, &NetworkEventForwarder::ForwardEvent),
			QuakeEventDataPushActor::skEventType);
		eventManager->RemoveListener(
			MakeDelegate(networkEventForwarder, &NetworkEventForwarder::ForwardEvent),
			QuakeEventDataJumpActor::skEventType);
		eventManager->RemoveListener(
			MakeDelegate(networkEventForwarder, &NetworkEventForwarder::ForwardEvent),
			QuakeEventDataMoveActor::skEventType);
		eventManager->RemoveListener(
			MakeDelegate(networkEventForwarder, &NetworkEventForwarder::ForwardEvent),
			QuakeEventDataFallActor::skEventType);
		eventManager->RemoveListener(
			MakeDelegate(networkEventForwarder, &NetworkEventForwarder::ForwardEvent),
			QuakeEventDataRotateActor::skEventType);

		delete networkEventForwarder;
	}

	mNetworkEventForwarders.clear();
}


ActorFactory* QuakeLogic::CreateActorFactory(void)
{
	return new QuakeActorFactory();
}

LevelManager* QuakeLogic::CreateLevelManager(void)
{
	QuakeLevelManager* levelManager = new QuakeLevelManager();
	levelManager->AddLevelSearchDir(L"world/quake/");
	levelManager->LoadLevelList(L"*.xml");
	return levelManager;
}


eastl::shared_ptr<PlayerActor> QuakeLogic::CreatePlayerActor(const eastl::string &actorResource,
	tinyxml2::XMLElement *overrides, const Transform *initialTransform, const ActorId serversActorId)
{
	QuakeActorFactory* actorFactory = dynamic_cast<QuakeActorFactory*>(mActorFactory);
	LogAssert(actorFactory, "quake actor factory is not initialized");
	if (!mIsProxy && serversActorId != INVALID_ACTOR_ID)
		return eastl::shared_ptr<PlayerActor>();

	if (mIsProxy && serversActorId == INVALID_ACTOR_ID)
		return eastl::shared_ptr<PlayerActor>();

	eastl::shared_ptr<PlayerActor> pActor = actorFactory->CreatePlayerActor(
		ToWideString(actorResource.c_str()).c_str(), overrides, initialTransform, serversActorId);
	if (pActor)
	{
		mActors.insert(eastl::make_pair(pActor->GetId(), pActor));
		if (!mIsProxy && (mGameState == BGS_SPAWNINGPLAYERACTORS || mGameState == BGS_RUNNING))
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
		return eastl::shared_ptr<PlayerActor>();
	}
}

bool QuakeLogic::LoadGameDelegate(tinyxml2::XMLElement* pLevelData)
{
	for (auto actor : mActors)
	{
		eastl::shared_ptr<Actor> pActor = actor.second;
		eastl::shared_ptr<PhysicComponent> pPhysicalComponent =
			pActor->GetComponent<PhysicComponent>(PhysicComponent::Name).lock();
		if (pPhysicalComponent)
		{
			if (pPhysicalComponent->GetShape() == "BSP")
			{
				eastl::shared_ptr<ResHandle>& resHandle =
					ResCache::Get()->GetHandle(&BaseResource(ToWideString(pPhysicalComponent->GetMesh().c_str())));
				if (resHandle)
				{
					eastl::map<eastl::string, eastl::string> modelResources;
					eastl::map<eastl::string, eastl::string> triggerResources;
					eastl::map<eastl::string, eastl::string> targetResources;

					modelResources["ammo_bullets"] = "actors/quake/models/ammo/bullet.xml";
					modelResources["ammo_cells"] = "actors/quake/models/ammo/cell.xml";
					modelResources["ammo_grenades"] = "actors/quake/models/ammo/grenade.xml";
					modelResources["ammo_lightning"] = "actors/quake/models/ammo/lightning.xml";
					modelResources["ammo_rockets"] = "actors/quake/models/ammo/rocket.xml";
					modelResources["ammo_shells"] = "actors/quake/models/ammo/shell.xml";
					modelResources["ammo_slugs"] = "actors/quake/models/ammo/slug.xml";
					modelResources["weapon_grenadelauncher"] = "actors/quake/models/weapon/grenadelauncher.xml";
					modelResources["weapon_lightning"] = "actors/quake/models/weapon/lightning.xml";
					modelResources["weapon_machinegun"] = "actors/quake/models/weapon/machinegun.xml";
					modelResources["weapon_plasmagun"] = "actors/quake/models/weapon/plasmagun.xml";
					modelResources["weapon_railgun"] = "actors/quake/models/weapon/railgun.xml";
					modelResources["weapon_shotgun"] = "actors/quake/models/weapon/shotgun.xml";
					modelResources["weapon_rocketlauncher"] = "actors/quake/models/weapon/rocketlauncher.xml";
					modelResources["item_armor_shard"] = "actors/quake/models/armor/armorshard.xml";
					modelResources["item_armor_combat"] = "actors/quake/models/armor/armorcombat.xml";
					modelResources["item_armor_body"] = "actors/quake/models/armor/armorbody.xml";
					modelResources["item_health_mega"] = "actors/quake/models/health/healthmega.xml";
					modelResources["item_health_small"] = "actors/quake/models/health/healthsmall.xml";
					modelResources["item_health_large"] = "actors/quake/models/health/healthlarge.xml";
					modelResources["item_health"] = "actors/quake/models/health/health.xml";
					targetResources["info_player_deathmatch"] = "actors/quake/target/location.xml";
					targetResources["target_speaker"] = "actors/quake/target/speaker.xml";
					triggerResources["trigger_teleport"] = "actors/quake/trigger/teleporter.xml";
					triggerResources["trigger_push"] = "actors/quake/trigger/push.xml";

					const eastl::shared_ptr<BspResourceExtraData>& extra =
						eastl::static_pointer_cast<BspResourceExtraData>(resHandle->GetExtra());
					BspLoader& bspLoader = extra->GetLoader();

					eastl::map<eastl::string, BSPEntity> targets;
					for (int i = 0; i < bspLoader.mNumEntities; i++)
					{
						const BSPEntity& entity = bspLoader.mEntities[i];
						eastl::string target = bspLoader.GetValueForKey(&entity, "targetname");
						if (!target.empty()) 
							targets[target] = entity;
					}

					for (int i = 0; i < bspLoader.mNumEntities; i++)
					{
						const BSPEntity& entity = bspLoader.mEntities[i];
						eastl::string className = bspLoader.GetValueForKey(&entity, "classname");

						if (modelResources.find(className) != modelResources.end())
						{
							eastl::string gameType = bspLoader.GetValueForKey(&entity, "gametype");
							eastl::string notGameType = bspLoader.GetValueForKey(&entity, "not_gametype");

							if (gameType.empty() && notGameType.empty() ||
								gameType.find("duel") != eastl::string::npos ||
								notGameType.find("duel") == eastl::string::npos)
							{
								BSPVector3 origin;
								if (bspLoader.GetVectorForKey(&entity, "origin", origin))
								{
									Transform initTransform;
									initTransform.SetTranslation(origin[0], origin[1], origin[2]);
									eastl::shared_ptr<Actor> pActor = CreateActor(
										modelResources[className], nullptr, &initTransform);
									if (pActor)
									{
										// fire an event letting everyone else know that we created a new actor
										eastl::shared_ptr<EventDataNewActor> pNewActorEvent(
											new EventDataNewActor(pActor->GetId()));
										BaseEventManager::Get()->QueueEvent(pNewActorEvent);
									}
								}
							}
						}
						else if (targetResources.find(className) != targetResources.end())
						{
							eastl::string gameType = bspLoader.GetValueForKey(&entity, "gametype");
							eastl::string notGameType = bspLoader.GetValueForKey(&entity, "not_gametype");

							if (gameType.empty() && notGameType.empty() ||
								gameType.find("duel") != eastl::string::npos ||
								notGameType.find("duel") == eastl::string::npos)
							{
								BSPVector3 origin;
								if (bspLoader.GetVectorForKey(&entity, "origin", origin))
								{
									Transform initTransform;
									initTransform.SetTranslation(origin[0], origin[1], origin[2]);
									eastl::shared_ptr<Actor> pActor = CreateActor(
										targetResources[className], nullptr, &initTransform);
									if (pActor)
									{
										float angle = bspLoader.GetFloatForKey(&entity, "angle");
										if (angle)
										{
											eastl::shared_ptr<TransformComponent> pTransformComponent(
												pActor->GetComponent<TransformComponent>(TransformComponent::Name).lock());
											if (pTransformComponent)
											{
												Matrix4x4<float> yawRotation = Rotation<4, float>(
													AxisAngle<4, float>(Vector4<float>::Unit(2), angle * (float)GE_C_DEG_TO_RAD));
												pTransformComponent->SetRotation(yawRotation);
											}
										}

										const char* target = bspLoader.GetValueForKey(&entity, "targetname");
										if (target)
										{
											if (className != "target_speaker")
											{
												eastl::shared_ptr<LocationTarget> pLocationTarget(
													pActor->GetComponent<LocationTarget>(LocationTarget::Name).lock());
												if (pLocationTarget)
													pLocationTarget->SetTarget(target);
											}
										}

										eastl::shared_ptr<AudioComponent> pAudioComponent(
											pActor->GetComponent<AudioComponent>(AudioComponent::Name).lock());
										if (pAudioComponent)
										{
											eastl::string audios = bspLoader.GetValueForKey(&entity, "noise");
											if (!audios.empty())
											{
												pAudioComponent->ClearAudios();

												audios.erase(eastl::remove(audios.begin(), audios.end(), '\r'), audios.end());
												audios.erase(eastl::remove(audios.begin(), audios.end(), '\n'), audios.end());
												audios.erase(eastl::remove(audios.begin(), audios.end(), '\t'), audios.end());
												size_t audioBegin = 0, audioEnd = 0;
												do
												{
													audioEnd = audios.find(',', audioBegin);
													pAudioComponent->AddAudio("audio/quake/" + audios.substr(audioBegin, audioEnd - audioBegin));

													audioBegin = audioEnd + 1;
												} while (audioEnd != eastl::string::npos);

												pAudioComponent->PostInit();
											}
										}

										// fire an event letting everyone else know that we created a new actor
										eastl::shared_ptr<EventDataNewActor> pNewActorEvent(
											new EventDataNewActor(pActor->GetId()));
										BaseEventManager::Get()->QueueEvent(pNewActorEvent);
									}
								}
							}
						}
						else if (triggerResources.find(className) != triggerResources.end())
						{
							eastl::string gameType = bspLoader.GetValueForKey(&entity, "gametype");
							eastl::string notGameType = bspLoader.GetValueForKey(&entity, "not_gametype");

							if (gameType.empty() && notGameType.empty() ||
								gameType.find("duel") != eastl::string::npos ||
								notGameType.find("duel") == eastl::string::npos)
							{
								Transform initTransform;
								eastl::shared_ptr<Actor> pActor = CreateActor(
									triggerResources[className], nullptr, &initTransform);
								if (pActor)
								{
									float angle = bspLoader.GetFloatForKey(&entity, "angle");
									if (angle)
									{
										eastl::shared_ptr<TransformComponent> pTransformComponent(
											pActor->GetComponent<TransformComponent>(TransformComponent::Name).lock());
										if (pTransformComponent)
										{
											Matrix4x4<float> yawRotation = Rotation<4, float>(
												AxisAngle<4, float>(Vector4<float>::Unit(2), angle * (float)GE_C_DEG_TO_RAD));
											pTransformComponent->SetRotation(yawRotation);
										}
									}
								}

								const char* model = bspLoader.GetValueForKey(&entity, "model");
								const char* target = bspLoader.GetValueForKey(&entity, "target");
								if (model && target)
								{
									if (className == "trigger_teleport")
									{
										eastl::shared_ptr<TeleporterTrigger> pTeleporterTrigger(
											pActor->GetComponent<TeleporterTrigger>(TeleporterTrigger::Name).lock());
										if (pTeleporterTrigger)
										{
											Transform targetTransform;

											BSPVector3 origin;
											if (bspLoader.GetVectorForKey(&targets[target], "origin", origin))
												targetTransform.SetTranslation(origin[0], origin[1], origin[2]);

											float angle = bspLoader.GetFloatForKey(&targets[target], "angle");
											if (angle)
											{
												Matrix4x4<float> yawRotation = Rotation<4, float>(
													AxisAngle<4, float>(Vector4<float>::Unit(2), angle * (float)GE_C_DEG_TO_RAD));
												targetTransform.SetRotation(yawRotation);
											}
											pTeleporterTrigger->SetTarget(targetTransform);
										}
									}
									else if (className == "trigger_push")
									{
										eastl::shared_ptr<PushTrigger> pPushTrigger(
											pActor->GetComponent<PushTrigger>(PushTrigger::Name).lock());
										if (pPushTrigger)
										{
											Transform targetTransform;

											BSPVector3 origin;
											if (bspLoader.GetVectorForKey(&targets[target], "origin", origin))
												targetTransform.SetTranslation(origin[0], origin[1], origin[2]);

											float angle = bspLoader.GetFloatForKey(&targets[target], "angle");
											if (angle)
											{
												Matrix4x4<float> yawRotation = Rotation<4, float>(
													AxisAngle<4, float>(Vector4<float>::Unit(2), angle * (float)GE_C_DEG_TO_RAD));
												targetTransform.SetRotation(yawRotation);
											}
											pPushTrigger->SetTarget(targetTransform);
										}
									}

									if (strcmp(model, ""))
									{
										// add the model as a brush
										if (model[0] == '*')
										{
											int modelnr = atoi(&model[1]);
											if ((modelnr >= 0) && (modelnr < bspLoader.mNumModels))
											{
												const BSPModel& model = bspLoader.mDModels[modelnr];
												const BSPBrush& brush = bspLoader.mDBrushes[model.firstBrush];
												eastl::vector<Plane3<float>> planes;
												for (int p = 0; p < brush.numSides; p++)
												{
													int sideid = brush.firstSide + p;
													BSPBrushSide& brushside = bspLoader.mDBrushsides[sideid];
													int planeid = brushside.planeNum;
													BSPPlane& plane = bspLoader.mDPlanes[planeid];
													planes.push_back(Plane3<float>(
														Vector3<float>{plane.normal[0], plane.normal[1], plane.normal[2]}, plane.dist));
												}
												eastl::shared_ptr<PhysicComponent> pPhysicComponent(
													pActor->GetComponent<PhysicComponent>(PhysicComponent::Name).lock());
												if (pPhysicComponent)
												{
													BaseGamePhysic* gamePhysics = GameLogic::Get()->GetGamePhysics().get();
													gamePhysics->AddPointCloud(planes.data(), planes.size(), pActor, 
														pPhysicComponent->GetDensity(), pPhysicComponent->GetMaterial());
												}
											}
										}
									}
								}

								// fire an event letting everyone else know that we created a new actor
								eastl::shared_ptr<EventDataNewActor> pNewActorEvent(new EventDataNewActor(pActor->GetId()));
								BaseEventManager::Get()->QueueEvent(pNewActorEvent);
							}
						}
						else if (className == "worldspawn")
						{
							eastl::shared_ptr<Actor> pActor = CreateActor("actors/quake/music/music.xml", nullptr);
							if (pActor)
							{
								eastl::shared_ptr<AudioComponent> pAudioComponent(
									pActor->GetComponent<AudioComponent>(AudioComponent::Name).lock());
								if (pAudioComponent)
								{
									eastl::string audios = bspLoader.GetValueForKey(&entity, "noise");
									if (!audios.empty())
									{
										pAudioComponent->ClearAudios();

										audios.erase(eastl::remove(audios.begin(), audios.end(), '\r'), audios.end());
										audios.erase(eastl::remove(audios.begin(), audios.end(), '\n'), audios.end());
										audios.erase(eastl::remove(audios.begin(), audios.end(), '\t'), audios.end());
										size_t audioBegin = 0, audioEnd = 0;
										do
										{
											audioEnd = audios.find(',', audioBegin);
											pAudioComponent->AddAudio("audio/quake/" + audios.substr(audioBegin, audioEnd - audioBegin));

											audioBegin = audioEnd + 1;
										} while (audioEnd != eastl::string::npos);

										pAudioComponent->PostInit();
									}
								}

								// fire an event letting everyone else know that we created a new actor
								eastl::shared_ptr<EventDataNewActor> pNewActorEvent(
									new EventDataNewActor(pActor->GetId()));
								BaseEventManager::Get()->QueueEvent(pNewActorEvent);
							}
						}
					}
				}
			}
		}
	}

	return true;
}

/*
Spawns an item and tosses it forward
*/
eastl::shared_ptr<Actor> DropItem(
	const Transform& transform, const eastl::shared_ptr<Actor>& item)
{
	AxisAngle<4, float> angles = Rotation<4, float>(transform.GetRotation());

	Vector3<float> direction = HProject(angles.mAxis);
	direction[YAW] = -1.0f;
	Normalize(direction);

	direction[PITCH] *= 2.0f;
	direction[ROLL] *= 2.0f;
	direction[YAW] *= 6.0f;

	// fire the targets of the spawn point
	EventManager::Get()->TriggerEvent(
		eastl::make_shared<QuakeEventDataPushActor>(item->GetId(), direction));

	// auto-remove after 30 seconds
	return item;
}

eastl::shared_ptr<Actor> CreateItemWeapon(WeaponType weapon, const Transform& initTransform)
{
	eastl::shared_ptr<Actor> pActor;
	switch (weapon)
	{
	case WP_SHOTGUN:
		pActor = GameLogic::Get()->CreateActor("actors/quake/models/weapon/shotgun.xml", nullptr, &initTransform);
		break;
	case WP_GRENADE_LAUNCHER:
		pActor = GameLogic::Get()->CreateActor("actors/quake/models/weapon/grenadelauncher.xml", nullptr, &initTransform);
		break;
	case WP_ROCKET_LAUNCHER:
		pActor = GameLogic::Get()->CreateActor("actors/quake/models/weapon/rocketlauncher.xml", nullptr, &initTransform);
		break;
	case WP_LIGHTNING:
		pActor = GameLogic::Get()->CreateActor("actors/quake/models/weapon/lightning.xml", nullptr, &initTransform);
		break;
	case WP_RAILGUN:
		pActor = GameLogic::Get()->CreateActor("actors/quake/models/weapon/railgun.xml", nullptr, &initTransform);
		break;
	case WP_PLASMAGUN:
		pActor = GameLogic::Get()->CreateActor("actors/quake/models/weapon/plasmagun.xml", nullptr, &initTransform);
		break;
	default:
		break;
	}

	LogError("Couldn't find item for weapon " + eastl::to_string(weapon));
	return pActor;
}

/*
Toss the weapon and powerups for the killed player
*/
void TossClientItems(const eastl::shared_ptr<PlayerActor>& player)
{
	// drop the weapon if not a gauntlet or machinegun
	WeaponType weapon = (WeaponType)player->GetState().weapon;

	// make a special check to see if they are changing to a new
	// weapon that isn't the mg or gauntlet.  Without this, a client
	// can pick up a weapon, be killed, and not drop the weapon because
	// their weapon change hasn't completed yet and they are still holding the MG.
	if (weapon == WP_MACHINEGUN)
	{
		if (player->GetState().weaponState == WEAPON_DROPPING)
		{
			weapon = WP_NONE;
		}
		if (!(player->GetState().stats[STAT_WEAPONS] & (1 << weapon)))
		{
			weapon = WP_NONE;
		}
	}

	if (weapon > WP_MACHINEGUN &&
		player->GetState().ammo[weapon])
	{
		// find the item type for this weapon
		eastl::shared_ptr<TransformComponent> pTransformComponent(
			player->GetComponent<TransformComponent>(TransformComponent::Name).lock());
		if (pTransformComponent)
		{
			// spawn the item
			Transform itemTransform;
			itemTransform.SetTranslation(pTransformComponent->GetTransform().GetTranslationW1());
			eastl::shared_ptr<Actor> item = CreateItemWeapon(weapon, itemTransform);

			DropItem(pTransformComponent->GetTransform(), item);
		}
	}
}

void LookAtKiller(
	const eastl::shared_ptr<Actor>& inflictor,
	const eastl::shared_ptr<PlayerActor>& player,
	const eastl::shared_ptr<PlayerActor>& attacker)
{
	if (attacker && attacker != player)
	{
		Vector4<float> playerTranslation, attackerTranslation;
		eastl::shared_ptr<TransformComponent> pTransformComponent(
			player->GetComponent<TransformComponent>(TransformComponent::Name).lock());
		if (pTransformComponent)
			playerTranslation = pTransformComponent->GetTransform().GetTranslationW1();

		pTransformComponent =
			attacker->GetComponent<TransformComponent>(TransformComponent::Name).lock();
		if (pTransformComponent)
			attackerTranslation = pTransformComponent->GetTransform().GetTranslationW1();

		Vector4<float> direction = attackerTranslation - playerTranslation;
		Normalize(direction);
		Matrix4x4<float> rotation = Rotation<4, float>(AxisAngle<4, float>(direction, 0.f));

		Transform transform;
		transform.SetRotation(rotation);
		EventManager::Get()->TriggerEvent(
			eastl::make_shared<QuakeEventDataRotateActor>(player->GetId(), transform));
		player->GetState().stats[STAT_DEAD_YAW] = 0;
	}
	else if (inflictor && inflictor != player)
	{
		Vector4<float> playerTranslation, inflictorTranslation;
		eastl::shared_ptr<TransformComponent> pTransformComponent(
			player->GetComponent<TransformComponent>(TransformComponent::Name).lock());
		if (pTransformComponent)
			playerTranslation = pTransformComponent->GetTransform().GetTranslationW1();

		pTransformComponent =
			attacker->GetComponent<TransformComponent>(TransformComponent::Name).lock();
		if (pTransformComponent)
			inflictorTranslation = pTransformComponent->GetTransform().GetTranslationW1();

		Vector4<float> direction = inflictorTranslation - playerTranslation;
		Normalize(direction);
		Matrix4x4<float> rotation = Rotation<4, float>(AxisAngle<4, float>(direction, 0.f));

		Transform transform;
		transform.SetRotation(rotation);
		EventManager::Get()->TriggerEvent(
			eastl::make_shared<QuakeEventDataRotateActor>(player->GetId(), transform));
		player->GetState().stats[STAT_DEAD_YAW] = 0;
	}
	else
	{
		Transform playerTransform;
		eastl::shared_ptr<TransformComponent> pTransformComponent(
			player->GetComponent<TransformComponent>(TransformComponent::Name).lock());
		if (pTransformComponent)
			playerTransform = pTransformComponent->GetTransform();

		EventManager::Get()->TriggerEvent(
			eastl::make_shared<QuakeEventDataRotateActor>(player->GetId(), playerTransform));
		player->GetState().stats[STAT_DEAD_YAW] = 0;
	}
}


void Die(int damage, MeansOfDeath meansOfDeath,
	const eastl::shared_ptr<Actor>& inflictor,
	const eastl::shared_ptr<PlayerActor>& player,
	const eastl::shared_ptr<PlayerActor>& attacker)
{
	eastl::shared_ptr<Actor> ent;

	if (player->GetState().moveType == PM_DEAD)
		return;

	player->GetState().moveType = PM_DEAD;
	player->GetState().viewHeight = DEAD_VIEWHEIGHT;
	/*
	if (meansOfDeath < 0 || meansOfDeath >= sizeof(modNames) / sizeof(modNames[0]))
	{
	obit = "<bad obituary>";
	}
	else
	{
	obit = modNames[meansOfDeath];
	}

	LogInformation("Kill: %i %i %i: %s killed %s by %s\n",
	killer, self->state->number, meansOfDeath, killerName,
	self->client->pers.netname, obit);

	// broadcast the death event to everyone
	*/

	player->GetState().persistant[PERS_KILLED]++;

	if (attacker)
	{
		attacker->GetState().lastKilled = player->GetId();

		if (attacker == player)//|| OnSameTeam(self, attacker))
		{
			attacker->GetState().persistant[PERS_SCORE] -= 1;
			/*
			if (g_gametype.integer == GT_TEAM)
			level.teamScores[attacker->GetState().persistant[PERS_TEAM]] -= 1;
			*/
		}
		else
		{
			attacker->GetState().persistant[PERS_SCORE] += 1;
			/*
			if (g_gametype.integer == GT_TEAM)
			level.teamScores[attacker->GetState().persistant[PERS_TEAM]] += 1;
			*/

			if (meansOfDeath == MOD_GAUNTLET)
			{
				// play humiliation on player
				attacker->GetState().persistant[PERS_GAUNTLET_FRAG_COUNT]++;

				// add the sprite over the player's head
				attacker->GetState().eFlags &= ~(
					EF_AWARD_IMPRESSIVE | EF_AWARD_EXCELLENT |
					EF_AWARD_GAUNTLET | EF_AWARD_ASSIST |
					EF_AWARD_DEFEND | EF_AWARD_CAP);
				attacker->GetState().eFlags |= EF_AWARD_GAUNTLET;
				//attacker->GetState().rewardTime = level.time + REWARD_SPRITE_TIME;

				// also play humiliation on target
				player->GetState().persistant[PERS_PLAYEREVENTS] ^= PLAYEREVENT_GAUNTLETREWARD;
			}

			// check for two kills in a short amount of time
			// if this is close enough to the last kill, give a reward sound
			/*
			if (level.time - attacker->GetState().lastKillTime < CARNAGE_REWARD_TIME)
			{
			// play excellent on player
			attacker->GetState().persistant[PERS_EXCELLENT_COUNT]++;

			// add the sprite over the player's head
			attacker->GetState().eFlags &= ~(
			EF_AWARD_IMPRESSIVE | EF_AWARD_EXCELLENT |
			EF_AWARD_GAUNTLET | EF_AWARD_ASSIST |
			EF_AWARD_DEFEND | EF_AWARD_CAP);
			attacker->GetState().eFlags |= EF_AWARD_EXCELLENT;
			attacker->GetState().rewardTime = level.time + REWARD_SPRITE_TIME;
			}
			attacker->GetState().lastKillTime = level.time;
			*/
		}
	}
	else
	{
		player->GetState().persistant[PERS_SCORE] -= 1;
		/*
		if (g_gametype.integer == GT_TEAM)
		level.teamScores[player->GetState().persistant[PERS_TEAM]] -= 1;
		*/
	}

	// Add team bonuses
	//Team_FragBonuses(self, inflictor, attacker);

	// if client is in a nodrop area, don't drop anything (but return CTF flags!)
	TossClientItems(player);

	//Cmd_Score_f(self);		// show scores
	// send updated scores to any clients that are following this one,
	// or they would get stale scoreboards

	player->GetState().takeDamage = true;	// can still be gibbed

	player->GetState().weapon = WP_NONE;
	player->GetState().contents = CONTENTS_CORPSE;
	LookAtKiller(inflictor, player, attacker);

	//player->GetState().viewangles = Vector3<float>{ player->GetState().angles };
	//player->GetState().loopSound = 0;

	//self->maxs[2] = -8;

	// don't allow respawn until the death anim is done
	// g_forcerespawn may force spawning at some later time
	//player->GetState().respawnTime = level.time + 1700;

	// remove powerups
	memset(player->GetState().powerups, 0, sizeof(player->GetState().powerups));

	// never gib in a nodrop
	int anim = BOTH_DEATH1;
	/*
	if ((player->GetState().stats[STAT_HEALTH] <= GIB_HEALTH &&
	!(contents & CONTENTS_NODROP)) || meansOfDeath == MOD_SUICIDE)
	{
	// gib death
	//GibEntity(self, killer);
	}
	else
	*/
	{
		// for the no-blood option, we need to prevent the health
		// from going to gib level
		if (player->GetState().stats[STAT_HEALTH] <= GIB_HEALTH)
			player->GetState().stats[STAT_HEALTH] = GIB_HEALTH + 1;

		player->GetState().legsAnim = anim;
		player->GetState().torsoAnim = anim;

		// call for animation death
		EventManager::Get()->TriggerEvent(
			eastl::make_shared<QuakeEventDataDeadActor>(player->GetId()));

		// play pain sound
		EventManager::Get()->TriggerEvent(
			eastl::make_shared<EventDataPlaySound>("audio/quake/sound/player/death1.wav"));

		//AddEvent(self, EV_DEATH1 + i, killer);

		// the body can still be gibbed
		//self->die = body_die;

		// globally cycle through the different death animations
		//i = (i + 1) % 3;
	}
	//trap_LinkEntity(self);
}


int CheckArmor(eastl::shared_ptr<PlayerActor> playerActor, int damage, int dflags)
{
	if (!damage)
		return 0;

	if (!playerActor)
		return 0;

	if (dflags & DAMAGE_NO_ARMOR)
		return 0;

	// armor
	int save = ceil(damage * ARMOR_PROTECTION);
	if (save >= playerActor->GetState().stats[STAT_ARMOR])
		save = playerActor->GetState().stats[STAT_ARMOR];

	if (!save)
		return 0;
	playerActor->GetState().stats[STAT_ARMOR] -= save;

	return save;
}

/*
Called just before a snapshot is sent to the given player.
Totals up all damage and generates both the player state
damage values to that player for pain blends and kicks, and
global pain sound events for all players.
*/
void DamageFeedback(const eastl::shared_ptr<PlayerActor>& player)
{
	if (player->GetState().moveType == PM_DEAD)
		return;

	// total points of damage shot at the player this frame
	int count = player->GetState().damageBlood + player->GetState().damageArmor;
	if (count == 0)
		return;		// didn't take any damage

	if (count > 255) count = 255;

	// send the information to the client

	// world damage (falling, slime, etc) uses a special code
	// to make the blend blob centered instead of positional
	if (player->GetState().damageFromWorld)
	{
		player->GetState().damagePitch = 255;
		player->GetState().damageYaw = 255;

		player->GetState().damageFromWorld = false;
	}
	else
	{
		/*
		vectoangles(player->GetState().damageFrom, angles);
		player->GetState().damagePitch = angles[PITCH] / 360.0 * 256;
		player->GetState().damageYaw = angles[YAW] / 360.0 * 256;
		*/
	}

	// play an apropriate pain sound
	//if ((level.time > player->GetState().painDebounceTime) && !(player->GetState().flags & FL_GODMODE))
	{
		//player->GetState().painDebounceTime = level.time + 700;
		//AddEvent(player, EV_PAIN, player->health);
		//EventManager::Get()->TriggerEvent(
		//	eastl::make_shared<EventDataPlaySound>(GetId()));
		player->GetState().damageEvent++;
	}

	player->GetState().damageCount = count;

	//
	// clear totals
	//
	player->GetState().damageBlood = 0;
	player->GetState().damageArmor = 0;
	player->GetState().damageKnockback = 0;
}



/*
============
Damage

target		player that is being damaged
inflictor	player that is causing the damage
attacker	player that caused the inflictor to damage target

dir			direction of the attack for knockback
point		point at which the damage is being inflicted, used for headshots
damage		amount of damage being inflicted
knockback	force to be applied against targ as a result of the damage

inflictor, attacker, dir, and point can be NULL for environmental effects

dflags		these flags are used to control how T_Damage works
DAMAGE_RADIUS			damage was indirect (from a nearby explosion)
DAMAGE_NO_ARMOR			armor does not protect from this damage
DAMAGE_NO_KNOCKBACK		do not affect velocity, just view angles
DAMAGE_NO_PROTECTION	kills godmode, armor, everything
============
*/

void Damage(int damage, int dflags, int mod,
	Vector3<float> dir, Vector3<float> point,
	const eastl::shared_ptr<PlayerActor>& target,
	const eastl::shared_ptr<Actor>& inflictor,
	const eastl::shared_ptr<PlayerActor>& attacker)
{
	if (!target->GetState().takeDamage)
	{
		return;
	}


	// reduce damage by the attacker's handicap value
	// unless they are rocket jumping
	if (attacker && attacker != target)
	{
		int max = attacker->GetState().stats[STAT_MAX_HEALTH];
		damage = damage * max / 100;
	}

	if (dir != Vector3<float>::Zero())
	{
		dflags |= DAMAGE_NO_KNOCKBACK;
	}
	else
	{
		Normalize(dir);
	}

	int knockback = damage;
	if (knockback > 200)
		knockback = 200;

	if (dflags & DAMAGE_NO_KNOCKBACK)
		knockback = 0;

	// figure momentum add, even if the damage won't be taken
	if (knockback && target)
	{
		Vector3<float>	kvel;
		float	mass;

		mass = 200;

		//kvel = dir * (g_knockback.value * (float)knockback / mass));
		//target->GetState().velocity += kvel;

		// set the timer so that the other client can't cancel
		// out the movement immediately
		if (!target->GetState().moveTime)
		{
			int		t;

			t = knockback * 2;
			if (t < 50)
			{
				t = 50;
			}
			if (t > 200)
			{
				t = 200;
			}
			target->GetState().moveTime = t;
			//target->GetState().moveFlags |= PMF_TIME_KNOCKBACK;
		}
	}

	// battlesuit protects from all radius damage (but takes knockback)
	// and protects 50% against all damage
	if (target && target->GetState().powerups[PW_BATTLESUIT])
	{
		//AddEvent(targ, EV_POWERUP_BATTLESUIT, 0);
		if ((dflags & DAMAGE_RADIUS) || (mod == MOD_FALLING))
		{
			return;
		}
		damage *= 0.5;
	}

	// add to the attacker's hit counter (if the target isn't a general entity like a prox mine)
	if (attacker && target != attacker &&
		target->GetState().stats[STAT_HEALTH] > 0 &&
		target->GetState().eType != ET_MISSILE &&
		target->GetState().eType != ET_GENERAL)
	{
		/*
		if (OnSameTeam(target, attacker))
		{
		attacker->GetState().persistant[PERS_HITS]--;
		}
		else
		*/
		{
			attacker->GetState().persistant[PERS_HITS]++;
		}
		attacker->GetState().persistant[PERS_ATTACKEE_ARMOR] =
			(target->GetState().stats[STAT_HEALTH] << 8) | (target->GetState().stats[STAT_ARMOR]);
	}

	// always give half damage if hurting self
	// calculated after knockback, so rocket jumping works
	if (target == attacker)
		damage *= 0.5;

	if (damage < 1)
		damage = 1;

	int take = damage;
	int save = 0;

	// save some from armor
	int asave = CheckArmor(target, take, dflags);
	take -= asave;

	// add to the damage inflicted on a player this frame
	// the total will be turned into screen blends and view angle kicks
	// at the end of the frame
	if (target)
	{
		if (attacker)
		{
			target->GetState().persistant[PERS_ATTACKER] = attacker->GetId();
		}
		else
		{
			target->GetState().persistant[PERS_ATTACKER] = ENTITYNUM_WORLD;
		}
		target->GetState().damageArmor += asave;
		target->GetState().damageBlood += take;
		target->GetState().damageKnockback += knockback;
		if (dir != Vector3<float>::Zero())
		{
			target->GetState().damageFrom = Vector3<float>{ dir };
			target->GetState().damageFromWorld = false;
		}
		else
		{
			Transform playerTransform;
			eastl::shared_ptr<TransformComponent> pTransformComponent(
				target->GetComponent<TransformComponent>(TransformComponent::Name).lock());
			if (pTransformComponent)
			{
				target->GetState().damageFrom = pTransformComponent->GetTransform().GetTranslation();
				target->GetState().damageFromWorld = true;
			}
		}
	}

	if (target)
	{
		// set the last client who damaged the target
		target->GetState().lastHurt = attacker->GetId();
		target->GetState().lastHurtMod = mod;
	}

	// do the damage
	if (take)
	{
		target->GetState().stats[STAT_HEALTH] =
			target->GetState().stats[STAT_HEALTH] - take;

		if (target->GetState().stats[STAT_HEALTH] <= 0)
		{
			//target->GetState().flags |= FL_NO_KNOCKBACK;

			if (target->GetState().stats[STAT_HEALTH] < -999)
				target->GetState().stats[STAT_HEALTH] = -999;

			//targ->enemy = attacker;
			Die(take, (MeansOfDeath)mod, inflictor, target, attacker);
		}
		else //if (targ->pain)
		{
			//targ->pain(targ, attacker, take);
			DamageFeedback(attacker);

			if (target->GetState().stats[STAT_HEALTH] < 25) 
			{
				// play pain sound
				EventManager::Get()->TriggerEvent(
					eastl::make_shared<EventDataPlaySound>("audio/quake/sound/player/pain25_1.wav"));
			}
			else if (target->GetState().stats[STAT_HEALTH] < 50) 
			{
				// play pain sound
				EventManager::Get()->TriggerEvent(
					eastl::make_shared<EventDataPlaySound>("audio/quake/sound/player/pain50_1.wav"));
			}
			else if (target->GetState().stats[STAT_HEALTH] < 75) 
			{
				// play pain sound
				EventManager::Get()->TriggerEvent(
					eastl::make_shared<EventDataPlaySound>("audio/quake/sound/player/pain75_1.wav"));
			}
			else 
			{
				// play pain sound
				EventManager::Get()->TriggerEvent(
					eastl::make_shared<EventDataPlaySound>("audio/quake/sound/player/pain100_1.wav"));
			}
		}
	}
}


/*
CanDamage

Returns true if the inflictor can directly damage the target.  Used for
explosions and melee attacks.
*/
bool CanDamage(const eastl::shared_ptr<PlayerActor>& target, Vector3<float> origin)
{
	return true;
}

bool LogAccuracyHit(
	const eastl::shared_ptr<PlayerActor>& target,
	const eastl::shared_ptr<PlayerActor>& attacker)
{
	if (!target->GetState().takeDamage)
	{
		return false;
	}

	if (target == attacker)
	{
		return false;
	}

	if (!target)
	{
		return false;
	}

	if (!attacker)
	{
		return false;
	}

	if (target->GetState().stats[STAT_HEALTH] <= 0)
	{
		return false;
	}

	return true;
}

bool RadiusDamage(
	float damage, float radius, int mod, Vector3<float> origin,
	const eastl::shared_ptr<PlayerActor>& attacker,
	const eastl::shared_ptr<Actor>& ignore)
{
	float points, dist;
	int numListedEntities;
	Vector3<float> mins, maxs;
	Vector3<float> v;
	Vector3<float> dir;
	int i, e;
	bool hitClient = false;

	if (radius < 1)
		radius = 1;

	for (i = 0; i < 3; i++)
	{
		mins[i] = origin[i] - radius;
		maxs[i] = origin[i] + radius;
	}

	//numListedEntities = trap_EntitiesInBox(mins, maxs, entityList, MAX_GENTITIES);

	for (e = 0; e < numListedEntities; e++)
	{
		eastl::shared_ptr<PlayerActor> actor; // ent = &entities[entityList[e]];

		if (actor == ignore)
			continue;
		if (!actor->GetState().takeDamage)
			continue;

		// find the distance from the edge of the bounding box
		for (i = 0; i < 3; i++)
		{
			/*
			if (origin[i] < ent->absmin[i])
			{
			v[i] = ent->absmin[i] - origin[i];
			}
			else if (origin[i] > ent->absmax[i])
			{
			v[i] = origin[i] - ent->absmax[i];
			}
			else
			{
			v[i] = 0;
			}
			*/
		}

		dist = Length(v);
		if (dist >= radius)
		{
			continue;
		}

		points = damage * (1.0 - dist / radius);

		if (CanDamage(actor, origin))
		{
			if (LogAccuracyHit(actor, attacker))
			{
				hitClient = true;
			}
			//dir = actor->GetState().currentOrigin - origin;
			// push the center of mass higher than the origin so players
			// get knocked into the air more
			dir[2] += 24;
			Damage((int)points, DAMAGE_RADIUS, mod, dir, origin, actor, NULL, attacker);
		}
	}

	return hitClient;
}

void BounceProjectile(
	Vector3<float> start, Vector3<float> impact,
	Vector3<float> dir, Vector3<float> endout)
{
	Vector3<float> v, newv;
	float dot;

	v = impact - start;
	dot = Dot(v, dir);
	newv = v + dir * (-2 * dot);

	Normalize(newv);
	endout = impact + newv * 8192.f;
}

/*
======================================================================

GAUNTLET

======================================================================
*/

void Gauntlet(const eastl::shared_ptr<PlayerActor>& player)
{

}

void QuakeLogic::GauntletAttack(
	const eastl::shared_ptr<PlayerActor>& player, 
	const Vector3<float>& muzzle, const Vector3<float>& forward,
	const Vector3<float>& right, const Vector3<float>& up)
{
	//EntityTrace tr;
	eastl::shared_ptr<Actor> ent;
	//eastl::shared_ptr<Actor> traceEnt;
	int damage;

	//set muzzle location relative to pivoting eye
	Vector3<float> end = muzzle + forward * 32.f;

	// play attack sound
	EventManager::Get()->TriggerEvent(
		eastl::make_shared<EventDataPlaySound>("audio/quake/sound/weapons/melee/fstrun.ogg"));

	Vector3<float> collision, collisionNormal;
	ActorId actorCollisionId = mPhysics->CastRay(muzzle, end, collision, collisionNormal);
	if (collision == NULL) return; // no surface impact

	if (actorCollisionId != INVALID_ACTOR_ID &&
		eastl::dynamic_shared_pointer_cast<PlayerActor>(mActors[actorCollisionId]))
	{
		eastl::shared_ptr<PlayerActor> target =
			eastl::dynamic_shared_pointer_cast<PlayerActor>(mActors[actorCollisionId]);
		if (LogAccuracyHit(target, player))
			player->GetState().accuracyHits++;

		//rotation = ((69069 * randSeed + 1) & 0x7fff) % 360;
		Transform initTransform;
		initTransform.SetTranslation(collision);
		CreateActor("actors/quake/effects/bleed.xml", nullptr, &initTransform);

		damage = 50;
		Damage(damage, 0, MOD_GAUNTLET, forward, muzzle, target, player, player);
	}

	/*
	trap_Trace(&tr, muzzle, NULL, NULL, end, ent->es->number, MASK_SHOT);
	if (tr.surfaceFlags & SURF_NOIMPACT)
	{
	return false;
	}

	traceEnt = &entities[tr.entityNum];

	// send blood impact
	if (traceEnt->takedamage && traceEnt->client)
	{
	tent = TempEntity(tr.endpos, EV_MISSILE_HIT);
	tent->state->otherEntityNum = traceEnt->state->number;
	tent->state->eventParm = DirToByte(tr.plane.mNormal);
	tent->state->weapon = ent->state->weapon;
	}

	if (!traceEnt->takedamage)
	return false;
	*/
}


/*
======================================================================

MACHINEGUN

======================================================================
*/


void QuakeLogic::BulletFire(
	const eastl::shared_ptr<PlayerActor>& player,
	const Vector3<float>& muzzle, const Vector3<float>& forward,
	const Vector3<float>& right, const Vector3<float>& up, float spread, int damage)
{
	float r = (Randomizer::Rand() & 0x7fff) / ((float)0x7fff) * GE_C_PI * 2.f;
	float u = sin(r) * (2.f * ((Randomizer::Rand() & 0x7fff) / ((float)0x7fff) - 0.5)) * spread * 16.f;
	r = cos(r) * (2.f * ((Randomizer::Rand() & 0x7fff) / ((float)0x7fff) - 0.5)) * spread * 16.f;
	Vector3<float> end = muzzle + forward * 8192.f * 16.f;
	end += right * r;
	end += up * u;

	// play firing sound
	EventManager::Get()->TriggerEvent(
		eastl::make_shared<EventDataPlaySound>("audio/quake/sound/weapons/machinegun/ric1.ogg"));

	Vector3<float> collision, collisionNormal;
	ActorId actorCollisionId = mPhysics->CastRay(muzzle, end, collision, collisionNormal);
	if (collision == NULL) return; // no surface impact

	if (actorCollisionId != INVALID_ACTOR_ID &&
		eastl::dynamic_shared_pointer_cast<PlayerActor>(mActors[actorCollisionId]))
	{
		eastl::shared_ptr<PlayerActor> target =
			eastl::dynamic_shared_pointer_cast<PlayerActor>(mActors[actorCollisionId]);
		if (LogAccuracyHit(target, player))
			player->GetState().accuracyHits++;

		//rotation = ((69069 * randSeed + 1) & 0x7fff) % 360;
		Transform initTransform;
		initTransform.SetTranslation(collision);
		CreateActor("actors/quake/effects/bleed.xml", nullptr, &initTransform);

		Damage(damage, 0, MOD_MACHINEGUN, forward, collision, target, player, player);
	}
	else
	{
		Transform initTransform;
		initTransform.SetTranslation(collision);
		CreateActor("actors/quake/effects/bulletexplosion.xml", nullptr, &initTransform);
	}
}


/*
======================================================================

SHOTGUN

======================================================================
*/

// DEFAULT_SHOTGUN_SPREAD and DEFAULT_SHOTGUN_COUNT	are in bg_public.h, because
// client predicts same spreads
#define	DEFAULT_SHOTGUN_DAMAGE	10

bool QuakeLogic::ShotgunPellet(const eastl::shared_ptr<PlayerActor>& player, 
	const Vector3<float>& forward, const Vector3<float>& start, const Vector3<float>& end)
{
	Vector3<float> collision, collisionNormal;
	ActorId actorCollisionId = mPhysics->CastRay(start, end, collision, collisionNormal);
	if (collision == NULL) return false; // no surface impact

	if (actorCollisionId != INVALID_ACTOR_ID &&
		eastl::dynamic_shared_pointer_cast<PlayerActor>(mActors[actorCollisionId]))
	{
		eastl::shared_ptr<PlayerActor> target =
			eastl::dynamic_shared_pointer_cast<PlayerActor>(mActors[actorCollisionId]);
		if (LogAccuracyHit(target, player))
			player->GetState().accuracyHits++;

		//rotation = ((69069 * randSeed + 1) & 0x7fff) % 360;
		Transform initTransform;
		initTransform.SetTranslation(collision);
		CreateActor("actors/quake/effects/bleed.xml", nullptr, &initTransform);

		int damage = DEFAULT_SHOTGUN_DAMAGE;
		Damage(damage, 0, MOD_SHOTGUN, forward, collision, target, player, player);
		return true;
	}
	else
	{
		Transform initTransform;
		initTransform.SetTranslation(collision);
		CreateActor("actors/quake/effects/bulletexplosion.xml", nullptr, &initTransform);
	}

	return false;
}

void QuakeLogic::ShotgunFire(
	const eastl::shared_ptr<PlayerActor>& player,
	const Vector3<float>& muzzle, const Vector3<float>& forward,
	const Vector3<float>& right, const Vector3<float>& up)
{
	// play firing sound
	EventManager::Get()->TriggerEvent(
		eastl::make_shared<EventDataPlaySound>("audio/quake/sound/weapons/shotgun/sshotf1b.ogg"));

	// generate the "random" spread pattern
	for (unsigned int i = 0; i < DEFAULT_SHOTGUN_COUNT; i++)
	{
		float r = (2.f * ((Randomizer::Rand() & 0x7fff) / ((float)0x7fff) - 0.5)) * DEFAULT_SHOTGUN_SPREAD * 16.f;
		float u = (2.f * ((Randomizer::Rand() & 0x7fff) / ((float)0x7fff) - 0.5)) * DEFAULT_SHOTGUN_SPREAD * 16.f;
		Vector3<float> end = muzzle + forward * 8192.f * 16.f;
		end += right * r;
		end += up * u;

		if (ShotgunPellet(player, forward, muzzle, end))
			player->GetState().accuracyHits++;
	}
}


/*
======================================================================

GRENADE LAUNCHER

======================================================================
*/

void QuakeLogic::GrenadeLauncherFire(
	const eastl::shared_ptr<PlayerActor>& player, const Vector3<float>& muzzle,
	const Vector3<float>& forward, const Vector3<float>& right,
	const Vector3<float>& up, const EulerAngles<float>& viewAngles)
{
	Matrix4x4<float> yawRotation = Rotation<4, float>(
		AxisAngle<4, float>(Vector4<float>::Unit(2), viewAngles.mAngle[2]));
	Matrix4x4<float> pitchRotation = Rotation<4, float>(
		AxisAngle<4, float>(Vector4<float>::Unit(1), viewAngles.mAngle[1] + GE_C_QUARTER_PI));

	Transform initTransform;
	initTransform.SetRotation(yawRotation * pitchRotation);
	initTransform.SetTranslation(muzzle);

	float r = (Randomizer::Rand() & 0x7fff) / ((float)0x7fff) * GE_C_PI * 2.f;
	float u = sin(r) * (2.f * ((Randomizer::Rand() & 0x7fff) / ((float)0x7fff) - 0.5)) * 16.f;
	r = cos(r) * (2.f * ((Randomizer::Rand() & 0x7fff) / ((float)0x7fff) - 0.5)) * 16.f;
	Vector3<float> end = muzzle + forward * 8192.f * 16.f;
	end += right * r;
	end += up * u;
	Vector3<float> direction = end - muzzle;
	Normalize(direction);

	eastl::shared_ptr<Actor> pGameActor = 
		CreateActor("actors/quake/effects/grenadelauncherfire.xml", nullptr, &initTransform);
	if (pGameActor)
	{
		eastl::shared_ptr<PhysicComponent> pPhysicalComponent =
			pGameActor->GetComponent<PhysicComponent>(PhysicComponent::Name).lock();
		if (pPhysicalComponent)
		{
			direction[PITCH] *= 800000.f;
			direction[ROLL] *= 800000.f;
			direction[YAW] *= 400000.f;
			pPhysicalComponent->ApplyForce(direction);
		}
	}

	// play firing sound
	EventManager::Get()->TriggerEvent(
		eastl::make_shared<EventDataPlaySound>("audio/quake/sound/weapons/grenade/grenlf1a.ogg"));

	/*
	bolt->damage = 100;
	bolt->splashDamage = 100;
	bolt->splashRadius = 150;
	bolt->methodOfDeath = MOD_GRENADE;
	bolt->splashMethodOfDeath = MOD_GRENADE_SPLASH;

	// splash damage
	if (ent->splashDamage) {
		if (G_RadiusDamage(ent->r.currentOrigin, ent->parent, ent->splashDamage, ent->splashRadius, ent
			, ent->splashMethodOfDeath)) {
			g_entities[ent->r.ownerNum].client->accuracy_hits++;
		}
	}

	mod = cgs.media.dishFlashModel;
	shader = cgs.media.grenadeExplosionShader;
	sfx = cgs.media.sfx_rockexp;
	mark = cgs.media.burnMarkShader;
	radius = 64;
	light = 300;

	m = FireGrenade(ent, muzzle, forward);
	//	VectorAdd( m->state->pos.trDelta, ent->client->ps.velocity, m->state->pos.trDelta );	// "real" physics
	*/
}

/*
======================================================================

ROCKET

======================================================================
*/

void QuakeLogic::RocketLauncherFire(
	const eastl::shared_ptr<PlayerActor>& player, const Vector3<float>& muzzle,
	const Vector3<float>& forward, const Vector3<float>& right,
	const Vector3<float>& up, const EulerAngles<float>& viewAngles)
{
	Matrix4x4<float> yawRotation = Rotation<4, float>(
		AxisAngle<4, float>(Vector4<float>::Unit(2), viewAngles.mAngle[2]));
	Matrix4x4<float> pitchRotation = Rotation<4, float>(
		AxisAngle<4, float>(Vector4<float>::Unit(1), viewAngles.mAngle[1]));

	Transform initTransform;
	initTransform.SetRotation(yawRotation * pitchRotation);
	initTransform.SetTranslation(muzzle);

	float r = (Randomizer::Rand() & 0x7fff) / ((float)0x7fff) * GE_C_PI * 2.f;
	float u = sin(r) * (2.f * ((Randomizer::Rand() & 0x7fff) / ((float)0x7fff) - 0.5)) * 16.f;
	r = cos(r) * (2.f * ((Randomizer::Rand() & 0x7fff) / ((float)0x7fff) - 0.5)) * 16.f;
	Vector3<float> end = muzzle + forward * 8192.f * 16.f;
	end += right * r;
	end += up * u;
	Vector3<float> direction = end - muzzle;
	Normalize(direction);

	eastl::shared_ptr<Actor> pGameActor =
		CreateActor("actors/quake/effects/rocketlauncherfire.xml", nullptr, &initTransform);
	if (pGameActor)
	{
		eastl::shared_ptr<PhysicComponent> pPhysicalComponent =
			pGameActor->GetComponent<PhysicComponent>(PhysicComponent::Name).lock();
		if (pPhysicalComponent)
		{
			direction[PITCH] *= 300.f;
			direction[ROLL] *= 300.f;
			direction[YAW] *= 300.f;
			pPhysicalComponent->ApplyForce(direction);
		}
	}

	// play firing sound
	EventManager::Get()->TriggerEvent(
		eastl::make_shared<EventDataPlaySound>("audio/quake/sound/weapons/rocket/rocklf1a.ogg"));

	/*
	bolt->damage = 100;
	bolt->splashDamage = 100;
	bolt->splashRadius = 150;
	bolt->methodOfDeath = MOD_GRENADE;
	bolt->splashMethodOfDeath = MOD_GRENADE_SPLASH;

	// splash damage
	if (ent->splashDamage) {
	if (G_RadiusDamage(ent->r.currentOrigin, ent->parent, ent->splashDamage, ent->splashRadius, ent
	, ent->splashMethodOfDeath)) {
	g_entities[ent->r.ownerNum].client->accuracy_hits++;
	}
	}

	mod = cgs.media.dishFlashModel;
	shader = cgs.media.grenadeExplosionShader;
	sfx = cgs.media.sfx_rockexp;
	mark = cgs.media.burnMarkShader;
	radius = 64;
	light = 300;

	m = FireRocket(ent, muzzle, forward);
	//	VectorAdd( m->state->pos.trDelta, ent->client->ps.velocity, m->state->pos.trDelta );	// "real" physics
	*/
}


/*
======================================================================

PLASMA GUN

======================================================================
*/

void QuakeLogic::PlasmagunFire(
	const eastl::shared_ptr<PlayerActor>& player,
	const Vector3<float>& muzzle, const Vector3<float>& forward,
	const Vector3<float>& right, const Vector3<float>& up)
{
	/*
	Entity	*m;

	m = FirePlasma(ent, muzzle, forward);
	//	VectorAdd( m->state->pos.trDelta, ent->client->ps.velocity, m->state->pos.trDelta );	// "real" physics
	*/
}

/*
======================================================================

RAILGUN

======================================================================
*/

#define	MAX_RAIL_HITS	4
void QuakeLogic::RailgunFire(
	const eastl::shared_ptr<PlayerActor>& player,
	const Vector3<float>& muzzle, const Vector3<float>& forward,
	const Vector3<float>& right, const Vector3<float>& up)
{
	/*
	Vector3<float> end;

	EntityTrace trace;
	Entity	*tent;
	Entity	*traceEnt;
	int damage;
	int i;
	int hits;
	int unlinked;
	int passent;
	Entity *unlinkedEntities[MAX_RAIL_HITS];

	damage = 100;

	end = muzzle + forward * 8192.f;

	// trace only against the solids, so the railgun will go through people
	unlinked = 0;
	hits = 0;
	passent = ent->state->number;
	do
	{
		//trap_Trace(&trace, muzzle, NULL, NULL, end, passent, MASK_SHOT);
		if (trace.entityNum >= ENTITYNUM_MAX_NORMAL)
		{
			break;
		}
		traceEnt = &entities[trace.entityNum];
		if (traceEnt->takedamage)
		{
			if (LogAccuracyHit(traceEnt, ent))
			{
				hits++;
			}
			Damage(traceEnt, ent, ent, forward, trace.endpos, damage, 0, MOD_RAILGUN);
		}
		if (trace.contents & CONTENTS_SOLID)
		{
			break;		// we hit something solid enough to stop the beam
		}
		// unlink this entity, so the next trace will go past it
		//trap_UnlinkEntity(traceEnt);
		unlinkedEntities[unlinked] = traceEnt;
		unlinked++;
	} while (unlinked < MAX_RAIL_HITS);

	// link back in any entities we unlinked
	for (i = 0; i < unlinked; i++)
	{
	trap_LinkEntity(unlinkedEntities[i]);
	}

	// the final trace endpos will be the terminal point of the rail trail

	// snap the endpos to integers to save net bandwidth, but nudged towards the line
	//SnapVectorTowards(trace.endpos, muzzle);

	// send railgun beam effect
	tent = TempEntity(trace.endpos, EV_RAILTRAIL);

	// set player number for custom colors on the railtrail
	tent->state->clientNum = ent->state->clientNum;

	tent->state->origin2 = Vector3<float>{ muzzle };
	// move origin a bit to come closer to the drawn gun muzzle
	tent->state->origin2 += right * 4.f;
	tent->state->origin2 += up * -1.f;

	// no explosion at end if SURF_NOIMPACT, but still make the trail
	if (trace.surfaceFlags & SURF_NOIMPACT)
	{
		tent->state->eventParm = 255;	// don't make the explosion at the end
	}
	else
	{
		tent->state->eventParm = DirToByte(trace.plane.mNormal);
	}
	tent->state->clientNum = ent->state->clientNum;

	// give the shooter a reward sound if they have made two railgun hits in a row
	if (hits == 0)
	{
		// complete miss
		player->GetState().accurateCount = 0;
	}
	else
	{
		// check for "impressive" reward sound
		player->GetState().accurateCount += hits;
		if (player->GetState().accurateCount >= 2)
		{
			player->GetState().accurateCount -= 2;
			player->GetState().persistant[PERS_IMPRESSIVE_COUNT]++;
			// add the sprite over the player's head
			player->GetState().eFlags &= ~
				(EF_AWARD_IMPRESSIVE | EF_AWARD_EXCELLENT |
					EF_AWARD_GAUNTLET | EF_AWARD_ASSIST |
					EF_AWARD_DEFEND | EF_AWARD_CAP);
			player->GetState().eFlags |= EF_AWARD_IMPRESSIVE;
			player->GetState().rewardTime = level.time + REWARD_SPRITE_TIME;
		}
		player->GetState().accuracyHits++;
	}
	*/
}


/*
======================================================================

LIGHTNING GUN

======================================================================
*/

void QuakeLogic::LightningFire(
	const eastl::shared_ptr<PlayerActor>& player,
	const Vector3<float>& muzzle, const Vector3<float>& forward,
	const Vector3<float>& right, const Vector3<float>& up)
{
	/*
	EntityTrace		tr;
	Vector3<float>		end;
	Entity	*traceEnt, *tent;
	int			damage, i, passent;

	damage = 8;

	passent = ent->state->number;
	for (i = 0; i < 10; i++)
	{
		end = muzzle + forward * LIGHTNING_RANGE;
		//trap_Trace(&tr, muzzle, NULL, NULL, end, passent, MASK_SHOT);

		if (tr.entityNum == ENTITYNUM_NONE)
		{
			return;
		}

		traceEnt = &entities[tr.entityNum];

		if (traceEnt->takedamage)
		{
			Damage(traceEnt, ent, ent, forward, tr.endpos,
				damage, 0, MOD_LIGHTNING);
		}

		if (traceEnt->takedamage && traceEnt->client)
		{
			tent = TempEntity(tr.endpos, EV_MISSILE_HIT);
			tent->state->otherEntityNum = traceEnt->state->number;
			tent->state->eventParm = DirToByte(tr.plane.mNormal);
			tent->state->weapon = ent->state->weapon;
			if (LogAccuracyHit(traceEnt, ent))
			{
				ent->client->accuracy_hits++;
			}
		}
		else if (!(tr.surfaceFlags & SURF_NOIMPACT))
		{
			tent = TempEntity(tr.endpos, EV_MISSILE_MISS);
			tent->state->eventParm = DirToByte(tr.plane.mNormal);
		}

		break;
	}
	*/
}


void QuakeLogic::FireWeaponDelegate(BaseEventDataPtr pEventData)
{
	eastl::shared_ptr<QuakeEventDataFireWeapon> pCastEventData =
		eastl::static_pointer_cast<QuakeEventDataFireWeapon>(pEventData);

	ActorId actorId = pCastEventData->GetId();
	eastl::shared_ptr<PlayerActor> pPlayerActor(
		eastl::dynamic_shared_pointer_cast<PlayerActor>(
		GameLogic::Get()->GetActor(actorId).lock()));

	// track shots taken for accuracy tracking. gauntet is just not tracked
	if (pPlayerActor->GetState().weapon != WP_GAUNTLET)
		pPlayerActor->GetState().accuracyShots++;

	// set aiming directions
	Vector3<float> origin;
	Matrix4x4<float> rotation;
	EulerAngles<float> viewAngles;
	eastl::shared_ptr<TransformComponent> pTransformComponent(
		pPlayerActor->GetComponent<TransformComponent>(TransformComponent::Name).lock());
	if (pTransformComponent)
	{
		viewAngles.mAxis[1] = 1;
		viewAngles.mAxis[2] = 2;
		pTransformComponent->GetTransform().GetRotation(viewAngles);
		origin = pTransformComponent->GetTransform().GetTranslation();
		Matrix4x4<float> yawRotation = Rotation<4, float>(
			AxisAngle<4, float>(Vector4<float>::Unit(2), viewAngles.mAngle[2]));
		Matrix4x4<float> pitchRotation = Rotation<4, float>(
			AxisAngle<4, float>(Vector4<float>::Unit(1), viewAngles.mAngle[1]));

		rotation = yawRotation * pitchRotation;
	}
	Vector3<float> forward = HProject(rotation * Vector4<float>::Unit(PITCH));
	Vector3<float> right = HProject(rotation * Vector4<float>::Unit(ROLL));
	Vector3<float> up = HProject(rotation * Vector4<float>::Unit(YAW));

	//set muzzle location relative to pivoting eye
	Vector3<float> muzzle = origin;
	muzzle[2] += pPlayerActor->GetState().viewHeight;
	muzzle += forward * 14.f;
	muzzle -= right * 14.f;

	// fire the specific weapon
	switch (pPlayerActor->GetState().weapon)
	{
		case WP_GAUNTLET:
			GauntletAttack(pPlayerActor, muzzle, forward, right, up);
			break;
		case WP_LIGHTNING:
			LightningFire(pPlayerActor, muzzle, forward, right, up);
			break;
		case WP_SHOTGUN:
			ShotgunFire(pPlayerActor, muzzle, forward, right, up);
			break;
		case WP_MACHINEGUN:
			BulletFire(pPlayerActor, muzzle, forward, right, up, MACHINEGUN_SPREAD, MACHINEGUN_DAMAGE);
			break;
		case WP_GRENADE_LAUNCHER:
			GrenadeLauncherFire(pPlayerActor, muzzle, forward, right, up, viewAngles);
			break;
		case WP_ROCKET_LAUNCHER:
			RocketLauncherFire(pPlayerActor, muzzle, forward, right, up, viewAngles);
			break;
		case WP_PLASMAGUN:
			PlasmagunFire(pPlayerActor, muzzle, forward, right, up);
			break;
		case WP_RAILGUN:
			RailgunFire(pPlayerActor, muzzle, forward, right, up);
			break;
		default:
			// FIXME Error( "Bad ent->state->weapon" );
			break;
	}
}


bool QuakeLogic::SpotTelefrag(const eastl::shared_ptr<Actor>& spot)
{
	for (ActorMap::const_iterator it = mActors.begin(); it != mActors.end(); ++it)
	{
		eastl::shared_ptr<PlayerActor> playerActor = 
			eastl::dynamic_shared_pointer_cast<PlayerActor>((*it).second);
		if (playerActor)
		{
			eastl::shared_ptr<TransformComponent> pTransformComponent(
				spot->GetComponent<TransformComponent>(TransformComponent::Name).lock());
			if (pTransformComponent)
			{
				Vector3<float> location = pTransformComponent->GetTransform().GetTranslation();
				if (mPhysics->FindIntersection(playerActor->GetId(), location))
					return true;
			}
		}
	}
	return false;
}

const eastl::shared_ptr<Actor>& QuakeLogic::SelectNearestSpawnPoint(const Vector3<float>& from)
{
	Vector3<float> delta;
	float dist, nearestDist;
	eastl::shared_ptr<Actor> spot;
	eastl::shared_ptr<Actor> nearestSpot;

	nearestDist = 999999;
	nearestSpot = NULL;
	spot = NULL;
	for (ActorMap::const_iterator it = mActors.begin(); it != mActors.end(); ++it)
	{
		spot = (*it).second;
		if (spot->GetComponent<LocationTarget>(LocationTarget::Name).lock())
		{
			eastl::shared_ptr<TransformComponent> pTransformComponent(
				spot->GetComponent<TransformComponent>(TransformComponent::Name).lock());
			if (pTransformComponent)
			{
				delta = pTransformComponent->GetPosition() - from;
				dist = Length(delta);
				if (dist < nearestDist)
				{
					nearestDist = dist;
					nearestSpot = spot;
				}
			}
		}
	}

	return nearestSpot;
}

#define	MAX_SPAWN_POINTS	128
const eastl::shared_ptr<Actor>& QuakeLogic::SelectRandomSpawnPoint()
{
	eastl::shared_ptr<Actor> spot = NULL;
	eastl::shared_ptr<Actor> spots[MAX_SPAWN_POINTS];
	int count, selection;

	count = 0;
	for (ActorMap::const_iterator it = mActors.begin(); it != mActors.end(); ++it)
	{
		spot = (*it).second;
		if (spot->GetComponent<LocationTarget>(LocationTarget::Name).lock())
		{
			if (SpotTelefrag(spot))
				continue;

			spots[count] = spot;
			count++;
		}
		else spot = NULL;
	}

	if (!count)
	{
		// no spots that won't telefrag
		return spot;
	}

	selection = Randomizer::Rand() % count;
	return spots[selection];
}

const eastl::shared_ptr<Actor>& QuakeLogic::SelectRandomFurthestSpawnPoint(
	const Vector3<float>& avoidPoint, Transform& transform)
{
	float dists[64];

	int numSpots = 0;
	eastl::shared_ptr<Actor> spot = NULL;
	eastl::shared_ptr<Actor> spots[64];
	for (ActorMap::const_iterator it = mActors.begin(); it != mActors.end(); ++it)
	{
		spot = (*it).second;
		if (spot->GetComponent<LocationTarget>(LocationTarget::Name).lock())
		{
			if (SpotTelefrag(spot))
				continue;

			eastl::shared_ptr<TransformComponent> pTransformComponent(
				spot->GetComponent<TransformComponent>(TransformComponent::Name).lock());
			if (pTransformComponent)
			{
				Vector3<float> location = pTransformComponent->GetTransform().GetTranslation();
				Vector3<float> delta = location - avoidPoint;
				float dist = Length(delta);
				int i;
				for (i = 0; i < numSpots; i++)
				{
					if (dist > dists[i])
					{
						if (numSpots >= 64)
							numSpots = 64 - 1;
						for (int j = numSpots; j > i; j--)
						{
							dists[j] = dists[j - 1];
							spots[j] = spots[j - 1];
						}
						dists[i] = dist;
						spots[i] = spot;
						numSpots++;
						if (numSpots > 64)
							numSpots = 64;
						break;
					}
				}
				if (i >= numSpots && numSpots < 64)
				{
					dists[numSpots] = dist;
					spots[numSpots] = spot;
					numSpots++;
				}
			}
		}
		else spot = NULL;
	}
	if (!numSpots)
	{
		if (!spot)
			LogError("Couldn't find a spawn point");

		return SelectSpawnPoint(Vector3<float>::Zero(), transform);
	}

	// select a random spot from the spawn points furthest away
	int rnd = (int)(((Randomizer::Rand() & 0x7fff) / ((float)0x7fff)) * (numSpots / 2));

	eastl::shared_ptr<TransformComponent> pTransformComponent(
		spots[rnd]->GetComponent<TransformComponent>(TransformComponent::Name).lock());
	if (pTransformComponent)
	{
		transform.SetTranslation(pTransformComponent->GetTransform().GetTranslation());
		transform.SetRotation(pTransformComponent->GetTransform().GetRotation());
	}
	return spots[rnd];
}

const eastl::shared_ptr<Actor>& QuakeLogic::SelectSpawnPoint(
	const Vector3<float>& avoidPoint, Transform& transform)
{
	return SelectRandomFurthestSpawnPoint(avoidPoint, transform);
}

const eastl::shared_ptr<Actor>& QuakeLogic::SelectInitialSpawnPoint(Transform& transform)
{
	eastl::shared_ptr<Actor> spot = NULL;
	for (ActorMap::const_iterator it = mActors.begin(); it != mActors.end(); ++it)
	{
		spot = (*it).second;
		if (spot->GetComponent<LocationTarget>(LocationTarget::Name).lock())
		{
			if (SpotTelefrag(spot))
				return SelectSpawnPoint(Vector3<float>::Zero(), transform);
			break;
		}
		else spot = NULL;
	}

	if (!spot)
		return SelectSpawnPoint(Vector3<float>::Zero(), transform);

	eastl::shared_ptr<TransformComponent> pTransformComponent(
		spot->GetComponent<TransformComponent>(TransformComponent::Name).lock());
	if (pTransformComponent)
	{
		transform.SetTranslation(pTransformComponent->GetTransform().GetTranslation());
		transform.SetRotation(pTransformComponent->GetTransform().GetRotation());
	}
	return spot;
}

/*
Called every time a player is placed fresh in the world:
after the first PlayerBegin, and after each respawn
*/
void QuakeLogic::PlayerSpawn(const eastl::shared_ptr<PlayerActor>& playerActor)
{
	// always clear the kamikaze flag
	playerActor->GetState().eFlags &= ~EF_KAMIKAZE;

	// toggle the teleport bit so the client knows to not lerp
	// and never clear the voted flag
	int flags = playerActor->GetState().eFlags & (EF_TELEPORT_BIT | EF_VOTED | EF_TEAMVOTED);
	flags ^= EF_TELEPORT_BIT;

	// clear everything
	int accuracyHits = playerActor->GetState().accuracyHits;
	int accuracyShots = playerActor->GetState().accuracyShots;
	int persistant[MAX_PERSISTANT];
	for (int i = 0; i < MAX_PERSISTANT; i++)
		persistant[i] = playerActor->GetState().persistant[i];

	memset(&playerActor->GetState(), 0, sizeof(playerActor->GetState()));
	memset(&playerActor->GetAction(), 0, sizeof(playerActor->GetAction()));

	//	client->areabits = savedAreaBits;
	playerActor->GetState().accuracyHits = accuracyHits;
	playerActor->GetState().accuracyShots = accuracyShots;

	for (int i = 0; i < MAX_PERSISTANT; i++)
		playerActor->GetState().persistant[i] = persistant[i];

	// increment the spawncount so the client will detect the respawn
	playerActor->GetState().persistant[PERS_SPAWN_COUNT]++;
	//playerActor->GetState().persistant[PERS_TEAM] = sessionTeam;
	playerActor->GetState().stats[STAT_MAX_HEALTH] = 100;
	playerActor->GetState().eFlags = flags;
	playerActor->GetState().takeDamage = true;
	playerActor->GetState().contents = CONTENTS_BODY;
	playerActor->GetState().viewHeight = DEFAULT_VIEWHEIGHT;
	playerActor->GetState().stats[STAT_WEAPONS] |= (1 << WP_SHOTGUN);
	playerActor->GetState().ammo[WP_SHOTGUN] = 1000;
	playerActor->GetState().stats[STAT_WEAPONS] |= (1 << WP_ROCKET_LAUNCHER);
	playerActor->GetState().ammo[WP_ROCKET_LAUNCHER] = 1000;
	playerActor->GetState().stats[STAT_WEAPONS] |= (1 << WP_RAILGUN);
	playerActor->GetState().ammo[WP_RAILGUN] = 1000;
	playerActor->GetState().stats[STAT_WEAPONS] |= (1 << WP_PLASMAGUN);
	playerActor->GetState().ammo[WP_PLASMAGUN] = 1000;
	playerActor->GetState().stats[STAT_WEAPONS] |= (1 << WP_MACHINEGUN);
	playerActor->GetState().ammo[WP_MACHINEGUN] = 1000;
	playerActor->GetState().stats[STAT_WEAPONS] |= (1 << WP_LIGHTNING);
	playerActor->GetState().ammo[WP_LIGHTNING] = 1000;
	playerActor->GetState().stats[STAT_WEAPONS] |= (1 << WP_GRENADE_LAUNCHER);
	playerActor->GetState().ammo[WP_GRENADE_LAUNCHER] = 1000;
	playerActor->GetState().stats[STAT_WEAPONS] |= (1 << WP_GAUNTLET);
	playerActor->GetState().ammo[WP_GAUNTLET] = -1;

	// health will count down towards max_health
	playerActor->GetState().stats[STAT_HEALTH] = playerActor->GetState().stats[STAT_MAX_HEALTH] + 25;

	// find a spawn point
	Transform spawnTransform;
	eastl::shared_ptr<Actor> spawnPoint;
	eastl::shared_ptr<TransformComponent> pTransformComponent(
		playerActor->GetComponent<TransformComponent>(TransformComponent::Name).lock());
	if (pTransformComponent)
	{
		spawnPoint = SelectSpawnPoint(
			pTransformComponent->GetTransform().GetTranslation(), spawnTransform);
		pTransformComponent->SetTransform(spawnTransform);
	}

	// force the base weapon up
	playerActor->GetAction().weaponSelect = WP_MACHINEGUN;
	playerActor->GetAction().weaponSelectTime = 0;

	playerActor->GetState().weapon = WP_MACHINEGUN;
	playerActor->GetState().weaponState = WEAPON_READY;

	// don't allow full run speed for a bit
	//playerActor->GetState().moveFlags |= PMF_TIME_KNOCKBACK;
	playerActor->GetState().moveTime = 100;
	//playerActor->GetState().respawnTime = level.time;

	// set default animations
	playerActor->GetState().torsoAnim = TORSO_STAND;
	playerActor->GetState().legsAnim = LEGS_IDLE;

	// fire the targets of the spawn point
	EventManager::Get()->TriggerEvent(
		eastl::make_shared<QuakeEventDataSpawnActor>(playerActor->GetId(), spawnTransform));
}

int PickupAmmo(const eastl::shared_ptr<PlayerActor>& player, const eastl::shared_ptr<AmmoPickup>& ammo)
{
	player->GetState().ammo[ammo->GetType()] += ammo->GetAmount();
	if (player->GetState().ammo[ammo->GetType()] > 200)
		player->GetState().ammo[ammo->GetType()] = 200;

	return ammo->GetWait();
}

int PickupWeapon(const eastl::shared_ptr<PlayerActor>& player, const eastl::shared_ptr<WeaponPickup>& weapon)
{
	// add the weapon
	player->GetState().stats[STAT_WEAPONS] |= (1 << weapon->GetType());

	// add ammo
	player->GetState().ammo[weapon->GetType()] += weapon->GetAmmo();
	if (player->GetState().ammo[weapon->GetType()] > 200)
		player->GetState().ammo[weapon->GetType()] = 200;

	return weapon->GetWait();
}

int PickupHealth(const eastl::shared_ptr<PlayerActor>& player, const eastl::shared_ptr<HealthPickup>& health)
{
	int max;
	if (health->GetAmount() != 5 && health->GetAmount() != 100)
		max = player->GetState().stats[STAT_MAX_HEALTH];
	else
		max = player->GetState().stats[STAT_MAX_HEALTH] * 2;

	player->GetState().stats[STAT_HEALTH] += health->GetAmount();
	if (player->GetState().stats[STAT_HEALTH] > max)
		player->GetState().stats[STAT_HEALTH] = max;

	return health->GetWait();
}

int PickupArmor(const eastl::shared_ptr<PlayerActor>& player, const eastl::shared_ptr<ArmorPickup>& armor)
{
	player->GetState().stats[STAT_ARMOR] += armor->GetAmount();
	if (player->GetState().stats[STAT_ARMOR] > player->GetState().stats[STAT_MAX_HEALTH] * 2)
		player->GetState().stats[STAT_ARMOR] = player->GetState().stats[STAT_MAX_HEALTH] * 2;

	return armor->GetWait();
}

/*
CanItemBeGrabbed
Returns false if the item should not be picked up.
*/
bool CanItemBeGrabbed(const eastl::shared_ptr<Actor>& item, const eastl::shared_ptr<PlayerActor>& player)
{
	if (item->GetType() == "Ammo")
	{
		eastl::shared_ptr<AmmoPickup> pAmmoPickup = 
			item->GetComponent<AmmoPickup>(AmmoPickup::Name).lock();
		if (pAmmoPickup)
		{
			if (player->GetState().ammo[pAmmoPickup->GetCode()] >= 200)
				return false;		// can't hold any more

			return true;
		}
	}
	else if (item->GetType() == "Armor")
	{
		if (player->GetState().stats[STAT_ARMOR] >= 
			player->GetState().stats[STAT_MAX_HEALTH] * 2)
			return false;

		return true;
	}
	else if (item->GetType() == "Health")
	{
		// small and mega healths will go over the max, otherwise
		// don't pick up if already at max
		eastl::shared_ptr<HealthPickup> pHealthPickup =
			item->GetComponent<HealthPickup>(HealthPickup::Name).lock();
		if (pHealthPickup)
		{
			if (pHealthPickup->GetAmount() == 5 || pHealthPickup->GetAmount() == 100)
			{
				if (player->GetState().stats[STAT_HEALTH] >= 
					player->GetState().stats[STAT_MAX_HEALTH] * 2)
					return false;

				return true;
			}

			if (player->GetState().stats[STAT_HEALTH] >= 
				player->GetState().stats[STAT_MAX_HEALTH])
				return false;

			return true;
		}
	}
	else if (item->GetType() == "Weapon")
	{
		return true;	// weapons are always picked up
	}
	else
	{ 
		LogError("Unknown type " + item->GetType());
	}

	return false;
}

void QuakeLogic::PhysicsTriggerEnterDelegate(BaseEventDataPtr pEventData)
{
	eastl::shared_ptr<EventDataPhysTriggerEnter> pCastEventData =
		eastl::static_pointer_cast<EventDataPhysTriggerEnter>(pEventData);

	eastl::shared_ptr<Actor> pItemActor(
		GameLogic::Get()->GetActor(pCastEventData->GetTriggerId()).lock());

	eastl::shared_ptr<PlayerActor> pPlayerActor(
		eastl::dynamic_shared_pointer_cast<PlayerActor>(
		GameLogic::Get()->GetActor(pCastEventData->GetOtherActor()).lock()));

	if (pPlayerActor)
	{
		// dead players
		if (pPlayerActor->GetState().stats[STAT_HEALTH] <= 0)
			return;

		if (!CanItemBeGrabbed(pItemActor, pPlayerActor))
			return; // can't hold it

		int respawn;
		if (pItemActor->GetType() == "Weapon")
		{
			eastl::shared_ptr<WeaponPickup> pWeaponPickup =
				pItemActor->GetComponent<WeaponPickup>(WeaponPickup::Name).lock();
			respawn = PickupWeapon(pPlayerActor, pWeaponPickup);
		}
		else if (pItemActor->GetType() == "Ammo")
		{
			eastl::shared_ptr<AmmoPickup> pAmmoPickup =
				pItemActor->GetComponent<AmmoPickup>(AmmoPickup::Name).lock();
			respawn = PickupAmmo(pPlayerActor, pAmmoPickup);
		}
		else if (pItemActor->GetType() == "Armor")
		{
			eastl::shared_ptr<ArmorPickup> pArmorPickup =
				pItemActor->GetComponent<ArmorPickup>(ArmorPickup::Name).lock();
			respawn = PickupArmor(pPlayerActor, pArmorPickup);
		}
		else if (pItemActor->GetType() == "Health")
		{
			eastl::shared_ptr<HealthPickup> pHealthPickup =
				pItemActor->GetComponent<HealthPickup>(HealthPickup::Name).lock();
			respawn = PickupHealth(pPlayerActor, pHealthPickup);
		}

		// play the normal pickup sound
		//QueueEvent(EventType.EvtData_PlaySound, "audio\\computerbeep3.wav");
		// global sound to play
		//QueueEvent(EventType.EvtData_PlaySound, "audio\\computerbeep3.wav");
		// dropped items will not respawn
		//QueueEvent(EventType.EvtData_Request_Destroy_Actor, sphere:GetActorId());

		if (pItemActor->GetType() == "Weapon")
		{
			eastl::shared_ptr<WeaponPickup> pWeaponPickup =
				pItemActor->GetComponent<WeaponPickup>(WeaponPickup::Name).lock();
			if (pWeaponPickup)
			{
				pPlayerActor->GetState().stats[STAT_WEAPONS] |= 1 << pWeaponPickup->GetCode();
				if (!pPlayerActor->GetState().ammo[pWeaponPickup->GetCode()])
					pPlayerActor->GetState().ammo[pWeaponPickup->GetCode()] = 1;
			}
		}
	}
}


void QuakeLogic::PhysicsTriggerLeaveDelegate(BaseEventDataPtr pEventData)
{
	eastl::shared_ptr<EventDataPhysTriggerLeave> pCastEventData =
		eastl::static_pointer_cast<EventDataPhysTriggerLeave>(pEventData);

	eastl::shared_ptr<Actor> pTrigger(
		GameLogic::Get()->GetActor(pCastEventData->GetTriggerId()).lock());
	eastl::shared_ptr<PlayerActor> pPlayerActor(
		eastl::dynamic_shared_pointer_cast<PlayerActor>(
		GameLogic::Get()->GetActor(pCastEventData->GetOtherActor()).lock()));
}


void QuakeLogic::PhysicsCollisionDelegate(BaseEventDataPtr pEventData)
{
	eastl::shared_ptr<EventDataPhysCollision> pCastEventData =
		eastl::static_pointer_cast<EventDataPhysCollision>(pEventData);

	eastl::shared_ptr<Actor> pGameActorA(
		GameLogic::Get()->GetActor(pCastEventData->GetActorA()).lock());
	eastl::shared_ptr<Actor> pGameActorB(
		GameLogic::Get()->GetActor(pCastEventData->GetActorB()).lock());
	if (pGameActorA && pGameActorB)
	{
		eastl::shared_ptr<Actor> pItemActor;
		eastl::shared_ptr<PlayerActor> pPlayerActor;
		if (eastl::dynamic_shared_pointer_cast<PlayerActor>(pGameActorA) &&
			!eastl::dynamic_shared_pointer_cast<PlayerActor>(pGameActorB))
		{
			pPlayerActor = eastl::dynamic_shared_pointer_cast<PlayerActor>(pGameActorA);
			pItemActor = pGameActorB;

		}
		else if (!eastl::dynamic_shared_pointer_cast<PlayerActor>(pGameActorA) &&
				eastl::dynamic_shared_pointer_cast<PlayerActor>(pGameActorB))
		{
			pPlayerActor = eastl::dynamic_shared_pointer_cast<PlayerActor>(pGameActorB);
			pItemActor = pGameActorA;
		}
		else return;

		// dead players
		if (pPlayerActor->GetState().stats[STAT_HEALTH] <= 0)
			return;

		if (pItemActor->GetType() == "Trigger")
		{
			eastl::shared_ptr<TeleporterTrigger> pTeleporterTrigger =
				pItemActor->GetComponent<TeleporterTrigger>(TeleporterTrigger::Name).lock();
			if (pTeleporterTrigger)
			{

			}

			eastl::shared_ptr<PushTrigger> pPushTrigger =
				pItemActor->GetComponent<PushTrigger>(PushTrigger::Name).lock();
			if (pPushTrigger)
			{

			}
		}
		else
		{
			
		}
	}
}

void QuakeLogic::PhysicsSeparationDelegate(BaseEventDataPtr pEventData)
{
	eastl::shared_ptr<EventDataPhysSeparation> pCastEventData =
		eastl::static_pointer_cast<EventDataPhysSeparation>(pEventData);

	eastl::shared_ptr<Actor> pGameActorA(
		GameLogic::Get()->GetActor(pCastEventData->GetActorA()).lock());
	eastl::shared_ptr<Actor> pGameActorB(
		GameLogic::Get()->GetActor(pCastEventData->GetActorB()).lock());
	if (pGameActorA && pGameActorB)
	{
		eastl::shared_ptr<Actor> pItemActor;
		eastl::shared_ptr<PlayerActor> pPlayerActor;
		if (eastl::dynamic_shared_pointer_cast<PlayerActor>(pGameActorA) &&
			!eastl::dynamic_shared_pointer_cast<PlayerActor>(pGameActorB))
		{
			pPlayerActor = eastl::dynamic_shared_pointer_cast<PlayerActor>(pGameActorA);
			pItemActor = pGameActorB;

		}
		else if (!eastl::dynamic_shared_pointer_cast<PlayerActor>(pGameActorA) &&
				eastl::dynamic_shared_pointer_cast<PlayerActor>(pGameActorB))
		{
			pPlayerActor = eastl::dynamic_shared_pointer_cast<PlayerActor>(pGameActorB);
			pItemActor = pGameActorA;
		}
		else return;
	}
}