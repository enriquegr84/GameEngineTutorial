//========================================================================
// Pathing.h : Implements a simple pathing system using A*
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

#ifndef PATHING_H
#define PATHING_H

#include "GameEngineStd.h"

#include "Core/Logger/Logger.h"
#include "Mathematic/Algebra/Vector3.h"

class PathingTransition;
class PathingCluster;
class PathingNode;
class PathingArc;

class PathPlanNode;
class PathFinder;
class PathPlan;

typedef eastl::vector<PathingArc*> PathingArcVec;
typedef eastl::vector<PathingNode*> PathingNodeVec;
typedef eastl::vector<PathingCluster*> PathingClusterVec;
typedef eastl::vector<PathingTransition*> PathingTransitionVec;

typedef eastl::list<PathPlanNode*> PathPlanNodeList;
typedef eastl::map<PathingNode*, PathPlan*> PathPlanMap;
typedef eastl::map<PathingNode*, PathingNodeVec> PathingNodeMap;
typedef eastl::map<PathingArc*, PathingNodeVec> PathingArcNodeMap;
typedef eastl::map<PathingCluster*, PathingNodeVec> PathingClusterNodeMap;
typedef eastl::map<PathingNode*, PathPlanNode*> PathingNodeToPathPlanNodeMap;

const float PATHING_DEFAULT_NODE_TOLERANCE = 4.0f;
const float PATHING_DEFAULT_ARC_WEIGHT = 0.001f;

//--------------------------------------------------------------------------------------------------------
// class PathingNode				- Chapter 18, page 636
// This class represents a single node in the pathing graph.
//
//--------------------------------------------------------------------------------------------------------
class PathingNode
{
	unsigned int mId;
	Vector3<float> mPos;
	PathingArcVec mArcs;
	PathingClusterVec mClusters;
	PathingTransitionVec mTransitions;

	eastl::map<PathingNode*, float> mVisibleNodes;

	float mTolerance;
	ActorId mActorId;

public:
	explicit PathingNode(unsigned int id, ActorId actorId, 
		const Vector3<float>& pos, float tolerance = PATHING_DEFAULT_NODE_TOLERANCE)
		: mId(id), mActorId(actorId), mPos(pos), mTolerance(tolerance)
	{ }

	unsigned int GetId(void) const { return mId; }
	void SetActorId(ActorId actorId) { mActorId = actorId; }
	ActorId GetActorId(void) const { return mActorId; }
	float GetTolerance(void) const { return mTolerance; }
	const Vector3<float>& GetPos(void) const { return mPos; }

	void AddVisibleNode(PathingNode* pNode, float value);
	void GetVisibileNodes(eastl::map<PathingNode*, float>& visibilities);
	float FindVisibleNode(PathingNode* pNode);
	bool IsVisibleNode(PathingNode* pNode);

	void AddArc(PathingArc* pArc);
	PathingArc* FindArc(PathingNode* pLinkedNode);
	PathingArc* FindArc(unsigned int arcType, PathingNode* pLinkedNode);
	const PathingArcVec& GetArcs() { return mArcs; }
	void GetArcs(unsigned int arcType, PathingArcVec& outArcs);
	void RemoveArc(unsigned int id);
	void RemoveArcs();

	void AddCluster(PathingCluster* pCluster);
	PathingCluster* FindCluster(PathingNode* pTargetNode);
	PathingCluster* FindCluster(unsigned int clusterType, PathingNode* pTargetNode);
	const PathingClusterVec& GetClusters() { return mClusters; }
	void GetClusters(unsigned int arcType, PathingClusterVec& outClusters);
	void RemoveCluster(unsigned int arcType);
	void RemoveClusters();

	void AddTransition(PathingTransition* pTransition);
	const PathingTransitionVec& GetTransitions() { return mTransitions; }
	void GetTransitions(unsigned int id, PathingTransitionVec& transitions);
	void RemoveTransitions(unsigned int id);
	void RemoveTransitions();

};


//--------------------------------------------------------------------------------------------------------
// class PathingArc				- Chapter 18, page 636
// This class represents an arc that links two nodes, allowing travel between them.
//--------------------------------------------------------------------------------------------------------
class PathingArc
{
	unsigned int mId;
	unsigned int mType;
	float mWeight;

	PathingNode* mNode;  // node which is linked to

public:
	explicit PathingArc(unsigned int id, unsigned int type, PathingNode* pNode, float weight = 0.f) 
		: mId(id), mType(type), mNode(pNode), mWeight(weight)
	{ 

	}

	unsigned int GetId(void) const { return mId; }
	unsigned int GetType(void) const { return mType; }
	float GetWeight(void) const { return mWeight; }
	PathingNode* GetNode() const { return mNode; }
};

//--------------------------------------------------------------------------------------------------------
// class PathingCluster
// This class represents a transition that links two clusters.
//--------------------------------------------------------------------------------------------------------
class PathingCluster
{
	unsigned int mType;
	ActorId mActor;

	PathingNode* mNode;  // node transition
	PathingNode* mTarget;  // node target

public:
	explicit PathingCluster(unsigned int type, ActorId actor = INVALID_ACTOR_ID)
		: mType(type), mActor(actor)
	{

	}

	unsigned int GetType(void) const { return mType; }
	ActorId GetActor(void) const { return mActor; }
	PathingNode* GetNode() const { return mNode; }
	PathingNode* GetTarget() const { return mTarget; }

	void LinkClusters(PathingNode* pNode, PathingNode* pTarget);
};

//--------------------------------------------------------------------------------------------------------
// class PathingTransition
// This class represents the transitions which an arc may do.
//--------------------------------------------------------------------------------------------------------
class PathingTransition
{
	unsigned int mId;
	unsigned int mType;

	eastl::vector<float> mWeights;
	eastl::vector<PathingNode*> mNodes;  // transition nodes
	eastl::vector<Vector3<float>> mConnections; // transition interpolation

public:
	explicit PathingTransition(
		unsigned int id, unsigned int type, const eastl::vector<PathingNode*>& nodes, 
		const eastl::vector<float>& weights, const eastl::vector<Vector3<float>>& connections)
		: mId(id), mType(type), mNodes(nodes), mWeights(weights), mConnections(connections)
	{

	}

	unsigned int GetId(void) const { return mId; }
	unsigned int GetType(void) const { return mType; }

	const eastl::vector<PathingNode*>& GetNodes(void) const { return mNodes; }
	const eastl::vector<float>& GetWeights(void) const { return mWeights; }
	const eastl::vector<Vector3<float>>& GetConnections(void) const { return mConnections; }
};


//--------------------------------------------------------------------------------------------------------
// class PathingPlan				- Chapter 18, page 636
// This class represents a complete path and is used by the higher-level AI to determine where to go.
//--------------------------------------------------------------------------------------------------------
class PathPlan
{
	friend class PathFinder;

	PathingArcVec mPath;
	PathingArcVec::iterator mIndex;
	Vector3<float> mCurrentDirection;

public:
	PathPlan(void) 
	{ 
		mIndex = mPath.end();
		mCurrentDirection = Vector3<float>::Zero(); 
	}
	
	const PathingArcVec& GetArcs(void) const { return mPath; }

	void ResetPath(void) 
	{ 
		mIndex = mPath.begin(); 
		mCurrentDirection = Vector3<float>::Zero();
	}

	PathingArc* GetCurrentArc(void) const
	{
		LogAssert(mIndex != mPath.end(), "Invalid index");
		return (*mIndex);
	}

	bool CheckForNextNode(const Vector3<float>& pos);
	bool CheckForEnd(void);
	
private:
	void AddArc(PathingArc* pArc);
};


//--------------------------------------------------------------------------------------------------------
// class PathPlanNode						- Chapter 18, page 636
// This class is a helper used in PathingGraph::FindPath().
//--------------------------------------------------------------------------------------------------------
class PathPlanNode
{
	PathPlanNode* mPrevNode;  // node we just came from
	PathingArc* mPathingArc;  // pointer to the pathing arc from the pathing graph
	PathingNode* mPathingNode;  // pointer to the pathing node from the pathing graph
	PathingNode* mGoalNode;  // pointer to the goal node
	bool mClosed;  // the node is closed if it's already been processed
	float mGoal;  // cost of the entire path up to this point (often called g)
	
public:
	explicit PathPlanNode(PathingArc* pArc, PathPlanNode* pPrevNode, PathingNode* pGoalNode);
	explicit PathPlanNode(PathingNode* pNode, PathPlanNode* pPrevNode, PathingNode* pGoalNode);
	PathPlanNode* GetPrev(void) const { return mPrevNode; }
	PathingArc* GetPathingArc(void) const { return mPathingArc; }
	PathingNode* GetPathingNode(void) const { return mPathingNode; }
	bool IsClosed(void) const { return mClosed; }
	float GetGoal(void) const { return mGoal; }
	
	void UpdatePrevNode(PathPlanNode* pPrev);
	void SetClosed(bool toClose = true) { mClosed = toClose; }
	bool IsBetterChoiceThan(PathPlanNode* pRight) { return (mGoal < pRight->GetGoal()); }
	
private:
	void UpdatePathCost(void);
};


//--------------------------------------------------------------------------------------------------------
// class PathFinder								- Chapter 18, page 638
// This class implements the PathFinder algorithm.
//--------------------------------------------------------------------------------------------------------
class PathFinder
{
	PathingNodeToPathPlanNodeMap mNodes;
	PathingNode* mStartNode;
	PathingNode* mGoalNode;
	PathPlanNodeList mOpenSet;
	
public:
	PathFinder(void);
	~PathFinder(void);
	void Destroy(void);
	
	PathPlan* operator()(PathingNode* pStartNode, PathingNode* pGoalNode);
	PathingNode* operator()(
		PathingNode* pStartNode, PathingNodeVec& searchNodes, float threshold = FLT_MAX);
	void operator()(PathingNode* pStartNode, PathingNodeVec& searchNodes, 
		PathPlanMap& plans, int skipArc = -1, float threshold = FLT_MAX);

private:
	PathPlanNode* AddToOpenSet(PathingArc* pArc, PathPlanNode* pPrevNode);
	PathPlanNode* AddToOpenSet(PathingNode* pNode, PathPlanNode* pPrevNode);
	void AddToClosedSet(PathPlanNode* pNode);
	void InsertNode(PathPlanNode* pNode);
	void ReinsertNode(PathPlanNode* pNode);
	PathPlan* RebuildPath(PathPlanNode* pGoalNode);
};


//--------------------------------------------------------------------------------------------------------
// class PathingGraph					- Chapter 18, 636
// This class is the main interface into the pathing system.  It holds the pathing graph itself and owns
// all the PathingNode and Pathing Arc objects.
//--------------------------------------------------------------------------------------------------------
class PathingGraph
{
	PathingClusterVec mClusters; // master list of all clusters
	PathingNodeVec mNodes;  // master list of all nodes
	PathingArcVec mArcs;  // master list of all arcs
	
public:
	PathingGraph(void) {}
	~PathingGraph(void) { DestroyGraph(); }
	void DestroyGraph(void);

	void FindNodes(PathingNodeVec&, const Vector3<float>& pos, float radius);
	PathingNode* FindClosestNode(const Vector3<float>& pos);
	PathingNode* FindFurthestNode(const Vector3<float>& pos);
	PathingNode* FindNode(unsigned int nodeId);
	PathingNode* FindRandomNode(void);

	PathingNode* FindNodes(
		PathingNode* pStartNode, PathingNodeVec& searchNodes, float threshold = FLT_MAX);
	void FindPlans(PathingNode* pStartNode, PathingNodeVec& searchNodes, 
		PathPlanMap& plans, int skipArc = -1, float threshold = FLT_MAX);
	PathPlan* FindPath(const Vector3<float>& startPoint, const Vector3<float>& endPoint);
	PathPlan* FindPath(const Vector3<float>& startPoint, PathingNode* pGoalNode);
	PathPlan* FindPath(PathingNode* pStartNode, const Vector3<float>& endPoint);
	PathPlan* FindPath(PathingNode* pStartNode, PathingNode* pGoalNode);

	// helpers
	void InsertCluster(PathingCluster* pCluster);
	void InsertNode(PathingNode* pNode);
	void InsertArc(PathingArc* pArc);
	const PathingClusterVec& GetClusters() { return mClusters; }
	const PathingNodeVec& GetNodes() { return mNodes; }
	const PathingArcVec& GetArcs() { return mArcs; }
};


#endif