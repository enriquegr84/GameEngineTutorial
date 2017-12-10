// Copyright (C) 2002-2012 Nikolaus Gebhardt
// This file is part of the "Irrlicht Engine".
// For conditions of distribution and use, see copyright notice in irrlicht.h

#ifndef _IMESHMANIPULATOR_H_INCLUDED_
#define _IMESHMANIPULATOR_H_INCLUDED_

#include "IMeshBuffer.h"
#include "IAnimatedMesh.h"

#include "Utilities/VertexManipulator.h"

//! An interface for easy manipulation of meshes.
/** Scale, set alpha value, flip surfaces, and so on. This exists for
fixing problems with wrong imported or exported meshes quickly after
loading. It is not intended for doing mesh modifications and/or
animations during runtime.
*/
class IMeshManipulator
{
public:

	//! Flips the direction of surfaces.
	/** Changes backfacing triangles to frontfacing
	triangles and vice versa.
	\param mesh Mesh on which the operation is performed. */
	virtual void FlipSurfaces(const IMesh* mesh) const = 0;

	//! Sets the alpha vertex color value of the whole mesh to a new value.
	/** \param mesh Mesh on which the operation is performed.
	\param alpha New alpha value. Must be a value between 0 and 255. */
	void SetVertexColorAlpha(IMesh* mesh, int alpha) const
	{
		Apply(VertexColorSetAlphaManipulator(alpha), mesh);
	}

	//! Sets the alpha vertex color value of the whole mesh to a new value.
	/** \param buffer Meshbuffer on which the operation is performed.
	\param alpha New alpha value. Must be a value between 0 and 255. */
	void SetVertexColorAlpha(IMeshBuffer* buffer, int alpha) const
	{
		Apply(VertexColorSetAlphaManipulator(alpha), buffer);
	}

	//! Sets the colors of all vertices to one color
	/** \param mesh Mesh on which the operation is performed.
	\param color New color. */
	void SetVertexColors(IMesh* mesh, Color color) const
	{
		Apply(VertexColorSetManipulator(color), mesh);
	}

	//! Sets the colors of all vertices to one color
	/** \param buffer Meshbuffer on which the operation is performed.
	\param color New color. */
	void SetVertexColors(IMeshBuffer* buffer, Color color) const
	{
		Apply(VertexColorSetManipulator(color), buffer);
	}

	//! Recalculates all normals of the mesh.
	/** \param mesh: Mesh on which the operation is performed.
	\param smooth: If the normals shall be smoothed.
	\param angleWeighted: If the normals shall be smoothed in relation to their angles. 
	More expensive, but also higher precision. */
	virtual void RecalculateNormals(IMesh* mesh, 
		bool smooth = false, bool angleWeighted = false) const=0;

	//! Recalculates all normals of the mesh buffer.
	/** \param buffer: Mesh buffer on which the operation is performed.
	\param smooth: If the normals shall be smoothed.
	\param angleWeighted: If the normals shall be smoothed in relation to their angles. 
	More expensive, but also higher precision. */
	virtual void RecalculateNormals(IMeshBuffer* buffer, 
		bool smooth = false, bool angleWeighted = false) const=0;

	//! Recalculates tangents, requires a tangent mesh
	/** \param mesh Mesh on which the operation is performed.
	\param recalculateNormals If the normals shall be recalculated, 
	otherwise original normals of the mesh are used unchanged.
	\param smooth If the normals shall be smoothed.
	\param angleWeighted If the normals shall be smoothed in relation to their angles.
	More expensive, but also higher precision. */
	virtual void RecalculateTangents(IMesh* mesh, bool recalculateNormals=false, 
		bool smooth=false, bool angleWeighted=false) const=0;

	//! Recalculates tangents, requires a tangent mesh buffer
	/** \param buffer Meshbuffer on which the operation is performed.
	\param recalculateNormals If the normals shall be recalculated, 
	otherwise original normals of the buffer are used unchanged.
	\param smooth If the normals shall be smoothed.
	\param angleWeighted If the normals shall be smoothed in relation to their angles. 
	More expensive, but also higher precision. */
	virtual void RecalculateTangents(IMeshBuffer* buffer, 
		bool recalculateNormals=false, bool smooth=false, bool angleWeighted=false) const=0;

	//! Scales the actual mesh, not a scene node.
	/** \param mesh Mesh on which the operation is performed.
	\param factor Scale factor for each axis. */
	void Scale(IMesh* mesh, const Vector3<float>& factor) const
	{
		Apply(VertexPositionScaleManipulator(factor), mesh, true);
	}

	//! Scales the actual meshbuffer, not a scene node.
	/** \param buffer Meshbuffer on which the operation is performed.
	\param factor Scale factor for each axis. */
	void Scale(IMeshBuffer* buffer, const Vector3<float>& factor) const
	{
		Apply(VertexPositionScaleManipulator(factor), buffer, true);
	}

	//! Scale the texture coords of a mesh.
	/** \param mesh Mesh on which the operation is performed.
	\param factor Vector which defines the scale for each axis.
	\param level Number of texture coord, starting from 1. 
	Support for level 2 exists for LightMap buffers. */
	void ScaleTCoords(IMesh* mesh, const Vector2f& factor, u32 level=1) const
	{
		Apply(VertexTCoordsScaleManipulator(factor, level), mesh);
	}

	//! Scale the texture coords of a meshbuffer.
	/** \param buffer Meshbuffer on which the operation is performed.
	\param factor Vector which defines the scale for each axis.
	\param level Number of texture coord, starting from 1. 
	Support for level 2 exists for LightMap buffers. */
	void ScaleTCoords(IMeshBuffer* buffer, const Vector2f& factor, u32 level=1) const
	{
		Apply(VertexTCoordsScaleManipulator(factor, level), buffer);
	}

	//! Applies a transformation to a mesh
	/** \param mesh Mesh on which the operation is performed.
	\param m transformation matrix. */
	void Transform(IMesh* mesh, const matrix4& m) const
	{
		Apply(VertexPositionTransformManipulator(m), mesh, true);
	}

	//! Applies a transformation to a meshbuffer
	/** \param buffer Meshbuffer on which the operation is performed.
	\param m transformation matrix. */
	void Transform(IMeshBuffer* buffer, const matrix4& m) const
	{
		Apply(VertexPositionTransformManipulator(m), buffer, true);
	}

	//! Creates a planar texture mapping on the mesh
	/** \param mesh: Mesh on which the operation is performed.
	\param resolution: resolution of the planar mapping. This is
	the value specifying which is the relation between world space
	and texture coordinate space. */
	virtual void MakePlanarTextureMapping(IMesh* mesh, f32 resolution=0.001f) const=0;

	//! Creates a planar texture mapping on the meshbuffer
	/** \param meshbuffer: Buffer on which the operation is performed.
	\param resolution: resolution of the planar mapping. This is
	the value specifying which is the relation between world space
	and texture coordinate space. */
	virtual void MakePlanarTextureMapping(IMeshBuffer* meshbuffer, f32 resolution=0.001f) const=0;

	//! Creates a planar texture mapping on the buffer
	/** This method is currently implemented towards the LWO planar mapping. 
	A more general biasing might be required.
	\param mesh Mesh on which the operation is performed.
	\param resolutionS Resolution of the planar mapping in horizontal direction. 
	This is the ratio between object space and texture space.
	\param resolutionT Resolution of the planar mapping in vertical direction. 
	This is the ratio between object space and texture space.
	\param axis The axis along which the texture is projected. 
	The allowed values are 0 (X), 1(Y), and 2(Z).
	\param offset Vector added to the vertex positions (in object coordinates).
	*/
	virtual void MakePlanarTextureMapping(IMesh* mesh, f32 resolutionS, f32 resolutionT, u8 axis, const Vector3<float>& offset) const=0;

	//! Creates a planar texture mapping on the meshbuffer
	/** This method is currently implemented towards the LWO planar mapping. 
	A more general biasing might be required.
	\param buffer Buffer on which the operation is performed.
	\param resolutionS Resolution of the planar mapping in horizontal direction. 
	This is the ratio between object space and texture space.
	\param resolutionT Resolution of the planar mapping in vertical direction. 
	This is the ratio between object space and texture space.
	\param axis The axis along which the texture is projected. 
	The allowed values are 0 (X), 1(Y), and 2(Z).
	\param offset Vector added to the vertex positions (in object coordinates).
	*/
	virtual void MakePlanarTextureMapping(IMeshBuffer* buffer, 
		f32 resolutionS, f32 resolutionT, u8 axis, const Vector3<float>& offset) const=0;

	//! Clones a static IMesh into a modifiable SMesh.
	/** All meshbuffers in the returned SMesh
	are of type SMeshBuffer or SMeshBufferLightMap.
	\param mesh Mesh to copy.
	\return Cloned mesh. If you no longer need the
	cloned mesh, you should call SMesh::drop(). See
	IReferenceCounted::drop() for more information. */
	virtual IMesh* CreateMeshCopy(const IMesh* mesh) const = 0;

	//! Creates a copy of the mesh, which will only consist of VertexTangents vertices.
	/** This is useful if you want to draw tangent space normal
	mapped geometry because it calculates the tangent and binormal
	data which is needed there.
	\param mesh Input mesh
	\param recalculateNormals The normals are recalculated if set,
	otherwise the original ones are kept. Note that keeping the
	normals may introduce inaccurate tangents if the normals are
	very different to those calculated from the faces.
	\param smooth The normals/tangents are smoothed across the
	meshbuffer's faces if this flag is set.
	\param angleWeighted Improved smoothing calculation used
	\param recalculateTangents Whether are actually calculated, 
	or just the mesh with proper type is created.
	\return Mesh consisting only of VertexTangents vertices. If
	you no longer need the cloned mesh, you should call
	IMesh::drop(). See IReferenceCounted::drop() for more
	information. */
	virtual IMesh* CreateMeshWithTangents(const IMesh* mesh, bool recalculateNormals=false, 
		bool smooth=false, bool angleWeighted=false, bool recalculateTangents=true) const=0;

	//! Creates a copy of the mesh, which will only consist of VertexBuffer2TCoord vertices.
	/** \param mesh Input mesh
	\return Mesh consisting only of VertexBuffer2TCoord vertices. If
	you no longer need the cloned mesh, you should call
	IMesh::drop(). See IReferenceCounted::drop() for more
	information. */
	virtual IMesh* CreateMeshWith2TCoords(const IMesh* mesh) const = 0;

	//! Creates a copy of the mesh, which will only consist of VertexBuffer vertices.
	/** \param mesh Input mesh
	\return Mesh consisting only of VertexBuffer vertices. If
	you no longer need the cloned mesh, you should call
	IMesh::drop(). See IReferenceCounted::drop() for more
	information. */
	virtual IMesh* CreateMeshWith1TCoords(const IMesh* mesh) const = 0;

	//! Creates a copy of a mesh with all vertices unwelded
	/** \param mesh Input mesh
	\return Mesh consisting only of unique faces. All vertices
	which were previously shared are now duplicated. If you no
	longer need the cloned mesh, you should call IMesh::drop(). See
	IReferenceCounted::drop() for more information. */
	virtual IMesh* CreateMeshUniquePrimitives(const IMesh* mesh) const = 0;

	//! Creates a copy of a mesh with vertices welded
	/** \param mesh Input mesh
	\param tolerance The threshold for vertex comparisons.
	\return Mesh without redundant vertices. If you no longer need
	the cloned mesh, you should call IMesh::drop(). See
	IReferenceCounted::drop() for more information. */
	virtual IMesh* CreateMeshWelded(const IMesh* mesh, f32 tolerance=ROUNDING_ERROR_f32) const = 0;

	//! Get amount of polygons in mesh.
	/** \param mesh Input mesh
	\return Number of polygons in mesh. */
	virtual int GetPolyCount(IMesh* mesh) const = 0;

	//! Get amount of polygons in mesh.
	/** \param mesh Input mesh
	\return Number of polygons in mesh. */
	virtual int GetPolyCount(IAnimatedMesh* mesh) const = 0;

	//! Create a new AnimatedMesh and adds the mesh to it
	/** \param mesh Input mesh
	\param type The type of the animated mesh to create.
	\return Newly created animated mesh with mesh as its only
	content. When you don't need the animated mesh anymore, you
	should call IAnimatedMesh::drop(). See
	IReferenceCounted::drop() for more information. */
	virtual IAnimatedMesh* CreateAnimatedMesh(const shared_ptr<IMesh>& mesh, 
		E_ANIMATED_MESH_TYPE type = EAMT_UNKNOWN) const = 0;

	//! Vertex cache optimization according to the Forsyth paper
	/** More information can be found at
	http://home.comcast.net/~tom_forsyth/papers/fast_vert_cache_opt.html

	The function is thread-safe (read: you can optimize several
	meshes in different threads).

	\param mesh Source mesh for the operation.
	\return A new mesh optimized for the vertex cache. */
	virtual IMesh* CreateForsythOptimizedMesh(const IMesh* mesh) const = 0;

	//! Apply a manipulator on the Meshbuffer
	/** \param func A functor defining the mesh manipulation.
	\param buffer The Meshbuffer to apply the manipulator to.
	\param boundingBoxUpdate Specifies if the bounding box should 
	be updated during manipulation.
	\return True if the functor was successfully applied, else false. */
	template <typename Functor>
	bool Apply(const Functor& func, IMeshBuffer* buffer, bool boundingBoxUpdate=false) const
	{
		return apply_(func, buffer, boundingBoxUpdate, func);
	}


	//! Apply a manipulator on the Mesh
	/** \param func A functor defining the mesh manipulation.
	\param mesh The Mesh to apply the manipulator to.
	\param boundingBoxUpdate Specifies if the bounding box should be updated during manipulation.
	\return True if the functor was successfully applied, else false. */
	template <typename Functor>
	bool Apply(const Functor& func, IMesh* mesh, bool boundingBoxUpdate=false) const
	{
		if (!mesh)
			return true;
		bool result = true;
		AABBox3f bufferbox;
		for (u32 i=0; i<mesh->GetMeshBufferCount(); ++i)
		{
			result &= Apply(func, mesh->GetMeshBuffer(i).get(), boundingBoxUpdate);
			if (boundingBoxUpdate)
			{
				if (0==i)
					bufferbox.Reset(mesh->GetMeshBuffer(i)->GetBoundingBox());
				else
					bufferbox.AddInternalBox(mesh->GetMeshBuffer(i)->GetBoundingBox());
			}
		}
		if (boundingBoxUpdate)
			mesh->SetBoundingBox(bufferbox);
		return result;
	}

protected:
	//! Apply a manipulator based on the type of the functor
	/** \param func A functor defining the mesh manipulation.
	\param buffer The Meshbuffer to apply the manipulator to.
	\param boundingBoxUpdate Specifies if the bounding box should 
	be updated during manipulation.
	\param typeTest Unused parameter, which handles the proper call 
	selection based on the type of the Functor which is passed in two times.
	\return True if the functor was successfully applied, else false. */
	template <typename Functor>
	bool apply_(const Functor& func, IMeshBuffer* buffer, 
		bool boundingBoxUpdate, const IVertexManipulator& typeTest) const
	{
		if (!buffer)
			return true;

		AABBox3f bufferbox;
		for (u32 i=0; i<buffer->GetVertexCount(); ++i)
		{
			switch (buffer->GetVertexType())
			{
			case EVT_STANDARD:
				{
					VertexBuffer* verts = (VertexBuffer*)buffer->GetVertices();
					func(verts[i]);
				}
				break;
			case EVT_2TCOORDS:
				{
					Vertex2TCoords* verts = (Vertex2TCoords*)buffer->GetVertices();
					func(verts[i]);
				}
				break;
			case EVT_TANGENTS:
				{
					VertexTangents* verts = (VertexTangents*)buffer->GetVertices();
					func(verts[i]);
				}
				break;
			}
			if (boundingBoxUpdate)
			{
				if (0==i)
					bufferbox.Reset(buffer->GetPosition(0));
				else
					bufferbox.AddInternalPoint(buffer->GetPosition(i));
			}
		}
		if (boundingBoxUpdate)
			buffer->SetBoundingBox(bufferbox);
		return true;
	}
};

#endif
