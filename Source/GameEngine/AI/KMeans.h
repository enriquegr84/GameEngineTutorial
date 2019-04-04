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

#include "Pathing.h"

#include "Core/Logger/Logger.h"

class Point
{

public:

	Point()
	{
		mPointId = -1;
		mDimension = 0;
		mClusterId = -1;
	}

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

class PointArc
{

public:

	PointArc()
	{
		mArcId = -1;
		mArcType = 0;
	}

	PointArc(int arcId, int arcType)
	{
		mArcId = arcId;
		mArcType = arcType;
	}

	int GetId() { return mArcId; }
	int GetType() { return mArcType; }

private:
	int mArcId, mArcType;
};

class Clustering
{

public:
	Clustering(int clusterId, Point point)
	{
		mClusterId = clusterId;

		int dimension = point.GetDimension();
		for (int i = 0; i < dimension; i++)
			mCenter.push_back(point.GetValue(i));
		mCenterPoint = point;

		mPoints.push_back(point);
	}

	int GetId() { return mClusterId; }

	void AddPoint(Point point)  { mPoints.push_back(point);  }
	void AddIsolatedPoint(Point point) { mIsolatedPoints.push_back(point); }
	void ClearPoints() { mPoints.clear(); }
	void ClearIsolatedPoints() { mIsolatedPoints.clear(); }
	int GetNearestPointIndex(const eastl::vector<float>& point);
	Point GetPoint(int index) { return mPoints[index]; }
	int GetPointIndex(int pointId)
	{
		for (unsigned int i = 0; i < mPoints.size(); i++)
		{
			if (mPoints[i].GetId() == pointId)
				return i;
		}
		return -1;
	}
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
	bool RemoveIsolatedPoint(int pointId)
	{
		for (unsigned int i = 0; i < mIsolatedPoints.size(); i++)
		{
			if (mIsolatedPoints[i].GetId() == pointId)
			{
				mIsolatedPoints.erase(mIsolatedPoints.begin() + i);
				return true;
			}
		}
		return false;
	}

	float GetCenter(int index) { return mCenter[index]; }
	Point GetCenterPoint() { return mCenterPoint; }
	void SetCenter(int index, float value) { mCenter[index] = value; }
	void SetCenterPoint(Point point) { mCenterPoint = point; }
	void SetCenterPoint(int index) { mCenterPoint = mPoints[index]; }
	const eastl::vector<float>& GetCenterValue() { return mCenter; }
	int GetSize() { return mPoints.size(); }

	bool IsIsolatedPoint(Point point)
	{
		for (unsigned int i = 0; i < mIsolatedPoints.size(); i++)
		{
			if (mIsolatedPoints[i].GetId() == point.GetId())
				return true;
		}
		return false;
	}
	Point GetIsolatedPoint(int index) { return mIsolatedPoints[index]; }
	int GetIsolatedSize() { return mIsolatedPoints.size(); }

private:

	int mClusterId;
	Point mCenterPoint;
	eastl::vector<float> mCenter;
	eastl::vector<Point> mPoints;
	eastl::vector<Point> mIsolatedPoints;

};

class KMeans
{

public:

	KMeans(int K, int maxIterations, eastl::shared_ptr<PathingGraph> pPathingGraph)
	{
		mK = K;
		mIterations = maxIterations;
		mPathingGraph = pPathingGraph;
	}

	void Run(eastl::vector<Point> & points);

	bool IsArc(int arcId);
	const eastl::vector<PointArc>& GetArcs() { return mArcs; }
	const eastl::vector<Clustering>& GetClusters() { return mClusters; }

private:

	unsigned int FindClusterId(unsigned int pointId);
	int GetNearestClusterId(Point point); // return nearest point (uses euclidean distance)

	void AddArc(PointArc arc);
	PointArc GetArc(int index) { return mArcs[index]; }
	void ClearArcs() { mArcs.clear(); }

	int mK; // number of clusters
	int mIterations, mDimension, mTotalPoints;

	eastl::vector<PointArc> mArcs;
	eastl::vector<Clustering> mClusters;
	eastl::shared_ptr<PathingGraph> mPathingGraph;
};


#endif