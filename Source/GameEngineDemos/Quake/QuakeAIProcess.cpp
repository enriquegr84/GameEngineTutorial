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
#include "QuakeAIManager.h"
#include "QuakeAIProcess.h"

QuakeAIProcess::QuakeAIProcess()
  :  RealtimeProcess()
{

}

QuakeAIProcess::~QuakeAIProcess(void)
{

}

void QuakeAIProcess::ConstructPath(PathingNode* playerClusterNode, unsigned int playerClusterType,
	eastl::map<PathingNode*, float>& playerVisibleNodes, eastl::vector<PathingArcVec>& playerPathPlan)
{
	//lets traverse the closest cluster nodes and actors from our current position
	//it is the best way to expand the area uniformly
	PathingClusterVec playerClusters;
	playerClusterNode->GetClusters(playerClusterType, playerClusters);
	unsigned int clusterSize = playerClusters.size() <= 10 ? playerClusters.size() : 10;
	for (unsigned int clusterIdx = 0; clusterIdx < clusterSize; clusterIdx++)
	{
		PathingCluster* pathCluster = playerClusters[clusterIdx];

		PathingArcVec pathPlan;
		PathingNode* currentNode = playerClusterNode;
		while (currentNode != pathCluster->GetTarget())
		{
			PathingCluster* currentCluster = 
				currentNode->FindCluster(playerClusterType, pathCluster->GetTarget());
			PathingArc* currentArc = currentNode->FindArc(currentCluster->GetNode());

			pathPlan.push_back(currentArc);
			currentNode = currentArc->GetNode();
			if (playerVisibleNodes.find(currentNode) != playerVisibleNodes.end())
				playerVisibleNodes.erase(currentNode);
		}
		playerPathPlan.push_back(pathPlan);
	}

	//if there are missing visible nodes we search deeper
	if (!playerVisibleNodes.empty())
	{
		PathingNodeArcMap pathNodePlan[10];
		for (unsigned int clusterIdx = 0; clusterIdx < clusterSize; clusterIdx++)
		{
			PathingCluster* pathCluster = playerClusters[clusterIdx];

			PathingArcVec pathPlan;
			PathingNode* currentNode = playerClusterNode;
			while (currentNode != pathCluster->GetTarget())
			{
				for (PathingArc* currentArc : currentNode->GetArcs())
				{
					if (playerVisibleNodes.find(currentArc->GetNode()) != playerVisibleNodes.end())
						for (PathingArcVec::iterator itPath = pathPlan.begin(); itPath != pathPlan.end(); itPath++)
							pathNodePlan[clusterIdx][currentArc->GetNode()].push_back(*itPath);
				}
				PathingCluster* currentCluster = currentNode->FindCluster(playerClusterType, pathCluster->GetTarget());
				PathingArc* currentArc = currentNode->FindArc(currentCluster->GetNode());

				pathPlan.push_back(currentArc);
				currentNode = currentArc->GetNode();
			}
		}

		//now we select a minimum of missing visible nodes
		unsigned int minimumVisibleNodes = 0;
		bool foundVisibleNode = false;
		do
		{
			foundVisibleNode = false;
			for (unsigned int clusterIdx = 0; clusterIdx < clusterSize; clusterIdx++)
			{
				for (auto playerVisibleNode : playerVisibleNodes)
				{
					PathingNode* visibleNode = playerVisibleNode.first;
					if (pathNodePlan[clusterIdx].find(visibleNode) != pathNodePlan[clusterIdx].end())
					{
						foundVisibleNode = true;
						playerVisibleNodes.erase(visibleNode);
						playerPathPlan.push_back(pathNodePlan[clusterIdx][visibleNode]);
						break;
					}
				}
			}
		} while (minimumVisibleNodes < 10 && foundVisibleNode);
	}
}

void QuakeAIProcess::TraverseNode(PathingNode* playerNode, 
	PathingCluster* otherPlayerCluster, unsigned int otherPlayerClusterType, bool expandNodes)
{
	//first we find those nodes which contains actor or/and were visible
	eastl::map<PathingNode*, float> otherPlayerVisibleNodes;
	eastl::map<PathingNode*, float> otherPlayerNodes;
	otherPlayerNodes[otherPlayerCluster->GetNode()] = 0.f;

	PathingNode* currentNode = otherPlayerCluster->GetNode();
	while (currentNode != otherPlayerCluster->GetTarget())
	{
		for (PathingArc* currentArc : currentNode->GetArcs())
		{
			otherPlayerNodes[currentArc->GetNode()] = currentArc->GetWeight();
			if (currentArc->GetNode()->IsVisibleNode(playerNode))
				otherPlayerVisibleNodes[currentArc->GetNode()] = currentArc->GetWeight();
		}

		PathingCluster* currentCluster = 
			currentNode->FindCluster(otherPlayerClusterType, otherPlayerCluster->GetTarget());
		PathingArc* currentArc = currentNode->FindArc(currentCluster->GetNode());
		otherPlayerNodes[currentArc->GetNode()] = currentArc->GetWeight();
		if (currentArc->GetNode()->IsVisibleNode(playerNode))
			otherPlayerVisibleNodes[currentArc->GetNode()] = currentArc->GetWeight();

		currentNode = currentArc->GetNode();
	}

	// next we construct path for those nodes which were visible
	if (!otherPlayerVisibleNodes.empty())
	{
		eastl::vector<PathingArcVec> otherPlayerPathPlan;
		ConstructPath(
			otherPlayerCluster->GetNode(), otherPlayerClusterType, otherPlayerVisibleNodes, otherPlayerPathPlan);

		//lets expand further for a better combat simulation
		if (expandNodes)
		{
			for (unsigned int otherPlanIdx = 0; otherPlanIdx < otherPlayerPathPlan.size(); otherPlanIdx++)
			{
				PathingNode* expandOtherPlayerNode = otherPlayerPathPlan[otherPlanIdx].back()->GetNode();

				PathingClusterVec otherPlayerClusters;
				expandOtherPlayerNode->GetClusters(otherPlayerClusterType, otherPlayerClusters);
				unsigned int otherClusterSize = otherPlayerClusters.size() <= 10 ? otherPlayerClusters.size() : 10;
				for (unsigned int otherClusterIdx = 0; otherClusterIdx < otherClusterSize; otherClusterIdx++)
				{
					PathingCluster* expandOtherPlayerCluster = otherPlayerClusters[otherClusterIdx];

					TraverseNode(playerNode, expandOtherPlayerCluster, otherPlayerClusterType, false);
				}
			}
		}
	}
}

void QuakeAIProcess::TraverseNode(PathingCluster* playerCluster, unsigned int playerClusterType, 
	PathingCluster* otherPlayerCluster, unsigned int otherPlayerClusterType, bool expandNodes)
{
	//first we find those nodes which contains actor or/and were visible
	eastl::map<PathingNode*, float> playerVisibleNodes;
	eastl::map<PathingNode*, float> playerNodes;
	playerNodes[playerCluster->GetNode()] = 0.f;

	PathingNode* currentNode = playerCluster->GetNode();
	while (currentNode != playerCluster->GetTarget())
	{
		for (PathingArc* currentArc : currentNode->GetArcs())
			playerNodes[currentArc->GetNode()] = currentArc->GetWeight();

		PathingCluster* currentCluster = 
			currentNode->FindCluster(playerClusterType, playerCluster->GetTarget());
		PathingArc* currentArc = currentNode->FindArc(currentCluster->GetNode());
		playerNodes[currentArc->GetNode()] = currentArc->GetWeight();

		currentNode = currentArc->GetNode();
	}

	eastl::map<PathingNode*, float> otherPlayerVisibleNodes;
	eastl::map<PathingNode*, float> otherPlayerNodes;
	otherPlayerNodes[otherPlayerCluster->GetNode()] = 0.f;

	currentNode = otherPlayerCluster->GetNode();
	while (currentNode != otherPlayerCluster->GetTarget())
	{
		for (PathingArc* currentArc : currentNode->GetArcs())
		{
			otherPlayerNodes[currentArc->GetNode()] = currentArc->GetWeight();
			for (auto playerNode : playerNodes)
			{
				if (currentArc->GetNode()->IsVisibleNode(playerNode.first))
				{
					playerVisibleNodes[playerNode.first] = currentArc->GetWeight();
					otherPlayerVisibleNodes[currentArc->GetNode()] = currentArc->GetWeight();
				}
			}
		}

		PathingCluster* currentCluster = 
			currentNode->FindCluster(otherPlayerClusterType, otherPlayerCluster->GetTarget());
		PathingArc* currentArc = currentNode->FindArc(currentCluster->GetNode());
		otherPlayerNodes[currentArc->GetNode()] = currentArc->GetWeight();
		for (auto playerNode : playerNodes)
		{
			if (currentArc->GetNode()->IsVisibleNode(playerNode.first))
			{
				playerVisibleNodes[playerNode.first] = currentArc->GetWeight();
				otherPlayerVisibleNodes[currentArc->GetNode()] = currentArc->GetWeight();
			}
		}

		currentNode = currentArc->GetNode();
	}

	// next we construct path for those nodes which were visible
	if (!playerVisibleNodes.empty())
	{
		eastl::vector<PathingArcVec> playerPathPlan, otherPlayerPathPlan;
		ConstructPath(playerCluster->GetNode(), 
			playerClusterType, playerVisibleNodes, playerPathPlan);
		ConstructPath(otherPlayerCluster->GetNode(), 
			otherPlayerClusterType, otherPlayerVisibleNodes, otherPlayerPathPlan);

		//lets expand further for a better combat simulation
		if (expandNodes)
		{
			for (unsigned int planIdx = 0; planIdx < playerPathPlan.size(); planIdx++)
			{
				PathingNode* expandPlayerNode = playerPathPlan[planIdx].back()->GetNode();

				PathingClusterVec playerClusters;
				expandPlayerNode->GetClusters(playerClusterType, playerClusters);
				unsigned int clusterSize = playerClusters.size() <= 10 ? playerClusters.size() : 10;
				for (unsigned int clusterIdx = 0; clusterIdx < clusterSize; clusterIdx++)
				{
					for (unsigned int otherPlanIdx = 0; otherPlanIdx < otherPlayerPathPlan.size(); otherPlanIdx++)
					{
						PathingNode* expandOtherPlayerNode = otherPlayerPathPlan[otherPlanIdx].back()->GetNode();

						PathingClusterVec otherPlayerClusters;
						expandOtherPlayerNode->GetClusters(otherPlayerClusterType, otherPlayerClusters);
						unsigned int otherClusterSize = otherPlayerClusters.size() <= 10 ? otherPlayerClusters.size() : 10;
						for (unsigned int otherClusterIdx = 0; otherClusterIdx < otherClusterSize; otherClusterIdx++)
						{
							PathingCluster* expandPlayerCluster = playerClusters[clusterIdx];
							PathingCluster* expandOtherPlayerCluster = otherPlayerClusters[otherClusterIdx];

							TraverseNode(expandPlayerNode, expandOtherPlayerCluster, otherPlayerClusterType, false);
							TraverseNode(expandOtherPlayerNode, expandPlayerCluster, playerClusterType, false);
							TraverseNode(expandPlayerCluster, playerClusterType, expandOtherPlayerCluster, otherPlayerClusterType, false);
						}
					}
				}
			}
		}
	}
}

void QuakeAIProcess::TraverseCluster(
	PathingNode* playerNode, PathingNode* otherPlayerNode, unsigned int* iteration, bool expandClusters)
{
	//single case playerNode - otherPlayerNode
	(*iteration)++;
	printf("it %u \n", (*iteration));

	PathingClusterVec playerClusters;
	unsigned int playerClusterType = GAT_JUMP;
	playerNode->GetClusters(playerClusterType, playerClusters);
	unsigned int clusterSize = playerClusters.size() <= 30 ? playerClusters.size() : 30;
	for (unsigned int playerClusterIdx = 0; playerClusterIdx < clusterSize; playerClusterIdx++)
	{
		PathingCluster* playerCluster = playerClusters[playerClusterIdx];
		TraverseNode(otherPlayerNode, playerCluster, playerClusterType, expandClusters);

		TraverseCluster(playerCluster->GetTarget(), otherPlayerNode, iteration, expandClusters);
	}

	PathingClusterVec otherPlayerClusters;
	unsigned int otherPlayerClusterType = GAT_JUMP;
	otherPlayerNode->GetClusters(otherPlayerClusterType, otherPlayerClusters);
	unsigned int otherClusterSize = otherPlayerClusters.size() <= 30 ? otherPlayerClusters.size() : 30;
	for (unsigned int otherPlayerClusterIdx = 0; otherPlayerClusterIdx < otherClusterSize; otherPlayerClusterIdx++)
	{
		PathingCluster* otherPlayerCluster = otherPlayerNode->GetClusters()[otherPlayerClusterIdx];
		TraverseNode(playerNode, otherPlayerCluster, otherPlayerClusterType, expandClusters);

		TraverseCluster(playerNode, otherPlayerCluster->GetTarget(), iteration, expandClusters);
	}

	for (unsigned int playerClusterIdx = 0; playerClusterIdx < clusterSize; playerClusterIdx++)
	{
		PathingCluster* playerCluster = playerNode->GetClusters()[playerClusterIdx];
		for (unsigned int otherPlayerClusterIdx = 0; otherPlayerClusterIdx < otherClusterSize; otherPlayerClusterIdx++)
		{
			PathingCluster* otherPlayerCluster = otherPlayerNode->GetClusters()[otherPlayerClusterIdx];
			TraverseNode(playerCluster, playerClusterType, otherPlayerCluster, otherPlayerClusterType, expandClusters);

			TraverseCluster(playerCluster->GetTarget(), otherPlayerCluster->GetTarget(), iteration, expandClusters);
		}
	}
}

void QuakeAIProcess::ThreadProc( )
{
	while (true)
	{
		eastl::map<GameViewType, eastl::vector<eastl::shared_ptr<PlayerActor>>> players;

		GameApplication* gameApp = (GameApplication*)Application::App;
		const GameViewList& gameViews = gameApp->GetGameViews();
		for (auto it = gameViews.begin(); it != gameViews.end(); ++it)
		{
			eastl::shared_ptr<BaseGameView> pView = *it;
			if (pView->GetActorId() != INVALID_ACTOR_ID)
			{
				players[pView->GetType()].push_back(
					eastl::dynamic_shared_pointer_cast<PlayerActor>(
					GameLogic::Get()->GetActor(pView->GetActorId()).lock()));
			}
		}

		if (players.find(GV_HUMAN) != players.end() && players.find(GV_AI) != players.end())
		{
			eastl::shared_ptr<PathingGraph> pathingGraph =
				GameLogic::Get()->GetAIManager()->GetPathingGraph();

			eastl::map<eastl::shared_ptr<PlayerActor>, PathingNode*> playerNodes, aiNodes;
			for (eastl::shared_ptr<PlayerActor> pPlayerActor : players[GV_HUMAN])
			{
				eastl::shared_ptr<TransformComponent> pTransformComponent(
					pPlayerActor->GetComponent<TransformComponent>(TransformComponent::Name).lock());
				if (pTransformComponent)
				{
					playerNodes[pPlayerActor] = 
						pathingGraph->FindClosestNode(pTransformComponent->GetPosition());
				}
			}

			for (eastl::shared_ptr<PlayerActor> pPlayerActor : players[GV_AI])
			{
				eastl::shared_ptr<TransformComponent> pTransformComponent(
					pPlayerActor->GetComponent<TransformComponent>(TransformComponent::Name).lock());
				if (pTransformComponent)
				{
					aiNodes[pPlayerActor] =
						pathingGraph->FindClosestNode(pTransformComponent->GetPosition());
				}
			}

			unsigned int iteration = 0;
			for (auto playerNode : playerNodes)
			{
				for (auto aiNode : aiNodes)
				{
					TraverseCluster(playerNode.second, aiNode.second, &iteration, true);
				}
			}
		}
		/*
		EventManager::Get()->QueueEvent(
			eastl::shared_ptr<QuakeEventDataAIDecisionMaking>(new QuakeEventDataAIDecisionMaking()));
		*/
	}

	Succeed();
}

/*
void SimulateVisibility()
{
	//visibility between pathing transitions
	float totalTime = 0.f, totalVisibleTime = 0.f, distance = 0.f;
	if (visibleArc->GetWeight() > pathArc->GetWeight())
	{
		float totalArcTime = 0.f;
		unsigned int pathConnection = 0;
		unsigned int visibleConnection = 0;
		eastl::vector<Vector3<float>> pathConnections = pathTransition->GetConnections();
		eastl::vector<Vector3<float>> visibleConnections = visibleTransition->GetConnections();
		for (; pathConnection < pathConnections.size(); pathConnection++)
		{
			if (visibleNodes[][pathingNodes[visibleConnections[visibleConnection]]])
			{
				float deltaTime = pathTransition->GetWeights()[pathConnection];
				distance += Length(visibleConnections[visibleConnection] - pathConnections[pathConnection]) * deltaTime;
				totalVisibleTime += deltaTime;
			}
			while (totalArcTime <= totalTime)
			{
				totalArcTime += visibleTransition->GetWeights()[visibleConnection];
				if (visibleConnection + 1 < visibleConnections.size())
					visibleConnection++;
				else
					break;
			}
			totalTime += pathTransition->GetWeights()[pathConnection];
		}

		pathConnection--;
		for (; visibleConnection < visibleConnections.size(); visibleConnection++)
		{
			if (visibleNodes[pathingNodes[pathConnections[pathConnection]]][pathingNodes[visibleConnections[visibleConnection]]])
			{
				float deltaTime = pathTransition->GetWeights()[pathConnection];
				distance += Length(visibleConnections[visibleConnection] - pathConnections[pathConnection]) * deltaTime;
				totalVisibleTime += deltaTime;
			}
		}
	}

	if (totalVisibleTime > 0.f)
	{
		distance /= totalVisibleTime;

		pathArc->AddVisibility(visibleArc, VT_DISTANCE, distance);
		pathArc->AddVisibility(visibleArc, VT_WEIGHT, totalVisibleTime);
	}
}
*/