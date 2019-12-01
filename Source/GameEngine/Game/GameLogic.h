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

class ActorFactory;
class LevelManager;
class AIManager;
class BaseGameView;
class BaseGamePhysic;

enum BaseGameState
{
    BGS_INVALID,
	BGS_INITIALIZING,
	BGS_MAINMENU,
	BGS_WAITINGFORPLAYERS,
	BGS_LOADINGGAMEENVIRONMENT,
	BGS_WAITINGFORPLAYERSTOLOADENVIRONMENT,
	BGS_SPAWNINGPLAYERACTORS,
	BGS_RUNNING
};

typedef eastl::map<ActorId, eastl::shared_ptr<Actor>> ActorMap;


class BaseGameLogic
{
public:
	virtual eastl::weak_ptr<Actor> GetActor(const ActorId id) = 0;
	virtual eastl::shared_ptr<Actor> CreateActor(const eastl::string &actorResource, 
		tinyxml2::XMLElement *overrides, const Transform *initialTransform = NULL,
		const ActorId serversActorId = INVALID_ACTOR_ID) = 0;
	virtual void DestroyActor(const ActorId actorId) = 0;
	virtual bool LoadGame(const char* levelResource) = 0;
	virtual void SetProxy() = 0;
	virtual void OnUpdate(float time, float elapsedTime) = 0;
	virtual void ChangeState(enum BaseGameState newState) = 0;
	virtual void SyncActor(const ActorId id, Transform const &transform) = 0;
};


class GameLogic : public BaseGameLogic
{
	friend class GameApplication;					// This is only to gain access to the view list


public:

	GameLogic();

	virtual ~GameLogic();

	bool Init(void);

	void SetIsProxy(bool isProxy)
	{
		mIsProxy = isProxy;
	}
	const bool IsProxy() const { return mIsProxy; }

	ActorId GetNewActorID(void)
	{
		return ++mLastActorId;
	}

	virtual eastl::shared_ptr<Actor> CreateActor(const eastl::string &actorResource,
		tinyxml2::XMLElement *overrides, const Transform *initialTransform = NULL,
		const ActorId serversActorId = INVALID_ACTOR_ID);

	virtual void DestroyActor(const ActorId actorId);
	virtual eastl::weak_ptr<Actor> GetActor(const ActorId actorId);
	virtual void ModifyActor(const ActorId actorId, tinyxml2::XMLElement *overrides);

	virtual void SyncActor(const ActorId id, Transform const &transform) {}

	// editor functions
	eastl::string GetActorXml(const ActorId id);

	LevelManager* GetLevelManager() { return mLevelManager; }
	AIManager* GetAIManager() { return mAIManager; }

	// [rez] Subclasses shouldn't override this function; use LoadGameDelegate() instead
	virtual bool LoadGame(const char* levelResource) override;
	virtual void SetProxy();

	virtual void ResetViewType() { }
	virtual void UpdateViewType(const eastl::shared_ptr<BaseGameView>& pView, bool add) { }

	// Logic Update
	virtual void OnUpdate(float time, float elapsedTime);

	// Changing Game Logic State
	virtual void ChangeState(BaseGameState newState);
	const BaseGameState GetState() const { return mGameState; }

	// Render Diagnostics
	void ToggleRenderDiagnostics() { mIsRenderDiagnostics = !mIsRenderDiagnostics; }
	virtual void RenderDiagnostics();
	virtual eastl::shared_ptr<BaseGamePhysic> GetGamePhysics(void) { return mPhysics; }

	void AttachProcess(eastl::shared_ptr<Process> pProcess)
	{
		if (mProcessManager) { mProcessManager->AttachProcess(pProcess); }
	}

	// event delegates
	void RequestDestroyActorDelegate(BaseEventDataPtr pEventData);

	// Getter for the main global game. This is the game that is used by the majority of the 
	// engine, though you are free to define your own as long as you instantiate it.
	// It is not valid to have more than one global game.
	static GameLogic* Get(void);

protected:

	static GameLogic* mGame;

	virtual ActorFactory* CreateActorFactory(void);
	virtual LevelManager* CreateLevelManager(void);
	virtual AIManager* CreateAIManager(void);

	// [rez] Override this function to do any game-specific loading.
	virtual bool LoadGameDelegate(tinyxml2::XMLElement* pLevelData) { return true; }

	void SyncActorDelegate(BaseEventDataPtr pEventData);
	void RequestNewActorDelegate(BaseEventDataPtr pEventData);

	float mLifetime;								//indicates how long this game has been in session

	ActorMap mActors;
	ActorId mLastActorId;
	BaseGameState mGameState;							// game state: loading, running, etc.
	int mExpectedPlayers;							// how many local human players
	int mExpectedRemotePlayers;					// expected remote human players
	int mExpectedAI;								// how many AI players
	int mHumanPlayersAttached;
	int mAIPlayersAttached;
	int mHumanGamesLoaded;

	AIManager*	mAIManager;
	ProcessManager* mProcessManager;
	LevelManager* mLevelManager;					// Manages loading and changing levels
    ActorFactory* mActorFactory;
    
	bool mIsProxy;									// set if this is a proxy game logic, not a real one
	int mRemotePlayerId;							// if we are a remote player - what is out socket number on the server

	bool mIsRenderDiagnostics;						// Are we rendering diagnostics?
	eastl::shared_ptr<BaseGamePhysic> mPhysics;
};


#endif
