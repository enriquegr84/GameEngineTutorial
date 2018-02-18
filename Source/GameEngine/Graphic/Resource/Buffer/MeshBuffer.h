// Copyright (C) 2002-2012 Nikolaus Gebhardt
// This file is part of the "Irrlicht Engine".
// For conditions of distribution and use, see copyright notice in irrlicht.h

#ifndef MESHBUFFER_H
#define MESHBUFFER_H

#include "Buffer.h"
#include "IndexBuffer.h"
#include "VertexBuffer.h"

#include "Graphic/Effect/Material.h"
#include "Mathematic/Surface/Mesh.h"

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
MeshBuffer is a simple implementation of a MeshBuffer, which supports
up to 65535 vertices.

Since meshbuffers are used for drawing, and hence will be exposed
to the driver, chances are high that they are grab()'ed from somewhere.
It's therefore required to dynamically allocate meshbuffers which are
passed to a video driver and only drop the buffer once it's not used in
the current code block anymore.
*/

template <typename Real>
class GRAPHIC_ITEM MeshBuffer : public Buffer
{
public:

	MeshBuffer(unsigned int numVertices, unsigned int numIndices, 
		uint32_t numPrimitives, size_t meshSize, bool createStorage = false);
	~MeshBuffer();

	//! Get the material of this meshbuffer
	/** \return Material of this buffer. */
	const eastl::shared_ptr<Material>& GetMaterial() const
	{
		return mMaterial;
	}

	//! Get material of this meshbuffer
	/** \return Material of this buffer */
	eastl::shared_ptr<Material>& GetMaterial()
	{
		return mMaterial;
	}


	//! Material for this meshbuffer.
	eastl::shared_ptr<Material> mMaterial;

	//! mesh of this meshbuffer.
	eastl::shared_ptr<Mesh<Real>> mMesh;
};

#endif