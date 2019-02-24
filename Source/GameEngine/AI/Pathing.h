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

class PathingArc;
class PathingNode;
class PathPlanNode;
class AStar;

typedef eastl::list<PathingArc*> PathingArcList;
typedef eastl::list<PathingNode*> PathingNodeList;
typedef eastl::vector<PathingNode*> PathingNodeVec;
typedef eastl::list<PathPlanNode*> PathPlanNodeList;
typedef eastl::map<PathingNode*, PathingNodeVec> PathingNodeMap;
typedef eastl::map<PathingNode*, PathPlanNode*> PathingNodeToPathPlanNodeMap;

const float PATHING_DEFAULT_NODE_TOLERANCE = 5.0f;
const float PATHING_DEFAULT_ARC_WEIGHT = 1.0f;

//--------------------------------------------------------------------------------------------------------
// class PathingNode				- Chapter 18, page 636
// This class represents a single node in the pathing graph.
//
//--------------------------------------------------------------------------------------------------------
class PathingNode
{
	unsigned int mId;
	Vector3<float> mPos;
	PathingArcList mArcs;

	ActorId mActorId;
	float mTolerance;
	
public:
	explicit PathingNode(unsigned int id, ActorId actorId, 
		const Vector3<float>& pos, float tolerance = PATHING_DEFAULT_NODE_TOLERANCE)
		: mId(id), mActorId(actorId), mPos(pos) 
	{ 
		mTolerance = tolerance; 
	}

	void SetActorId(ActorId actorId) { mActorId = actorId; }
	ActorId GetActorId(void) const { return mActorId; }
	float GetTolerance(void) const { return mTolerance; }
	const Vector3<float>& GetPos(void) const { return mPos; }

	void AddArc(PathingArc* pArc);
	PathingArc* FindArc(PathingNode* pLinkedNode);
	PathingArc* FindArc(unsigned int arcType, PathingNode* pLinkedNode);
	void GetNeighbors(PathingNodeList& outNeighbors);
	float GetCostFromNode(PathingNode* pFromNode);
};


//--------------------------------------------------------------------------------------------------------
// class PathingArc				- Chapter 18, page 636
// This class represents an arc that links two nodes, allowing travel between them.
//--------------------------------------------------------------------------------------------------------
class PathingArc
{
	float mWeight;
	unsigned int mType;
	Vector3<float> mConnection; //an optional interpolation vector which connects nodes 
	PathingNode* mNodes[2];  // an arc always connects two nodes

public:
	explicit PathingArc(unsigned int type = 0, 
		float weight = PATHING_DEFAULT_ARC_WEIGHT, 
		const Vector3<float>& connect = Vector3<float>::Zero()) 
	{ 
		mType = type;
		mWeight = weight; 
		mConnection = connect;
	}
	float GetWeight(void) const { return mWeight; }
	unsigned int GetType(void) const { return mType; }
	const Vector3<float>& GetConnection(void) const { return mConnection; }
	void LinkNodes(PathingNode* pNodeA, PathingNode* pNodeB);
	PathingNode* GetNeighbor(PathingNode* pMe);
};


//--------------------------------------------------------------------------------------------------------
// class PathingPlan				- Chapter 18, page 636
// This class represents a complete path and is used by the higher-level AI to determine where to go.
//--------------------------------------------------------------------------------------------------------
class PathPlan
{
	friend class AStar;

	PathingNodeList mPath;
	PathingNodeList::iterator mIndex;
	
public:
	PathPlan(void) { mIndex = mPath.end(); }
	
	void ResetPath(void) { mIndex = mPath.begin(); }
	const Vector3<float>& GetCurrentNodePosition(void) const 
	{ LogAssert(mIndex != mPath.end(), "Invalid index"); return (*mIndex)->GetPos(); }
	bool CheckForNextNode(const Vector3<float>& pos);
	bool CheckForEnd(void);
	
private:
	void AddNode(PathingNode* pNode);
};


//--------------------------------------------------------------------------------------------------------
// class PathPlanNode						- Chapter 18, page 636
// This class is a helper used in PathingGraph::FindPath().  It tracks the heuristical and cost data for
// a given node when building a path.
//--------------------------------------------------------------------------------------------------------
class PathPlanNode
{
	PathPlanNode* mPrev;  // node we just came from
	PathingNode* mPathingNode;  // pointer to the pathing node from the pathing graph
	PathingNode* mGoalNode;  // pointer to the goal node
	bool mClosed;  // the node is closed if it's already been processed
	float mGoal;  // cost of the entire path up to this point (often called g)
	float mHeuristic;  // estimated cost of this node to the goal (often called h)
	float mFitness;  // estimated cost from start to the goal through this node (often called f)
	
public:
	explicit PathPlanNode(PathingNode* pNode, PathPlanNode* pPrevNode, PathingNode* pGoalNode);
	PathPlanNode* GetPrev(void) const { return mPrev; }
	PathingNode* GetPathingNode(void) const { return mPathingNode; }
	bool IsClosed(void) const { return mClosed; }
	float GetGoal(void) const { return mGoal; }
	float GetHeuristic(void) const { return mHeuristic; }
	float GetFitness(void) const { return mFitness; }
	
	void UpdatePrevNode(PathPlanNode* pPrev);
	void SetClosed(bool toClose = true) { mClosed = toClose; }
	
	bool IsBetterChoiceThan(PathPlanNode* pRight) { return (mFitness < pRight->GetFitness()); }
	
private:
	void UpdateHeuristics(void);
};


//--------------------------------------------------------------------------------------------------------
// class AStar								- Chapter 18, page 638
// This class implements the A* algorithm.
//--------------------------------------------------------------------------------------------------------
class AStar
{
	PathingNodeToPathPlanNodeMap mNodes;
	PathingNode* mStartNode;
	PathingNode* mGoalNode;
	PathPlanNodeList mOpenSet;
	
public:
	AStar(void);
	~AStar(void);
	void Destroy(void);
	
	PathPlan* operator()(PathingNode* pStartNode, PathingNode* pGoalNode);

private:
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
	PathingNodeVec mNodes;  // master list of all nodes
	PathingArcList mArcs;  // master list of all arcs
	
public:
	PathingGraph(void) {}
	~PathingGraph(void) { DestroyGraph(); }
	void DestroyGraph(void);

	PathingNode* FindClosestNode(const Vector3<float>& pos);
	PathingNode* FindFurthestNode(const Vector3<float>& pos);
	PathingNode* FindRandomNode(void);
	PathPlan* FindPath(const Vector3<float>& startPoint, const Vector3<float>& endPoint);
	PathPlan* FindPath(const Vector3<float>& startPoint, PathingNode* pGoalNode);
	PathPlan* FindPath(PathingNode* pStartNode, const Vector3<float>& endPoint);
	PathPlan* FindPath(PathingNode* pStartNode, PathingNode* pGoalNode);
	
	// helpers
	void InsertNode(PathingNode* pNode);
	void LinkNodes(PathingNode* pNodeA, PathingNode* pNodeB);
};


#endif