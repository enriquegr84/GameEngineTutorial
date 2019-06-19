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
	void ConstructActorPath(PathingCluster* playerCluster, 
		PathingNode* currentNode, float currentDistance, PathingArcVec& currentPathPlan,
		eastl::vector<ActorId>& actors, float* actorPathDistance, PathingArcVec& actorPathPlan);
	void ConstructPath(PathingNode* playerNode, 
		PathingCluster* playerCluster, eastl::vector<PathingArcVec>& playerPathPlan);
	void Simulation(
		NodeState& playerState, eastl::vector<PathingArcVec>& playerPathPlans,
		NodeState& otherPlayerState, eastl::vector<PathingArcVec>& otherPlayerPathPlans);
	void EvaluatePlayers(
		NodeState& playerState, NodeState& otherPlayerState);

private:

	FILE * mFile;

	QuakeAIManager*	mAIManager;

	eastl::map<ActorId, float> mExcludeActors;
	NodeState mPlayerState, mOtherPlayerState;
};

#endif