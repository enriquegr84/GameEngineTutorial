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
		short x, y, z;

		template <class Archive>
		void serialize(Archive & ar)
		{
			ar(x, y, z);
		}
	};

	struct VisibleNode
	{
		unsigned short id;

		template <class Archive>
		void serialize(Archive & ar)
		{
			ar(id);
		}
	};

	struct ClusterNode
	{
		unsigned short type;
		unsigned short actor;
		unsigned short nodeid;
		unsigned short targetid;

		template <class Archive>
		void serialize(Archive & ar)
		{
			ar(type, actor, nodeid, targetid);
		}
	};

	struct ArcNode
	{
		int id;
		unsigned short type;
		unsigned short nodeid;
		float weight;

		template <class Archive>
		void serialize(Archive & ar)
		{
			ar(id, type, nodeid, weight);
		}
	};

	struct TransitionNode
	{
		int id;
		unsigned short type;
		std::vector<float> weights;
		std::vector<unsigned short> nodes;
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
		unsigned short id;
		unsigned short actorid;
		unsigned short clusterid;
		float tolerance;
		Vec3 position;
		std::vector<ArcNode> arcs;
		std::vector<ClusterNode> clusters;
		std::vector<TransitionNode> transitions;
		std::vector<VisibleNode> visibles;

		template <class Archive>
		void save(Archive & ar) const
		{
			ar(id, actorid, clusterid, tolerance, position, arcs, clusters, transitions, visibles);
		}

		template <class Archive>
		void load(Archive & ar)
		{
			ar(id, actorid, clusterid, tolerance, position, arcs, clusters, transitions, visibles);
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
}

QuakeAIManager::QuakeAIManager() : AIManager()
{
	mLastArcId = 0;
	mLastNodeId = 0;

	mYaw = 0.0f;
	mPitchTarget = 0.0f;

	mMaxJumpSpeed = 3.4f;
	mMaxFallSpeed = 240.0f;
	mMaxRotateSpeed = 180.0f;
	mMoveSpeed = 6.0f;
	mJumpSpeed = 2.6f;
	mJumpMoveSpeed = 7.2f;
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
	//set data
	CerealTypes::Graph data;

	for (PathingNode* pathNode : mPathingGraph->GetNodes())
	{
		CerealTypes::GraphNode node;

		node.id = pathNode->GetId();
		node.actorid = pathNode->GetActorId();
		node.clusterid = pathNode->GetCluster();
		node.tolerance = pathNode->GetTolerance();
		node.position.x = (short)round(pathNode->GetPos()[0]);
		node.position.y = (short)round(pathNode->GetPos()[1]);
		node.position.z = (short)round(pathNode->GetPos()[2]);

		eastl::map<PathingNode*, float> visibilityNodes;
		pathNode->GetVisibileNodes(visibilityNodes);
		for (auto visibilityNode : visibilityNodes)
		{
			CerealTypes::VisibleNode visibleNode;
			visibleNode.id = visibilityNode.first->GetId();

			node.visibles.push_back(visibleNode);
		}

		for (PathingArc* pathArc : pathNode->GetArcs())
		{
			CerealTypes::ArcNode arcNode;
			arcNode.id = pathArc->GetId();
			arcNode.type = pathArc->GetType();
			arcNode.nodeid = pathArc->GetNode()->GetId();
			arcNode.weight = pathArc->GetWeight();

			node.arcs.push_back(arcNode);
		}

		for (PathingCluster* pathCluster : pathNode->GetClusters())
		{
			CerealTypes::ClusterNode clusterNode;
			clusterNode.type = pathCluster->GetType();
			clusterNode.actor = pathCluster->GetActor();
			clusterNode.nodeid = pathCluster->GetNode()->GetId();
			clusterNode.targetid = pathCluster->GetTarget()->GetId();

			node.clusters.push_back(clusterNode);
		}

		for (PathingTransition* pathTransition : pathNode->GetTransitions())
		{
			CerealTypes::TransitionNode transitionNode;
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
					(short)round(connection[0]), (short)round(connection[1]), (short)round(connection[2]) });
			}

			node.transitions.push_back(transitionNode);
		}

		data.nodes.push_back(node);
	}

	std::ofstream os(path.c_str(), std::ios::binary);
	cereal::BinaryOutputArchive archive(os);
	archive(data);
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

	mLastArcId = 0;
	mLastNodeId = 0;
	mPathingGraph = eastl::make_shared<PathingGraph>();

	eastl::map<unsigned int, PathingNode*> pathingNodeGraph;
	for (CerealTypes::GraphNode node : data.nodes)
	{
		unsigned short clusterId = node.clusterid;
		unsigned short pathNodeId = node.id;
		ActorId actorId = node.actorid;
		float tolerance = node.tolerance;
		Vector3<float> position{ 
			(float)node.position.x, (float)node.position.y, (float)node.position.z };
		if (mLastNodeId < pathNodeId) mLastNodeId = pathNodeId;

		PathingNode* pathNode = new PathingNode(pathNodeId, actorId, position, tolerance);
		pathNode->SetCluster(clusterId);
		mPathingGraph->InsertNode(pathNode);

		pathingNodeGraph[pathNodeId] = pathNode;
	}

	for (CerealTypes::GraphNode node : data.nodes)
	{
		unsigned short pathNodeId = node.id;
		PathingNode* pathNode = pathingNodeGraph[pathNodeId];

		for (CerealTypes::VisibleNode visibleNode : node.visibles)
		{
			PathingNode* visibilityNode = pathingNodeGraph[visibleNode.id];
			pathNode->AddVisibleNode(
				visibilityNode, Length(visibilityNode->GetPos() - pathNode->GetPos()));
		}

		for (CerealTypes::ArcNode arc : node.arcs)
		{
			unsigned int arcId = arc.id;
			unsigned short arcType = arc.type;
			int arcNode = arc.nodeid;
			float weight = arc.weight;
			if (mLastArcId < arcId) mLastArcId = arcId;

			PathingArc* pathArc = new PathingArc(arcId, arcType, pathingNodeGraph[arcNode], weight);
			mPathingGraph->InsertArc(pathArc);

			pathNode->AddArc(pathArc);
		}

		for (CerealTypes::ClusterNode cluster : node.clusters)
		{
			int clusterType = cluster.type;
			int clusterActor = cluster.actor;
			int clusterNode = cluster.nodeid;
			int clusterTarget = cluster.targetid;

			PathingCluster* pathCluster = new PathingCluster(clusterType, clusterActor);
			pathCluster->LinkClusters(pathingNodeGraph[clusterNode], pathingNodeGraph[clusterTarget]);
			mPathingGraph->InsertCluster(pathCluster);

			pathNode->AddCluster(pathCluster);
		}

		for (CerealTypes::TransitionNode transition : node.transitions)
		{
			unsigned int transitionId = transition.id;
			unsigned short transitionType = transition.type;

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
}

ActorId QuakeAIManager::GetPlayerTarget(ActorId player)
{
	if (mPlayerTargets.find(player) != mPlayerTargets.end())
		return mPlayerTargets[player];
	else
		return INVALID_ACTOR_ID;
}

WeaponType QuakeAIManager::GetPlayerWeapon(ActorId player)
{
	if (mPlayerWeapons.find(player) != mPlayerWeapons.end())
		return mPlayerWeapons[player];
	else
		return WP_NONE;
}

void QuakeAIManager::GetPlayerPath(ActorId player, PathingArcVec& playerPath)
{
	if (mPlayerPaths.find(player) != mPlayerPaths.end())
		for (PathingArc* path : mPlayerPaths[player])
			playerPath.push_back(path);
}

PathingNode* QuakeAIManager::GetPlayerNode(ActorId player)
{
	if (mPlayerNodes.find(player) != mPlayerNodes.end())
		return mPlayerNodes[player];
	else
		return NULL;
}

void QuakeAIManager::SetPlayerTarget(ActorId player, ActorId playerTarget)
{
	mPlayerTargets[player] = playerTarget;
}

void QuakeAIManager::SetPlayerWeapon(ActorId player, WeaponType playerWeapon)
{
	mPlayerWeapons[player] = playerWeapon;
}

void QuakeAIManager::SetPlayerPath(ActorId player, PathingArcVec& playerPath)
{
	mPlayerPaths[player].clear();
	for (PathingArc* path : playerPath)
		mPlayerPaths[player].push_back(path);
}

void QuakeAIManager::SetPlayerNode(ActorId player, PathingNode* playerNode)
{
	mPlayerNodes[player] = playerNode;
}

//map generation via physics simulation
void QuakeAIManager::CreateMap(ActorId playerId)
{
	GameApplication* gameApp = (GameApplication*)Application::App;
	QuakeLogic* game = static_cast<QuakeLogic *>(GameLogic::Get());
	Level* level = game->GetLevelManager()->GetLevel(
		ToWideString(gameApp->mOption.mLevel.c_str()));

	mPlayerActor = eastl::dynamic_shared_pointer_cast<PlayerActor>(
		GameLogic::Get()->GetActor(playerId).lock());
	/*
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
	*/
	// we group the graph nodes in clusters
	CreateClusters();

	eastl::wstring levelPath = L"ai/quake/" + level->GetName() + L".bin";
	GameLogic::Get()->GetAIManager()->SavePathingGraph(
		FileSystem::Get()->GetPath(ToString(levelPath.c_str())));

	// we need to handle firing grenades separately since they cannot be simulated by raycasting 
	// as they describe different trajectories
	/*
	eastl::shared_ptr<Actor> pGameActor(
		GameLogic::Get()->CreateActor("actors/quake/effects/simulategrenadelauncherfire.xml", nullptr));
	eastl::shared_ptr<PhysicComponent> pPhysicalComponent =
		pGameActor->GetComponent<PhysicComponent>(PhysicComponent::Name).lock();
	if (pPhysicalComponent)
		pPhysicalComponent->SetIgnoreCollision(mPlayerActor->GetId(), true);
	for (PathingNode* pNode : mPathingGraph->GetNodes())
		SimulateGrenadeLauncherFire(pNode, pGameActor);
	EventManager::Get()->TriggerEvent(
		eastl::make_shared<EventDataRequestDestroyActor>(pGameActor->GetId()));
	*/
	mActorNodes.clear();

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
		for (PathingArc* pathArc : pathNode->GetArcs())
		{
			PathingTransition* pathTransition = pathNode->FindTransition(pathArc->GetId());
			if (pathTransition)
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
					pathArc->GetId(), pathArc->GetType(), nodes, nodeWeights, nodePositions));
			}
			else
			{
				pathNode->AddTransition(new PathingTransition(pathArc->GetId(), pathArc->GetType(), 
					{ pathArc->GetNode() }, { pathArc->GetWeight() }, { pathArc->GetNode()->GetPos() }));
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
		PathingNode* pNode = mOpenSet.front();
		SimulateMovement(pNode);

		// we have processed this node so remove it from the open set
		mClosedSet.push_back(pNode);
		mOpenSet.erase(mOpenSet.begin());
	}

	// process the item actors which we have met
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
				pNode->RemoveTransitions();
				pNode->RemoveArcs();

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
		PathingNode* pNode = mClosedSet.front();

		// if the node is a trigger we don't simulate jump on it
		if (mActorNodes.find(pNode) != mActorNodes.end())
		{
			eastl::shared_ptr<Actor> pGameActor(
				GameLogic::Get()->GetActor(mActorNodes[pNode]).lock());
			if (pGameActor->GetType() != "Trigger")
				SimulateJump(pNode);
		}
		else SimulateJump(pNode);

		// we have processed this node so remove it from the closed set
		mClosedSet.erase(mClosedSet.begin());
	}
}

void QuakeAIManager::CreateClusters()
{
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
	KMeans kmeans(200, iters);
	kmeans.Run(points);

	for (Point point : points)
	{
		PathingNode* pathNode = mPathingGraph->FindNode(point.GetId());
		pathNode->SetCluster(point.GetCluster());
	}

	eastl::vector<unsigned int> searchClusters;
	for (Clustering kCluster : kmeans.GetClusters())
		searchClusters.push_back(kCluster.GetId());

	for (PathingNode* pathNode : mPathingGraph->GetNodes())
	{
		ClusterPlanMap clusterPlans;

		//add cluster transitions with jumps and moves
		mPathingGraph->FindPlans(pathNode, searchClusters, clusterPlans);
		for (auto clusterPlan : clusterPlans)
		{
			PathingArcVec pathArcs = clusterPlan.second->GetArcs();
			if (pathArcs.size())
			{
				PathingNode* pathTarget = pathArcs.back()->GetNode();

				if (pathTarget != pathNode)
				{
					PathingNode* currentNode = pathNode;
					for (PathingArc* pArc : pathArcs)
					{
						bool addCluster = true;
						PathingClusterVec clusters;
						currentNode->GetClusters(GAT_JUMP, clusters);
						for (PathingCluster* cluster : clusters)
						{
							if (cluster->GetTarget() == pathTarget)
							{
								addCluster = false;
								break;
							}
						}

						if (addCluster)
						{
							PathingCluster* pathCluster =
								new PathingCluster(GAT_JUMP, pathTarget->GetActorId());
							pathCluster->LinkClusters(pArc->GetNode(), pathTarget);
							currentNode->AddCluster(pathCluster);
							mPathingGraph->InsertCluster(pathCluster);
						}
						else break;

						currentNode = pArc->GetNode();
					}
				}
			}
			delete clusterPlan.second;
		}
		clusterPlans.clear();

		//add cluster transitions only with moves
		mPathingGraph->FindPlans(pathNode, searchClusters, clusterPlans, GAT_JUMP);
		for (auto clusterPlan : clusterPlans)
		{
			PathingArcVec pathArcs = clusterPlan.second->GetArcs();
			if (pathArcs.size())
			{
				PathingNode* pathTarget = pathArcs.back()->GetNode();
				if (pathTarget != pathNode)
				{
					PathingNode* currentNode = pathNode;
					for (PathingArc* pArc : pathArcs)
					{
						bool addCluster = true;
						PathingClusterVec clusters;
						currentNode->GetClusters(GAT_MOVE, clusters);
						for (PathingCluster* cluster : clusters)
						{
							if (cluster->GetTarget() == pathTarget)
							{
								addCluster = false;
								break;
							}
						}

						if (addCluster)
						{
							PathingCluster* pathCluster =
								new PathingCluster(GAT_MOVE, pathTarget->GetActorId());
							pathCluster->LinkClusters(pArc->GetNode(), pathTarget);
							currentNode->AddCluster(pathCluster);
							mPathingGraph->InsertCluster(pathCluster);
						}
						else break;

						currentNode = pArc->GetNode();
					}
				}
			}

			delete clusterPlan.second;
		}
	}

	GameApplication* gameApp = (GameApplication*)Application::App;
	QuakeLogic* game = static_cast<QuakeLogic *>(GameLogic::Get());

	eastl::vector<eastl::shared_ptr<Actor>> searchActors;
	game->GetAmmoActors(searchActors);
	game->GetWeaponActors(searchActors);
	game->GetHealthActors(searchActors);
	game->GetArmorActors(searchActors);

	for (PathingNode* pathNode : mPathingGraph->GetNodes())
	{
		ActorPlanMap actorPlans;

		//add cluster transitions with jumps and moves
		mPathingGraph->FindPlans(pathNode, searchActors, actorPlans, -1, 1.0f);
		for (auto actorPlan : actorPlans)
		{
			PathingArcVec pathArcs = actorPlan.second->GetArcs();
			if (pathArcs.size())
			{
				PathingNode* pathTarget = pathArcs.back()->GetNode();
				if (pathTarget != pathNode)
				{
					PathingNode* currentNode = pathNode;
					for (PathingArc* pArc : pathArcs)
					{
						bool addCluster = true;
						PathingClusterVec clusters;
						currentNode->GetClusters(GAT_JUMP, clusters);
						for (PathingCluster* cluster : clusters)
						{
							if (cluster->GetTarget() == pathTarget)
							{
								addCluster = false;
								break;
							}
						}

						if (addCluster)
						{
							PathingCluster* pathCluster =
								new PathingCluster(GAT_JUMP, pathTarget->GetActorId());
							pathCluster->LinkClusters(pArc->GetNode(), pathTarget);
							currentNode->AddCluster(pathCluster);
							mPathingGraph->InsertCluster(pathCluster);
						}
						else break;

						currentNode = pArc->GetNode();
					}
				}
			}

			delete actorPlan.second;
		}

		actorPlans.clear();

		//add cluster transitions only with moves
		mPathingGraph->FindPlans(pathNode, searchActors, actorPlans, GAT_JUMP, 1.0f);
		for (auto actorPlan : actorPlans)
		{
			PathingArcVec pathArcs = actorPlan.second->GetArcs();
			if (pathArcs.size())
			{
				PathingNode* pathTarget = pathArcs.back()->GetNode();
				if (pathTarget != pathNode)
				{
					PathingNode* currentNode = pathNode;
					for (PathingArc* pArc : pathArcs)
					{
						bool addCluster = true;
						PathingClusterVec clusters;
						currentNode->GetClusters(GAT_MOVE, clusters);
						for (PathingCluster* cluster : clusters)
						{
							if (cluster->GetTarget() == pathTarget)
							{
								addCluster = false;
								break;
							}
						}

						if (addCluster)
						{
							PathingCluster* pathCluster =
								new PathingCluster(GAT_MOVE, pathTarget->GetActorId());
							pathCluster->LinkClusters(pArc->GetNode(), pathTarget);
							currentNode->AddCluster(pathCluster);
							mPathingGraph->InsertCluster(pathCluster);
						}
						else break;

						currentNode = pArc->GetNode();
					}
				}
			}

			delete actorPlan.second;
		}
		actorPlans.clear();
	}

	for (PathingNode* pathNode : mPathingGraph->GetNodes())
		pathNode->OrderClusters();
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
	mOpenSet.push_back(pNewNode);
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
			if (pEndNode->GetActorId() == INVALID_ACTOR_ID ||
				pNode->GetActorId() != pEndNode->GetActorId())
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
			if (pEndNode->GetActorId() == INVALID_ACTOR_ID ||
				pNode->GetActorId() != pEndNode->GetActorId())
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
								mOpenSet.push_back(pNewNode);
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
						{
							pClosestNode->SetActorId(pItemActor->GetId());
							mActorNodes[pClosestNode] = pItemActor->GetId();
						}
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