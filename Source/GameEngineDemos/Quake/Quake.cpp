//========================================================================
//
// QuakeLogic Implementation       - Chapter 21, page 725
//
//========================================================================

#include "Physic/Physic.h"
#include "Physic/PhysicEventListener.h"
#include "Physic/Importer/PhysicResource.h"

#include "Quake.h"
#include "QuakeApp.h"
#include "QuakeView.h"
#include "QuakeAIProcess.h"
#include "QuakeAIView.h"
#include "QuakeNetwork.h"
#include "QuakeEvents.h"
#include "QuakeActorFactory.h"
#include "QuakeLevelManager.h"
#include "QuakeAIManager.h"

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

	//  This is commented out because while the view is created and waiting, the player is NOT attached yet. 
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

// Quake Actors
eastl::shared_ptr<Actor> QuakeLogic::GetRandomActor()
{
	eastl::vector<eastl::shared_ptr<Actor>> actors;
	GetAmmoActors(actors);
	GetWeaponActors(actors);
	GetHealthActors(actors);
	GetArmorActors(actors);

	int selection = Randomizer::Rand() % actors.size();
	return actors[selection];
}

void QuakeLogic::GetAmmoActors(eastl::vector<eastl::shared_ptr<Actor>>& ammo)
{
	for (auto actor : mActors)
	{
		eastl::shared_ptr<Actor> pActor = actor.second;
		if (pActor->GetType() == "Ammo")
			ammo.push_back(pActor);
	}
}

void QuakeLogic::GetArmorActors(eastl::vector<eastl::shared_ptr<Actor>>& armor)
{
	for (auto actor : mActors)
	{
		eastl::shared_ptr<Actor> pActor = actor.second;
		if (pActor->GetType() == "Armor")
			armor.push_back(pActor);
	}
}

void QuakeLogic::GetWeaponActors(eastl::vector<eastl::shared_ptr<Actor>>& weapon)
{
	for (auto actor : mActors)
	{
		eastl::shared_ptr<Actor> pActor = actor.second;
		if (pActor->GetType() == "Weapon")
			weapon.push_back(pActor);
	}
}

void QuakeLogic::GetHealthActors(eastl::vector<eastl::shared_ptr<Actor>>& health)
{
	for (auto actor : mActors)
	{
		eastl::shared_ptr<Actor> pActor = actor.second;
		if (pActor->GetType() == "Health")
			health.push_back(pActor);
	}
}

void QuakeLogic::GetPlayerActors(eastl::vector<eastl::shared_ptr<PlayerActor>>& player)
{
	for (auto actor : mActors)
	{
		eastl::shared_ptr<Actor> pActor = actor.second;
		if (pActor->GetType() == "Player")
			player.push_back(eastl::dynamic_shared_pointer_cast<PlayerActor>(pActor));
	}
}

void QuakeLogic::GetTriggerActors(eastl::vector<eastl::shared_ptr<Actor>>& trigger)
{
	for (auto actor : mActors)
	{
		eastl::shared_ptr<Actor> pActor = actor.second;
		if (pActor->GetType() == "Trigger")
			trigger.push_back(pActor);
	}
}

void QuakeLogic::GetTargetActors(eastl::vector<eastl::shared_ptr<Actor>>& target)
{
	for (auto actor : mActors)
	{
		eastl::shared_ptr<Actor> pActor = actor.second;
		if (pActor->GetType() == "Target")
			target.push_back(pActor);
	}
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
				eastl::shared_ptr<BaseGameView> aiView(new QuakeAIView());
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
						pPlayerActor->PlayerSpawn();
						pView->OnAttach(pView->GetId(), pPlayerActor->GetId());

						if (!gameApp->mOption.mLevelMod)
						{
							eastl::shared_ptr<EventDataNewActor> pNewActorEvent(
								new EventDataNewActor(pPlayerActor->GetId(), pView->GetId()));
							BaseEventManager::Get()->TriggerEvent(pNewActorEvent);
						}
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
						pPlayerActor->PlayerSpawn();
						pView->OnAttach(pView->GetId(), pPlayerActor->GetId());

						eastl::shared_ptr<EventDataNewActor> pNewActorEvent(
							new EventDataNewActor(pPlayerActor->GetId(), pNetworkGameView->GetId()));
						BaseEventManager::Get()->TriggerEvent(pNewActorEvent);
					}
				}
				else if (pView->GetType() == GV_AI)
				{
					eastl::shared_ptr<QuakeAIView> pAiView = 
						eastl::static_pointer_cast<QuakeAIView, BaseGameView>(pView);
					eastl::shared_ptr<PlayerActor> pPlayerActor =
						CreatePlayerActor("actors\\quake\\players\\ai_player.xml", NULL);
					if (pPlayerActor)
					{
						pPlayerActor->PlayerSpawn();
						pAiView->OnAttach(pView->GetId(), pPlayerActor->GetId());

						eastl::shared_ptr<EventDataNewActor> pNewActorEvent(
							new EventDataNewActor(pPlayerActor->GetId(), pAiView->GetId()));
						BaseEventManager::Get()->TriggerEvent(pNewActorEvent);
					}
				}
			}

			if (gameApp->mOption.mLevelMod)
			{
				for (auto it = gameViews.begin(); it != gameViews.end(); ++it)
				{
					eastl::shared_ptr<BaseGameView> pView = *it;
					if (pView->GetType() == GV_HUMAN)
					{
						eastl::shared_ptr<BaseGameView> pAiView(new QuakeAIView());
						gameApp->AddView(pAiView);

						pAiView->OnAttach(pAiView->GetId(), pView->GetActorId());

						eastl::shared_ptr<EventDataNewActor> pNewActorEvent(
							new EventDataNewActor(pView->GetActorId(), pAiView->GetId()));
						BaseEventManager::Get()->TriggerEvent(pNewActorEvent);

						break;
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

	eastl::shared_ptr<PlayerActor> pPlayerActor(
		eastl::dynamic_shared_pointer_cast<PlayerActor>(
		GameLogic::Get()->GetActor(pCastEventData->GetId()).lock()));
	if (pPlayerActor)
	{
		eastl::shared_ptr<PhysicComponent> pPhysicalComponent =
			pPlayerActor->GetComponent<PhysicComponent>(PhysicComponent::Name).lock();
		if (pPhysicalComponent)
			pPhysicalComponent->KinematicJump(pCastEventData->GetDirection());

		if (pPlayerActor->GetAction().triggerPush != INVALID_ACTOR_ID)
		{
			pPlayerActor->GetAction().triggerPush = INVALID_ACTOR_ID;

			// play jumppad sound
			EventManager::Get()->TriggerEvent(
				eastl::make_shared<EventDataPlaySound>("audio/quake/sound/world/jumppad.wav"));
		}
		else
		{
			if (pPlayerActor->GetState().jumpTime == 0)
			{
				pPlayerActor->GetState().jumpTime = 200;

				// play jump sound
				EventManager::Get()->TriggerEvent(
					eastl::make_shared<EventDataPlaySound>("audio/quake/sound/player/jump1.wav"));
			}
		}
	}
}

void QuakeLogic::SpawnActorDelegate(BaseEventDataPtr pEventData)
{
	eastl::shared_ptr<QuakeEventDataSpawnActor> pCastEventData =
		eastl::static_pointer_cast<QuakeEventDataSpawnActor>(pEventData);

	eastl::shared_ptr<PlayerActor> pPlayerActor(
		eastl::dynamic_shared_pointer_cast<PlayerActor>(
		GameLogic::Get()->GetActor(pCastEventData->GetId()).lock()));
	if (pPlayerActor)
	{
		// find a spawn point
		Transform spawnTransform;
		eastl::shared_ptr<TransformComponent> pTransformComponent(
			pPlayerActor->GetComponent<TransformComponent>(TransformComponent::Name).lock());
		if (pTransformComponent)
		{
			SelectSpawnPoint(pTransformComponent->GetTransform().GetTranslation(), spawnTransform);
			pTransformComponent->SetTransform(spawnTransform);

			QuakeAIManager* aiManager =
				dynamic_cast<QuakeAIManager*>(GameLogic::Get()->GetAIManager());
			aiManager->SpawnActor(pPlayerActor->GetId());
		}

		eastl::shared_ptr<PhysicComponent> pPhysicalComponent =
			pPlayerActor->GetComponent<PhysicComponent>(PhysicComponent::Name).lock();
		if (pPhysicalComponent)
			pPhysicalComponent->SetTransform(spawnTransform);

		// play teleporter sound
		EventManager::Get()->TriggerEvent(
			eastl::make_shared<EventDataPlaySound>("audio/quake/sound/world/teleout.ogg"));
	}
}

void QuakeLogic::MoveActorDelegate(BaseEventDataPtr pEventData)
{
	eastl::shared_ptr<QuakeEventDataMoveActor> pCastEventData =
		eastl::static_pointer_cast<QuakeEventDataMoveActor>(pEventData);

	eastl::shared_ptr<PlayerActor> pPlayerActor(
		eastl::dynamic_shared_pointer_cast<PlayerActor>(
			GameLogic::Get()->GetActor(pCastEventData->GetId()).lock()));
	if (pPlayerActor)
	{
		eastl::shared_ptr<PhysicComponent> pPhysicalComponent =
			pPlayerActor->GetComponent<PhysicComponent>(PhysicComponent::Name).lock();
		if (pPhysicalComponent)
		{
			pPhysicalComponent->KinematicMove(pCastEventData->GetDirection());

			if (pPhysicalComponent->OnGround() && 
				pPlayerActor->GetState().moveTime == 0 &&
				Length(pCastEventData->GetDirection()) > 0.f)
			{
				pPlayerActor->GetState().moveTime = 400;

				// play footstep sound
				EventManager::Get()->TriggerEvent(
					eastl::make_shared<EventDataPlaySound>("audio/quake/sound/player/footsteps/boot1.ogg"));
			}
		}
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
		MakeDelegate(this, &QuakeLogic::SplashDamageDelegate),
		QuakeEventDataSplashDamage::skEventType);
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
		MakeDelegate(this, &QuakeLogic::SplashDamageDelegate),
		QuakeEventDataSplashDamage::skEventType);
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
		QuakeEventDataSplashDamage::skEventType);
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
			QuakeEventDataSplashDamage::skEventType);
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

AIManager* QuakeLogic::CreateAIManager(void)
{
	QuakeAIManager* aiManager = new QuakeAIManager();
	return aiManager;
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
	eastl::string levelPath = 
		"ai/quake/" + eastl::string(pLevelData->Attribute("name")) + ".bin";
	GetAIManager()->LoadPathingGraph(
		ToWideString(FileSystem::Get()->GetPath(levelPath.c_str()).c_str()));
	AttachProcess(eastl::shared_ptr<Process>(new QuakeAIProcess()));

	for (auto actor : mActors)
	{
		eastl::shared_ptr<Actor> pActor = actor.second;
		eastl::shared_ptr<PhysicComponent> pPhysicalComponent =
			pActor->GetComponent<PhysicComponent>(PhysicComponent::Name).lock();
		if (pPhysicalComponent)
		{
			if (pPhysicalComponent->GetShape() == "BSP")
			{
				eastl::shared_ptr<ResHandle>& resHandle = ResCache::Get()->GetHandle(
					&BaseResource(ToWideString(pPhysicalComponent->GetMesh().c_str())));
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
					//targetResources["target_speaker"] = "actors/quake/target/speaker.xml";
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

													eastl::shared_ptr<TransformComponent> pTransform(
														pActor->GetComponent<TransformComponent>(TransformComponent::Name).lock());
													if (pTransform)
														pTransform->SetPosition(gamePhysics->GetCenter(pActor->GetId()));
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

eastl::shared_ptr<Actor> CreateItemWeapon(WeaponType weapon, const Transform& initTransform)
{
	eastl::shared_ptr<Actor> pActor;
	switch (weapon)
	{
		case WP_SHOTGUN:
			pActor = GameLogic::Get()->CreateActor(
				"actors/quake/models/weapon/shotgun.xml", nullptr, &initTransform);
			break;
		case WP_GRENADE_LAUNCHER:
			pActor = GameLogic::Get()->CreateActor(
				"actors/quake/models/weapon/grenadelauncher.xml", nullptr, &initTransform);
			break;
		case WP_ROCKET_LAUNCHER:
			pActor = GameLogic::Get()->CreateActor(
				"actors/quake/models/weapon/rocketlauncher.xml", nullptr, &initTransform);
			break;
		case WP_LIGHTNING:
			pActor = GameLogic::Get()->CreateActor(
				"actors/quake/models/weapon/lightning.xml", nullptr, &initTransform);
			break;
		case WP_RAILGUN:
			pActor = GameLogic::Get()->CreateActor(
				"actors/quake/models/weapon/railgun.xml", nullptr, &initTransform);
			break;
		case WP_PLASMAGUN:
			pActor = GameLogic::Get()->CreateActor(
				"actors/quake/models/weapon/plasmagun.xml", nullptr, &initTransform);
			break;
		default:
			break;
	}

	LogError("Couldn't find item for weapon " + eastl::to_string(weapon));
	return pActor;
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
	if (player->GetState().moveType == PM_DEAD)
		return;

	player->GetState().moveType = PM_DEAD;
	player->GetState().viewHeight = DEAD_VIEWHEIGHT;

	player->GetState().persistant[PERS_KILLED]++;

	if (attacker)
	{
		attacker->GetState().lastKilled = player->GetId();

		if (attacker != player)
		{
			attacker->GetState().persistant[PERS_SCORE] += 1;

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

				// also play humiliation on target
				player->GetState().persistant[PERS_PLAYEREVENTS] ^= PLAYEREVENT_GAUNTLETREWARD;
			}
		}
		else
		{
			attacker->GetState().persistant[PERS_SCORE] -= 1;
		}
	}
	else
	{
		player->GetState().persistant[PERS_SCORE] -= 1;
	}

	// send updated scores to any clients that are following this one,
	// or they would get stale scoreboards
	player->GetState().takeDamage = true;	// can still be gibbed

	player->GetState().weapon = WP_NONE;
	player->GetState().contents = CONTENTS_CORPSE;
	LookAtKiller(inflictor, player, attacker);

	// remove powerups
	memset(player->GetState().powerups, 0, sizeof(player->GetState().powerups));

	// never gib in a nodrop
	int anim = BOTH_DEATH1;

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
	int save = (int)ceil(damage * ARMOR_PROTECTION);
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

	// play an apropriate pain sound
	player->GetState().damageEvent++;
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
============
*/

void Damage(int damage, int dflags, int mod,
	Vector3<float> dir, Vector3<float> point,
	const eastl::shared_ptr<PlayerActor>& target,
	const eastl::shared_ptr<Actor>& inflictor,
	const eastl::shared_ptr<PlayerActor>& attacker)
{
	if (!target->GetState().takeDamage)
		return;

	// reduce damage by the attacker's handicap value
	// unless they are rocket jumping
	if (attacker && attacker != target)
	{
		int max = attacker->GetState().stats[STAT_MAX_HEALTH];
		damage = damage * max / 100;
	}

	if (dir != Vector3<float>::Zero())
		dflags |= DAMAGE_NO_KNOCKBACK;
	else
		Normalize(dir);

	int knockback = damage;
	if (knockback > 200)
		knockback = 200;

	if (dflags & DAMAGE_NO_KNOCKBACK)
		knockback = 0;

	// figure momentum add, even if the damage won't be taken
	if (knockback && target)
	{
		Vector3<float>	kvel;
		float mass = 200;

		//kvel = dir * (g_knockback.value * (float)knockback / mass));
		//target->GetState().velocity += kvel;

		// set the timer so that the other client can't cancel
		// out the movement immediately
		if (!target->GetState().moveTime)
		{
			int t = knockback * 2;
			if (t < 50) t = 50;
			if (t > 200) t = 200;

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
			return;

		damage *= 0.5f;
	}

	// add to the attacker's hit counter (if the target isn't a general entity like a prox mine)
	if (attacker && target != attacker &&
		target->GetState().stats[STAT_HEALTH] > 0 &&
		target->GetState().eType != ET_MISSILE &&
		target->GetState().eType != ET_GENERAL)
	{
		attacker->GetState().persistant[PERS_HITS]++;
		attacker->GetState().persistant[PERS_ATTACKEE_ARMOR] =
			(target->GetState().stats[STAT_HEALTH] << 8) | (target->GetState().stats[STAT_ARMOR]);
	}

	// always give half damage if hurting self
	// calculated after knockback, so rocket jumping works
	if (target == attacker)
		damage *= 0.5f;

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
			target->GetState().persistant[PERS_ATTACKER] = attacker->GetId();
		else
			target->GetState().persistant[PERS_ATTACKER] = ENTITYNUM_WORLD;

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

bool LogAccuracyHit(
	const eastl::shared_ptr<PlayerActor>& target,
	const eastl::shared_ptr<PlayerActor>& attacker)
{
	if (!target->GetState().takeDamage)
		return false;

	if (target == attacker)
		return false;

	if (!target)
		return false;

	if (!attacker)
		return false;

	if (target->GetState().stats[STAT_HEALTH] <= 0)
		return false;

	return true;
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

bool QuakeLogic::RadiusDamage(float damage, float radius, int mod,
	Vector3<float> origin, const eastl::shared_ptr<PlayerActor>& attacker)
{
	bool hitClient = false;

	if (radius < 1)
		radius = 1;

	for (ActorMap::const_iterator it = mActors.begin(); it != mActors.end(); ++it)
	{
		eastl::shared_ptr<PlayerActor> playerActor =
			eastl::dynamic_shared_pointer_cast<PlayerActor>((*it).second);
		if (playerActor)
		{
			if (!playerActor->GetState().takeDamage)
				continue;

			eastl::shared_ptr<TransformComponent> pTransformComponent(
				playerActor->GetComponent<TransformComponent>(TransformComponent::Name).lock());
			if (pTransformComponent)
			{
				Vector3<float> location = pTransformComponent->GetTransform().GetTranslation();

				float dist = Length(origin - location);
				if (dist >= radius)
					continue;

				float points = damage * (1.f - dist / radius);
				if (CanDamage(playerActor, origin))
				{
					if (LogAccuracyHit(playerActor, attacker))
						hitClient = true;

					Vector3<float> dir = location - origin;
					// push the center of mass higher than the origin so players
					// get knocked into the air more
					dir[2] += 24;
					Damage((int)points, DAMAGE_RADIUS, mod, dir, origin, playerActor, NULL, attacker);
				}
			}
		}
	}

	return hitClient;
}

void QuakeLogic::SplashDamageDelegate(BaseEventDataPtr pEventData)
{
	eastl::shared_ptr<QuakeEventDataSplashDamage> pCastEventData =
		eastl::static_pointer_cast<QuakeEventDataSplashDamage>(pEventData);

	eastl::shared_ptr<Actor> pGameActor(
		GameLogic::Get()->GetActor(pCastEventData->GetId()).lock());
	if (pGameActor && pGameActor->GetType() == "Fire")
	{
		if (pGameActor->GetComponent<GrenadeFire>(GrenadeFire::Name).lock())
		{
			eastl::shared_ptr<GrenadeFire> pGrenadeFire =
				pGameActor->GetComponent<GrenadeFire>(GrenadeFire::Name).lock();
			RadiusDamage(100, 150, MOD_GRENADE, pCastEventData->GetOrigin(), 
				eastl::dynamic_shared_pointer_cast<PlayerActor>(pGrenadeFire->mAttacker));

			eastl::shared_ptr<EventDataRequestDestroyActor>
				pRequestDestroyActorEvent(new EventDataRequestDestroyActor(pGameActor->GetId()));
			EventManager::Get()->QueueEvent(pRequestDestroyActorEvent);
		}
		else if (pGameActor->GetComponent<RocketFire>(RocketFire::Name).lock())
		{
			eastl::shared_ptr<RocketFire> pRocketFire =
				pGameActor->GetComponent<RocketFire>(RocketFire::Name).lock();
			RadiusDamage(100, 120, MOD_ROCKET, pCastEventData->GetOrigin(),
				eastl::dynamic_shared_pointer_cast<PlayerActor>(pRocketFire->mAttacker));

			eastl::shared_ptr<EventDataRequestDestroyActor>
				pRequestDestroyActorEvent(new EventDataRequestDestroyActor(pGameActor->GetId()));
			EventManager::Get()->QueueEvent(pRequestDestroyActorEvent);
		}
		else if (pGameActor->GetComponent<PlasmaFire>(PlasmaFire::Name).lock())
		{
			eastl::shared_ptr<PlasmaFire> pPlasmaFire =
				pGameActor->GetComponent<PlasmaFire>(PlasmaFire::Name).lock();
			RadiusDamage(20, 60, MOD_PLASMA, pCastEventData->GetOrigin(),
				eastl::dynamic_shared_pointer_cast<PlayerActor>(pPlasmaFire->mAttacker));

			eastl::shared_ptr<EventDataRequestDestroyActor>
				pRequestDestroyActorEvent(new EventDataRequestDestroyActor(pGameActor->GetId()));
			EventManager::Get()->QueueEvent(pRequestDestroyActorEvent);
		}
	}
}

/*
======================================================================

GAUNTLET

======================================================================
*/

void QuakeLogic::GauntletAttack(
	const eastl::shared_ptr<PlayerActor>& player, 
	const Vector3<float>& muzzle, const Vector3<float>& forward)
{
	//set muzzle location relative to pivoting eye
	Vector3<float> end = muzzle + forward * 32.f;

	// play attack sound
	EventManager::Get()->TriggerEvent(
		eastl::make_shared<EventDataPlaySound>("audio/quake/sound/weapons/melee/fstrun.ogg"));

	eastl::vector<ActorId> collisionActors;
	eastl::vector<Vector3<float>> collisions, collisionNormals;
	mPhysics->CastRay(muzzle, end, collisionActors, collisions, collisionNormals);

	ActorId closestCollisionId = INVALID_ACTOR_ID;
	Vector3<float> closestCollision = NULL;
	for (unsigned int i = 0; i < collisionActors.size(); i++)
	{
		if (collisionActors[i] != player->GetId())
		{
			if (closestCollision != NULL)
			{
				if (Length(closestCollision - muzzle) > Length(collisions[i] - muzzle))
				{
					closestCollisionId = collisionActors[i];
					closestCollision = collisions[i];
				}
			}
			else
			{
				closestCollisionId = collisionActors[i];
				closestCollision = collisions[i];
			}
		}
	}

	if (closestCollisionId != INVALID_ACTOR_ID &&
		eastl::dynamic_shared_pointer_cast<PlayerActor>(mActors[closestCollisionId]))
	{
		eastl::shared_ptr<PlayerActor> target =
			eastl::dynamic_shared_pointer_cast<PlayerActor>(mActors[closestCollisionId]);
		if (LogAccuracyHit(target, player))
			player->GetState().accuracyHits++;

		//rotation = ((69069 * randSeed + 1) & 0x7fff) % 360;
		Transform initTransform;
		initTransform.SetTranslation(closestCollision);
		CreateActor("actors/quake/effects/bleed.xml", nullptr, &initTransform);

		int damage = 50;
		Damage(damage, 0, MOD_GAUNTLET, forward, muzzle, target, player, player);
	}
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
	float r = ((Randomizer::Rand() & 0x7fff) / (float)0x7fff) * (float)GE_C_PI * 2.f;
	float u = sin(r) * (2.f * ((Randomizer::Rand() & 0x7fff) / (float)0x7fff) - 0.5f) * spread * 16.f;
	r = cos(r) * (2.f * ((Randomizer::Rand() & 0x7fff) / (float)0x7fff) - 0.5f) * spread * 16.f;
	Vector3<float> end = muzzle + forward * 8192.f * 16.f;
	end += right * r;
	end += up * u;

	// play firing sound
	EventManager::Get()->TriggerEvent(
		eastl::make_shared<EventDataPlaySound>("audio/quake/sound/weapons/machinegun/ric1.ogg"));

	eastl::vector<ActorId> collisionActors;
	eastl::vector<Vector3<float>> collisions, collisionNormals;
	mPhysics->CastRay(muzzle, end, collisionActors, collisions, collisionNormals);

	ActorId closestCollisionId = INVALID_ACTOR_ID;
	Vector3<float> closestCollision = NULL;
	for (unsigned int i = 0; i < collisionActors.size(); i++)
	{
		if (collisionActors[i] != player->GetId())
		{
			if (closestCollision != NULL)
			{
				if (Length(closestCollision - muzzle) > Length(collisions[i] - muzzle))
				{
					closestCollisionId = collisionActors[i];
					closestCollision = collisions[i];
				}
			}
			else
			{
				closestCollisionId = collisionActors[i];
				closestCollision = collisions[i];
			}
		}
	}

	if (closestCollisionId != INVALID_ACTOR_ID &&
		eastl::dynamic_shared_pointer_cast<PlayerActor>(mActors[closestCollisionId]))
	{
		eastl::shared_ptr<PlayerActor> target =
			eastl::dynamic_shared_pointer_cast<PlayerActor>(mActors[closestCollisionId]);
		if (LogAccuracyHit(target, player))
			player->GetState().accuracyHits++;

		//rotation = ((69069 * randSeed + 1) & 0x7fff) % 360;
		Transform initTransform;
		initTransform.SetTranslation(closestCollision);
		CreateActor("actors/quake/effects/bleed.xml", nullptr, &initTransform);

		Damage(damage, 0, MOD_MACHINEGUN, forward, closestCollision, target, player, player);
	}
	else
	{
		Transform initTransform;
		initTransform.SetTranslation(closestCollision);
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
	eastl::vector<ActorId> collisionActors;
	eastl::vector<Vector3<float>> collisions, collisionNormals;
	mPhysics->CastRay(start, end, collisionActors, collisions, collisionNormals);

	ActorId closestCollisionId = INVALID_ACTOR_ID;
	Vector3<float> closestCollision = NULL;
	for (unsigned int i = 0; i < collisionActors.size(); i++)
	{
		if (collisionActors[i] != player->GetId())
		{
			if (closestCollision != NULL)
			{
				if (Length(closestCollision - start) > Length(collisions[i] - start))
				{
					closestCollisionId = collisionActors[i];
					closestCollision = collisions[i];
				}
			}
			else
			{
				closestCollisionId = collisionActors[i];
				closestCollision = collisions[i];
			}
		}
	}

	if (closestCollisionId != INVALID_ACTOR_ID &&
		eastl::dynamic_shared_pointer_cast<PlayerActor>(mActors[closestCollisionId]))
	{
		eastl::shared_ptr<PlayerActor> target =
			eastl::dynamic_shared_pointer_cast<PlayerActor>(mActors[closestCollisionId]);
		if (LogAccuracyHit(target, player))
			player->GetState().accuracyHits++;

		//rotation = ((69069 * randSeed + 1) & 0x7fff) % 360;
		Transform initTransform;
		initTransform.SetTranslation(closestCollision);
		CreateActor("actors/quake/effects/bleed.xml", nullptr, &initTransform);

		int damage = DEFAULT_SHOTGUN_DAMAGE;
		Damage(damage, 0, MOD_SHOTGUN, forward, closestCollision, target, player, player);
		return true;
	}
	else
	{
		Transform initTransform;
		initTransform.SetTranslation(closestCollision);
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
		float r = (2.f * ((Randomizer::Rand() & 0x7fff) / (float)0x7fff) - 0.5f) * DEFAULT_SHOTGUN_SPREAD * 16.f;
		float u = (2.f * ((Randomizer::Rand() & 0x7fff) / (float)0x7fff) - 0.5f) * DEFAULT_SHOTGUN_SPREAD * 16.f;
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
	const Vector3<float>& forward, const EulerAngles<float>& viewAngles)
{
	Matrix4x4<float> yawRotation = Rotation<4, float>(
		AxisAngle<4, float>(Vector4<float>::Unit(2), viewAngles.mAngle[2]));
	Matrix4x4<float> pitchRotation = Rotation<4, float>(
		AxisAngle<4, float>(Vector4<float>::Unit(1), viewAngles.mAngle[1] + (float)GE_C_QUARTER_PI));

	Transform initTransform;
	initTransform.SetRotation(yawRotation * pitchRotation);
	initTransform.SetTranslation(muzzle);

	Vector3<float> end = muzzle + forward * 8192.f * 16.f;
	Vector3<float> direction = end - muzzle;
	Normalize(direction);

	eastl::shared_ptr<Actor> pGameActor = 
		CreateActor("actors/quake/effects/grenadelauncherfire.xml", nullptr, &initTransform);
	if (pGameActor)
	{
		eastl::shared_ptr<GrenadeFire> pGrenadeFire =
			pGameActor->GetComponent<GrenadeFire>(GrenadeFire::Name).lock();
		if (pGrenadeFire)
			pGrenadeFire->mAttacker = player;

		eastl::shared_ptr<PhysicComponent> pPhysicalComponent =
			pGameActor->GetComponent<PhysicComponent>(PhysicComponent::Name).lock();
		if (pPhysicalComponent)
		{
			pPhysicalComponent->SetIgnoreCollision(player->GetId(), true);

			direction[PITCH] *= 800000.f;
			direction[ROLL] *= 800000.f;
			direction[YAW] *= 500000.f;
			pPhysicalComponent->ApplyForce(direction);
		}
	}

	// play firing sound
	EventManager::Get()->TriggerEvent(
		eastl::make_shared<EventDataPlaySound>("audio/quake/sound/weapons/grenade/grenlf1a.ogg"));
}

/*
======================================================================

ROCKET

======================================================================
*/

void QuakeLogic::RocketLauncherFire(
	const eastl::shared_ptr<PlayerActor>& player, const Vector3<float>& muzzle, 
	const Vector3<float>& forward, const EulerAngles<float>& viewAngles)
{
	Matrix4x4<float> yawRotation = Rotation<4, float>(
		AxisAngle<4, float>(Vector4<float>::Unit(2), viewAngles.mAngle[2]));
	Matrix4x4<float> pitchRotation = Rotation<4, float>(
		AxisAngle<4, float>(Vector4<float>::Unit(1), viewAngles.mAngle[1]));

	Transform initTransform;
	initTransform.SetRotation(yawRotation * pitchRotation);
	initTransform.SetTranslation(muzzle);

	Vector3<float> end = muzzle + forward * 8192.f * 16.f;
	Vector3<float> direction = end - muzzle;
	Normalize(direction);

	eastl::shared_ptr<Actor> pGameActor =
		CreateActor("actors/quake/effects/rocketlauncherfire.xml", nullptr, &initTransform);
	if (pGameActor)
	{
		eastl::shared_ptr<RocketFire> pRocketFire =
			pGameActor->GetComponent<RocketFire>(RocketFire::Name).lock();
		if (pRocketFire)
			pRocketFire->mAttacker = player;

		eastl::shared_ptr<PhysicComponent> pPhysicalComponent =
			pGameActor->GetComponent<PhysicComponent>(PhysicComponent::Name).lock();
		if (pPhysicalComponent)
		{
			pPhysicalComponent->SetGravity(Vector3<float>::Zero());
			pPhysicalComponent->SetIgnoreCollision(player->GetId(), true);

			direction[PITCH] *= 1000000.f;
			direction[ROLL] *= 1000000.f;
			direction[YAW] *= 1000000.f;
			pPhysicalComponent->ApplyForce(direction);
		}
	}

	// play firing sound
	EventManager::Get()->TriggerEvent(
		eastl::make_shared<EventDataPlaySound>("audio/quake/sound/weapons/rocket/rocklf1a.ogg"));
}


/*
======================================================================

PLASMA GUN

======================================================================
*/

void QuakeLogic::PlasmagunFire(
	const eastl::shared_ptr<PlayerActor>& player, const Vector3<float>& muzzle, 
	const Vector3<float>& forward, const EulerAngles<float>& viewAngles)
{
	Matrix4x4<float> yawRotation = Rotation<4, float>(
		AxisAngle<4, float>(Vector4<float>::Unit(2), viewAngles.mAngle[2]));
	Matrix4x4<float> pitchRotation = Rotation<4, float>(
		AxisAngle<4, float>(Vector4<float>::Unit(1), viewAngles.mAngle[1]));

	Transform initTransform;
	initTransform.SetRotation(yawRotation * pitchRotation);
	initTransform.SetTranslation(muzzle);

	Vector3<float> end = muzzle + forward * 8192.f * 16.f;
	Vector3<float> direction = end - muzzle;
	Normalize(direction);

	eastl::shared_ptr<Actor> pGameActor =
		CreateActor("actors/quake/effects/plasmagunfire.xml", nullptr, &initTransform);
	if (pGameActor)
	{
		eastl::shared_ptr<PlasmaFire> pPlasmaFire =
			pGameActor->GetComponent<PlasmaFire>(PlasmaFire::Name).lock();
		if (pPlasmaFire)
			pPlasmaFire->mAttacker = player;

		eastl::shared_ptr<PhysicComponent> pPhysicalComponent =
			pGameActor->GetComponent<PhysicComponent>(PhysicComponent::Name).lock();
		if (pPhysicalComponent)
		{
			pPhysicalComponent->SetGravity(Vector3<float>::Zero());
			pPhysicalComponent->SetIgnoreCollision(player->GetId(), true);

			direction[PITCH] *= 4000.f;
			direction[ROLL] *= 4000.f;
			direction[YAW] *= 4000.f;
			pPhysicalComponent->ApplyForce(direction);
		}
	}

	// play firing sound
	EventManager::Get()->TriggerEvent(
		eastl::make_shared<EventDataPlaySound>("audio/quake/sound/weapons/plasma/hyprbf1a.ogg"));
}

/*
======================================================================

RAILGUN

======================================================================
*/

void QuakeLogic::RailgunFire(const eastl::shared_ptr<PlayerActor>& player,
	const Vector3<float>& muzzle, const Vector3<float>& forward)
{
	Vector3<float> end = muzzle + forward * 8192.f * 16.f;

	// play firing sound
	EventManager::Get()->TriggerEvent(
		eastl::make_shared<EventDataPlaySound>("audio/quake/sound/weapons/railgun/railgf1a.ogg"));

	eastl::vector<ActorId> collisionActors;
	eastl::vector<Vector3<float>> collisions, collisionNormals;
	mPhysics->CastRay(muzzle, end, collisionActors, collisions, collisionNormals);

	ActorId closestCollisionId = INVALID_ACTOR_ID;
	Vector3<float> closestCollision = NULL;
	for (unsigned int i = 0; i < collisionActors.size(); i++)
	{
		if (collisionActors[i] != player->GetId())
		{
			if (closestCollision != NULL)
			{
				if (Length(closestCollision - muzzle) > Length(collisions[i] - muzzle))
				{
					closestCollisionId = collisionActors[i];
					closestCollision = collisions[i];
				}
			}
			else
			{
				closestCollisionId = collisionActors[i];
				closestCollision = collisions[i];
			}
		}
	}

	if (closestCollision != NULL)
	{
		Vector3<float> direction = closestCollision - muzzle;
		float scale = Length(direction);
		Normalize(direction);

		Matrix4x4<float> yawRotation = Rotation<4, float>(
			AxisAngle<4, float>(Vector4<float>::Unit(2), atan2(direction[1], direction[0])));
		Matrix4x4<float> pitchRotation = Rotation<4, float>(
			AxisAngle<4, float>(Vector4<float>::Unit(1), -asin(direction[2])));

		Transform initTransform;
		initTransform.SetRotation(yawRotation * pitchRotation);
		initTransform.SetScale(Vector3<float>{scale, 4.f, 4.f});
		initTransform.SetTranslation(muzzle + (closestCollision - muzzle) / 2.f);
		CreateActor("actors/quake/effects/railgunfire.xml", nullptr, &initTransform);

		if (closestCollisionId != INVALID_ACTOR_ID &&
			eastl::dynamic_shared_pointer_cast<PlayerActor>(mActors[closestCollisionId]))
		{
			eastl::shared_ptr<PlayerActor> target =
				eastl::dynamic_shared_pointer_cast<PlayerActor>(mActors[closestCollisionId]);
			if (LogAccuracyHit(target, player))
				player->GetState().accuracyHits++;

			initTransform.MakeIdentity();
			initTransform.SetTranslation(closestCollision);
			CreateActor("actors/quake/effects/bleed.xml", nullptr, &initTransform);

			int damage = 100;
			Damage(damage, 0, MOD_RAILGUN, forward, closestCollision, target, player, player);
		}
	}
}


/*
======================================================================

LIGHTNING GUN

======================================================================
*/

void QuakeLogic::LightningFire(const eastl::shared_ptr<PlayerActor>& player,
	const Vector3<float>& muzzle, const Vector3<float>& forward)
{
	Vector3<float> end = muzzle + forward * (float)LIGHTNING_RANGE;

	// play firing sound
	EventManager::Get()->TriggerEvent(
		eastl::make_shared<EventDataPlaySound>("audio/quake/sound/weapons/lightning/lg_hum.ogg"));

	eastl::vector<ActorId> collisionActors;
	eastl::vector<Vector3<float>> collisions, collisionNormals;
	mPhysics->CastRay(muzzle, end, collisionActors, collisions, collisionNormals);

	ActorId closestCollisionId = INVALID_ACTOR_ID;
	Vector3<float> closestCollision = end;
	for (unsigned int i = 0; i < collisionActors.size(); i++)
	{
		if (collisionActors[i] != player->GetId())
		{
			if (closestCollision != NULL)
			{
				if (Length(closestCollision - muzzle) > Length(collisions[i] - muzzle))
				{
					closestCollisionId = collisionActors[i];
					closestCollision = collisions[i];
				}
			}
			else
			{
				closestCollisionId = collisionActors[i];
				closestCollision = collisions[i];
			}
		}
	}

	if (closestCollision != NULL)
	{
		Vector3<float> direction = closestCollision - muzzle;
		float scale = Length(direction);
		Normalize(direction);

		Matrix4x4<float> yawRotation = Rotation<4, float>(
			AxisAngle<4, float>(Vector4<float>::Unit(2), atan2(direction[1], direction[0])));
		Matrix4x4<float> pitchRotation = Rotation<4, float>(
			AxisAngle<4, float>(Vector4<float>::Unit(1), -asin(direction[2])));

		Transform initTransform;
		initTransform.SetRotation(yawRotation * pitchRotation);
		initTransform.SetScale(Vector3<float>{scale, 4.f, 4.f});
		initTransform.SetTranslation(muzzle + (closestCollision - muzzle) / 2.f);
		CreateActor("actors/quake/effects/lightningfire.xml", nullptr, &initTransform);

		if (closestCollisionId != INVALID_ACTOR_ID &&
			eastl::dynamic_shared_pointer_cast<PlayerActor>(mActors[closestCollisionId]))
		{
			eastl::shared_ptr<PlayerActor> target =
				eastl::dynamic_shared_pointer_cast<PlayerActor>(mActors[closestCollisionId]);
			if (LogAccuracyHit(target, player))
				player->GetState().accuracyHits++;

			initTransform.MakeIdentity();
			initTransform.SetTranslation(closestCollision);
			CreateActor("actors/quake/effects/bleed.xml", nullptr, &initTransform);

			int damage = 6;
			Damage(damage, 0, MOD_LIGHTNING, forward, closestCollision, target, player, player);
		}
	}
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
	muzzle += up * (float)pPlayerActor->GetState().viewHeight;
	muzzle += forward * 10.f;
	muzzle -= right * 11.f;

	// fire the specific weapon
	switch (pPlayerActor->GetState().weapon)
	{
		case WP_GAUNTLET:
			GauntletAttack(pPlayerActor, muzzle, forward);
			break;
		case WP_SHOTGUN:
			ShotgunFire(pPlayerActor, muzzle, forward, right, up);
			break;
		case WP_MACHINEGUN:
			BulletFire(pPlayerActor, muzzle, forward, right, up, MACHINEGUN_SPREAD, MACHINEGUN_DAMAGE);
			break;
		case WP_GRENADE_LAUNCHER:
			GrenadeLauncherFire(pPlayerActor, muzzle, forward, viewAngles);
			break;
		case WP_ROCKET_LAUNCHER:
			RocketLauncherFire(pPlayerActor, muzzle, forward, viewAngles);
			break;
		case WP_PLASMAGUN:
			PlasmagunFire(pPlayerActor, muzzle, forward, viewAngles);
			break;
		case WP_RAILGUN:
			RailgunFire(pPlayerActor, muzzle, forward);
			break;
		case WP_LIGHTNING:
			LightningFire(pPlayerActor, muzzle, forward);
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

void QuakeLogic::SelectNearestSpawnPoint(const Vector3<float>& from, eastl::shared_ptr<Actor>& nearestSpot)
{
	float nearestDist = 999999;
	eastl::shared_ptr<Actor> spot = NULL;
	for (ActorMap::const_iterator it = mActors.begin(); it != mActors.end(); ++it)
	{
		spot = (*it).second;
		if (spot->GetComponent<LocationTarget>(LocationTarget::Name).lock())
		{
			eastl::shared_ptr<TransformComponent> pTransformComponent(
				spot->GetComponent<TransformComponent>(TransformComponent::Name).lock());
			if (pTransformComponent)
			{
				Vector3<float> delta = pTransformComponent->GetPosition() - from;
				float dist = Length(delta);
				if (dist < nearestDist)
				{
					nearestDist = dist;
					nearestSpot = spot;
				}
			}
		}
	}
}

#define	MAX_SPAWN_POINTS	128
void QuakeLogic::SelectRandomSpawnPoint(eastl::shared_ptr<Actor>& spot)
{
	Transform transform;
	eastl::shared_ptr<Actor> spots[MAX_SPAWN_POINTS];

	int count = 0;
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

	if (count)
	{
		int selection = Randomizer::Rand() % count;
		spot = spots[selection];
	}
}

void QuakeLogic::SelectRandomFurthestSpawnPoint(const Vector3<float>& avoidPoint, Transform& transform)
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

		SelectSpawnPoint(Vector3<float>::Zero(), transform);
	}
	else
	{
		// select a random spot from the spawn points furthest away
		int rnd = (int)(((Randomizer::Rand() & 0x7fff) / (float)0x7fff) * (numSpots / 2));

		eastl::shared_ptr<TransformComponent> pTransformComponent(
			spots[rnd]->GetComponent<TransformComponent>(TransformComponent::Name).lock());
		if (pTransformComponent)
		{
			transform.SetTranslation(pTransformComponent->GetTransform().GetTranslation());
			transform.SetRotation(pTransformComponent->GetTransform().GetRotation());
		}
	}
}

void QuakeLogic::SelectSpawnPoint(const Vector3<float>& avoidPoint, Transform& transform)
{
	return SelectRandomFurthestSpawnPoint(avoidPoint, transform);
}

void QuakeLogic::SelectInitialSpawnPoint(Transform& transform)
{
	eastl::shared_ptr<Actor> spot = NULL;
	for (ActorMap::const_iterator it = mActors.begin(); it != mActors.end(); ++it)
	{
		spot = (*it).second;
		if (spot->GetComponent<LocationTarget>(LocationTarget::Name).lock())
		{
			if (SpotTelefrag(spot))
			{
				SelectSpawnPoint(Vector3<float>::Zero(), transform);
				return;
			}
			break;
		}
		else spot = NULL;
	}

	if (spot)
	{
		eastl::shared_ptr<TransformComponent> pTransformComponent(
			spot->GetComponent<TransformComponent>(TransformComponent::Name).lock());
		if (pTransformComponent)
		{
			transform.SetTranslation(pTransformComponent->GetTransform().GetTranslation());
			transform.SetRotation(pTransformComponent->GetTransform().GetRotation());
		}
	}
	else SelectSpawnPoint(Vector3<float>::Zero(), transform);
}

int PickupAmmo(const eastl::shared_ptr<PlayerActor>& player, const eastl::shared_ptr<AmmoPickup>& ammo)
{
	player->GetState().ammo[ammo->GetCode()] += ammo->GetAmount();
	if (player->GetState().ammo[ammo->GetCode()] > 200)
		player->GetState().ammo[ammo->GetCode()] = 200;

	EventManager::Get()->TriggerEvent(
		eastl::make_shared<EventDataPlaySound>("audio/quake/sound/misc/am_pkup.wav"));

	return ammo->GetWait();
}

int PickupWeapon(const eastl::shared_ptr<PlayerActor>& player, const eastl::shared_ptr<WeaponPickup>& weapon)
{
	// add the weapon
	player->GetState().stats[STAT_WEAPONS] |= (1 << weapon->GetCode());

	// add ammo
	player->GetState().ammo[weapon->GetCode()] += weapon->GetAmmo();
	if (player->GetState().ammo[weapon->GetCode()] > 200)
		player->GetState().ammo[weapon->GetCode()] = 200;

	// play weapon pickup sound
	EventManager::Get()->TriggerEvent(
		eastl::make_shared<EventDataPlaySound>("audio/quake/sound/misc/w_pkup.wav"));

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

	// play health pickup sound
	if (health->GetCode() == 1)
	{
		EventManager::Get()->TriggerEvent(
			eastl::make_shared<EventDataPlaySound>("audio/quake/sound/items/n_health.wav"));
	}
	else if (health->GetCode() == 2)
	{
		EventManager::Get()->TriggerEvent(
			eastl::make_shared<EventDataPlaySound>("audio/quake/sound/items/l_health.wav"));
	}
	else if (health->GetCode() == 3)
	{
		EventManager::Get()->TriggerEvent(
			eastl::make_shared<EventDataPlaySound>("audio/quake/sound/items/m_health.wav"));
	}
	else if (health->GetCode() == 4)
	{
		EventManager::Get()->TriggerEvent(
			eastl::make_shared<EventDataPlaySound>("audio/quake/sound/items/s_health.wav"));
	}

	return health->GetWait();
}

int PickupArmor(const eastl::shared_ptr<PlayerActor>& player, const eastl::shared_ptr<ArmorPickup>& armor)
{
	player->GetState().stats[STAT_ARMOR] += armor->GetAmount();
	if (player->GetState().stats[STAT_ARMOR] > player->GetState().stats[STAT_MAX_HEALTH] * 2)
		player->GetState().stats[STAT_ARMOR] = player->GetState().stats[STAT_MAX_HEALTH] * 2;

	// play armor pickup sound
	if (armor->GetCode() == 1)
	{
		EventManager::Get()->TriggerEvent(
			eastl::make_shared<EventDataPlaySound>("audio/quake/sound/misc/ar2_pkup.wav"));
	}
	else if (armor->GetCode() == 2)
	{
		EventManager::Get()->TriggerEvent(
			eastl::make_shared<EventDataPlaySound>("audio/quake/sound/misc/ar2_pkup.wav"));
	}
	else if (armor->GetCode() == 3)
	{
		EventManager::Get()->TriggerEvent(
			eastl::make_shared<EventDataPlaySound>("audio/quake/sound/misc/ar1_pkup.wav"));
	}

	return armor->GetWait();
}

/*
CanItemBeGrabbed
Returns false if the item should not be picked up.
*/
bool QuakeLogic::CanItemBeGrabbed(const eastl::shared_ptr<Actor>& item, const eastl::shared_ptr<PlayerActor>& player)
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

		if (pItemActor->GetType() == "Weapon")
		{
			eastl::shared_ptr<WeaponPickup> pWeaponPickup =
				pItemActor->GetComponent<WeaponPickup>(WeaponPickup::Name).lock();
			if (pWeaponPickup->mRespawnTime)
				return;

			pWeaponPickup->mRespawnTime = (float)PickupWeapon(pPlayerActor, pWeaponPickup);
		}
		else if (pItemActor->GetType() == "Ammo")
		{
			eastl::shared_ptr<AmmoPickup> pAmmoPickup =
				pItemActor->GetComponent<AmmoPickup>(AmmoPickup::Name).lock();
			if (pAmmoPickup->mRespawnTime)
				return;

			pAmmoPickup->mRespawnTime = (float)PickupAmmo(pPlayerActor, pAmmoPickup);
		}
		else if (pItemActor->GetType() == "Armor")
		{
			eastl::shared_ptr<ArmorPickup> pArmorPickup =
				pItemActor->GetComponent<ArmorPickup>(ArmorPickup::Name).lock();
			if (pArmorPickup->mRespawnTime)
				return;

			pArmorPickup->mRespawnTime = (float)PickupArmor(pPlayerActor, pArmorPickup);
		}
		else if (pItemActor->GetType() == "Health")
		{
			eastl::shared_ptr<HealthPickup> pHealthPickup =
				pItemActor->GetComponent<HealthPickup>(HealthPickup::Name).lock();
			if (pHealthPickup->mRespawnTime)
				return;

			pHealthPickup->mRespawnTime = (float)PickupHealth(pPlayerActor, pHealthPickup);
		}

		QuakeAIManager* aiManager =
			dynamic_cast<QuakeAIManager*>(GameLogic::Get()->GetAIManager());
		aiManager->DetectActor(pPlayerActor, pItemActor);
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
			if (pItemActor->GetComponent<PushTrigger>(PushTrigger::Name).lock())
			{
				pPlayerActor->GetAction().triggerPush = pItemActor->GetId();

				QuakeAIManager* aiManager =
					dynamic_cast<QuakeAIManager*>(GameLogic::Get()->GetAIManager());
				aiManager->DetectActor(pPlayerActor, pItemActor);
			}
			else if (pItemActor->GetComponent<TeleporterTrigger>(TeleporterTrigger::Name).lock())
			{
				pPlayerActor->GetAction().triggerTeleporter = pItemActor->GetId();

				QuakeAIManager* aiManager =
					dynamic_cast<QuakeAIManager*>(GameLogic::Get()->GetAIManager());
				aiManager->DetectActor(pPlayerActor, pItemActor);
			}
		}
		else if (pItemActor->GetType() == "Fire")
		{
			if (pItemActor->GetComponent<RocketFire>(RocketFire::Name).lock())
			{
				eastl::shared_ptr<RocketFire> pRocketFire =
					pItemActor->GetComponent<RocketFire>(RocketFire::Name).lock();
				pRocketFire->mExplosionTime = 1.f;
			}
			else if (pItemActor->GetComponent<PlasmaFire>(PlasmaFire::Name).lock())
			{
				eastl::shared_ptr<PlasmaFire> pPlasmaFire =
					pItemActor->GetComponent<PlasmaFire>(PlasmaFire::Name).lock();
				pPlasmaFire->mExplosionTime = 1.f;
			}
		}
	}
	else if (pGameActorA || pGameActorB)
	{
		eastl::shared_ptr<Actor> pItemActor;
		if (pGameActorA)
			pItemActor = eastl::dynamic_shared_pointer_cast<Actor>(pGameActorA);
		else
			pItemActor = eastl::dynamic_shared_pointer_cast<Actor>(pGameActorB);
		if (pItemActor->GetType() == "Fire")
		{
			if (pItemActor->GetComponent<RocketFire>(RocketFire::Name).lock())
			{
				eastl::shared_ptr<RocketFire> pRocketFire =
					pItemActor->GetComponent<RocketFire>(RocketFire::Name).lock();
				pRocketFire->mExplosionTime = 1.f;
			}
			else if (pItemActor->GetComponent<PlasmaFire>(PlasmaFire::Name).lock())
			{
				eastl::shared_ptr<PlasmaFire> pPlasmaFire =
					pItemActor->GetComponent<PlasmaFire>(PlasmaFire::Name).lock();
				pPlasmaFire->mExplosionTime = 1.f;
			}
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