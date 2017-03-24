//========================================================================
// GameLogic.h : Defines the Base Game Logic class 
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

#ifndef GAMELOGIC_H
#define GAMELOGIC_H

#include "GameEngineStd.h"

#include "Core/Process/ProcessManager.h"
#include "Core/Event/EventManager.h"
#include "Game/Actor/Actor.h"

#include "Mathematic/Algebra/Transform.h"
#include "Mathematic/Algebra/Matrix4x4.h"

class PathingGraph;
class ActorFactory;
class LevelManager;
class BaseGamePhysic;

enum BaseGameState
{
    BGS_Invalid,
	BGS_Initializing,
	BGS_MainMenu,
	BGS_WaitingForPlayers,
	BGS_LoadingGameEnvironment,
	BGS_WaitingForPlayersToLoadEnvironment,
	BGS_SpawningPlayersActors,
	BGS_Running
};

typedef eastl::map<ActorId, eastl::shared_ptr<Actor>> ActorMap;

/*
	The game logic defines the game universe, what is in there and how they interact. 
	It also defines how the game state can be changed by external stimulus. The main
	components of the game logic are: GameState, Physics, Event, ProcessManager and
	Command Interpreter.
*/
class BaseGameLogic
{
public:
	virtual eastl::weak_ptr<Actor> GetActor(const ActorId id) = 0;
	virtual eastl::shared_ptr<Actor> CreateActor(const eastl::string &actorResource, 
		XMLElement *overrides, const Matrix4x4<float> *initialTransform = NULL, 
		const ActorId serversActorId = INVALID_ACTOR_ID) = 0;
	virtual void DestroyActor(const ActorId actorId) = 0;
	virtual bool LoadGame(const char* levelResource) = 0;
	virtual void SetProxy() = 0;
	virtual void OnUpdate(float time, float elapsedTime) = 0;
	virtual void ChangeState(enum BaseGameState newState) = 0;
	virtual void MoveActor(const ActorId id, Transform const &transform) = 0;
};


class GameLogic : public BaseGameLogic
{
	friend class GameApplication;					// This is only to gain access to the view list

protected:
	float m_Lifetime;								//indicates how long this game has been in session
	ProcessManager* m_pProcessManager;				// a game logic entity
	ActorMap m_actors;
	ActorId m_LastActorId;
	BaseGameState m_State;							// game state: loading, running, etc.
	int m_ExpectedPlayers;							// how many local human players
	int m_ExpectedRemotePlayers;					// expected remote human players
	int m_ExpectedAI;								// how many AI players
	int m_HumanPlayersAttached;
	int m_AIPlayersAttached;
	int m_HumanGamesLoaded;
	//shared_ptr<PathingGraph> m_pPathingGraph;		// the pathing graph
    ActorFactory* m_pActorFactory;
    
	bool m_bProxy;									// set if this is a proxy game logic, not a real one
	int m_remotePlayerId;							// if we are a remote player - what is out socket number on the server

	bool m_RenderDiagnostics;						// Are we rendering diagnostics?
	eastl::shared_ptr<BaseGamePhysic> mPhysics;

	LevelManager* mLevelManager;					// Manages loading and chaining levels

public:

	GameLogic();
	virtual ~GameLogic();
    bool Init(void);

	void SetProxy(bool isProxy) 
	{ 
		m_bProxy = isProxy; 
	}
	const bool IsProxy() const { return m_bProxy; }

	// [mrmike] CanRunLua() is a bit of a hack - but I can't have Lua scripts running on the clients. They should belong to the logic.
	// FUTURE WORK - Perhaps the scripts can have a marker or even a special place in the resource file for any scripts that can run on remote clients
	const bool CanRunLua() const { return !IsProxy() || GetState()!=BGS_Running; }

	ActorId GetNewActorID( void )
	{
		return ++m_LastActorId;
	}
	
	//shared_ptr<PathingGraph> GetPathingGraph(void) { return m_pPathingGraph; }
	//RandomGenerator& GetRNG(void) { return m_Random; }

	// [rez] note: don't store this strong pointer outside of this class 
    virtual eastl::shared_ptr<Actor> CreateActor(const eastl::string &actorResource, XMLElement *overrides,
		const Transform *initialTransform=NULL, const ActorId serversActorId=INVALID_ACTOR_ID);

    virtual void DestroyActor(const ActorId actorId);
    virtual eastl::weak_ptr<Actor> GetActor(const ActorId actorId);
	virtual void ModifyActor(const ActorId actorId, XMLElement *overrides);

	virtual void MoveActor(const ActorId id, Matrix4x4<float> const &mat) {}

    // editor functions
	eastl::string GetActorXml(const ActorId id);

	// Level management
	const LevelManager* GetLevelManager() { return mLevelManager; }
	// [rez] Subclasses shouldn't override this function; use LoadGameDelegate() instead
    virtual bool LoadGame(const char* levelResource) override;  
	virtual void SetProxy();

	// Logic Update
	virtual void OnUpdate(float time, float elapsedTime);

	// Changing Game Logic State
	virtual void ChangeState(BaseGameState newState);
	const BaseGameState GetState() const { return m_State; }

	// Render Diagnostics
	void ToggleRenderDiagnostics() { m_RenderDiagnostics = !m_RenderDiagnostics; }
	virtual void RenderDiagnostics();
	virtual eastl::shared_ptr<BaseGamePhysic> GetGamePhysics(void) { return mPhysics; }
	
	void AttachProcess(eastl::shared_ptr<Process> pProcess)
	{ if (m_pProcessManager) {m_pProcessManager->AttachProcess(pProcess);} }

    // event delegates
    void RequestDestroyActorDelegate(BaseEventDataPtr pEventData);

protected:
    virtual ActorFactory* CreateActorFactory(void);

    // [rez] Override this function to do any game-specific loading.
    virtual bool LoadGameDelegate(XMLElement* pLevelData) { return true; }

    void MoveActorDelegate(BaseEventDataPtr pEventData);
    void RequestNewActorDelegate(BaseEventDataPtr pEventData);
};


#endif
