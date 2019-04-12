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

class ClusteringTransition;
class ClusteringNode;
class ClusteringArc;

typedef eastl::vector<Cluster*> ClusterVec;
typedef eastl::vector<ClusterArc*> ClusterArcVec;
typedef eastl::vector<ClusteringArc*> ClusteringArcVec;
typedef eastl::vector<ClusteringNode*> ClusteringNodeVec;
typedef eastl::list<ClusterPlanNode*> ClusterPlanNodeList;
typedef eastl::map<Cluster*, ClusterPlan*> ClusterPlanMap;
typedef eastl::vector<ClusteringTransition*> ClusteringTransitionVec;
typedef eastl::map<ClusteringNode*, ClusteringNodeVec> ClusteringNodeMap;
typedef eastl::map<ClusteringNode*, ClusteringArcVec> ClusteringNodeArcMap;
typedef eastl::map<ClusteringArc*, ClusteringNodeVec> ClusteringArcNodeMap;
typedef eastl::map<ClusteringNode*, eastl::map<ClusteringNode*, float>> ClusteringNodeDoubleMap;
typedef eastl::map<ClusteringNode*, ClusterPlanNode*> ClusteringNodeToClusterPlanNodeMap;

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
	ClusteringNodeVec mIsolatedNodes;  // nodes without connection in the cluster

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
	void RemoveArc(unsigned int id);
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
	bool IsIsolatedNode(ClusteringNode* pNode);
	void InsertIsolatedNode(ClusteringNode* pNode)
	{
		LogAssert(pNode, "Invalid node");

		mIsolatedNodes.push_back(pNode);
	}
	void InsertNode(ClusteringNode* pNode)
	{
		LogAssert(pNode, "Invalid node");

		mNodes.push_back(pNode);
	}
	const ClusteringNodeVec& GetNodes() { return mNodes; }
	const ClusteringNodeVec& GetIsolatedNodes() { return mIsolatedNodes; }
};

//--------------------------------------------------------------------------------------------------------
// class ClusterArc
// This class represents an arc that links two cluster, allowing travel between them.
//--------------------------------------------------------------------------------------------------------
class ClusterArc
{
	unsigned int mId;
	unsigned int mType;

	Cluster* mCluster[2];  // cluster which is linked to

public:
	explicit ClusterArc(unsigned int id, unsigned int type)
		: mId(id), mType(type)
	{
	}

	unsigned int GetId(void) const { return mId; }
	unsigned int GetType(void) const { return mType; }
	Cluster* GetCluster() const { return mCluster[0]; }
	Cluster* GetTarget() const { return mCluster[1]; }

	void LinkClusters(Cluster* pStart, Cluster* pEnd)
	{
		LogAssert(pStart, "Invalid cluster");
		LogAssert(pEnd, "Invalid cluster");

		mCluster[0] = pStart;
		mCluster[1] = pEnd;
	}
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
	ClusteringTransitionVec mTransitions;

	eastl::map<ClusteringNode*, float> mVisibleNodes;

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

	void AddVisibleNode(ClusteringNode* pNode, float value);
	void GetVisibileNodes(eastl::map<ClusteringNode*, float>& visibilities);
	float FindVisibleNode(ClusteringNode* pNode);
	bool IsVisibleNode(ClusteringNode* pNode);

	void AddArc(ClusteringArc* pArc);
	ClusteringArc* FindArc(unsigned int id);
	ClusteringArc* FindArc(Cluster* pLinkedCluster);
	ClusteringArc* FindArc(unsigned int arcType, Cluster* pLinkedCluster);
	ClusteringArc* FindArc(ClusteringNode* pLinkedNode);
	ClusteringArc* FindArc(unsigned int arcType, ClusteringNode* pLinkedNode);
	const ClusteringArcVec& GetArcs() { return mArcs; }
	void RemoveArc(unsigned int id);
	void RemoveArcs();

	void GetNeighbors(unsigned int arcType, ClusteringArcVec& outNeighbors);

	void AddTransition(ClusteringTransition* pTransition);
	ClusteringTransition* FindTransition(unsigned int id);
	const ClusteringTransitionVec& GetTransitions() { return mTransitions; }
	void RemoveTransition(unsigned int id);
	void RemoveTransitions();
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

	ClusteringNode* mNode;  // node which is linked to

public:
	explicit ClusteringArc(unsigned int id, unsigned int type, ClusteringNode* pNode, float weight = 0.f)
		: mId(id), mType(type), mNode(pNode), mWeight(weight)
	{ 

	}

	unsigned int GetId(void) const { return mId; }
	unsigned int GetType(void) const { return mType; }
	float GetWeight(void) const { return mWeight; }
	ClusteringNode* GetNode() const { return mNode; }
};


//--------------------------------------------------------------------------------------------------------
// class ClusteringTransition
// This class represents the transitions which an arc may do.
//--------------------------------------------------------------------------------------------------------
class ClusteringTransition
{
	unsigned int mId;
	unsigned int mType;

	eastl::vector<float> mWeights;
	eastl::vector<ClusteringNode*> mNodes;  // transition nodes
	eastl::vector<Vector3<float>> mConnections; // transition interpolation

public:
	explicit ClusteringTransition(
		unsigned int id, unsigned int type, const eastl::vector<ClusteringNode*>& nodes,
		const eastl::vector<float>& weights, const eastl::vector<Vector3<float>>& connections)
		: mId(id), mType(type), mNodes(nodes), mWeights(weights), mConnections(connections)
	{

	}

	unsigned int GetId(void) const { return mId; }
	unsigned int GetType(void) const { return mType; }

	const eastl::vector<ClusteringNode*>& GetNodes(void) const { return mNodes; }
	const eastl::vector<float>& GetWeights(void) const { return mWeights; }
	const eastl::vector<Vector3<float>>& GetConnections(void) const { return mConnections; }
};


//--------------------------------------------------------------------------------------------------------
// class ClusterPlan
// This class represents a complete path and is used by the higher-level AI to determine where to go.
//--------------------------------------------------------------------------------------------------------
class ClusterPlan
{
	friend class ClusterFinder;

	ClusteringArcVec mPath;
	ClusteringArcVec::iterator mIndex;

public:
	ClusterPlan(void)
	{
		mIndex = mPath.end();
	}

	const ClusteringArcVec& GetArcs(void) const { return mPath; }

	void ResetPath(void)
	{
		mIndex = mPath.begin();
	}

	ClusteringArc* GetCurrentArc(void) const
	{
		LogAssert(mIndex != mPath.end(), "Invalid index");
		return (*mIndex);
	}

	bool CheckForNextNode(const Vector3<float>& pos);
	bool CheckForEnd(void);

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

	void operator()(ClusteringNode* pStartNode, ClusterVec& searchClusters, ClusterPlanMap& clusters, int skipArc = -1);
	ClusterPlan* operator()(ClusteringNode* pStartNode, ClusteringNode* pGoalNode);
	ClusterPlan* operator()(ClusteringNode* pStartNode, Cluster* pGoalCluster);

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
	ClusterArcVec mArcs;
	
public:
	ClusteringGraph(void) {}
	~ClusteringGraph(void) { DestroyGraph(); }
	void DestroyGraph(void);

	void FindClusters(ClusterVec&, const Vector3<float>& pos, float radius);
	ClusteringNode* FindClosestNode(const Vector3<float>& pos);
	ClusteringNode* FindFurthestNode(const Vector3<float>& pos);
	Cluster* FindCluster(unsigned int clusterId);
	Cluster* FindRandomCluster(void);

	void FindClusters(ClusteringNode* pStartNode, ClusterVec& searchClusters, ClusterPlanMap& clusters, int skipArc = -1);
	ClusterPlan* FindNode(const Vector3<float>& startPoint, const Vector3<float>& endPoint);
	ClusterPlan* FindNode(const Vector3<float>& startPoint, Cluster* pGoalCluster);
	ClusterPlan* FindNode(ClusteringNode* pStartNode, ClusteringNode* pGoalNode);
	ClusterPlan* FindNode(ClusteringNode* pStartNode, Cluster* pGoalCluster);

	// helpers
	void InsertCluster(Cluster* pCluster);
	void InsertArc(ClusterArc* pArc);
	const ClusterVec& GetClusters() { return mClusters; }
	const ClusterArcVec& GetArcs() { return mArcs; }
};


#endif