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
#ifdef _DEBUG
		//setDebugName("StandardMesh");
#endif
	}

	//! destructor
	virtual ~StandardMesh()
	{
	}

	//! clean mesh
	virtual void Clear()
	{
		mMeshBuffers.clear();
		//mBoundingBox.Reset(0.f, 0.f, 0.f);
	}


	//! returns amount of mesh buffers.
	virtual unsigned int GetMeshBufferCount() const
	{
		return mMeshBuffers.size();
	}

	//! Get pointer to a mesh buffer.
	virtual eastl::shared_ptr<MeshBuffer> GetMeshBuffer(unsigned int nr) const
	{
		return mMeshBuffers[nr];
	}

	//! Get pointer to a mesh buffer which fits a material
	virtual eastl::shared_ptr<MeshBuffer> GetMeshBuffer(const Material &material) const
	{
		for (int i = (int)mMeshBuffers.size() - 1; i >= 0; --i)
		{
			if (&material == mMeshBuffers[i]->GetMaterial().get())
				return mMeshBuffers[i];
		}

		return NULL;
	}

	//! returns an axis aligned bounding box
	virtual const AlignedBox3<float>& GetBoundingBox() const
	{
		return mBoundingBox;
	}

	//! set user axis aligned bounding box
	virtual void SetBoundingBox(const AlignedBox3<float>& box)
	{
		mBoundingBox = box;
	}

	//! recalculates the bounding box
	void RecalculateBoundingBox()
	{
		/*
		if (mMeshBuffers.size())
		{
			mBoundingBox = mMeshBuffers[0]->GetBoundingBox();
			for (unsigned int i = 1; i < mMeshBuffers.size(); ++i)
			{
				mBoundingBox.AddInternalBox(mMeshBuffers[i]->GetBoundingBox());
			}
		}
		else mBoundingBox.Reset(0.0f, 0.0f, 0.0f);
		*/
	}

	//! adds a MeshBuffer
	/** The bounding box is not updated automatically. */
	void AddMeshBuffer(const eastl::shared_ptr<MeshBuffer>& buf)
	{
		if (buf)
			mMeshBuffers.push_back(buf);
	}

	//! The meshbuffers of this mesh
	eastl::vector<eastl::shared_ptr<MeshBuffer>> mMeshBuffers;

	//! The bounding box of this mesh
	AlignedBox3<float> mBoundingBox;
};

#endif

