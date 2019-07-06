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
// PathingCluster
//--------------------------------------------------------------------------------------------------------

void PathingCluster::LinkClusters(PathingNode* pNode, PathingNode* pTarget)
{
	LogAssert(pNode, "Invalid node");
	LogAssert(pTarget, "Invalid node");

	mNode = pNode;
	mTarget = pTarget;
}

//--------------------------------------------------------------------------------------------------------
// PathingNode
//--------------------------------------------------------------------------------------------------------

void PathingNode::AddVisibleNode(PathingNode* pNode, float value)
{
	mVisibleNodes[pNode] = value;
}

float PathingNode::FindVisibleNode(PathingNode* pNode)
{
	if (mVisibleNodes.find(pNode) != mVisibleNodes.end())
		return mVisibleNodes[pNode];

	return FLT_MAX;
}

bool PathingNode::IsVisibleNode(PathingNode* pNode)
{
	return mVisibleNodes.find(pNode) != mVisibleNodes.end();
}

const eastl::map<PathingNode*, float>& PathingNode::GetVisibileNodes()
{
	return mVisibleNodes;
}

void PathingNode::AddArc(PathingArc* pArc)
{
	LogAssert(pArc, "Invalid arc");
	mArcs.push_back(pArc);
}

void PathingNode::GetArcs(unsigned int arcType, PathingArcVec& outArcs)
{
	for (PathingArcVec::iterator it = mArcs.begin(); it != mArcs.end(); ++it)
	{
		PathingArc* pArc = *it;
		if (arcType == AT_NORMAL)
		{
			if (pArc->GetType() == AT_NORMAL)
				outArcs.push_back(pArc);
		}
		else
		{
			if (pArc->GetType() & arcType)
				outArcs.push_back(pArc);
		}
	}
}

PathingArc* PathingNode::FindArc(PathingNode* pLinkedNode)
{
	LogAssert(pLinkedNode, "Invalid node");
	
	for (PathingArcVec::iterator it = mArcs.begin(); it != mArcs.end(); ++it)
	{
		PathingArc* pArc = *it;
		if (pArc->GetNode() == pLinkedNode)
			return pArc;
	}
	return NULL;
}

PathingArc* PathingNode::FindArc(unsigned int arcType, PathingNode* pLinkedNode)
{
	LogAssert(pLinkedNode, "Invalid node");

	for (PathingArcVec::iterator it = mArcs.begin(); it != mArcs.end(); ++it)
	{
		PathingArc* pArc = *it;
		if (pArc->GetType() == arcType)
		{
			if (pArc->GetNode() == pLinkedNode)
				return pArc;
		}
	}
	return NULL;
}

void PathingNode::RemoveArcs()
{
	for (PathingArcVec::iterator it = mArcs.begin(); it != mArcs.end(); ++it)
	{
		PathingArc* pArc = (*it);
		delete pArc;
	}

	mArcs.clear();
}

void PathingNode::RemoveArc(unsigned int id)
{
	for (PathingArcVec::iterator it = mArcs.begin(); it != mArcs.end(); ++it)
	{
		PathingArc* pArc = (*it);
		if (pArc->GetId() != id)
		{
			delete pArc;
			mArcs.erase(it);
			break;
		}
	}
}

void PathingNode::AddCluster(PathingCluster* pCluster)
{
	LogAssert(pCluster, "Invalid cluster");
	mClusters.push_back(pCluster);
}

void PathingNode::AddClusterActor(PathingCluster* pCluster)
{
	LogAssert(pCluster, "Invalid cluster actor");
	mClusterActors.push_back(pCluster);
}

PathingCluster* PathingNode::GetCluster(unsigned int clusterType, unsigned short clusterId)
{
	for (PathingClusterVec::iterator it = mClusters.begin(); it != mClusters.end(); ++it)
	{
		PathingCluster* pCluster = *it;
		if (pCluster->GetType() == clusterType)
		{
			if (pCluster->GetTarget()->GetCluster() == clusterId && pCluster->GetActor() == INVALID_ACTOR_ID)
				return pCluster;
		}
	}

	return NULL;
}

void PathingNode::GetClusters(unsigned short clusterId, eastl::map<unsigned int, PathingCluster*>& outClusters)
{
	for (PathingClusterVec::iterator it = mClusters.begin(); it != mClusters.end(); ++it)
	{
		PathingCluster* pCluster = *it;
		if (pCluster->GetTarget()->GetCluster() == clusterId && pCluster->GetActor() == INVALID_ACTOR_ID)
			outClusters[pCluster->GetType()] = pCluster;
	}
}

void PathingNode::GetClusters(unsigned int clusterType, PathingClusterVec& outClusters, unsigned int limit)
{
	for (PathingClusterVec::iterator it = mClusters.begin(); it != mClusters.end(); ++it)
	{
		if (outClusters.size() < limit)
		{
			PathingCluster* pCluster = *it;
			if (pCluster->GetType() == clusterType && pCluster->GetActor() == INVALID_ACTOR_ID)
				outClusters.push_back(pCluster);
		}
		else break;
	}
}

void PathingNode::GetClusters(unsigned int clusterType, PathingClusterVec& outClusters)
{
	for (PathingClusterVec::iterator it = mClusters.begin(); it != mClusters.end(); ++it)
	{
		PathingCluster* pCluster = *it;
		if (pCluster->GetType() == clusterType && pCluster->GetActor() == INVALID_ACTOR_ID)
			outClusters.push_back(pCluster);
	}
}

void PathingNode::GetClusters(PathingClusterVec& outClusters, unsigned int limit)
{
	for (PathingClusterVec::iterator it = mClusters.begin(); it != mClusters.end(); ++it)
	{
		if (outClusters.size() < limit)
		{
			PathingCluster* pCluster = *it;
			if (pCluster->GetActor() == INVALID_ACTOR_ID)
				outClusters.push_back(pCluster);
		}
		else break;
	}
}

PathingCluster* PathingNode::GetClusterActor(unsigned int clusterType, unsigned short clusterId)
{
	for (PathingClusterVec::iterator it = mClusterActors.begin(); it != mClusterActors.end(); ++it)
	{
		PathingCluster* pCluster = *it;
		if (pCluster->GetType() == clusterType)
		{
			if (pCluster->GetTarget()->GetCluster() == clusterId)
				return pCluster;
		}
	}

	return NULL;
}

void PathingNode::GetClusterActors(unsigned short clusterId, eastl::map<unsigned int, PathingCluster*>& outClusters)
{
	for (PathingClusterVec::iterator it = mClusterActors.begin(); it != mClusterActors.end(); ++it)
	{
		PathingCluster* pCluster = *it;
		if (pCluster->GetTarget()->GetCluster() == clusterId)
			outClusters[pCluster->GetType()] = pCluster;
	}
}

void PathingNode::GetClusterActors(unsigned int clusterType, PathingClusterVec& outClusters, unsigned int limit)
{
	for (PathingClusterVec::iterator it = mClusterActors.begin(); it != mClusterActors.end(); ++it)
	{
		if (outClusters.size() < limit)
		{
			PathingCluster* pCluster = *it;
			if (pCluster->GetType() == clusterType)
				outClusters.push_back(pCluster);
		}
		else break;
	}
}

void PathingNode::GetClusterActors(unsigned int clusterType, PathingClusterVec& outClusters)
{
	for (PathingClusterVec::iterator it = mClusterActors.begin(); it != mClusterActors.end(); ++it)
	{
		PathingCluster* pCluster = *it;
		if (pCluster->GetType() == clusterType)
			outClusters.push_back(pCluster);
	}
}

void PathingNode::GetClusterActors(PathingClusterVec& outClusters, unsigned int limit)
{
	for (PathingClusterVec::iterator it = mClusterActors.begin(); it != mClusterActors.end(); ++it)
	{
		if (outClusters.size() < limit)
		{
			PathingCluster* pCluster = *it;
			outClusters.push_back(pCluster);
		}
		else break;
	}
}

PathingCluster* PathingNode::FindCluster(PathingNode* pTargetNode)
{
	LogAssert(pTargetNode, "Invalid node");

	for (PathingClusterVec::iterator it = mClusters.begin(); it != mClusters.end(); ++it)
	{
		PathingCluster* pCluster = *it;
		if (pCluster->GetTarget() == pTargetNode && pCluster->GetActor() == INVALID_ACTOR_ID)
			return pCluster;
	}
	return NULL;
}

PathingCluster* PathingNode::FindCluster(unsigned int clusterType, PathingNode* pTargetNode)
{
	LogAssert(pTargetNode, "Invalid node");

	for (PathingClusterVec::iterator it = mClusters.begin(); it != mClusters.end(); ++it)
	{
		PathingCluster* pCluster = *it;
		if (pCluster->GetType() == clusterType)
		{
			if (pCluster->GetTarget() == pTargetNode && pCluster->GetActor() == INVALID_ACTOR_ID)
				return pCluster;
		}
	}
	return NULL;
}

PathingCluster* PathingNode::FindClusterActor(PathingNode* pTargetNode)
{
	LogAssert(pTargetNode, "Invalid node");

	for (PathingClusterVec::iterator it = mClusterActors.begin(); it != mClusterActors.end(); ++it)
	{
		PathingCluster* pCluster = *it;
		if (pCluster->GetTarget() == pTargetNode)
			return pCluster;
	}
	return NULL;
}

PathingCluster* PathingNode::FindClusterActor(unsigned int clusterType, PathingNode* pTargetNode)
{
	LogAssert(pTargetNode, "Invalid node");

	for (PathingClusterVec::iterator it = mClusterActors.begin(); it != mClusterActors.end(); ++it)
	{
		PathingCluster* pCluster = *it;
		if (pCluster->GetType() == clusterType)
		{
			if (pCluster->GetTarget() == pTargetNode)
				return pCluster;
		}
	}
	return NULL;
}

void PathingNode::OrderClusters(bool ascending)
{
	eastl::map<unsigned int, eastl::vector<PathingCluster*>> clusterTypes;
	for (PathingClusterVec::iterator it = mClusters.begin(); it != mClusters.end(); ++it)
	{
		PathingCluster* pCluster = *it;
		clusterTypes[pCluster->GetType()].push_back(pCluster);
	}

	for (auto clusterType : clusterTypes)
	{
		eastl::map<float, eastl::vector<PathingCluster*>> clusters;
		for (PathingCluster* cluster : clusterType.second)
		{
			float clusterWeight = 0.f;
			PathingNode* currentNode = this;
			while (currentNode != cluster->GetTarget())
			{
				PathingCluster* currentCluster = cluster->GetActor() == INVALID_ACTOR_ID ?
					currentNode->FindCluster(clusterType.first, cluster->GetTarget()) :
					currentNode->FindClusterActor(clusterType.first, cluster->GetTarget());
				PathingArc* currentArc = currentNode->FindArc(currentCluster->GetNode());
				clusterWeight += currentArc->GetWeight();
				currentNode = currentArc->GetNode();
			}
			clusters[clusterWeight].push_back(cluster);
		}

		clusterTypes[clusterType.first].clear();
		for (auto cluster : clusters)
		{
			if (ascending)
			{
				for (PathingCluster* pathCluster : cluster.second)
					clusterTypes[clusterType.first].push_back(pathCluster);
			}
			else
			{
				for (PathingCluster* pathCluster : cluster.second)
					clusterTypes[clusterType.first].insert(0, pathCluster);
			}
		}
	}

	mClusters.clear();
	mClusterActors.clear();
	for (auto clusterType : clusterTypes)
	{
		for (auto cluster : clusterType.second)
		{
			mClusters.push_back(cluster);
			if (cluster->GetActor() != INVALID_ACTOR_ID)
				mClusterActors.push_back(cluster);
		}
	}
}

void PathingNode::RemoveClusters()
{
	for (PathingClusterVec::iterator it = mClusters.begin(); it != mClusters.end(); ++it)
	{
		PathingCluster* pCluster = *it;
		delete pCluster;
	}

	mClusters.clear();
}
void PathingNode::AddTransition(PathingTransition* pTransition)
{
	LogAssert(pTransition, "Invalid transition");
	mTransitions.push_back(pTransition);
}

PathingTransition* PathingNode::FindTransition(unsigned int id)
{
	for (PathingTransitionVec::iterator it = mTransitions.begin(); it != mTransitions.end(); ++it)
	{
		PathingTransition* pTransition = *it;
		if (pTransition->GetId() == id)
			return pTransition;
	}

	return NULL;
}

void PathingNode::RemoveTransitions()
{
	for (PathingTransitionVec::iterator it = mTransitions.begin(); it != mTransitions.end(); ++it)
	{
		PathingTransition* pTransition = *it;
		delete pTransition;
	}

	mTransitions.clear();
}

void PathingNode::RemoveTransition(unsigned int id)
{
	for (PathingTransitionVec::iterator it = mTransitions.begin(); it != mTransitions.end(); ++it)
	{
		PathingTransition* pTransition = *it;
		if (pTransition->GetId() == id)
		{
			delete pTransition;
			mTransitions.erase(it);
			break;
		}
	}
}

//--------------------------------------------------------------------------------------------------------
// PathPlan
//--------------------------------------------------------------------------------------------------------
bool PathPlan::CheckForNextNode(const Vector3<float>& pos)
{
	if (mIndex == mPath.end())
		return false;

	Vector3<float> diff = pos - (*mIndex)->GetNode()->GetPos();
	if (Length(diff) <= (float)PATHING_DEFAULT_NODE_TOLERANCE)
	{
		mIndex++;
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

void PathPlan::AddArc(PathingArc* pArc)
{
	LogAssert(pArc, "Invalid arc");
	mPath.insert(mPath.begin(), pArc);
}


//--------------------------------------------------------------------------------------------------------
// PathPlanNode
//--------------------------------------------------------------------------------------------------------
PathPlanNode::PathPlanNode(PathingArc* pArc, PathPlanNode* pPrevNode, PathingNode* pGoalNode)
{
	LogAssert(pArc, "Invalid arc");
	
	mPathingArc = pArc;
	mPathingNode = pArc->GetNode();
	mPrevNode = pPrevNode;  // NULL is a valid value, though it should only be NULL for the start node
	mGoalNode = pGoalNode;
	mClosed = false;
	UpdatePathCost();
}

PathPlanNode::PathPlanNode(PathingNode* pNode, PathPlanNode* pPrevNode, PathingNode* pGoalNode)
{
	LogAssert(pNode, "Invalid node");

	mPathingArc = NULL;
	mPathingNode = pNode;
	mPrevNode = pPrevNode;  // NULL is a valid value, though it should only be NULL for the start node
	mGoalNode = pGoalNode;
	mClosed = false;
	UpdatePathCost();
}

void PathPlanNode::UpdateNode(PathingArc* pArc, PathPlanNode* pPrev)
{
	LogAssert(pPrev, "Invalid node");
	mPathingArc = pArc;
	mPathingNode = pArc->GetNode();
	mPrevNode = pPrev;
	UpdatePathCost();
}

void PathPlanNode::UpdatePathCost(void)
{
	// total cost (g)
	if (mPrevNode)
		mGoal = mPrevNode->GetGoal() + mPathingArc->GetWeight();
	else
		mGoal = 0;
}


//--------------------------------------------------------------------------------------------------------
// PathFinder
//--------------------------------------------------------------------------------------------------------
PathFinder::PathFinder(void)
{
	mStartNode = NULL;
	mGoalNode = NULL;
}

PathFinder::~PathFinder(void)
{
	Destroy();
}

void PathFinder::Destroy(void)
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
// PathFinder::operator()					- Chapter 18, page 638
//
PathPlan* PathFinder::operator()(PathingNode* pStartNode, PathingNode* pGoalNode, int skipArc, float threshold)
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
	// we couldn't find a path to the goal. The start node is the only node that is initially in 
	// the open set.
	AddToOpenSet(mStartNode, NULL);

	while (!mOpenSet.empty())
	{
		// grab the most likely candidate
		PathPlanNode* planNode = mOpenSet.front();

		// lets find out if we successfully found a path.
		if (planNode->GetPathingNode() == mGoalNode)
			return RebuildPath(planNode);

		// we're processing this node so remove it from the open set and add it to the closed set
		mOpenSet.pop_front();
		AddToClosedSet(planNode);

		// get the neighboring nodes
		PathingArcVec neighbors;
		planNode->GetPathingNode()->GetArcs(AT_NORMAL, neighbors);
		planNode->GetPathingNode()->GetArcs(AT_ACTION, neighbors);

		// loop though all the neighboring nodes and evaluate each one
		for (PathingArcVec::iterator it = neighbors.begin(); it != neighbors.end(); ++it)
		{
			if (skipArc == (*it)->GetType()) continue;

			PathingNode* pNodeToEvaluate = (*it)->GetNode();

			// Try and find a PathPlanNode object for this node.
			PathingNodeToPathPlanNodeMap::iterator findIt = mNodes.find(pNodeToEvaluate);
			
			// If one exists and it's in the closed list, we've already evaluated the node.  We can
			// safely skip it.
			if (findIt != mNodes.end() && findIt->second->IsClosed())
				continue;

			// figure out the cost for this route through the node
			float costForThisPath = planNode->GetGoal() + (*it)->GetWeight();
			if (costForThisPath >= threshold)
				continue;

			bool isPathBetter = false;

			// Grab the PathPlanNode if there is one.
			PathPlanNode* pPathPlanNodeToEvaluate = NULL;
			if (findIt != mNodes.end())
				pPathPlanNodeToEvaluate = findIt->second;

			// No PathPlanNode means we've never evaluated this pathing node so we need to add it to 
			// the open set, which has the side effect of setting all the cost data.
			if (!pPathPlanNodeToEvaluate)
				pPathPlanNodeToEvaluate = AddToOpenSet((*it), planNode);
			
			// If this node is already in the open set, check to see if this route to it is better than
			// the last.
			else if (costForThisPath < pPathPlanNodeToEvaluate->GetGoal())
				isPathBetter = true;
			
			// If this path is better, relink the nodes appropriately, update the cost data, and
			// reinsert the node into the open list priority queue.
			if (isPathBetter)
			{
				pPathPlanNodeToEvaluate->UpdateNode((*it), planNode);
				ReinsertNode(pPathPlanNodeToEvaluate);
			}
		}
	}
	
	return NULL;
}

//
// PathFinder::operator()					- Chapter 18, page 638
//
PathPlan* PathFinder::operator()(PathingNode* pStartNode, PathingNodeVec& searchNodes, int skipArc, float threshold)
{
	LogAssert(pStartNode, "Invalid node");

	// set our members
	mStartNode = pStartNode;
	mGoalNode = NULL;

	// The open set is a priority queue of the nodes to be evaluated.  If it's ever empty, it means 
	// we couldn't find a path to the goal. The start node is the only node that is initially in 
	// the open set.
	AddToOpenSet(mStartNode, NULL);

	float minCostGoal = FLT_MAX;
	PathPlan* pathPlan = NULL;
	while (!mOpenSet.empty())
	{
		// grab the most likely candidate
		PathPlanNode* planNode = mOpenSet.front();

		// lets find out if we successfully found a path.
		PathingNodeVec::iterator itNode = 
			eastl::find(searchNodes.begin(), searchNodes.end(), planNode->GetPathingNode());
		if (itNode != searchNodes.end())
		{
			if (planNode->GetGoal() < minCostGoal)
			{
				delete pathPlan;

				minCostGoal = planNode->GetGoal();
				pathPlan = RebuildPath(planNode);
			}
		}

		// we're processing this node so remove it from the open set and add it to the closed set
		mOpenSet.pop_front();
		AddToClosedSet(planNode);

		// get the neighboring nodes
		PathingArcVec neighbors;
		planNode->GetPathingNode()->GetArcs(AT_NORMAL, neighbors);
		planNode->GetPathingNode()->GetArcs(AT_ACTION, neighbors);

		// loop though all the neighboring nodes and evaluate each one
		for (PathingArcVec::iterator it = neighbors.begin(); it != neighbors.end(); ++it)
		{
			if (skipArc == (*it)->GetType()) continue;

			PathingNode* pNodeToEvaluate = (*it)->GetNode();

			// Try and find a PathPlanNode object for this node.
			PathingNodeToPathPlanNodeMap::iterator findIt = mNodes.find(pNodeToEvaluate);

			// If one exists and it's in the closed list, we've already evaluated the node.  We can
			// safely skip it.
			if (findIt != mNodes.end() && findIt->second->IsClosed())
				continue;

			// figure out the cost for this route through the node
			float costForThisPath = planNode->GetGoal() + (*it)->GetWeight();
			if (costForThisPath >= threshold)
				continue;

			if (costForThisPath >= minCostGoal)
				continue;

			bool isPathBetter = false;
			// Grab the PathPlanNode if there is one.
			PathPlanNode* pPathPlanNodeToEvaluate = NULL;
			if (findIt != mNodes.end())
				pPathPlanNodeToEvaluate = findIt->second;

			// No PathPlanNode means we've never evaluated this pathing node so we need to add it to 
			// the open set, which has the side effect of setting all the cost data.
			if (!pPathPlanNodeToEvaluate)
				pPathPlanNodeToEvaluate = AddToOpenSet((*it), planNode);

			// If this node is already in the open set, check to see if this route to it is better than
			// the last.
			else if (costForThisPath < pPathPlanNodeToEvaluate->GetGoal())
				isPathBetter = true;

			// If this path is better, relink the nodes appropriately, update the cost data, and
			// reinsert the node into the open list priority queue.
			if (isPathBetter)
			{
				pPathPlanNodeToEvaluate->UpdateNode((*it), planNode);
				ReinsertNode(pPathPlanNodeToEvaluate);
			}
		}
	}

	return pathPlan;
}

void PathFinder::operator()(PathingNode* pStartNode, 
	PathingNodeVec& searchNodes, PathPlanMap& plans, int skipArc, float threshold)
{
	LogAssert(pStartNode, "Invalid node");

	// set our members
	mStartNode = pStartNode;
	mGoalNode = NULL;

	// The open set is a priority queue of the nodes to be evaluated.  If it's ever empty, it means 
	// we couldn't find a path to the goal. The start node is the only node that is initially in 
	// the open set.
	AddToOpenSet(mStartNode, NULL);

	eastl::map<PathingNode*, float> minCostNode;
	for (PathingNode* node : searchNodes)
		minCostNode[node] = FLT_MAX;

	while (!mOpenSet.empty())
	{
		// grab the most likely candidate
		PathPlanNode* planNode = mOpenSet.front();

		// lets find out if we successfully found a node.
		PathingNodeVec::iterator itNode =
			eastl::find(searchNodes.begin(), searchNodes.end(), planNode->GetPathingNode());
		if (itNode != searchNodes.end())
		{
			if (plans.find((*itNode)) == plans.end())
			{
				minCostNode[(*itNode)] = planNode->GetGoal();
				plans[(*itNode)] = RebuildPath(planNode);
			}
			else if (planNode->GetGoal() < minCostNode[(*itNode)])
			{
				minCostNode[(*itNode)] = planNode->GetGoal();

				delete plans[(*itNode)];
				plans[(*itNode)] = RebuildPath(planNode);
			}
		}

		// we're processing this node so remove it from the open set and add it to the closed set
		mOpenSet.pop_front();
		AddToClosedSet(planNode);

		// get the neighboring nodes
		PathingArcVec neighbors;
		planNode->GetPathingNode()->GetArcs(AT_NORMAL, neighbors);
		planNode->GetPathingNode()->GetArcs(AT_ACTION, neighbors);

		// loop though all the neighboring nodes and evaluate each one
		for (PathingArcVec::iterator it = neighbors.begin(); it != neighbors.end(); ++it)
		{
			if (skipArc == (*it)->GetType()) continue;

			PathingNode* pNodeToEvaluate = (*it)->GetNode();

			// Try and find a PathPlanNode object for this node.
			PathingNodeToPathPlanNodeMap::iterator findIt = mNodes.find(pNodeToEvaluate);

			// If one exists and it's in the closed list, we've already evaluated the node.  We can
			// safely skip it.
			if (findIt != mNodes.end() && findIt->second->IsClosed())
				continue;

			// figure out the cost for this route through the node
			float costForThisPath = planNode->GetGoal() + (*it)->GetWeight();
			if (costForThisPath >= threshold)
				continue;

			bool isPathBetter = false;
			// Grab the PathPlanNode if there is one.
			PathPlanNode* pPathPlanNodeToEvaluate = NULL;
			if (findIt != mNodes.end())
				pPathPlanNodeToEvaluate = findIt->second;

			// No PathPlanNode means we've never evaluated this pathing node so we need to add it to 
			// the open set, which has the side effect of setting all the cost data.
			if (!pPathPlanNodeToEvaluate)
				pPathPlanNodeToEvaluate = AddToOpenSet((*it), planNode);

			// If this node is already in the open set, check to see if this route to it is better than
			// the last.
			else if (costForThisPath < pPathPlanNodeToEvaluate->GetGoal())
				isPathBetter = true;

			// If this path is better, relink the nodes appropriately, update the cost data, and
			// reinsert the node into the open list priority queue.
			if (isPathBetter)
			{
				pPathPlanNodeToEvaluate->UpdateNode((*it), planNode);
				ReinsertNode(pPathPlanNodeToEvaluate);
			}
		}
	}
}

void PathFinder::operator()(PathingNode* pStartNode,
	eastl::vector<eastl::shared_ptr<Actor>>& searchActors, ActorPlanMap& plans, int skipArc, float threshold)
{
	LogAssert(pStartNode, "Invalid node");

	// set our members
	mStartNode = pStartNode;
	mGoalNode = NULL;

	// The open set is a priority queue of the nodes to be evaluated.  If it's ever empty, it means 
	// we couldn't find a path to the goal. The start node is the only node that is initially in 
	// the open set.
	AddToOpenSet(mStartNode, NULL);

	eastl::map<eastl::shared_ptr<Actor>, float> minCostActor;
	for (eastl::shared_ptr<Actor> actor : searchActors)
		minCostActor[actor] = FLT_MAX;

	while (!mOpenSet.empty())
	{
		// grab the most likely candidate
		PathPlanNode* planNode = mOpenSet.front();

		// lets find out if we successfully found an actor.
		eastl::vector<eastl::shared_ptr<Actor>>::iterator itActor;
		for (itActor = searchActors.begin(); itActor != searchActors.end(); itActor++)
			if ((*itActor)->GetId() == planNode->GetPathingNode()->GetActorId())
				break;

		if (itActor != searchActors.end())
		{
			if (plans.find((*itActor)) == plans.end())
			{
				minCostActor[(*itActor)] = planNode->GetGoal();
				plans[(*itActor)] = RebuildPath(planNode);
			}
			else if (planNode->GetGoal() < minCostActor[(*itActor)])
			{
				minCostActor[(*itActor)] = planNode->GetGoal();

				delete plans[(*itActor)];
				plans[(*itActor)] = RebuildPath(planNode);
			}
		}

		// we're processing this node so remove it from the open set and add it to the closed set
		mOpenSet.pop_front();
		AddToClosedSet(planNode);

		// get the neighboring nodes
		PathingArcVec neighbors;
		planNode->GetPathingNode()->GetArcs(AT_NORMAL, neighbors);
		planNode->GetPathingNode()->GetArcs(AT_ACTION, neighbors);

		// loop though all the neighboring nodes and evaluate each one
		for (PathingArcVec::iterator it = neighbors.begin(); it != neighbors.end(); ++it)
		{
			if (skipArc == (*it)->GetType()) continue;

			PathingNode* pNodeToEvaluate = (*it)->GetNode();

			// Try and find a PathPlanNode object for this node.
			PathingNodeToPathPlanNodeMap::iterator findIt = mNodes.find(pNodeToEvaluate);

			// If one exists and it's in the closed list, we've already evaluated the node.  We can
			// safely skip it.
			if (findIt != mNodes.end() && findIt->second->IsClosed())
				continue;

			// figure out the cost for this route through the node
			float costForThisPath = planNode->GetGoal() + (*it)->GetWeight();
			if (costForThisPath >= threshold)
				continue;

			bool isPathBetter = false;
			// Grab the PathPlanNode if there is one.
			PathPlanNode* pPathPlanNodeToEvaluate = NULL;
			if (findIt != mNodes.end())
				pPathPlanNodeToEvaluate = findIt->second;

			// No PathPlanNode means we've never evaluated this pathing node so we need to add it to 
			// the open set, which has the side effect of setting all the cost data.
			if (!pPathPlanNodeToEvaluate)
				pPathPlanNodeToEvaluate = AddToOpenSet((*it), planNode);

			// If this node is already in the open set, check to see if this route to it is better than
			// the last.
			else if (costForThisPath < pPathPlanNodeToEvaluate->GetGoal())
				isPathBetter = true;

			// If this path is better, relink the nodes appropriately, update the cost data, and
			// reinsert the node into the open list priority queue.
			if (isPathBetter)
			{
				pPathPlanNodeToEvaluate->UpdateNode((*it), planNode);
				ReinsertNode(pPathPlanNodeToEvaluate);
			}
		}
	}
}

void PathFinder::operator()(PathingNode* pStartNode,
	eastl::vector<unsigned short>& searchClusters, ClusterPlanMap& plans, int skipArc, float threshold)
{
	LogAssert(pStartNode, "Invalid node");

	// set our members
	mStartNode = pStartNode;
	mGoalNode = NULL;

	// The open set is a priority queue of the nodes to be evaluated.  If it's ever empty, it means 
	// we couldn't find a path to the goal. The start node is the only node that is initially in 
	// the open set.
	AddToOpenSet(mStartNode, NULL);

	eastl::map<unsigned short, float> minCostCluster;
	for (unsigned short cluster : searchClusters)
		minCostCluster[cluster] = FLT_MAX;

	while (!mOpenSet.empty())
	{
		// grab the most likely candidate
		PathPlanNode* planNode = mOpenSet.front();

		// lets find out if we successfully found a cluster.
		eastl::vector<unsigned short>::iterator itCluster =
			eastl::find(searchClusters.begin(), searchClusters.end(), planNode->GetPathingNode()->GetCluster());
		if (itCluster != searchClusters.end())
		{
			if (plans.find((*itCluster)) == plans.end())
			{
				minCostCluster[(*itCluster)] = planNode->GetGoal();
				plans[(*itCluster)] = RebuildPath(planNode);
			}
			else if (planNode->GetGoal() < minCostCluster[(*itCluster)])
			{
				minCostCluster[(*itCluster)] = planNode->GetGoal();

				delete plans[(*itCluster)];
				plans[(*itCluster)] = RebuildPath(planNode);
			}
		}

		// we're processing this node so remove it from the open set and add it to the closed set
		mOpenSet.pop_front();
		AddToClosedSet(planNode);

		// get the neighboring nodes
		PathingArcVec neighbors;
		planNode->GetPathingNode()->GetArcs(AT_NORMAL, neighbors);
		planNode->GetPathingNode()->GetArcs(AT_ACTION, neighbors);

		// loop though all the neighboring nodes and evaluate each one
		for (PathingArcVec::iterator it = neighbors.begin(); it != neighbors.end(); ++it)
		{
			if (skipArc == (*it)->GetType()) continue;

			PathingNode* pNodeToEvaluate = (*it)->GetNode();

			// Try and find a PathPlanNode object for this node.
			PathingNodeToPathPlanNodeMap::iterator findIt = mNodes.find(pNodeToEvaluate);

			// If one exists and it's in the closed list, we've already evaluated the node.  We can
			// safely skip it.
			if (findIt != mNodes.end() && findIt->second->IsClosed())
				continue;

			// figure out the cost for this route through the node
			float costForThisPath = planNode->GetGoal() + (*it)->GetWeight();
			if (costForThisPath >= threshold)
				continue;

			bool isPathBetter = false;
			// Grab the PathPlanNode if there is one.
			PathPlanNode* pPathPlanNodeToEvaluate = NULL;
			if (findIt != mNodes.end())
				pPathPlanNodeToEvaluate = findIt->second;

			// No PathPlanNode means we've never evaluated this pathing node so we need to add it to 
			// the open set, which has the side effect of setting all the cost data.
			if (!pPathPlanNodeToEvaluate)
				pPathPlanNodeToEvaluate = AddToOpenSet((*it), planNode);

			// If this node is already in the open set, check to see if this route to it is better than
			// the last.
			else if (costForThisPath < pPathPlanNodeToEvaluate->GetGoal())
				isPathBetter = true;

			// If this path is better, relink the nodes appropriately, update the cost data, and
			// reinsert the node into the open list priority queue.
			if (isPathBetter)
			{
				pPathPlanNodeToEvaluate->UpdateNode((*it), planNode);
				ReinsertNode(pPathPlanNodeToEvaluate);
			}
		}
	}
}

PathPlanNode* PathFinder::AddToOpenSet(PathingArc* pArc, PathPlanNode* pPrevNode)
{
	LogAssert(pArc, "Invalid arc");

	// create a new PathPlanNode if necessary
	PathingNode* pNode = pArc->GetNode();
	PathingNodeToPathPlanNodeMap::iterator it = mNodes.find(pNode);
	PathPlanNode* pThisNode = NULL;
	if (it == mNodes.end())
	{
		pThisNode = new PathPlanNode(pArc,pPrevNode,mGoalNode);
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

PathPlanNode* PathFinder::AddToOpenSet(PathingNode* pNode, PathPlanNode* pPrevNode)
{
	LogAssert(pNode, "Invalid node");

	// create a new PathPlanNode if necessary
	PathingNodeToPathPlanNodeMap::iterator it = mNodes.find(pNode);
	PathPlanNode* pThisNode = NULL;
	if (it == mNodes.end())
	{
		pThisNode = new PathPlanNode(pNode, pPrevNode, mGoalNode);
		mNodes.insert(eastl::make_pair(pNode, pThisNode));
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

void PathFinder::AddToClosedSet(PathPlanNode* pNode)
{
	LogAssert(pNode, "Invalid node");
	pNode->SetClosed();
}

//
// PathFinder::InsertNode					- Chapter 17, page 636
//
void PathFinder::InsertNode(PathPlanNode* pNode)
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

void PathFinder::ReinsertNode(PathPlanNode* pNode)
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

PathPlan* PathFinder::RebuildPath(PathPlanNode* pGoalNode)
{
	LogAssert(pGoalNode, "Invalid node");

	PathPlan* pPlan = new PathPlan();

	PathPlanNode* pNode = pGoalNode;
	while (pNode)
	{
		if (pNode->GetPathingArc() != NULL)
			pPlan->AddArc(pNode->GetPathingArc());
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
	{
		// destroy all arcs and transitions
		(*it)->RemoveTransitions();
		(*it)->RemoveArcs();
		delete (*it);
	}
	mNodes.clear();
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

void PathingGraph::FindNodes(PathingNodeVec& nodes, const Vector3<float>& pos, float radius)
{
	// This is a simple brute-force O(n) algorithm that could be made a LOT faster by utilizing
	// spatial partitioning, like an octree (or quadtree for flat worlds) or something similar.
	for (PathingNodeVec::iterator it = mNodes.begin(); it != mNodes.end(); ++it)
	{
		PathingNode* pNode = *it;
		Vector3<float> diff = pos - pNode->GetPos();
		if (Length(diff) <= radius)
			nodes.push_back(pNode);
	}
}

PathingNode* PathingGraph::FindNode(unsigned int nodeId)
{
	for (PathingNodeVec::iterator it = mNodes.begin(); it != mNodes.end(); ++it)
	{
		PathingNode* pNode = *it;
		if (pNode->GetId() == nodeId)
			return pNode;
	}

	return NULL;
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

void PathingGraph::FindPlans(PathingNode* pStartNode, 
	PathingNodeVec& searchNodes, PathPlanMap& plans, int skipArc, float threshold)
{
	// find the best path using an A* search algorithm
	PathFinder pathFinder;
	pathFinder(pStartNode, searchNodes, plans, skipArc, threshold);
}

void PathingGraph::FindPlans(PathingNode* pStartNode,
	eastl::vector<eastl::shared_ptr<Actor>>& searchActors, ActorPlanMap& plans, int skipArc, float threshold)
{
	// find the best path using an A* search algorithm
	PathFinder pathFinder;
	pathFinder(pStartNode, searchActors, plans, skipArc, threshold);
}

void PathingGraph::FindPlans(PathingNode* pStartNode,
	eastl::vector<unsigned short>& searchClusters, ClusterPlanMap& plans, int skipArc, float threshold)
{
	// find the best path using an A* search algorithm
	PathFinder pathFinder;
	pathFinder(pStartNode, searchClusters, plans, skipArc, threshold);
}

PathPlan* PathingGraph::FindPath(
	const Vector3<float>& startPoint, const Vector3<float>& endPoint, int skipArc, float threshold)
{
	PathingNode* pStart = FindClosestNode(startPoint);
	PathingNode* pGoal = FindClosestNode(endPoint);
	return FindPath(pStart, pGoal, skipArc, threshold);
}

PathPlan* PathingGraph::FindPath(
	PathingNode* pStartNode, PathingNodeVec& searchNodes, int skipArc, float threshold)
{
	// find the best path using an A* search algorithm
	PathFinder pathFinder;
	return pathFinder(pStartNode, searchNodes, skipArc, threshold);
}

PathPlan* PathingGraph::FindPath(
	const Vector3<float>& startPoint, PathingNode* pGoalNode, int skipArc, float threshold)
{
	PathingNode* pStart = FindClosestNode(startPoint);
	return FindPath(pStart, pGoalNode, skipArc, threshold);
}

PathPlan* PathingGraph::FindPath(
	PathingNode* pStartNode, const Vector3<float>& endPoint, int skipArc, float threshold)
{
	PathingNode* pGoal = FindClosestNode(endPoint);
	return FindPath(pStartNode, pGoal, skipArc, threshold);
}

PathPlan* PathingGraph::FindPath(
	PathingNode* pStartNode, PathingNode* pGoalNode, int skipArc, float threshold)
{
	// find the best path using an A* search algorithm
	PathFinder pathFinder;
	return pathFinder(pStartNode, pGoalNode, skipArc, threshold);
}

void PathingGraph::InsertNode(PathingNode* pNode)
{
	LogAssert(pNode, "Invalid node");

	mNodes.push_back(pNode);
}

void PathingGraph::InsertCluster(PathingCluster* pCluster)
{
	LogAssert(pCluster, "Invalid cluster");

	mClusters.push_back(pCluster);
}

void PathingGraph::InsertArc(PathingArc* pArc)
{
	LogAssert(pArc, "Invalid arc");

	mArcs.push_back(pArc);
}

void PathingGraph::InsertVisibleCluster(unsigned short clusterA, unsigned short clusterB)
{
	mVisibleClusters[clusterA][clusterB] = true;
}

bool PathingGraph::IsVisibleCluster(unsigned short clusterA, unsigned short clusterB)
{
	bool isVisible = false;
	if (mVisibleClusters.find(clusterA) != mVisibleClusters.end())
		if (mVisibleClusters[clusterA].find(clusterB) != mVisibleClusters[clusterA].end())
			isVisible = true;

	return isVisible;
}