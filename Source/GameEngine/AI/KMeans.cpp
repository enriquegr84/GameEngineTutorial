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
	float sum = 0.0;
	int dimension = point.size();
	for (int i = 0; i < dimension; i++)
		sum += (float)pow(mPoints[0].GetValue(i) - point[i], 2.0);

	int nearestClusterIdx = 0;
	float minDist = sqrt(sum);
	for (unsigned int i = 1; i < mPoints.size(); i++)
	{
		sum = 0.0;
		for (int j = 0; j < dimension; j++)
			sum += (float)pow(mPoints[i].GetValue(j) - point[j], 2.0);

		float dist = sqrt(sum);
		if (dist < minDist)
		{
			minDist = dist;
			nearestClusterIdx = i;
		}
	}

	return nearestClusterIdx;
}

// return nearest cluster Id (uses euclidean distance)
int KMeans::GetNearestClusterId(Point point,
	eastl::map<int, eastl::map<int, float>>& distances)
{
	float sum = FLT_MAX;
	if (distances.find(point.GetId()) != distances.end() &&
		distances.find(mClusters[0].GetCenterPoint().GetId()) != distances.end())
	{
		if (distances[point.GetId()].find(mClusters[0].GetCenterPoint().GetId()) != 
			distances[point.GetId()].end() &&
			distances[mClusters[0].GetCenterPoint().GetId()].find(point.GetId()) !=
			distances[mClusters[0].GetCenterPoint().GetId()].end())
		{
			sum = distances[mClusters[0].GetCenterPoint().GetId()][point.GetId()];
			sum += distances[point.GetId()][mClusters[0].GetCenterPoint().GetId()];
		}
	}

	int nearestClusterId = 0;
	float minDist = sqrt(sum);
	for (int i = 1; i < mK; i++)
	{
		sum = FLT_MAX;
		if (distances.find(point.GetId()) != distances.end() &&
			distances.find(mClusters[i].GetCenterPoint().GetId()) != distances.end())
		{
			if (distances[point.GetId()].find(mClusters[i].GetCenterPoint().GetId()) !=
				distances[point.GetId()].end() &&
				distances[mClusters[i].GetCenterPoint().GetId()].find(point.GetId()) !=
				distances[mClusters[i].GetCenterPoint().GetId()].end())
			{
				sum = distances[mClusters[i].GetCenterPoint().GetId()][point.GetId()];
				sum += distances[point.GetId()][mClusters[i].GetCenterPoint().GetId()];
			}
		}

		float dist = sqrt(sum);
		if (dist < minDist)
		{
			minDist = dist;
			nearestClusterId = i;
		}
	}

	return nearestClusterId;
}

void KMeans::Run(eastl::vector<Point> & points, 
	eastl::map<int, eastl::map<int, float>>& distances)
{
	mTotalPoints = points.size();
	mDimension = points[0].GetDimension();

	//Initializing Clusters
	eastl::vector<int> usedPointIds;
	for (int i = 0; i < mK; i++)
	{
		while (true)
		{
			int index = rand() % mTotalPoints;

			if (eastl::find(usedPointIds.begin(), usedPointIds.end(), index) == usedPointIds.end())
			{
				usedPointIds.push_back(index);
				points[index].SetCluster(i);
				Clustering cluster(i, points[index]);
				mClusters.push_back(cluster);
				break;
			}
		}
	}

	printf("Clusters initialized = %i \n", mClusters.size());
	printf("Running K-Means Clustering..\n");

	int iter = 1;
	while (true)
	{
		bool done = true;

		// associates each point to the nearest center
		for (int i = 0; i < mTotalPoints; i++)
		{
			int currentClusterId = points[i].GetCluster();
			int nearestClusterId = GetNearestClusterId(points[i], distances);

			if (currentClusterId != nearestClusterId)
			{
				if (currentClusterId != -1)
					mClusters[currentClusterId].RemovePoint(points[i].GetId());

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
		}

		if (done == true || iter >= mIterations)
		{
			printf("Break in iteration %i\n\n", iter);
			break;
		}
		iter++;
	}

	// shows elements of clusters
	int totalPoints = 0;
	for (int i = 0; i < mK; i++)
	{
		int clusterSize = mClusters[i].GetSize();
		totalPoints += clusterSize;

		printf("\nCluster %i : ", mClusters[i].GetId());
		for (int j = 0; j < mDimension; j++)
			printf("%f ", mClusters[i].GetCenterPoint().GetValue(j));
		for (int j = 0; j < mDimension; j++)
			printf("%f ", mClusters[i].GetCenter(j));

		for (int j = 0; j < clusterSize; j++)
		{
			printf("\nPoint %i : ", mClusters[i].GetPoint(j).GetId());
			for (int p = 0; p < mDimension; p++)
				printf("%f ", mClusters[i].GetPoint(j).GetValue(p));
		}
	}

	printf("total points %i \n", totalPoints);
}