//========================================================================
// Clustering.h : Implements a simple clustering system using A*
//
// Part of the GameEngine Application
//
// GameEngine is the sample application that encapsulates much of the source code
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
//    http://code.google.com/p/GameEngine/
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

#ifndef CLUSTERING_H
#define CLUSTERING_H

#include "GameEngineStd.h"

#include "Core/Logger/Logger.h"
#include "Mathematic/Algebra/Vector3.h"

class Cluster;
class ClusterPlan;
class ClusterPlanNode;
class ClusterFinder;
class ClusterArc; 

class ClusteringNode;
class ClusteringArc;

typedef eastl::vector<Cluster*> ClusterVec;
typedef eastl::vector<ClusterArc*> ClusterArcVec;
typedef eastl::vector<ClusteringArc*> ClusteringArcVec;
typedef eastl::vector<ClusteringNode*> ClusteringNodeVec;
typedef eastl::list<ClusterPlanNode*> ClusterPlanNodeList;
typedef eastl::map<ClusteringNode*, ClusteringNodeVec> ClusteringNodeMap;
typedef eastl::map<ClusteringArc*, ClusteringNodeVec> ClusteringArcNodeMap;
typedef eastl::map<ClusteringNode*, ClusterPlanNode*> ClusteringNodeToClusterPlanNodeMap;
typedef eastl::map<ClusteringArc*, eastl::map<ClusteringArc*, float>> ClusteringArcDoubleMap;
typedef eastl::map<ClusteringNode*, eastl::map<ClusteringNode*, float>> ClusteringNodeDoubleMap;
typedef eastl::map<ClusteringNode*, eastl::map<ClusteringArc*, float>> ClusteringNodeArcDoubleMap;
typedef eastl::map<ClusteringArc*, eastl::map<ClusteringNode*, float>> ClusteringArcNodeDoubleMap;
typedef eastl::map<ClusteringNode*, eastl::map<ClusteringNode*, Vector3<float>>> ClusteringNodeVecMap;
typedef eastl::map<ClusteringNode*, eastl::map<ClusteringArc*, Vector3<float>>> ClusteringNodeArcVecMap;

const float CLUSTERING_DEFAULT_NODE_TOLERANCE = 4.0f;
const float CLUSTERING_DEFAULT_ARC_WEIGHT = 0.001f;

//--------------------------------------------------------------------------------------------------------
// class Cluster
// This class represents a single cluster.
//
//--------------------------------------------------------------------------------------------------------
class Cluster
{
	unsigned int mId;
	ClusteringNode* mCenter;

	ClusterArcVec mArcs;  // master list of all arcs
	ClusteringNodeVec mNodes;  // master list of all nodes

	eastl::vector<ActorId> mActors; //actors within the cluster

public:
	explicit Cluster(unsigned int id)
		: mId(id)
	{ }
	~Cluster(void) { DestroyCluster(); }
	void DestroyCluster(void);

	unsigned int GetId(void) const { return mId; }
	ClusteringNode* GetCenter(void) const { return mCenter; }
	void SetCenter(ClusteringNode* center) { mCenter = center; }

	void AddActor(ActorId actorId) 
	{ 
		if (eastl::find(mActors.begin(), mActors.end(), actorId) == mActors.end())
			mActors.push_back(actorId); 
	}
	const eastl::vector<ActorId>& GetActors(void) const { return mActors; }
	void RemoveActors() { mActors.clear(); }

	void AddArc(ClusterArc* pArc);
	ClusterArc* FindArc(Cluster* pLinkedCluster);
	ClusterArc* FindArc(unsigned int arcType, Cluster* pLinkedCluster);
	const ClusterArcVec& GetArcs() { return mArcs; }
	void RemoveArcs();

	void GetNeighbors(unsigned int arcType, ClusterArcVec& outNeighbors);

	void FindNodes(ClusteringNodeVec&, const Vector3<float>& pos, float radius);
	ClusteringNode* FindClosestNode(const Vector3<float>& pos);
	ClusteringNode* FindFurthestNode(const Vector3<float>& pos);
	ClusteringNode* FindNode(unsigned int nodeId);
	ClusteringNode* FindRandomNode(void);

	ClusterPlan* FindNode(const Vector3<float>& startPoint, const Vector3<float>& endPoint);
	ClusterPlan* FindNode(const Vector3<float>& startPoint, ClusteringNode* pGoalNode);
	ClusterPlan* FindNode(ClusteringNode* pStartNode, const Vector3<float>& endPoint);
	ClusterPlan* FindNode(ClusteringNode* pStartNode, ClusteringNode* pGoalNode);
	ClusterPlan* FindNode(ClusteringNode* pStartNode, Cluster* pGoalCluster);

	// helpers
	void InsertNode(ClusteringNode* pNode);
	const ClusteringNodeVec& GetNodes() { return mNodes; }
};

//--------------------------------------------------------------------------------------------------------
// class ClusterArc
// This class represents an arc that links two cluster, allowing travel between them.
//--------------------------------------------------------------------------------------------------------
class ClusterArc
{
	unsigned int mId;
	unsigned int mType;
	float mWeight;

	Cluster* mClusters[2];  // an arc always connects two nodes

public:
	explicit ClusterArc(unsigned int id, unsigned int type, float weight = 0.f)
		: mId(id), mType(type), mWeight(weight)
	{

	}

	unsigned int GetId(void) const { return mId; }
	unsigned int GetType(void) const { return mType; }
	float GetWeight(void) const { return mWeight; }

	void LinkClusters(Cluster* pClusterA, Cluster* pClusterB);
	Cluster* GetNeighbor(Cluster* pMe);
	Cluster* GetNeighbor() { return mClusters[1]; }
	Cluster* GetOrigin() { return mClusters[0]; }
};


//--------------------------------------------------------------------------------------------------------
// class ClusteringNode
// This class represents a single node in the clustering graph.
//
//--------------------------------------------------------------------------------------------------------
class ClusteringNode
{
	unsigned int mId;
	Cluster* mCluster;

	Vector3<float> mPos;
	ClusteringArcVec mArcs;

	ActorId mActor;

public:
	explicit ClusteringNode(unsigned int id, const Vector3<float>& pos)
		: mId(id), mCluster(NULL), mActor(INVALID_ACTOR_ID), mPos(pos)
	{ }

	unsigned int GetId(void) const { return mId; }
	void SetCluster(Cluster* cluster) { mCluster = cluster; }
	Cluster* GetCluster(void) const { return mCluster; }
	void SetActor(ActorId actorId) { mActor = actorId; }
	ActorId GetActor(void) const { return mActor; }

	const Vector3<float>& GetPos(void) const { return mPos; }

	void AddArc(ClusteringArc* pArc);
	ClusteringArc* FindArc(ClusteringNode* pLinkedNode);
	ClusteringArc* FindArc(unsigned int arcType, ClusteringNode* pLinkedNode);
	const ClusteringArcVec& GetArcs() { return mArcs; }
	void RemoveArcs();

	void GetNeighbors(unsigned int arcType, ClusteringArcVec& outNeighbors);
};


//--------------------------------------------------------------------------------------------------------
// class ClusteringArc
// This class represents an arc that links two nodes, allowing travel between them.
//--------------------------------------------------------------------------------------------------------
class ClusteringArc
{
	unsigned int mId;
	unsigned int mType;
	float mWeight;
	Vector3<float> mConnection; //an optional interpolation vector which connects nodes 
	ClusteringNode* mNodes[2];  // an arc always connects two nodes

public:
	explicit ClusteringArc(unsigned int id, unsigned int type, 
		float weight = 0.f, const Vector3<float>& connect = NULL) 
		: mId(id), mType(type), mWeight(weight), mConnection(connect)
	{ 

	}

	unsigned int GetId(void) const { return mId; }
	unsigned int GetType(void) const { return mType; }
	float GetWeight(void) const { return mWeight; }
	const Vector3<float>& GetConnection(void) const { return mConnection; }

	void LinkNodes(ClusteringNode* pNodeA, ClusteringNode* pNodeB);
	ClusteringNode* GetNeighbor(ClusteringNode* pMe);
	ClusteringNode* GetNeighbor() { return mNodes[1]; }
	ClusteringNode* GetOrigin() { return mNodes[0]; }
};


//--------------------------------------------------------------------------------------------------------
// class ClusterPlan
// This class represents a complete path and is used by the higher-level AI to determine where to go.
//--------------------------------------------------------------------------------------------------------
class ClusterPlan
{
	friend class ClusterFinder;

	ClusteringArcVec mPath;

public:

	ClusterPlan(void) { }

	const ClusteringArcVec& GetArcs(void) const { return mPath; }

private:
	void AddArc(ClusteringArc* pArc);
};


//--------------------------------------------------------------------------------------------------------
// class ClusterPlanNode
// This class is a helper used in ClusteringGraph::FindCluster().
//--------------------------------------------------------------------------------------------------------
class ClusterPlanNode
{
	ClusterPlanNode* mPrevNode;  // node we just came from
	ClusteringArc* mClusteringArc;  // pointer to the clustering arc from the clustering graph
	ClusteringNode* mClusteringNode;  // pointer to the clustering node from the clustering graph
	bool mClosed;  // the node is closed if it's already been processed
	float mGoal;  // cost of the entire cluster up to this point (often called g)
	
public:
	explicit ClusterPlanNode(ClusteringArc* pArc, ClusterPlanNode* pPrevNode);
	explicit ClusterPlanNode(ClusteringNode* pNode, ClusterPlanNode* pPrevNode);
	ClusterPlanNode* GetPrev(void) const { return mPrevNode; }
	ClusteringArc* GetClusteringArc(void) const { return mClusteringArc; }
	ClusteringNode* GetClusteringNode(void) const { return mClusteringNode; }
	bool IsClosed(void) const { return mClosed; }
	float GetGoal(void) const { return mGoal; }
	
	void UpdatePrevNode(ClusterPlanNode* pPrev);
	void SetClosed(bool toClose = true) { mClosed = toClose; }
	bool IsBetterChoiceThan(ClusterPlanNode* pRight) { return (mGoal < pRight->GetGoal()); }
	
private:
	void UpdateClusterCost(void);
};


//--------------------------------------------------------------------------------------------------------
// class ClusterFinder
// This class implements the ClusterFinder algorithm.
//--------------------------------------------------------------------------------------------------------
class ClusterFinder
{
	ClusteringNodeToClusterPlanNodeMap mNodes;
	ClusteringNode* mStartNode;
	ClusteringNode* mGoalNode;
	ClusterPlanNodeList mOpenSet;

public:
	ClusterFinder(void);
	~ClusterFinder(void);
	void Destroy(void);

	ClusterPlan* operator()(ClusteringNode* pStartNode, ClusteringNode* pGoalNode, bool searchInNodes = false);
	ClusterPlan* operator()(ClusteringNode* pStartNode, Cluster* pGoalCluster, bool searchInNodes = false);

private:
	ClusterPlanNode * AddToOpenSet(ClusteringArc* pArc, ClusterPlanNode* pPrevNode);
	ClusterPlanNode* AddToOpenSet(ClusteringNode* pNode, ClusterPlanNode* pPrevNode);
	void AddToClosedSet(ClusterPlanNode* pNode);
	void InsertNode(ClusterPlanNode* pNode);
	void ReinsertNode(ClusterPlanNode* pNode);
	ClusterPlan* RebuildPath(ClusterPlanNode* pGoalNode);
};

//--------------------------------------------------------------------------------------------------------
// class ClusteringGraph
// This class is the main interface into the clustering system. It holds the clustering graph itself and owns
// all the ClusteringNode and Clustering Arc objects.
//--------------------------------------------------------------------------------------------------------
class ClusteringGraph
{
	ClusterVec mClusters;
	
public:
	ClusteringGraph(void) {}
	~ClusteringGraph(void) { DestroyGraph(); }
	void DestroyGraph(void);

	void FindClusters(ClusterVec&, const Vector3<float>& pos, float radius);
	ClusteringNode* FindClosestNode(const Vector3<float>& pos);
	ClusteringNode* FindFurthestNode(const Vector3<float>& pos);
	Cluster* FindCluster(unsigned int clusterId);
	Cluster* FindRandomCluster(void);

	ClusterPlan* FindCluster(const Vector3<float>& startPoint, const Vector3<float>& endPoint);
	ClusterPlan* FindCluster(const Vector3<float>& startPoint, Cluster* pGoalCluster);
	ClusterPlan* FindCluster(ClusteringNode* pStartNode, ClusteringNode* pGoalNode);
	ClusterPlan* FindCluster(ClusteringNode* pStartNode, Cluster* pGoalCluster);

	// helpers
	void InsertCluster(Cluster* pCluster);
	const ClusterVec& GetClusters() { return mClusters; }
};


#endif