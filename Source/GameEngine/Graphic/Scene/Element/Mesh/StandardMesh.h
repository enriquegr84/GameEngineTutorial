// Copyright (C) 2002-2012 Nikolaus Gebhardt
// This file is part of the "Irrlicht Engine".
// For conditions of distribution and use, see copyright notice in irrlicht.h

#ifndef STANDARDMESH_H
#define STANDARDMESH_H

#include "GameEngineStd.h"

#include "Graphic/Resource/Buffer/MeshBuffer.h"
#include "Graphic/Scene/Element/Mesh/Mesh.h"

//! Simple implementation of the BaseMesh interface.
class StandardMesh : public BaseMesh
{
public:
	//! constructor
	StandardMesh()
	{
	}

	//! destructor
	virtual ~StandardMesh()
	{
	}

	//! clean mesh
	virtual void Clear()
	{
		mMeshBuffers.clear();
	}


	//! returns amount of mesh buffers.
	virtual unsigned int GetMeshBufferCount() const
	{
		return mMeshBuffers.size();
	}

	//! Get pointer to a mesh buffer.
	virtual eastl::shared_ptr<BaseMeshBuffer> GetMeshBuffer(unsigned int nr) const
	{
		if (mMeshBuffers.empty())
			return nullptr;

		if (nr < 0 || nr >= mMeshBuffers.size())
			return nullptr;

		return mMeshBuffers[nr];
	}

	//! Get pointer to a mesh buffer which fits a material
	virtual eastl::shared_ptr<BaseMeshBuffer> GetMeshBuffer(const Material &material) const
	{
		for (auto meshBuffer : mMeshBuffers)
		{
			if (&material == meshBuffer->GetMaterial().get())
				meshBuffer;
		}

		return nullptr;
	}

	//! Returns the type of the animated mesh.
	virtual MeshType GetMeshType() const
	{
		return MT_STANDARD;
	}

	//! Adds a new meshbuffer to the mesh, access it as last one
	virtual void AddMeshBuffer(BaseMeshBuffer* meshBuffer) 
	{
		mMeshBuffers.push_back(eastl::shared_ptr<MeshBuffer>((MeshBuffer*)meshBuffer));
	}

protected:

	//! The meshbuffer of this mesh
	eastl::vector<eastl::shared_ptr<MeshBuffer>> mMeshBuffers;

};

#endif

