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
	float* visibleTime, float* visibleDistance, float* visibleHeight,
	float* otherVisibleTime, float* otherVisibleDistance, float* otherVisibleHeight)
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

		//at this point we don't know how potentialy visible are the node in which
		//each player has arriven. We will calculate an estimation of the closest
		//clusters from every player and we take that average calculation as a 
		//percentage of a minimum of visibility time (i.e. 1.0 second)
		otherIndex--;

		PathingClusterVec playerClusters, otherPlayerClusters;
		currentTransition->GetNodes()[index]->GetClusters(GAT_JUMP, playerClusters, 20);
		otherCurrentTransition->GetNodes()[otherIndex]->GetClusters(GAT_JUMP, otherPlayerClusters, 20);

		unsigned int visibleClusterCount = 0;
		unsigned int visibleClusterSize = playerClusters.size() * otherPlayerClusters.size();
		for (PathingCluster* playerCluster : playerClusters)
		{
			for (PathingCluster* otherPlayerCluster : otherPlayerClusters)
			{
				if (mAIManager->GetPathingGraph()->IsVisibleCluster(
					playerCluster->GetTarget()->GetCluster(), 
					otherPlayerCluster->GetTarget()->GetCluster()))
				{
					visibleClusterCount++;
				}
			}
		}

		if (visibleClusterCount > 0)
		{
			visibleWeight = 1.0f * (visibleClusterCount /(float)visibleClusterSize);

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
	NodeState& playerState, PathingArcVec& playerPathPlan,
	NodeState& otherPlayerState, PathingArcVec& otherPlayerPathPlan)
{
	PathingNode* playerNode = playerState.node;
	PathingNode* otherPlayerNode = otherPlayerState.node;
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
	playerState.Copy(playerNodeState);
	otherPlayerState.Copy(otherPlayerNodeState);
}

void QuakeAIProcess::ConstructPath(NodeState& playerState,
	PathingCluster* playerCluster, PathingArcVec& playerPathPlan)
{
	PathingNode* currentNode = playerState.node;
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
	PathingCluster* playerCluster, PathingArcVec& playerPathPlan)
{
	PathingNode* currentNode = playerState.node;
	float maxPathDistance = 0.f;
	while (currentNode != playerCluster->GetTarget())
	{
		PathingCluster* currentCluster = currentNode->FindCluster(
			playerCluster->GetType(), playerCluster->GetTarget());
		PathingArc* currentArc = currentNode->FindArc(currentCluster->GetNode());

		currentNode = currentArc->GetNode();
		maxPathDistance += currentArc->GetWeight();
	}
	//add extra time
	maxPathDistance += 1.0f;
	mAIManager->FindPath(playerState, playerCluster, playerPathPlan, maxPathDistance);
}


void QuakeAIProcess::EvaluatePlayers(NodeState& playerState, NodeState& otherPlayerState)
{
	eastl::map<PathingCluster*, PathingArcVec> playerPathPlans, otherPlayerPathPlans;

	PathingClusterVec playerClusters;
	playerState.node->GetClusters(GAT_JUMP, playerClusters, 60);

	unsigned int clusterCount = 0;
	unsigned int clusterSize = playerClusters.size();
	//construct path based on closest actors to each cluster pathway
	for (unsigned int playerClusterIdx = 0; playerClusterIdx < clusterSize; playerClusterIdx++)
	{
		PathingCluster* playerCluster = playerClusters[playerClusterIdx];

		PathingArcVec playerPathPlan;
		ConstructActorPath(playerState, playerCluster, playerPathPlan);
		if (playerPathPlan.size())
		{
			//construct path
			playerPathPlans[playerCluster] = playerPathPlan;

			clusterCount++;
			if (clusterCount >= 30) break;
		}
	}

	//construct path based on cluster pathway
	for (unsigned int playerClusterIdx = 0; playerClusterIdx < clusterSize; playerClusterIdx++)
	{
		PathingCluster* playerCluster = playerClusters[playerClusterIdx];
		if (playerPathPlans.find(playerCluster) != playerPathPlans.end())
			continue;

		PathingArcVec playerPathPlan;
		ConstructPath(playerState, playerCluster, playerPathPlan);
		if (playerPathPlan.size())
		{
			//construct path
			playerPathPlans[playerCluster] = playerPathPlan;
		}
	}

	PathingClusterVec otherPlayerClusters;
	otherPlayerState.node->GetClusters(GAT_JUMP, otherPlayerClusters, 60);

	unsigned int otherClusterSize = otherPlayerClusters.size();
	unsigned int otherClusterCount = 0;
	for (unsigned int otherPlayerClusterIdx = 0; otherPlayerClusterIdx < otherClusterSize; otherPlayerClusterIdx++)
	{
		PathingCluster* otherPlayerCluster = otherPlayerClusters[otherPlayerClusterIdx];

		PathingArcVec otherPlayerPathPlan;
		ConstructActorPath(otherPlayerState, otherPlayerCluster, otherPlayerPathPlan);
		if (otherPlayerPathPlan.size())
		{
			//construct path
			otherPlayerPathPlans[otherPlayerCluster] = otherPlayerPathPlan;

			otherClusterCount++;
			if (otherClusterCount >= 30) break;
		}
	}

	for (unsigned int otherPlayerClusterIdx = 0; otherPlayerClusterIdx < otherClusterSize; otherPlayerClusterIdx++)
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
	for (unsigned int playerClusterIdx = 0; playerClusterIdx < clusterSize; playerClusterIdx++)
	{
		PathingCluster* playerCluster = playerClusters[playerClusterIdx];

		for (unsigned int otherPlayerClusterIdx = 0; otherPlayerClusterIdx < otherClusterSize; otherPlayerClusterIdx++)
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

	PathingCluster* playerCluster = NULL;
	for (auto playerClustersState : playerClustersStates)
	{
		NodeState playerNodeState(playerState);
		playerNodeState.heuristic = FLT_MAX;
		for (auto playerClusterState : playerClustersState.second)
		{
			if (playerClusterState.second.heuristic < playerNodeState.heuristic)
			{
				playerNodeState = playerClusterState.second;
			}
		}
		
		if (playerNodeState.valid)
		{
			if (playerNodeState.heuristic > mPlayerState.heuristic)
			{
				mPlayerState.Copy(playerNodeState);
				playerCluster = playerClustersState.first;
			}
		}
	}

	if (playerClustersStates.find(playerCluster) != playerClustersStates.end())
	{
		fprintf(mAIManager->mFile, "\n player cluster : %u ", playerCluster->GetTarget()->GetCluster());

		for (auto playerClusterState : playerClustersStates[playerCluster])
		{
			fprintf(mAIManager->mFile, "other player cluster : %u ",
				playerClusterState.first->GetTarget()->GetCluster());
			fprintf(mAIManager->mFile, "heuristic : %f ", playerClusterState.second.heuristic);

			if (playerClusterState.second.weapon != WP_NONE)
			{
				fprintf(mAIManager->mFile, " weapon : %u ", playerClusterState.second.weapon);
				fprintf(mAIManager->mFile, " damage : %i ",
					playerClusterState.second.damage[playerClusterState.second.weapon - 1]);
			}

			if (!playerClusterState.second.items.empty())
				fprintf(mAIManager->mFile, " actors : ");
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
		}

		if (mPlayerState.valid)
		{
			fprintf(mAIManager->mFile, "\n player path : ");
			for (PathingArc* pathArc : mPlayerState.path)
				fprintf(mAIManager->mFile, "%u ", pathArc->GetNode()->GetId());
			fprintf(mAIManager->mFile, "\n player heuristic : %f ", mPlayerState.heuristic);

			if (mPlayerState.weapon != WP_NONE)
			{
				fprintf(mAIManager->mFile, " weapon : %u ", mPlayerState.weapon);
				fprintf(mAIManager->mFile, " damage : %i ",
					mPlayerState.damage[mPlayerState.weapon - 1]);
			}

			if (!mPlayerState.items.empty())
				fprintf(mAIManager->mFile, " actors : ");
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
	}

	mOtherPlayerState.Copy(otherPlayerState);
	mOtherPlayerState.heuristic = FLT_MAX;

	PathingCluster* otherPlayerCluster = NULL;
	for (auto otherPlayerClustersState : otherPlayerClustersStates)
	{
		NodeState otherPlayerNodeState(otherPlayerState);
		otherPlayerNodeState.heuristic = -FLT_MAX;
		for (auto otherPlayerClusterState : otherPlayerClustersState.second)
		{
			if (otherPlayerClusterState.second.heuristic > otherPlayerNodeState.heuristic)
			{
				otherPlayerNodeState = otherPlayerClusterState.second;
			}
		}

		if (otherPlayerNodeState.valid)
		{
			if (otherPlayerNodeState.heuristic < mOtherPlayerState.heuristic)
			{
				mOtherPlayerState.Copy(otherPlayerNodeState);
				otherPlayerCluster = otherPlayerClustersState.first;
			}
		}
	}

	if (otherPlayerClustersStates.find(otherPlayerCluster) != otherPlayerClustersStates.end())
	{
		fprintf(mAIManager->mFile, "\n other player cluster : %u ",
			otherPlayerCluster->GetTarget()->GetCluster());

		for (auto otherPlayerClusterState : otherPlayerClustersStates[otherPlayerCluster])
		{
			fprintf(mAIManager->mFile, "player cluster : %u ",
				otherPlayerClusterState.first->GetTarget()->GetCluster());
			fprintf(mAIManager->mFile, "heuristic : %f ", otherPlayerClusterState.second.heuristic);

			if (otherPlayerClusterState.second.weapon != WP_NONE)
			{
				fprintf(mAIManager->mFile, " weapon : %u ", otherPlayerClusterState.second.weapon);
				fprintf(mAIManager->mFile, " damage : %i ",
					otherPlayerClusterState.second.damage[otherPlayerClusterState.second.weapon - 1]);
			}

			if (!otherPlayerClusterState.second.items.empty())
				fprintf(mAIManager->mFile, " actors : ");
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
		}

		if (mOtherPlayerState.valid)
		{
			fprintf(mAIManager->mFile, "\n other player path : ");
			for (PathingArc* pathArc : mOtherPlayerState.path)
				fprintf(mAIManager->mFile, "%u ", pathArc->GetNode()->GetId());
			fprintf(mAIManager->mFile, "\n other player heuristic : %f ", mOtherPlayerState.heuristic);

			if (mOtherPlayerState.weapon != WP_NONE)
			{
				fprintf(mAIManager->mFile, " weapon : %u ", mOtherPlayerState.weapon);
				fprintf(mAIManager->mFile, " damage : %i ",
					mOtherPlayerState.damage[mOtherPlayerState.weapon - 1]);
			}

			if (!mOtherPlayerState.items.empty())
				fprintf(mAIManager->mFile, " actors : ");
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
				fprintf(mAIManager->mFile, "\n blue player path : ");
				for (PathingArc* pathArc : mPlayerState.path)
					fprintf(mAIManager->mFile, "%u ", pathArc->GetNode()->GetId());
				fprintf(mAIManager->mFile, "\n blue player   %f", mPlayerState.heuristic);
				//printf("\n blue player actors  %u : ", mPlayerState.player);
				if (!mPlayerState.items.empty()) 
					fprintf(mAIManager->mFile, " actors : ");
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
				fprintf(mAIManager->mFile, "\n red player path : ");
				for (PathingArc* pathArc : mOtherPlayerState.path) 
					fprintf(mAIManager->mFile, "%u ", pathArc->GetNode()->GetId());
				fprintf(mAIManager->mFile, "\n red player   %f", mOtherPlayerState.heuristic);
				//printf("\n red player actors %u : ", mOtherPlayerState.player);
				if (!mOtherPlayerState.items.empty()) 
					fprintf(mAIManager->mFile, " actors : ");
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