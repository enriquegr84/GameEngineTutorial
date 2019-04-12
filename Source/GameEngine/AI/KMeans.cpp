//========================================================================
// KMeans.cpp : Implements a clustering algorithm
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

#include "KMeans.h"

// return nearest point (uses euclidean distance)
int Clustering::GetNearestPointIndex(const eastl::vector<float>& point)
{
	int dimension = point.size();
	int nearestPointIdx = -1;
	float minDist = FLT_MAX;
	for (unsigned int i = 0; i < mPoints.size(); i++)
	{
		float sum = 0.0;
		for (int j = 0; j < dimension; j++)
			sum += (float)pow(mPoints[i].GetValue(j) - point[j], 2.0);

		float dist = sqrt(sum);
		if (dist < minDist)
		{
			minDist = dist;
			nearestPointIdx = i;
		}
	}

	return nearestPointIdx;
}

// return nearest point (uses euclidean distance)
int KMeans::GetNearestClusterId(Point point)
{
	float sum = 0.0;
	for (int i = 0; i < mDimension; i++)
		sum += (float)pow(mClusters[0].GetCenter(i) - point.GetValue(i), 2.0);

	int nearestClusterId = 0;
	float minDist = sqrt(sum);
	for (int i = 1; i < mK; i++)
	{
		sum = 0.0;
		for (int j = 0; j < mDimension; j++)
			sum += (float)pow(mClusters[i].GetCenter(j) - point.GetValue(j), 2.0);

		float dist = sqrt(sum);
		if (dist < minDist)
		{
			minDist = dist;
			nearestClusterId = i;
		}
	}

	return nearestClusterId;
}

unsigned int KMeans::FindClusterId(unsigned int pointId)
{
	for (int i = 0; i < mK; i++)
	{
		if (mClusters[i].GetCenterPoint().GetId() == pointId)
			return i;
	}

	return -1;
}

void KMeans::AddArc(PointArc arc)
{
	bool addArc = true;
	for (PointArc pointArc : mArcs)
	{
		if (pointArc.GetId() == arc.GetId())
		{
			addArc = false;
			break;
		}
	}
	if (addArc) mArcs.push_back(arc);
}

bool KMeans::IsArc(int arcId)
{
	for (PointArc pointArc : mArcs)
	{
		if (pointArc.GetId() == arcId)
			return true;
	}
	return false;
}

void KMeans::Run(eastl::vector<Point> & points)
{
	mTotalPoints = points.size();
	mDimension = points[0].GetDimension();

	//Initializing Clusters
	eastl::vector<int> usedPointIds;
	for (int k = 0; k < mK; k++)
	{
		while (true)
		{
			int index = rand() % mTotalPoints;

			if (eastl::find(usedPointIds.begin(), usedPointIds.end(), index) == usedPointIds.end())
			{
				usedPointIds.push_back(index);
				points[index].SetCluster(k);
				Clustering cluster(k, points[index]);
				mClusters.push_back(cluster);
				break;
			}
		}
	}

	eastl::map<unsigned int, PathingNode*> nodes;
	for (PathingNode* pathNode : mPathingGraph->GetNodes())
		nodes[pathNode->GetId()] = pathNode;

	printf("Clusters initialized = %i \n", mClusters.size());
	printf("Running K-Means Clustering..\n");

	int iter = 1;
	while (true)
	{
		bool done = true;

		PathingNodeVec pathNodes;
		for (int i = 0; i < mK; i++)
			pathNodes.push_back(nodes[mClusters[i].GetCenterPoint().GetId()]);

		// associates each point to the nearest center
		for (int i = 0; i < mTotalPoints; i++)
		{
			int nearestClusterId;
			int currentClusterId = points[i].GetCluster();

			PathingNode* nearestClusterNode = 
				mPathingGraph->FindNodes(nodes[points[i].GetId()], pathNodes);
			if (nearestClusterNode == NULL)
				nearestClusterId = GetNearestClusterId(points[i]);
			else
				nearestClusterId = FindClusterId(nearestClusterNode->GetId());

			if (currentClusterId != nearestClusterId)
			{
				if (currentClusterId != -1)
				{
					mClusters[currentClusterId].RemovePoint(points[i].GetId());
					mClusters[currentClusterId].RemoveIsolatedPoint(points[i].GetId());
				}

				points[i].SetCluster(nearestClusterId);
				mClusters[nearestClusterId].AddPoint(points[i]);
				done = false;
			}
		}

		// recalculating the center of each cluster
		for (int i = 0; i < mK; i++)
		{
			int clusterSize = mClusters[i].GetSize();
			for (int j = 0; j < mDimension; j++)
			{
				float sum = 0.0;
				if (clusterSize > 0)
				{
					for (int p = 0; p < clusterSize; p++)
						sum += mClusters[i].GetPoint(p).GetValue(j);

					mClusters[i].SetCenter(j, sum / clusterSize);
				}
			}

			PathingNodeVec clusterNodes;
			for (int p = 0; p < clusterSize; p++)
				clusterNodes.push_back(nodes[mClusters[i].GetPoint(p).GetId()]);

			//we find node connections within the cluster to determine the center
			PathingNodeVec minNodes;
			PathingNode* pathConnection = NULL;
			PathingNodePlanDoubleMap nodeConnections;
			mPathingGraph->FindConnections(clusterNodes, nodeConnections, 1.0f);

			Vector3<float> centerPos{
				mClusters[i].GetCenter(0),mClusters[i].GetCenter(1), mClusters[i].GetCenter(2) };
			for (auto nodeConnection : nodeConnections)
			{
				PathingNodeVec currentNodes;
				PathingNode* pathNode = nodeConnection.first;
				for (auto node : nodeConnection.second)
				{
					PathingNode* connectNode = node.first;
					if (nodeConnections[connectNode][pathNode] &&
						nodeConnections[pathNode][connectNode])
					{
						currentNodes.push_back(connectNode);
					}
				}

				if (currentNodes.size() == minNodes.size())
				{
					//we take the closest to the center
					if (pathConnection)
					{
						if (Length(pathConnection->GetPos() - centerPos) >
							Length(pathNode->GetPos() - centerPos))
						{
							minNodes = currentNodes;
							pathConnection = pathNode;
						}
					}
					else
					{
						minNodes = currentNodes;
						pathConnection = pathNode;
					}
				}
				else if (currentNodes.size() > minNodes.size() && currentNodes.size() > 1)
				{
					minNodes = currentNodes;
					pathConnection = pathNode;
				}
			}

			if (done == true || iter >= mIterations)
			{
				for (auto nodeConnection : nodeConnections)
				{
					for (auto node : nodeConnection.second)
					{
						PathPlan* pPlan = node.second;
						if (pPlan)
						{
							for (PathingArc* pArc : pPlan->GetArcs())
								AddArc(PointArc(pArc->GetId(), pArc->GetType()));
						}
					}
				}
			}

			//delete plan from pathfinder
			for (auto nodeConnection : nodeConnections)
			{
				for (auto node : nodeConnection.second)
					delete node.second;
			}

			//set isolated points
			mClusters[i].ClearIsolatedPoints();
			for (PathingNode* clusterNode : clusterNodes)
			{
				if (eastl::find(minNodes.begin(), minNodes.end(), clusterNode) == minNodes.end())
				{
					int index = mClusters[i].GetPointIndex(clusterNode->GetId());
					mClusters[i].AddIsolatedPoint(mClusters[i].GetPoint(index));
				}
			}

			if (pathConnection)
			{
				mClusters[i].SetCenterPoint(
					mClusters[i].GetPointIndex(pathConnection->GetId()));
			}
			else
			{
				mClusters[i].SetCenterPoint(
					mClusters[i].GetNearestPointIndex(mClusters[i].GetCenterValue()));
			}
		}

		if (done == true || iter >= mIterations)
		{
			printf("Break in iteration %i\n\n", iter);
			break;
		}
		iter++;
	}
}