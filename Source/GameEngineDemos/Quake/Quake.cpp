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
					eastl::shared_ptr<Actor> pActor = 
						CreatePlayerActor("actors\\quake\\players\\player.xml", NULL);
					if (pActor)
					{
						pView->OnAttach(pView->GetId(), pActor->GetId());

						eastl::shared_ptr<EventDataNewActor> pNewActorEvent(
							new EventDataNewActor(pActor->GetId(), pView->GetId()));
						BaseEventManager::Get()->TriggerEvent(pNewActorEvent);
					}
				}
				else if (pView->GetType() == GV_REMOTE)
				{
					eastl::shared_ptr<NetworkGameView> pNetworkGameView =
						eastl::static_pointer_cast<NetworkGameView, BaseGameView>(pView);
					eastl::shared_ptr<Actor> pActor = 
						CreatePlayerActor("actors\\quake\\players\\remote_player.xml", NULL);
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
					eastl::shared_ptr<QuakeAIPlayerView> pAiView = 
						eastl::static_pointer_cast<QuakeAIPlayerView, BaseGameView>(pView);
					eastl::shared_ptr<Actor> pActor = 
						CreatePlayerActor("actors\\quake\\players\\ai_player.xml", NULL);
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

void QuakeLogic::FireWeaponDelegate(BaseEventDataPtr pEventData)
{
	eastl::shared_ptr<QuakeEventDataJumpActor> pCastEventData =
		eastl::static_pointer_cast<QuakeEventDataJumpActor>(pEventData);

	eastl::shared_ptr<Actor> pGameActor(
		GameLogic::Get()->GetActor(pCastEventData->GetId()).lock());
	if (pGameActor)
	{
		/*
		local aggressor = self:GetActorById(eventData.id);

		if (aggressor == nil) then
			print("FireWeapon from noone?");
		return;
		end;

		print("FireWeapon!");
		local pos = Vec3:Create(aggressor : GetPos());
		local lookAt = Vec3:Create(aggressor : GetLookAt());
		lookAt.y = lookAt.y + 1;
		local dir = lookAt * 2;
		pos = pos + dir;
		local ypr = Vec3:Create({ x = 0, y = 0, z = 0 });
		local ball = CreateActor("actors\\sphere.xml", pos, ypr);
		if (ball ~= -1) then
			dir : Normalize();
		ApplyForce(dir, .3, ball);
		end
		*/
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
		MakeDelegate(this, &QuakeLogic::PhysicsCollisionDelegate),
		EventDataPhysCollision::skEventType);

	pGlobalEventManager->AddListener(
		MakeDelegate(this, &QuakeLogic::FireWeaponDelegate),
		QuakeEventDataFireWeapon::skEventType);
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
		MakeDelegate(this, &QuakeLogic::PhysicsCollisionDelegate),
		EventDataPhysCollision::skEventType);
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
		EventDataPhysCollision::skEventType);
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
			EventDataPhysCollision::skEventType);
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


eastl::shared_ptr<Actor> QuakeLogic::CreatePlayerActor(const eastl::string &actorResource, 
	tinyxml2::XMLElement *overrides, const Transform *initialTransform, const ActorId serversActorId)
{
	QuakeActorFactory* actorFactory = dynamic_cast<QuakeActorFactory*>(mActorFactory);
	LogAssert(actorFactory, "quake actor factory is not initialized");
	if (!mIsProxy && serversActorId != INVALID_ACTOR_ID)
		return eastl::shared_ptr<Actor>();

	if (mIsProxy && serversActorId == INVALID_ACTOR_ID)
		return eastl::shared_ptr<Actor>();

	eastl::shared_ptr<Actor> pActor = actorFactory->CreatePlayerActor(
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
		return eastl::shared_ptr<Actor>();
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
					targetResources["info_player_deathmatch"] = "actors/quake/target/destination.xml";
					targetResources["info_player_intermission"] = "actors/quake/target/destination.xml";
					targetResources["info_notnull"] = "actors/quake/target/destination.xml";
					targetResources["info_null"] = "actors/quake/target/destination.xml";
					targetResources["target_speaker"] = "actors/quake/target/speaker.xml";
					triggerResources["trigger_teleport"] = "actors/quake/trigger/teleporter.xml";
					triggerResources["trigger_push"] = "actors/quake/trigger/push.xml";

					const eastl::shared_ptr<BspResourceExtraData>& extra =
						eastl::static_pointer_cast<BspResourceExtraData>(resHandle->GetExtra());
					BspLoader& bspLoader = extra->GetLoader();
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
											if (className == "info_player_deathmatch")
											{
												eastl::shared_ptr<RespawnTarget> pRespawnTarget(
													pActor->GetComponent<RespawnTarget>(RespawnTarget::Name).lock());
												if (pRespawnTarget)
													pRespawnTarget->SetTarget(target);
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
													pAudioComponent->AddAudio("audio/quake/" + audios.substr(audioBegin, audioEnd));

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
											pTeleporterTrigger->SetModel(model);
											pTeleporterTrigger->SetTarget(target);
										}
									}
									else if (className == "trigger_push")
									{
										eastl::shared_ptr<PushTrigger> pPushTrigger(
											pActor->GetComponent<PushTrigger>(PushTrigger::Name).lock());
										if (pPushTrigger)
										{
											pPushTrigger->SetModel(model);
											pPushTrigger->SetTarget(target);
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
											pAudioComponent->AddAudio("audio/quake/" + audios.substr(audioBegin, audioEnd));

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
		if (eastl::dynamic_shared_pointer_cast<PlayerActor>(pGameActorA) &&
			!eastl::dynamic_shared_pointer_cast<PlayerActor>(pGameActorB))
		{
			eastl::shared_ptr<PlayerActor> pPlayerActor =
				eastl::dynamic_shared_pointer_cast<PlayerActor>(pGameActorA);

			// dead players don't activate triggers
			if (pPlayerActor->GetState().stats[STAT_HEALTH] <= 0)
				return;

			if (pGameActorB->GetType() == "Trigger")
			{
				eastl::shared_ptr<TeleporterTrigger> pTeleporterTrigger =
					pGameActorB->GetComponent<TeleporterTrigger>(TeleporterTrigger::Name).lock();
				if (pTeleporterTrigger)
				{

				}

				eastl::shared_ptr<PushTrigger> pPushTrigger =
					pGameActorB->GetComponent<PushTrigger>(PushTrigger::Name).lock();
				if (pPushTrigger)
				{

				}
			}
			else
			{
				if (!CanItemBeGrabbed(pGameActorB, pPlayerActor))
					return; // can't hold it

				if (pGameActorB->GetType() == "Weapon")
				{
					eastl::shared_ptr<WeaponPickup> pWeaponPickup =
						pGameActorB->GetComponent<WeaponPickup>(WeaponPickup::Name).lock();
					if (pWeaponPickup)
					{
						pPlayerActor->GetState().stats[STAT_WEAPONS] |= 1 << pWeaponPickup->GetId();
						if (!pPlayerActor->GetState().ammo[pWeaponPickup->GetId()])
							pPlayerActor->GetState().ammo[pWeaponPickup->GetId()] = 1;
					}
				}
			}
		}
		else if (!eastl::dynamic_shared_pointer_cast<PlayerActor>(pGameActorA) &&
			eastl::dynamic_shared_pointer_cast<PlayerActor>(pGameActorB))
		{
			eastl::shared_ptr<PlayerActor> pPlayerActor =
				eastl::dynamic_shared_pointer_cast<PlayerActor>(pGameActorB);

			// dead players don't activate triggers
			if (pPlayerActor->GetState().stats[STAT_HEALTH] <= 0)
				return;

			if (!CanItemBeGrabbed(pGameActorA, pPlayerActor)) 
				return; // can't hold it

			if (pGameActorA->GetType() == "Weapon")
			{
				eastl::shared_ptr<WeaponPickup> pWeaponPickup =
					pGameActorA->GetComponent<WeaponPickup>(WeaponPickup::Name).lock();
				if (pWeaponPickup)
				{
					pPlayerActor->GetState().stats[STAT_WEAPONS] |= 1 << pWeaponPickup->GetId();
					if (!pPlayerActor->GetState().ammo[pWeaponPickup->GetId()])
						pPlayerActor->GetState().ammo[pWeaponPickup->GetId()] = 1;
				}
			}
		}
	}
	/*
	local actorA = self:GetActorById(eventData.actorA);
	local actorB = self:GetActorById(eventData.actorB);

	--one of the actors isn't in the script manager
	if (actorA == nil or actorB == nil) then
	return;
	end

	local teapot = nil;
	local sphere = nil;

	if (actorA.actorType == "Teapot" and actorB.actorType == "Sphere") then
	teapot = actorA;
	sphere = actorB;
	elseif(actorA.actorType == "Sphere" and actorB.actorType == "Teapot") then
	teapot = actorB;
	sphere = actorA;
	end

	-- needs to be a teapot and sphere collision for us to care
	if (teapot == nil or sphere == nil) then
	return;
	end

	-- If we get here, there was a collision between a teapot and a sphere.Damage the teapot.
	self:_DamageTeapot(teapot);

	--destroy the sphere
	self : RemoveSphere(sphere);
	QueueEvent(EventType.EvtData_Request_Destroy_Actor, sphere:GetActorId());

	--play the hit sound
	QueueEvent(EventType.EvtData_PlaySound, "audio\\computerbeep3.wav");
	*/
}


/*
========================
TouchJumpPad
========================
*/
/*
void TouchJumpPad(const eastl::shared_ptr<PlayerActor>& player, EntityState *jumppad)
{
	Vector3<float>	angles;
	float p;
	int effectNum;

	// spectators don't use jump pads
	if (ps->pm_type != PM_NORMAL)
	{
		return;
	}

	// flying characters don't hit bounce pads
	if (ps->powerups[PW_FLIGHT])
	{
		return;
	}

	// if we didn't hit this same jumppad the previous frame
	// then don't play the event sound again if we are in a fat trigger
	if (ps->jumppad_ent != jumppad->number)
	{
		vectoangles(jumppad->origin2, angles);
		p = fabs(AngleNormalize180(angles[PITCH]));
		if (p < 45)
		{
			effectNum = 0;
		}
		else
		{
			effectNum = 1;
		}
		AddPredictableEventToPlayerstate(EV_JUMP_PAD, effectNum, ps);
	}
	// remember hitting this jumppad this frame
	ps->jumppad_ent = jumppad->number;
	ps->jumppad_frame = ps->pmove_framecount;
	// give the player the velocity from the jumppad
	ps->velocity = Vector3<float>{ jumppad->origin2 };
}
*/
/*
================
CanItemBeGrabbed
Returns false if the item should not be picked up.
================
*/
bool QuakeLogic::CanItemBeGrabbed(const eastl::shared_ptr<Actor>& item, const eastl::shared_ptr<PlayerActor>& player)
{
	if (item->GetType() == "Ammo")
	{
		eastl::shared_ptr<AmmoPickup> pAmmoPickup = 
			item->GetComponent<AmmoPickup>(AmmoPickup::Name).lock();
		if (pAmmoPickup)
		{
			if (player->GetState().ammo[pAmmoPickup->GetId()] >= 200)
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
