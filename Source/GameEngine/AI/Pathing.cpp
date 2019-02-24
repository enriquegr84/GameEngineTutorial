//========================================================================
// Pathing.cpp : Implements a simple pathing system using A*
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

 
#include "Pathing.h"

#include "Core/OS/OS.h"

//--------------------------------------------------------------------------------------------------------
// PathingNode
//--------------------------------------------------------------------------------------------------------

void PathingNode::AddArc(PathingArc* pArc)
{
	LogAssert(pArc, "Invalid arc");
	mArcs.push_back(pArc);
}

void PathingNode::GetNeighbors(PathingNodeList& outNeighbors)
{
	for (PathingArcList::iterator it = mArcs.begin(); it != mArcs.end(); ++it)
	{
		PathingArc* pArc = *it;
		outNeighbors.push_back(pArc->GetNeighbor(this));
	}
}

float PathingNode::GetCostFromNode(PathingNode* pFromNode)
{
	LogAssert(pFromNode, "Invalid node");
	PathingArc* pArc = FindArc(pFromNode);
	LogAssert(pArc, "Invalid arc");
	Vector3<float> diff = pFromNode->GetPos() - mPos;
	return pArc->GetWeight() * Length(diff);
}

PathingArc* PathingNode::FindArc(PathingNode* pLinkedNode)
{
	LogAssert(pLinkedNode, "Invalid node");
	
	for (PathingArcList::iterator it = mArcs.begin(); it != mArcs.end(); ++it)
	{
		PathingArc* pArc = *it;
		if (pArc->GetNeighbor(this) == pLinkedNode)
			return pArc;
	}
	return NULL;
}


//--------------------------------------------------------------------------------------------------------
// PathingArc
//--------------------------------------------------------------------------------------------------------
void PathingArc::LinkNodes(PathingNode* pNodeA, PathingNode* pNodeB)
{
	LogAssert(pNodeA, "Invalid node");
	LogAssert(pNodeB, "Invalid node");

	mNodes[0] = pNodeA;
	mNodes[1] = pNodeB;
}

PathingNode* PathingArc::GetNeighbor(PathingNode* pMe)
{
	LogAssert(pMe, "Invalid node");
	
	if (mNodes[0] == pMe)
		return mNodes[1];
	else
		return mNodes[0];
}


//--------------------------------------------------------------------------------------------------------
// PathPlan
//--------------------------------------------------------------------------------------------------------
bool PathPlan::CheckForNextNode(const Vector3<float>& pos)
{
	if (mIndex == mPath.end())
		return false;

	Vector3<float> diff = pos - (*mIndex)->GetPos();
	
	// DEBUG dump target orientation
	/*
	wchar_t str[64];  // I'm sure this is overkill
	memset(str,0,sizeof(wchar_t));
	wprintf_s(str,64,_T("distance: %f\n"),diff.Length());
    GE_LOG("AI", str);
	*/
	// end DEBUG
	
	if (Length(diff) <= (*mIndex)->GetTolerance())
	{
		++mIndex;
		return true;
	}
	return false;
}

bool PathPlan::CheckForEnd(void)
{
	if (mIndex == mPath.end())
		return true;
	return false;
}

void PathPlan::AddNode(PathingNode* pNode)
{
	LogAssert(pNode, "Invalid node");
	mPath.push_front(pNode);
}


//--------------------------------------------------------------------------------------------------------
// PathPlanNode
//--------------------------------------------------------------------------------------------------------
PathPlanNode::PathPlanNode(PathingNode* pNode, PathPlanNode* pPrevNode, PathingNode* pGoalNode)
{
	LogAssert(pNode, "Invalid node");
	
	mPathingNode = pNode;
	mPrev = pPrevNode;  // NULL is a valid value, though it should only be NULL for the start node
	mGoalNode = pGoalNode;
	mClosed = false;
	UpdateHeuristics();
}

void PathPlanNode::UpdatePrevNode(PathPlanNode* pPrev)
{
	LogAssert(pPrev, "Invalid node");
	mPrev = pPrev;
	UpdateHeuristics();
}

void PathPlanNode::UpdateHeuristics(void)
{
	// total cost (g)
	if (mPrev)
		mGoal = mPrev->GetGoal() + mPathingNode->GetCostFromNode(mPrev->GetPathingNode());
	else
		mGoal = 0;

	// heuristic (h)
	Vector3<float> diff = mPathingNode->GetPos() - mGoalNode->GetPos();
	mHeuristic = Length(diff);

	// cost to goal (f)
	mFitness = mGoal + mHeuristic;
}


//--------------------------------------------------------------------------------------------------------
// AStar
//--------------------------------------------------------------------------------------------------------
AStar::AStar(void)
{
	mStartNode = NULL;
	mGoalNode = NULL;
}

AStar::~AStar(void)
{
	Destroy();
}

void AStar::Destroy(void)
{
	// destroy all the PathPlanNode objects and clear the map
	for (PathingNodeToPathPlanNodeMap::iterator it = mNodes.begin(); it != mNodes.end(); ++it)
		delete it->second;
	mNodes.clear();
	
	// clear the open set
	mOpenSet.clear();
	
	// clear the start & goal nodes
	mStartNode = NULL;
	mGoalNode = NULL;
}


//
// AStar::operator()					- Chapter 18, page 638
//
PathPlan* AStar::operator()(PathingNode* pStartNode, PathingNode* pGoalNode)
{
	LogAssert(pStartNode, "Invalid node");
	LogAssert(pGoalNode, "Invalid node");

	// if the start and end nodes are the same, we're close enough to b-line to the goal
	if (pStartNode == pGoalNode)
		return NULL;

	// set our members
	mStartNode = pStartNode;
	mGoalNode = pGoalNode;
		
	// The open set is a priority queue of the nodes to be evaluated.  If it's ever empty, it means 
	// we couldn't find a path to the goal.  The start node is the only node that is initially in 
	// the open set.
	AddToOpenSet(mStartNode, NULL);

	while (!mOpenSet.empty())
	{
		// grab the most likely candidate
		PathPlanNode* pNode = mOpenSet.front();

		// If this node is our goal node, we've successfully found a path.
		if (pNode->GetPathingNode() == mGoalNode)
			return RebuildPath(pNode);

		// we're processing this node so remove it from the open set and add it to the closed set
		mOpenSet.pop_front();
		AddToClosedSet(pNode);

		// get the neighboring nodes
		PathingNodeList neighbors;
		pNode->GetPathingNode()->GetNeighbors(neighbors);

		// loop though all the neighboring nodes and evaluate each one
		for (PathingNodeList::iterator it = neighbors.begin(); it != neighbors.end(); ++it)
		{
			PathingNode* pNodeToEvaluate = *it;

			// Try and find a PathPlanNode object for this node.
			PathingNodeToPathPlanNodeMap::iterator findIt = mNodes.find(pNodeToEvaluate);
			
			// If one exists and it's in the closed list, we've already evaluated the node.  We can
			// safely skip it.
			if (findIt != mNodes.end() && findIt->second->IsClosed())
				continue;
			
			// figure out the cost for this route through the node
			float costForThisPath = 
				pNode->GetGoal() + pNodeToEvaluate->GetCostFromNode(pNode->GetPathingNode());
			bool isPathBetter = false;

			// Grab the PathPlanNode if there is one.
			PathPlanNode* pPathPlanNodeToEvaluate = NULL;
			if (findIt != mNodes.end())
				pPathPlanNodeToEvaluate = findIt->second;

			// No PathPlanNode means we've never evaluated this pathing node so we need to add it to 
			// the open set, which has the side effect of setting all the heuristic data.  It also 
			// means that this is the best path through this node that we've found so the nodes are 
			// linked together (which is why we don't bother setting isPathBetter to true; it's done
			// for us in AddToOpenSet()).
			if (!pPathPlanNodeToEvaluate)
				pPathPlanNodeToEvaluate = AddToOpenSet(pNodeToEvaluate,pNode);
			
			// If this node is already in the open set, check to see if this route to it is better than
			// the last.
			else if (costForThisPath < pPathPlanNodeToEvaluate->GetGoal())
				isPathBetter = true;
			
			// If this path is better, relink the nodes appropriately, update the heuristics data, and
			// reinsert the node into the open list priority queue.
			if (isPathBetter)
			{
				pPathPlanNodeToEvaluate->UpdatePrevNode(pNode);
				ReinsertNode(pPathPlanNodeToEvaluate);
			}
		}
	}
	
	// If we get here, there's no path to the goal.
	return NULL;
}

PathPlanNode* AStar::AddToOpenSet(PathingNode* pNode, PathPlanNode* pPrevNode)
{
	LogAssert(pNode, "Invalid node");

	// create a new PathPlanNode if necessary
	PathingNodeToPathPlanNodeMap::iterator it = mNodes.find(pNode);
	PathPlanNode* pThisNode = NULL;
	if (it == mNodes.end())
	{
		pThisNode = new PathPlanNode(pNode,pPrevNode,mGoalNode);
		mNodes.insert(eastl::make_pair(pNode,pThisNode));
	}
	else
	{
		LogWarning("Adding existing PathPlanNode to open set");
		pThisNode = it->second;
		pThisNode->SetClosed(false);
	}
	
	// now insert it into the priority queue
	InsertNode(pThisNode);

	return pThisNode;
}

void AStar::AddToClosedSet(PathPlanNode* pNode)
{
	LogAssert(pNode, "Invalid node");
	pNode->SetClosed();
}

//
// AStar::InsertNode					- Chapter 17, page 636
//
void AStar::InsertNode(PathPlanNode* pNode)
{
	LogAssert(pNode, "Invalid node");
	
	// just add the node if the open set is empty
	if (mOpenSet.empty())
	{
		mOpenSet.push_back(pNode);
		return;
	}

	// otherwise, perform an insertion sort	
	PathPlanNodeList::iterator it = mOpenSet.begin();
	PathPlanNode* pCompare = *it;
	while (pCompare->IsBetterChoiceThan(pNode))
	{
		++it;
		
		if (it != mOpenSet.end())
			pCompare = *it;
		else
			break;
	}
	mOpenSet.insert(it,pNode);
}

void AStar::ReinsertNode(PathPlanNode* pNode)
{
	LogAssert(pNode, "Invalid node");

	for (PathPlanNodeList::iterator it = mOpenSet.begin(); it != mOpenSet.end(); ++it)
	{
		if (pNode == (*it))
		{
			mOpenSet.erase(it);
			InsertNode(pNode);
			return;
		}
	}

	// if we get here, the node was never in the open set to begin with
    LogWarning("Attemping to reinsert node that was never in the open list");
	InsertNode(pNode);
}

PathPlan* AStar::RebuildPath(PathPlanNode* pGoalNode)
{
	LogAssert(pGoalNode, "Invalid node");

	PathPlan* pPlan = new PathPlan();

	PathPlanNode* pNode = pGoalNode;
	while (pNode)
	{
		pPlan->AddNode(pNode->GetPathingNode());
		pNode = pNode->GetPrev();
	}
	
	return pPlan;
}


//--------------------------------------------------------------------------------------------------------
// PathingGraph
//--------------------------------------------------------------------------------------------------------
void PathingGraph::DestroyGraph(void)
{
	// destroy all the nodes
	for (PathingNodeVec::iterator it = mNodes.begin(); it != mNodes.end(); ++it)
		delete (*it);
	mNodes.clear();
	
	// destroy all the arcs
	for (PathingArcList::iterator it = mArcs.begin(); it != mArcs.end(); ++it)
		delete (*it);
	mArcs.clear();
}

PathingNode* PathingGraph::FindClosestNode(const Vector3<float>& pos)
{
	// This is a simple brute-force O(n) algorithm that could be made a LOT faster by utilizing
	// spatial partitioning, like an octree (or quadtree for flat worlds) or something similar.
	PathingNode* pClosestNode = NULL;
	float length = FLT_MAX;
	for (PathingNodeVec::iterator it = mNodes.begin(); it != mNodes.end(); ++it)
	{
		PathingNode* pNode = *it;
		Vector3<float> diff = pos - pNode->GetPos();
		if (Length(diff) < length)
		{
			pClosestNode = pNode;
			length = Length(diff);
		}
	}
	
	return pClosestNode;
}

PathingNode* PathingGraph::FindFurthestNode(const Vector3<float>& pos)
{
	// This is a simple brute-force O(n) algorithm that could be made a LOT faster by utilizing
	// spatial partitioning, like an octree (or quadtree for flat worlds) or something similar.
	PathingNode* pFurthestNode = NULL;
	float length = 0;
	for (PathingNodeVec::iterator it = mNodes.begin(); it != mNodes.end(); ++it)
	{
		PathingNode* pNode = *it;
		Vector3<float> diff = pos - pNode->GetPos();
		if (Length(diff) > length)
		{
			pFurthestNode = pNode;
			length = Length(diff);
		}
	}

	return pFurthestNode;
}

PathingNode* PathingGraph::FindRandomNode(void)
{
	// cache this since it's not guaranteed to be constant time
	unsigned int numNodes = (unsigned int)mNodes.size();
	
	// choose a random node
	unsigned int node = (int)(Randomizer::FRand() * numNodes);
	
	// if we're in the lower half of the node list, start from the bottom
	if (node <= numNodes / 2)
	{
		PathingNodeVec::iterator it = mNodes.begin();
		for (unsigned int i = 0; i < node; i++)
			++it;
		return (*it);
	}

	// otherwise, start from the top
	else
	{
		PathingNodeVec::iterator it = mNodes.end();
		for (unsigned int i = numNodes; i >= node; i--)
			--it;
		return (*it);
	}
}

PathPlan* PathingGraph::FindPath(const Vector3<float>& startPoint, const Vector3<float>& endPoint)
{
	// Find the closest nodes to the start and end points.  There really should be some ray-casting 
	// to ensure that we can actually make it to the closest node, but this is good enough for now.
	PathingNode* pStart = FindClosestNode(startPoint);
	PathingNode* pGoal = FindClosestNode(endPoint);
	return FindPath(pStart,pGoal);
}

PathPlan* PathingGraph::FindPath(const Vector3<float>& startPoint, PathingNode* pGoalNode)
{
	PathingNode* pStart = FindClosestNode(startPoint);
	return FindPath(pStart,pGoalNode);
}

PathPlan* PathingGraph::FindPath(PathingNode* pStartNode, const Vector3<float>& endPoint)
{
	PathingNode* pGoal = FindClosestNode(endPoint);
	return FindPath(pStartNode,pGoal);
}

PathPlan* PathingGraph::FindPath(PathingNode* pStartNode, PathingNode* pGoalNode)
{
	// find the best path using an A* search algorithm
	AStar aStar;
	return aStar(pStartNode,pGoalNode);
}

void PathingGraph::InsertNode(PathingNode* pNode)
{
	LogAssert(pNode, "Invalid node");

	mNodes.push_back(pNode);
}

void PathingGraph::LinkNodes(PathingNode* pNodeA, PathingNode* pNodeB)
{
	LogAssert(pNodeA, "Invalid node");
	LogAssert(pNodeB, "Invalid node");
	
	PathingArc* pArc = new PathingArc();
	pArc->LinkNodes(pNodeA,pNodeB);
	pNodeA->AddArc(pArc);
	pNodeB->AddArc(pArc);
	mArcs.push_back(pArc);
}
