// Copyright (C) 2002-2012 Nikolaus Gebhardt
// This file is part of the "Irrlicht Engine".
// For conditions of distribution and use, see copyright notice in irrlicht.h

#ifndef MESHBUFFER_H
#define MESHBUFFER_H

/*
#include "Utilities/aabbox3.h"
#include "Utilities/VertexBuffer.h"
#include "Utilities/VertexIndex.h"

#include "Graphics/Material/Material.h"
#include "Graphics/EPrimitiveTypes.h"
#include "Graphics/EHardwareBufferFlags.h"
*/


//! Struct for holding a mesh with a single material.
/** A part of an BaseMesh which has the same material on each face of 
that group. Logical groups of an IMesh need not be put into separate 
mesh buffers, but can be. Separately animated parts of the mesh must 
be put into separate mesh buffers.
Some mesh buffer implementations have limitations on the number of
vertices the buffer can hold. In that case, logical grouping can help.
Moreover, the number of vertices should be optimized for the GPU upload,
which often depends on the type of gfx card. Typial figures are
1000-10000 vertices per buffer.
SMeshBuffer is a simple implementation of a MeshBuffer, which supports
up to 65535 vertices.

Since meshbuffers are used for drawing, and hence will be exposed
to the driver, chances are high that they are grab()'ed from somewhere.
It's therefore required to dynamically allocate meshbuffers which are
passed to a video driver and only drop the buffer once it's not used in
the current code block anymore.
*/
class BaseMeshBuffer
{
public:

	//! Get the material of this meshbuffer
	/** \return Material of this buffer. */
	virtual Material& GetMaterial() = 0;

	//! Get the material of this meshbuffer
	/** \return Material of this buffer. */
	virtual const Material& GetMaterial() const = 0;

	//! Get type of vertex data which is stored in this meshbuffer.
	/** \return Vertex type of this buffer. */
	virtual E_VERTEX_TYPE GetVertexType() const = 0;

	//! Get access to vertex data. The data is an array of vertices.
	/** Which vertex type is used can be determined by getVertexType().
	\return Pointer to array of vertices. */
	virtual const void* GetVertices() const = 0;

	//! Get access to vertex data. The data is an array of vertices.
	/** Which vertex type is used can be determined by getVertexType().
	\return Pointer to array of vertices. */
	virtual void* GetVertices() = 0;

	//! Get amount of vertices in meshbuffer.
	/** \return Number of vertices in this buffer. */
	virtual unsigned int GetVertexCount() const = 0;

	//! Get type of index data which is stored in this meshbuffer.
	/** \return Index type of this buffer. */
	virtual E_INDEX_TYPE GetIndexType() const = 0;

	//! Get access to Indices.
	/** \return Pointer to indices array. */
	virtual const unsigned int* GetIndices() const = 0;

	//! Get access to Indices.
	/** \return Pointer to indices array. */
	virtual unsigned int* GetIndices() = 0;

	//! Get amount of indices in this meshbuffer.
	/** \return Number of indices in this buffer. */
	virtual unsigned int GetIndexCount() const = 0;

	//! Get the axis aligned bounding box of this meshbuffer.
	/** \return Axis aligned bounding box of this buffer. */
	virtual const AABBox3<float>& GetBoundingBox() const = 0;

	//! Set axis aligned bounding box
	/** \param box User defined axis aligned bounding box to use
	for this buffer. */
	virtual void SetBoundingBox(const AABBox3<float>& box) = 0;

	//! Recalculates the bounding box. Should be called if the mesh changed.
	virtual void RecalculateBoundingBox() = 0;

	//! returns position of vertex i
	virtual const Vector3<float>& GetPosition(unsigned int i) const = 0;

	//! returns position of vertex i
	virtual Vector3<float>& GetPosition(unsigned int i) = 0;

	//! returns normal of vertex i
	virtual const Vector3<float>& GetNormal(unsigned int i) const = 0;

	//! returns normal of vertex i
	virtual Vector3<float>& GetNormal(unsigned int i) = 0;

	//! returns texture coord of vertex i
	virtual const Vector2<float>& GetTCoords(unsigned int i) const = 0;

	//! returns texture coord of vertex i
	virtual Vector2<float>& GetTCoords(unsigned int i) = 0;

	//! Append the vertices and indices to the current buffer
	/** Only works for compatible vertex types.
	\param vertices Pointer to a vertex array.
	\param numVertices Number of vertices in the array.
	\param indices Pointer to index array.
	\param numIndices Number of indices in array. */
	virtual void Append(const void* const vertices, unsigned int numVertices, 
		const unsigned int* const indices, unsigned int numIndices) = 0;

	//! Append the meshbuffer to the current buffer
	/** Only works for compatible vertex types
	\param other Buffer to append to this one. */
	virtual void Append(const BaseMeshBuffer* const other) = 0;

	//! get the current hardware mapping hint
	virtual E_HARDWARE_MAPPING GetHardwareMappingHintVertex() const = 0;

	//! get the current hardware mapping hint
	virtual E_HARDWARE_MAPPING GetHardwareMappingHintIndex() const = 0;

	//! set the hardware mapping hint, for driver
	virtual void SetHardwareMappingHint(E_HARDWARE_MAPPING newMappingHint, E_BUFFER_TYPE buffer = EBT_VERTEX_AND_INDEX) = 0;

	//! flags the meshbuffer as changed, reloads hardware buffers
	virtual void SetDirty(E_BUFFER_TYPE buffer = EBT_VERTEX_AND_INDEX) = 0;

	//! Get the currently used ID for identification of changes.
	/** This shouldn't be used for anything outside the VideoDriver. */
	virtual unsigned int GetChangedIDVertex() const = 0;

	//! Get the currently used ID for identification of changes.
	/** This shouldn't be used for anything outside the VideoDriver. */
	virtual unsigned int GetChangedIDIndex() const = 0;
};

//! Template implementation of the IMeshBuffer interface
template <class T>
class MeshBuffer : public BaseMeshBuffer
{
public:
	//! Default constructor for empty meshbuffer
	MeshBuffer() : mChangedIDVertex(1), mChangedIDIndex(1), 
		mMappingHintVertex(EHM_NEVER), mMappingHintIndex(EHM_NEVER)
	{
		#ifdef _DEBUG
		//setDebugName("MeshBuffer");
		#endif
	}


	//! Get material of this meshbuffer
	/** \return Material of this buffer */
	virtual const Material& GetMaterial() const
	{
		return mMaterial;
	}


	//! Get material of this meshbuffer
	/** \return Material of this buffer */
	virtual Material& GetMaterial()
	{
		return mMaterial;
	}


	//! Get pointer to vertices
	/** \return Pointer to vertices. */
	virtual const void* GetVertices() const
	{
		return mVertices.data();
	}


	//! Get pointer to vertices
	/** \return Pointer to vertices. */
	virtual void* GetVertices()
	{
		return mVertices.data();
	}


	//! Get number of vertices
	/** \return Number of vertices. */
	virtual unsigned int GetVertexCount() const
	{
		return mVertices.size();
	}

	//! Get type of index data which is stored in this meshbuffer.
	/** \return Index type of this buffer. */
	virtual E_INDEX_TYPE GetIndexType() const
	{
		return EIT_16BIT;
	}

	//! Get pointer to indices
	/** \return Pointer to indices. */
	virtual const u16* GetIndices() const
	{
		return mIndices.data();
	}


	//! Get pointer to indices
	/** \return Pointer to indices. */
	virtual u16* GetIndices()
	{
		return mIndices.data();
	}


	//! Get number of indices
	/** \return Number of indices. */
	virtual unsigned int GetIndexCount() const
	{
		return mIndices.size();
	}


	//! Get the axis aligned bounding box
	/** \return Axis aligned bounding box of this buffer. */
	virtual const AABBox3<float>& GetBoundingBox() const
	{
		return mBoundingBox;
	}


	//! Set the axis aligned bounding box
	/** \param box New axis aligned bounding box for this buffer. */
	//! set user axis aligned bounding box
	virtual void SetBoundingBox(const AABBox3f& box)
	{
		mBoundingBox = box;
	}


	//! Recalculate the bounding box.
	/** should be called if the mesh changed. */
	virtual void RecalculateBoundingBox()
	{
		if (!mVertices.empty())
		{
			mBoundingBox.Reset(mVertices[0].mPos);
			for (unsigned int i=1; i<mVertices.size(); ++i)
				mBoundingBox.AddInternalPoint(mVertices[i].mPos);
		}
		else mBoundingBox.Reset(0, 0, 0);
	}


	//! Get type of vertex data stored in this buffer.
	/** \return Type of vertex data. */
	virtual E_VERTEX_TYPE GetVertexType() const
	{
		return T().GetType();
	}

	//! returns position of vertex i
	virtual const Vector3<float>& GetPosition(unsigned int i) const
	{
		return mVertices[i].mPos;
	}

	//! returns position of vertex i
	virtual Vector3<float>& GetPosition(unsigned int i)
	{
		return mVertices[i].mPos;
	}

	//! returns normal of vertex i
	virtual const Vector3<float>& GetNormal(unsigned int i) const
	{
		return mVertices[i].mNormal;
	}

	//! returns normal of vertex i
	virtual Vector3<float>& GetNormal(unsigned int i)
	{
		return mVertices[i].mNormal;
	}

	//! returns texture coord of vertex i
	virtual const Vector2<float>& GetTCoords(unsigned int i) const
	{
		return mVertices[i].mTCoords;
	}

	//! returns texture coord of vertex i
	virtual Vector2<float>& GetTCoords(unsigned int i)
	{
		return mVertices[i].mTCoords;
	}


	//! Append the vertices and indices to the current buffer
	/** Only works for compatible types, i.e. either the same type
	or the main buffer is of standard type. Otherwise, behavior is
	undefined.
	*/
	virtual void Append(const void* const vertices, unsigned int numVertices, 
		const unsigned int* const indices, unsigned int numIndices)
	{
		if (vertices == GetVertices())
			return;

		const unsigned int vertexCount = GetVertexCount();
		unsigned int i;

		for (i=0; i<numVertices; ++i)
		{
			mVertices.push_back(reinterpret_cast<const T*>(vertices)[i]);
			mBoundingBox.AddInternalPoint(reinterpret_cast<const T*>(vertices)[i].mPos);
		}

		for (i=0; i<numIndices; ++i)
		{
			mIndices.push_back(indices[i]+vertexCount);
		}
	}


	//! Append the meshbuffer to the current buffer
	/** Only works for compatible types, i.e. either the same type
	or the main buffer is of standard type. Otherwise, behavior is
	undefined.
	\param other Meshbuffer to be appended to this one.
	*/
	virtual void Append(const BaseMeshBuffer* const other)
	{
		/*
		if (this==other)
			return;

		const unsigned int vertexCount = GetVertexCount();
		unsigned int i;

		for (i=0; i<other->getVertexCount(); ++i)
			mVertices.push_back(reinterpret_cast<const T*>(other->getVertices())[i]);

		for (i=0; i<other->getIndexCount(); ++i)
			mIndices.push_back(other->getIndices()[i]+vertexCount);

		mBoundingBox.addInternalBox(other->VGetBoundingBox());
		*/
	}


	//! get the current hardware mapping hint
	virtual E_HARDWARE_MAPPING GetHardwareMappingHintVertex() const
	{
		return mMappingHintVertex;
	}

	//! get the current hardware mapping hint
	virtual E_HARDWARE_MAPPING GetHardwareMappingHintIndex() const
	{
		return mMappingHintIndex;
	}

	//! set the hardware mapping hint, for driver
	virtual void SetHardwareMappingHint( E_HARDWARE_MAPPING newMappingHint, E_BUFFER_TYPE Buffer=EBT_VERTEX_AND_INDEX )
	{
		if (Buffer==EBT_VERTEX_AND_INDEX || Buffer==EBT_VERTEX)
			mMappingHintVertex=newMappingHint;
		if (Buffer==EBT_VERTEX_AND_INDEX || Buffer==EBT_INDEX)
			mMappingHintIndex=newMappingHint;
	}


	//! flags the mesh as changed, reloads hardware buffers
	virtual void SetDirty(E_BUFFER_TYPE Buffer=EBT_VERTEX_AND_INDEX)
	{
		if (Buffer==EBT_VERTEX_AND_INDEX || Buffer==EBT_VERTEX)
			++mChangedIDVertex;
		if (Buffer==EBT_VERTEX_AND_INDEX || Buffer==EBT_INDEX)
			++mChangedIDIndex;
	}

	//! Get the currently used ID for identification of changes.
	/** This shouldn't be used for anything outside the VideoDriver. */
	virtual unsigned int GetChangedIDVertex() const {return mChangedIDVertex;}

	//! Get the currently used ID for identification of changes.
	/** This shouldn't be used for anything outside the VideoDriver. */
	virtual unsigned int GetChangedIDIndex() const {return mChangedIDIndex;}

	unsigned int mChangedIDVertex;
	unsigned int mChangedIDIndex;

	//! hardware mapping hint
	E_HARDWARE_MAPPING mMappingHintVertex;
	E_HARDWARE_MAPPING mMappingHintIndex;

	//! Material for this meshbuffer.
	Material mMaterial;
	//! Vertices of this buffer
	eastl::vector<T> mVertices;
	//! Indices into the vertices of this buffer.
	eastl::vector<unsigned int> mIndices;
	//! Bounding box of this meshbuffer.
	AABBox3<float> mBoundingBox;
};

//! Standard meshbuffer
typedef MeshBuffer<VertexBuffer> MeshBufferVertex;
//! Meshbuffer with two texture coords per vertex, e.g. for lightmaps
typedef MeshBuffer<Vertex2TCoords> MeshBufferLightMap;
//! Meshbuffer with vertices having tangents stored, e.g. for normal mapping
typedef MeshBuffer<VertexTangents> MeshBufferTangents;


#endif


