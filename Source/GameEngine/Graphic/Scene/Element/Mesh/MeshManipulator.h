// Copyright (C) 2002-2012 Nikolaus Gebhardt
// This file is part of the "Irrlicht Engine".
// For conditions of distribution and use, see copyright notice in irrlicht.h

#ifndef _MESHMANIPULATOR_H_INCLUDED_
#define _MESHMANIPULATOR_H_INCLUDED_

#include "IMeshManipulator.h"

//! An interface for easy manipulation of meshes.
/** Scale, set alpha value, flip surfaces, and so on. This exists for fixing
problems with wrong imported or exported meshes quickly after loading. It is
not intended for doing mesh modifications and/or animations during runtime.
*/
class MeshManipulator : public IMeshManipulator
{
public:
	//! Flips the direction of surfaces.
	/** Changes backfacing triangles to frontfacing triangles and vice versa.
	\param mesh: Mesh on which the operation is performed. */
	virtual void FlipSurfaces(const IMesh* mesh) const;

	//! Recalculates all normals of the mesh.
	/** \param mesh: Mesh on which the operation is performed.
	\param smooth: Whether to use smoothed normals. */
	virtual void RecalculateNormals(IMesh* mesh, 
		bool smooth = false, bool angleWeighted = false) const;

	//! Recalculates all normals of the mesh buffer.
	/** \param buffer: Mesh buffer on which the operation is performed.
	\param smooth: Whether to use smoothed normals. */
	virtual void RecalculateNormals(IMeshBuffer* buffer, 
		bool smooth = false, bool angleWeighted = false) const;

	//! Clones a static IMesh into a modifiable SMesh.
	virtual IMesh* CreateMeshCopy(const IMesh* mesh) const;

	//! Creates a planar texture mapping on the mesh
	/** \param mesh: Mesh on which the operation is performed.
	\param resolution: resolution of the planar mapping. This is the value
	specifying which is the relation between world space and
	texture coordinate space. */
	virtual void MakePlanarTextureMapping(IMesh* mesh, float resolution=0.001f) const;

	//! Creates a planar texture mapping on the meshbuffer
	virtual void MakePlanarTextureMapping(IMeshBuffer* meshbuffer, float resolution=0.001f) const;

	//! Creates a planar texture mapping on the meshbuffer
	void MakePlanarTextureMapping(IMeshBuffer* buffer, float resolutionS, float resolutionT, 
		u8 axis, const Vector3<float>& offset) const;

	//! Creates a planar texture mapping on the mesh
	void MakePlanarTextureMapping(IMesh* mesh, float resolutionS, float resolutionT, 
		u8 axis, const Vector3<float>& offset) const;

	//! Recalculates tangents, requires a tangent mesh buffer
	virtual void RecalculateTangents(IMeshBuffer* buffer, bool recalculateNormals=false,
		bool smooth=false, bool angleWeighted=false) const;

	//! Recalculates tangents, requires a tangent mesh
	virtual void RecalculateTangents(IMesh* mesh, bool recalculateNormals=false, 
		bool smooth=false, bool angleWeighted=false) const;

	//! Creates a copy of the mesh, which will only consist of VertexTangents vertices.
	virtual IMesh* CreateMeshWithTangents(const IMesh* mesh, bool recalculateNormals=false, 
		bool smooth=false, bool angleWeighted=false, bool recalculateTangents=true) const;

	//! Creates a copy of the mesh, which will only consist of S3D2TCoords vertices.
	virtual IMesh* CreateMeshWith2TCoords(const IMesh* mesh) const;

	//! Creates a copy of the mesh, which will only consist of VertexBuffer vertices.
	virtual IMesh* CreateMeshWith1TCoords(const IMesh* mesh) const;

	//! Creates a copy of the mesh, which will only consist of unique triangles, i.e. no vertices are shared.
	virtual IMesh* CreateMeshUniquePrimitives(const IMesh* mesh) const;

	//! Creates a copy of the mesh, which will have all duplicated vertices removed, i.e. maximal amount 
	//	of vertices are shared via indexing.
	virtual IMesh* CreateMeshWelded(const IMesh* mesh, float tolerance=ROUNDING_ERROR_float) const;

	//! Returns amount of polygons in mesh.
	virtual int GetPolyCount(IMesh* mesh) const;

	//! Returns amount of polygons in mesh.
	virtual int GetPolyCount(IAnimatedMesh* mesh) const;

	//! create a new AnimatedMesh and adds the mesh to it
	virtual IAnimatedMesh* CreateAnimatedMesh(const shared_ptr<IMesh>& mesh, E_ANIMATED_MESH_TYPE type) const;

	//! create a mesh optimized for the vertex cache
	virtual IMesh* CreateForsythOptimizedMesh(const IMesh* mesh) const;
};


#endif
