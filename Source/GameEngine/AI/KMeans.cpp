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

// return nearest cluster Id (uses euclidean distance)
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
		for (int j = 0; j < mTotalPoints; j++)
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

void KMeans::Run(eastl::vector<Point> & points)
{
	mTotalPoints = points.size();
	mDimension = points[0].GetDimension();

	//Initializing Clusters
	eastl::vector<int> usedPointIds;

	for (int i = 1; i <= mK; i++)
	{
		while (true)
		{
			int index = rand() % mTotalPoints;

			if (eastl::find(usedPointIds.begin(), usedPointIds.end(), index) == usedPointIds.end())
			{
				usedPointIds.push_back(index);
				points[index].SetCluster(i);
				Cluster cluster(i, points[index]);
				mClusters.push_back(cluster);
				break;
			}
		}
	}

	int iter = 1;
	while (true)
	{
		bool done = true;

		// associates each point to the nearest center
		for (int i = 0; i < mTotalPoints; i++)
		{
			int currentClusterId = points[i].GetCluster();
			int nearestClusterId = GetNearestClusterId(points[i]);

			if (currentClusterId != nearestClusterId)
			{
				if (currentClusterId != -1)
					mClusters[currentClusterId].RemovePoint(points[i].GetID());

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
	for (int i = 0; i < mK; i++)
	{
		int clusterSize = mClusters[i].GetSize();

		printf("\nCluster %i : ", mClusters[i].GetID());
		for (int j = 0; j < mDimension; j++)
			printf("%f ", mClusters[i].GetCenter(j));

		for (int j = 0; j < clusterSize; j++)
		{
			printf("\nPoint %i : ", mClusters[i].GetPoint(j).GetID());
			for (int p = 0; p < mDimension; p++)
				printf("%f ", mClusters[i].GetPoint(j).GetValue(p));
		}
	}
}