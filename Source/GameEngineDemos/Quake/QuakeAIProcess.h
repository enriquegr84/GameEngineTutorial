//========================================================================
// QuakeAIProcess.h : Defines real time ai process
//
// Part of the GameCode4 Application
//
// GameCode4 is the sample application that encapsulates much of the source code
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
//    http://code.google.com/p/gamecode4/
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

#ifndef QUAKEAIPROCESS_H
#define QUAKEAIPROCESS_H

#include "Core/Process/RealtimeProcess.h"
#include "Core/Event/EventManager.h"

#include "QuakeAIManager.h"

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

	NodeState(NodeState const& state) : 
		valid(state.valid),
		node(state.node), path(state.path),
		player(state.player), target(state.target), 
		weapon(state.weapon), heuristic(state.heuristic),
		items(state.items), itemAmount(state.itemAmount), itemDistance(state.itemDistance)
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
	}

	~NodeState()
	{

	}

	void NodeState::Copy(NodeState const& state)
	{
		valid = state.valid;
		player = state.player;
		target = state.target;

		node = state.node;
		path = state.path;

		items = state.items;
		itemAmount = state.itemAmount;
		itemDistance = state.itemDistance;

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

//
// class QuakeAIProcess
//
class QuakeAIProcess : public RealtimeProcess
{
public:

	QuakeAIProcess();
	~QuakeAIProcess();

	virtual void ThreadProc(void);

protected:

	float HeuristicPlayerItems(NodeState& playerState);
	void Heuristic(NodeState& playerState, NodeState& otherPlayerState);
	void Damage(NodeState& state, float visibleTime, float visibleDistance, float visibleHeight);
	void PickupItem(NodeState& playerState, eastl::map<ActorId, float>& actors);

	void Visibility(
		PathingNode* playerNode, PathingArcVec& playerPathPlan,
		float* visibleTime, float* visibleDistance, float* visibleHeight,
		PathingNode* otherPlayerNode, PathingArcVec& otherPlayerPathPlan,
		float* otherVisibleTime, float* otherVisibleDistance, float* otherVisibleHeight);
	void ConstructPath(PathingNode* playerNode, 
		PathingCluster* playerClusters, eastl::vector<PathingArcVec>& playerPathPlan,
		eastl::map<ActorId, eastl::map<PathingCluster*, PathingArcVec>>& actorPathPlans);
	void Simulation(
		NodeState& playerState, eastl::vector<PathingArcVec>& playerPathPlans,
		NodeState& otherPlayerState, eastl::vector<PathingArcVec>& otherPlayerPathPlans);

	void EvaluatePlayers(
		NodeState& playerState, NodeState& otherPlayerState);

private:

	FILE * mFile;
	NodeState mPlayerState, mOtherPlayerState;

	QuakeAIManager*	mAIManager;
};

#endif