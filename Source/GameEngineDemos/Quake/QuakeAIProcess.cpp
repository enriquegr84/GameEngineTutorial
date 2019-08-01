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

QuakeAIProcess::QuakeAIProcess() : RealtimeProcess()
{
	mAIManager = dynamic_cast<QuakeAIManager*>(GameLogic::Get()->GetAIManager());
}

QuakeAIProcess::~QuakeAIProcess(void)
{

}

void QuakeAIProcess::Visibility(
	PathingNode* playerNode, PathingArcVec& playerPathPlan, 
	PathingNode* otherPlayerNode, PathingArcVec& otherPlayerPathPlan,
	float* visibleTime, float* visibleDistance, float* visibleHeight,
	float* otherVisibleTime, float* otherVisibleDistance, float* otherVisibleHeight)
{
	float totalTime = 0.f, totalArcTime = 0.f;
	unsigned int index = 0, otherIndex = 0, otherPathIndex = 0;

	PathingNode* currentNode = playerNode;
	PathingTransition* currentTransition = NULL;
	PathingNodeVec transitionNodes;

	PathingNode* otherCurrentNode = otherPlayerNode;
	PathingTransition* otherCurrentTransition =
		otherCurrentNode->FindTransition(otherPlayerPathPlan[otherPathIndex]->GetId());
	PathingNodeVec otherTransitionNodes = otherCurrentTransition->GetNodes();

	//lets calculate the visibility for simultaneous path travelling.
	for (PathingArc* currentArc : playerPathPlan)
	{
		index = 0;
		currentTransition = currentNode->FindTransition(currentArc->GetId());
		transitionNodes = currentTransition->GetNodes();
		for (; index < transitionNodes.size(); index++)
		{
			float visibleWeight = currentTransition->GetWeights()[index];
			if (mAIManager->GetPathingGraph()->IsVisibleCluster(
				transitionNodes[index]->GetCluster(), otherTransitionNodes[otherIndex]->GetCluster()))
			{
				if (transitionNodes[index]->IsVisibleNode(otherTransitionNodes[otherIndex]))
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
				if (otherIndex + 1 >= otherTransitionNodes.size())
				{
					if (otherPathIndex + 1 < otherPlayerPathPlan.size())
					{
						otherIndex = 0;
						otherCurrentNode = otherPlayerPathPlan[otherPathIndex]->GetNode();

						otherPathIndex++;
						otherCurrentTransition = otherCurrentNode->FindTransition(
							otherPlayerPathPlan[otherPathIndex]->GetId());
						otherTransitionNodes = otherCurrentTransition->GetNodes();
					}
					else break;
				}
				else otherIndex++;
			}
			totalTime += visibleWeight;
		}
		currentNode = currentArc->GetNode();
	}

	//lets put a minimum of total time
	if (totalTime < 3.0f)
	{
		float visibleWeight = 3.0f - totalTime;
		totalTime = 3.0f;

		if (mAIManager->GetPathingGraph()->IsVisibleCluster(
			playerPathPlan.back()->GetNode()->GetCluster(),
			otherPlayerPathPlan.back()->GetNode()->GetCluster()))
		{
			if (playerPathPlan.back()->GetNode()->IsVisibleNode(
				otherPlayerPathPlan.back()->GetNode()))
			{
				(*visibleDistance) += Length(
					otherPlayerPathPlan.back()->GetNode()->GetPos() -
					playerPathPlan.back()->GetNode()->GetPos()) * visibleWeight;
				(*visibleHeight) +=
					(playerPathPlan.back()->GetNode()->GetPos()[2] -
					otherPlayerPathPlan.back()->GetNode()->GetPos()[2]) * visibleWeight;
				(*visibleTime) += visibleWeight;

				(*otherVisibleDistance) += Length(
					otherPlayerPathPlan.back()->GetNode()->GetPos() -
					playerPathPlan.back()->GetNode()->GetPos()) * visibleWeight;
				(*otherVisibleHeight) +=
					(otherPlayerPathPlan.back()->GetNode()->GetPos()[2] -
					playerPathPlan.back()->GetNode()->GetPos()[2]) * visibleWeight;
				(*otherVisibleTime) += visibleWeight;
			}
		}
	}

	//we add extra time for potential visibility if the ending clusters are visible
	if (mAIManager->GetPathingGraph()->IsVisibleCluster(
		playerPathPlan.back()->GetNode()->GetCluster(),
		otherPlayerPathPlan.back()->GetNode()->GetCluster()))
	{
		if (playerPathPlan.back()->GetNode()->IsVisibleNode(
			otherPlayerPathPlan.back()->GetNode()))
		{
			float visibleWeight = 1.0f;
			totalTime += visibleWeight;

			(*visibleDistance) += Length(
				otherPlayerPathPlan.back()->GetNode()->GetPos() -
				playerPathPlan.back()->GetNode()->GetPos()) * visibleWeight;
			(*visibleHeight) +=
				(playerPathPlan.back()->GetNode()->GetPos()[2] -
				otherPlayerPathPlan.back()->GetNode()->GetPos()[2]) * visibleWeight;
			(*visibleTime) += visibleWeight;

			(*otherVisibleDistance) += Length(
				otherPlayerPathPlan.back()->GetNode()->GetPos() -
				playerPathPlan.back()->GetNode()->GetPos()) * visibleWeight;
			(*otherVisibleHeight) +=
				(otherPlayerPathPlan.back()->GetNode()->GetPos()[2] -
				playerPathPlan.back()->GetNode()->GetPos()[2]) * visibleWeight;
			(*otherVisibleTime) += visibleWeight;
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

void QuakeAIProcess::Simulation(
	NodeState& playerState, PathingArcVec& playerPathPlan,
	NodeState& otherPlayerState, PathingArcVec& otherPlayerPathPlan)
{
	PathingNode* playerNode = playerState.plan.node;
	PathingNode* otherPlayerNode = otherPlayerState.plan.node;
	eastl::map<PathingArcVec, eastl::map<PathingArcVec, NodeState>> playerStates, otherPlayerStates;

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

	float visibleTime = 0, otherVisibleTime = 0;
	float visibleHeight = 0, otherVisibleHeight = 0;
	float visibleDistance = 0, otherVisibleDistance = 0;
	if (pathPlanWeight > otherPathPlanWeight)
	{
		Visibility(
			playerNode, playerPathPlan, otherPlayerNode, otherPlayerPathPlan,
			&visibleTime, &visibleDistance, &visibleHeight, 
			&otherVisibleTime, &otherVisibleDistance, &otherVisibleHeight);
	}
	else
	{
		Visibility(
			otherPlayerNode, otherPlayerPathPlan, playerNode, playerPathPlan,
			&otherVisibleTime, &otherVisibleDistance, &otherVisibleHeight, 
			&visibleTime, &visibleDistance, &visibleHeight);
	}

	//calculate damage
	NodeState playerNodeState(playerState);
	playerNodeState.plan.path = playerPathPlan;
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
	mAIManager->PickupItems(playerNodeState, pathActors, mExcludeActors);

	NodeState otherPlayerNodeState(otherPlayerState);
	otherPlayerNodeState.plan.path = otherPlayerPathPlan;
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
	mAIManager->PickupItems(otherPlayerNodeState, otherPathActors, mExcludeActors);

	//we calculate the heuristic
	mAIManager->CalculateHeuristic(playerNodeState, otherPlayerNodeState);
	playerState.Copy(playerNodeState);
	otherPlayerState.Copy(otherPlayerNodeState);
}

void QuakeAIProcess::ConstructPath(NodeState& playerState,
	PathingCluster* playerCluster, PathingArcVec& playerPathPlan)
{
	PathingNode* currentNode = playerState.plan.node;
	while (currentNode != playerCluster->GetTarget())
	{
		PathingCluster* currentCluster = currentNode->FindCluster(
			playerCluster->GetType(), playerCluster->GetTarget());
		PathingArc* currentArc = currentNode->FindArc(currentCluster->GetNode());
		playerPathPlan.push_back(currentArc);

		currentNode = currentArc->GetNode();
	}
}

void QuakeAIProcess::ConstructActorPath(NodeState& playerState,
	PathingCluster* playerCluster, PathingArcVec& playerActorPlan)
{
	float maxPathWeight = 0.f;
	PathingNode* currentNode = playerState.plan.node;
	while (currentNode != playerCluster->GetTarget())
	{
		PathingCluster* currentCluster = currentNode->FindCluster(
			playerCluster->GetType(), playerCluster->GetTarget());
		PathingArc* currentArc = currentNode->FindArc(currentCluster->GetNode());

		currentNode = currentArc->GetNode();
		maxPathWeight += currentArc->GetWeight();
	}
	//add extra time
	maxPathWeight += maxPathWeight * 0.3f;
	mAIManager->FindPath(playerState, playerCluster, playerActorPlan, mExcludeActors, maxPathWeight);
}

void QuakeAIProcess::EvaluatePlayers(NodeState& playerState, NodeState& otherPlayerState)
{
	eastl::map<PathingCluster*, PathingArcVec> playerPathPlans, otherPlayerPathPlans;

	//search player surrounding clusters
	PathingClusterVec playerClusters;
	playerState.plan.node->GetClusters(GAT_JUMP, playerClusters, 60);

	/*
		player path construction in two phases:
		- actor path planning
		- path planning
	*/
	unsigned int playerClusterIdx = 0;
	unsigned int clusterSize = playerClusters.size();
	//fprintf(mAIManager->mLogInformation, "\n blue player actors ");
	//construct path based on closest actors to each cluster pathway
	for (; playerClusterIdx < clusterSize; playerClusterIdx++)
	{
		PathingCluster* playerCluster = playerClusters[playerClusterIdx];

		PathingArcVec playerActorPlan;
		ConstructActorPath(playerState, playerCluster, playerActorPlan);
		if (!playerActorPlan.empty())
		{
			//construct path
			playerPathPlans[playerCluster] = playerActorPlan;
		}
	}

	//construct path based on cluster pathway
	for (playerClusterIdx = 0; playerClusterIdx < clusterSize; playerClusterIdx++)
	{
		PathingCluster* playerCluster = playerClusters[playerClusterIdx];
		if (playerPathPlans.find(playerCluster) != playerPathPlans.end())
			continue;

		PathingArcVec playerPathPlan;
		ConstructPath(playerState, playerCluster, playerPathPlan);
		if (!playerPathPlan.empty())
		{
			//construct path
			playerPathPlans[playerCluster] = playerPathPlan;
		}
	}

	//search other player surrounding clusters
	PathingClusterVec otherPlayerClusters;
	otherPlayerState.plan.node->GetClusters(GAT_JUMP, otherPlayerClusters, 60);

	//other player path construction
	unsigned int otherPlayerClusterIdx = 0;
	unsigned int otherClusterSize = otherPlayerClusters.size();
	//fprintf(mAIManager->mLogInformation, "\n red player actors ");
	for (; otherPlayerClusterIdx < otherClusterSize; otherPlayerClusterIdx++)
	{
		PathingCluster* otherPlayerCluster = otherPlayerClusters[otherPlayerClusterIdx];

		PathingArcVec otherPlayerActorPlan;
		ConstructActorPath(otherPlayerState, otherPlayerCluster, otherPlayerActorPlan);
		if (!otherPlayerActorPlan.empty())
		{
			//construct path
			otherPlayerPathPlans[otherPlayerCluster] = otherPlayerActorPlan;
		}
	}

	for (otherPlayerClusterIdx = 0; otherPlayerClusterIdx < otherClusterSize; otherPlayerClusterIdx++)
	{
		PathingCluster* otherPlayerCluster = otherPlayerClusters[otherPlayerClusterIdx];
		if (otherPlayerPathPlans.find(otherPlayerCluster) != otherPlayerPathPlans.end())
			continue;

		PathingArcVec otherPlayerPathPlan;
		ConstructPath(otherPlayerState, otherPlayerCluster, otherPlayerPathPlan);
		if (otherPlayerPathPlan.size())
		{
			//construct path
			otherPlayerPathPlans[otherPlayerCluster] = otherPlayerPathPlan;
		}
	}

	eastl::map<PathingCluster*, eastl::map<PathingCluster*, NodeState>> playerClustersStates, otherPlayerClustersStates;
	for (playerClusterIdx = 0; playerClusterIdx < clusterSize; playerClusterIdx++)
	{
		PathingCluster* playerCluster = playerClusters[playerClusterIdx];

		for (otherPlayerClusterIdx = 0; otherPlayerClusterIdx < otherClusterSize; otherPlayerClusterIdx++)
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

	eastl::map<PathingCluster*, NodeState> currentClusterStates, otherCurrentClusterPlanStates;
	if (otherPlayerState.valid && otherPlayerState.plan.path.size())
	{
		for (playerClusterIdx = 0; playerClusterIdx < clusterSize; playerClusterIdx++)
		{
			PathingCluster* playerCluster = playerClusters[playerClusterIdx];

			NodeState state(playerState);
			NodeState otherState(otherPlayerState);
			Simulation(state, playerPathPlans[playerCluster], otherState, otherPlayerState.plan.path);

			if (state.valid && otherState.valid)
			{
				currentClusterStates[playerCluster] = state;
				otherCurrentClusterPlanStates[playerCluster] = otherState;
			}
		}
	}

	eastl::map<PathingCluster*, NodeState> otherCurrentClusterStates, currentClusterPlanStates;
	if (playerState.valid && playerState.plan.path.size())
	{
		for (otherPlayerClusterIdx = 0; otherPlayerClusterIdx < otherClusterSize; otherPlayerClusterIdx++)
		{
			PathingCluster* otherPlayerCluster = otherPlayerClusters[otherPlayerClusterIdx];

			NodeState state(playerState);
			NodeState otherState(otherPlayerState);
			Simulation(state, playerState.plan.path, otherState, otherPlayerPathPlans[otherPlayerCluster]);

			if (state.valid && otherState.valid)
			{
				otherCurrentClusterStates[otherPlayerCluster] = otherState;
				currentClusterPlanStates[otherPlayerCluster] = state;
			}
		}
	}

	NodeState currentPlanState, otherCurrentPlanState;
	if (playerState.valid && otherPlayerState.valid &&
		playerState.plan.path.size() && otherPlayerState.plan.path.size())
	{
		NodeState state(playerState);
		NodeState otherState(otherPlayerState);
		Simulation(state, playerState.plan.path, otherState, otherPlayerState.plan.path);

		if (state.valid && otherState.valid)
		{
			otherCurrentPlanState = otherState;
			currentPlanState = state;
		}
	}

	//minimax
	mPlayerState.Copy(playerState);
	mPlayerState.heuristic = -FLT_MAX;
	mPlayerState.valid = false;

	PathingCluster* playerCluster = NULL;
	for (auto playerClustersState : playerClustersStates)
	{
		NodeState playerNodeState(playerState);
		playerNodeState.heuristic = FLT_MAX;
		playerNodeState.valid = false;
		for (auto playerClusterState : playerClustersState.second)
		{
			if (playerClusterState.second.heuristic < playerNodeState.heuristic)
			{
				playerNodeState = playerClusterState.second;
			}
		}

		if (currentClusterStates.find(playerClustersState.first) != currentClusterStates.end())
		{
			if (currentClusterStates[playerClustersState.first].heuristic < playerNodeState.heuristic)
			{
				playerNodeState = currentClusterStates[playerClustersState.first];
			}
		}
		
		if (playerNodeState.valid)
		{
			if (playerNodeState.heuristic > mPlayerState.heuristic)
			{
				mPlayerState.Copy(playerNodeState);
				playerCluster = playerClustersState.first;
			}
			else if (abs(playerNodeState.heuristic - mPlayerState.heuristic) < GE_ROUNDING_ERROR)
			{
				//lets take the longest path for idle situations
				if (playerNodeState.plan.path.size() > mPlayerState.plan.path.size())
				{
					mPlayerState.Copy(playerNodeState);
					playerCluster = playerClustersState.first;
				}
			}
		}
	}

	if (playerState.valid)
	{
		NodeState playerNodeState(playerState);
		playerNodeState.heuristic = FLT_MAX;
		playerNodeState.valid = false;
		for (auto currentClusterPlanState : currentClusterPlanStates)
		{
			if (currentClusterPlanState.second.heuristic < playerNodeState.heuristic)
			{
				playerNodeState = currentClusterPlanState.second;
			}
		}

		if (currentPlanState.valid)
		{
			if (currentPlanState.heuristic < playerNodeState.heuristic)
			{
				playerNodeState = currentPlanState;
			}
		}
		
		//we keep the current plan if the heuristic is greater or close to the best heuristic
		if (playerNodeState.valid)
		{
			if (playerNodeState.heuristic > mPlayerState.heuristic)
			{
				mPlayerState.Copy(playerNodeState);
				playerCluster = NULL;
			}
			else if (abs(playerNodeState.heuristic - mPlayerState.heuristic) < 0.02f)
			{
				mPlayerState.Copy(playerNodeState);
				playerCluster = NULL;
			}
		}
	}

	if (mPlayerState.valid)
	{
		if (playerCluster != NULL)
		{
			mPlayerState.plan.id = mAIManager->GetNewPlanID();

			fprintf(mAIManager->mLogInformation, "\n player current position : %f %f %f ", 
				playerState.plan.node->GetPos()[0], playerState.plan.node->GetPos()[1], playerState.plan.node->GetPos()[2]);
			fprintf(mAIManager->mLogInformation, "\n player current cluster : %u ", playerState.plan.node->GetCluster());
			fprintf(mAIManager->mLogInformation, "\n player target cluster : %u ", playerCluster->GetTarget()->GetCluster());

			for (auto playerClusterState : playerClustersStates[playerCluster])
			{
				fprintf(mAIManager->mLogInformation, "other player cluster : %u ",
					playerClusterState.first->GetTarget()->GetCluster());
				fprintf(mAIManager->mLogInformation, "heuristic : %f ", playerClusterState.second.heuristic);

				if (playerClusterState.second.weapon != WP_NONE)
				{
					fprintf(mAIManager->mLogInformation, " weapon : %u ", playerClusterState.second.weapon);
					fprintf(mAIManager->mLogInformation, " damage : %i ",
						playerClusterState.second.damage[playerClusterState.second.weapon - 1]);
				}

				if (!playerClusterState.second.items.empty())
					fprintf(mAIManager->mLogInformation, " actors : ");
				for (eastl::shared_ptr<Actor> pItemActor : playerClusterState.second.items)
				{
					if (pItemActor->GetType() == "Weapon")
					{
						eastl::shared_ptr<WeaponPickup> pWeaponPickup =
							pItemActor->GetComponent<WeaponPickup>(WeaponPickup::Name).lock();
						fprintf(mAIManager->mLogInformation, "weapon %u ", pWeaponPickup->GetCode());
					}
					else if (pItemActor->GetType() == "Ammo")
					{
						eastl::shared_ptr<AmmoPickup> pAmmoPickup =
							pItemActor->GetComponent<AmmoPickup>(AmmoPickup::Name).lock();
						fprintf(mAIManager->mLogInformation, "ammo %u ", pAmmoPickup->GetCode());
					}
					else if (pItemActor->GetType() == "Armor")
					{
						eastl::shared_ptr<ArmorPickup> pArmorPickup =
							pItemActor->GetComponent<ArmorPickup>(ArmorPickup::Name).lock();
						fprintf(mAIManager->mLogInformation, "armor %u ", pArmorPickup->GetCode());
					}
					else if (pItemActor->GetType() == "Health")
					{
						eastl::shared_ptr<HealthPickup> pHealthPickup =
							pItemActor->GetComponent<HealthPickup>(HealthPickup::Name).lock();
						fprintf(mAIManager->mLogInformation, "health %u ", pHealthPickup->GetCode());
					}
				}
			}
		}
		else
		{
			fprintf(mAIManager->mLogInformation, "\n player same position : %f %f %f ",
				playerState.plan.node->GetPos()[0], playerState.plan.node->GetPos()[1], playerState.plan.node->GetPos()[2]);
			fprintf(mAIManager->mLogInformation, "\n player same plan cluster : %u ", playerState.plan.node->GetCluster());
			fprintf(mAIManager->mLogInformation, "\n player target cluster : ");

			for (auto currentClusterPlanState : currentClusterPlanStates)
			{
				fprintf(mAIManager->mLogInformation, "other player cluster : %u ",
					currentClusterPlanState.first->GetTarget()->GetCluster());
				fprintf(mAIManager->mLogInformation, "heuristic : %f ", currentClusterPlanState.second.heuristic);

				if (currentClusterPlanState.second.weapon != WP_NONE)
				{
					fprintf(mAIManager->mLogInformation, " weapon : %u ", currentClusterPlanState.second.weapon);
					fprintf(mAIManager->mLogInformation, " damage : %i ",
						currentClusterPlanState.second.damage[currentClusterPlanState.second.weapon - 1]);
				}

				if (!currentClusterPlanState.second.items.empty())
					fprintf(mAIManager->mLogInformation, " actors : ");
				for (eastl::shared_ptr<Actor> pItemActor : currentClusterPlanState.second.items)
				{
					if (pItemActor->GetType() == "Weapon")
					{
						eastl::shared_ptr<WeaponPickup> pWeaponPickup =
							pItemActor->GetComponent<WeaponPickup>(WeaponPickup::Name).lock();
						fprintf(mAIManager->mLogInformation, "weapon %u ", pWeaponPickup->GetCode());
					}
					else if (pItemActor->GetType() == "Ammo")
					{
						eastl::shared_ptr<AmmoPickup> pAmmoPickup =
							pItemActor->GetComponent<AmmoPickup>(AmmoPickup::Name).lock();
						fprintf(mAIManager->mLogInformation, "ammo %u ", pAmmoPickup->GetCode());
					}
					else if (pItemActor->GetType() == "Armor")
					{
						eastl::shared_ptr<ArmorPickup> pArmorPickup =
							pItemActor->GetComponent<ArmorPickup>(ArmorPickup::Name).lock();
						fprintf(mAIManager->mLogInformation, "armor %u ", pArmorPickup->GetCode());
					}
					else if (pItemActor->GetType() == "Health")
					{
						eastl::shared_ptr<HealthPickup> pHealthPickup =
							pItemActor->GetComponent<HealthPickup>(HealthPickup::Name).lock();
						fprintf(mAIManager->mLogInformation, "health %u ", pHealthPickup->GetCode());
					}
				}
			}
		}


		fprintf(mAIManager->mLogInformation, "\n player path id %i : ", mPlayerState.plan.id);
		for (PathingArc* pathArc : mPlayerState.plan.path)
			fprintf(mAIManager->mLogInformation, "%u ", pathArc->GetNode()->GetId());
		fprintf(mAIManager->mLogInformation, "\n player heuristic : %f ", mPlayerState.heuristic);

		if (mPlayerState.weapon != WP_NONE)
		{
			fprintf(mAIManager->mLogInformation, " weapon : %u ", mPlayerState.weapon);
			fprintf(mAIManager->mLogInformation, " damage : %i ",
				mPlayerState.damage[mPlayerState.weapon - 1]);
		}

		if (!mPlayerState.items.empty())
			fprintf(mAIManager->mLogInformation, " actors : ");
		for (eastl::shared_ptr<Actor> pItemActor : mPlayerState.items)
		{
			if (pItemActor->GetType() == "Weapon")
			{
				eastl::shared_ptr<WeaponPickup> pWeaponPickup =
					pItemActor->GetComponent<WeaponPickup>(WeaponPickup::Name).lock();
				fprintf(mAIManager->mLogInformation, "weapon %u ", pWeaponPickup->GetCode());
			}
			else if (pItemActor->GetType() == "Ammo")
			{
				eastl::shared_ptr<AmmoPickup> pAmmoPickup =
					pItemActor->GetComponent<AmmoPickup>(AmmoPickup::Name).lock();
				fprintf(mAIManager->mLogInformation, "ammo %u ", pAmmoPickup->GetCode());
			}
			else if (pItemActor->GetType() == "Armor")
			{
				eastl::shared_ptr<ArmorPickup> pArmorPickup =
					pItemActor->GetComponent<ArmorPickup>(ArmorPickup::Name).lock();
				fprintf(mAIManager->mLogInformation, "armor %u ", pArmorPickup->GetCode());
			}
			else if (pItemActor->GetType() == "Health")
			{
				eastl::shared_ptr<HealthPickup> pHealthPickup =
					pItemActor->GetComponent<HealthPickup>(HealthPickup::Name).lock();
				fprintf(mAIManager->mLogInformation, "health %u ", pHealthPickup->GetCode());
			}
		}
	}

	mOtherPlayerState.Copy(otherPlayerState);
	mOtherPlayerState.heuristic = FLT_MAX;
	mOtherPlayerState.valid = false;

	PathingCluster* otherPlayerCluster = NULL;
	for (auto otherPlayerClustersState : otherPlayerClustersStates)
	{
		NodeState otherPlayerNodeState(otherPlayerState);
		otherPlayerNodeState.heuristic = -FLT_MAX;
		otherPlayerNodeState.valid = false;
		for (auto otherPlayerClusterState : otherPlayerClustersState.second)
		{
			if (otherPlayerClusterState.second.heuristic > otherPlayerNodeState.heuristic)
			{
				otherPlayerNodeState = otherPlayerClusterState.second;
			}
		}

		if (otherCurrentClusterStates.find(otherPlayerClustersState.first) != otherCurrentClusterStates.end())
		{
			if (otherCurrentClusterStates[otherPlayerClustersState.first].heuristic > otherPlayerNodeState.heuristic)
			{
				otherPlayerNodeState = otherCurrentClusterStates[otherPlayerClustersState.first];
			}
		}

		if (otherPlayerNodeState.valid)
		{
			if (otherPlayerNodeState.heuristic < mOtherPlayerState.heuristic)
			{
				mOtherPlayerState.Copy(otherPlayerNodeState);
				otherPlayerCluster = otherPlayerClustersState.first;
			}
			else if (abs(otherPlayerNodeState.heuristic - mOtherPlayerState.heuristic) < GE_ROUNDING_ERROR)
			{
				//lets take the longest path for idle situations
				if (otherPlayerNodeState.plan.path.size() > mOtherPlayerState.plan.path.size())
				{
					mOtherPlayerState.Copy(otherPlayerNodeState);
					otherPlayerCluster = otherPlayerClustersState.first;
				}
			}
		}
	}

	if (otherPlayerState.valid)
	{
		NodeState otherPlayerNodeState(otherPlayerState);
		otherPlayerNodeState.heuristic = -FLT_MAX;
		otherPlayerNodeState.valid = false;
		for (auto otherCurrentClusterPlanState : otherCurrentClusterPlanStates)
		{
			if (otherCurrentClusterPlanState.second.heuristic > otherPlayerNodeState.heuristic)
			{
				otherPlayerNodeState = otherCurrentClusterPlanState.second;
			}
		}

		if (otherCurrentPlanState.valid)
		{
			if (otherCurrentPlanState.heuristic > otherPlayerNodeState.heuristic)
			{
				otherPlayerNodeState = otherCurrentPlanState;
			}
		}

		//we keep the current plan if the heuristic is less or close to the best player heuristic
		if (otherPlayerNodeState.valid)
		{
			if (otherPlayerNodeState.heuristic < mOtherPlayerState.heuristic)
			{
				mOtherPlayerState.Copy(otherPlayerNodeState);
				otherPlayerCluster = NULL;
			}
			else if (abs(otherPlayerNodeState.heuristic - mOtherPlayerState.heuristic) < 0.02f)
			{
				mOtherPlayerState.Copy(otherPlayerNodeState);
				otherPlayerCluster = NULL;
			}
		}
	}

	if (mOtherPlayerState.valid)
	{
		if (otherPlayerCluster != NULL)
		{
			mOtherPlayerState.plan.id = mAIManager->GetNewPlanID();

			fprintf(mAIManager->mLogInformation, "\n other player current position : %f %f %f ",
				otherPlayerState.plan.node->GetPos()[0], otherPlayerState.plan.node->GetPos()[1], otherPlayerState.plan.node->GetPos()[2]);
			fprintf(mAIManager->mLogInformation, "\n other player current cluster : %u ",
				otherPlayerState.plan.node->GetCluster());
			fprintf(mAIManager->mLogInformation, "\n other player target cluster : %u ",
				otherPlayerCluster->GetTarget()->GetCluster());

			for (auto otherPlayerClusterState : otherPlayerClustersStates[otherPlayerCluster])
			{
				fprintf(mAIManager->mLogInformation, "player cluster : %u ",
					otherPlayerClusterState.first->GetTarget()->GetCluster());
				fprintf(mAIManager->mLogInformation, "heuristic : %f ", otherPlayerClusterState.second.heuristic);

				if (otherPlayerClusterState.second.weapon != WP_NONE)
				{
					fprintf(mAIManager->mLogInformation, " weapon : %u ", otherPlayerClusterState.second.weapon);
					fprintf(mAIManager->mLogInformation, " damage : %i ",
						otherPlayerClusterState.second.damage[otherPlayerClusterState.second.weapon - 1]);
				}

				if (!otherPlayerClusterState.second.items.empty())
					fprintf(mAIManager->mLogInformation, " actors : ");
				for (eastl::shared_ptr<Actor> pItemActor : otherPlayerClusterState.second.items)
				{
					if (pItemActor->GetType() == "Weapon")
					{
						eastl::shared_ptr<WeaponPickup> pWeaponPickup =
							pItemActor->GetComponent<WeaponPickup>(WeaponPickup::Name).lock();
						fprintf(mAIManager->mLogInformation, "weapon %u ", pWeaponPickup->GetCode());
					}
					else if (pItemActor->GetType() == "Ammo")
					{
						eastl::shared_ptr<AmmoPickup> pAmmoPickup =
							pItemActor->GetComponent<AmmoPickup>(AmmoPickup::Name).lock();
						fprintf(mAIManager->mLogInformation, "ammo %u ", pAmmoPickup->GetCode());
					}
					else if (pItemActor->GetType() == "Armor")
					{
						eastl::shared_ptr<ArmorPickup> pArmorPickup =
							pItemActor->GetComponent<ArmorPickup>(ArmorPickup::Name).lock();
						fprintf(mAIManager->mLogInformation, "armor %u ", pArmorPickup->GetCode());
					}
					else if (pItemActor->GetType() == "Health")
					{
						eastl::shared_ptr<HealthPickup> pHealthPickup =
							pItemActor->GetComponent<HealthPickup>(HealthPickup::Name).lock();
						fprintf(mAIManager->mLogInformation, "health %u ", pHealthPickup->GetCode());
					}
				}
			}
		}
		else
		{
			fprintf(mAIManager->mLogInformation, "\n other player same position : %f %f %f ",
				otherPlayerState.plan.node->GetPos()[0], otherPlayerState.plan.node->GetPos()[1], otherPlayerState.plan.node->GetPos()[2]);
			fprintf(mAIManager->mLogInformation, "\n other player same plan cluster : %u ",
				otherPlayerState.plan.node->GetCluster());
			fprintf(mAIManager->mLogInformation, "\n other player target cluster :  ");

			for (auto otherCurrentClusterPlanState : otherCurrentClusterPlanStates)
			{
				fprintf(mAIManager->mLogInformation, "player cluster : %u ",
					otherCurrentClusterPlanState.first->GetTarget()->GetCluster());
				fprintf(mAIManager->mLogInformation, "heuristic : %f ", otherCurrentClusterPlanState.second.heuristic);

				if (otherCurrentClusterPlanState.second.weapon != WP_NONE)
				{
					fprintf(mAIManager->mLogInformation, " weapon : %u ", otherCurrentClusterPlanState.second.weapon);
					fprintf(mAIManager->mLogInformation, " damage : %i ",
						otherCurrentClusterPlanState.second.damage[otherCurrentClusterPlanState.second.weapon - 1]);
				}

				if (!otherCurrentClusterPlanState.second.items.empty())
					fprintf(mAIManager->mLogInformation, " actors : ");
				for (eastl::shared_ptr<Actor> pItemActor : otherCurrentClusterPlanState.second.items)
				{
					if (pItemActor->GetType() == "Weapon")
					{
						eastl::shared_ptr<WeaponPickup> pWeaponPickup =
							pItemActor->GetComponent<WeaponPickup>(WeaponPickup::Name).lock();
						fprintf(mAIManager->mLogInformation, "weapon %u ", pWeaponPickup->GetCode());
					}
					else if (pItemActor->GetType() == "Ammo")
					{
						eastl::shared_ptr<AmmoPickup> pAmmoPickup =
							pItemActor->GetComponent<AmmoPickup>(AmmoPickup::Name).lock();
						fprintf(mAIManager->mLogInformation, "ammo %u ", pAmmoPickup->GetCode());
					}
					else if (pItemActor->GetType() == "Armor")
					{
						eastl::shared_ptr<ArmorPickup> pArmorPickup =
							pItemActor->GetComponent<ArmorPickup>(ArmorPickup::Name).lock();
						fprintf(mAIManager->mLogInformation, "armor %u ", pArmorPickup->GetCode());
					}
					else if (pItemActor->GetType() == "Health")
					{
						eastl::shared_ptr<HealthPickup> pHealthPickup =
							pItemActor->GetComponent<HealthPickup>(HealthPickup::Name).lock();
						fprintf(mAIManager->mLogInformation, "health %u ", pHealthPickup->GetCode());
					}
				}
			}
		}

		fprintf(mAIManager->mLogInformation, "\n other player path id %i : ", mOtherPlayerState.plan.id);
		for (PathingArc* pathArc : mOtherPlayerState.plan.path)
			fprintf(mAIManager->mLogInformation, "%u ", pathArc->GetNode()->GetId());
		fprintf(mAIManager->mLogInformation, "\n other player heuristic : %f ", mOtherPlayerState.heuristic);

		if (mOtherPlayerState.weapon != WP_NONE)
		{
			fprintf(mAIManager->mLogInformation, " weapon : %u ", mOtherPlayerState.weapon);
			fprintf(mAIManager->mLogInformation, " damage : %i ",
				mOtherPlayerState.damage[mOtherPlayerState.weapon - 1]);
		}

		if (!mOtherPlayerState.items.empty())
			fprintf(mAIManager->mLogInformation, " actors : ");
		for (eastl::shared_ptr<Actor> pItemActor : mOtherPlayerState.items)
		{
			if (pItemActor->GetType() == "Weapon")
			{
				eastl::shared_ptr<WeaponPickup> pWeaponPickup =
					pItemActor->GetComponent<WeaponPickup>(WeaponPickup::Name).lock();
				fprintf(mAIManager->mLogInformation, "weapon %u ", pWeaponPickup->GetCode());
			}
			else if (pItemActor->GetType() == "Ammo")
			{
				eastl::shared_ptr<AmmoPickup> pAmmoPickup =
					pItemActor->GetComponent<AmmoPickup>(AmmoPickup::Name).lock();
				fprintf(mAIManager->mLogInformation, "ammo %u ", pAmmoPickup->GetCode());
			}
			else if (pItemActor->GetType() == "Armor")
			{
				eastl::shared_ptr<ArmorPickup> pArmorPickup =
					pItemActor->GetComponent<ArmorPickup>(ArmorPickup::Name).lock();
				fprintf(mAIManager->mLogInformation, "armor %u ", pArmorPickup->GetCode());
			}
			else if (pItemActor->GetType() == "Health")
			{
				eastl::shared_ptr<HealthPickup> pHealthPickup =
					pItemActor->GetComponent<HealthPickup>(HealthPickup::Name).lock();
				fprintf(mAIManager->mLogInformation, "health %u ", pHealthPickup->GetCode());
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
				//printf("\n ITERATION %u", iteration);
				Timer::RealTimeDate realTime = Timer::GetRealTimeAndDate();
				fprintf(mAIManager->mLogInformation, "\n\n ITERATION %u time %u:%u:%u \n", 
					iteration, realTime.Hour, realTime.Minute, realTime.Second);

				fprintf(mAIManager->mLogInformation, "\n blue player ai guessing (red)");
				for (ActorId player : players[GV_HUMAN])
				{
					eastl::shared_ptr<PlayerActor> pHumanPlayer =
						eastl::dynamic_shared_pointer_cast<PlayerActor>(
						GameLogic::Get()->GetActor(player).lock());

					NodeState playerState(pHumanPlayer);
					mAIManager->GetPlayerPlan(player, playerState.plan);
					for (ActorId aiPlayer : players[GV_AI])
					{
						NodeState aiPlayerState;
						mAIManager->GetPlayerGuessState(aiPlayer, aiPlayerState);
						mAIManager->GetPlayerGuessPlan(aiPlayer, aiPlayerState.plan);
						aiPlayerState.ResetItems();

						mExcludeActors.clear();
						mAIManager->GetPlayerGuessItems(aiPlayerState.player, mExcludeActors);
						EvaluatePlayers(playerState, aiPlayerState);
					}
				}

				fprintf(mAIManager->mLogInformation, "\n blue player heuristic %f", mPlayerState.heuristic);
				//printf("\n blue player actors  %u : ", mPlayerState.player);
				if (!mPlayerState.items.empty()) 
					fprintf(mAIManager->mLogInformation, " actors : ");
				for (eastl::shared_ptr<Actor> pItemActor : mPlayerState.items)
				{
					if (pItemActor->GetType() == "Weapon")
					{
						eastl::shared_ptr<WeaponPickup> pWeaponPickup =
							pItemActor->GetComponent<WeaponPickup>(WeaponPickup::Name).lock();
						fprintf(mAIManager->mLogInformation, "weapon %u ", pWeaponPickup->GetCode());
						//printf("weapon %u ", pWeaponPickup->GetCode());
					}
					else if (pItemActor->GetType() == "Ammo")
					{
						eastl::shared_ptr<AmmoPickup> pAmmoPickup =
							pItemActor->GetComponent<AmmoPickup>(AmmoPickup::Name).lock();
						fprintf(mAIManager->mLogInformation, "ammo %u ", pAmmoPickup->GetCode());
						//printf("ammo %u ", pAmmoPickup->GetCode());
					}
					else if (pItemActor->GetType() == "Armor")
					{
						eastl::shared_ptr<ArmorPickup> pArmorPickup =
							pItemActor->GetComponent<ArmorPickup>(ArmorPickup::Name).lock();
						fprintf(mAIManager->mLogInformation, "armor %u ", pArmorPickup->GetCode());
						//printf("armor %u ", pArmorPickup->GetCode());
					}
					else if (pItemActor->GetType() == "Health")
					{
						eastl::shared_ptr<HealthPickup> pHealthPickup =
							pItemActor->GetComponent<HealthPickup>(HealthPickup::Name).lock();
						fprintf(mAIManager->mLogInformation, "health %u ", pHealthPickup->GetCode());
						//printf("health %u ", pHealthPickup->GetCode());
					}
				}
				if (mPlayerState.weapon != WP_NONE)
				{
					fprintf(mAIManager->mLogInformation, " weapon : %u ", mPlayerState.weapon);
					fprintf(mAIManager->mLogInformation, " damage : %i ", 
						mPlayerState.damage[mPlayerState.weapon - 1]);
				}
				else
				{
					fprintf(mAIManager->mLogInformation, " weapon : 0 ");
					fprintf(mAIManager->mLogInformation, " damage : 0 ");
				}

				mAIManager->SetPlayerGuessState(mOtherPlayerState.player, mOtherPlayerState);
				mAIManager->SetPlayerGuessUpdated(mOtherPlayerState.player, true);

				mAIManager->SetPlayerState(mPlayerState.player, mPlayerState);
				mAIManager->SetPlayerUpdated(mPlayerState.player, true);

				fprintf(mAIManager->mLogInformation, "\n\n red player ai guessing (blue)");
				for (ActorId aiPlayer : players[GV_AI])
				{
					eastl::shared_ptr<PlayerActor> pAIPlayer =
						eastl::dynamic_shared_pointer_cast<PlayerActor>(
						GameLogic::Get()->GetActor(aiPlayer).lock());

					NodeState aiPlayerState(pAIPlayer);
					mAIManager->GetPlayerPlan(aiPlayer, aiPlayerState.plan);
					for (ActorId player : players[GV_HUMAN])
					{
						NodeState playerState;
						mAIManager->GetPlayerGuessState(player, playerState);
						mAIManager->GetPlayerGuessPlan(player, playerState.plan);
						playerState.ResetItems();

						mExcludeActors.clear();
						mAIManager->GetPlayerGuessItems(playerState.player, mExcludeActors);
						EvaluatePlayers(playerState, aiPlayerState);
					}
				}

				fprintf(mAIManager->mLogInformation, "\n red player heuristic %f", mOtherPlayerState.heuristic);
				//printf("\n red player actors %u : ", mOtherPlayerState.player);
				if (!mOtherPlayerState.items.empty()) 
					fprintf(mAIManager->mLogInformation, " actors : ");
				for (eastl::shared_ptr<Actor> pItemActor : mOtherPlayerState.items)
				{
					if (pItemActor->GetType() == "Weapon")
					{
						eastl::shared_ptr<WeaponPickup> pWeaponPickup =
							pItemActor->GetComponent<WeaponPickup>(WeaponPickup::Name).lock();
						fprintf(mAIManager->mLogInformation, "weapon %u ", pWeaponPickup->GetCode());
						//printf("weapon %u ", pWeaponPickup->GetCode());
					}
					else if (pItemActor->GetType() == "Ammo")
					{
						eastl::shared_ptr<AmmoPickup> pAmmoPickup =
							pItemActor->GetComponent<AmmoPickup>(AmmoPickup::Name).lock();
						fprintf(mAIManager->mLogInformation, "ammo %u ", pAmmoPickup->GetCode());
						//printf("ammo %u ", pAmmoPickup->GetCode());
					}
					else if (pItemActor->GetType() == "Armor")
					{
						eastl::shared_ptr<ArmorPickup> pArmorPickup =
							pItemActor->GetComponent<ArmorPickup>(ArmorPickup::Name).lock();
						fprintf(mAIManager->mLogInformation, "armor %u ", pArmorPickup->GetCode());
						//printf("armor %u ", pArmorPickup->GetCode());
					}
					else if (pItemActor->GetType() == "Health")
					{
						eastl::shared_ptr<HealthPickup> pHealthPickup =
							pItemActor->GetComponent<HealthPickup>(HealthPickup::Name).lock();
						fprintf(mAIManager->mLogInformation, "health %u ", pHealthPickup->GetCode());
						//printf("health %u ", pHealthPickup->GetCode());
					}
				}
				if (mOtherPlayerState.weapon != WP_NONE)
				{
					fprintf(mAIManager->mLogInformation, " weapon : %u ", mOtherPlayerState.weapon);
					fprintf(mAIManager->mLogInformation, 
						" damage : %i ", mOtherPlayerState.damage[mOtherPlayerState.weapon - 1]);
				}
				else
				{
					fprintf(mAIManager->mLogInformation, " weapon : 0 ");
					fprintf(mAIManager->mLogInformation, " damage : 0 ");
				}

				mAIManager->SetPlayerGuessState(mPlayerState.player, mPlayerState);
				mAIManager->SetPlayerGuessUpdated(mPlayerState.player, true);

				mAIManager->SetPlayerState(mOtherPlayerState.player, mOtherPlayerState);
				mAIManager->SetPlayerUpdated(mOtherPlayerState.player, true);
			}

			mAIManager->SetEnable(true);
		}
	}

	Succeed();
}