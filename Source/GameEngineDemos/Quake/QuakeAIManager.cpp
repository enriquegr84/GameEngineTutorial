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

#include <cereal/types/vector.hpp>
#include <cereal/types/memory.hpp>
#include <cereal/archives/binary.hpp>
#include <fstream>

namespace CerealTypes
{
	struct Vec3 
	{ 
		int x, y, z; 
	
		template <class Archive>
		void serialize(Archive & ar)
		{
			ar(x, y, z);
		}
	};

	struct VisibleNode
	{
		int id;

		template <class Archive>
		void serialize(Archive & ar)
		{
			ar(id);
		}
	};

	struct GraphArcNode
	{
		int id;
		int type;
		int nodeid;
		float weight;

		template <class Archive>
		void serialize(Archive & ar)
		{
			ar(id, type, nodeid, weight);
		}
	};

	struct GraphNodeTransition
	{
		int id;
		int type;
		std::vector<float> weights;
		std::vector<int> nodes;
		std::vector<Vec3> connections;

		template <class Archive>
		void save(Archive & ar) const
		{
			ar(id, type, weights, nodes, connections);
		}

		template <class Archive>
		void load(Archive & ar)
		{
			ar(id, type, weights, nodes, connections);
		}
	};

	struct GraphNode
	{
		int id;
		int actorid;
		float tolerance;
		Vec3 position;
		std::vector<GraphArcNode> arcs;
		std::vector<GraphNodeTransition> transitions;
		std::vector<VisibleNode> visiblenodes;

		template <class Archive>
		void save(Archive & ar) const
		{
			ar(id, actorid, tolerance, position, arcs, transitions, visiblenodes);
		}

		template <class Archive>
		void load(Archive & ar)
		{
			ar(id, actorid, tolerance, position, arcs, transitions, visiblenodes);
		}
	};

	struct Graph
	{
		std::vector<GraphNode> nodes;

		template <class Archive>
		void save(Archive & ar) const
		{
			ar(nodes);
		}

		template <class Archive>
		void load(Archive & ar)
		{
			ar(nodes);
		}
	};

	struct ClusterVisibleNode
	{
		int id;

		template <class Archive>
		void serialize(Archive & ar)
		{
			ar(id);
		}
	};

	struct ClusterArcNode
	{
		int id;
		int type;
		int nodeid;
		float weight;

		template <class Archive>
		void serialize(Archive & ar)
		{
			ar(id, type, nodeid, weight);
		}
	};

	struct ClusterTransition
	{
		int id;
		int type;
		std::vector<float> weights;
		std::vector<int> nodes;
		std::vector<Vec3> connections;

		template <class Archive>
		void save(Archive & ar) const
		{
			ar(id, type, weights, nodes, connections);
		}

		template <class Archive>
		void load(Archive & ar)
		{
			ar(id, type, weights, nodes, connections);
		}
	};

	struct ClusterNode
	{
		int id;
		int actorid;
		bool isisolated;
		Vec3 position;
		std::vector<ClusterArcNode> arcs;
		std::vector<ClusterTransition> transitions;
		std::vector<ClusterVisibleNode> visiblenodes;

		template <class Archive>
		void save(Archive & ar) const
		{
			ar(id, actorid, isisolated, position, arcs, transitions, visiblenodes);
		}

		template <class Archive>
		void load(Archive & ar)
		{
			ar(id, actorid, isisolated, position, arcs, transitions, visiblenodes);
		}
	};

	struct ClusterLink
	{
		int id;
		int type;
		int clusterid;

		template <class Archive>
		void serialize(Archive & ar)
		{
			ar(id, type, clusterid);
		}
	};

	struct ClusterData
	{
		int id;
		int centerid;
		std::vector<ClusterNode> nodes;
		std::vector<ClusterLink> arcs;

		template <class Archive>
		void save(Archive & ar) const
		{
			ar(id, centerid, nodes, arcs);
		}

		template <class Archive>
		void load(Archive & ar)
		{
			ar(id, centerid, nodes, arcs);
		}
	};

	struct ClusterGraph
	{
		std::vector<ClusterData> clusters;

		template <class Archive>
		void save(Archive & ar) const
		{
			ar(clusters);
		}

		template <class Archive>
		void load(Archive & ar)
		{
			ar(clusters);
		}
	};
}

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
// QuakeAIManager::SavePathingGraph
//
//    Saves the AI pathing graph information to an XML file
//
void QuakeAIManager::SavePathingGraph(const eastl::string& path)
{
	//set some random data
	CerealTypes::Graph data;

	for (PathingNode* pathNode : mPathingGraph->GetNodes())
	{
		CerealTypes::GraphNode node;

		node.id = pathNode->GetId();
		node.actorid = pathNode->GetActorId();
		node.tolerance = pathNode->GetTolerance();
		node.position.x = (int)round(pathNode->GetPos()[0]);
		node.position.y = (int)round(pathNode->GetPos()[1]);
		node.position.z = (int)round(pathNode->GetPos()[2]);

		eastl::map<PathingNode*, float> visibilityNodes;
		pathNode->GetVisibileNodes(visibilityNodes);
		for (auto visibilityNode : visibilityNodes)
		{
			CerealTypes::VisibleNode visibleNode;
			visibleNode.id = visibilityNode.first->GetId();

			node.visiblenodes.push_back(visibleNode);
		}

		for (PathingArc* pathArc : pathNode->GetArcs())
		{
			CerealTypes::GraphArcNode arcNode;
			arcNode.id = pathArc->GetId();
			arcNode.type = pathArc->GetType();
			arcNode.nodeid = pathArc->GetNode()->GetId();
			arcNode.weight = pathArc->GetWeight();

			node.arcs.push_back(arcNode);
		}

		for (PathingTransition* pathTransition : pathNode->GetTransitions())
		{
			CerealTypes::GraphNodeTransition transitionNode;
			transitionNode.id = pathTransition->GetId();
			transitionNode.type = pathTransition->GetType();

			for (PathingNode* pNode : pathTransition->GetNodes())
			{
				transitionNode.nodes.push_back(pNode->GetId());
			}
			for (float weight : pathTransition->GetWeights())
			{
				transitionNode.weights.push_back(weight);
			}
			for (Vector3<float> connection : pathTransition->GetConnections())
			{
				transitionNode.connections.push_back(CerealTypes::Vec3{
					(int)round(connection[0]), (int)round(connection[1]), (int)round(connection[2]) });
			}

			node.transitions.push_back(transitionNode);
		}

		data.nodes.push_back(node);
	}

	std::ofstream os(path.c_str(), std::ios::binary);
	cereal::BinaryOutputArchive archive(os);
	archive(data);

	/*
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

		eastl::map<PathingNode*, float> visibilityNodes;
		pathNode->GetVisibilities(VT_DISTANCE, visibilityNodes);
		for (auto visibilityNode : visibilityNodes)
		{
			tinyxml2::XMLElement* pVisibility = doc.NewElement("VisibilityNode");
			pVisibility->SetAttribute("id", eastl::to_string(visibilityNode.first->GetId()).c_str());
			pNode->LinkEndChild(pVisibility);
		}

		for (PathingArc* pathArc : pathNode->GetArcs())
		{
			tinyxml2::XMLElement* pArc = doc.NewElement("Arc");
			pArc->SetAttribute("id", eastl::to_string(pathArc->GetId()).c_str());
			pArc->SetAttribute("type", eastl::to_string(pathArc->GetType()).c_str());
			pArc->SetAttribute("node", eastl::to_string(pathArc->GetNode()->GetId()).c_str());
			pArc->SetAttribute("weight", eastl::to_string(pathArc->GetWeight()).c_str());
			pNode->LinkEndChild(pArc);
		}
	}

	doc.SaveFile(path.c_str());
	*/
}


/////////////////////////////////////////////////////////////////////////////
// QuakeAIManager::LoadPathingGraph
//
//    Loads the AI pathing graph information from an XML file
//
void QuakeAIManager::LoadPathingGraph(const eastl::wstring& path)
{
	//set data
	CerealTypes::Graph data;

	std::ifstream is(path.c_str(), std::ios::binary);
	if (is.fail())
	{
		LogError(strerror(errno));
	}
	cereal::BinaryInputArchive archive(is);
	archive(data);

	mPathingGraph = eastl::make_shared<PathingGraph>();

	eastl::map<unsigned int, PathingNode*> pathingNodeGraph;
	for (CerealTypes::GraphNode node : data.nodes)
	{
		int pathNodeId = node.id;
		ActorId actorId = node.actorid;
		float tolerance = node.tolerance;
		Vector3<float> position{ 
			(float)node.position.x, (float)node.position.y, (float)node.position.z };

		PathingNode* pathNode = new PathingNode(pathNodeId, actorId, position, tolerance);
		mPathingGraph->InsertNode(pathNode);

		pathingNodeGraph[pathNodeId] = pathNode;
	}

	for (CerealTypes::GraphNode node : data.nodes)
	{
		int pathNodeId = node.id;
		PathingNode* pathNode = pathingNodeGraph[pathNodeId];

		for (CerealTypes::VisibleNode visibleNode : node.visiblenodes)
		{
			PathingNode* visibilityNode = pathingNodeGraph[visibleNode.id];
			pathNode->AddVisibleNode(
				visibilityNode, Length(visibilityNode->GetPos() - pathNode->GetPos()));
		}

		for (CerealTypes::GraphArcNode arc : node.arcs)
		{
			int arcId = arc.id;
			int arcType = arc.type;
			int arcNode = arc.nodeid;
			float weight = arc.weight;

			PathingArc* pathArc = new PathingArc(arcId, arcType, pathingNodeGraph[arcNode], weight);
			mPathingGraph->InsertArc(pathArc);

			pathNode->AddArc(pathArc);
		}

		for (CerealTypes::GraphNodeTransition transition : node.transitions)
		{
			int transitionId = transition.id;
			int transitionType = transition.type;

			eastl::vector<float> weights;
			eastl::vector<PathingNode*> nodes;
			eastl::vector<Vector3<float>> connections;
			for (int nodeid : transition.nodes)
			{
				nodes.push_back(pathingNodeGraph[nodeid]);
			}
			for (float weight : transition.weights)
			{
				weights.push_back(weight);
			}
			for (CerealTypes::Vec3 connection : transition.connections)
			{
				connections.push_back(Vector3<float>{
					(float)connection.x, (float)connection.y, (float)connection.z});
			}

			PathingTransition* pathTransition = new PathingTransition(
				transitionId, transitionType, nodes, weights, connections);
			pathNode->AddTransition(pathTransition);
		}
	}

	/*
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

	PathingNodeDoubleMap visibleNodes;
	for (tinyxml2::XMLElement* pNode = pRoot->FirstChildElement(); pNode; pNode = pNode->NextSiblingElement())
	{
		int pathNodeId = 0;
		pathNodeId = pNode->IntAttribute("id", pathNodeId);
		PathingNode* pathNode = pathingNodeGraph[pathNodeId];

		for (tinyxml2::XMLElement* pElement = pNode->FirstChildElement(); pElement; pElement = pElement->NextSiblingElement())
		{
			if (pElement->Name() == "Arc")
			{
				int arcId = 0;
				int arcType = 0;
				int arcNode = 0;
				float weight = 0.f;

				arcId = pElement->IntAttribute("id", arcId);
				arcType = pElement->IntAttribute("type", arcType);
				arcNode = pElement->IntAttribute("node", arcNode);
				weight = pElement->FloatAttribute("weight", weight);

				PathingArc* pathArc = new PathingArc(arcId, arcType, pathingNodeGraph[arcNode], weight);
				mPathingGraph->InsertArc(pathArc);

				pathNode->AddArc(pathArc);
			}
			else if (pElement->Name() == "VisibleNode")
			{
				int nodeId = 0;
				nodeId = pElement->IntAttribute("id", nodeId);
				visibleNodes[pathNode][pathingNodeGraph[nodeId]] = 1.0f;
			}
		}
	}
	*/
}

/////////////////////////////////////////////////////////////////////////////
// QuakeAIManager::LoadClusteringGraph
//
//    Loads the AI clustering graph information from an XML file
//
void QuakeAIManager::LoadClusteringGraph(const eastl::wstring& path)
{
	//set data
	CerealTypes::ClusterGraph data;

	std::ifstream is(path.c_str(), std::ios::binary);
	cereal::BinaryInputArchive archive(is);
	archive(data);

	mClusteringGraph = eastl::make_shared<ClusteringGraph>();

	eastl::map<unsigned int, Cluster*> clusteringGraph;
	eastl::map<unsigned int, ClusteringNode*> clusteringNodes;
	for (CerealTypes::ClusterData cluster : data.clusters)
	{
		int clusterId = cluster.id;
		int centerId = cluster.centerid;

		clusteringGraph[clusterId] = new Cluster(clusterId);
		mClusteringGraph->InsertCluster(clusteringGraph[clusterId]);

		for (CerealTypes::ClusterNode node : cluster.nodes)
		{
			int clusterNodeId = node.id;
			ActorId actorId = node.actorid;
			Vector3<float> position{
				(float)node.position.x, (float)node.position.y, (float)node.position.z };

			ClusteringNode* clusterNode = new ClusteringNode(clusterNodeId, position);
			clusterNode->SetActor(actorId);
			clusterNode->SetCluster(clusteringGraph[clusterId]);
			clusteringGraph[clusterId]->InsertNode(clusterNode);
			clusteringGraph[clusterId]->AddActor(actorId);
			if (node.isisolated)
				clusteringGraph[clusterId]->InsertIsolatedNode(clusterNode);

			clusteringNodes[clusterNodeId] = clusterNode;
		}
		clusteringGraph[clusterId]->SetCenter(clusteringNodes[centerId]);
	}

	ClusteringNodeDoubleMap visibleNodes;
	for (CerealTypes::ClusterData cluster : data.clusters)
	{
		int clusterId = cluster.id;
		int centerId = cluster.centerid;

		for (CerealTypes::ClusterLink clusterLink : cluster.arcs)
		{
			int arcId = clusterLink.id;
			int arcType = clusterLink.type;
			int arcCluster = clusterLink.clusterid;

			ClusterArc* clusterArc = new ClusterArc(arcId, arcType, clusteringGraph[arcCluster]);
			clusteringGraph[clusterId]->AddArc(clusterArc);
		}

		for (CerealTypes::ClusterNode node : cluster.nodes)
		{
			int clusterNodeId = node.id;
			ClusteringNode* clusterNode = clusteringNodes[clusterNodeId];

			for (CerealTypes::ClusterVisibleNode visibleNode : node.visiblenodes)
			{
				ClusteringNode* visibilityNode = clusteringNodes[visibleNode.id];
				clusterNode->AddVisibleNode(
					visibilityNode, Length(visibilityNode->GetPos() - clusterNode->GetPos()));
			}

			for (CerealTypes::ClusterArcNode arc : node.arcs)
			{
				int arcId = arc.id;
				int arcType = arc.type;
				int arcNode = arc.nodeid;
				float weight = arc.weight;

				ClusteringArc* clusterArc = new ClusteringArc(arcId, arcType, clusteringNodes[arcNode], weight);
				clusterNode->AddArc(clusterArc);
			}

			for (CerealTypes::ClusterTransition transition : node.transitions)
			{
				int transitionId = transition.id;
				int transitionType = transition.type;

				eastl::vector<float> weights;
				eastl::vector<ClusteringNode*> nodes;
				eastl::vector<Vector3<float>> connections;
				for (int nodeid : transition.nodes)
				{
					nodes.push_back(clusteringNodes[nodeid]);
				}
				for (float weight : transition.weights)
				{
					weights.push_back(weight);
				}
				for (CerealTypes::Vec3 connection : transition.connections)
				{
					connections.push_back(Vector3<float>{
						(float)connection.x, (float)connection.y, (float)connection.z});
				}

				ClusteringTransition* clusterTransition = new ClusteringTransition(
					transitionId, transitionType, nodes, weights, connections);
				clusterNode->AddTransition(clusterTransition);
			}
		}
	}

	/*
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

	eastl::map<unsigned int, ClusteringArc*> clusteringArcs;
	for (tinyxml2::XMLElement* pCluster = pRoot->FirstChildElement(); pCluster; pCluster = pCluster->NextSiblingElement())
	{
		int clusterId = 0;
		clusterId = pCluster->IntAttribute("id", clusterId);
		Cluster* cluster = clusteringGraph[clusterId];

		for (tinyxml2::XMLElement* pClusterElement = pCluster->FirstChildElement(); pClusterElement; pClusterElement = pClusterElement->NextSiblingElement())
		{
			if (pClusterElement->Name() == "Arc")
			{
				int arcId = 0;
				int arcType = 0;
				int arcCluster = 0;

				arcId = pClusterElement->IntAttribute("id", arcId);
				arcType = pClusterElement->IntAttribute("type", arcType);
				arcCluster = pClusterElement->IntAttribute("cluster", arcCluster);

				ClusterArc* clusterArc = new ClusterArc(arcId, arcType, clusteringGraph[arcCluster]);
				cluster->AddArc(clusterArc);
			}
			else if (pClusterElement->Name() == "Node")
			{
				int clusterNodeId = 0;
				clusterNodeId = pClusterElement->IntAttribute("id", clusterNodeId);
				ClusteringNode* clusterNode = clusteringNodes[clusterNodeId];

				for (tinyxml2::XMLElement* pClusterNode = pClusterElement->FirstChildElement(); pClusterNode; pClusterNode = pClusterNode->NextSiblingElement())
				{
					if (pClusterNode->Name() == "Arc")
					{
						int arcId = 0;
						int arcType = 0;
						int arcNode = 0;
						float weight = 0.f;

						arcId = pClusterNode->IntAttribute("id", arcId);
						arcType = pClusterNode->IntAttribute("type", arcType);
						arcNode = pClusterNode->IntAttribute("node", arcNode);
						weight = pClusterNode->FloatAttribute("weight", weight);

						ClusteringArc* clusterArc = new ClusteringArc(arcId, arcType, clusteringNodes[arcNode], weight);
						clusterNode->AddArc(clusterArc);

						clusteringArcs[clusterArc->GetId()] = clusterArc;

						for (tinyxml2::XMLElement* pTransition = pClusterNode->FirstChildElement("Transition"); pTransition; pTransition = pTransition->NextSiblingElement())
						{
							int transitionId = 0;
							int transitionType = 0;
							int transitionNode = 0;

							transitionId = pTransition->IntAttribute("id", transitionId);
							transitionType = pTransition->IntAttribute("type", transitionType);
							transitionNode = pTransition->IntAttribute("node", transitionNode);

							ClusteringTransition* clusterTransition = 
								new ClusteringTransition(transitionId, transitionType, clusteringNodes[transitionNode]);
							clusterNode->AddTransition(clusterTransition);
						}
					}
				}
			}
		}
	}

	// load visibility
	ClusteringNodeeDoubleMap visibleNodes;
	for (tinyxml2::XMLElement* pNode = pRoot->FirstChildElement(); pNode; pNode = pNode->NextSiblingElement())
	{
		int pathNodeId = 0;
		pathNodeId = pNode->IntAttribute("id", pathNodeId);
		ClusteringNode* clusterNode = clusteringNodes[pathNodeId];

		for (tinyxml2::XMLElement* pElement = pNode->FirstChildElement(); pElement; pElement = pElement->NextSiblingElement())
		{
			if (pElement->Name() == "VisibleNode")
			{
				int nodeId = 0;
				nodeId = pElement->IntAttribute("id", nodeId);
				visibleNodes[clusterNode][clusteringNodes[nodeId]] = 1.0f;
			}
		}
	}

	//AddVisibility(visibleNodes);
	*/
}

/////////////////////////////////////////////////////////////////////////////
// QuakeAIManager::SaveClusteringGraph
//
//    Saves the AI clustering graph information to an XML file
//
void QuakeAIManager::SaveClusteringGraph(const eastl::string& path)
{
	//set data
	CerealTypes::ClusterGraph data;

	for (Cluster* cluster : mClusteringGraph->GetClusters())
	{
		CerealTypes::ClusterData clusterData;
		clusterData.id = cluster->GetId();
		clusterData.centerid = cluster->GetCenter()->GetId();

		for (ClusterArc* clusterArc : cluster->GetArcs())
		{
			CerealTypes::ClusterLink link;
			link.id = clusterArc->GetId();
			link.type = clusterArc->GetType();
			link.clusterid = clusterArc->GetCluster()->GetId();

			clusterData.arcs.push_back(link);
		}

		for (ClusteringNode* clusterNode : cluster->GetNodes())
		{
			CerealTypes::ClusterNode node;

			node.id = clusterNode->GetId();
			node.actorid = clusterNode->GetActor();
			node.isisolated = cluster->IsIsolatedNode(clusterNode);
			node.position.x = (int)round(clusterNode->GetPos()[0]);
			node.position.y = (int)round(clusterNode->GetPos()[1]);
			node.position.z = (int)round(clusterNode->GetPos()[2]);

			eastl::map<ClusteringNode*, float> visibilityNodes;
			clusterNode->GetVisibileNodes(visibilityNodes);
			for (auto visibilityNode : visibilityNodes)
			{
				CerealTypes::ClusterVisibleNode visibleNode;
				visibleNode.id = visibilityNode.first->GetId();

				node.visiblenodes.push_back(visibleNode);
			}

			for (ClusteringArc* clusterArc : clusterNode->GetArcs())
			{
				CerealTypes::ClusterArcNode arcNode;
				arcNode.id = clusterArc->GetId();
				arcNode.type = clusterArc->GetType();
				arcNode.nodeid = clusterArc->GetNode()->GetId();
				arcNode.weight = clusterArc->GetWeight();

				node.arcs.push_back(arcNode);
			}

			for (ClusteringTransition* clusterTransition : clusterNode->GetTransitions())
			{
				CerealTypes::ClusterTransition transition;
				transition.id = clusterTransition->GetId();
				transition.type = clusterTransition->GetType();

				for (ClusteringNode* pNode : clusterTransition->GetNodes())
				{
					transition.nodes.push_back(pNode->GetId());
				}
				for (float weight : clusterTransition->GetWeights())
				{
					transition.weights.push_back(weight);
				}
				for (Vector3<float> connection : clusterTransition->GetConnections())
				{
					transition.connections.push_back(CerealTypes::Vec3{
						(int)round(connection[0]), (int)round(connection[1]), (int)round(connection[2]) });
				}

				node.transitions.push_back(transition);
			}

			clusterData.nodes.push_back(node);
		}

		data.clusters.push_back(clusterData);
	}

	std::ofstream os(path.c_str(), std::ios::binary);
	cereal::BinaryOutputArchive archive(os);
	archive(data);

/*
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

			eastl::map<ClusteringNode*, float> visibilityNodes;
			clusterNode->GetVisibilities(VT_DISTANCE, visibilityNodes);
			for (auto visibilityNode : visibilityNodes)
			{
				tinyxml2::XMLElement* pVisibility = doc.NewElement("VisibilityNode");
				pVisibility->SetAttribute("id", eastl::to_string(visibilityNode.first->GetId()).c_str());
				pNode->LinkEndChild(pVisibility);
			}

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
				}
			}
		}
	}
	
	doc.SaveFile(path.c_str());
*/
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

	// we obtain visibility information from pathing graph 
	SimulateVisibility();

	GameLogic::Get()->GetAIManager()->SavePathingGraph(
		FileSystem::Get()->GetPath("ai/quake/bloodrunpathing.bin"));

	// we group the graph nodes in clusters
	CreateClusters();

	GameLogic::Get()->GetAIManager()->SaveClusteringGraph(
		FileSystem::Get()->GetPath("ai/quake/bloodrunclustering.bin"));

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

	// there are hundred of millions of combinations depending on the size of the map
	// which will take forever to simulate visibility thats why we have to make an aproximation
	// by grouping every position to its neareast node

	// first we get visibility info from every node by raycasting
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
				pathNode->AddVisibleNode(visibleNode, Length(visibleNode->GetPos() - pathNode->GetPos()));
		}
	}

	//next we recreate each transition to its nearest node position
	eastl::map<Vector3<float>, PathingNode*> pathingNodes;
	for (PathingNode* pathNode : mPathingGraph->GetNodes())
	{
		pathingNodes[pathNode->GetPos()] = pathNode;
		for (PathingTransition* pathTransition : pathNode->GetTransitions())
		{
			for (Vector3<float> pathConnection : pathTransition->GetConnections())
				pathingNodes[pathConnection] = mPathingGraph->FindClosestNode(pathConnection);
		}
	}

	for (PathingNode* pathNode : mPathingGraph->GetNodes())
	{
		for (PathingTransition* pathTransition : pathNode->GetTransitions())
		{
			PathingNodeVec nodes;
			eastl::map<PathingNode*, float> weights;
			eastl::map<PathingNode*, Vector3<float>> positions;
			for (unsigned int idx = 0; idx < pathTransition->GetConnections().size(); idx++)
			{
				PathingNode* pathConnection = pathingNodes[pathTransition->GetConnections()[idx]];
				if (eastl::find(nodes.begin(), nodes.end(), pathConnection) == nodes.end())
				{
					weights[pathConnection] = 0;
					nodes.push_back(pathConnection);
				}

				weights[pathConnection] += pathTransition->GetWeights()[idx];
				positions[pathConnection] = pathTransition->GetConnections()[idx];
			}
			
			eastl::vector<float> nodeWeights;
			eastl::vector<Vector3<float>> nodePositions;
			for (PathingNode* node : nodes)
			{
				nodeWeights.push_back(weights[node]);
				nodePositions.push_back(positions[node]);
			}

			pathNode->RemoveTransition(pathTransition->GetId());
			pathNode->AddTransition(new PathingTransition(
				pathTransition->GetId(), pathTransition->GetType(), nodes, nodeWeights, nodePositions));
		}
	}

	//visibility between pathing transitions
	/*
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
				pNode->RemoveArcs();
				pNode->RemoveTransitions();
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

						PathingTransition* pathTransition = pathNode->FindTransition(pathArc->GetId());
						if (pathTransition)
						{
							ClusteringNodeVec clusteringNodes;
							for (PathingNode* pNode : pathTransition->GetNodes())
								clusteringNodes.push_back(clusterNodes[pNode->GetId()]);

							clusterNode->AddTransition(new ClusteringTransition(clusterArc->GetId(), clusterArc->GetType(),
								clusteringNodes, pathTransition->GetWeights(), pathTransition->GetConnections()));
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

						PathingTransition* pathTransition = pathNode->FindTransition(pathArc.second->GetId());
						if (pathTransition)
						{
							ClusteringNodeVec clusteringNodes;
							for (PathingNode* pNode : pathTransition->GetNodes())
								clusteringNodes.push_back(clusterNodes[pNode->GetId()]);

							clusterNode->AddTransition(new ClusteringTransition(clusterArc->GetId(), clusterArc->GetType(),
								clusteringNodes, pathTransition->GetWeights(), pathTransition->GetConnections()));
						}
					}
				}
			}
		}
	}

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

					ClusteringTransition* clusteringTransition = 
						new ClusteringTransition(clusteringArc->GetId(), pBeginArc->GetType(), 
						{ pBeginArc->GetNode() }, { pBeginArc->GetWeight() }, { clusterNode->GetPos() });
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

					ClusteringTransition* clusteringTransition =
						new ClusteringTransition(clusteringArc->GetId(), pBeginArc->GetType(),
						{ pBeginArc->GetNode() }, { pBeginArc->GetWeight() }, { clusterNode->GetPos() });
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

	//finally we add visibility info to clusters
	for (PathingNode* pathNode : mPathingGraph->GetNodes())
	{
		ClusteringNode* clusterNode = clusterNodes[pathNode->GetId()];
		eastl::map<PathingNode*, float> visibleNodes;
		pathNode->GetVisibileNodes(visibleNodes);
		for (auto visibleNode : visibleNodes)
		{
			clusterNode->AddVisibleNode(
				clusterNodes[visibleNode.first->GetId()], visibleNode.second);
		}
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
				Vector3<float> position = pNode->GetPos();

				eastl::vector<float> weights;
				eastl::vector<PathingNode*> nodes;
				eastl::vector<Vector3<float>> positions;
				for (unsigned int idx = 0; idx < nodePositions.size(); idx++)
				{
					deltaTime += 0.02f;

					if (Length(nodePositions[idx] - position) >= 16.f)
					{
						nodes.push_back(pNode);
						weights.push_back(deltaTime);
						positions.push_back(nodePositions[idx]);

						deltaTime = 0.f;
					}
				}

				if (!nodes.empty())
				{
					PathingTransition* pTransition = new PathingTransition(
						pArc->GetId(), GAT_TELEPORT, nodes, weights, positions);
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
				Vector3<float> position = pNode->GetPos();

				eastl::vector<float> weights;
				eastl::vector<PathingNode*> nodes;
				eastl::vector<Vector3<float>> positions;
				for (unsigned int idx = 0; idx < nodePositions.size(); idx++)
				{
					deltaTime += 0.02f;

					if (Length(nodePositions[idx] - position) >= 16.f)
					{
						nodes.push_back(pNode);
						weights.push_back(deltaTime);
						positions.push_back(nodePositions[idx]);

						deltaTime = 0.f;
					}
				}

				if (!nodes.empty())
				{
					PathingTransition* pTransition = new PathingTransition(
						pArc->GetId(), GAT_PUSH, nodes, weights, positions);
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
				if (Length((*itMove) - positions.back()) >= 16.f)
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

								//lets interpolate transitions from the already created arc
								if (!positions.empty())
								{
									PathingTransition* pTransition = new PathingTransition(
										pArc->GetId(), arcType, { pCurrentNode }, times, positions);
									pCurrentNode->AddTransition(pTransition);
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

									//lets interpolate transitions from the already created arc
									if (!positions.empty())
									{
										PathingTransition* pTransition = new PathingTransition(
											pArc->GetId(), arcType, { pCurrentNode }, times, positions);
										pCurrentNode->AddTransition(pTransition);
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
					pEndNode->GetPos()[2] - pNode->GetPos()[2] >= 20.f)
				{
					Vector3<float> diff = pEndNode->GetPos() - position;
					if (Length(diff) <= PATHING_DEFAULT_NODE_TOLERANCE)
					{
						PathingArc* pArc = new PathingArc(GetNewArcID(), GAT_JUMP, pEndNode, totalTime);
						pNode->AddArc(pArc);

						//lets interpolate transitions from the already created arc
						float deltaTime = 0.f;
						Vector3<float> position = pNode->GetPos();

						eastl::vector<float> weights;
						eastl::vector<PathingNode*> nodes;
						eastl::vector<Vector3<float>> positions;
						for (unsigned int idx = 0; idx < nodePositions.size(); idx++)
						{
							deltaTime += 0.02f;

							if (Length(nodePositions[idx] - position) >= 16.f)
							{
								nodes.push_back(pNode);
								weights.push_back(deltaTime);
								positions.push_back(nodePositions[idx]);

								deltaTime = 0.f;
							}
						}

						if (!nodes.empty())
						{
							PathingTransition* pTransition = new PathingTransition(
								pArc->GetId(), GAT_JUMP, nodes, weights, positions);
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