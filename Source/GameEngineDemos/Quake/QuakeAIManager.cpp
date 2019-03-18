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
	mLastNodeId = 0;
	mLastArcId = 0;

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
			float weight = 0.f;
			PathingNode* links[2];
			Vector3<float> connection = NULL;

			arcId = pNode->IntAttribute("id", arcId);
			arcType = pArc->IntAttribute("type", arcType);
			weight = pArc->FloatAttribute("weight", weight);
			if (arcType == 13 || arcType == 14) continue;

			tinyxml2::XMLElement* pLinkElement = pArc->FirstChildElement("Link");
			if (pLinkElement)
			{
				int nodeA = 0;
				int nodeB = 0;
				nodeA = pLinkElement->IntAttribute("a", nodeA);
				nodeB = pLinkElement->IntAttribute("b", nodeB);

				links[0] = pathingNodeGraph[nodeA];
				links[1] = pathingNodeGraph[nodeB];
			}

			tinyxml2::XMLElement* pConnectionElement = pArc->FirstChildElement("Connection");
			if (pConnectionElement)
			{
				float x = 0;
				float y = 0;
				float z = 0;
				x = pConnectionElement->FloatAttribute("x", x);
				y = pConnectionElement->FloatAttribute("y", y);
				z = pConnectionElement->FloatAttribute("z", z);

				connection = Vector3<float>{ x, y, z };
			}

			PathingArc* pathArc = new PathingArc(arcId, arcType, weight, connection);
			pathArc->LinkNodes(links[0], links[1]);
			pathNode->AddArc(pathArc);
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
// QuakeAIManager::SaveMapGraph
//
//    Saves the AI pathing graph information to an XML file
//
void QuakeAIManager::SaveMapGraph(const eastl::string& path)
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
			pArc->SetAttribute("weight", eastl::to_string(pathArc->GetWeight()).c_str());
			pNode->LinkEndChild(pArc);

			tinyxml2::XMLElement* pLink = doc.NewElement("Link");
			pLink->SetAttribute("a", eastl::to_string(pathArc->GetOrigin()->GetId()).c_str());
			pLink->SetAttribute("b", eastl::to_string(pathArc->GetNeighbor()->GetId()).c_str());
			pArc->LinkEndChild(pLink);

			if (pathArc->GetConnection() != NULL)
			{
				tinyxml2::XMLElement* pConnection = doc.NewElement("Connection");
				pConnection->SetAttribute("x", eastl::to_string((int)round(pathArc->GetConnection()[0])).c_str());
				pConnection->SetAttribute("y", eastl::to_string((int)round(pathArc->GetConnection()[1])).c_str());
				pConnection->SetAttribute("z", eastl::to_string((int)round(pathArc->GetConnection()[2])).c_str());
				pArc->LinkEndChild(pConnection);
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

	//first we store the most important points of the map
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

	// we obtain visibility information from the created waypoint graph by using raycasting
	SimulateVisibility();

	/*
	eastl::vector<Point> points;
	eastl::map<int, eastl::map<int, float>> distances;
	for (PathingNode* pathNode : mPathingGraph->GetNodes())
	{
		eastl::vector<float> pos{
		pathNode->GetPos()[0], pathNode->GetPos()[1], pathNode->GetPos()[2]};
		Point point(pathNode->GetId(), pos);
		points.push_back(point);
	}

	//path distances
	for (PathingNode* pathNode : mPathingGraph->GetNodes())
		for (auto node : mPathingGraph->FindPaths(pathNode, 1.0f))
			distances[pathNode->GetId()][node.first->GetId()] = node.second;

	//Running K-Means Clustering
	int iters = 100;
	KMeans kmeans(500, iters);
	kmeans.Run(points, distances);

	GameLogic::Get()->GetAIManager()->SaveMapGraph(
		FileSystem::Get()->GetPath("ai/quake/bloodrun - copia.xml"));
	*/
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
	PathingArcNodeMap pathArcNodes;
	PathingArcNodeDoubleMap pathArcNodesTime;

	PathingNodeDoubleMap visibleNodes;
	PathingNodeArcDoubleMap visibleNodeArcs;

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
				visibleNodes[pathNode][visibleNode] = 1.f;
			else
				visibleNodes[pathNode][visibleNode] = 0.f;
		}

		for (PathingNode* visibleNode : mPathingGraph->GetNodes())
		{
			for (PathingArc* visibleArc : visibleNode->GetArcs())
			{
				if (visibleArc->GetType() == AIAT_JUMP || 
					visibleArc->GetType() == AIAT_FALL ||
					visibleArc->GetType() == AIAT_PUSH)
				{
					Vector3<float> end = visibleArc->GetConnection() +
						(float)mPlayerActor->GetState().viewHeight * Vector3<float>::Unit(YAW);

					eastl::vector<ActorId> collisionActors;
					eastl::vector<Vector3<float>> collisions, collisionNormals;
					gamePhysics->CastRay(muzzle, end, collisionActors, collisions, collisionNormals);

					Vector3<float> collision = NULL;
					for (unsigned int i = 0; i < collisionActors.size(); i++)
						if (collisionActors[i] == INVALID_ACTOR_ID)
							collision = collisions[i];

					if (collision == NULL)
						visibleNodeArcs[pathNode][visibleArc] = 1.0f;
					else
						visibleNodeArcs[pathNode][visibleArc] = 0.f;
				}
			}
		}

		for (PathingArc* pathArc : pathNode->GetArcs())
		{
			if (pathArc->GetType() == AIAT_MOVE)
			{
				Vector3<float> pathDirection = 
					pathArc->GetNeighbor()->GetPos() - pathArc->GetOrigin()->GetPos();
				float pathDistance = Length(pathDirection) * (0.02f / pathArc->GetWeight());
				Normalize(pathDirection);

				float deltaTime = 0.f;
				PathingNode* currentPathNode = NULL;
				pathArcNodes[pathArc] = PathingNodeVec();
				for (float w = 0; w <= pathArc->GetWeight(); w += 0.02f)
				{
					PathingNode* closestPathNode = 
						mPathingGraph->FindClosestNode(pathArc->GetOrigin()->GetPos() + 
						pathDirection * (pathDistance * w / pathArc->GetWeight()));

					if (closestPathNode != currentPathNode)
						deltaTime = 0.02f;

					if (eastl::find(
							pathArcNodes[pathArc].begin(), 
							pathArcNodes[pathArc].end(), 
							closestPathNode) == pathArcNodes[pathArc].end())
					{
						pathArcNodes[pathArc].push_back(closestPathNode);
					}
					pathArcNodesTime[pathArc][closestPathNode] = deltaTime;

					deltaTime += 0.02f;
					currentPathNode = closestPathNode;
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
			if (visibleNodes[pathNode][visibleNode])
			{
				mVisibleNodes[pathNode][visibleNode] = 
					Length(visibleNode->GetPos() - pathNode->GetPos());
			}
		}

		//nodes to arcs
		for (PathingArc* pathArc : mPathingGraph->GetArcs())
		{
			if (pathArc->GetType() == AIAT_MOVE)
			{
				float totalTime = 0.f, distance = 0.f;
				for (PathingNode* pathArcNode : pathArcNodes[pathArc])
				{
					if (visibleNodes[pathNode][pathArcNode])
					{
						float deltaTime = pathArcNodesTime[pathArc][pathNode];
						distance += Length(
							pathArcNode->GetPos() - pathNode->GetPos()) * deltaTime;
						totalTime += deltaTime;
					}
				}
				if (totalTime > 0.f)
				{
					distance /= totalTime;

					mVisibleNodeArcs[pathNode][pathArc] = distance;
					mVisibleNodeArcsTime[pathNode][pathArc] = totalTime;
				}
			}
			else
			{
				if (pathArc->GetType() == AIAT_JUMPTARGET || 
					pathArc->GetType() == AIAT_FALLTARGET ||
					pathArc->GetType() == AIAT_PUSHTARGET)
				{
					PathingArc* visibleArc = pathArc->GetOrigin()->FindArc(
						pathArc->GetType()-1, pathArc->GetNeighbor());

					float totalTime = 0.f, distance = 0.f;
					do
					{
						if (visibleNodeArcs[pathNode][visibleArc])
						{
							float deltaTime = visibleArc->GetWeight();
							distance += Length(
								visibleArc->GetConnection() - pathNode->GetPos()) * deltaTime;
							totalTime += visibleArc->GetWeight();
						}
						visibleArc = visibleArc->GetNeighbor()->FindArc(
							pathArc->GetType() - 1, pathArc->GetNeighbor());
					} while(pathArc->GetNeighbor() != visibleArc->GetOrigin());

					if (totalTime > 0.f)
					{
						distance /= totalTime;

						mVisibleNodeArcs[pathNode][pathArc] = distance;
						mVisibleNodeArcsTime[pathNode][pathArc] = totalTime;
					}
				}
			}
		}
	}

	//arcs to nodes
	for (PathingArc* pathArc : mPathingGraph->GetArcs())
	{
		if (pathArc->GetType() == AIAT_MOVE)
		{
			for (PathingNode* pathNode : mPathingGraph->GetNodes())
			{
				float totalTime = 0.f, distance = 0.f;
				for (PathingNode* pathArcNode : pathArcNodes[pathArc])
				{
					if (visibleNodes[pathNode][pathArcNode])
					{
						float deltaTime = pathArcNodesTime[pathArc][pathNode];
						distance += Length(pathArcNode->GetPos() - pathNode->GetPos()) * deltaTime;
						totalTime += deltaTime;
					}
				}
				if (totalTime > 0.f)
				{
					distance /= totalTime;

					mVisibleArcNodes[pathArc][pathNode] = distance;
					mVisibleArcNodesTime[pathArc][pathNode] = totalTime;
				}
			}
		}
	}

	//arcs to arcs
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
		Vector3<float> position = itActorNode->first->GetPos();
		eastl::shared_ptr<Actor> pItemActor(
			GameLogic::Get()->GetActor(itActorNode->second).lock());
		PathingNode* pClosestNode = mPathingGraph->FindClosestNode(position);
		if (pClosestNode != NULL)
		{
			pClosestNode->SetActorId(pItemActor->GetId());

			if (pItemActor->GetType() == "Trigger")
			{
				pClosestNode->RemoveArcs();
				if (pItemActor->GetComponent<PushTrigger>(PushTrigger::Name).lock())
				{
					eastl::shared_ptr<PushTrigger> pPushTrigger =
						pItemActor->GetComponent<PushTrigger>(PushTrigger::Name).lock();

					Vector3<float> targetPosition = pPushTrigger->GetTarget().GetTranslation();
					SimulateTriggerPush(pClosestNode, targetPosition);
				}
				else if (pItemActor->GetComponent<TeleporterTrigger>(TeleporterTrigger::Name).lock())
				{
					eastl::shared_ptr<TeleporterTrigger> pTeleporterTrigger =
						pItemActor->GetComponent<TeleporterTrigger>(TeleporterTrigger::Name).lock();

					Vector3<float> targetPosition = pTeleporterTrigger->GetTarget().GetTranslation();
					SimulateTriggerTeleport(pClosestNode, targetPosition);
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
	mActorNodes.clear();
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

	// nodes closed to teleport position
	eastl::vector<PathingNode*> nodes;
	eastl::map<PathingNode*, float> nodeTimes;
	eastl::map<PathingNode*, Vector3<float>> nodePositions;

	// gravity falling simulation
	transform = gamePhysics->GetTransform(mPlayerActor->GetId());

	PathingNode* pFallingNode = pNode;
	float totalTime = 0.f, deltaTime = 0.f, fallSpeed = 0.f;
	while (!gamePhysics->OnGround(mPlayerActor->GetId()) && totalTime <= 10.f)
	{
		float jumpSpeed = gamePhysics->GetJumpSpeed(mPlayerActor->GetId());

		totalTime += 0.02f;
		deltaTime += 0.02f;
		fallSpeed += (20.f / (jumpSpeed * 0.5f));
		if (fallSpeed > mMaxFallSpeed) fallSpeed = mMaxFallSpeed;

		PathingNode* pClosestNode =
			mPathingGraph->FindClosestNode(transform.GetTranslation());
		if (pClosestNode != NULL)
		{
			if (pClosestNode != pFallingNode)
				deltaTime = 0.02f;

			if (eastl::find(nodes.begin(), nodes.end(), pClosestNode) == nodes.end())
				nodes.push_back(pClosestNode);
			nodeTimes[pClosestNode] = deltaTime;
			nodePositions[pClosestNode] = transform.GetTranslation();
			pFallingNode = pClosestNode;
		}

		Normalize(direction);
		direction[PITCH] *= jumpSpeed * (fallSpeed / 4.f);
		direction[ROLL] *= jumpSpeed * (fallSpeed / 4.f);
		direction[YAW] = -jumpSpeed * fallSpeed;

		gamePhysics->FallDirection(mPlayerActor->GetId(), direction);
		gamePhysics->OnUpdate(0.02f);

		transform = gamePhysics->GetTransform(mPlayerActor->GetId());
	}

	if (totalTime > 10.f) return;
	totalTime += 0.02f;

	Vector3<float> position = transform.GetTranslation();
	PathingNode* pEndNode = mPathingGraph->FindClosestNode(position);
	if (pNode != pEndNode)
	{
		if (pEndNode != NULL && pNode->FindArc(AIAT_TELEPORTTARGET, pEndNode) == NULL)
		{
			Vector3<float> diff = pEndNode->GetPos() - position;
			if (Length(diff) <= PATHING_DEFAULT_NODE_TOLERANCE)
			{
				PathingArc* pArc = new PathingArc(GetNewArcID(), AIAT_TELEPORTTARGET, totalTime);
				pArc->LinkNodes(pNode, pEndNode);
				pNode->AddArc(pArc);

				PathingNode* pCurrentNode = pNode;
				eastl::vector<PathingNode*>::iterator itNode;
				for (itNode = nodes.begin(); itNode != nodes.end(); itNode++)
				{
					pFallingNode = (*itNode);
					if (pCurrentNode->FindArc(AIAT_TELEPORT, pEndNode) == NULL)
					{
						PathingArc* pArc = new PathingArc(GetNewArcID(),
							AIAT_TELEPORT, nodeTimes[pFallingNode], nodePositions[pFallingNode]);
						pArc->LinkNodes(pEndNode, pFallingNode);
						pCurrentNode->AddArc(pArc);
					}

					pCurrentNode = pFallingNode;
				}

				if (pCurrentNode != pEndNode)
				{
					if (pCurrentNode->FindArc(AIAT_TELEPORT, pEndNode) == NULL)
					{
						deltaTime = 0.02f;
						PathingArc* pArc = new PathingArc(GetNewArcID(), 
							AIAT_TELEPORT, deltaTime, pEndNode->GetPos());
						pArc->LinkNodes(pEndNode, pEndNode);
						pCurrentNode->AddArc(pArc);
					}
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

	// nodes closed to jump position
	eastl::vector<PathingNode*> nodes;
	eastl::map<PathingNode*, float> nodeTimes;
	eastl::map<PathingNode*, Vector3<float>> nodePositions;

	// gravity falling simulation
	transform = gamePhysics->GetTransform(mPlayerActor->GetId());

	PathingNode* pFallingNode = pNode;
	float totalTime = 0.f, deltaTime = 0.f, fallSpeed = 0.f;
	while (!gamePhysics->OnGround(mPlayerActor->GetId()) && totalTime <= 10.f)
	{
		float jumpSpeed = gamePhysics->GetJumpSpeed(mPlayerActor->GetId());

		totalTime += 0.02f;
		deltaTime += 0.02f;
		fallSpeed += (20.f / (jumpSpeed * 0.5f));
		if (fallSpeed > mMaxFallSpeed) fallSpeed = mMaxFallSpeed;

		PathingNode* pClosestNode =
			mPathingGraph->FindClosestNode(transform.GetTranslation());
		if (pClosestNode != NULL)
		{
			if (pClosestNode != pFallingNode)
				deltaTime = 0.02f;

			if (eastl::find(nodes.begin(), nodes.end(), pClosestNode) == nodes.end())
				nodes.push_back(pClosestNode);
			nodeTimes[pClosestNode] = deltaTime;
			nodePositions[pClosestNode] = transform.GetTranslation();
			pFallingNode = pClosestNode;
		}

		Normalize(direction);
		direction[PITCH] *= jumpSpeed * (fallSpeed / 4.f);
		direction[ROLL] *= jumpSpeed * (fallSpeed / 4.f);
		direction[YAW] = -jumpSpeed * fallSpeed;

		gamePhysics->FallDirection(mPlayerActor->GetId(), direction);
		gamePhysics->OnUpdate(0.02f);

		transform = gamePhysics->GetTransform(mPlayerActor->GetId());
	}

	if (totalTime > 10.f) return;
	totalTime += 0.02f;

	//we store the jump if we find a landing node
	Vector3<float> position = transform.GetTranslation();
	PathingNode* pEndNode = mPathingGraph->FindClosestNode(position);
	if (pNode != pEndNode)
	{
		if (pEndNode != NULL && pNode->FindArc(AIAT_PUSHTARGET, pEndNode) == NULL)
		{
			Vector3<float> diff = pEndNode->GetPos() - position;
			if (Length(diff) <= PATHING_DEFAULT_NODE_TOLERANCE)
			{
				PathingArc* pArc = new PathingArc(GetNewArcID(), AIAT_PUSHTARGET, totalTime);
				pArc->LinkNodes(pNode, pEndNode);
				pNode->AddArc(pArc);

				PathingNode* pCurrentNode = pNode;
				eastl::vector<PathingNode*>::iterator itNode;
				for (itNode = nodes.begin(); itNode != nodes.end(); itNode++)
				{
					pFallingNode = (*itNode);
					if (pCurrentNode->FindArc(AIAT_PUSH, pEndNode) == NULL)
					{
						PathingArc* pArc = new PathingArc(GetNewArcID(),
							AIAT_PUSH, nodeTimes[pFallingNode], nodePositions[pFallingNode]);
						pArc->LinkNodes(pEndNode, pFallingNode);
						pCurrentNode->AddArc(pArc);
					}

					pCurrentNode = pFallingNode;
				}

				if (pCurrentNode != pEndNode)
				{
					if (pCurrentNode->FindArc(AIAT_PUSH, pEndNode) == NULL)
					{
						deltaTime = 0.02f;
						PathingArc* pArc = new PathingArc(GetNewArcID(), 
							AIAT_PUSH, deltaTime, pEndNode->GetPos());
						pArc->LinkNodes(pEndNode, pEndNode);
						pCurrentNode->AddArc(pArc);
					}
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
	for (int angle = 0; angle < 360; angle += 10)
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
	eastl::vector<PathingNode*> nodes;
	eastl::map<PathingNode*, float> nodeTimes;
	eastl::map<PathingNode*, Vector3<float>> nodePositions;

	for (int angle = 0; angle < 360; angle += 10)
	{
		Matrix4x4<float> rotation = Rotation<4, float>(
			AxisAngle<4, float>(Vector4<float>::Unit(2), angle * (float)GE_C_DEG_TO_RAD));

		transform.SetTranslation(pNode->GetPos());
		transform.SetRotation(rotation);

		//create movements on the ground
		eastl::vector<Vector3<float>> movements;
		Vector3<float> position = pNode->GetPos();
		PathingNode* pEndNode = NULL;
		PathingNode* pCurrentNode = pNode;
		do
		{
			pEndNode = NULL;

			if (!gamePhysics->OnGround(mPlayerActor->GetId()))
			{
				float deltaTime = 0.f;

				if (!movements.empty())
				{
					eastl::vector<Vector3<float>>::iterator itMove;
					for (itMove = movements.begin(); itMove != movements.end(); itMove++)
					{
						deltaTime += 0.02f;
						PathingNode* pClosestNode = mPathingGraph->FindClosestNode((*itMove));
						if (pCurrentNode != pClosestNode)
						{
							Vector3<float> diff = pClosestNode->GetPos() - (*itMove);
							if (Length(diff) >= 16.f)
							{
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
										PathingNode* pNewNode = new PathingNode(
											GetNewNodeID(), INVALID_ACTOR_ID, (*itMove));
										mPathingGraph->InsertNode(pNewNode);
										PathingArc* pArc = new PathingArc(GetNewArcID(), AIAT_MOVE, deltaTime);
										pArc->LinkNodes(pCurrentNode, pNewNode);
										pCurrentNode->AddArc(pArc);

										mOpenSet[pNewNode] = true;
										pCurrentNode = pNewNode;

										deltaTime = 0.f;
									}
								}
							}
							else if (Length(diff) <= PATHING_DEFAULT_NODE_TOLERANCE)
							{
								if (pCurrentNode->FindArc(AIAT_MOVE, pClosestNode) == NULL)
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
										PathingArc* pArc = new PathingArc(GetNewArcID(), AIAT_MOVE, deltaTime);
										pArc->LinkNodes(pCurrentNode, pClosestNode);
										pCurrentNode->AddArc(pArc);

										pCurrentNode = pClosestNode;

										deltaTime = 0.f;
									}
								}
							}
						}
					}
				}
				movements.clear();

				nodes.clear();
				nodeTimes.clear();
				nodePositions.clear();

				deltaTime = 0.f;
				float totalTime = 0.f, fallSpeed = 2.f;

				PathingNode* pFallingNode = pCurrentNode;
				do
				{
					float jumpSpeed = gamePhysics->GetJumpSpeed(mPlayerActor->GetId());

					totalTime += 0.02f;
					deltaTime += 0.02f;
					fallSpeed += (20.f / (jumpSpeed * 0.5f));
					if (fallSpeed > mMaxFallSpeed) fallSpeed = mMaxFallSpeed;

					PathingNode* pClosestNode =
						mPathingGraph->FindClosestNode(transform.GetTranslation());
					if (pClosestNode != NULL)
					{
						if (pClosestNode != pFallingNode)
							deltaTime = 0.02f;

						if (eastl::find(nodes.begin(), nodes.end(), pClosestNode) == nodes.end())
							nodes.push_back(pClosestNode);
						nodeTimes[pClosestNode] = deltaTime;
						nodePositions[pClosestNode] = transform.GetTranslation();
						pFallingNode = pClosestNode;
					}

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
				} while (!gamePhysics->OnGround(mPlayerActor->GetId()) && totalTime <= 10.f);

				if (totalTime > 10.f) break;

				//we store the fall if we find a landing node
				position = transform.GetTranslation();
				pEndNode = mPathingGraph->FindClosestNode(position);
				if (pCurrentNode != pEndNode)
				{
					if (pEndNode != NULL && pCurrentNode->FindArc(AIAT_FALLTARGET, pEndNode) == NULL)
					{
						Vector3<float> diff = pEndNode->GetPos() - position;
						if (Length(diff) <= PATHING_DEFAULT_NODE_TOLERANCE)
						{
							Vector3<float> scale = gamePhysics->GetScale(mPlayerActor->GetId()) / 2.f;

							Transform start;
							start.SetTranslation(pCurrentNode->GetPos() + scale[YAW] * Vector3<float>::Unit(YAW));
							Transform end;
							end.SetTranslation(pEndNode->GetPos() + scale[YAW] * Vector3<float>::Unit(YAW));
							Vector3<float> collision, collisionNormal;
							ActorId actorId = gamePhysics->ConvexSweep(
								mPlayerActor->GetId(), start, end, collision, collisionNormal);
							if (collision != NULL && actorId == INVALID_ACTOR_ID) break;
						}
						else if (Length(diff) >= 16.f)
						{
							if (!Cliff(position))
							{
								pEndNode = new PathingNode(GetNewNodeID(), INVALID_ACTOR_ID, position);
								mPathingGraph->InsertNode(pEndNode);
							}
							else break;
						}
						else break;

						PathingArc* pArc = new PathingArc(GetNewArcID(), AIAT_FALLTARGET, totalTime);
						pArc->LinkNodes(pCurrentNode, pEndNode);
						pCurrentNode->AddArc(pArc);

						eastl::vector<PathingNode*>::iterator itNode;
						for (itNode = nodes.begin(); itNode != nodes.end(); itNode++)
						{
							pFallingNode = (*itNode);
							if (pCurrentNode->FindArc(AIAT_FALL, pEndNode) == NULL)
							{
								PathingArc* pArc = new PathingArc(GetNewArcID(),
									AIAT_FALL, nodeTimes[pFallingNode], nodePositions[pFallingNode]);
								pArc->LinkNodes(pEndNode, pFallingNode);
								pCurrentNode->AddArc(pArc);
							}

							pCurrentNode = pFallingNode;
						}

						if (pCurrentNode != pEndNode)
						{
							if (pCurrentNode->FindArc(AIAT_FALL, pEndNode) == NULL)
							{
								deltaTime = 0.02f;
								PathingArc* pArc = new PathingArc(GetNewArcID(), AIAT_FALL, deltaTime);
								pArc->LinkNodes(pEndNode, pEndNode);
								pCurrentNode->AddArc(pArc);
							}
							pCurrentNode = pEndNode;
						}
					}
					else break;
				}
			}
			else
			{
				PathingNode* pClosestNode = mPathingGraph->FindClosestNode(position);
				if (pClosestNode != pCurrentNode)
				{
					// if we find a link to the closest node then we stop
					Vector3<float> diff = pClosestNode->GetPos() - position;
					if (Length(diff) <= PATHING_DEFAULT_NODE_TOLERANCE)
					{
						if (pCurrentNode->FindArc(AIAT_MOVE, pClosestNode) != NULL)
						{
							pEndNode = pClosestNode;
							break;
						}
					}
				}
				movements.push_back(position);

				Vector3<float> direction; // forward vector
#if defined(GE_USE_MAT_VEC)
				direction = HProject(rotation * Vector4<float>::Unit(PITCH));
#else
				direction = HProject(Vector4<float>::Unit(PITCH) * rotation);
#endif

				gamePhysics->SetTransform(mPlayerActor->GetId(), transform);
				gamePhysics->WalkDirection(mPlayerActor->GetId(), direction * mMoveSpeed);
				gamePhysics->OnUpdate(0.02f);
			}

			transform = gamePhysics->GetTransform(mPlayerActor->GetId());
			position = transform.GetTranslation();

		} while (FindClosestMovement(movements, position) >= 4.f); // stalling is a break condition

		if (!movements.empty())
		{
			float deltaTime = 0.f;
			eastl::vector<Vector3<float>>::iterator itMove;
			for (itMove = movements.begin(); itMove != movements.end(); itMove++)
			{
				deltaTime += 0.02f;
				PathingNode* pClosestNode = mPathingGraph->FindClosestNode((*itMove));
				if (pCurrentNode != pClosestNode)
				{
					Vector3<float> diff = pClosestNode->GetPos() - (*itMove);
					if (Length(diff) >= 16.f)
					{
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
								PathingNode* pNewNode = new PathingNode(
									GetNewNodeID(), INVALID_ACTOR_ID, (*itMove));
								mPathingGraph->InsertNode(pNewNode);
								PathingArc* pArc = new PathingArc(GetNewArcID(), AIAT_MOVE, deltaTime);
								pArc->LinkNodes(pCurrentNode, pNewNode);
								pCurrentNode->AddArc(pArc);

								mOpenSet[pNewNode] = true;
								pCurrentNode = pNewNode;

								deltaTime = 0.f;
							}
						}
					}
					else if (Length(diff) <= PATHING_DEFAULT_NODE_TOLERANCE)
					{
						if (pCurrentNode->FindArc(AIAT_MOVE, pClosestNode) == NULL)
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
								PathingArc* pArc = new PathingArc(GetNewArcID(), AIAT_MOVE, deltaTime);
								pArc->LinkNodes(pCurrentNode, pClosestNode);
								pCurrentNode->AddArc(pArc);

								pCurrentNode = pClosestNode;

								deltaTime = 0.f;
							}
						}
					}
				}
			}
			deltaTime += 0.02f;

			if (pEndNode != NULL && pCurrentNode != pEndNode)
			{
				if (pCurrentNode->FindArc(AIAT_MOVE, pEndNode) == NULL)
				{
					Vector3<float> scale = gamePhysics->GetScale(mPlayerActor->GetId()) / 2.f;

					Transform start;
					start.SetTranslation(pCurrentNode->GetPos() + scale[YAW] * Vector3<float>::Unit(YAW));
					Transform end;
					end.SetTranslation(pEndNode->GetPos() + scale[YAW] * Vector3<float>::Unit(YAW));
					Vector3<float> collision, collisionNormal;
					ActorId actorId = gamePhysics->ConvexSweep(
						mPlayerActor->GetId(), start, end, collision, collisionNormal);
					if (collision == NULL || actorId != INVALID_ACTOR_ID)
					{
						PathingArc* pArc = new PathingArc(GetNewArcID(), AIAT_MOVE, deltaTime);
						pArc->LinkNodes(pCurrentNode, pEndNode);
						pCurrentNode->AddArc(pArc);
					}
				}
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
	for (int angle = 0; angle < 360; angle += 10)
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

		// nodes closed to jump position
		eastl::vector<PathingNode*> nodes;
		eastl::map<PathingNode*, float> nodeTimes;
		eastl::map<PathingNode*, Vector3<float>> nodePositions;

		float fallSpeed = 0.f, deltaTime = 0.f, totalTime = 0.f;
		PathingNode* pFallingNode = pNode;

		// gravity falling simulation
		while (!gamePhysics->OnGround(mPlayerActor->GetId()) && totalTime <= 10.f)
		{
			float jumpSpeed = gamePhysics->GetJumpSpeed(mPlayerActor->GetId());

			totalTime += 0.02f;
			deltaTime += 0.02f;
			fallSpeed += (20.f / (jumpSpeed * 0.5f));
			if (fallSpeed > mMaxFallSpeed) fallSpeed = mMaxFallSpeed;

			PathingNode* pClosestNode =
				mPathingGraph->FindClosestNode(transform.GetTranslation());
			if (pClosestNode != NULL)
			{
				if (pClosestNode != pFallingNode)
					deltaTime = 0.02f;

				if (eastl::find(nodes.begin(), nodes.end(), pClosestNode) == nodes.end())
					nodes.push_back(pClosestNode);
				nodeTimes[pClosestNode] = deltaTime;
				nodePositions[pClosestNode] = transform.GetTranslation();
				pFallingNode = pClosestNode;
			}

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
			if (pEndNode != NULL && pNode->FindArc(AIAT_JUMPTARGET, pEndNode) == NULL)
			{
				//check if we have done a clean jump (no collisions)
				if (Length(pEndNode->GetPos() - pNode->GetPos()) >= 300.f ||
					pEndNode->GetPos()[2] - pNode->GetPos()[2] >= 30.f)
				{
					Vector3<float> diff = pEndNode->GetPos() - position;
					if (Length(diff) <= PATHING_DEFAULT_NODE_TOLERANCE)
					{
						PathingArc* pArc = new PathingArc(GetNewArcID(), AIAT_JUMPTARGET, totalTime);
						pArc->LinkNodes(pNode, pEndNode);
						pNode->AddArc(pArc);

						PathingNode* pCurrentNode = pNode;
						eastl::vector<PathingNode*>::iterator itNode;
						for (itNode = nodes.begin(); itNode != nodes.end(); itNode++)
						{
							pFallingNode = (*itNode);
							if (pCurrentNode->FindArc(AIAT_JUMP, pEndNode) == NULL)
							{
								PathingArc* pArc = new PathingArc(GetNewArcID(),
									AIAT_JUMP, nodeTimes[pFallingNode], nodePositions[pFallingNode]);
								pArc->LinkNodes(pEndNode, pFallingNode);
								pCurrentNode->AddArc(pArc);
							}

							pCurrentNode = pFallingNode;
						}

						if (pCurrentNode != pEndNode)
						{
							if (pCurrentNode->FindArc(AIAT_JUMP, pEndNode) == NULL)
							{
								deltaTime = 0.02f;
								PathingArc* pArc = new PathingArc(GetNewArcID(),
									AIAT_JUMP, deltaTime, pEndNode->GetPos());
								pArc->LinkNodes(pEndNode, pEndNode);
								pCurrentNode->AddArc(pArc);
							}
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
						mActorNodes[pClosestNode] = pItemActor->GetId();
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