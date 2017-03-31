//========================================================================
//
// GameDemoLogic Implementation       - Chapter 21, page 725
//
//========================================================================

#include "GameDemoView.h"

#include "GameDemo.h"

//
// GameDemoLogic::GameDemoLogic
//
GameDemoLogic::GameDemoLogic()
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


void GameDemoLogic::SetProxy()
{
	// FUTURE WORK: This can go in the base game logic!!!!
	BaseGameLogic::SetProxy();
}


//
// GameDemoLogic::ChangeState
//
void GameDemoLogic::ChangeState(BaseGameState newState)
{
	BaseGameLogic::ChangeState(newState);

	switch (newState)
	{
	case BGS_WAITINGFORPLAYERS:
	{

		// spawn all local players (should only be one, though we might support more in the future)
		LogAssert(mExpectedPlayers == 1, "needs two players at least");
		for (int i = 0; i < mExpectedPlayers; ++i)
		{
			eastl::shared_ptr<BaseGameView> playersView(
				new GameDemoHumanView(gameApp->mRenderer));
			gameApp->AddView(playersView);

			if (mProxy)
			{
				// if we are a remote player, all we have to do is spawn our view - the server will do the rest.
				return;
			}
		}
		// spawn all remote player's views on the game
		for (int i = 0; i < mExpectedRemotePlayers; ++i)
		{
			eastl::shared_ptr<BaseGameView> remoteGameView(new NetworkGameView);
			AddView(remoteGameView);
		}
		/*
		// spawn all AI's views on the game
		for (int i = 0; i < m_ExpectedAI; ++i)
		{
		shared_ptr<IGameView> aiView(new AITeapotView(shared_ptr<PathingGraph>()));
		AddView(aiView);
		}
		*/
		break;
	}


	case BGS_SpawningPlayersActors:
	{
		if (m_bProxy)
		{
			// only the server needs to do this.
			return;
		}

		const GameViewList& gameViews = g_DemosApp.GetGameViews();
		for (auto it = gameViews.begin(); it != gameViews.end(); ++it)
		{
			shared_ptr<IGameView> pView = *it;
			if (pView->GetType() == GameView_Human)
			{
				StrongActorPtr pActor = CreateActor("actors\\player_teapot.xml", NULL);
				if (pActor)
				{
					shared_ptr<EvtData_New_Actor> pNewActorEvent(
						new EvtData_New_Actor(pActor->GetId(), pView->GetId()));

					// [rez] This needs to happen asap because the constructor function for Lua 
					// (which is called in through VCreateActor()) queues an event that expects 
					// this event to have been handled
					IEventManager::Get()->TriggerEvent(pNewActorEvent);
				}
			}
			else if (pView->GetType() == GameView_Remote)
			{
				shared_ptr<NetworkGameView> pNetworkGameView =
					static_pointer_cast<NetworkGameView, IGameView>(pView);
				StrongActorPtr pActor = CreateActor("actors\\remote_teapot.xml", NULL);
				if (pActor)
				{
					shared_ptr<EvtData_New_Actor> pNewActorEvent(
						new EvtData_New_Actor(pActor->GetId(), pNetworkGameView->GetId()));
					IEventManager::Get()->QueueEvent(pNewActorEvent);
				}
			}
			/*
			else if (pView->GetType() == GameView_AI)
			{
			shared_ptr<AITeapotView> pAiView = static_pointer_cast<AITeapotView, IGameView>(pView);
			StrongActorPtr pActor = CreateActor("actors\\ai_teapot.xml", NULL);
			if (pActor)
			{
			shared_ptr<EvtData_New_Actor> pNewActorEvent(
			new EvtData_New_Actor(pActor->GetId(), pAiView->GetId()));
			IEventManager::Get()->QueueEvent(pNewActorEvent);
			}
			}
			*/
		}

		break;
	}
	}
}

//
// GameDemoLogic::AddView
//
void GameDemoLogic::AddView(shared_ptr<IGameView> pView, ActorId actor)
{
	g_DemosApp.AddView(pView, actor);
	//  This is commented out because while the view is created and waiting, the player has NOT attached yet. 
	//	if (pView->GetType() == GameView_Remote)
	//	{
	//		m_HumanPlayersAttached++;
	//	}
	if (pView->GetType() == GameView_Human)
	{
		m_HumanPlayersAttached++;
	}
	else if (pView->GetType() == GameView_AI)
	{
		m_AIPlayersAttached++;
	}
}

void GameDemoLogic::MoveActor(const ActorId id, matrix4 const &mat)
{
	BaseGameLogic::MoveActor(id, mat);

	// [rez] HACK: This will be removed whenever the gameplay update stuff is in.  This is meant to model the death
	// zone under the grid.

	// FUTURE WORK - This would make a great basis for a Trigger actor that ran a LUA script when other
	//               actors entered or left it!

	StrongActorPtr pActor = MakeStrongPtr(GetActor(id));
	if (pActor)
	{
		shared_ptr<TransformComponent> pTransformComponent =
			MakeStrongPtr(pActor->GetComponent<TransformComponent>(TransformComponent::g_Name));
		if (pTransformComponent && pTransformComponent->GetPosition().Y < -25)
		{
			shared_ptr<EvtData_Destroy_Actor> pDestroyActorEvent(new EvtData_Destroy_Actor(id));
			IEventManager::Get()->QueueEvent(pDestroyActorEvent);
		}
	}
}

void GameDemoLogic::RequestStartGameDelegate(BaseEventDataPtr pEventData)
{
	ChangeState(BGS_WaitingForPlayers);
}

void GameDemoLogic::EnvironmentLoadedDelegate(BaseEventDataPtr pEventData)
{
	++m_HumanGamesLoaded;
}


// FUTURE WORK - this isn't Demos specific so it can go into the game agnostic base class
void GameDemoLogic::RemoteClientDelegate(BaseEventDataPtr pEventData)
{
	// This event is always sent from clients to the game server.

	shared_ptr<EvtData_Remote_Client> pCastEventData = static_pointer_cast<EvtData_Remote_Client>(pEventData);
	const int sockID = pCastEventData->GetSocketId();
	const int ipAddress = pCastEventData->GetIpAddress();

	// go find a NetworkGameView that doesn't have a socket ID, and attach this client to that view.
	const GameViewList& gameViews = g_DemosApp.GetGameViews();
	for (auto it = gameViews.begin(); it != gameViews.end(); ++it)
	{
		shared_ptr<IGameView> pView = *it;
		if (pView->GetType() == GameView_Remote)
		{
			shared_ptr<NetworkGameView> pNetworkGameView = static_pointer_cast<NetworkGameView, IGameView>(pView);
			if (!pNetworkGameView->HasRemotePlayerAttached())
			{
				pNetworkGameView->AttachRemotePlayer(sockID);
				CreateNetworkEventForwarder(sockID);
				m_HumanPlayersAttached++;

				return;
			}
		}
	}

}

void GameDemoLogic::NetworkPlayerActorAssignmentDelegate(BaseEventDataPtr pEventData)
{
	if (!m_bProxy)
		return;

	// we're a remote client getting an actor assignment.
	// the server assigned us a playerId when we first attached (the server's socketId, actually)
	shared_ptr<EvtData_Network_Player_Actor_Assignment> pCastEventData =
		static_pointer_cast<EvtData_Network_Player_Actor_Assignment>(pEventData);

	if (pCastEventData->GetActorId() == INVALID_ACTOR_ID)
	{
		m_remotePlayerId = pCastEventData->GetSocketId();
		return;
	}

	const GameViewList& gameViews = g_DemosApp.GetGameViews();
	for (auto it = gameViews.begin(); it != gameViews.end(); ++it)
	{
		shared_ptr<IGameView> pView = *it;
		if (pView->GetType() == GameView_Human)
		{
			shared_ptr<DemosHumanView> pHumanView =
				static_pointer_cast<DemosHumanView, IGameView>(pView);
			if (m_remotePlayerId == pCastEventData->GetSocketId())
			{
				pHumanView->SetControlledActor(pCastEventData->GetActorId());
			}
			return;
		}
	}

	GE_ERROR("Could not find HumanView to attach actor to!");
}

void GameDemoLogic::StartThrustDelegate(BaseEventDataPtr pEventData)
{
	shared_ptr<EvtData_StartThrust> pCastEventData =
		static_pointer_cast<EvtData_StartThrust>(pEventData);
	StrongActorPtr pActor = MakeStrongPtr(GetActor(pCastEventData->GetActorId()));
	if (pActor)
	{
		shared_ptr<PhysicsComponent> pPhysicalComponent =
			MakeStrongPtr(pActor->GetComponent<PhysicsComponent>(PhysicsComponent::g_Name));
		if (pPhysicalComponent)
		{
			pPhysicalComponent->ApplyAcceleration(pCastEventData->GetAcceleration());
		}
	}
}

void GameDemoLogic::EndThrustDelegate(BaseEventDataPtr pEventData)
{
	shared_ptr<EvtData_StartThrust> pCastEventData =
		static_pointer_cast<EvtData_StartThrust>(pEventData);
	StrongActorPtr pActor = MakeStrongPtr(GetActor(pCastEventData->GetActorId()));
	if (pActor)
	{
		shared_ptr<PhysicsComponent> pPhysicalComponent =
			MakeStrongPtr(pActor->GetComponent<PhysicsComponent>(PhysicsComponent::g_Name));
		if (pPhysicalComponent)
		{
			pPhysicalComponent->RemoveAcceleration();
		}
	}
}

void GameDemoLogic::StartSteerDelegate(BaseEventDataPtr pEventData)
{
	shared_ptr<EvtData_StartThrust> pCastEventData =
		static_pointer_cast<EvtData_StartThrust>(pEventData);
	StrongActorPtr pActor = MakeStrongPtr(GetActor(pCastEventData->GetActorId()));
	if (pActor)
	{
		shared_ptr<PhysicsComponent> pPhysicalComponent = MakeStrongPtr(
			pActor->GetComponent<PhysicsComponent>(PhysicsComponent::g_Name));
		if (pPhysicalComponent)
		{
			pPhysicalComponent->ApplyAngularAcceleration(pCastEventData->GetAcceleration());
		}
	}
}

void GameDemoLogic::EndSteerDelegate(BaseEventDataPtr pEventData)
{
	shared_ptr<EvtData_StartThrust> pCastEventData =
		static_pointer_cast<EvtData_StartThrust>(pEventData);
	StrongActorPtr pActor = MakeStrongPtr(GetActor(pCastEventData->GetActorId()));
	if (pActor)
	{
		shared_ptr<PhysicsComponent> pPhysicalComponent =
			MakeStrongPtr(pActor->GetComponent<PhysicsComponent>(PhysicsComponent::g_Name));
		if (pPhysicalComponent)
		{
			pPhysicalComponent->RemoveAngularAcceleration();
		}
	}
}

void GameDemoLogic::TestScriptDelegate(BaseEventDataPtr pEventData)
{
	shared_ptr<EvtData_ScriptEventTest_FromLua> pCastEventData =
		static_pointer_cast<EvtData_ScriptEventTest_FromLua>(pEventData);
	GE_LOG("Lua", eastl::string("Event received in C++ from Lua: ") + eastl::string(pCastEventData->GetNum()));
}

void GameDemoLogic::RegisterAllDelegates(void)
{
	// FUTURE WORK: Lots of these functions are ok to go into the base game logic!
	IEventManager* pGlobalEventManager = IEventManager::Get();
	pGlobalEventManager->AddListener(MakeDelegate(this, &DemosLogic::RemoteClientDelegate), EvtData_Remote_Client::sk_EventType);
	pGlobalEventManager->AddListener(MakeDelegate(this, &DemosLogic::MoveActorDelegate), EvtData_Move_Actor::sk_EventType);
	pGlobalEventManager->AddListener(MakeDelegate(this, &DemosLogic::RequestStartGameDelegate), EvtData_Request_Start_Game::sk_EventType);
	pGlobalEventManager->AddListener(MakeDelegate(this, &DemosLogic::NetworkPlayerActorAssignmentDelegate), EvtData_Network_Player_Actor_Assignment::sk_EventType);
	pGlobalEventManager->AddListener(MakeDelegate(this, &DemosLogic::EnvironmentLoadedDelegate), EvtData_Environment_Loaded::sk_EventType);
	pGlobalEventManager->AddListener(MakeDelegate(this, &DemosLogic::EnvironmentLoadedDelegate), EvtData_Remote_Environment_Loaded::sk_EventType);

	// FUTURE WORK: Only these belong in Demos.
	pGlobalEventManager->AddListener(MakeDelegate(this, &DemosLogic::StartThrustDelegate), EvtData_StartThrust::sk_EventType);
	pGlobalEventManager->AddListener(MakeDelegate(this, &DemosLogic::EndThrustDelegate), EvtData_EndThrust::sk_EventType);
	pGlobalEventManager->AddListener(MakeDelegate(this, &DemosLogic::StartSteerDelegate), EvtData_StartSteer::sk_EventType);
	pGlobalEventManager->AddListener(MakeDelegate(this, &DemosLogic::EndSteerDelegate), EvtData_EndSteer::sk_EventType);

	pGlobalEventManager->AddListener(MakeDelegate(this, &DemosLogic::TestScriptDelegate), EvtData_ScriptEventTest_FromLua::sk_EventType);
}

void GameDemoLogic::RemoveAllDelegates(void)
{
	// FUTURE WORK: See the note in RegisterDelegates above....
	IEventManager* pGlobalEventManager = IEventManager::Get();
	pGlobalEventManager->RemoveListener(MakeDelegate(this, &DemosLogic::RemoteClientDelegate), EvtData_Remote_Client::sk_EventType);
	pGlobalEventManager->RemoveListener(MakeDelegate(this, &DemosLogic::MoveActorDelegate), EvtData_Move_Actor::sk_EventType);
	pGlobalEventManager->RemoveListener(MakeDelegate(this, &DemosLogic::RequestStartGameDelegate), EvtData_Request_Start_Game::sk_EventType);
	pGlobalEventManager->RemoveListener(MakeDelegate(this, &DemosLogic::NetworkPlayerActorAssignmentDelegate), EvtData_Network_Player_Actor_Assignment::sk_EventType);
	pGlobalEventManager->RemoveListener(MakeDelegate(this, &DemosLogic::EnvironmentLoadedDelegate), EvtData_Environment_Loaded::sk_EventType);
	pGlobalEventManager->RemoveListener(MakeDelegate(this, &DemosLogic::EnvironmentLoadedDelegate), EvtData_Remote_Environment_Loaded::sk_EventType);
	if (m_bProxy)
	{
		pGlobalEventManager->RemoveListener(MakeDelegate(this, &DemosLogic::RequestNewActorDelegate), EvtData_Request_New_Actor::sk_EventType);
	}

	// FUTURE WORK: These belong in teapot wars!
	pGlobalEventManager->RemoveListener(MakeDelegate(this, &DemosLogic::StartThrustDelegate), EvtData_StartThrust::sk_EventType);
	pGlobalEventManager->RemoveListener(MakeDelegate(this, &DemosLogic::EndThrustDelegate), EvtData_EndThrust::sk_EventType);
	pGlobalEventManager->RemoveListener(MakeDelegate(this, &DemosLogic::StartSteerDelegate), EvtData_StartSteer::sk_EventType);
	pGlobalEventManager->RemoveListener(MakeDelegate(this, &DemosLogic::EndSteerDelegate), EvtData_EndSteer::sk_EventType);

	pGlobalEventManager->RemoveListener(MakeDelegate(this, &DemosLogic::TestScriptDelegate), EvtData_ScriptEventTest_FromLua::sk_EventType);
}

void GameDemoLogic::CreateNetworkEventForwarder(const int socketId)
{
	NetworkEventForwarder* pNetworkEventForwarder = new NetworkEventForwarder(socketId);

	IEventManager* pGlobalEventManager = IEventManager::Get();

	// then add those events that need to be sent along to amy attached clients
	pGlobalEventManager->AddListener(MakeDelegate(pNetworkEventForwarder, &NetworkEventForwarder::ForwardEvent), EvtData_PhysCollision::sk_EventType);
	pGlobalEventManager->AddListener(MakeDelegate(pNetworkEventForwarder, &NetworkEventForwarder::ForwardEvent), EvtData_Destroy_Actor::sk_EventType);
	pGlobalEventManager->AddListener(MakeDelegate(pNetworkEventForwarder, &NetworkEventForwarder::ForwardEvent), EvtData_Fire_Weapon::sk_EventType);
	pGlobalEventManager->AddListener(MakeDelegate(pNetworkEventForwarder, &NetworkEventForwarder::ForwardEvent), EvtData_Environment_Loaded::sk_EventType);
	pGlobalEventManager->AddListener(MakeDelegate(pNetworkEventForwarder, &NetworkEventForwarder::ForwardEvent), EvtData_New_Actor::sk_EventType);
	pGlobalEventManager->AddListener(MakeDelegate(pNetworkEventForwarder, &NetworkEventForwarder::ForwardEvent), EvtData_Move_Actor::sk_EventType);
	pGlobalEventManager->AddListener(MakeDelegate(pNetworkEventForwarder, &NetworkEventForwarder::ForwardEvent), EvtData_Request_New_Actor::sk_EventType);
	pGlobalEventManager->AddListener(MakeDelegate(pNetworkEventForwarder, &NetworkEventForwarder::ForwardEvent), EvtData_Network_Player_Actor_Assignment::sk_EventType);

	m_networkEventForwarders.push_back(pNetworkEventForwarder);
}

void GameDemoLogic::DestroyAllNetworkEventForwarders(void)
{
	for (auto it = m_networkEventForwarders.begin(); it != m_networkEventForwarders.end(); ++it)
	{
		NetworkEventForwarder* pNetworkEventForwarder = (*it);

		IEventManager* pGlobalEventManager = IEventManager::Get();
		pGlobalEventManager->RemoveListener(MakeDelegate(pNetworkEventForwarder, &NetworkEventForwarder::ForwardEvent), EvtData_PhysCollision::sk_EventType);
		pGlobalEventManager->RemoveListener(MakeDelegate(pNetworkEventForwarder, &NetworkEventForwarder::ForwardEvent), EvtData_Destroy_Actor::sk_EventType);
		pGlobalEventManager->RemoveListener(MakeDelegate(pNetworkEventForwarder, &NetworkEventForwarder::ForwardEvent), EvtData_Fire_Weapon::sk_EventType);
		pGlobalEventManager->RemoveListener(MakeDelegate(pNetworkEventForwarder, &NetworkEventForwarder::ForwardEvent), EvtData_Environment_Loaded::sk_EventType);
		pGlobalEventManager->RemoveListener(MakeDelegate(pNetworkEventForwarder, &NetworkEventForwarder::ForwardEvent), EvtData_New_Actor::sk_EventType);
		pGlobalEventManager->RemoveListener(MakeDelegate(pNetworkEventForwarder, &NetworkEventForwarder::ForwardEvent), EvtData_Move_Actor::sk_EventType);
		pGlobalEventManager->RemoveListener(MakeDelegate(pNetworkEventForwarder, &NetworkEventForwarder::ForwardEvent), EvtData_Request_New_Actor::sk_EventType);
		pGlobalEventManager->RemoveListener(MakeDelegate(pNetworkEventForwarder, &NetworkEventForwarder::ForwardEvent), EvtData_Network_Player_Actor_Assignment::sk_EventType);

		delete pNetworkEventForwarder;
	}

	m_networkEventForwarders.clear();
}


bool GameDemoLogic::LoadGameDelegate(TiXmlElement* pLevelData)
{
	RegisterTeapotScriptEvents();
	return true;
}