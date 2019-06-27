//========================================================================
// QuakeAIProcess.cpp : Defines ai process that can run in a thread
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

#include "Quake.h"
#include "QuakeApp.h"
#include "QuakeEvents.h"
#include "QuakeAIProcess.h"

#include "Core/OS/OS.h"

QuakeAIProcess::QuakeAIProcess()
  :  RealtimeProcess()
{
	mAIManager = dynamic_cast<QuakeAIManager*>(GameLogic::Get()->GetAIManager());
}

QuakeAIProcess::~QuakeAIProcess(void)
{

}

void QuakeAIProcess::Visibility(
	PathingNode* playerNode, PathingArcVec& playerPathPlan, 
	PathingNode* otherPlayerNode, PathingArcVec& otherPlayerPathPlan,
	bool* isVisible, float* visibleTime, float* visibleDistance, float* visibleHeight,
	bool* isOtherVisible, float* otherVisibleTime, float* otherVisibleDistance, float* otherVisibleHeight)
{
	float visibleWeight = 0.f;
	float totalTime = 0.f, totalArcTime = 0.f;
	unsigned int index = 0, otherIndex = 0, otherPathIndex = 0;

	PathingNode* currentNode = playerNode;
	PathingTransition* currentTransition = NULL;

	PathingNode* otherCurrentNode = otherPlayerNode;
	PathingTransition* otherCurrentTransition =
		otherCurrentNode->FindTransition(otherPlayerPathPlan[otherPathIndex]->GetId());

	for (PathingArc* currentArc : playerPathPlan)
	{
		index = 0;
		currentTransition = currentNode->FindTransition(currentArc->GetId());
		for (; index < currentTransition->GetNodes().size(); index++)
		{
			visibleWeight = currentTransition->GetWeights()[index];

			if (mAIManager->GetPathingGraph()->IsVisibleCluster(
				currentTransition->GetNodes()[index]->GetCluster(),
				otherCurrentTransition->GetNodes()[otherIndex]->GetCluster()))
			{
				if (currentTransition->GetNodes()[index]->IsVisibleNode(
					otherCurrentTransition->GetNodes()[otherIndex]))
				{
					(*visibleDistance) += Length(
						otherCurrentTransition->GetConnections()[otherIndex] -
						currentTransition->GetConnections()[index]) * visibleWeight;
					(*visibleHeight) +=
						(currentTransition->GetConnections()[index][2] -
						otherCurrentTransition->GetConnections()[otherIndex][2]) * visibleWeight;
					(*visibleTime) += visibleWeight;

					(*otherVisibleDistance) += Length(
						otherCurrentTransition->GetConnections()[otherIndex] -
						currentTransition->GetConnections()[index]) * visibleWeight;
					(*otherVisibleHeight) +=
						(otherCurrentTransition->GetConnections()[otherIndex][2] -
						currentTransition->GetConnections()[index][2]) * visibleWeight;
					(*otherVisibleTime) += visibleWeight;
				}
			}
			while (totalArcTime <= totalTime)
			{
				totalArcTime += otherCurrentTransition->GetWeights()[otherIndex];
				if (otherIndex + 1 >= otherCurrentTransition->GetNodes().size())
				{
					if (otherPathIndex + 1 < otherPlayerPathPlan.size())
					{
						otherIndex = 0;
						otherCurrentNode = otherPlayerPathPlan[otherPathIndex]->GetNode();

						otherPathIndex++;
						otherCurrentTransition = otherCurrentNode->FindTransition(
							otherPlayerPathPlan[otherPathIndex]->GetId());
					}
					else break;
				}
				else otherIndex++;
			}
			totalTime += visibleWeight;
		}
		currentNode = currentArc->GetNode();
	}

	if (currentTransition)
	{
		index--;
		otherCurrentTransition = otherCurrentNode->FindTransition(
			otherPlayerPathPlan[otherPathIndex]->GetId());
		for (; otherIndex < otherCurrentTransition->GetNodes().size(); otherIndex++)
		{
			visibleWeight = currentTransition->GetWeights()[index];

			if (mAIManager->GetPathingGraph()->IsVisibleCluster(
				currentTransition->GetNodes()[index]->GetCluster(), 
				otherCurrentTransition->GetNodes()[otherIndex]->GetCluster()))
			{
				if (currentTransition->GetNodes()[index]->IsVisibleNode(
					otherCurrentTransition->GetNodes()[otherIndex]))
				{
					(*visibleDistance) += Length(
						otherCurrentTransition->GetConnections()[otherIndex] -
						currentTransition->GetConnections()[index]) * visibleWeight;
					(*visibleHeight) +=
						(currentTransition->GetConnections()[index][2] -
						otherCurrentTransition->GetConnections()[otherIndex][2]) * visibleWeight;
					(*visibleTime) += visibleWeight;

					(*otherVisibleDistance) += Length(
						otherCurrentTransition->GetConnections()[otherIndex] -
						currentTransition->GetConnections()[index]) * visibleWeight;
					(*otherVisibleHeight) +=
						(otherCurrentTransition->GetConnections()[otherIndex][2] -
						currentTransition->GetConnections()[index][2]) * visibleWeight;
					(*otherVisibleTime) += visibleWeight;
				}
			}
		}

		//lets put a minimum of potential visibility time
		otherIndex--;
		if (totalTime < 3.0f)
		{
			visibleWeight = 3.0f - totalTime;

			if (mAIManager->GetPathingGraph()->IsVisibleCluster(
				currentTransition->GetNodes()[index]->GetCluster(),
				otherCurrentTransition->GetNodes()[otherIndex]->GetCluster()))
			{
				if (currentTransition->GetNodes()[index]->IsVisibleNode(
					otherCurrentTransition->GetNodes()[otherIndex]))
				{
					(*visibleDistance) += Length(
						otherCurrentTransition->GetConnections()[otherIndex] -
						currentTransition->GetConnections()[index]) * visibleWeight;
					(*visibleHeight) +=
						(currentTransition->GetConnections()[index][2] -
						otherCurrentTransition->GetConnections()[otherIndex][2]) * visibleWeight;
					(*visibleTime) += visibleWeight;

					(*otherVisibleDistance) += Length(
						otherCurrentTransition->GetConnections()[otherIndex] -
						currentTransition->GetConnections()[index]) * visibleWeight;
					(*otherVisibleHeight) +=
						(otherCurrentTransition->GetConnections()[otherIndex][2] -
						currentTransition->GetConnections()[index][2]) * visibleWeight;
					(*otherVisibleTime) += visibleWeight;
				}
			}
		}

		//if visible time is below a minimum we take it as potential visibility
		if ((*visibleTime) <= 0.4f)
		{
			//though it wasn't found any visible node along the path we need to take into 
			//consideration any potential visible node. That is how we differentiate when 
			//there is no chance to see any node and when we might be able to find some
			PathingArcVec::reverse_iterator itOtherPathPlan = otherPlayerPathPlan.rbegin();
			for (; itOtherPathPlan != otherPlayerPathPlan.rend(); itOtherPathPlan++)
			{
				PathingArc* otherPathArc = (*itOtherPathPlan);
				PathingNode* otherPathNode = otherPathArc->GetNode();

				visibleWeight = 3.0f;
				for (index = 0; index < currentTransition->GetNodes().size(); index++)
				{
					if (mAIManager->GetPathingGraph()->IsVisibleCluster(
						currentTransition->GetNodes()[index]->GetCluster(), otherPathNode->GetCluster()))
					{
						if (currentTransition->GetNodes()[index]->IsVisibleNode(otherPathNode))
						{
							(*visibleDistance) =
								Length(otherPathNode->GetPos() - currentTransition->GetConnections()[index]);
							(*visibleDistance) *= visibleWeight;
							(*visibleHeight) = 
								(currentTransition->GetConnections()[index][2] - otherPathNode->GetPos()[2]);
							(*visibleHeight) *= visibleWeight;
							(*visibleTime) = visibleWeight;

							(*otherVisibleDistance) = 
								Length(otherPathNode->GetPos() - currentTransition->GetConnections()[index]);
							(*otherVisibleDistance) *= visibleWeight;
							(*otherVisibleHeight) = 
								(otherPathNode->GetPos()[2] - currentTransition->GetConnections()[index][2]);
							(*otherVisibleHeight) *= visibleWeight;
							(*otherVisibleTime) = visibleWeight;

							(*isVisible) = false;
							(*isOtherVisible) = false;
							break;
						}
					}
				}

				if (!(*isVisible)) break;
			}
		}

		if ((*otherVisibleTime) <= 0.4f)
		{
			//though it wasn't found any visible node along the path we need to take into 
			//consideration any potential visible node. That is how we differentiate when 
			//there is no chance to see any node and when we might be able to find some
			PathingArcVec::reverse_iterator itPathPlan = playerPathPlan.rbegin();
			for (; itPathPlan != playerPathPlan.rend(); itPathPlan++)
			{
				PathingArc* pathArc = (*itPathPlan);

				visibleWeight = 3.0f;
				for (otherIndex = 0; otherIndex < otherCurrentTransition->GetNodes().size(); otherIndex++)
				{
					if (mAIManager->GetPathingGraph()->IsVisibleCluster(
						otherCurrentTransition->GetNodes()[otherIndex]->GetCluster(), pathArc->GetNode()->GetCluster()))
					{
						if (pathArc->GetNode()->IsVisibleNode(otherCurrentTransition->GetNodes()[otherIndex]))
						{
							(*visibleDistance) = 
								Length(otherCurrentTransition->GetConnections()[otherIndex] - pathArc->GetNode()->GetPos());
							(*visibleDistance) *= visibleWeight;
							(*visibleHeight) = 
								(pathArc->GetNode()->GetPos()[2] - otherCurrentTransition->GetConnections()[otherIndex][2]);
							(*visibleHeight) *= visibleWeight;
							(*visibleTime) = visibleWeight;

							(*otherVisibleDistance) = 
								Length(otherCurrentTransition->GetConnections()[otherIndex] - pathArc->GetNode()->GetPos());
							(*otherVisibleDistance) *= visibleWeight;
							(*otherVisibleHeight) = 
								(otherCurrentTransition->GetConnections()[otherIndex][2] - pathArc->GetNode()->GetPos()[2]);
							(*otherVisibleHeight) *= visibleWeight;
							(*otherVisibleTime) = visibleWeight;

							(*isVisible) = false;
							(*isOtherVisible) = false;
							break;
						}
					}
				}

				if (!(*isOtherVisible)) break;
			}
		}

		//average
		if ((*visibleTime) > 0.f)
		{
			(*visibleDistance) /= (*visibleTime);
			(*visibleHeight) /= (*visibleTime);
		}

		if ((*otherVisibleTime) > 0.f)
		{
			(*otherVisibleDistance) /= (*otherVisibleTime);
			(*otherVisibleHeight) /= (*otherVisibleTime);
		}
	}
}

void QuakeAIProcess::Simulation(
	NodeState& playerState, eastl::vector<PathingArcVec>& playerPathPlans,
	NodeState& otherPlayerState, eastl::vector<PathingArcVec>& otherPlayerPathPlans)
{
	PathingNode* playerNode = playerState.node;
	PathingNode* otherPlayerNode = otherPlayerState.node;
	eastl::map<PathingArcVec, eastl::map<PathingArcVec, NodeState>> playerStates, otherPlayerStates;
	for (PathingArcVec playerPathPlan : playerPathPlans)
	{
		float pathPlanWeight = 0.f;
		eastl::map<ActorId, float> actors;
		if (playerNode->GetActorId() != INVALID_ACTOR_ID)
			actors[playerNode->GetActorId()] = pathPlanWeight;

		PathingNode* currentNode = playerNode;
		for (PathingArc* playerArc : playerPathPlan)
		{
			pathPlanWeight += playerArc->GetWeight();
			if (playerArc->GetNode()->GetActorId() != INVALID_ACTOR_ID)
				if (actors.find(playerArc->GetNode()->GetActorId()) == actors.end())
					actors[playerArc->GetNode()->GetActorId()] = pathPlanWeight;

			currentNode = playerArc->GetNode();
		}

		for (PathingArcVec otherPlayerPathPlan : otherPlayerPathPlans)
		{
			float otherPathPlanWeight = 0.f;
			eastl::map<ActorId, float> otherActors;
			if (otherPlayerNode->GetActorId() != INVALID_ACTOR_ID)
				otherActors[otherPlayerNode->GetActorId()] = otherPathPlanWeight;

			currentNode = otherPlayerNode;
			for (PathingArc* otherPlayerArc : otherPlayerPathPlan)
			{
				otherPathPlanWeight += otherPlayerArc->GetWeight();
				if (otherPlayerArc->GetNode()->GetActorId() != INVALID_ACTOR_ID)
					if (otherActors.find(otherPlayerArc->GetNode()->GetActorId()) == otherActors.end())
						otherActors[otherPlayerArc->GetNode()->GetActorId()] = otherPathPlanWeight;

				currentNode = otherPlayerArc->GetNode();
			}

			bool isVisible = true, isOtherVisible = true;
			float visibleTime = 0, otherVisibleTime = 0;
			float visibleHeight = 0, otherVisibleHeight = 0;
			float visibleDistance = 0, otherVisibleDistance = 0;
			if (pathPlanWeight > otherPathPlanWeight)
			{
				Visibility(
					playerNode, playerPathPlan, otherPlayerNode, otherPlayerPathPlan,
					&isVisible, &visibleTime, &visibleDistance, &visibleHeight, 
					&isOtherVisible, &otherVisibleTime, &otherVisibleDistance, &otherVisibleHeight);
			}
			else
			{
				Visibility(
					otherPlayerNode, otherPlayerPathPlan, playerNode, playerPathPlan,
					&isOtherVisible, &otherVisibleTime, &otherVisibleDistance, &otherVisibleHeight, 
					&isVisible, &visibleTime, &visibleDistance, &visibleHeight);
			}

			//calculate damage
			NodeState playerNodeState(playerState);
			playerNodeState.isActualDamage = isVisible;
			playerNodeState.path = playerPathPlan;
			playerNodeState.node = playerPathPlan.back()->GetNode();
			mAIManager->CalculateDamage(playerNodeState, visibleTime, visibleDistance, visibleHeight);

			eastl::map<ActorId, float> pathActors;
			eastl::map<ActorId, float>::iterator itActor;
			for (itActor = actors.begin(); itActor != actors.end(); itActor++)
			{
				if (mAIManager->CanItemBeGrabbed((*itActor).first, (*itActor).second, playerState))
				{
					if (otherActors.find((*itActor).first) != otherActors.end())
					{
						if (!mAIManager->CanItemBeGrabbed((*itActor).first, (*itActor).second, otherPlayerState) ||
							otherActors[(*itActor).first] > (*itActor).second)
						{
							pathActors[(*itActor).first] = (*itActor).second;
						}
					}
					else pathActors[(*itActor).first] = (*itActor).second;
				}
			}
			mAIManager->PickupItems(playerNodeState, pathActors);

			NodeState otherPlayerNodeState(otherPlayerState);
			otherPlayerNodeState.isActualDamage = isOtherVisible;
			otherPlayerNodeState.path = otherPlayerPathPlan;
			otherPlayerNodeState.node = otherPlayerPathPlan.back()->GetNode();
			mAIManager->CalculateDamage(otherPlayerNodeState, otherVisibleTime, otherVisibleDistance, otherVisibleHeight);
			
			eastl::map<ActorId, float> otherPathActors;
			eastl::map<ActorId, float>::iterator itOtherActor;
			for (itOtherActor = otherActors.begin(); itOtherActor != otherActors.end(); itOtherActor++)
			{
				if (mAIManager->CanItemBeGrabbed((*itOtherActor).first, (*itOtherActor).second, otherPlayerState))
				{
					if (actors.find((*itOtherActor).first) != actors.end())
					{
						if (!mAIManager->CanItemBeGrabbed((*itOtherActor).first, (*itOtherActor).second, playerState) ||
							actors[(*itOtherActor).first] > (*itOtherActor).second)
						{
							otherPathActors[(*itOtherActor).first] = (*itOtherActor).second;
						}
					}
					else otherPathActors[(*itOtherActor).first] = (*itOtherActor).second;
				}
			}
			mAIManager->PickupItems(otherPlayerNodeState, otherPathActors);

			//we calculate the heuristic
			mAIManager->CalculateHeuristic(playerNodeState, otherPlayerNodeState);
			playerStates[playerPathPlan][otherPlayerPathPlan] = playerNodeState;
			otherPlayerStates[otherPlayerPathPlan][playerPathPlan] = otherPlayerNodeState;
		}
	}

	//we proceed with the minimax algorithm between players
	NodeState bestPlayerState;
	bestPlayerState.heuristic = -FLT_MAX;
	for (auto evaluatePlayerState : playerStates)
	{
		NodeState playerNodeState;
		playerNodeState.heuristic = FLT_MAX;
		for (auto evaluatePlayerNodeState : evaluatePlayerState.second)
			if (evaluatePlayerNodeState.second.heuristic < playerNodeState.heuristic)
				playerNodeState = evaluatePlayerNodeState.second;

		if (playerNodeState.heuristic > bestPlayerState.heuristic)
			bestPlayerState = playerNodeState;
	}

	//we do exactly the same for the other player
	NodeState bestOtherPlayerState;
	bestOtherPlayerState.heuristic = FLT_MAX;
	for (auto evaluateOtherPlayerState : otherPlayerStates)
	{
		NodeState otherPlayerNodeState;
		otherPlayerNodeState.heuristic = -FLT_MAX;
		for (auto evaluateOtherPlayerNodeState : evaluateOtherPlayerState.second)
			if (evaluateOtherPlayerNodeState.second.heuristic > otherPlayerNodeState.heuristic)
				otherPlayerNodeState = evaluateOtherPlayerNodeState.second;

		if (otherPlayerNodeState.heuristic < bestOtherPlayerState.heuristic)
			bestOtherPlayerState = otherPlayerNodeState;
	}

	playerState.Copy(bestPlayerState);
	otherPlayerState.Copy(bestOtherPlayerState);
}

void QuakeAIProcess::ConstructPath(NodeState& playerState,
	PathingCluster* playerCluster, eastl::vector<PathingArcVec>& playerPathPlan)
{
	PathingArcVec clusterPathPlan;
	PathingNode* currentNode = playerState.node;
	float maxPathDistance = 0.f;
	while (currentNode != playerCluster->GetTarget())
	{
		PathingCluster* currentCluster = currentNode->FindCluster(
			playerCluster->GetType(), playerCluster->GetTarget());
		PathingArc* currentArc = currentNode->FindArc(currentCluster->GetNode());
		clusterPathPlan.push_back(currentArc);

		currentNode = currentArc->GetNode();
		maxPathDistance += currentArc->GetWeight();
	}
	//add extra time
	maxPathDistance +=  1.0f;

	PathingArcVec actorPathPlan;
	mAIManager->FindPath(playerState, playerCluster, actorPathPlan, maxPathDistance);

	if (clusterPathPlan.size()) playerPathPlan.push_back(clusterPathPlan);
	if (actorPathPlan.size()) playerPathPlan.push_back(actorPathPlan);
}

void QuakeAIProcess::EvaluatePlayers(NodeState& playerState, NodeState& otherPlayerState)
{
	eastl::map<PathingCluster*, eastl::vector<PathingArcVec>> playerPathPlans, otherPlayerPathPlans;

	PathingClusterVec pathingClusters[2];
	playerState.node->GetClusters(GAT_MOVE, pathingClusters[0], 15);
	playerState.node->GetClusters(GAT_JUMP, pathingClusters[1], 50);

	PathingClusterVec playerClusters;
	for (unsigned int clusterType = 0; clusterType < 2; clusterType++)
		for (PathingCluster* pathingCluster : pathingClusters[clusterType])
			playerClusters.push_back(pathingCluster);

	fprintf(mAIManager->mFile, "\n blue player path \n");
	unsigned int clusterSize = playerClusters.size();
	for (unsigned int playerClusterIdx = 0; playerClusterIdx < clusterSize; playerClusterIdx++)
	//parallel_for(size_t(0), clusterSize, [&](size_t playerClusterIdx)
	{
		PathingCluster* playerCluster = playerClusters[playerClusterIdx];

		//construct path
		playerPathPlans[playerCluster] = eastl::vector<PathingArcVec>();
		ConstructPath(playerState, playerCluster, playerPathPlans[playerCluster]);
	}

	PathingClusterVec otherPathingClusters[2];
	otherPlayerState.node->GetClusters(GAT_MOVE, otherPathingClusters[0], 15);
	otherPlayerState.node->GetClusters(GAT_JUMP, otherPathingClusters[1], 50);

	PathingClusterVec otherPlayerClusters;
	for (unsigned int clusterType = 0; clusterType < 2; clusterType++)
		for (PathingCluster* otherPathingCluster : otherPathingClusters[clusterType])
			otherPlayerClusters.push_back(otherPathingCluster);

	fprintf(mAIManager->mFile, "\n red player path \n");
	unsigned int otherClusterSize = otherPlayerClusters.size();
	for (unsigned int otherPlayerClusterIdx = 0; otherPlayerClusterIdx < otherClusterSize; otherPlayerClusterIdx++)
	//parallel_for(size_t(0), otherClusterSize, [&](size_t otherPlayerClusterIdx)
	{
		PathingCluster* otherPlayerCluster = otherPlayerClusters[otherPlayerClusterIdx];

		//construct path
		otherPlayerPathPlans[otherPlayerCluster] = eastl::vector<PathingArcVec>();
		ConstructPath(otherPlayerState, otherPlayerCluster, otherPlayerPathPlans[otherPlayerCluster]);
	}

	//fprintf(mFile, "\n playerCluster - otherPlayerCluster \n");
	eastl::map<PathingCluster*, eastl::map<PathingCluster*, NodeState>> playerClustersStates, otherPlayerClustersStates;
	for (unsigned int playerClusterIdx = 0; playerClusterIdx < clusterSize; playerClusterIdx++)
	//parallel_for(size_t(0), clusterSize, [&](size_t playerClusterIdx)
	{
		PathingCluster* playerCluster = playerClusters[playerClusterIdx];

		for (unsigned int otherPlayerClusterIdx = 0; otherPlayerClusterIdx < otherClusterSize; otherPlayerClusterIdx++)
		//parallel_for(size_t(0), otherClusterSize, [&](size_t otherPlayerClusterIdx)
		{
			PathingCluster* otherPlayerCluster = otherPlayerClusters[otherPlayerClusterIdx];

			NodeState state(playerState);
			NodeState otherState(otherPlayerState);
			Simulation(state, playerPathPlans[playerCluster], otherState, otherPlayerPathPlans[otherPlayerCluster]);

			if (state.valid && otherState.valid)
			{
				playerClustersStates[playerCluster][otherPlayerCluster] = state;
				otherPlayerClustersStates[otherPlayerCluster][playerCluster] = otherState;
			}
		}
	}

	//minimax
	mPlayerState.Copy(playerState);
	mPlayerState.heuristic = -FLT_MAX;
	fprintf(mAIManager->mFile, "\n minimax player \n");
	for (auto playerClustersState : playerClustersStates)
	{
		fprintf(mAIManager->mFile, "\n player cluster : %u ",
			playerClustersState.first->GetTarget()->GetCluster());

		NodeState playerNodeState(playerState);
		playerNodeState.heuristic = FLT_MAX;
		for (auto playerClusterState : playerClustersState.second)
		{
			if (playerClusterState.second.weapon != WP_NONE)
			{
				fprintf(mAIManager->mFile, " weapon : %u ", playerClusterState.second.weapon);
				fprintf(mAIManager->mFile, " damage : %i ",
					playerClusterState.second.damage[playerClusterState.second.weapon - 1]);
			}

			PathingArcVec::iterator itArc = playerClusterState.second.path.begin();
			for (; itArc != playerClusterState.second.path.end(); itArc++)
			{
				//printf("%u ", (*itArc)->GetNode()->GetId());
			}

			if (!playerClusterState.second.items.empty())
			{
				fprintf(mAIManager->mFile, "other player cluster : %u ",
					playerClusterState.first->GetTarget()->GetCluster());
				fprintf(mAIManager->mFile, "heuristic : %f ", playerClusterState.second.heuristic);

				fprintf(mAIManager->mFile, " actors : ");
			}
			for (eastl::shared_ptr<Actor> pItemActor : playerClusterState.second.items)
			{
				if (pItemActor->GetType() == "Weapon")
				{
					eastl::shared_ptr<WeaponPickup> pWeaponPickup =
						pItemActor->GetComponent<WeaponPickup>(WeaponPickup::Name).lock();
					fprintf(mAIManager->mFile, "weapon %u ", pWeaponPickup->GetCode());
				}
				else if (pItemActor->GetType() == "Ammo")
				{
					eastl::shared_ptr<AmmoPickup> pAmmoPickup =
						pItemActor->GetComponent<AmmoPickup>(AmmoPickup::Name).lock();
					fprintf(mAIManager->mFile, "ammo %u ", pAmmoPickup->GetCode());
				}
				else if (pItemActor->GetType() == "Armor")
				{
					eastl::shared_ptr<ArmorPickup> pArmorPickup =
						pItemActor->GetComponent<ArmorPickup>(ArmorPickup::Name).lock();
					fprintf(mAIManager->mFile, "armor %u ", pArmorPickup->GetCode());
				}
				else if (pItemActor->GetType() == "Health")
				{
					eastl::shared_ptr<HealthPickup> pHealthPickup =
						pItemActor->GetComponent<HealthPickup>(HealthPickup::Name).lock();
					fprintf(mAIManager->mFile, "health %u ", pHealthPickup->GetCode());
				}
			}

			if (abs(playerClusterState.second.heuristic - playerNodeState.heuristic) <= GE_ROUNDING_ERROR)
			{
				if (playerNodeState.weapon != WP_NONE && playerClusterState.second.weapon != WP_NONE)
				{
					if (playerClusterState.second.damage[playerClusterState.second.weapon - 1] >
						playerNodeState.damage[playerNodeState.weapon - 1])
					{
						playerNodeState = playerClusterState.second;
					}
				}
				else if (playerClusterState.second.weapon != WP_NONE)
				{
					playerNodeState = playerClusterState.second;
				}
			}
			else if (playerClusterState.second.heuristic < playerNodeState.heuristic)
			{
				playerNodeState = playerClusterState.second;
			}
		}
		if (playerNodeState.valid)
		{
			fprintf(mAIManager->mFile, "\n min heuristic : %f ", playerNodeState.heuristic);
			if (playerNodeState.weapon != WP_NONE)
			{
				fprintf(mAIManager->mFile, " weapon : %u ", playerNodeState.weapon);
				fprintf(mAIManager->mFile, " damage : %i ", playerNodeState.damage[playerNodeState.weapon - 1]);
			}
			else
			{
				fprintf(mAIManager->mFile, " weapon : 0 ");
				fprintf(mAIManager->mFile, " damage : 0 ");
			}

			PathingArcVec::iterator itArc = playerNodeState.path.begin();
			for (; itArc != playerNodeState.path.end(); itArc++)
			{
				//printf("%u ", (*itArc)->GetNode()->GetId());
			}

			if (!playerNodeState.items.empty()) fprintf(mAIManager->mFile, " actors : ");
			for (eastl::shared_ptr<Actor> pItemActor : playerNodeState.items)
			{
				if (pItemActor->GetType() == "Weapon")
				{
					eastl::shared_ptr<WeaponPickup> pWeaponPickup =
						pItemActor->GetComponent<WeaponPickup>(WeaponPickup::Name).lock();
					fprintf(mAIManager->mFile, "weapon %u ", pWeaponPickup->GetCode());
				}
				else if (pItemActor->GetType() == "Ammo")
				{
					eastl::shared_ptr<AmmoPickup> pAmmoPickup =
						pItemActor->GetComponent<AmmoPickup>(AmmoPickup::Name).lock();
					fprintf(mAIManager->mFile, "ammo %u ", pAmmoPickup->GetCode());
				}
				else if (pItemActor->GetType() == "Armor")
				{
					eastl::shared_ptr<ArmorPickup> pArmorPickup =
						pItemActor->GetComponent<ArmorPickup>(ArmorPickup::Name).lock();
					fprintf(mAIManager->mFile, "armor %u ", pArmorPickup->GetCode());
				}
				else if (pItemActor->GetType() == "Health")
				{
					eastl::shared_ptr<HealthPickup> pHealthPickup =
						pItemActor->GetComponent<HealthPickup>(HealthPickup::Name).lock();
					fprintf(mAIManager->mFile, "health %u ", pHealthPickup->GetCode());
				}
			}
		}

		if (abs(playerNodeState.heuristic - mPlayerState.heuristic) <= GE_ROUNDING_ERROR)
		{
			if (mPlayerState.weapon != WP_NONE && playerNodeState.weapon != WP_NONE)
			{
				if (playerNodeState.damage[playerNodeState.weapon - 1] >
					mPlayerState.damage[mPlayerState.weapon - 1])
				{
					mPlayerState.Copy(playerNodeState);
				}
			}
			else if (playerNodeState.weapon != WP_NONE)
			{
				mPlayerState.Copy(playerNodeState);
			}
		}
		else if (playerNodeState.heuristic > mPlayerState.heuristic)
		{
			mPlayerState.Copy(playerNodeState);
		}
	}
	if (mPlayerState.valid)
	{
		fprintf(mAIManager->mFile, "\n max heuristic : %f ", mPlayerState.heuristic);
		if (mPlayerState.weapon != WP_NONE)
		{
			fprintf(mAIManager->mFile, " weapon : %u ", mPlayerState.weapon);
			fprintf(mAIManager->mFile, " damage : %i ", mPlayerState.damage[mPlayerState.weapon - 1]);
		}
		else
		{
			fprintf(mAIManager->mFile, " weapon : 0 ");
			fprintf(mAIManager->mFile, " damage : 0 ");
		}

		PathingArcVec::iterator itArc = mPlayerState.path.begin();
		for (; itArc != mPlayerState.path.end(); itArc++)
		{
			//printf("%u ", (*itArc)->GetNode()->GetId());
		}

		if (!mPlayerState.items.empty()) fprintf(mAIManager->mFile, " actors : ");
		for (eastl::shared_ptr<Actor> pItemActor : mPlayerState.items)
		{
			if (pItemActor->GetType() == "Weapon")
			{
				eastl::shared_ptr<WeaponPickup> pWeaponPickup =
					pItemActor->GetComponent<WeaponPickup>(WeaponPickup::Name).lock();
				fprintf(mAIManager->mFile, "weapon %u ", pWeaponPickup->GetCode());
			}
			else if (pItemActor->GetType() == "Ammo")
			{
				eastl::shared_ptr<AmmoPickup> pAmmoPickup =
					pItemActor->GetComponent<AmmoPickup>(AmmoPickup::Name).lock();
				fprintf(mAIManager->mFile, "ammo %u ", pAmmoPickup->GetCode());
			}
			else if (pItemActor->GetType() == "Armor")
			{
				eastl::shared_ptr<ArmorPickup> pArmorPickup =
					pItemActor->GetComponent<ArmorPickup>(ArmorPickup::Name).lock();
				fprintf(mAIManager->mFile, "armor %u ", pArmorPickup->GetCode());
			}
			else if (pItemActor->GetType() == "Health")
			{
				eastl::shared_ptr<HealthPickup> pHealthPickup =
					pItemActor->GetComponent<HealthPickup>(HealthPickup::Name).lock();
				fprintf(mAIManager->mFile, "health %u ", pHealthPickup->GetCode());
			}
		}
	}

	mOtherPlayerState.Copy(otherPlayerState);
	mOtherPlayerState.heuristic = FLT_MAX;
	fprintf(mAIManager->mFile, "\n minimax otherPlayer \n");
	for (auto otherPlayerClustersState : otherPlayerClustersStates)
	{
		fprintf(mAIManager->mFile, "\n other player cluster : %u ",
			otherPlayerClustersState.first->GetTarget()->GetCluster());

		NodeState otherPlayerNodeState(otherPlayerState);
		otherPlayerNodeState.heuristic = -FLT_MAX;
		for (auto otherPlayerClusterState : otherPlayerClustersState.second)
		{
			if (otherPlayerClusterState.second.weapon != WP_NONE)
			{
				fprintf(mAIManager->mFile, " weapon : %u ", otherPlayerClusterState.second.weapon);
				fprintf(mAIManager->mFile, " damage : %i ",
					otherPlayerClusterState.second.damage[
					otherPlayerClusterState.second.weapon - 1]);
			}

			//printf("nodes : ");
			PathingArcVec::iterator itArc = otherPlayerClusterState.second.path.begin();
			for (; itArc != otherPlayerClusterState.second.path.end(); itArc++)
			{
				//printf("%u ", (*itArc)->GetNode()->GetId());
			}

			if (!otherPlayerClusterState.second.items.empty())
			{
				fprintf(mAIManager->mFile, "player cluster : %u ",
					otherPlayerClusterState.first->GetTarget()->GetCluster());
				fprintf(mAIManager->mFile, "heuristic : %f ", otherPlayerClusterState.second.heuristic);


				fprintf(mAIManager->mFile, " actors : ");
			}
			for (eastl::shared_ptr<Actor> pItemActor : otherPlayerClusterState.second.items)
			{
				if (pItemActor->GetType() == "Weapon")
				{
					eastl::shared_ptr<WeaponPickup> pWeaponPickup =
						pItemActor->GetComponent<WeaponPickup>(WeaponPickup::Name).lock();
					fprintf(mAIManager->mFile, "weapon %u ", pWeaponPickup->GetCode());
				}
				else if (pItemActor->GetType() == "Ammo")
				{
					eastl::shared_ptr<AmmoPickup> pAmmoPickup =
						pItemActor->GetComponent<AmmoPickup>(AmmoPickup::Name).lock();
					fprintf(mAIManager->mFile, "ammo %u ", pAmmoPickup->GetCode());
				}
				else if (pItemActor->GetType() == "Armor")
				{
					eastl::shared_ptr<ArmorPickup> pArmorPickup =
						pItemActor->GetComponent<ArmorPickup>(ArmorPickup::Name).lock();
					fprintf(mAIManager->mFile, "armor %u ", pArmorPickup->GetCode());
				}
				else if (pItemActor->GetType() == "Health")
				{
					eastl::shared_ptr<HealthPickup> pHealthPickup =
						pItemActor->GetComponent<HealthPickup>(HealthPickup::Name).lock();
					fprintf(mAIManager->mFile, "health %u ", pHealthPickup->GetCode());
				}
			}

			if (abs(otherPlayerClusterState.second.heuristic - otherPlayerNodeState.heuristic) <= GE_ROUNDING_ERROR)
			{
				if (otherPlayerNodeState.weapon != WP_NONE && otherPlayerClusterState.second.weapon != WP_NONE)
				{
					if (otherPlayerClusterState.second.damage[otherPlayerClusterState.second.weapon - 1] >
						otherPlayerNodeState.damage[otherPlayerNodeState.weapon - 1])
					{
						otherPlayerNodeState = otherPlayerClusterState.second;
					}
				}
				else if (otherPlayerClusterState.second.weapon != WP_NONE)
				{
					otherPlayerNodeState = otherPlayerClusterState.second;
				}
			}
			else if (otherPlayerClusterState.second.heuristic > otherPlayerNodeState.heuristic)
			{
				otherPlayerNodeState = otherPlayerClusterState.second;
			}
		}

		if (otherPlayerNodeState.valid)
		{
			fprintf(mAIManager->mFile, "\n max heuristic : %f ", otherPlayerNodeState.heuristic);
			if (otherPlayerNodeState.weapon != WP_NONE)
			{
				fprintf(mAIManager->mFile, " weapon : %u ", otherPlayerNodeState.weapon);
				fprintf(mAIManager->mFile, " damage : %i ",
					otherPlayerNodeState.damage[otherPlayerNodeState.weapon - 1]);
			}
			else
			{
				fprintf(mAIManager->mFile, " weapon : 0 ");
				fprintf(mAIManager->mFile, " damage : 0 ");
			}

			//printf("nodes : ");
			PathingArcVec::iterator itArc = otherPlayerNodeState.path.begin();
			for (; itArc != otherPlayerNodeState.path.end(); itArc++)
			{
				//printf("%u ", (*itArc)->GetNode()->GetId());
			}

			if (!otherPlayerNodeState.items.empty()) fprintf(mAIManager->mFile, " actors : ");
			for (eastl::shared_ptr<Actor> pItemActor : otherPlayerNodeState.items)
			{
				if (pItemActor->GetType() == "Weapon")
				{
					eastl::shared_ptr<WeaponPickup> pWeaponPickup =
						pItemActor->GetComponent<WeaponPickup>(WeaponPickup::Name).lock();
					fprintf(mAIManager->mFile, "weapon %u ", pWeaponPickup->GetCode());
				}
				else if (pItemActor->GetType() == "Ammo")
				{
					eastl::shared_ptr<AmmoPickup> pAmmoPickup =
						pItemActor->GetComponent<AmmoPickup>(AmmoPickup::Name).lock();
					fprintf(mAIManager->mFile, "ammo %u ", pAmmoPickup->GetCode());
				}
				else if (pItemActor->GetType() == "Armor")
				{
					eastl::shared_ptr<ArmorPickup> pArmorPickup =
						pItemActor->GetComponent<ArmorPickup>(ArmorPickup::Name).lock();
					fprintf(mAIManager->mFile, "armor %u ", pArmorPickup->GetCode());
				}
				else if (pItemActor->GetType() == "Health")
				{
					eastl::shared_ptr<HealthPickup> pHealthPickup =
						pItemActor->GetComponent<HealthPickup>(HealthPickup::Name).lock();
					fprintf(mAIManager->mFile, "health %u ", pHealthPickup->GetCode());
				}
			}
		}

		if (abs(otherPlayerNodeState.heuristic - mOtherPlayerState.heuristic) <= GE_ROUNDING_ERROR)
		{
			if (mOtherPlayerState.weapon != WP_NONE && otherPlayerNodeState.weapon != WP_NONE)
			{
				if (otherPlayerNodeState.damage[otherPlayerNodeState.weapon - 1] >
					mOtherPlayerState.damage[mOtherPlayerState.weapon - 1])
				{
					mOtherPlayerState.Copy(otherPlayerNodeState);
				}
			}
			else if (otherPlayerNodeState.weapon != WP_NONE)
			{
				mOtherPlayerState.Copy(otherPlayerNodeState);
			}
		}
		else if (otherPlayerNodeState.heuristic < mOtherPlayerState.heuristic)
		{
			mOtherPlayerState.Copy(otherPlayerNodeState);
		}
	}
	if (mOtherPlayerState.valid)
	{
		fprintf(mAIManager->mFile, "\n min heuristic : %f ", mOtherPlayerState.heuristic);
		if (mOtherPlayerState.weapon != WP_NONE)
		{
			fprintf(mAIManager->mFile, " weapon : %u ", mOtherPlayerState.weapon);
			fprintf(mAIManager->mFile, " damage : %i ",
				mOtherPlayerState.damage[mOtherPlayerState.weapon - 1]);
		}
		else
		{
			fprintf(mAIManager->mFile, " weapon : 0 ");
			fprintf(mAIManager->mFile, " damage : 0 ");
		}

		//printf("nodes : ");
		PathingArcVec::iterator itArc = mOtherPlayerState.path.begin();
		for (; itArc != mOtherPlayerState.path.end(); itArc++)
		{
			//printf("%u ", (*itArc)->GetNode()->GetId());
		}

		if (!mOtherPlayerState.items.empty()) fprintf(mAIManager->mFile, " actors : ");
		for (eastl::shared_ptr<Actor> pItemActor : mOtherPlayerState.items)
		{
			if (pItemActor->GetType() == "Weapon")
			{
				eastl::shared_ptr<WeaponPickup> pWeaponPickup =
					pItemActor->GetComponent<WeaponPickup>(WeaponPickup::Name).lock();
				fprintf(mAIManager->mFile, "weapon %u ", pWeaponPickup->GetCode());
			}
			else if (pItemActor->GetType() == "Ammo")
			{
				eastl::shared_ptr<AmmoPickup> pAmmoPickup =
					pItemActor->GetComponent<AmmoPickup>(AmmoPickup::Name).lock();
				fprintf(mAIManager->mFile, "ammo %u ", pAmmoPickup->GetCode());
			}
			else if (pItemActor->GetType() == "Armor")
			{
				eastl::shared_ptr<ArmorPickup> pArmorPickup =
					pItemActor->GetComponent<ArmorPickup>(ArmorPickup::Name).lock();
				fprintf(mAIManager->mFile, "armor %u ", pArmorPickup->GetCode());
			}
			else if (pItemActor->GetType() == "Health")
			{
				eastl::shared_ptr<HealthPickup> pHealthPickup =
					pItemActor->GetComponent<HealthPickup>(HealthPickup::Name).lock();
				fprintf(mAIManager->mFile, "health %u ", pHealthPickup->GetCode());
			}
		}
	}
}

void QuakeAIProcess::ThreadProc( )
{
	unsigned int iteration = 0;

	while (true)
	{
		if (GameLogic::Get()->GetState() == BGS_RUNNING)
		{
			eastl::map<GameViewType, eastl::vector<ActorId>> players;

			GameApplication* gameApp = (GameApplication*)Application::App;
			const GameViewList& gameViews = gameApp->GetGameViews();
			for (auto it = gameViews.begin(); it != gameViews.end(); ++it)
			{
				eastl::shared_ptr<BaseGameView> pView = *it;
				if (pView->GetActorId() != INVALID_ACTOR_ID)
					players[pView->GetType()].push_back(pView->GetActorId());
			}

			if (players.find(GV_HUMAN) != players.end() && players.find(GV_AI) != players.end())
			{
				eastl::vector<ActorId>::iterator itAIPlayer;
				for (itAIPlayer = players[GV_AI].begin(); itAIPlayer != players[GV_AI].end(); )
				{
					bool removeAIPlayer = false;
					for (ActorId player : players[GV_HUMAN])
						if (player == (*itAIPlayer))
							removeAIPlayer = true;

					if (removeAIPlayer)
						itAIPlayer = players[GV_AI].erase(itAIPlayer);
					else
						itAIPlayer++;
				}

				iteration++;
				printf("\n ITERATION %u \n", iteration);
				fprintf(mAIManager->mFile, "\n\n ITERATION %u \n\n", iteration);

				fprintf(mAIManager->mFile, "\n blue player ai guessing (red)");
				for (ActorId player : players[GV_HUMAN])
				{
					eastl::shared_ptr<PlayerActor> pHumanPlayer =
						eastl::dynamic_shared_pointer_cast<PlayerActor>(
						GameLogic::Get()->GetActor(player).lock());
					eastl::shared_ptr<TransformComponent> pTransformComponent(
						pHumanPlayer->GetComponent<TransformComponent>(TransformComponent::Name).lock());

					NodeState playerState(pHumanPlayer);
					playerState.node =
						mAIManager->GetPathingGraph()->FindClosestNode(pTransformComponent->GetPosition());
					for (ActorId aiPlayer : players[GV_AI])
					{
						NodeState aiPlayerState;
						mAIManager->GetPlayerGuessState(aiPlayer, aiPlayerState);
						aiPlayerState.node = mAIManager->GetPlayerGuessNode(aiPlayer);
						aiPlayerState.ResetItems();

						mAIManager->SetExcludeActors(aiPlayerState.player);
						EvaluatePlayers(playerState, aiPlayerState);
					}
				}

				/*
				printf("\n blue player pos %f %f %f, id %u, heuristic %f, target %u, weapon %u, damage %u, paths %u \n",
				mPlayerState.node->GetPos()[0], mPlayerState.node->GetPos()[1], mPlayerState.node->GetPos()[2],
				mPlayerState.node->GetId(), mPlayerState.heuristic, mPlayerState.target, mPlayerState.weapon,
				mPlayerState.weapon > 0 ? mPlayerState.damage[mPlayerState.weapon - 1] : 0, mPlayerState.path.size());
				*/
				//printf("\n blue player nodes %u : ", mPlayerState.player);
				PathingArcVec::iterator itArc = mPlayerState.path.begin();
				for (; itArc != mPlayerState.path.end(); itArc++)
				{
					//printf("%u ", (*itArc)->GetNode()->GetId());
				}

				fprintf(mAIManager->mFile, "\n\n blue player   %f", mPlayerState.heuristic);
				//printf("\n blue player actors  %u : ", mPlayerState.player);
				if (!mPlayerState.items.empty()) fprintf(mAIManager->mFile, " actors : ");
				for (eastl::shared_ptr<Actor> pItemActor : mPlayerState.items)
				{
					if (pItemActor->GetType() == "Weapon")
					{
						eastl::shared_ptr<WeaponPickup> pWeaponPickup =
							pItemActor->GetComponent<WeaponPickup>(WeaponPickup::Name).lock();
						fprintf(mAIManager->mFile, "weapon %u ", pWeaponPickup->GetCode());
						//printf("weapon %u ", pWeaponPickup->GetCode());
					}
					else if (pItemActor->GetType() == "Ammo")
					{
						eastl::shared_ptr<AmmoPickup> pAmmoPickup =
							pItemActor->GetComponent<AmmoPickup>(AmmoPickup::Name).lock();
						fprintf(mAIManager->mFile, "ammo %u ", pAmmoPickup->GetCode());
						//printf("ammo %u ", pAmmoPickup->GetCode());
					}
					else if (pItemActor->GetType() == "Armor")
					{
						eastl::shared_ptr<ArmorPickup> pArmorPickup =
							pItemActor->GetComponent<ArmorPickup>(ArmorPickup::Name).lock();
						fprintf(mAIManager->mFile, "armor %u ", pArmorPickup->GetCode());
						//printf("armor %u ", pArmorPickup->GetCode());
					}
					else if (pItemActor->GetType() == "Health")
					{
						eastl::shared_ptr<HealthPickup> pHealthPickup =
							pItemActor->GetComponent<HealthPickup>(HealthPickup::Name).lock();
						fprintf(mAIManager->mFile, "health %u ", pHealthPickup->GetCode());
						//printf("health %u ", pHealthPickup->GetCode());
					}
				}
				if (mPlayerState.weapon != WP_NONE)
				{
					fprintf(mAIManager->mFile, " weapon : %u ", mPlayerState.weapon);
					fprintf(mAIManager->mFile, " damage : %i ", mPlayerState.damage[mPlayerState.weapon - 1]);
				}
				else
				{
					fprintf(mAIManager->mFile, " weapon : 0 ");
					fprintf(mAIManager->mFile, " damage : 0 ");
				}

				NodeState otherPlayerGuessState; 
				mAIManager->GetPlayerGuessState(mOtherPlayerState.player, otherPlayerGuessState);
				otherPlayerGuessState.CopyItems(mOtherPlayerState);
				mAIManager->SetPlayerGuessState(mOtherPlayerState.player, otherPlayerGuessState);
				mAIManager->SetPlayerGuessPath(mOtherPlayerState.player, mOtherPlayerState.path);
				mAIManager->SetPlayerGuessUpdated(mOtherPlayerState.player, true);

				mAIManager->SetPlayerTarget(mPlayerState.player, mPlayerState.target);
				mAIManager->SetPlayerWeapon(mPlayerState.player, mPlayerState.weapon);
				mAIManager->SetPlayerState(mPlayerState.player, mPlayerState);
				mAIManager->SetPlayerUpdated(mPlayerState.player, true);

				fprintf(mAIManager->mFile, "\n\n red player ai guessing (blue)");
				for (ActorId aiPlayer : players[GV_AI])
				{
					eastl::shared_ptr<PlayerActor> pAIPlayer =
						eastl::dynamic_shared_pointer_cast<PlayerActor>(
						GameLogic::Get()->GetActor(aiPlayer).lock());
					eastl::shared_ptr<TransformComponent> pTransformComponent(
						pAIPlayer->GetComponent<TransformComponent>(TransformComponent::Name).lock());

					NodeState aiPlayerState(pAIPlayer);
					aiPlayerState.node =
						mAIManager->GetPathingGraph()->FindClosestNode(pTransformComponent->GetPosition());
					for (ActorId player : players[GV_HUMAN])
					{
						NodeState playerState;
						mAIManager->GetPlayerGuessState(player, playerState);
						playerState.node = mAIManager->GetPlayerGuessNode(player);
						playerState.ResetItems();

						mAIManager->SetExcludeActors(playerState.player);
						EvaluatePlayers(playerState, aiPlayerState);
					}
				}

				/*
				printf("\n red player pos %f %f %f, id %u, heuristic %f, target %u, weapon %u, damage %u, paths %u \n",
				mOtherPlayerState.node->GetPos()[0], mOtherPlayerState.node->GetPos()[1], mOtherPlayerState.node->GetPos()[2],
				mOtherPlayerState.node->GetId(), mOtherPlayerState.heuristic, mOtherPlayerState.target, mOtherPlayerState.weapon,
				mOtherPlayerState.weapon > 0 ? mOtherPlayerState.damage[mOtherPlayerState.weapon - 1] : 0, mOtherPlayerState.path.size());
				*/
				//printf("\n red player nodes %u : ", mOtherPlayerState.player);
				itArc = mOtherPlayerState.path.begin();
				for (; itArc != mOtherPlayerState.path.end(); itArc++)
				{
					//printf("%u ", (*itArc)->GetNode()->GetId());
				}

				fprintf(mAIManager->mFile, "\n red player   %f", mOtherPlayerState.heuristic);
				//printf("\n red player actors %u : ", mOtherPlayerState.player);
				if (!mOtherPlayerState.items.empty()) fprintf(mAIManager->mFile, " actors : ");
				for (eastl::shared_ptr<Actor> pItemActor : mOtherPlayerState.items)
				{
					if (pItemActor->GetType() == "Weapon")
					{
						eastl::shared_ptr<WeaponPickup> pWeaponPickup =
							pItemActor->GetComponent<WeaponPickup>(WeaponPickup::Name).lock();
						fprintf(mAIManager->mFile, "weapon %u ", pWeaponPickup->GetCode());
						//printf("weapon %u ", pWeaponPickup->GetCode());
					}
					else if (pItemActor->GetType() == "Ammo")
					{
						eastl::shared_ptr<AmmoPickup> pAmmoPickup =
							pItemActor->GetComponent<AmmoPickup>(AmmoPickup::Name).lock();
						fprintf(mAIManager->mFile, "ammo %u ", pAmmoPickup->GetCode());
						//printf("ammo %u ", pAmmoPickup->GetCode());
					}
					else if (pItemActor->GetType() == "Armor")
					{
						eastl::shared_ptr<ArmorPickup> pArmorPickup =
							pItemActor->GetComponent<ArmorPickup>(ArmorPickup::Name).lock();
						fprintf(mAIManager->mFile, "armor %u ", pArmorPickup->GetCode());
						//printf("armor %u ", pArmorPickup->GetCode());
					}
					else if (pItemActor->GetType() == "Health")
					{
						eastl::shared_ptr<HealthPickup> pHealthPickup =
							pItemActor->GetComponent<HealthPickup>(HealthPickup::Name).lock();
						fprintf(mAIManager->mFile, "health %u ", pHealthPickup->GetCode());
						//printf("health %u ", pHealthPickup->GetCode());
					}
				}
				if (mOtherPlayerState.weapon != WP_NONE)
				{
					fprintf(mAIManager->mFile, " weapon : %u ", mOtherPlayerState.weapon);
					fprintf(mAIManager->mFile, " damage : %i ", mOtherPlayerState.damage[mOtherPlayerState.weapon - 1]);
				}
				else
				{
					fprintf(mAIManager->mFile, " weapon : 0 ");
					fprintf(mAIManager->mFile, " damage : 0 ");
				}

				NodeState playerGuessState;
				mAIManager->GetPlayerGuessState(mPlayerState.player, playerGuessState);
				playerGuessState.CopyItems(mPlayerState);
				mAIManager->SetPlayerGuessState(mPlayerState.player, playerGuessState);
				mAIManager->SetPlayerGuessPath(mPlayerState.player, mPlayerState.path);
				mAIManager->SetPlayerGuessUpdated(mPlayerState.player, true);

				mAIManager->SetPlayerTarget(mOtherPlayerState.player, mOtherPlayerState.target);
				mAIManager->SetPlayerWeapon(mOtherPlayerState.player, mOtherPlayerState.weapon);
				mAIManager->SetPlayerState(mOtherPlayerState.player, mOtherPlayerState);
				mAIManager->SetPlayerUpdated(mOtherPlayerState.player, true);
			}

			mAIManager->SetEnable(true);
		}
	}

	Succeed();
}