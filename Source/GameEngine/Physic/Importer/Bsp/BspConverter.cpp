/*
Bullet Continuous Collision Detection and Physics Library
Copyright (c) 2003-2006 Erwin Coumans  http://continuousphysics.com/Bullet/

This software is provided 'as-is', without any express or implied warranty.
In no event will the authors be held liable for any damages arising from the use of this software.
Permission is granted to anyone to use this software for any purpose, 
including commercial applications, and to alter it and redistribute it freely, 
subject to the following restrictions:

1. The origin of this software must not be misrepresented; you must not claim that you wrote the original software. If you use this software in a product, an acknowledgment in the product documentation would be appreciated but is not required.
2. Altered source versions must be plainly marked as such, and must not be misrepresented as being the original software.
3. This notice may not be removed or altered from any source distribution.
*/

#include "BspConverter.h"

#include "Core/Logger/Logger.h"

#include "LinearMath/btVector3.h"
#include "LinearMath/btGeometryUtil.h"
#include <stdio.h>
#include <string.h>

// ------------------------------------------------------------------------------------------------
//  Creates the curved surface bezier from a face array.
void BspConverter::CreateCurvedSurfaceBezier(BspLoader& bspLoader, BSPSurface* surface)
{
	unsigned int j, k;

	// number of control points across & up
	const unsigned int controlWidth = surface->patchWidth;
	const unsigned int controlHeight = surface->patchHeight;

	if (0 == controlWidth || 0 == controlHeight)
		return;

	// number of biquadratic patches
	const unsigned int biquadWidth = (controlWidth - 1) / 2;
	const unsigned int biquadHeight = (controlHeight - 1) / 2;

	// Create space for a temporary array of the patch's control points
	eastl::vector<S3DVertex2TCoords> controlPoint(controlWidth * controlHeight);
	for (j = 0; j < controlPoint.size(); ++j)
		copy(&controlPoint[j], &bspLoader.mDrawVertices[surface->firstVert + j]);

	// create a temporary patch
	SBezier bezier;

	//Loop through the biquadratic patches
	int tessellation = 8;
	for (j = 0; j < biquadHeight; ++j)
	{
		for (k = 0; k < biquadWidth; ++k)
		{
			// set up this patch
			const int inx = j * controlWidth * 2 + k * 2;

			// setup bezier control points for this patch
			bezier.control[0] = controlPoint[inx + 0];
			bezier.control[1] = controlPoint[inx + 1];
			bezier.control[2] = controlPoint[inx + 2];
			bezier.control[3] = controlPoint[inx + controlWidth + 0];
			bezier.control[4] = controlPoint[inx + controlWidth + 1];
			bezier.control[5] = controlPoint[inx + controlWidth + 2];
			bezier.control[6] = controlPoint[inx + controlWidth * 2 + 0];
			bezier.control[7] = controlPoint[inx + controlWidth * 2 + 1];
			bezier.control[8] = controlPoint[inx + controlWidth * 2 + 2];

			bezier.tesselate(tessellation);
		}
	}

	btAlignedObjectArray<btVector3>	vertices;
	for (size_t i = 0; i < bezier.vertices.size(); i++)
	{
		S3DVertex2TCoords pVertex = bezier.vertices[i];
		vertices.push_back(btVector3(
			pVertex.vPosition.x, pVertex.vPosition.y, pVertex.vPosition.z));
	}
	AddConvexVerticesCollider(vertices);

	/*
	btAlignedObjectArray<btScalar> vertices;
	for (size_t i = 0; i < bezier.vertices.size(); i++)
	{
		S3DVertex2TCoords pVertex = bezier.vertices[i];
		vertices.push_back(pVertex.vPosition.x);
		vertices.push_back(pVertex.vPosition.y);
		vertices.push_back(pVertex.vPosition.z);
	}
	btAlignedObjectArray<int> indices;
	for (size_t i = 0; i < bezier.indices.size(); i++)
		indices.push_back(bezier.indices[i]);

	AddTriangleMeshCollider(vertices, indices);
	*/
}

void BspConverter::ConvertBsp(BspLoader& bspLoader, float scaling)
{
	bspLoader.ParseEntities();

	//progressBegin("Loading bsp");

	for (int i = 0; i < bspLoader.mNumDrawSurfaces; i++)
	{
		char info[128];
		sprintf(info, "Reading bspSurface %i from total %i (%f percent)\n",
			i, bspLoader.mNumDrawSurfaces, (100.f*(float)i / float(bspLoader.mNumDrawSurfaces)));
		printf(info);

		BSPSurface& surface = bspLoader.mDrawSurfaces[i];
		if (surface.surfaceType == MST_PATCH)
		{
			eastl::string shader = bspLoader.mDShaders[surface.shaderNum].shader;

			if (shader.find("base_floor") == eastl::string::npos &&
				shader.find("gothic_floor") == eastl::string::npos &&
				shader.find("gothic_block") == eastl::string::npos)
				continue;

			int tesselation = 8;
			CreateCurvedSurfaceBezier(bspLoader, &surface);
		}
	}

	for (int i=0;i<bspLoader.mNumLeafs;i++)
	{
		char info[128];
		sprintf(info, "Reading bspLeaf %i from total %i (%f percent)\n",
			i, bspLoader.mNumLeafs, (100.f*(float)i / float(bspLoader.mNumLeafs)));
		printf(info);
			
		bool isValidBrush = false;
			
		BSPLeaf& leaf = bspLoader.mDLeafs[i];
		for (int b=0;b<leaf.numLeafBrushes;b++)
		{
			btAlignedObjectArray<btVector3> planeEquations;
				
			int brushid = bspLoader.mDLeafBrushes[leaf.firstLeafBrush+b];

			BSPBrush& brush = bspLoader.mDBrushes[brushid];
			if (brush.shaderNum!=-1)
			{
				if (bspLoader.mDShaders[ brush.shaderNum ].contentFlags & BSPCONTENTS_SOLID)
				{
					brush.shaderNum = -1;

					for (int p=0;p<brush.numSides;p++)
					{
						int sideid = brush.firstSide+p;
						BSPBrushSide& brushside = bspLoader.mDBrushsides[sideid];
						int planeid = brushside.planeNum;
						BSPPlane& plane = bspLoader.mDPlanes[planeid];
						btVector3 planeEq;
						planeEq.setValue(
							plane.normal[0],
							plane.normal[1],
							plane.normal[2]);
						planeEq[3] = scaling*-plane.dist;

						planeEquations.push_back(planeEq);
						isValidBrush=true;
					}
					if (isValidBrush)
					{
						btAlignedObjectArray<btVector3>	vertices;
						btGeometryUtil::getVerticesFromPlaneEquations(planeEquations,vertices);

						AddConvexVerticesCollider(vertices);
					}
				}
			} 
		}
	}

	//progressEnd();
}