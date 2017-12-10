// Copyright (C) 2002-2012 Nikolaus Gebhardt
// This file is part of the "Irrlicht Engine".
// For conditions of distribution and use, see copyright notice in irrlicht.h

#ifndef _IMESHBUFFER_H_INCLUDED_
#define _IMESHBUFFER_H_INCLUDED_

#include "Utilities/aabbox3.h"
#include "Utilities/VertexBuffer.h"
#include "Utilities/VertexIndex.h"

#include "Graphics/Material/Material.h"
#include "Graphics/EPrimitiveTypes.h"
#include "Graphics/EHardwareBufferFlags.h"


//! Struct for holding a mesh with a single material.
/** A part of an IMesh which has the same material on each face of that
group. Logical groups of an IMesh need not be put into separate mesh
buffers, but can be. Separately animated parts of the mesh must be put
into separate mesh buffers.
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
class IMeshBuffer
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
	virtual u32 GetVertexCount() const = 0;

	//! Get type of index data which is stored in this meshbuffer.
	/** \return Index type of this buffer. */
	virtual E_INDEX_TYPE GetIndexType() const =0;

	//! Get access to Indices.
	/** \return Pointer to indices array. */
	virtual const u16* GetIndices() const = 0;

	//! Get access to Indices.
	/** \return Pointer to indices array. */
	virtual u16* GetIndices() = 0;

	//! Get amount of indices in this meshbuffer.
	/** \return Number of indices in this buffer. */
	virtual u32 GetIndexCount() const = 0;

	//! Get the axis aligned bounding box of this meshbuffer.
	/** \return Axis aligned bounding box of this buffer. */
	virtual const AABBox3f& GetBoundingBox() const = 0;

	//! Set axis aligned bounding box
	/** \param box User defined axis aligned bounding box to use
	for this buffer. */
	virtual void SetBoundingBox(const AABBox3f& box) = 0;

	//! Recalculates the bounding box. Should be called if the mesh changed.
	virtual void RecalculateBoundingBox() = 0;

	//! returns position of vertex i
	virtual const Vector3<float>& GetPosition(u32 i) const = 0;

	//! returns position of vertex i
	virtual Vector3<float>& GetPosition(u32 i) = 0;

	//! returns normal of vertex i
	virtual const Vector3<float>& GetNormal(u32 i) const = 0;

	//! returns normal of vertex i
	virtual Vector3<float>& GetNormal(u32 i) = 0;

	//! returns texture coord of vertex i
	virtual const Vector2f& GetTCoords(u32 i) const = 0;

	//! returns texture coord of vertex i
	virtual Vector2f& GetTCoords(u32 i) = 0;

	//! Append the vertices and indices to the current buffer
	/** Only works for compatible vertex types.
	\param vertices Pointer to a vertex array.
	\param numVertices Number of vertices in the array.
	\param indices Pointer to index array.
	\param numIndices Number of indices in array. */
	virtual void Append(const void* const vertices, u32 numVertices, const u16* const indices, u32 numIndices) = 0;

	//! Append the meshbuffer to the current buffer
	/** Only works for compatible vertex types
	\param other Buffer to append to this one. */
	virtual void Append(const IMeshBuffer* const other) = 0;

	//! get the current hardware mapping hint
	virtual E_HARDWARE_MAPPING GetHardwareMappingHintVertex() const = 0;

	//! get the current hardware mapping hint
	virtual E_HARDWARE_MAPPING GetHardwareMappingHintIndex() const = 0;

	//! set the hardware mapping hint, for driver
	virtual void SetHardwareMappingHint( E_HARDWARE_MAPPING newMappingHint, E_BUFFER_TYPE buffer=EBT_VERTEX_AND_INDEX ) = 0;

	//! flags the meshbuffer as changed, reloads hardware buffers
	virtual void SetDirty(E_BUFFER_TYPE buffer=EBT_VERTEX_AND_INDEX) = 0;

	//! Get the currently used ID for identification of changes.
	/** This shouldn't be used for anything outside the VideoDriver. */
	virtual u32 GetChangedIDVertex() const = 0;

	//! Get the currently used ID for identification of changes.
	/** This shouldn't be used for anything outside the VideoDriver. */
	virtual u32 GetChangedIDIndex() const = 0;
};

#endif


