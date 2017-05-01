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


void GameDemoLogic::UpdateViewType(const eastl::shared_ptr<BaseGameView>& pView)
{
	GameLogic::UpdateViewType(pView);

	//  This is commented out because while the view is created and waiting, the player has NOT attached yet. 
	/*
	if (pView->GetType() == GV_REMOTE)
	{
	mHumanPlayersAttached++;
	}
	*/
	if (pView->GetType() == GV_HUMAN)
	{
		mHumanPlayersAttached++;
	}
	else if (pView->GetType() == GV_AI)
	{
		mAIPlayersAttached++;
	}
}

void GameDemoLogic::SetProxy()
{
	// FUTURE WORK: This can go in the base game logic!!!!
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
		case BGS_WAITINGFORPLAYERS:
		{

			// spawn all local players (should only be one, though we might support more in the future)
			LogAssert(mExpectedPlayers == 1, "needs two players at least");
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
				eastl::shared_ptr<BaseGameView> remoteGameView(new NetworkGameView);
				gameApp->AddView(remoteGameView);
			}

			// spawn all AI's views on the game
			for (int i = 0; i < mExpectedAI; ++i)
			{
				eastl::shared_ptr<BaseGameView> aiView(new AITeapotView(eastl::shared_ptr<PathingGraph>()));
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
					eastl::shared_ptr<Actor> pActor = CreateActor("actors\\player_teapot.xml", NULL);
					if (pActor)
					{
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
					eastl::shared_ptr<Actor> pActor = CreateActor("actors\\remote_teapot.xml", NULL);
					if (pActor)
					{
						eastl::shared_ptr<EventDataNewActor> pNewActorEvent(
							new EventDataNewActor(pActor->GetId(), pNetworkGameView->GetId()));
						BaseEventManager::Get()->QueueEvent(pNewActorEvent);
					}
				}
				else if (pView->GetType() == GV_AI)
				{
					eastl::shared_ptr<AITeapotView> pAiView = 
						eastl::static_pointer_cast<AITeapotView, BaseGameView>(pView);
					eastl::shared_ptr<Actor> pActor = CreateActor("actors\\ai_teapot.xml", NULL);
					if (pActor)
					{
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

void GameDemoLogic::MoveActor(const ActorId id, Transform const &transform)
{
	GameLogic::MoveActor(id, transform);

	// [rez] HACK: This will be removed whenever the gameplay update stuff is in.  
	//	This is meant to model the death zone under the grid.

	// FUTURE WORK - This would make a great basis for a Trigger actor that ran a LUA script when other
	//               actors entered or left it!

	eastl::shared_ptr<Actor> pActor = eastl::shared_ptr<Actor>(GetActor(id));
	if (pActor)
	{
		eastl::shared_ptr<TransformComponent> pTransformComponent = 
			eastl::shared_ptr<TransformComponent>(
				pActor->GetComponent<TransformComponent>(TransformComponent::Name));
		if (pTransformComponent && pTransformComponent->GetPosition()[1] < -25)
		{
			eastl::shared_ptr<EventDataDestroyActor> pDestroyActorEvent(new EventDataDestroyActor(id));
			BaseEventManager::Get()->QueueEvent(pDestroyActorEvent);
		}
	}
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
			{
				pHumanView->SetControlledActor(pCastEventData->GetActorId());
			}
			return;
		}
	}

	LogError("Could not find HumanView to attach actor to!");
}

void GameDemoLogic::StartThrustDelegate(BaseEventDataPtr pEventData)
{
	eastl::shared_ptr<EventDataStartThrust> pCastEventData =
		eastl::static_pointer_cast<EventDataStartThrust>(pEventData);
	eastl::shared_ptr<Actor> pActor = eastl::shared_ptr<Actor>(
		GetActor(pCastEventData->GetActorId()));
	if (pActor)
	{
		eastl::shared_ptr<PhysicComponent> pPhysicalComponent = 
			eastl::shared_ptr<PhysicComponent>(
				pActor->GetComponent<PhysicComponent>(PhysicComponent::Name));
		if (pPhysicalComponent)
			pPhysicalComponent->ApplyAcceleration(pCastEventData->GetAcceleration());
	}
}

void GameDemoLogic::EndThrustDelegate(BaseEventDataPtr pEventData)
{
	eastl::shared_ptr<EventDataStartThrust> pCastEventData =
		eastl::static_pointer_cast<EventDataStartThrust>(pEventData);
	eastl::shared_ptr<Actor> pActor = eastl::shared_ptr<Actor>(
		GetActor(pCastEventData->GetActorId()));
	if (pActor)
	{
		eastl::shared_ptr<PhysicComponent> pPhysicalComponent =
			eastl::shared_ptr<PhysicComponent>(
				pActor->GetComponent<PhysicComponent>(PhysicComponent::Name));
		if (pPhysicalComponent)
			pPhysicalComponent->RemoveAcceleration();
	}
}

void GameDemoLogic::StartSteerDelegate(BaseEventDataPtr pEventData)
{
	eastl::shared_ptr<EventDataStartThrust> pCastEventData =
		eastl::static_pointer_cast<EventDataStartThrust>(pEventData);
	eastl::shared_ptr<Actor> pActor = eastl::shared_ptr<Actor>(
		GetActor(pCastEventData->GetActorId()));
	if (pActor)
	{
		eastl::shared_ptr<PhysicComponent> pPhysicalComponent = 
			eastl::shared_ptr<PhysicComponent>(
				pActor->GetComponent<PhysicComponent>(PhysicComponent::Name));
		if (pPhysicalComponent)
			pPhysicalComponent->ApplyAngularAcceleration(pCastEventData->GetAcceleration());
	}
}

void GameDemoLogic::EndSteerDelegate(BaseEventDataPtr pEventData)
{
	eastl::shared_ptr<EventDataStartThrust> pCastEventData =
		eastl::static_pointer_cast<EventDataStartThrust>(pEventData);
	eastl::shared_ptr<Actor> pActor = eastl::shared_ptr<Actor>(
		GetActor(pCastEventData->GetActorId()));
	if (pActor)
	{
		eastl::shared_ptr<PhysicComponent> pPhysicalComponent =
			eastl::shared_ptr<PhysicComponent>(
				pActor->GetComponent<PhysicComponent>(PhysicComponent::Name));
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
		MakeDelegate(this, &GameDemoLogic::MoveActorDelegate), 
		EventDataMoveActor::skEventType);
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

	// FUTURE WORK: Only these belong in Demos.
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
		MakeDelegate(this, &GameDemoLogic::MoveActorDelegate), 
		EventDataMoveActor::skEventType);
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

	// FUTURE WORK: These belong in teapot wars!
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
		EventDataPhysCollision::skEventType);
	pGlobalEventManager->AddListener(
		MakeDelegate(pNetworkEventForwarder, &NetworkEventForwarder::ForwardEvent), 
		EventDataDestroyActor::skEventType);
	pGlobalEventManager->AddListener(
		MakeDelegate(pNetworkEventForwarder, &NetworkEventForwarder::ForwardEvent), 
		EventDataFireWeapon::skEventType);
	pGlobalEventManager->AddListener(
		MakeDelegate(pNetworkEventForwarder, &NetworkEventForwarder::ForwardEvent), 
		EventDataEnvironmentLoaded::skEventType);
	pGlobalEventManager->AddListener(
		MakeDelegate(pNetworkEventForwarder, &NetworkEventForwarder::ForwardEvent), 
		EventDataNewActor::skEventType);
	pGlobalEventManager->AddListener(
		MakeDelegate(pNetworkEventForwarder, &NetworkEventForwarder::ForwardEvent), 
		EventDataMoveActor::skEventType);
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
		NetworkEventForwarder* pNetworkEventForwarder = (*it);

		BaseEventManager* pGlobalEventManager = BaseEventManager::Get();
		pGlobalEventManager->RemoveListener(
			MakeDelegate(pNetworkEventForwarder, &NetworkEventForwarder::ForwardEvent), 
			EventDataPhysCollision::skEventType);
		pGlobalEventManager->RemoveListener(
			MakeDelegate(pNetworkEventForwarder, &NetworkEventForwarder::ForwardEvent), 
			EventDataDestroyActor::skEventType);
		pGlobalEventManager->RemoveListener(
			MakeDelegate(pNetworkEventForwarder, &NetworkEventForwarder::ForwardEvent), 
			EventDataFireWeapon::skEventType);
		pGlobalEventManager->RemoveListener(
			MakeDelegate(pNetworkEventForwarder, &NetworkEventForwarder::ForwardEvent), 
			EventDataEnvironmentLoaded::skEventType);
		pGlobalEventManager->RemoveListener(
			MakeDelegate(pNetworkEventForwarder, &NetworkEventForwarder::ForwardEvent), 
			EventDataNewActor::skEventType);
		pGlobalEventManager->RemoveListener(
			MakeDelegate(pNetworkEventForwarder, &NetworkEventForwarder::ForwardEvent), 
			EventDataMoveActor::skEventType);
		pGlobalEventManager->RemoveListener(
			MakeDelegate(pNetworkEventForwarder, &NetworkEventForwarder::ForwardEvent), 
			EventDataRequestNewActor::skEventType);
		pGlobalEventManager->RemoveListener(
			MakeDelegate(pNetworkEventForwarder, &NetworkEventForwarder::ForwardEvent), 
			EventDataNetworkPlayerActorAssignment::skEventType);

		delete pNetworkEventForwarder;
	}

	mNetworkEventForwarders.clear();
}


bool GameDemoLogic::LoadGameDelegate(XMLElement* pLevelData)
{
	return true;
}