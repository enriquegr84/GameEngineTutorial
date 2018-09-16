// Copyright (C) 2002-2012 Nikolaus Gebhardt
// This file is part of the "Irrlicht Engine".
// For conditions of distribution and use, see copyright notice in irrlicht.h

#ifndef STANDARDMESH_H
#define STANDARDMESH_H

#include "GameEngineStd.h"

#include "Graphic/Resource/Buffer/MeshBuffer.h"
#include "Graphic/Effect/Mesh.h"

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
		mMeshBuffer.reset();
	}


	//! returns amount of mesh buffers.
	virtual unsigned int GetMeshBufferCount() const
	{
		return 1;
	}

	//! Get pointer to a mesh buffer.
	virtual eastl::shared_ptr<MeshBuffer> GetMeshBuffer(unsigned int nr) const
	{
		return mMeshBuffer;
	}

	//! Get pointer to a mesh buffer which fits a material
	virtual eastl::shared_ptr<MeshBuffer> GetMeshBuffer(const Material &material) const
	{
		if (&material == mMeshBuffer->GetMaterial().get())
			mMeshBuffer;

		return nullptr;
	}

	//! The meshbuffer of this mesh
	eastl::shared_ptr<MeshBuffer> mMeshBuffer;
};

#endif

