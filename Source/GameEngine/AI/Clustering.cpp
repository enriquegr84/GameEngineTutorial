//========================================================================
// Clustering.cpp : Implements a simple clustering system using A*
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

 
#include "Clustering.h"

#include "Core/OS/OS.h"

//--------------------------------------------------------------------------------------------------------
// Cluster
//--------------------------------------------------------------------------------------------------------

void Cluster::DestroyCluster(void)
{
	// destroy all the nodes
	for (ClusteringNodeVec::iterator it = mNodes.begin(); it != mNodes.end(); ++it)
		delete (*it);
	mNodes.clear();

	// destroy all the arcs
	for (ClusterArcVec::iterator it = mArcs.begin(); it != mArcs.end(); ++it)
		delete (*it);
	mArcs.clear();
}

void Cluster::AddArc(ClusterArc* pArc)
{
	LogAssert(pArc, "Invalid arc");
	mArcs.push_back(pArc);
}

void Cluster::GetNeighbors(unsigned int arcType, ClusterArcVec& outNeighbors)
{
	for (ClusterArcVec::iterator it = mArcs.begin(); it != mArcs.end(); ++it)
	{
		ClusterArc* pArc = *it;
		if (arcType == AT_NORMAL)
		{
			if (pArc->GetType() == AT_NORMAL)
				outNeighbors.push_back(pArc);
		}
		else
		{
			if (pArc->GetType() & arcType)
				outNeighbors.push_back(pArc);
		}
	}
}

ClusterArc* Cluster::FindArc(Cluster* pLinkedCluster)
{
	LogAssert(pLinkedCluster, "Invalid cluster");

	for (ClusterArcVec::iterator it = mArcs.begin(); it != mArcs.end(); ++it)
	{
		ClusterArc* pArc = *it;
		if (pArc->GetCluster() == pLinkedCluster)
			return pArc;
	}
	return NULL;
}

ClusterArc* Cluster::FindArc(unsigned int arcType, Cluster* pLinkedCluster)
{
	LogAssert(pLinkedCluster, "Invalid cluster");

	for (ClusterArcVec::iterator it = mArcs.begin(); it != mArcs.end(); ++it)
	{
		ClusterArc* pArc = *it;
		if (pArc->GetType() == arcType)
		{
			if (pArc->GetCluster() == pLinkedCluster)
				return pArc;
		}
	}
	return NULL;
}

void Cluster::RemoveArcs(unsigned int arcType)
{
	ClusterArcVec keepArcs;
	for (ClusterArcVec::iterator it = mArcs.begin(); it != mArcs.end(); ++it)
	{
		ClusterArc* pArc = (*it);
		if (pArc->GetType() != arcType)
			keepArcs.push_back(pArc);
		else
			delete pArc;
	}
	mArcs = keepArcs;
}


ClusteringNode* Cluster::FindClosestNode(const Vector3<float>& pos)
{
	// This is a simple brute-force O(n) algorithm that could be made a LOT faster by utilizing
	// spatial partitioning, like an octree (or quadtree for flat worlds) or something similar.
	ClusteringNode* pClosestNode = NULL;
	float length = FLT_MAX;
	for (ClusteringNodeVec::iterator it = mNodes.begin(); it != mNodes.end(); ++it)
	{
		ClusteringNode* pNode = *it;
		Vector3<float> diff = pos - pNode->GetPos();
		if (Length(diff) < length)
		{
			pClosestNode = pNode;
			length = Length(diff);
		}
	}

	return pClosestNode;
}

ClusteringNode* Cluster::FindFurthestNode(const Vector3<float>& pos)
{
	// This is a simple brute-force O(n) algorithm that could be made a LOT faster by utilizing
	// spatial partitioning, like an octree (or quadtree for flat worlds) or something similar.
	ClusteringNode* pFurthestNode = NULL;
	float length = 0;
	for (ClusteringNodeVec::iterator it = mNodes.begin(); it != mNodes.end(); ++it)
	{
		ClusteringNode* pNode = *it;
		Vector3<float> diff = pos - pNode->GetPos();
		if (Length(diff) > length)
		{
			pFurthestNode = pNode;
			length = Length(diff);
		}
	}

	return pFurthestNode;
}


void Cluster::FindNodes(ClusteringNodeVec& nodes, const Vector3<float>& pos, float radius)
{
	// This is a simple brute-force O(n) algorithm that could be made a LOT faster by utilizing
	// spatial partitioning, like an octree (or quadtree for flat worlds) or something similar.
	for (ClusteringNodeVec::iterator it = mNodes.begin(); it != mNodes.end(); ++it)
	{
		ClusteringNode* pNode = *it;
		Vector3<float> diff = pos - pNode->GetPos();
		if (Length(diff) <= radius)
			nodes.push_back(pNode);
	}
}

ClusteringNode* Cluster::FindNode(unsigned int nodeId)
{
	for (ClusteringNodeVec::iterator it = mNodes.begin(); it != mNodes.end(); ++it)
	{
		ClusteringNode* pNode = *it;
		if (pNode->GetId() == nodeId)
			return pNode;
	}

	return NULL;
}

ClusteringNode* Cluster::FindRandomNode(void)
{
	// cache this since it's not guaranteed to be constant time
	unsigned int numNodes = (unsigned int)mNodes.size();

	// choose a random node
	unsigned int node = (int)(Randomizer::FRand() * numNodes);

	// if we're in the lower half of the node list, start from the bottom
	if (node <= numNodes / 2)
	{
		ClusteringNodeVec::iterator it = mNodes.begin();
		for (unsigned int i = 0; i < node; i++)
			++it;
		return (*it);
	}

	// otherwise, start from the top
	else
	{
		ClusteringNodeVec::iterator it = mNodes.end();
		for (unsigned int i = numNodes; i >= node; i--)
			--it;
		return (*it);
	}
}

ClusterPlan* Cluster::FindNode(const Vector3<float>& startPoint, const Vector3<float>& endPoint)
{
	ClusteringNode* pStart = FindClosestNode(startPoint);
	ClusteringNode* pGoal = FindClosestNode(endPoint);
	return FindNode(pStart, pGoal);
}

ClusterPlan* Cluster::FindNode(const Vector3<float>& startPoint, ClusteringNode* pGoalNode)
{
	ClusteringNode* pStart = FindClosestNode(startPoint);
	return FindNode(pStart, pGoalNode);
}

ClusterPlan* Cluster::FindNode(ClusteringNode* pStartNode, const Vector3<float>& endPoint)
{
	ClusteringNode* pGoal = FindClosestNode(endPoint);
	return FindNode(pStartNode, pGoal);
}

ClusterPlan* Cluster::FindNode(ClusteringNode* pStartNode, ClusteringNode* pGoalNode)
{
	// find the best cluster using an A* search algorithm
	ClusterFinder clusterFinder;
	return clusterFinder(pStartNode, pGoalNode);
}

ClusterPlan* Cluster::FindNode(ClusteringNode* pStartNode, Cluster* pGoalCluster)
{
	// find the best cluster using an A* search algorithm
	ClusterFinder clusterFinder;
	return clusterFinder(pStartNode, pGoalCluster);
}

//--------------------------------------------------------------------------------------------------------
// ClusteringNode
//--------------------------------------------------------------------------------------------------------

void ClusteringNode::AddVisibility(ClusteringNode* pNode, unsigned int vT, float value)
{
	mVisibleNodes[vT][pNode] = value;
}
void ClusteringNode::AddVisibility(ClusteringArc* pArc, unsigned int vT, float value)
{
	mVisibleArcs[vT][pArc] = value;
}
float ClusteringNode::FindVisibility(ClusteringNode* pNode, unsigned int vT)
{
	return mVisibleNodes[vT][pNode];
}
float ClusteringNode::FindVisibility(ClusteringArc* pArc, unsigned int vT)
{
	return mVisibleArcs[vT][pArc];
}
void ClusteringNode::GetVisibilities(unsigned int vT, eastl::map<ClusteringNode*, float>& visibilities)
{
	visibilities = mVisibleNodes[vT];
}
void ClusteringNode::GetVisibilities(unsigned int vT, eastl::map<ClusteringArc*, float>& visibilities)
{
	visibilities = mVisibleArcs[vT];
}

void ClusteringNode::AddArc(ClusteringArc* pArc)
{
	LogAssert(pArc, "Invalid arc");
	mArcs.push_back(pArc);
}

void ClusteringNode::GetNeighbors(unsigned int arcType, ClusteringArcVec& outNeighbors)
{
	for (ClusteringArcVec::iterator it = mArcs.begin(); it != mArcs.end(); ++it)
	{
		ClusteringArc* pArc = *it;
		if (arcType == AT_NORMAL)
		{
			if (pArc->GetType() == AT_NORMAL)
				outNeighbors.push_back(pArc);
		}
		else
		{
			if (pArc->GetType() & arcType)
				outNeighbors.push_back(pArc);
		}
	}
}

ClusteringArc* ClusteringNode::FindArc(unsigned int id)
{
	for (ClusteringArcVec::iterator it = mArcs.begin(); it != mArcs.end(); ++it)
	{
		ClusteringArc* pArc = *it;
		if (pArc->GetId() == id)
			return pArc;
	}
	return NULL;
}

ClusteringArc* ClusteringNode::FindArc(ClusteringNode* pLinkedNode)
{
	LogAssert(pLinkedNode, "Invalid node");
	
	for (ClusteringArcVec::iterator it = mArcs.begin(); it != mArcs.end(); ++it)
	{
		ClusteringArc* pArc = *it;
		if (pArc->GetNode() == pLinkedNode)
			return pArc;
	}
	return NULL;
}

ClusteringArc* ClusteringNode::FindArc(unsigned int arcType, ClusteringNode* pLinkedNode)
{
	LogAssert(pLinkedNode, "Invalid node");

	for (ClusteringArcVec::iterator it = mArcs.begin(); it != mArcs.end(); ++it)
	{
		ClusteringArc* pArc = *it;
		if (pArc->GetType() == arcType)
		{
			if (pArc->GetNode() == pLinkedNode)
				return pArc;
		}
	}
	return NULL;
}

void ClusteringNode::RemoveArcs(unsigned int arcType)
{
	ClusteringArcVec keepArcs;
	for (ClusteringArcVec::iterator it = mArcs.begin(); it != mArcs.end(); ++it)
	{
		ClusteringArc* pArc = (*it);
		if (pArc->GetType() != arcType)
			keepArcs.push_back(pArc);
		else
			delete pArc;
	}
	mArcs = keepArcs;
}

void ClusteringNode::AddTransition(ClusteringTransition* pTransition)
{
	LogAssert(pTransition, "Invalid transition");
	mTransitions.push_back(pTransition);
}

ClusteringTransition* ClusteringNode::FindTransition(unsigned int id)
{
	for (ClusteringTransitionVec::iterator it = mTransitions.begin(); it != mTransitions.end(); ++it)
	{
		ClusteringTransition* pTransition = *it;
		if (pTransition->GetId() == id)
			return pTransition;
	}
	return NULL;
}

ClusteringTransition* ClusteringNode::FindTransition(ClusteringNode* pTransitionNode)
{
	LogAssert(pTransitionNode, "Invalid node");

	for (ClusteringTransitionVec::iterator it = mTransitions.begin(); it != mTransitions.end(); ++it)
	{
		ClusteringTransition* pTransition = *it;
		if (pTransition->GetNode() == pTransitionNode)
			return pTransition;
	}
	return NULL;
}

ClusteringTransition* ClusteringNode::FindTransition(unsigned int arcType, ClusteringNode* pTransitionNode)
{
	LogAssert(pTransitionNode, "Invalid node");

	for (ClusteringTransitionVec::iterator it = mTransitions.begin(); it != mTransitions.end(); ++it)
	{
		ClusteringTransition* pTransition = *it;
		if (pTransition->GetType() == arcType)
		{
			if (pTransition->GetNode() == pTransitionNode)
				return pTransition;
		}
	}
	return NULL;
}

void ClusteringNode::RemoveTransitions(unsigned int arcType)
{
	ClusteringTransitionVec keepTransitions;
	for (ClusteringTransitionVec::iterator it = mTransitions.begin(); it != mTransitions.end(); ++it)
	{
		ClusteringTransition* pTransition = *it;
		if (pTransition->GetType() == arcType)
			keepTransitions.push_back(pTransition);
		else
			delete pTransition;
	}

	mTransitions = keepTransitions;
}


//--------------------------------------------------------------------------------------------------------
// ClusteringArc
//--------------------------------------------------------------------------------------------------------

void ClusteringArc::AddVisibility(ClusteringNode* pNode, unsigned int vT, float value)
{
	mVisibleNodes[vT][pNode] = value;
}
void ClusteringArc::AddVisibility(ClusteringArc* pArc, unsigned int vT, float value)
{
	mVisibleArcs[vT][pArc] = value;
}
float ClusteringArc::FindVisibility(ClusteringNode* pNode, unsigned int vT)
{
	return mVisibleNodes[vT][pNode];
}
float ClusteringArc::FindVisibility(ClusteringArc* pArc, unsigned int vT)
{
	return mVisibleArcs[vT][pArc];
}
void ClusteringArc::GetVisibilities(unsigned int vT, eastl::map<ClusteringNode*, float>& visibilities)
{
	visibilities = mVisibleNodes[vT];
}
void ClusteringArc::GetVisibilities(unsigned int vT, eastl::map<ClusteringArc*, float>& visibilities)
{
	visibilities = mVisibleArcs[vT];
}

//--------------------------------------------------------------------------------------------------------
// ClusterPlan
//--------------------------------------------------------------------------------------------------------

void ClusterPlan::AddArc(ClusteringArc* pArc)
{
	LogAssert(pArc, "Invalid arc");
	mPath.insert(mPath.begin(), pArc);
}

//--------------------------------------------------------------------------------------------------------
// ClusterPlanNode
//--------------------------------------------------------------------------------------------------------
ClusterPlanNode::ClusterPlanNode(ClusteringArc* pArc, ClusterPlanNode* pPrevNode)
{
	LogAssert(pArc, "Invalid arc");
	
	mClusteringArc = pArc;
	mClusteringNode = pArc->GetNode();
	mPrevNode = pPrevNode;  // NULL is a valid value, though it should only be NULL for the start node
	mClosed = false;
	UpdateClusterCost();
}

ClusterPlanNode::ClusterPlanNode(ClusteringNode* pNode, ClusterPlanNode* pPrevNode)
{
	LogAssert(pNode, "Invalid node");

	mClusteringArc = NULL;
	mClusteringNode = pNode;
	mPrevNode = pPrevNode;  // NULL is a valid value, though it should only be NULL for the start node
	mClosed = false;
	UpdateClusterCost();
}

void ClusterPlanNode::UpdatePrevNode(ClusterPlanNode* pPrev)
{
	LogAssert(pPrev, "Invalid node");
	mPrevNode = pPrev;
	UpdateClusterCost();
}

void ClusterPlanNode::UpdateClusterCost(void)
{
	// total cost (g)
	if (mPrevNode)
		mGoal = mPrevNode->GetGoal() + mClusteringArc->GetWeight();
	else
		mGoal = 0;
}

//--------------------------------------------------------------------------------------------------------
// ClusterFinder
//--------------------------------------------------------------------------------------------------------
ClusterFinder::ClusterFinder(void)
{
	mStartNode = NULL;
	mGoalNode = NULL;
}

ClusterFinder::~ClusterFinder(void)
{
	Destroy();
}

void ClusterFinder::Destroy(void)
{
	// destroy all the ClusterPlanNode objects and clear the map
	for (ClusteringNodeToClusterPlanNodeMap::iterator it = mNodes.begin(); it != mNodes.end(); ++it)
		delete it->second;
	mNodes.clear();

	// clear the open set
	mOpenSet.clear();

	// clear the start & goal nodes
	mStartNode = NULL;
	mGoalNode = NULL;
}

//
// ClusterFinder::operator()					- Chapter 18, page 638
//
ClusterPlan* ClusterFinder::operator()(ClusteringNode* pStartNode, ClusteringNode* pGoalNode)
{
	LogAssert(pStartNode, "Invalid node");
	LogAssert(pGoalNode, "Invalid node");

	// if the start and end nodes are the same, we're close enough to b-line to the goal
	if (pStartNode == pGoalNode)
		return NULL;

	// set our members
	mStartNode = pStartNode;
	mGoalNode = pGoalNode;

	ClusterArcVec clusterArcs;
	mGoalNode->GetCluster()->GetNeighbors(AT_ACTION, clusterArcs);

	// The open set is a priority queue of the nodes to be evaluated.  If it's ever empty, it means 
	// we couldn't find a Cluster to the goal. The start node is the only node that is initially in 
	// the open set.
	AddToOpenSet(mStartNode, NULL);

	while (!mOpenSet.empty())
	{
		// grab the most likely candidate
		ClusterPlanNode* planNode = mOpenSet.front();

		// lets find out if we successfully found a Cluster.
		if (planNode->GetClusteringNode() == mGoalNode)
			return RebuildPath(planNode);

		// we're processing this node so remove it from the open set and add it to the closed set
		mOpenSet.pop_front();
		AddToClosedSet(planNode);

		// get the neighboring nodes
		ClusteringArcVec neighbors;
		planNode->GetClusteringNode()->GetNeighbors(AT_NORMAL, neighbors);
		planNode->GetClusteringNode()->GetNeighbors(AT_ACTION, neighbors);

		// loop through all the neighboring nodes and evaluate each one
		for (ClusteringArcVec::iterator it = neighbors.begin(); it != neighbors.end(); ++it)
		{
			ClusteringNode* pNodeToEvaluate = (*it)->GetNode();

			// we traverse only through clusters except when we are closer to the goal cluster
			// in which case we traverse through nodes
			/*
			if (pNodeToEvaluate->GetCluster() == planNode->GetClusteringNode()->GetCluster())
			{
				if (pNodeToEvaluate->GetCluster() != mGoalNode->GetCluster())
				{
					bool skipNode = true;
					for (ClusterArc* clusterArc : clusterArcs)
					{
						if (pNodeToEvaluate->GetCluster() == clusterArc->GetNeighbor())
						{
							skipNode = false;
							break;
						}
					}
					if (skipNode) continue;
				}
			}
			*/
			// Try and find a ClusterPlanNode object for this node.
			ClusteringNodeToClusterPlanNodeMap::iterator findIt = mNodes.find(pNodeToEvaluate);

			// If one exists and it's in the closed list, we've already evaluated the node.  We can
			// safely skip it.
			if (findIt != mNodes.end() && findIt->second->IsClosed())
				continue;

			// figure out the cost for this route through the node
			float costForThisCluster = planNode->GetGoal() + (*it)->GetWeight();
			bool isClusterBetter = false;

			/*
			fprintf(pFile, "arc node %f %f %f to node %f %f %f type %u cost %f\n",
			(*it)->GetOrigin()->GetPos()[0], (*it)->GetOrigin()->GetPos()[1], (*it)->GetOrigin()->GetPos()[2],
			(*it)->GetNeighbor()->GetPos()[0], (*it)->GetNeighbor()->GetPos()[1], (*it)->GetNeighbor()->GetPos()[2],
			(*it)->GetType(), costForThisCluster);
			*/
			// Grab the ClusterPlanNode if there is one.
			ClusterPlanNode* pClusterPlanNodeToEvaluate = NULL;
			if (findIt != mNodes.end())
				pClusterPlanNodeToEvaluate = findIt->second;

			// No ClusterPlanNode means we've never evaluated this Clustering node so we need to add it to 
			// the open set, which has the side effect of setting all the cost data.
			if (!pClusterPlanNodeToEvaluate)
				pClusterPlanNodeToEvaluate = AddToOpenSet((*it), planNode);

			// If this node is already in the open set, check to see if this route to it is better than
			// the last.
			else if (costForThisCluster < pClusterPlanNodeToEvaluate->GetGoal())
				isClusterBetter = true;

			// If this Cluster is better, relink the nodes appropriately, update the cost data, and
			// reinsert the node into the open list priority queue.
			if (isClusterBetter)
			{
				pClusterPlanNodeToEvaluate->UpdatePrevNode(planNode);
				ReinsertNode(pClusterPlanNodeToEvaluate);
			}
		}
	}
	
	return NULL;
}

//
// ClusterFinder::operator()					- Chapter 18, page 638
//
ClusterPlan* ClusterFinder::operator()(ClusteringNode* pStartNode, Cluster* pGoalCluster)
{
	LogAssert(pStartNode, "Invalid node");
	LogAssert(pGoalCluster, "Invalid cluster");

	// if the start and end nodes are the same, we're close enough to b-line to the goal
	if (pStartNode->GetCluster() == pGoalCluster)
		return NULL;

	// set our members
	mStartNode = pStartNode;
	mGoalNode = NULL;

	ClusterArcVec clusterArcs;
	pGoalCluster->GetNeighbors(AT_ACTION, clusterArcs);

	// The open set is a priority queue of the nodes to be evaluated.  If it's ever empty, it means 
	// we couldn't find a Cluster to the goal. The start node is the only node that is initially in 
	// the open set.
	AddToOpenSet(mStartNode, NULL);

	while (!mOpenSet.empty())
	{
		// grab the most likely candidate
		ClusterPlanNode* planNode = mOpenSet.front();

		// lets find out if we successfully found a Cluster.
		if (planNode->GetClusteringNode()->GetCluster() == pGoalCluster)
			return RebuildPath(planNode);

		// we're processing this node so remove it from the open set and add it to the closed set
		mOpenSet.pop_front();
		AddToClosedSet(planNode);

		// get the neighboring nodes
		ClusteringArcVec neighbors;
		planNode->GetClusteringNode()->GetNeighbors(AT_NORMAL, neighbors);
		planNode->GetClusteringNode()->GetNeighbors(AT_ACTION, neighbors);

		// loop though all the neighboring nodes and evaluate each one
		for (ClusteringArcVec::iterator it = neighbors.begin(); it != neighbors.end(); ++it)
		{
			ClusteringNode* pNodeToEvaluate = (*it)->GetNode();

			// we traverse only through clusters except when we are closer to the goal cluster
			// in which case we traverse through nodes
			/*
			if (pNodeToEvaluate->GetCluster() == planNode->GetClusteringNode()->GetCluster())
			{
				if (pNodeToEvaluate->GetCluster() != mGoalNode->GetCluster())
				{
					bool skipNode = true;
					for (ClusterArc* clusterArc : clusterArcs)
					{
						if (pNodeToEvaluate->GetCluster() == clusterArc->GetNeighbor())
						{
							skipNode = false;
							break;
						}
					}
					if (skipNode) continue;
				}
			}
			*/
			// Try and find a ClusterPlanNode object for this node.
			ClusteringNodeToClusterPlanNodeMap::iterator findIt = mNodes.find(pNodeToEvaluate);

			// If one exists and it's in the closed list, we've already evaluated the node.  We can
			// safely skip it.
			if (findIt != mNodes.end() && findIt->second->IsClosed())
				continue;

			// figure out the cost for this route through the node
			float costForThisCluster = planNode->GetGoal() + (*it)->GetWeight();
			bool isClusterBetter = false;

			/*
			fprintf(pFile, "arc node %f %f %f to node %f %f %f type %u cost %f\n",
			(*it)->GetOrigin()->GetPos()[0], (*it)->GetOrigin()->GetPos()[1], (*it)->GetOrigin()->GetPos()[2],
			(*it)->GetNeighbor()->GetPos()[0], (*it)->GetNeighbor()->GetPos()[1], (*it)->GetNeighbor()->GetPos()[2],
			(*it)->GetType(), costForThisCluster);
			*/
			// Grab the ClusterPlanNode if there is one.
			ClusterPlanNode* pClusterPlanNodeToEvaluate = NULL;
			if (findIt != mNodes.end())
				pClusterPlanNodeToEvaluate = findIt->second;

			// No ClusterPlanNode means we've never evaluated this Clustering node so we need to add it to 
			// the open set, which has the side effect of setting all the cost data.
			if (!pClusterPlanNodeToEvaluate)
				pClusterPlanNodeToEvaluate = AddToOpenSet((*it), planNode);

			// If this node is already in the open set, check to see if this route to it is better than
			// the last.
			else if (costForThisCluster < pClusterPlanNodeToEvaluate->GetGoal())
				isClusterBetter = true;

			// If this Cluster is better, relink the nodes appropriately, update the cost data, and
			// reinsert the node into the open list priority queue.
			if (isClusterBetter)
			{
				pClusterPlanNodeToEvaluate->UpdatePrevNode(planNode);
				ReinsertNode(pClusterPlanNodeToEvaluate);
			}
		}
	}

	return NULL;
}

ClusterPlanNode* ClusterFinder::AddToOpenSet(ClusteringArc* pArc, ClusterPlanNode* pPrevNode)
{
	LogAssert(pArc, "Invalid arc");

	// create a new ClusterPlanNode if necessary
	ClusteringNode* pNode = pArc->GetNode();
	ClusteringNodeToClusterPlanNodeMap::iterator it = mNodes.find(pNode);
	ClusterPlanNode* pThisNode = NULL;
	if (it == mNodes.end())
	{
		pThisNode = new ClusterPlanNode(pArc, pPrevNode);
		mNodes.insert(eastl::make_pair(pNode, pThisNode));
	}
	else
	{
		LogWarning("Adding existing ClusterPlanNode to open set");
		pThisNode = it->second;
		pThisNode->SetClosed(false);
	}

	// now insert it into the priority queue
	InsertNode(pThisNode);

	return pThisNode;
}

ClusterPlanNode* ClusterFinder::AddToOpenSet(ClusteringNode* pNode, ClusterPlanNode* pPrevNode)
{
	LogAssert(pNode, "Invalid node");

	// create a new ClusterPlanNode if necessary
	ClusteringNodeToClusterPlanNodeMap::iterator it = mNodes.find(pNode);
	ClusterPlanNode* pThisNode = NULL;
	if (it == mNodes.end())
	{
		pThisNode = new ClusterPlanNode(pNode, pPrevNode);
		mNodes.insert(eastl::make_pair(pNode, pThisNode));
	}
	else
	{
		LogWarning("Adding existing ClusterPlanNode to open set");
		pThisNode = it->second;
		pThisNode->SetClosed(false);
	}

	// now insert it into the priority queue
	InsertNode(pThisNode);

	return pThisNode;
}

void ClusterFinder::AddToClosedSet(ClusterPlanNode* pNode)
{
	LogAssert(pNode, "Invalid node");
	pNode->SetClosed();
}

//
// ClusterFinder::InsertNode					- Chapter 17, page 636
//
void ClusterFinder::InsertNode(ClusterPlanNode* pNode)
{
	LogAssert(pNode, "Invalid node");

	// just add the node if the open set is empty
	if (mOpenSet.empty())
	{
		mOpenSet.push_back(pNode);
		return;
	}

	// otherwise, perform an insertion sort	
	ClusterPlanNodeList::iterator it = mOpenSet.begin();
	ClusterPlanNode* pCompare = *it;
	while (pCompare->IsBetterChoiceThan(pNode))
	{
		++it;

		if (it != mOpenSet.end())
			pCompare = *it;
		else
			break;
	}
	mOpenSet.insert(it, pNode);
}

void ClusterFinder::ReinsertNode(ClusterPlanNode* pNode)
{
	LogAssert(pNode, "Invalid node");

	for (ClusterPlanNodeList::iterator it = mOpenSet.begin(); it != mOpenSet.end(); ++it)
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


ClusterPlan* ClusterFinder::RebuildPath(ClusterPlanNode* pGoalNode)
{
	LogAssert(pGoalNode, "Invalid node");

	ClusterPlan* pPlan = new ClusterPlan();

	ClusterPlanNode* pNode = pGoalNode;
	while (pNode)
	{
		if (pNode->GetClusteringArc() != NULL)
			pPlan->AddArc(pNode->GetClusteringArc());
		pNode = pNode->GetPrev();
	}

	return pPlan;
}

//--------------------------------------------------------------------------------------------------------
// ClusteringGraph
//--------------------------------------------------------------------------------------------------------
void ClusteringGraph::DestroyGraph(void)
{
	// destroy all the clusters
	for (ClusterVec::iterator it = mClusters.begin(); it != mClusters.end(); ++it)
		delete (*it);
	mClusters.clear();
}

ClusteringNode* ClusteringGraph::FindClosestNode(const Vector3<float>& pos)
{
	// This is a simple brute-force O(n) algorithm that could be made a LOT faster by utilizing
	// spatial partitioning, like an octree (or quadtree for flat worlds) or something similar.
	float length = FLT_MAX;
	ClusteringNode* pClosestNode = NULL;
	for (ClusterVec::iterator it = mClusters.begin(); it != mClusters.end(); ++it)
	{
		Cluster* pCluster = *it;
		for (ClusteringNode* pClusterNode : pCluster->GetNodes())
		{
			Vector3<float> diff = pos - pClusterNode->GetPos();
			if (Length(diff) < length)
			{
				pClosestNode = pClusterNode;
				length = Length(diff);
			}
		}
	}
	
	return pClosestNode;
}

ClusteringNode* ClusteringGraph::FindFurthestNode(const Vector3<float>& pos)
{
	// This is a simple brute-force O(n) algorithm that could be made a LOT faster by utilizing
	// spatial partitioning, like an octree (or quadtree for flat worlds) or something similar.
	float length = 0;
	ClusteringNode* pFurthestNode = NULL;
	for (ClusterVec::iterator it = mClusters.begin(); it != mClusters.end(); ++it)
	{
		Cluster* pCluster = *it;
		for (ClusteringNode* pClusterNode : pCluster->GetNodes())
		{
			Vector3<float> diff = pos - pClusterNode->GetPos();
			if (Length(diff) > length)
			{
				pFurthestNode = pClusterNode;
				length = Length(diff);
			}
		}
	}

	return pFurthestNode;
}

void ClusteringGraph::FindClusters(ClusterVec& clusters, const Vector3<float>& pos, float radius)
{
	// This is a simple brute-force O(n) algorithm that could be made a LOT faster by utilizing
	// spatial partitioning, like an octree (or quadtree for flat worlds) or something similar.
	for (ClusterVec::iterator it = mClusters.begin(); it != mClusters.end(); ++it)
	{
		Cluster* pCluster = *it;
		Vector3<float> diff = pos - pCluster->GetCenter()->GetPos();
		if (Length(diff) <= radius)
			clusters.push_back(pCluster);
	}
}

Cluster* ClusteringGraph::FindCluster(unsigned int clusterId)
{
	for (ClusterVec::iterator it = mClusters.begin(); it != mClusters.end(); ++it)
	{
		Cluster* pCluster = *it;
		if (pCluster->GetId() == clusterId)
			return pCluster;
	}

	return NULL;
}

Cluster* ClusteringGraph::FindRandomCluster(void)
{
	// cache this since it's not guaranteed to be constant time
	unsigned int numClusters = (unsigned int)mClusters.size();
	
	// choose a random cluster
	unsigned int cluster = (int)(Randomizer::FRand() * numClusters);
	
	// if we're in the lower half of the node list, start from the bottom
	if (cluster <= numClusters / 2)
	{
		ClusterVec::iterator it = mClusters.begin();
		for (unsigned int i = 0; i < cluster; i++)
			++it;
		return (*it);
	}

	// otherwise, start from the top
	else
	{
		ClusterVec::iterator it = mClusters.end();
		for (unsigned int i = numClusters; i >= cluster; i--)
			--it;
		return (*it);
	}
}

ClusterPlan* ClusteringGraph::FindCluster(const Vector3<float>& startPoint, const Vector3<float>& endPoint)
{
	ClusteringNode* pStart = FindClosestNode(startPoint);
	ClusteringNode* pGoal = FindClosestNode(endPoint);
	return FindCluster(pStart,pGoal);
}

ClusterPlan* ClusteringGraph::FindCluster(const Vector3<float>& startPoint, Cluster* pGoalCluster)
{
	ClusteringNode* pStart = FindClosestNode(startPoint);
	return FindCluster(pStart, pGoalCluster);
}

ClusterPlan* ClusteringGraph::FindCluster(ClusteringNode* pStartNode, ClusteringNode* pGoalNode)
{
	// find the best path using an A* search algorithm
	ClusterFinder clusterFinder;
	return clusterFinder(pStartNode, pGoalNode);
}

ClusterPlan* ClusteringGraph::FindCluster(ClusteringNode* pStartNode, Cluster* pGoalCluster)
{
	// find the best path using an A* search algorithm
	ClusterFinder clusterFinder;
	return clusterFinder(pStartNode, pGoalCluster);
}

void ClusteringGraph::InsertCluster(Cluster* pCluster)
{
	LogAssert(pCluster, "Invalid cluster");

	mClusters.push_back(pCluster);
}