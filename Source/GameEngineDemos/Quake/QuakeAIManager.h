//
//  This program is free software; you can redistribute it and/or
//  modify it under the terms of the GNU General Public License
//  as published by the Free Software Foundation; either version 3
//  of the License, or (at your option) any later version.
//
//  This program is distributed in the hope that it will be useful,
//  but WITHOUT ANY WARRANTY; without even the implied warranty of
//  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//  GNU General Public License for more details.
//
//  You should have received a copy of the GNU General Public License
//  along with this program; if not, write to the Free Software
//  Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.

#ifndef QUAKEAIMANAGER_H
#define QUAKEAIMANAGER_H

#include "QuakeStd.h"

#include "AI/AIManager.h"

#include "Actors/PlayerActor.h"
#include "Core/Event/EventManager.h"

#include "Physic/PhysicEventListener.h"
#include "Mathematic/Algebra/Matrix4x4.h"

enum GameActionType
{
	GAT_MOVE = 0x0000000,
	GAT_PUSH = 0x0000002,
	GAT_TELEPORT = 0x0000006,
	GAT_FALL = 0x000000A,
	GAT_JUMP = 0x00000E
};

//
// struct NodeState
//
struct NodeState
{
	NodeState()
	{
		valid = false;
		player = INVALID_ACTOR_ID;
		target = INVALID_ACTOR_ID;

		node = NULL;

		weapon = WP_NONE;
		heuristic = 0.f;
		for (unsigned int i = 0; i < MAX_STATS; i++)
		{
			stats[i] = 0;
		}

		for (unsigned int i = 0; i < MAX_WEAPONS; i++)
		{
			ammo[i] = 0;
			damage[i] = 0;
		}
	}

	NodeState(eastl::shared_ptr<PlayerActor> playerActor)
	{
		valid = true;
		player = playerActor->GetId();
		target = INVALID_ACTOR_ID;

		node = NULL;

		weapon = WP_NONE;
		heuristic = 0.f;

		for (unsigned int i = 0; i < MAX_STATS; i++)
		{
			stats[i] = playerActor->GetState().stats[i];
		}

		for (unsigned int i = 0; i < MAX_WEAPONS; i++)
		{
			ammo[i] = playerActor->GetState().ammo[i];
			damage[i] = 0;
		}
	}

	NodeState(const NodeState& state) :
		valid(state.valid),
		node(state.node), path(state.path),
		player(state.player), target(state.target),
		weapon(state.weapon), heuristic(state.heuristic)
	{
		for (unsigned int i = 0; i < MAX_STATS; i++)
		{
			stats[i] = state.stats[i];
		}

		for (unsigned int i = 0; i < MAX_WEAPONS; i++)
		{
			ammo[i] = state.ammo[i];
			damage[i] = state.damage[i];
		}

		for (eastl::shared_ptr<Actor> item : state.items)
		{
			items.push_back(item);
			itemAmount[item] = state.itemAmount.at(item);
			itemDistance[item] = state.itemDistance.at(item);
		}
	}

	~NodeState()
	{

	}

	void NodeState::Copy(NodeState& state)
	{
		valid = state.valid;
		player = state.player;
		target = state.target;

		node = state.node;
		path = state.path;

		weapon = state.weapon;
		heuristic = state.heuristic;

		for (unsigned int i = 0; i < MAX_STATS; i++)
		{
			stats[i] = state.stats[i];
		}

		for (unsigned int i = 0; i < MAX_WEAPONS; i++)
		{
			ammo[i] = state.ammo[i];
			damage[i] = state.damage[i];
		}

		for (eastl::shared_ptr<Actor> item : state.items)
		{
			items.push_back(item);
			itemAmount[item] = state.itemAmount[item];
			itemDistance[item] = state.itemDistance[item];
		}
	}

	void NodeState::CopyItems(NodeState& state)
	{
		items.clear();
		itemAmount.clear();
		itemDistance.clear();
		for (eastl::shared_ptr<Actor> item : state.items)
		{
			items.push_back(item);
			itemAmount[item] = state.itemAmount[item];
			itemDistance[item] = state.itemDistance[item];
		}
	}

	void NodeState::ResetItems()
	{
		items.clear();
		itemAmount.clear();
		itemDistance.clear();
	}

	bool valid;
	ActorId player;
	ActorId target;

	PathingNode* node;
	PathingArcVec path;

	float heuristic;
	WeaponType weapon;
	int stats[MAX_STATS];
	int ammo[MAX_WEAPONS];
	int damage[MAX_WEAPONS];

	eastl::vector<eastl::shared_ptr<Actor>> items;
	eastl::map<eastl::shared_ptr<Actor>, int> itemAmount;
	eastl::map<eastl::shared_ptr<Actor>, float> itemDistance;
};

class QuakeAIManager : public AIManager
{

public:
	QuakeAIManager();
	~QuakeAIManager();

	virtual void CreateMap(ActorId playerId);

	virtual void SavePathingGraph(const eastl::string& path);
	virtual void LoadPathingGraph(const eastl::wstring& path);

	virtual void OnUpdate(unsigned long deltaMs);

	ActorId GetPlayerTarget(ActorId player);
	WeaponType GetPlayerWeapon(ActorId player);
	void GetPlayerPath(ActorId player, PathingArcVec& playerPath);
	bool IsPlayerUpdated(ActorId player);

	void SetPlayerTarget(ActorId player, ActorId playerTarget);
	void SetPlayerWeapon(ActorId player, WeaponType playerWeapon);
	void SetPlayerPath(ActorId player, PathingArcVec& playerPath);
	void SetPlayerUpdated(ActorId player, bool update);

	void RemovePlayerGuessItems(ActorId player);
	void SetPlayerGuessItems(ActorId player, eastl::map<ActorId, float>& guessItems);
	void GetPlayerGuessItems(ActorId player, eastl::map<ActorId, float>& guessItems);

	NodeState& GetPlayerGuessState(ActorId player);
	PathingNode* GetPlayerGuessNode(ActorId player);
	void GetPlayerGuessPath(ActorId player, PathingArcVec& playerPath);
	bool IsPlayerGuessUpdate(ActorId player);

	void SetPlayerGuessState(ActorId player, eastl::shared_ptr<PlayerActor> playerActor);
	void SetPlayerGuessState(ActorId player, NodeState& playerState);
	void SetPlayerGuessNode(ActorId player, PathingNode* playerNode);
	void SetPlayerGuessPath(ActorId player, PathingArcVec& playerPath);
	void SetPlayerGuessUpdate(ActorId player, bool update);

	void SpawnActor(ActorId playerId);

protected:

	void SimulateJump(PathingNode* pNode);
	void SimulateMovement(PathingNode* pNode);
	void SimulateTriggerPush(PathingNode* pNode, const Vector3<float>& target);
	void SimulateTriggerTeleport(PathingNode* pNode, const Vector3<float>& target);
	void SimulateActorPosition(ActorId actorId, const Vector3<float>& position);
	void SimulateGrenadeLauncherFire(PathingNode* pNode, eastl::shared_ptr<Actor> pGameActor);

	void SimulateWaypoint();
	void SimulateVisibility();

	void CreateClusters();

	unsigned int GetNewArcID(void)
	{
		return ++mLastArcId;
	}

	unsigned int GetNewNodeID(void)
	{
		return ++mLastNodeId;
	}

	// event delegates
	void PhysicsTriggerEnterDelegate(BaseEventDataPtr pEventData);
	void PhysicsTriggerLeaveDelegate(BaseEventDataPtr pEventData);
	void PhysicsCollisionDelegate(BaseEventDataPtr pEventData);
	void PhysicsSeparationDelegate(BaseEventDataPtr pEventData);

	// Orientation Controls
	float mYaw;
	float mPitch;
	float mPitchTarget;
	float mPitchOnDown;
	float mYawOnDown;

	// Speed Controls
	float mMaxFallSpeed;
	float mMaxJumpSpeed;
	float mMaxRotateSpeed;
	float mFallSpeed;
	float mJumpSpeed;
	float mJumpMoveSpeed;
	float mMoveSpeed;
	float mRotateSpeed;

	eastl::shared_ptr<PlayerActor> mPlayerActor;

private:

	unsigned int mLastArcId;
	unsigned int mLastNodeId;

	//set of nodes to be analized from the ground
	eastl::vector<PathingNode*> mOpenSet, mClosedSet;

	//pathing nodes which contains actors from game
	eastl::map<PathingNode*, ActorId> mActorNodes;

	//player ai states
	eastl::map<ActorId, bool> mPlayers;
	eastl::map<ActorId, ActorId> mPlayerTargets;
	eastl::map<ActorId, WeaponType> mPlayerWeapons;
	eastl::map<ActorId, PathingArcVec> mPlayerPaths;

	eastl::map<ActorId, bool> mPlayerGuess;
	eastl::map<ActorId, NodeState> mPlayerGuessStates;
	eastl::map<ActorId, PathingNode*> mPlayerGuessNodes;
	eastl::map<ActorId, eastl::map<ActorId, float>> mPlayerGuessItems;

	eastl::map<ActorId, float> mPlayerGuessTime;
	eastl::map<ActorId, PathingArcVec> mPlayerGuessPlan;

	void RegisterAllDelegates(void);
	void RemoveAllDelegates(void);

};   // QuakeAIManager

#endif
