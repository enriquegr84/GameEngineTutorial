// Copyright (C) 2002-2012 Nikolaus Gebhardt
// This file is part of the "Irrlicht Engine".
// For conditions of distribution and use, see copyright notice in irrlicht.h

#ifndef MESH_H
#define MESH_H

#include "GameEngineStd.h"

#include "MeshBuffer.h"

//#include "Graphics/EHardwareBufferFlags.h"
#include "Graphic/Effect/Material.h"

class BaseMeshBuffer;

//! Class which holds the geometry of an object.
/** An IMesh is nothing more than a collection of some mesh buffers
(IMeshBuffer). SMesh is a simple implementation of an IMesh.
A mesh is usually added to an IMeshSceneNode in order to be rendered. */
class BaseMesh : public std::enable_shared_from_this<BaseMesh>
{
public:

	//! Get the amount of mesh buffers.
	/** \return Amount of mesh buffers (IMeshBuffer) in this mesh. */
	virtual unsigned int GetMeshBufferCount() const = 0;

	//! Get pointer to a mesh buffer.
	/** \param nr: Zero based index of the mesh buffer. The maximum value is
	getMeshBufferCount() - 1;
	\return Pointer to the mesh buffer or 0 if there is no such
	mesh buffer. */
	virtual eastl::shared_ptr<BaseMeshBuffer> GetMeshBuffer(u32 nr) const = 0;

	//! Get pointer to a mesh buffer which fits a material
	/** \param material: material to search for
	\return Pointer to the mesh buffer or 0 if there is no such
	mesh buffer. */
	virtual eastl::shared_ptr<BaseMeshBuffer> GetMeshBuffer(const Material &material) const = 0;

	//! Get an axis aligned bounding box of the mesh.
	/** \return Bounding box of this mesh. */
	virtual const AABBox3<float>& GetBoundingBox() const = 0;

	//! Set user-defined axis aligned bounding box
	/** \param box New bounding box to use for the mesh. */
	virtual void SetBoundingBox(const AABBox3f& box) = 0;

	//! Sets a flag of all contained materials to a new value.
	/** \param flag: Flag to set in all materials.
	\param newvalue: New value to set in all materials. */
	virtual void SetMaterialFlag(E_MATERIAL_FLAG flag, bool newvalue) = 0;

	//! Set the hardware mapping hint
	/** This methods allows to define optimization hints for the
	hardware. This enables, e.g., the use of hardware buffers on
	pltforms that support this feature. This can lead to noticeable
	performance gains. */
	virtual void SetHardwareMappingHint(E_HARDWARE_MAPPING newMappingHint, E_BUFFER_TYPE buffer = EBT_VERTEX_AND_INDEX) = 0;

	//! Flag the meshbuffer as changed, reloads hardware buffers
	/** This method has to be called every time the vertices or
	indices have changed. Otherwise, changes won't be updated
	on the GPU in the next render cycle. */
	virtual void SetDirty(E_BUFFER_TYPE buffer = EBT_VERTEX_AND_INDEX) = 0;
};

//! Simple implementation of the BaseMesh interface.
class Mesh : public BaseMesh
{
public:
	//! constructor
	Mesh()
	{
#ifdef _DEBUG
		//setDebugName("SMesh");
#endif
	}

	//! destructor
	virtual ~Mesh()
	{
	}

	//! clean mesh
	virtual void Clear()
	{
		mMeshBuffers.clear();
		mBoundingBox.Reset(0.f, 0.f, 0.f);
	}


	//! returns amount of mesh buffers.
	virtual unsigned int GetMeshBufferCount() const
	{
		return mMeshBuffers.size();
	}

	//! Get pointer to a mesh buffer.
	virtual eastl::shared_ptr<BaseMeshBuffer> GetMeshBuffer(unsigned int nr) const
	{
		return m_MeshBuffers[nr];
	}

	//! Get pointer to a mesh buffer which fits a material
	virtual eastl::shared_ptr<BaseMeshBuffer> GetMeshBuffer(const Material &material) const
	{
		for (int i = (int)mMeshBuffers.size() - 1; i >= 0; --i)
		{
			if (material == mMeshBuffers[i]->GetMaterial())
				return mMeshBuffers[i];
		}

		return NULL;
	}

	//! returns an axis aligned bounding box
	virtual const AABBox3<float>& GetBoundingBox() const
	{
		return mBoundingBox;
	}

	//! set user axis aligned bounding box
	virtual void SetBoundingBox(const AABBox3f& box)
	{
		mBoundingBox = box;
	}

	//! recalculates the bounding box
	void RecalculateBoundingBox()
	{
		if (mMeshBuffers.size())
		{
			mBoundingBox = mMeshBuffers[0]->GetBoundingBox();
			for (unsigned int i = 1; i< mMeshBuffers.size(); ++i)
				mBoundingBox.AddInternalBox(mMeshBuffers[i]->GetBoundingBox());
		}
		else mBoundingBox.Reset(0.0f, 0.0f, 0.0f);
	}

	//! adds a MeshBuffer
	/** The bounding box is not updated automatically. */
	void AddMeshBuffer(const eastl::shared_ptr<BaseMeshBuffer>& buf)
	{
		if (buf)
			mMeshBuffers.push_back(buf);
	}

	//! sets a flag of all contained materials to a new value
	virtual void SetMaterialFlag(E_MATERIAL_FLAG flag, bool newvalue)
	{
		for (u32 i = 0; i< mMeshBuffers.size(); ++i)
			mMeshBuffers[i]->GetMaterial().SetFlag(flag, newvalue);
	}

	//! set the hardware mapping hint, for driver
	virtual void SetHardwareMappingHint(E_HARDWARE_MAPPING newMappingHint, E_BUFFER_TYPE buffer = EBT_VERTEX_AND_INDEX)
	{
		for (u32 i = 0; i< mMeshBuffers.size(); ++i)
			mMeshBuffers[i]->SetHardwareMappingHint(newMappingHint, buffer);
	}

	//! flags the meshbuffer as changed, reloads hardware buffers
	virtual void SetDirty(E_BUFFER_TYPE buffer = EBT_VERTEX_AND_INDEX)
	{
		for (u32 i = 0; i< mMeshBuffers.size(); ++i)
			mMeshBuffers[i]->SetDirty(buffer);
	}

	//! The meshbuffers of this mesh
	eastl::vector<shared_ptr<BaseMeshBuffer>> mMeshBuffers;

	//! The bounding box of this mesh
	AABBox3<float> mBoundingBox;
};

#endif

