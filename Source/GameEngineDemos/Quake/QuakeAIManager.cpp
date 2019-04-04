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

#include "QuakeAIManager.h"

#include "Core/OS/OS.h"
#include "Core/Logger/Logger.h"
#include "Core/IO/XmlResource.h"
#include "Core/Event/EventManager.h"
#include "Core/Event/Event.h"

#include "Physic/PhysicEventListener.h"

#include "QuakeEvents.h"
#include "QuakeView.h"
#include "QuakeApp.h"
#include "Quake.h"

QuakeAIManager::QuakeAIManager() : AIManager()
{
	mLastArcId = 0;
	mLastNodeId = 0;
	mLastClusterArcId = 0;

	mYaw = 0.0f;
	mPitchTarget = 0.0f;

	mMaxJumpSpeed = 3.4f;
	mMaxFallSpeed = 240.0f;
	mMaxRotateSpeed = 180.0f;
	mMoveSpeed = 6.0f;
	mJumpSpeed = 3.4f;
	mJumpMoveSpeed = 10.0f;
	mFallSpeed = 0.0f;
	mRotateSpeed = 0.0f;
}   // QuakeAIManager

//-----------------------------------------------------------------------------

QuakeAIManager::~QuakeAIManager()
{

}   // ~QuakeAIManager

/////////////////////////////////////////////////////////////////////////////
// QuakeAIManager::LoadPathingGraph
//
//    Loads the AI pathing graph information from an XML file
//
void QuakeAIManager::LoadPathingGraph(const eastl::wstring& path)
{
	mPathingGraph = eastl::make_shared<PathingGraph>();

	// Load the map graph file
	tinyxml2::XMLElement* pRoot = XmlResourceLoader::LoadAndReturnRootXMLElement(path.c_str());
	LogAssert(pRoot, "AI xml doesn't exists");

	eastl::map<unsigned int, PathingNode*> pathingNodeGraph;
	for (tinyxml2::XMLElement* pNode = pRoot->FirstChildElement(); pNode; pNode = pNode->NextSiblingElement())
	{
		int pathNodeId = 0;
		ActorId actorId = INVALID_ACTOR_ID;
		float tolerance = PATHING_DEFAULT_NODE_TOLERANCE;
		Vector3<float> position = Vector3<float>::Zero();

		pathNodeId = pNode->IntAttribute("id", pathNodeId);
		actorId = pNode->IntAttribute("actorid", actorId);
		tolerance = pNode->FloatAttribute("tolerance", tolerance);

		tinyxml2::XMLElement* pPositionElement = pNode->FirstChildElement("Position");
		if (pPositionElement)
		{
			float x = 0;
			float y = 0;
			float z = 0;
			x = pPositionElement->FloatAttribute("x", x);
			y = pPositionElement->FloatAttribute("y", y);
			z = pPositionElement->FloatAttribute("z", z);

			position = Vector3<float>{ x, y, z };
		}

		PathingNode* pathNode = new PathingNode(pathNodeId, actorId, position, tolerance);
		mPathingGraph->InsertNode(pathNode);

		pathingNodeGraph[pathNodeId] = pathNode;
	}

	for (tinyxml2::XMLElement* pNode = pRoot->FirstChildElement(); pNode; pNode = pNode->NextSiblingElement())
	{
		int pathNodeId = 0;
		pathNodeId = pNode->IntAttribute("id", pathNodeId);
		PathingNode* pathNode = pathingNodeGraph[pathNodeId];

		for (tinyxml2::XMLElement* pArc = pNode->FirstChildElement("Arc"); pArc; pArc = pArc->NextSiblingElement())
		{
			int arcId = 0;
			int arcType = 0;
			int arcNode = 0;
			float weight = 0.f;

			arcId = pArc->IntAttribute("id", arcId);
			arcType = pArc->IntAttribute("type", arcType);
			arcNode = pArc->IntAttribute("node", arcNode);
			weight = pArc->FloatAttribute("weight", weight);

			PathingArc* pathArc = new PathingArc(arcId, arcType, pathingNodeGraph[arcNode], weight);
			pathNode->AddArc(pathArc);

			for (tinyxml2::XMLElement* pTransition = pArc->FirstChildElement("Transition"); pTransition; pTransition = pTransition->NextSiblingElement())
			{
				int transitionId = 0;
				int transitionType = 0;
				int transitionNode = 0;
				eastl::vector<float> transitionWeights;
				eastl::vector<Vector3<float>> transitionConnections;

				transitionId = pTransition->IntAttribute("id", transitionId);
				transitionType = pTransition->IntAttribute("type", transitionType);
				transitionNode = pTransition->IntAttribute("node", transitionNode);

				for (tinyxml2::XMLElement* pElement= pTransition->FirstChildElement(); pElement; pElement = pElement->NextSiblingElement())
				{
					if (pElement->Name() == "Weight")
					{
						float transitionWeight = 0;
						transitionWeight = pElement->FloatAttribute("value", transitionWeight);
						transitionWeights.push_back(transitionWeight);
					}
					else if (pElement->Name() == "Connection")
					{
						float x = 0;
						float y = 0;
						float z = 0;
						x = pElement->FloatAttribute("x", x);
						y = pElement->FloatAttribute("y", y);
						z = pElement->FloatAttribute("z", z);

						transitionConnections.push_back(Vector3<float>{ x, y, z });
					}
				}

				PathingTransition* pathTransition = new PathingTransition(
					transitionId, transitionType, pathingNodeGraph[transitionNode], transitionWeights, transitionConnections);
				pathNode->AddTransition(pathTransition);
			}
		}
	}
}

/////////////////////////////////////////////////////////////////////////////
// QuakeAIManager::SavePathingGraph
//
//    Saves the AI pathing graph information to an XML file
//
void QuakeAIManager::SavePathingGraph(const eastl::string& path)
{
	tinyxml2::XMLDocument doc;

	// base element
	tinyxml2::XMLElement* pBaseElement = doc.NewElement("PathingGraph");
	doc.InsertFirstChild(pBaseElement);

	for (PathingNode* pathNode : mPathingGraph->GetNodes())
	{
		tinyxml2::XMLElement* pNode = doc.NewElement("Node");
		pNode->SetAttribute("id", eastl::to_string(pathNode->GetId()).c_str());
		pNode->SetAttribute("actorid", eastl::to_string(pathNode->GetActorId()).c_str());
		pNode->SetAttribute("tolerance", eastl::to_string(pathNode->GetTolerance()).c_str());
		pBaseElement->LinkEndChild(pNode);

		tinyxml2::XMLElement* pPosition = doc.NewElement("Position");
		pPosition->SetAttribute("x", eastl::to_string((int)round(pathNode->GetPos()[0])).c_str());
		pPosition->SetAttribute("y", eastl::to_string((int)round(pathNode->GetPos()[1])).c_str());
		pPosition->SetAttribute("z", eastl::to_string((int)round(pathNode->GetPos()[2])).c_str());
		pNode->LinkEndChild(pPosition);

		for (PathingArc* pathArc : pathNode->GetArcs())
		{
			tinyxml2::XMLElement* pArc = doc.NewElement("Arc");
			pArc->SetAttribute("id", eastl::to_string(pathArc->GetId()).c_str());
			pArc->SetAttribute("type", eastl::to_string(pathArc->GetType()).c_str());
			pArc->SetAttribute("node", eastl::to_string(pathArc->GetNode()->GetId()).c_str());
			pArc->SetAttribute("weight", eastl::to_string(pathArc->GetWeight()).c_str());
			pNode->LinkEndChild(pArc);

			PathingTransition* pathTransition = pathNode->FindTransition(pathArc->GetId());
			if (pathTransition)
			{
				tinyxml2::XMLElement* pTransition = doc.NewElement("Transition");
				pTransition->SetAttribute("id", eastl::to_string(pathTransition->GetId()).c_str());
				pTransition->SetAttribute("type", eastl::to_string(pathTransition->GetType()).c_str());
				pTransition->SetAttribute("node", eastl::to_string(pathTransition->GetNode()->GetId()).c_str());
				pArc->LinkEndChild(pTransition);

				for (float weight : pathTransition->GetWeights())
				{
					tinyxml2::XMLElement* pConnection = doc.NewElement("Weight");
					pConnection->SetAttribute("value", eastl::to_string(weight).c_str());
					pTransition->LinkEndChild(pConnection);
				}

				for (Vector3<float> connection : pathTransition->GetConnections())
				{
					tinyxml2::XMLElement* pConnection = doc.NewElement("Connection");
					pConnection->SetAttribute("x", eastl::to_string((int)round(connection[0])).c_str());
					pConnection->SetAttribute("y", eastl::to_string((int)round(connection[1])).c_str());
					pConnection->SetAttribute("z", eastl::to_string((int)round(connection[2])).c_str());
					pTransition->LinkEndChild(pConnection);
				}
			}
		}
	}

	doc.SaveFile(path.c_str());
}


/////////////////////////////////////////////////////////////////////////////
// QuakeAIManager::LoadClusteringGraph
//
//    Loads the AI clustering graph information from an XML file
//
void QuakeAIManager::LoadClusteringGraph(const eastl::wstring& path)
{
	mClusteringGraph = eastl::make_shared<ClusteringGraph>();

	// Load the map graph file
	tinyxml2::XMLElement* pRoot = XmlResourceLoader::LoadAndReturnRootXMLElement(path.c_str());
	LogAssert(pRoot, "AI xml doesn't exists");

	eastl::map<unsigned int, Cluster*> clusteringGraph;
	eastl::map<unsigned int, ClusteringNode*> clusteringNodes;
	for (tinyxml2::XMLElement* pCluster = pRoot->FirstChildElement(); pCluster; pCluster = pCluster->NextSiblingElement())
	{
		int clusterId = 0;
		int centerId = 0;
		clusterId = pCluster->IntAttribute("id", clusterId);
		centerId = pCluster->IntAttribute("centerid", centerId);

		Cluster* cluster = new Cluster(clusterId);
		mClusteringGraph->InsertCluster(cluster);

		clusteringGraph[clusterId] = cluster;

		for (tinyxml2::XMLElement* pClusterNode = pCluster->FirstChildElement("Node"); pClusterNode; pClusterNode = pClusterNode->NextSiblingElement())
		{
			int clusterNodeId = 0;
			bool isIsolatedNode = false;
			ActorId actorId = INVALID_ACTOR_ID;
			Vector3<float> position = Vector3<float>::Zero();

			clusterNodeId = pClusterNode->IntAttribute("id", clusterNodeId);
			actorId = pClusterNode->IntAttribute("actorid", actorId);
			isIsolatedNode = pClusterNode->BoolAttribute("isisolated", isIsolatedNode);

			tinyxml2::XMLElement* pPositionElement = pClusterNode->FirstChildElement("Position");
			if (pPositionElement)
			{
				float x = 0;
				float y = 0;
				float z = 0;
				x = pPositionElement->FloatAttribute("x", x);
				y = pPositionElement->FloatAttribute("y", y);
				z = pPositionElement->FloatAttribute("z", z);

				position = Vector3<float>{ x, y, z };
			}

			ClusteringNode* clusterNode = new ClusteringNode(clusterNodeId, position);
			clusterNode->SetActor(actorId);
			clusterNode->SetCluster(cluster);
			cluster->InsertNode(clusterNode);
			cluster->AddActor(actorId);
			if (isIsolatedNode)
				cluster->InsertIsolatedNode(clusterNode);

			clusteringNodes[clusterNodeId] = clusterNode;
		}
		cluster->SetCenter(clusteringNodes[centerId]);

	}

	for (tinyxml2::XMLElement* pCluster = pRoot->FirstChildElement(); pCluster; pCluster = pCluster->NextSiblingElement())
	{
		int clusterId = 0;
		clusterId = pCluster->IntAttribute("id", clusterId);
		Cluster* cluster = clusteringGraph[clusterId];

		for (tinyxml2::XMLElement* pClusterArc = pCluster->FirstChildElement("Arc"); pClusterArc; pClusterArc = pClusterArc->NextSiblingElement())
		{
			int arcId = 0;
			int arcType = 0;
			int arcCluster = 0;

			arcId = pClusterArc->IntAttribute("id", arcId);
			arcType = pClusterArc->IntAttribute("type", arcType);
			arcCluster = pClusterArc->IntAttribute("cluster", arcCluster);

			ClusterArc* clusterArc = new ClusterArc(arcId, arcType, clusteringGraph[arcCluster]);
			cluster->AddArc(clusterArc);
		}

		for (tinyxml2::XMLElement* pClusterNode = pCluster->FirstChildElement("Node"); pClusterNode; pClusterNode = pClusterNode->NextSiblingElement())
		{
			int clusterNodeId = 0;
			clusterNodeId = pClusterNode->IntAttribute("id", clusterNodeId);
			ClusteringNode* clusterNode = clusteringNodes[clusterNodeId];

			for (tinyxml2::XMLElement* pClusterNodeArc = pClusterNode->FirstChildElement("Arc"); pClusterNodeArc; pClusterNodeArc = pClusterNodeArc->NextSiblingElement())
			{
				int arcId = 0;
				int arcType = 0;
				int arcNode = 0;
				float weight = 0.f;

				arcId = pClusterNodeArc->IntAttribute("id", arcId);
				arcType = pClusterNodeArc->IntAttribute("type", arcType);
				arcNode = pClusterNodeArc->IntAttribute("node", arcNode);
				weight = pClusterNodeArc->FloatAttribute("weight", weight);

				ClusteringArc* clusterArc = new ClusteringArc(arcId, arcType, clusteringNodes[arcNode], weight);
				clusterNode->AddArc(clusterArc);

				for (tinyxml2::XMLElement* pTransition = pClusterNodeArc->FirstChildElement("Transition"); pTransition; pTransition = pTransition->NextSiblingElement())
				{
					int transitionId = 0;
					int transitionType = 0;
					int transitionNode = 0;
					eastl::vector<float> transitionWeights;
					eastl::vector<Vector3<float>> transitionConnections;

					transitionId = pTransition->IntAttribute("id", transitionId);
					transitionType = pTransition->IntAttribute("type", transitionType);
					transitionNode = pTransition->IntAttribute("node", transitionNode);

					for (tinyxml2::XMLElement* pElement = pTransition->FirstChildElement(); pElement; pElement = pElement->NextSiblingElement())
					{
						if (pElement->Name() == "Weight")
						{
							float transitionWeight = 0;
							transitionWeight = pElement->FloatAttribute("value", transitionWeight);
							transitionWeights.push_back(transitionWeight);
						}
						else if (pElement->Name() == "Connection")
						{
							float x = 0;
							float y = 0;
							float z = 0;
							x = pElement->FloatAttribute("x", x);
							y = pElement->FloatAttribute("y", y);
							z = pElement->FloatAttribute("z", z);

							transitionConnections.push_back(Vector3<float>{ x, y, z });
						}
					}

					ClusteringTransition* clusterTransition = new ClusteringTransition(
						transitionId, transitionType, clusteringNodes[transitionNode], transitionWeights, transitionConnections);
					clusterNode->AddTransition(clusterTransition);
				}
			}
		}
	}

	/*
	tinyxml2::XMLElement* pVisibilityGraph = pRoot->FirstChildElement("VisibilityGraph");
	if (pVisibilityGraph != NULL)
	{
		for (tinyxml2::XMLElement* pElement = pVisibilityGraph->FirstChildElement(); pElement; pElement = pElement->NextSiblingElement())
		{
			if (pElement->Name() == "VisibleNode")
			{
				int pathNodeId = 0;
				pathNodeId = pElement->IntAttribute("id", pathNodeId);

				for (tinyxml2::XMLElement* pVisibleNode = pElement->FirstChildElement("Node"); pVisibleNode; pVisibleNode = pVisibleNode->NextSiblingElement())
				{
					int visibleNodeId = 0;
					visibleNodeId = pVisibleNode->IntAttribute("id", visibleNodeId);

					float distance = 0.f;
					distance = pVisibleNode->FloatAttribute("distance", distance);

					mVisibleNodes[pathingNodeGraph[pathNodeId]][pathingNodeGraph[visibleNodeId]] = distance;
				}
			}
			else if (pElement->Name() == "VisibleNodeArc")
			{
				int pathNodeId = 0;
				pathNodeId = pElement->IntAttribute("id", pathNodeId);

				for (tinyxml2::XMLElement* pVisibleArc = pElement->FirstChildElement("Arc"); pVisibleArc; pVisibleArc = pVisibleArc->NextSiblingElement())
				{
					int visibleArcId = 0;
					visibleArcId = pVisibleArc->IntAttribute("id", visibleArcId);

					float distance = 0.f;
					distance = pVisibleArc->FloatAttribute("distance", distance);

					float time = 0.f;
					time = pVisibleArc->FloatAttribute("time", time);

					mVisibleNodeArcs[pathingNodeGraph[pathNodeId]][pathingArcGraph[visibleArcId]] = distance;
					mVisibleNodeArcsTime[pathingNodeGraph[pathNodeId]][pathingArcGraph[visibleArcId]] = time;
				}
			}
			else if (pElement->Name() == "VisibleArcNode")
			{
				int pathArcId = 0;
				pathArcId = pElement->IntAttribute("id", pathArcId);

				for (tinyxml2::XMLElement* pVisibleNode = pElement->FirstChildElement("Node"); pVisibleNode; pVisibleNode = pVisibleNode->NextSiblingElement())
				{
					int visibleNodeId = 0;
					visibleNodeId = pVisibleNode->IntAttribute("id", visibleNodeId);

					float distance = 0.f;
					distance = pVisibleNode->FloatAttribute("distance", distance);

					float time = 0.f;
					time = pVisibleNode->FloatAttribute("time", time);

					mVisibleArcNodes[pathingArcGraph[pathArcId]][pathingNodeGraph[visibleNodeId]] = distance;
					mVisibleArcNodesTime[pathingArcGraph[pathArcId]][pathingNodeGraph[visibleNodeId]] = time;
				}
			}
			else if (pElement->Name() == "VisibleArc")
			{
				int pathArcId = 0;
				pathArcId = pElement->IntAttribute("id", pathArcId);

				for (tinyxml2::XMLElement* pVisibleArc = pElement->FirstChildElement("Arc"); pVisibleArc; pVisibleArc = pVisibleArc->NextSiblingElement())
				{
					int visibleArcId = 0;
					visibleArcId = pVisibleArc->IntAttribute("id", visibleArcId);

					float distance = 0.f;
					distance = pVisibleArc->FloatAttribute("distance", distance);

					float time = 0.f;
					time = pVisibleArc->FloatAttribute("time", time);

					mVisibleArcs[pathingArcGraph[pathArcId]][pathingArcGraph[visibleArcId]] = distance;
					mVisibleArcsTime[pathingArcGraph[pathArcId]][pathingArcGraph[visibleArcId]] = time;
				}
			}
		}
	}
	*/
}

/////////////////////////////////////////////////////////////////////////////
// QuakeAIManager::SaveClusteringGraph
//
//    Saves the AI clustering graph information to an XML file
//
void QuakeAIManager::SaveClusteringGraph(const eastl::string& path)
{
	tinyxml2::XMLDocument doc;

	// base element
	tinyxml2::XMLElement* pBaseElement = doc.NewElement("ClusteringGraph");
	doc.InsertFirstChild(pBaseElement);

	for (Cluster* cluster : mClusteringGraph->GetClusters())
	{
		tinyxml2::XMLElement* pCluster = doc.NewElement("Cluster");
		pCluster->SetAttribute("id", eastl::to_string(cluster->GetId()).c_str());
		pCluster->SetAttribute("centerid", eastl::to_string(cluster->GetCenter()->GetId()).c_str());
		pBaseElement->LinkEndChild(pCluster);

		for (ClusterArc* clusterArc : cluster->GetArcs())
		{
			tinyxml2::XMLElement* pArc = doc.NewElement("Arc");
			pArc->SetAttribute("id", eastl::to_string(clusterArc->GetId()).c_str());
			pArc->SetAttribute("type", eastl::to_string(clusterArc->GetType()).c_str());
			pArc->SetAttribute("cluster", eastl::to_string(clusterArc->GetCluster()->GetId()).c_str());
			pCluster->LinkEndChild(pArc);
		}

		for (ClusteringNode* clusterNode : cluster->GetNodes())
		{
			tinyxml2::XMLElement* pNode = doc.NewElement("Node");
			pNode->SetAttribute("id", eastl::to_string(clusterNode->GetId()).c_str());
			pNode->SetAttribute("actorid", eastl::to_string(clusterNode->GetActor()).c_str());
			pNode->SetAttribute("isisolated", eastl::to_string(cluster->IsIsolatedNode(clusterNode)).c_str());
			pCluster->LinkEndChild(pNode);

			tinyxml2::XMLElement* pPosition = doc.NewElement("Position");
			pPosition->SetAttribute("x", eastl::to_string((int)round(clusterNode->GetPos()[0])).c_str());
			pPosition->SetAttribute("y", eastl::to_string((int)round(clusterNode->GetPos()[1])).c_str());
			pPosition->SetAttribute("z", eastl::to_string((int)round(clusterNode->GetPos()[2])).c_str());
			pNode->LinkEndChild(pPosition);

			for (ClusteringArc* clusterNodeArc : clusterNode->GetArcs())
			{
				tinyxml2::XMLElement* pNodeArc = doc.NewElement("Arc");
				pNodeArc->SetAttribute("id", eastl::to_string(clusterNodeArc->GetId()).c_str());
				pNodeArc->SetAttribute("type", eastl::to_string(clusterNodeArc->GetType()).c_str());
				pNodeArc->SetAttribute("node", eastl::to_string(clusterNodeArc->GetNode()->GetId()).c_str());
				pNodeArc->SetAttribute("weight", eastl::to_string(clusterNodeArc->GetWeight()).c_str());
				pNode->LinkEndChild(pNodeArc);

				ClusteringTransition* clusterTransition = clusterNode->FindTransition(clusterNodeArc->GetId());
				if (clusterTransition)
				{
					tinyxml2::XMLElement* pTransition = doc.NewElement("Transition");
					pTransition->SetAttribute("id", eastl::to_string(clusterTransition->GetId()).c_str());
					pTransition->SetAttribute("type", eastl::to_string(clusterTransition->GetType()).c_str());
					pTransition->SetAttribute("node", eastl::to_string(clusterTransition->GetNode()->GetId()).c_str());
					pNodeArc->LinkEndChild(pTransition);

					for (float weight : clusterTransition->GetWeights())
					{
						tinyxml2::XMLElement* pConnection = doc.NewElement("Weight");
						pConnection->SetAttribute("value", eastl::to_string(weight).c_str());
						pTransition->LinkEndChild(pConnection);
					}

					for (Vector3<float> connection : clusterTransition->GetConnections())
					{
						tinyxml2::XMLElement* pConnection = doc.NewElement("Connection");
						pConnection->SetAttribute("x", eastl::to_string((int)round(connection[0])).c_str());
						pConnection->SetAttribute("y", eastl::to_string((int)round(connection[1])).c_str());
						pConnection->SetAttribute("z", eastl::to_string((int)round(connection[2])).c_str());
						pTransition->LinkEndChild(pConnection);
					}
				}
			}
		}
	}
	/*
	tinyxml2::XMLElement* pVisibilityGraph = doc.NewElement("VisibilityGraph");
	pBaseElement->LinkEndChild(pVisibilityGraph);

	for (PathingNode* pathNode : mPathingGraph->GetNodes())
	{
		if (mVisibleNodes.find(pathNode) != mVisibleNodes.end())
		{
			tinyxml2::XMLElement* pNode = doc.NewElement("VisibleNode");
			pNode->SetAttribute("id", eastl::to_string(pathNode->GetId()).c_str());
			pVisibilityGraph->LinkEndChild(pNode);

			for (auto visibilityNode : mVisibleNodes[pathNode])
			{
				PathingNode* visibleNode = visibilityNode.first;
				tinyxml2::XMLElement* pVisibleNode = doc.NewElement("Node");
				pVisibleNode->SetAttribute("id", eastl::to_string(visibleNode->GetId()).c_str());
				pVisibleNode->SetAttribute("distance", eastl::to_string(visibilityNode.second).c_str());
				pNode->LinkEndChild(pVisibleNode);
			}
		}
	}

	for (PathingNode* pathNode : mPathingGraph->GetNodes())
	{
		if (mVisibleNodeArcs.find(pathNode) != mVisibleNodeArcs.end())
		{
			tinyxml2::XMLElement* pNode = doc.NewElement("VisibleNodeArc");
			pNode->SetAttribute("id", eastl::to_string(pathNode->GetId()).c_str());
			pVisibilityGraph->LinkEndChild(pNode);

			for (auto visibilityNodeArc : mVisibleNodeArcs[pathNode])
			{
				PathingArc* visibleArc = visibilityNodeArc.first;

				tinyxml2::XMLElement* pArc = doc.NewElement("Arc");
				pArc->SetAttribute("id", eastl::to_string(visibleArc->GetId()).c_str());
				pArc->SetAttribute("distance", eastl::to_string(visibilityNodeArc.second).c_str());
				pArc->SetAttribute("time", eastl::to_string(mVisibleNodeArcsTime[pathNode][visibleArc]).c_str());
				pNode->LinkEndChild(pArc);
			}
		}
	}

	for (PathingArc* pathArc : mPathingGraph->GetArcs())
	{
		if (mVisibleArcNodes.find(pathArc) != mVisibleArcNodes.end())
		{
			tinyxml2::XMLElement* pArc = doc.NewElement("VisibleArcNode");
			pArc->SetAttribute("id", eastl::to_string(pathArc->GetId()).c_str());
			pVisibilityGraph->LinkEndChild(pArc);

			for (auto visibilityArcNode : mVisibleArcNodes[pathArc])
			{
				PathingNode* visibleNode = visibilityArcNode.first;
				
				tinyxml2::XMLElement* pNode = doc.NewElement("Node");
				pNode->SetAttribute("id", eastl::to_string(visibleNode->GetId()).c_str());
				pArc->SetAttribute("distance", eastl::to_string(visibilityArcNode.second).c_str());
				pArc->SetAttribute("time", eastl::to_string(mVisibleArcNodesTime[pathArc][visibleNode]).c_str());
				pArc->LinkEndChild(pNode);
			}
		}
	}

	for (PathingArc* pathArc : mPathingGraph->GetArcs())
	{
		if (mVisibleArcNodes.find(pathArc) != mVisibleArcNodes.end())
		{
			tinyxml2::XMLElement* pArc = doc.NewElement("VisibleArc");
			pArc->SetAttribute("id", eastl::to_string(pathArc->GetId()).c_str());
			pVisibilityGraph->LinkEndChild(pArc);

			for (auto visibilityArc : mVisibleArcs[pathArc])
			{
				PathingArc* visibleArc = visibilityArc.first;

				tinyxml2::XMLElement* pVisibleArc = doc.NewElement("Arc");
				pVisibleArc->SetAttribute("id", eastl::to_string(visibleArc->GetId()).c_str());
				pVisibleArc->SetAttribute("distance", eastl::to_string(visibilityArc.second).c_str());
				pVisibleArc->SetAttribute("time", eastl::to_string(mVisibleArcsTime[pathArc][visibleArc]).c_str());
				pArc->LinkEndChild(pVisibleArc);
			}
		}
	}
	*/
	doc.SaveFile(path.c_str());
}

//map generation via physics simulation
void QuakeAIManager::CreateMap(ActorId playerId)
{
	GameApplication* gameApp = (GameApplication*)Application::App;
	QuakeLogic* game = static_cast<QuakeLogic *>(GameLogic::Get());

	mPlayerActor = 
		eastl::dynamic_shared_pointer_cast<PlayerActor>(
		GameLogic::Get()->GetActor(playerId).lock());

	game->GetGamePhysics()->SetTriggerCollision(true);
	game->RemoveAllDelegates();
	RegisterAllDelegates();

	mPathingGraph = eastl::make_shared<PathingGraph>();

	eastl::vector<eastl::shared_ptr<Actor>> actors;
	game->GetAmmoActors(actors);
	game->GetWeaponActors(actors);
	game->GetHealthActors(actors);
	game->GetArmorActors(actors);
	game->GetTargetActors(actors);
	for (auto actor : actors)
	{
		eastl::shared_ptr<TransformComponent> pTransformComponent(
			actor->GetComponent<TransformComponent>(TransformComponent::Name).lock());
		if (pTransformComponent)
			SimulateActorPosition(actor->GetId(), pTransformComponent->GetPosition());
	}

	// we create the waypoint according to the character controller physics system. Every
	// simulation step, it will be generated new waypoints from different actions such as
	// movement, jumping or falling and its conections
	SimulateWaypoint();

	GameLogic::Get()->GetAIManager()->SavePathingGraph(
		FileSystem::Get()->GetPath("ai/quake/bloodrunpathing.xml"));

	// we group the graph nodes in clusters
	CreateClusters();

	GameLogic::Get()->GetAIManager()->SaveClusteringGraph(
		FileSystem::Get()->GetPath("ai/quake/bloodrunclustering.xml"));

	// we obtain visibility information from the cluster graph by using raycasting
	SimulateVisibility();

	// we need to handle firing grenades separately since they cannot be simulated by raycasting 
	// as they describe different trajectories
	/*
	eastl::shared_ptr<Actor> pGameActor(
		GameLogic::Get()->CreateActor("actors/quake/effects/simulategrenadelauncherfire.xml", nullptr));
	mActorCollisions[pGameActor->GetId()] = false;
	eastl::shared_ptr<PhysicComponent> pPhysicalComponent =
		pGameActor->GetComponent<PhysicComponent>(PhysicComponent::Name).lock();
	if (pPhysicalComponent)
		pPhysicalComponent->SetIgnoreCollision(mPlayerActor->GetId(), true);
	for (PathingNode* pNode : mPathingGraph->GetNodes())
		SimulateGrenadeLauncherFire(pNode, pGameActor);
	EventManager::Get()->TriggerEvent(
		eastl::make_shared<EventDataRequestDestroyActor>(pGameActor->GetId()));
	*/
	mActorCollisions.clear();
	mActorNodes.clear();

	game->GetGamePhysics()->SetTriggerCollision(false);
	game->RegisterAllDelegates();

	RemoveAllDelegates();
}

void QuakeAIManager::SimulateVisibility()
{
	eastl::shared_ptr<BaseGamePhysic> gamePhysics = GameLogic::Get()->GetGamePhysics();

	Transform transform;
	PathingDoubleMap visibleNodesTime;
	PathingDoubleMap visibleNodes;

	// first we need to get visibility info such as distance and timing 
	// from every node and arc by raycasting
	for (PathingNode* pathNode : mPathingGraph->GetNodes())
	{
		//set muzzle location relative to pivoting eye
		Vector3<float> muzzle = pathNode->GetPos();
		muzzle[2] += mPlayerActor->GetState().viewHeight;
		muzzle -= Vector3<float>::Unit(ROLL) * 11.f;

		for (PathingNode* visibleNode : mPathingGraph->GetNodes())
		{
			Vector3<float> end = visibleNode->GetPos() +
				(float)mPlayerActor->GetState().viewHeight * Vector3<float>::Unit(YAW);

			eastl::vector<ActorId> collisionActors;
			eastl::vector<Vector3<float>> collisions, collisionNormals;
			gamePhysics->CastRay(muzzle, end, collisionActors, collisions, collisionNormals);

			Vector3<float> collision = NULL;
			for (unsigned int i = 0; i < collisionActors.size(); i++)
				if (collisionActors[i] == INVALID_ACTOR_ID)
					collision = collisions[i];

			if (collision == NULL)
				visibleNodes[pathNode->GetPos()][visibleNode->GetPos()] = 1.f;
		}

		for (PathingNode* visibleNode : mPathingGraph->GetNodes())
		{
			for (PathingTransition* visibleTransition : visibleNode->GetTransitions())
			{
				for (Vector3<float> visibleConnection : visibleTransition->GetConnections())
				{
					Vector3<float> end = visibleConnection +
						(float)mPlayerActor->GetState().viewHeight * Vector3<float>::Unit(YAW);

					eastl::vector<ActorId> collisionActors;
					eastl::vector<Vector3<float>> collisions, collisionNormals;
					gamePhysics->CastRay(muzzle, end, collisionActors, collisions, collisionNormals);

					Vector3<float> collision = NULL;
					for (unsigned int i = 0; i < collisionActors.size(); i++)
						if (collisionActors[i] == INVALID_ACTOR_ID)
							collision = collisions[i];

					if (collision == NULL)
						visibleNodes[pathNode->GetPos()][visibleConnection] = 1.0f;
				}
			}
		}

		for (PathingTransition* pathTransition : pathNode->GetTransitions())
		{
			for (Vector3<float> pathConnection : pathTransition->GetConnections())
			{
				muzzle = pathConnection;
				muzzle[2] += mPlayerActor->GetState().viewHeight;
				muzzle -= Vector3<float>::Unit(ROLL) * 11.f;

				for (PathingNode* visibleNode : mPathingGraph->GetNodes())
				{
					for (PathingTransition* visibleTransition : visibleNode->GetTransitions())
					{
						for (Vector3<float> visibleConnection : visibleTransition->GetConnections())
						{
							Vector3<float> end = visibleConnection +
								(float)mPlayerActor->GetState().viewHeight * Vector3<float>::Unit(YAW);

							eastl::vector<ActorId> collisionActors;
							eastl::vector<Vector3<float>> collisions, collisionNormals;
							gamePhysics->CastRay(muzzle, end, collisionActors, collisions, collisionNormals);

							Vector3<float> collision = NULL;
							for (unsigned int i = 0; i < collisionActors.size(); i++)
								if (collisionActors[i] == INVALID_ACTOR_ID)
									collision = collisions[i];

							if (collision == NULL)
								visibleNodes[pathConnection][visibleConnection] = 1.0f;
						}
					}
				}
			}
		}
	}

	// now we fill all the visibility graph from nodes to arcs, 
	// arcs to nodes, arcs to arcs and between nodes
	for (PathingNode* pathNode : mPathingGraph->GetNodes())
	{
		//between nodes
		for (PathingNode* visibleNode : mPathingGraph->GetNodes())
		{
			if (visibleNodes[pathNode->GetPos()][visibleNode->GetPos()])
			{
				/*
				mVisibleNodes[pathNode][visibleNode] = 
					Length(visibleNode->GetPos() - pathNode->GetPos());
				*/
			}
		}

		//nodes to arcs
		for (PathingNode* visibleNode : mPathingGraph->GetNodes())
		{
			for (PathingArc* visibleArc : visibleNode->GetArcs())
			{
				PathingTransition* visibleTransition = visibleNode->FindTransition(visibleArc->GetId());
				if (visibleTransition)
				{
					unsigned int idx = 0;
					float totalTime = 0.f, distance = 0.f;

					for (Vector3<float> visibleConnection : visibleTransition->GetConnections())
					{
						if (visibleNodes[pathNode->GetPos()][visibleConnection])
						{
							float deltaTime = visibleTransition->GetWeights()[idx];
							distance += Length(visibleConnection - pathNode->GetPos()) * deltaTime;
							totalTime += deltaTime;
						}
						if (totalTime > 0.f)
						{
							distance /= totalTime;
							/*
							mVisibleNodeArcs[pathNode][pathArc] = distance;
							mVisibleNodeArcsTime[pathNode][pathArc] = totalTime;
							*/
						}
						idx++;
					}
				}
				else if (visibleNodes[pathNode->GetPos()][visibleArc->GetNode()->GetPos()])
				{
					/*
					mVisibleNodeArcs[pathNode][pathArc] =
						Length(pathArc->GetNode()->GetPos() - pathNode->GetPos());
					mVisibleNodeArcsTime[pathNode][pathArc] = pathArc->GetWeight();
					*/
				}
			}
		}
	}

	//arcs to nodes
	for (PathingNode* pathNode : mPathingGraph->GetNodes())
	{
		for (PathingArc* pathArc : pathNode->GetArcs())
		{
			PathingTransition* pathTransition = pathNode->FindTransition(pathArc->GetId());
			if (pathTransition)
			{
				for (PathingNode* visibleNode : mPathingGraph->GetNodes())
				{
					unsigned int idx = 0;
					float totalTime = 0.f, distance = 0.f;

					for (Vector3<float> pathConnection : pathTransition->GetConnections())
					{
						if (visibleNodes[pathConnection][visibleNode->GetPos()])
						{
							float deltaTime = pathTransition->GetWeights()[idx];
							distance += Length(pathNode->GetPos() - pathConnection) * deltaTime;
							totalTime += deltaTime;
						}
						if (totalTime > 0.f)
						{
							distance /= totalTime;
							/*
							mVisibleNodeArcs[pathNode][pathArc] = distance;
							mVisibleNodeArcsTime[pathNode][pathArc] = totalTime;
							*/
						}
						idx++;
					}
				}
			}
			else
			{
				for (PathingNode* visibleNode : mPathingGraph->GetNodes())
				{
					if (visibleNodes[pathArc->GetNode()->GetPos()][visibleNode->GetPos()])
					{
						/*
						mVisibleNodeArcs[pathNode][pathArc] =
							Length(pathArc->GetNode()->GetPos() - pathNode->GetPos());
						mVisibleNodeArcsTime[pathNode][pathArc] = pathArc->GetWeight();
						*/
					}
				}
			}
		}
	}

	//arcs to arcs
	/*
	for (PathingArc* pathArc : mPathingGraph->GetArcs())
	{
		if (pathArc->GetType() == AIAT_MOVE)
		{
			for (PathingArc* visibleArc : mPathingGraph->GetArcs())
			{
				float totalTime = 0.f, totalVisibleTime = 0.f, distance = 0.f;
				if (visibleArc->GetType() == AIAT_MOVE)
				{
					if (visibleArc->GetWeight() > pathArc->GetWeight())
					{
						PathingNodeVec::iterator itVisibleArcNode = pathArcNodes[visibleArc].begin();
						float totalArcTime = 0.f;
						for (PathingNode* pathArcNode : pathArcNodes[pathArc])
						{
							if (visibleNodes[pathArcNode][(*itVisibleArcNode)])
							{
								float deltaTime = pathArcNodesTime[pathArc][pathArcNode];
								distance += Length((*itVisibleArcNode)->GetPos() - pathArcNode->GetPos()) * deltaTime;
								totalVisibleTime += deltaTime;
							}
							while (totalArcTime <= totalTime)
							{
								totalArcTime += pathArcNodesTime[visibleArc][(*itVisibleArcNode)];
								itVisibleArcNode++;
							}
							totalTime += pathArcNodesTime[pathArc][pathArcNode];
						}

						for (; itVisibleArcNode < pathArcNodes[visibleArc].end(); itVisibleArcNode++)
						{
							PathingNode* pathArcNode = pathArcNodes[pathArc].back();
							if (visibleNodes[pathArcNode][(*itVisibleArcNode)])
							{
								float deltaTime = pathArcNodesTime[pathArc][pathArcNode];
								distance += Length((*itVisibleArcNode)->GetPos() - pathArcNode->GetPos()) * deltaTime;
								totalVisibleTime += deltaTime;
							}
						}
					}
					else
					{
						PathingNodeVec::iterator itPathArcNode = pathArcNodes[pathArc].begin();

						float totalArcTime = 0.f;
						for (PathingNode* visibleArcNode : pathArcNodes[visibleArc])
						{
							if (visibleNodes[visibleArcNode][(*itPathArcNode)])
							{
								float deltaTime = pathArcNodesTime[visibleArc][visibleArcNode];
								distance += Length((*itPathArcNode)->GetPos() - visibleArcNode->GetPos()) * deltaTime;
								totalVisibleTime += deltaTime;
							}
							while (totalArcTime <= totalTime)
							{
								totalArcTime += pathArcNodesTime[pathArc][(*itPathArcNode)];
								itPathArcNode++;
							}
							totalTime += pathArcNodesTime[visibleArc][visibleArcNode];
						}

						for (; itPathArcNode < pathArcNodes[pathArc].end(); itPathArcNode++)
						{
							PathingNode* visibleArcNode = pathArcNodes[visibleArc].back();
							if (visibleNodes[visibleArcNode][(*itPathArcNode)])
							{
								float deltaTime = pathArcNodesTime[pathArc][visibleArcNode];
								distance += Length((*itPathArcNode)->GetPos() - visibleArcNode->GetPos()) * deltaTime;
								totalVisibleTime += deltaTime;
							}
						}
					}

					if (totalVisibleTime > 0.f)
					{
						distance /= totalVisibleTime;

						mVisibleArcs[pathArc][visibleArc] = distance;
						mVisibleArcsTime[pathArc][visibleArc] = totalVisibleTime;
					}
				}
				else
				{
					if (visibleArc->GetType() == AIAT_JUMPTARGET ||
						visibleArc->GetType() == AIAT_FALLTARGET ||
						visibleArc->GetType() == AIAT_PUSHTARGET)
					{
						PathingArc* visiblePathArc = visibleArc->GetOrigin()->FindArc(
							visibleArc->GetType() - 1, visibleArc->GetNeighbor());

						if (visibleArc->GetWeight() > pathArc->GetWeight())
						{
							float totalArcTime = 0.f;
							for (PathingNode* pathArcNode : pathArcNodes[pathArc])
							{
								if (visibleNodeArcs[pathArcNode][visiblePathArc])
								{
									float deltaTime = pathArcNodesTime[pathArc][pathArcNode];
									distance += Length(
										visiblePathArc->GetConnection() - pathArcNode->GetPos()) * deltaTime;
									totalVisibleTime += deltaTime;
								}
								while (totalArcTime <= totalTime)
								{
									totalArcTime += visiblePathArc->GetWeight();
									visiblePathArc = visiblePathArc->GetNeighbor()->FindArc(
										pathArc->GetType() - 1, pathArc->GetNeighbor());
								}
								totalTime += pathArcNodesTime[pathArc][pathArcNode];
							}

							while(pathArc->GetNeighbor() != visiblePathArc->GetOrigin())
							{
								PathingNode* pathArcNode = pathArcNodes[pathArc].back();
								if (visibleNodeArcs[pathArcNode][visiblePathArc])
								{
									float deltaTime = pathArcNodesTime[pathArc][pathArcNode];
									distance += Length(
										visiblePathArc->GetConnection() - pathArcNode->GetPos()) * deltaTime;
									totalVisibleTime += deltaTime;
								}
							}
						}
						else
						{
							float totalArcTime = 0.f;
							PathingNodeVec::iterator itPathArcNode = pathArcNodes[pathArc].begin();

							while (pathArc->GetNeighbor() != visiblePathArc->GetOrigin())
							{
								PathingNode* pathArcNode = (*itPathArcNode);
								if (visibleNodeArcs[pathArcNode][visiblePathArc])
								{
									float deltaTime = pathArcNodesTime[pathArc][pathArcNode];
									distance += Length(
										visiblePathArc->GetConnection() - pathArcNode->GetPos()) * deltaTime;
									totalVisibleTime += deltaTime;
								}
								while (totalArcTime <= totalTime)
								{
									totalArcTime += visiblePathArc->GetWeight();
									visiblePathArc = visiblePathArc->GetNeighbor()->FindArc(
										pathArc->GetType() - 1, pathArc->GetNeighbor());
								}
								totalTime += pathArcNodesTime[pathArc][pathArcNode];
								itPathArcNode++;
							}

							for (; itPathArcNode < pathArcNodes[pathArc].end(); itPathArcNode++)
							{
								PathingNode* pathArcNode = (*itPathArcNode);
								if (visibleNodeArcs[pathArcNode][visiblePathArc])
								{
									float deltaTime = pathArcNodesTime[pathArc][pathArcNode];
									distance += Length(
										visiblePathArc->GetConnection() - pathArcNode->GetPos()) * deltaTime;
									totalVisibleTime += deltaTime;
								}
							}
						}

						if (totalVisibleTime > 0.f)
						{
							distance /= totalVisibleTime;

							mVisibleArcs[pathArc][visibleArc] = distance;
							mVisibleArcsTime[pathArc][visibleArc] = totalVisibleTime;
						}
					}
				}
			}
		}
	}
	*/
}

void QuakeAIManager::SimulateGrenadeLauncherFire(PathingNode* pNode, eastl::shared_ptr<Actor> pGameActor)
{
	eastl::shared_ptr<BaseGamePhysic> gamePhysics = GameLogic::Get()->GetGamePhysics();

	Transform transform;
	Vector3<float> direction;
	for (int yawAngle = 0; yawAngle < 360; yawAngle += 20)
	{
		for (int pitchAngle = -80; pitchAngle <= 80; pitchAngle += 20)
		{
			Matrix4x4<float> yawRotation = Rotation<4, float>(
				AxisAngle<4, float>(Vector4<float>::Unit(2), yawAngle * (float)GE_C_DEG_TO_RAD));
			Matrix4x4<float> pitchRotation = Rotation<4, float>(
				AxisAngle<4, float>(Vector4<float>::Unit(1), -pitchAngle * (float)GE_C_DEG_TO_RAD));
			Matrix4x4<float> rotation = yawRotation * pitchRotation;

			// forward vector
#if defined(GE_USE_MAT_VEC)
			direction = HProject(rotation * Vector4<float>::Unit(PITCH));
#else
			direction = HProject(Vector4<float>::Unit(PITCH) * rotation);
#endif
			Vector3<float> forward = HProject(rotation * Vector4<float>::Unit(PITCH));

			//set muzzle location relative to pivoting eye
			Vector3<float> muzzle = pNode->GetPos();
			muzzle[2] += mPlayerActor->GetState().viewHeight;
			muzzle -= Vector3<float>::Unit(ROLL) * 11.f;
			Vector3<float> end = muzzle + forward * 8192.f * 16.f;

			direction[PITCH] *= 800000.f;
			direction[ROLL] *= 800000.f;
			direction[YAW] *= 500000.f;

			// projectile simulation
			transform.SetTranslation(muzzle);
			transform.SetRotation(rotation);
			gamePhysics->SetTransform(pGameActor->GetId(), transform);
			gamePhysics->ApplyForce(pGameActor->GetId(), direction);
			gamePhysics->OnUpdate(0.01f);

			float deltaTime = 0.f;
			while (deltaTime < 3.0f)
			{
				gamePhysics->OnUpdate(0.1f);
				deltaTime += 0.1f;
			}

			transform = gamePhysics->GetTransform(pGameActor->GetId());

			PathingNodeVec impactNodes;
			mPathingGraph->FindNodes(impactNodes, transform.GetTranslation(), 150);
			for (PathingNode* impactNode : impactNodes)
			{
				float dist = Length(transform.GetTranslation() - impactNode->GetPos());
				float damage = 100 * (1.f - dist / 150); // calculate radius damage

				direction = transform.GetTranslation() - muzzle;
				Normalize(direction);
				//damage;
			}
		}
	}
}

void QuakeAIManager::SimulateWaypoint()
{
	eastl::shared_ptr<BaseGamePhysic> gamePhysics = GameLogic::Get()->GetGamePhysics();

	mActorNodes.clear();
	while (!mOpenSet.empty())
	{
		// grab the candidate
		eastl::map<PathingNode*, bool>::iterator itOpenSet = mOpenSet.begin();
		PathingNode* pNode = itOpenSet->first;
		SimulateMovement(pNode);

		// we have processed this node so remove it from the open set
		mClosedSet[pNode] = itOpenSet->second;
		mOpenSet.erase(itOpenSet);
	}

	//finally we process the item actors which we have met
	eastl::map<PathingNode*, ActorId>::iterator itActorNode;
	for (itActorNode = mActorNodes.begin(); itActorNode != mActorNodes.end(); itActorNode++)
	{
		eastl::shared_ptr<Actor> pItemActor(
			GameLogic::Get()->GetActor(itActorNode->second).lock());
		PathingNode* pNode = itActorNode->first;
		if (pNode != NULL)
		{
			if (pItemActor->GetType() == "Trigger")
			{
				pNode->RemoveArcs(GAT_MOVE);
				pNode->RemoveTransitions(GAT_MOVE);
				if (pItemActor->GetComponent<PushTrigger>(PushTrigger::Name).lock())
				{
					eastl::shared_ptr<PushTrigger> pPushTrigger =
						pItemActor->GetComponent<PushTrigger>(PushTrigger::Name).lock();

					Vector3<float> targetPosition = pPushTrigger->GetTarget().GetTranslation();
					SimulateTriggerPush(pNode, targetPosition);
				}
				else if (pItemActor->GetComponent<TeleporterTrigger>(TeleporterTrigger::Name).lock())
				{
					eastl::shared_ptr<TeleporterTrigger> pTeleporterTrigger =
						pItemActor->GetComponent<TeleporterTrigger>(TeleporterTrigger::Name).lock();

					Vector3<float> targetPosition = pTeleporterTrigger->GetTarget().GetTranslation();
					SimulateTriggerTeleport(pNode, targetPosition);
				}
			}
		}
	}

	while (!mClosedSet.empty())
	{
		// grab the candidate
		eastl::map<PathingNode*, bool>::iterator itOpenSet = mClosedSet.begin();
		PathingNode* pNode = itOpenSet->first;

		//check if its on ground
		if (itOpenSet->second)
		{
			// if the node is a trigger we don't simulate jump on it
			if (mActorNodes.find(pNode) != mActorNodes.end())
			{
				eastl::shared_ptr<Actor> pGameActor(
					GameLogic::Get()->GetActor(mActorNodes[pNode]).lock());
				if (pGameActor->GetType() != "Trigger")
					SimulateJump(pNode);
			}
			else SimulateJump(pNode);
		}

		// we have processed this node so remove it from the closed set
		mClosedSet.erase(itOpenSet);
	}
}

void QuakeAIManager::CreateClusters()
{
	mClusteringGraph = eastl::make_shared<ClusteringGraph>();

	eastl::vector<Point> points;
	eastl::map<int, eastl::map<int, float>> distances;
	for (PathingNode* pathNode : mPathingGraph->GetNodes())
	{
		eastl::vector<float> pos{
			pathNode->GetPos()[0], pathNode->GetPos()[1], pathNode->GetPos()[2] };
		Point point(pathNode->GetId(), pos);
		points.push_back(point);
	}

	//Running K-Means Clustering
	unsigned int iters = 100;
	KMeans kmeans(500, 100, mPathingGraph);
	kmeans.Run(points);

	eastl::map<unsigned int, ClusteringNode*> clusterNodes;
	for (Clustering kCluster : kmeans.GetClusters())
	{
		Cluster* cluster = new Cluster(kCluster.GetId());
		mClusteringGraph->InsertCluster(cluster);

		for (int pIdx = 0; pIdx < kCluster.GetSize(); pIdx++)
		{
			Point point = kCluster.GetPoint(pIdx);
			PathingNode* pathNode = mPathingGraph->FindNode(point.GetId());

			ClusteringNode* clusterNode = new ClusteringNode(pathNode->GetId(), pathNode->GetPos());
			clusterNode->SetActor(pathNode->GetActorId());
			clusterNode->SetCluster(cluster);

			clusterNodes[clusterNode->GetId()] = clusterNode;
			cluster->AddActor(pathNode->GetActorId());
			cluster->InsertNode(clusterNode);
		}

		for (int pIdx = 0; pIdx < kCluster.GetIsolatedSize(); pIdx++)
		{
			Point point = kCluster.GetIsolatedPoint(pIdx);
			cluster->InsertIsolatedNode(clusterNodes[point.GetId()]);
		}

		Point center = kCluster.GetCenterPoint();
		ClusteringNode* centerNode = cluster->FindNode(center.GetId());
		cluster->SetCenter(centerNode);
	}

	mLastArcId = 0;
	for (Cluster* cluster : mClusteringGraph->GetClusters())
	{
		for (ClusteringNode* clusterNode : cluster->GetNodes())
		{
			//we take arcs connection to other clusters which are not isolated
			if (cluster->IsIsolatedNode(clusterNode))
			{
				eastl::shared_ptr<Actor> pGameActor(
					GameLogic::Get()->GetActor(clusterNode->GetActor()).lock());
				if (!pGameActor || pGameActor->GetType() != "Trigger")
					continue;
			}

			PathingNode* pathNode = mPathingGraph->FindNode(clusterNode->GetId());

			eastl::map<unsigned int, eastl::map<unsigned int, PathingArc*>> pathingArcs;
			for (PathingArc* pathArc : pathNode->GetArcs())
			{
				ClusteringNode* targetNode = clusterNodes[pathArc->GetNode()->GetId()];
				if (targetNode)
				{
					Cluster* targetCluster = targetNode->GetCluster();
					if (targetNode->GetCluster() != cluster)
					{
						if (cluster->FindArc(AT_ACTION, targetCluster) == NULL)
						{
							ClusterArc* clusterArc = new ClusterArc(
								GetNewClusterArcID(), AT_ACTION, targetCluster);
							cluster->AddArc(clusterArc);
						}

						if (pathingArcs.find(pathArc->GetType()) != pathingArcs.end() &&
							pathingArcs[pathArc->GetType()].find(cluster->GetId()) !=
							pathingArcs[pathArc->GetType()].end())
						{
							PathingArc* pArc = pathingArcs[pathArc->GetType()][cluster->GetId()];
							if (Length(targetCluster->GetCenter()->GetPos() - pArc->GetNode()->GetPos()) >
								Length(targetCluster->GetCenter()->GetPos() - pathArc->GetNode()->GetPos()))
							{
								pathingArcs[pathArc->GetType()][cluster->GetId()] = pathArc;
							}
						}
						else pathingArcs[pathArc->GetType()][cluster->GetId()] = pathArc;
					}
				}
			}

			//first we save the arcs which are inside the cluster or were used in kmean clustering
			for (PathingArc* pathArc : pathNode->GetArcs())
			{
				ClusteringNode* targetNode = clusterNodes[pathArc->GetNode()->GetId()];
				if (targetNode)
				{
					Cluster* targetCluster = targetNode->GetCluster();
					if (targetNode->GetCluster() == cluster || kmeans.IsArc(pathArc->GetId()))
					{
						ClusteringArc* clusterArc = new ClusteringArc(
							GetNewArcID(), pathArc->GetType(), targetNode, pathArc->GetWeight());
						clusterNode->AddArc(clusterArc);

						PathingTransition* pTransition = pathNode->FindTransition(pathArc->GetId());
						if (pTransition)
						{
							ClusteringNode* cNode = clusterNodes[pathNode->GetId()];
							ClusteringTransition* cTransition = new ClusteringTransition(clusterArc->GetId(),
								pTransition->GetType(), targetNode, pTransition->GetWeights(), pTransition->GetConnections());
							cNode->AddTransition(cTransition);
						}
					}
				}
			}

			//next we save the arcs which connects other clusters
			for (auto pathingArc : pathingArcs)
			{
				for (auto pathArc : pathingArc.second)
				{
					ClusteringNode* targetNode = clusterNodes[pathArc.second->GetNode()->GetId()];
					if (targetNode)
					{
						Cluster* targetCluster = targetNode->GetCluster();
						ClusteringArc* clusterArc = new ClusteringArc(
							GetNewArcID(), pathArc.second->GetType(), targetNode, pathArc.second->GetWeight());
						clusterNode->AddArc(clusterArc);

						PathingTransition* pTransition = pathNode->FindTransition(pathArc.second->GetId());
						if (pTransition)
						{
							ClusteringNode* cNode = clusterNodes[pathNode->GetId()];
							ClusteringTransition* cTransition = new ClusteringTransition(clusterArc->GetId(),
								pTransition->GetType(), targetNode, pTransition->GetWeights(), pTransition->GetConnections());
							cNode->AddTransition(cTransition);
						}
					}
				}
			}
		}
	}

	GameLogic::Get()->GetAIManager()->SaveClusteringGraph(
		FileSystem::Get()->GetPath("ai/quake/bloodrunclustering - copia.xml"));

	//we check that every node within the cluster has a connection to cluster center and actors
	ClusteringNodeArcMap clusterNodeArcs;
	for (Cluster* cluster : mClusteringGraph->GetClusters())
	{
		//clusterActors
		eastl::map<ClusteringNode*, ActorId> clusterActors;
		for (ClusteringNode* clusterNode : cluster->GetNodes())
		{
			for (ActorId actor : cluster->GetActors())
				if (clusterNode->GetActor() == actor && actor != INVALID_ACTOR_ID)
					clusterActors[clusterNode] = actor;
		}
		clusterActors[cluster->GetCenter()] = cluster->GetCenter()->GetActor();

		eastl::map<ClusteringNode*, ActorId>::iterator itClActor;
		for (itClActor = clusterActors.begin(); itClActor != clusterActors.end(); itClActor++)
		{
			ClusteringNode* actorClusterNode = (*itClActor).first;
			for (ClusteringNode* clusterNode : cluster->GetNodes())
			{
				if (actorClusterNode == clusterNode)
					continue;

				if (cluster->IsIsolatedNode(clusterNode))
				{
					eastl::shared_ptr<Actor> pGameActor(
						GameLogic::Get()->GetActor(clusterNode->GetActor()).lock());
					if (!pGameActor || pGameActor->GetType() != "Trigger")
						continue;
				}

				bool isLinked = false;
				for (ClusteringArc* clusterNodeArc : clusterNode->GetArcs())
				{
					ClusteringNode* targetNode = clusterNodeArc->GetNode();
					if (targetNode && targetNode == actorClusterNode)
					{
						isLinked = true;
						break;
					}
				}

				//if no one was found then we execute the cluster pathfinder
				if (!isLinked)
				{
					ClusterPlan* clusterPlan = cluster->FindNode(clusterNode, actorClusterNode);

					float pCost = 0.f;
					ClusteringArc* pBeginArc = clusterPlan->GetArcs().front();
					ClusteringArc* pEndArc = clusterPlan->GetArcs().back();
					for (auto cArcPlan : clusterPlan->GetArcs())
						pCost += cArcPlan->GetWeight();

					ClusteringArc* clusteringArc = new ClusteringArc(
						GetNewArcID(), GAT_CLUSTER, pEndArc->GetNode(), pCost);
					ClusteringTransition* clusteringTransition = new ClusteringTransition(
						clusteringArc->GetId(), pBeginArc->GetType(), pBeginArc->GetNode());
					clusterNode->AddTransition(clusteringTransition);
					clusterNodeArcs[clusterNode].push_back(clusteringArc);

					delete clusterPlan;
				}
			}
		}
	}

	//we check that every node within the cluster has a connection to the linked clusters
	for (Cluster* cluster : mClusteringGraph->GetClusters())
	{
		for (ClusterArc* clusterArc : cluster->GetArcs())
		{
			for (ClusteringNode* clusterNode : cluster->GetNodes())
			{
				if (cluster->IsIsolatedNode(clusterNode))
				{
					eastl::shared_ptr<Actor> pGameActor(
						GameLogic::Get()->GetActor(clusterNode->GetActor()).lock());
					if (!pGameActor || pGameActor->GetType() != "Trigger")
						continue;
				}

				bool isLinked = false;
				for (ClusteringArc* clusterNodeArc : clusterNode->GetArcs())
				{
					ClusteringNode* targetNode = clusterNodeArc->GetNode();
					if (targetNode && targetNode->GetCluster() == clusterArc->GetCluster())
					{
						isLinked = true;
						break;
					}
				}

				//if no one was found then we execute the cluster pathfinder
				if (!isLinked)
				{
					ClusterPlan* clusterPlan = cluster->FindNode(clusterNode, clusterArc->GetCluster());

					float pCost = 0.f;
					ClusteringArc* pBeginArc = clusterPlan->GetArcs().front();
					ClusteringArc* pEndArc = clusterPlan->GetArcs().back();
					for (auto cArcPlan : clusterPlan->GetArcs())
						pCost += cArcPlan->GetWeight();

					ClusteringArc* clusteringArc = new ClusteringArc(
						GetNewArcID(), GAT_CLUSTER, pEndArc->GetNode(), pCost);
					ClusteringTransition* clusteringTransition = new ClusteringTransition(
						clusteringArc->GetId(), pBeginArc->GetType(), pBeginArc->GetNode());
					clusterNode->AddTransition(clusteringTransition);
					clusterNodeArcs[clusterNode].push_back(clusteringArc);

					delete clusterPlan;
				}
			}
		}
	}

	ClusteringNodeArcMap::iterator itNodeArc = clusterNodeArcs.begin();
	for (; itNodeArc != clusterNodeArcs.end(); itNodeArc++)
	{
		ClusteringNode* clusterNode = (*itNodeArc).first;
		for (ClusteringArc* clusterArc : (*itNodeArc).second)
			clusterNode->AddArc(clusterArc);
	}
}

void QuakeAIManager::SimulateActorPosition(ActorId actorId, const Vector3<float>& position)
{
	//lets move the character towards different directions
	eastl::shared_ptr<BaseGamePhysic> gamePhysics = GameLogic::Get()->GetGamePhysics();

	Transform transform;
	transform.SetTranslation(position);
	gamePhysics->SetTransform(mPlayerActor->GetId(), transform);
	gamePhysics->OnUpdate(0.02f);

	transform = gamePhysics->GetTransform(mPlayerActor->GetId());
	PathingNode* pNewNode = new PathingNode(
		GetNewNodeID(), actorId, transform.GetTranslation());
	mPathingGraph->InsertNode(pNewNode);
	mOpenSet[pNewNode] = true;
}

void QuakeAIManager::SimulateTriggerTeleport(PathingNode* pNode, const Vector3<float>& target)
{
	//lets move the character towards different directions
	eastl::shared_ptr<BaseGamePhysic> gamePhysics = GameLogic::Get()->GetGamePhysics();

	Vector3<float> direction = Vector3<float>::Unit(YAW); //up vector

	Transform transform;
	transform.SetTranslation(target);
	gamePhysics->SetTransform(mPlayerActor->GetId(), transform);
	gamePhysics->OnUpdate(0.02f);

	// gravity falling simulation
	transform = gamePhysics->GetTransform(mPlayerActor->GetId());

	float totalTime = 0.f, fallSpeed = 0.f;
	eastl::vector<Vector3<float>> nodePositions;
	while (!gamePhysics->OnGround(mPlayerActor->GetId()) && totalTime <= 10.f)
	{
		nodePositions.push_back(transform.GetTranslation());

		float jumpSpeed = gamePhysics->GetJumpSpeed(mPlayerActor->GetId());

		totalTime += 0.02f;
		fallSpeed += (20.f / (jumpSpeed * 0.5f));
		if (fallSpeed > mMaxFallSpeed) fallSpeed = mMaxFallSpeed;

		Normalize(direction);
		direction[PITCH] *= jumpSpeed * (fallSpeed / 4.f);
		direction[ROLL] *= jumpSpeed * (fallSpeed / 4.f);
		direction[YAW] = -jumpSpeed * fallSpeed;

		gamePhysics->FallDirection(mPlayerActor->GetId(), direction);
		gamePhysics->OnUpdate(0.02f);

		transform = gamePhysics->GetTransform(mPlayerActor->GetId());
	}

	if (totalTime >= 10.f) return;
	totalTime += 0.02f;

	Vector3<float> position = transform.GetTranslation();
	PathingNode* pEndNode = mPathingGraph->FindClosestNode(position);
	if (pNode != pEndNode)
	{
		if (pEndNode != NULL && pNode->FindArc(GAT_TELEPORT, pEndNode) == NULL)
		{
			Vector3<float> diff = pEndNode->GetPos() - position;
			if (Length(diff) <= PATHING_DEFAULT_NODE_TOLERANCE)
			{
				PathingArc* pArc = new PathingArc(GetNewArcID(), GAT_TELEPORT, pEndNode, totalTime);
				pNode->AddArc(pArc);

				//lets interpolate transitions from the already created arc
				float deltaTime = 0.f;
				eastl::vector<float> times{ 0 };
				eastl::vector<Vector3<float>> positions{ pNode->GetPos() };
				for (unsigned int idx = 0; idx < nodePositions.size(); idx++)
				{
					deltaTime += 0.02f;

					if (Length(nodePositions[idx] - positions.back()) >= 24.f)
					{
						times.push_back(deltaTime);
						positions.push_back(nodePositions[idx]);
						deltaTime = 0.f;
					}
				}
				if (positions.size() > 1)
				{
					deltaTime += 0.02f;
					times.push_back(deltaTime);
					positions.push_back(pEndNode->GetPos());

					PathingTransition* pTransition = new PathingTransition(
						pArc->GetId(), GAT_TELEPORT, pEndNode, times, positions);
					pNode->AddTransition(pTransition);
				}
			}
		}
	}
}

void QuakeAIManager::SimulateTriggerPush(PathingNode* pNode, const Vector3<float>& target)
{
	//lets move the character towards different directions
	eastl::shared_ptr<BaseGamePhysic> gamePhysics = GameLogic::Get()->GetGamePhysics();

	Vector3<float> direction = target - pNode->GetPos();
	float push = Length(target - pNode->GetPos());
	Normalize(direction);

	direction[PITCH] *= push / 90.f;
	direction[ROLL] *= push / 90.f;
	direction[YAW] = push / 30.f;

	Transform transform;
	transform.SetTranslation(pNode->GetPos());
	gamePhysics->SetTransform(mPlayerActor->GetId(), transform);
	gamePhysics->WalkDirection(mPlayerActor->GetId(), direction);
	gamePhysics->Jump(mPlayerActor->GetId(), direction);
	gamePhysics->OnUpdate(0.02f);

	// gravity falling simulation
	transform = gamePhysics->GetTransform(mPlayerActor->GetId());

	float totalTime = 0.f, fallSpeed = 0.f;
	eastl::vector<Vector3<float>> nodePositions;
	while (!gamePhysics->OnGround(mPlayerActor->GetId()) && totalTime <= 10.f)
	{
		nodePositions.push_back(transform.GetTranslation());

		float jumpSpeed = gamePhysics->GetJumpSpeed(mPlayerActor->GetId());

		totalTime += 0.02f;
		fallSpeed += (20.f / (jumpSpeed * 0.5f));
		if (fallSpeed > mMaxFallSpeed) fallSpeed = mMaxFallSpeed;

		Normalize(direction);
		direction[PITCH] *= jumpSpeed * (fallSpeed / 4.f);
		direction[ROLL] *= jumpSpeed * (fallSpeed / 4.f);
		direction[YAW] = -jumpSpeed * fallSpeed;

		gamePhysics->FallDirection(mPlayerActor->GetId(), direction);
		gamePhysics->OnUpdate(0.02f);

		transform = gamePhysics->GetTransform(mPlayerActor->GetId());
	}

	if (totalTime >= 10.f) return;
	totalTime += 0.02f;

	//we store the jump if we find a landing node
	Vector3<float> position = transform.GetTranslation();
	PathingNode* pEndNode = mPathingGraph->FindClosestNode(position);
	if (pNode != pEndNode)
	{
		if (pEndNode != NULL && pNode->FindArc(GAT_PUSH, pEndNode) == NULL)
		{
			Vector3<float> diff = pEndNode->GetPos() - position;
			if (Length(diff) <= PATHING_DEFAULT_NODE_TOLERANCE)
			{
				PathingArc* pArc = new PathingArc(GetNewArcID(), GAT_PUSH, pEndNode, totalTime);
				pNode->AddArc(pArc);

				//lets interpolate transitions from the already created arc
				float deltaTime = 0.f;
				eastl::vector<float> times{ 0 };
				eastl::vector<Vector3<float>> positions{ pNode->GetPos() };
				for (unsigned int idx = 0; idx < nodePositions.size(); idx++)
				{
					deltaTime += 0.02f;

					if (Length(nodePositions[idx] - positions.back()) >= 24.f)
					{
						times.push_back(deltaTime);
						positions.push_back(nodePositions[idx]);
						deltaTime = 0.f;
					}
				}
				if (positions.size() > 1)
				{
					deltaTime += 0.02f;
					times.push_back(deltaTime);
					positions.push_back(pEndNode->GetPos());

					PathingTransition* pTransition = new PathingTransition(
						pArc->GetId(), GAT_PUSH, pEndNode, times, positions);
					pNode->AddTransition(pTransition);
				}
			}
		}
	}
}

float FindClosestMovement(eastl::vector<Vector3<float>>& movements, const Vector3<float>& pos)
{
	float length = FLT_MAX;
	eastl::vector<Vector3<float>>::iterator it;
	for (it = movements.begin(); it != movements.end(); it++)
	{
		Vector3<float> diff = pos - (*it);
		if (Length(diff) < length)
			length = Length(diff);
	}

	return length;
}


// Cliff control
bool Cliff(const Vector3<float>& translation)
{
	for (int angle = 0; angle < 360; angle += 5)
	{
		Matrix4x4<float> rotation = Rotation<4, float>(
			AxisAngle<4, float>(Vector4<float>::Unit(2), angle * (float)GE_C_DEG_TO_RAD));

		// This will give us the "look at" vector 
		// in world space - we'll use that to move.
		Vector4<float> atWorld = Vector4<float>::Unit(PITCH); // forward vector
#if defined(GE_USE_MAT_VEC)
		atWorld = rotation * atWorld;
#else
		atWorld = atWorld * rotation;
#endif

		Vector3<float> position = translation + HProject(atWorld * 10.f);

		Transform start;
		start.SetRotation(rotation);
		start.SetTranslation(position);

		Transform end;
		end.SetRotation(rotation);
		end.SetTranslation(position - Vector3<float>::Unit(2) * 300.f);

		Vector3<float> collision, collisionNormal;
		collision = end.GetTranslation();
		ActorId actorId = GameLogic::Get()->GetGamePhysics()->CastRay(
			start.GetTranslation(), end.GetTranslation(), collision, collisionNormal);

		//Check whether we are close to a cliff
		//printf("distance cliff %f \n", abs(collision[2] - position[2]));
		if (abs(collision[2] - position[2]) > 60.f)
			return true;
	}
	return false;
}

void QuakeAIManager::SimulateMovement(PathingNode* pNode)
{
	//lets move the character towards different directions
	eastl::shared_ptr<BaseGamePhysic> gamePhysics = GameLogic::Get()->GetGamePhysics();

	Transform transform;
	transform.SetTranslation(pNode->GetPos());
	gamePhysics->SetTransform(mPlayerActor->GetId(), transform);
	gamePhysics->SetVelocity(mPlayerActor->GetId(), Vector3<float>::Zero());

	// nodes closed to falling position
	for (int angle = 0; angle < 360; angle += 5)
	{
		Matrix4x4<float> rotation = Rotation<4, float>(
			AxisAngle<4, float>(Vector4<float>::Unit(2), angle * (float)GE_C_DEG_TO_RAD));

		transform.SetTranslation(pNode->GetPos());
		transform.SetRotation(rotation);

		//create movements on the ground
		eastl::vector<Vector3<float>> movements;
		eastl::map<Vector3<float>, bool> groundMovements;

		PathingNode* pCurrentNode = pNode;
		Vector3<float> position = pNode->GetPos();

		do
		{
			if (!gamePhysics->OnGround(mPlayerActor->GetId()))
			{
				float totalTime = 0.f;
				float fallSpeed = 0.f;
				do
				{
					groundMovements[position] = false;
					movements.push_back(position);

					float jumpSpeed = gamePhysics->GetJumpSpeed(mPlayerActor->GetId());

					totalTime += 0.02f;
					fallSpeed += (20.f / (jumpSpeed * 0.5f));
					if (fallSpeed > mMaxFallSpeed) fallSpeed = mMaxFallSpeed;

					Vector3<float> direction; // forward vector
#if defined(GE_USE_MAT_VEC)
					direction = HProject(rotation * Vector4<float>::Unit(PITCH));
#else
					direction = HProject(Vector4<float>::Unit(PITCH) * rotation);
#endif
					direction[PITCH] *= jumpSpeed * (fallSpeed / 4.f);
					direction[ROLL] *= jumpSpeed * (fallSpeed / 4.f);
					direction[YAW] = -jumpSpeed * fallSpeed;

					gamePhysics->FallDirection(mPlayerActor->GetId(), direction);
					gamePhysics->OnUpdate(0.02f);

					transform = gamePhysics->GetTransform(mPlayerActor->GetId());
					position = transform.GetTranslation();
				} while (!gamePhysics->OnGround(mPlayerActor->GetId()) && totalTime <= 10.f);

				if (totalTime >= 10.f)
					break;
			}

			groundMovements[position] = true;
			movements.push_back(position);

			PathingNode* pClosestNode = mPathingGraph->FindClosestNode(position);
			if (pCurrentNode != pClosestNode)
			{
				Vector3<float> diff = pClosestNode->GetPos() - position;
				if (Length(diff) <= PATHING_DEFAULT_NODE_TOLERANCE)
				{
					//if we find any node close to our current position we stop
					break;
				}
			}

			Vector3<float> direction; // forward vector
#if defined(GE_USE_MAT_VEC)
			direction = HProject(rotation * Vector4<float>::Unit(PITCH));
#else
			direction = HProject(Vector4<float>::Unit(PITCH) * rotation);
#endif

			gamePhysics->SetTransform(mPlayerActor->GetId(), transform);
			gamePhysics->WalkDirection(mPlayerActor->GetId(), direction * mMoveSpeed);
			gamePhysics->OnUpdate(0.02f);

			transform = gamePhysics->GetTransform(mPlayerActor->GetId());
			position = transform.GetTranslation();

		} while (FindClosestMovement(movements, position) >= 4.f); // stalling is a break condition

		if (!movements.empty())
		{
			float deltaTime = 0.f, totalTime = 0.f;
			eastl::vector<Vector3<float>>::iterator itMove = movements.begin();
			eastl::vector<Vector3<float>> positions{ (*itMove) };
			eastl::vector<float> times{ 0 };
			itMove++;
			bool onGround = true;
			for (; itMove != movements.end(); itMove++)
			{
				totalTime += 0.02f;
				deltaTime += 0.02f;
				if (Length((*itMove) - positions.back()) >= 24.f)
				{
					times.push_back(deltaTime);
					positions.push_back((*itMove));

					deltaTime = 0.f;
				}

				if (groundMovements[(*itMove)])
				{
					PathingNode* pClosestNode = mPathingGraph->FindClosestNode((*itMove));
					Vector3<float> diff = pClosestNode->GetPos() - (*itMove);
					if (Length(diff) >= 16.f)
					{
						Vector3<float> move = (*itMove);
						Vector3<float> scale = gamePhysics->GetScale(mPlayerActor->GetId()) / 2.f;

						Transform start;
						start.SetTranslation(pCurrentNode->GetPos() + scale[YAW] * Vector3<float>::Unit(YAW));
						Transform end;
						end.SetTranslation(move + scale[YAW] * Vector3<float>::Unit(YAW));

						Vector3<float> collision, collisionNormal;
						ActorId actorId = gamePhysics->ConvexSweep(
							mPlayerActor->GetId(), start, end, collision, collisionNormal);
						if (!Cliff(*itMove))
						{
							Vector3<float> scale = gamePhysics->GetScale(mPlayerActor->GetId()) / 2.f;

							Transform start;
							start.SetTranslation(pCurrentNode->GetPos() + scale[YAW] * Vector3<float>::Unit(YAW));
							Transform end;
							end.SetTranslation((*itMove) + scale[YAW] * Vector3<float>::Unit(YAW));

							Vector3<float> collision, collisionNormal;
							ActorId actorId = gamePhysics->ConvexSweep(
								mPlayerActor->GetId(), start, end, collision, collisionNormal);
							if (collision == NULL || actorId != INVALID_ACTOR_ID)
							{
								unsigned int arcType = onGround ? GAT_MOVE : GAT_FALL;
								PathingNode* pNewNode = new PathingNode(GetNewNodeID(), INVALID_ACTOR_ID, (*itMove));
								PathingArc* pArc = new PathingArc(GetNewArcID(), arcType, pNewNode, totalTime);
								pCurrentNode->AddArc(pArc);

								if (positions.size() > 1)
								{
									if (deltaTime > 0.f)
									{
										times.push_back(deltaTime);
										positions.push_back(pNewNode->GetPos());
									}

									PathingTransition* pTransition = new PathingTransition(
										pArc->GetId(), arcType, pNewNode, times, positions);
									pNode->AddTransition(pTransition);
								}

								mPathingGraph->InsertNode(pNewNode);
								mOpenSet[pNewNode] = true;
								pCurrentNode = pNewNode;

								deltaTime = 0.f;
								totalTime = 0.f;

								onGround = true;
								positions = { pNewNode->GetPos() };
								times = { 0 };
							}
							else
							{
								if (!onGround)
								{
									break; //we stop processing movements if we find collision
								}
								break; //we stop processing movements if we find collision
							}
						}
						else
						{
							break; //we stop processing movements if any point don't pass the cliff control
						}
					}
					else if (pCurrentNode != pClosestNode)
					{
						if (Length(diff) <= PATHING_DEFAULT_NODE_TOLERANCE)
						{
							unsigned int arcType = onGround ? GAT_MOVE : GAT_FALL;
							if (pCurrentNode->FindArc(arcType, pClosestNode) == NULL)
							{
								Vector3<float> scale = gamePhysics->GetScale(mPlayerActor->GetId()) / 2.f;

								Transform start;
								start.SetTranslation(pCurrentNode->GetPos() + scale[YAW] * Vector3<float>::Unit(YAW));
								Transform end;
								end.SetTranslation(pClosestNode->GetPos() + scale[YAW] * Vector3<float>::Unit(YAW));

								Vector3<float> collision, collisionNormal;
								ActorId actorId = gamePhysics->ConvexSweep(
									mPlayerActor->GetId(), start, end, collision, collisionNormal);
								if (collision == NULL || actorId != INVALID_ACTOR_ID)
								{
									PathingArc* pArc = new PathingArc(GetNewArcID(), arcType, pClosestNode, totalTime);
									pCurrentNode->AddArc(pArc);

									if (positions.size() > 1)
									{
										if (deltaTime > 0.f)
										{
											times.push_back(deltaTime);
											positions.push_back(pClosestNode->GetPos());
										}

										PathingTransition* pTransition = new PathingTransition(
											pArc->GetId(), arcType, pClosestNode, times, positions);
										pNode->AddTransition(pTransition);
									}
								}
								else
								{
									break; //we stop processing movements if we find collision
								}
							}
							pCurrentNode = pClosestNode;

							deltaTime = 0.f;
							totalTime = 0.f;

							onGround = true;
							positions = { pClosestNode->GetPos() };
							times = { 0 };
						}
					}
				}
				else onGround = false;
			}
		}
	}
}

void QuakeAIManager::SimulateJump(PathingNode* pNode)
{
	//lets move the character towards different directions
	eastl::shared_ptr<BaseGamePhysic> gamePhysics = GameLogic::Get()->GetGamePhysics();

	Transform transform;
	Vector3<float> direction;
	for (int angle = 0; angle < 360; angle += 5)
	{
		Matrix4x4<float> rotation = Rotation<4, float>(
			AxisAngle<4, float>(Vector4<float>::Unit(2), angle * (float)GE_C_DEG_TO_RAD));

		// forward vector
#if defined(GE_USE_MAT_VEC)
		direction = HProject(rotation * Vector4<float>::Unit(PITCH));
#else
		direction = HProject(Vector4<float>::Unit(PITCH) * rotation);
#endif
		direction[PITCH] *= mJumpMoveSpeed;
		direction[ROLL] *= mJumpMoveSpeed;
		direction[YAW] = mJumpSpeed;

		transform.SetTranslation(pNode->GetPos());
		transform.SetRotation(rotation);
		gamePhysics->SetTransform(mPlayerActor->GetId(), transform);
		gamePhysics->WalkDirection(mPlayerActor->GetId(), direction);
		gamePhysics->Jump(mPlayerActor->GetId(), direction);
		gamePhysics->OnUpdate(0.02f);

		transform = gamePhysics->GetTransform(mPlayerActor->GetId());

		float fallSpeed = 0.f, totalTime = 0.f;
		PathingNode* pFallingNode = pNode;

		// gravity falling simulation
		eastl::vector<Vector3<float>> nodePositions;
		while (!gamePhysics->OnGround(mPlayerActor->GetId()) && totalTime <= 10.f)
		{
			nodePositions.push_back(transform.GetTranslation());
			float jumpSpeed = gamePhysics->GetJumpSpeed(mPlayerActor->GetId());

			totalTime += 0.02f;
			fallSpeed += (20.f / (jumpSpeed * 0.5f));
			if (fallSpeed > mMaxFallSpeed) fallSpeed = mMaxFallSpeed;

			// forward vector
#if defined(GE_USE_MAT_VEC)
			direction = HProject(rotation * Vector4<float>::Unit(PITCH));
#else
			direction = HProject(Vector4<float>::Unit(PITCH) * rotation);
#endif
			direction[PITCH] *= jumpSpeed * (fallSpeed / 4.f);
			direction[ROLL] *= jumpSpeed * (fallSpeed / 4.f);
			direction[YAW] = -jumpSpeed * fallSpeed;

			gamePhysics->FallDirection(mPlayerActor->GetId(), direction);
			gamePhysics->OnUpdate(0.02f);

			transform = gamePhysics->GetTransform(mPlayerActor->GetId());
		}

		if (totalTime > 10.f) continue;
		totalTime += 0.02f;

		//we store the jump if we find a landing node
		Vector3<float> position = transform.GetTranslation();
		PathingNode* pEndNode = mPathingGraph->FindClosestNode(position);
		if (pNode != pEndNode)
		{
			if (pEndNode != NULL && pNode->FindArc(GAT_JUMP, pEndNode) == NULL)
			{
				//check if we have done a clean jump (no collisions)
				if (Length(pEndNode->GetPos() - pNode->GetPos()) >= 300.f ||
					pEndNode->GetPos()[2] - pNode->GetPos()[2] >= 30.f)
				{
					Vector3<float> diff = pEndNode->GetPos() - position;
					if (Length(diff) <= PATHING_DEFAULT_NODE_TOLERANCE)
					{
						PathingArc* pArc = new PathingArc(GetNewArcID(), GAT_JUMP, pEndNode, totalTime);
						pNode->AddArc(pArc);

						//lets interpolate transitions from the already created arc
						float deltaTime = 0.f;
						eastl::vector<float> times{ 0 };
						eastl::vector<Vector3<float>> positions{ pNode->GetPos() };
						for (unsigned int idx = 0; idx < nodePositions.size(); idx++)
						{
							deltaTime += 0.02f;

							if (Length(nodePositions[idx] - positions.back()) >= 24.f)
							{
								times.push_back(deltaTime);
								positions.push_back(nodePositions[idx]);
								deltaTime = 0.f;
							}
						}
						if (positions.size() > 1)
						{
							deltaTime += 0.02f;
							times.push_back(deltaTime);
							positions.push_back(pEndNode->GetPos());

							PathingTransition* pTransition = new PathingTransition(
								pArc->GetId(), GAT_JUMP, pEndNode, times, positions);
							pNode->AddTransition(pTransition);
						}
					}
				}
			}
		}
	}
}

void QuakeAIManager::PhysicsTriggerEnterDelegate(BaseEventDataPtr pEventData)
{
	eastl::shared_ptr<EventDataPhysTriggerEnter> pCastEventData =
		eastl::static_pointer_cast<EventDataPhysTriggerEnter>(pEventData);

	eastl::shared_ptr<Actor> pItemActor(
		GameLogic::Get()->GetActor(pCastEventData->GetTriggerId()).lock());

	if (mPlayerActor->GetId() == pCastEventData->GetOtherActor())
	{
		eastl::shared_ptr<PhysicComponent> pPhysicComponent(
			mPlayerActor->GetComponent<PhysicComponent>(PhysicComponent::Name).lock());
		if (pPhysicComponent->OnGround())
		{
			if (pItemActor->GetType() == "Weapon" ||
				pItemActor->GetType() == "Ammo" ||
				pItemActor->GetType() == "Armor" ||
				pItemActor->GetType() == "Health")
			{
				Vector3<float> position = pPhysicComponent->GetTransform().GetTranslation();
				PathingNode* pClosestNode = mPathingGraph->FindClosestNode(position);
				if (pClosestNode != NULL)
				{
					Vector3<float> diff = pClosestNode->GetPos() - position;
					if (Length(diff) <= PATHING_DEFAULT_NODE_TOLERANCE)
						pClosestNode->SetActorId(pItemActor->GetId());
				}
			}
		}
	}
}


void QuakeAIManager::PhysicsTriggerLeaveDelegate(BaseEventDataPtr pEventData)
{
	eastl::shared_ptr<EventDataPhysTriggerLeave> pCastEventData =
		eastl::static_pointer_cast<EventDataPhysTriggerLeave>(pEventData);
}


void QuakeAIManager::PhysicsCollisionDelegate(BaseEventDataPtr pEventData)
{
	eastl::shared_ptr<EventDataPhysCollision> pCastEventData =
		eastl::static_pointer_cast<EventDataPhysCollision>(pEventData);

	eastl::shared_ptr<Actor> pGameActorA(
		GameLogic::Get()->GetActor(pCastEventData->GetActorA()).lock());
	eastl::shared_ptr<Actor> pGameActorB(
		GameLogic::Get()->GetActor(pCastEventData->GetActorB()).lock());
	if (pGameActorA && pGameActorB)
	{
		eastl::shared_ptr<Actor> pItemActor;
		eastl::shared_ptr<PlayerActor> pPlayerActor;
		if (eastl::dynamic_shared_pointer_cast<PlayerActor>(pGameActorA) &&
			!eastl::dynamic_shared_pointer_cast<PlayerActor>(pGameActorB))
		{
			pPlayerActor = eastl::dynamic_shared_pointer_cast<PlayerActor>(pGameActorA);
			pItemActor = pGameActorB;

		}
		else if (!eastl::dynamic_shared_pointer_cast<PlayerActor>(pGameActorA) &&
			eastl::dynamic_shared_pointer_cast<PlayerActor>(pGameActorB))
		{
			pPlayerActor = eastl::dynamic_shared_pointer_cast<PlayerActor>(pGameActorB);
			pItemActor = pGameActorA;
		}
		else
		{
			if (pGameActorA->GetType() == "Fire")
			{
				mActorCollisions[pGameActorA->GetId()] = true;
			}
			else if (pGameActorB->GetType() == "Fire")
			{
				mActorCollisions[pGameActorB->GetId()] = true;
			}
			return;
		}

		if (mPlayerActor->GetId() == pPlayerActor->GetId())
		{
			eastl::shared_ptr<PhysicComponent> pPhysicComponent(
				mPlayerActor->GetComponent<PhysicComponent>(PhysicComponent::Name).lock());
			if (pPhysicComponent->OnGround())
			{
				if (pItemActor->GetType() == "Trigger")
				{
					Vector3<float> position = pPhysicComponent->GetTransform().GetTranslation();
					PathingNode* pClosestNode = mPathingGraph->FindClosestNode(position);
					if (pClosestNode != NULL)
					{
						Vector3<float> diff = pClosestNode->GetPos() - position;
						if (Length(diff) <= PATHING_DEFAULT_NODE_TOLERANCE)
							mActorNodes[pClosestNode] = pItemActor->GetId();
					}
				}
			}
		}
	}
}

void QuakeAIManager::PhysicsSeparationDelegate(BaseEventDataPtr pEventData)
{
	eastl::shared_ptr<EventDataPhysSeparation> pCastEventData =
		eastl::static_pointer_cast<EventDataPhysSeparation>(pEventData);

}

void QuakeAIManager::RegisterAllDelegates(void)
{
	BaseEventManager* pGlobalEventManager = BaseEventManager::Get();
	pGlobalEventManager->AddListener(
		MakeDelegate(this, &QuakeAIManager::PhysicsTriggerEnterDelegate),
		EventDataPhysTriggerEnter::skEventType);
	pGlobalEventManager->AddListener(
		MakeDelegate(this, &QuakeAIManager::PhysicsTriggerLeaveDelegate),
		EventDataPhysTriggerLeave::skEventType);
	pGlobalEventManager->AddListener(
		MakeDelegate(this, &QuakeAIManager::PhysicsCollisionDelegate),
		EventDataPhysCollision::skEventType);
	pGlobalEventManager->AddListener(
		MakeDelegate(this, &QuakeAIManager::PhysicsSeparationDelegate),
		EventDataPhysSeparation::skEventType);
}

void QuakeAIManager::RemoveAllDelegates(void)
{
	BaseEventManager* pGlobalEventManager = BaseEventManager::Get();
	pGlobalEventManager->RemoveListener(
		MakeDelegate(this, &QuakeAIManager::PhysicsTriggerEnterDelegate),
		EventDataPhysTriggerEnter::skEventType);
	pGlobalEventManager->RemoveListener(
		MakeDelegate(this, &QuakeAIManager::PhysicsTriggerLeaveDelegate),
		EventDataPhysTriggerLeave::skEventType);
	pGlobalEventManager->RemoveListener(
		MakeDelegate(this, &QuakeAIManager::PhysicsCollisionDelegate),
		EventDataPhysCollision::skEventType);
	pGlobalEventManager->RemoveListener(
		MakeDelegate(this, &QuakeAIManager::PhysicsSeparationDelegate),
		EventDataPhysSeparation::skEventType);
}