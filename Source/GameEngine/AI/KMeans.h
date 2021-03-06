//========================================================================
// KMeans.h : Implements a clustering algorithm
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

#ifndef KMEANS_H
#define KMEANS_H

#include "GameEngineStd.h"

#include "Core/Logger/Logger.h"

class Point
{

public:
	Point(int pointId, eastl::vector<float>& values)
	{
		mPointId = pointId;
		mDimension = values.size();

		for (int i = 0; i < mDimension; i++)
			mValues.push_back(values[i]);

		mClusterId = -1;
	}

	int GetId() { return mPointId; }
	int GetDimension() { return mDimension; }
	int GetCluster() { return mClusterId; }
	void SetCluster(int clusterId) { mClusterId = clusterId; }
	float GetValue(int index) { return mValues[index]; }
	void AddValue(float value) { mValues.push_back(value); }

private:
	int mDimension;
	int mPointId, mClusterId;
	eastl::vector<float> mValues;
};

class Clustering
{

public:
	Clustering(int clusterId, Point point)
	{
		mClusterId = clusterId;

		int dimension = point.GetDimension();

		for (int i = 0; i < dimension; i++)
			mCenters.push_back(point.GetValue(i));

		mPoints.push_back(point);
	}

	int GetId() { return mClusterId; }

	void AddPoint(Point point) { mPoints.push_back(point); }
	bool RemovePoint(int pointId)
	{
		for (unsigned int i = 0; i < mPoints.size(); i++)
		{
			if (mPoints[i].GetId() == pointId)
			{
				mPoints.erase(mPoints.begin() + i);
				return true;
			}
		}
		return false;
	}

	float GetCenter(int index) { return mCenters[index]; }
	void SetCenter(int index, float value) { mCenters[index] = value; }

	Point GetPoint(int index) { return mPoints[index]; }
	int GetSize() { return mPoints.size(); }

private:
	int mClusterId;
	eastl::vector<float> mCenters;
	eastl::vector<Point> mPoints;

};

class KMeans
{

public:

	KMeans(int K, int maxIterations)
	{
		mK = K;
		mIterations = maxIterations;
	}

	void Run(eastl::vector<Point> & points);

	const eastl::vector<Clustering>& GetClusters() { return mClusters; }

private:

	int mK; // number of clusters
	int mIterations, mDimension, mTotalPoints;
	eastl::vector<Clustering> mClusters;

	// return nearest cluster Id (uses euclidean distance)
	int GetNearestClusterId(Point point);

};

#endif