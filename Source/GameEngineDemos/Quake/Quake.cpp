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
#include "QuakeManager.h"

//
// QuakeLogic::QuakeLogic
//
QuakeLogic::QuakeLogic()
	: GameLogic()
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
					eastl::shared_ptr<Actor> pActor = CreateActor("actors\\quake\\players\\player.xml", NULL);
					if (pActor)
					{
						pView->OnAttach(pView->GetId(), pActor->GetId());

						eastl::shared_ptr<EventDataNewActor> pNewActorEvent(
							new EventDataNewActor(pActor->GetId(), pView->GetId()));

						// [rez] This needs to happen asap because the constructor function for Lua 
						// (which is called in through VCreateActor()) queues an event that expects 
						// this event to have been handled
						BaseEventManager::Get()->TriggerEvent(pNewActorEvent);
					}
				}
				else if (pView->GetType() == GV_REMOTE)
				{
					eastl::shared_ptr<NetworkGameView> pNetworkGameView =
						eastl::static_pointer_cast<NetworkGameView, BaseGameView>(pView);
					eastl::shared_ptr<Actor> pActor = CreateActor("actors\\quake\\players\\remote_player.xml", NULL);
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
					eastl::shared_ptr<Actor> pActor = CreateActor("actors\\quake\\players\\ai_player.xml", NULL);
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
		MakeDelegate(this, &QuakeLogic::JumpActorDelegate),
		QuakeEventDataJumpActor::skEventType);
	pGlobalEventManager->AddListener(
		MakeDelegate(this, &QuakeLogic::MoveActorDelegate),
		QuakeEventDataMoveActor::skEventType);
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
	if (mIsProxy)
	{
		pGlobalEventManager->RemoveListener(
			MakeDelegate(this, &QuakeLogic::RequestNewActorDelegate), 
			EventDataRequestNewActor::skEventType);
	}

	pGlobalEventManager->RemoveListener(
		MakeDelegate(this, &QuakeLogic::JumpActorDelegate),
		QuakeEventDataJumpActor::skEventType);
	pGlobalEventManager->RemoveListener(
		MakeDelegate(this, &QuakeLogic::MoveActorDelegate),
		QuakeEventDataMoveActor::skEventType);
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
					targetResources["info_player_deathmatch"] = "actors/quake/target/playerdeathmatch.xml";
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
								BSPVector3 origin;
								if (bspLoader.GetVectorForKey(&entity, "origin", origin))
								{
									Transform initTransform;
									initTransform.SetTranslation(origin[0], origin[1], origin[2]);
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

										// fire an event letting everyone else know that we created a new actor
										eastl::shared_ptr<EventDataNewActor> pNewActorEvent(
											new EventDataNewActor(pActor->GetId()));
										BaseEventManager::Get()->QueueEvent(pNewActorEvent);
									}
								}
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