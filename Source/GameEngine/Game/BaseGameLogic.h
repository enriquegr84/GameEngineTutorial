//========================================================================
// BaseGameLogic.h : Defines the Base Game Logic class 
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

#ifndef BASEGAMELOGIC_H
#define BASEGAMELOGIC_H

#include "GameEngineStd.h"

#include "GameEngine/interfaces.h"

#include "Process/ProcessManager.h"
#include "Events/EventManager.h"
#include "Actors/Actor.h"

class PathingGraph;
class ActorFactory;
class LevelManager;

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

typedef eastl::map<ActorId, StrongActorPtr> ActorMap;


class BaseGameLogic : public BaseGameLogic
{
	friend class GameEngineApp;						// This is only to gain access to the view list

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
	shared_ptr<BaseGamePhysic> m_pPhysics;

	LevelManager* m_pLevelManager;					// Manages loading and chaining levels

public:

	BaseGameLogic();
	virtual ~BaseGameLogic();
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
    virtual StrongActorPtr CreateActor(const eastl::string &actorResource, XmlElement *overrides, 
		const matrix4 *initialTransform=NULL, const ActorId serversActorId=INVALID_ACTOR_ID);

    virtual void DestroyActor(const ActorId actorId);
    virtual WeakActorPtr GetActor(const ActorId actorId);
	virtual void ModifyActor(const ActorId actorId, XmlElement *overrides);

	virtual void MoveActor(const ActorId id, matrix4 const &mat) {}

    // editor functions
	eastl::string GetActorXml(const ActorId id);

	// Level management
	const LevelManager* GetLevelManager() { return m_pLevelManager; }
	// [rez] Subclasses shouldn't override this function; use LoadGameDelegate() instead
    virtual bool LoadGame(const wchar_t* levelResource) override;  
	virtual void SetProxy();

	// Logic Update
	virtual void OnUpdate(float time, float elapsedTime);

	// Changing Game Logic State
	virtual void ChangeState(BaseGameState newState);
	const BaseGameState GetState() const { return m_State; }

	// Render Diagnostics
	void ToggleRenderDiagnostics() { m_RenderDiagnostics = !m_RenderDiagnostics; }
	virtual void RenderDiagnostics();
	virtual shared_ptr<BaseGamePhysic> GetGamePhysics(void) { return m_pPhysics; }
	
	void AttachProcess(StrongProcessPtr pProcess) 
	{ if (m_pProcessManager) {m_pProcessManager->AttachProcess(pProcess);} }

    // event delegates
    void RequestDestroyActorDelegate(BaseEventDataPtr pEventData);

protected:
    virtual ActorFactory* CreateActorFactory(void);

    // [rez] Override this function to do any game-specific loading.
    virtual bool LoadGameDelegate(XmlElement* pLevelData) { return true; }

    void MoveActorDelegate(BaseEventDataPtr pEventData);
    void RequestNewActorDelegate(BaseEventDataPtr pEventData);
};


#endif
