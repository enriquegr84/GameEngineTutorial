// Copyright (C) 2002-2012 Nikolaus Gebhardt
// This file is part of the "Irrlicht Engine".
// For conditions of distribution and use, see copyright notice in irrlicht.h

#ifndef ANIMATEDMESH_H
#define ANIMATEDMESH_H

#include "Graphic/Effect/Mesh.h"

//! Simple implementation of the IAnimatedMesh interface.
class AnimatedMesh : public BaseAnimatedMesh
{

public:

	//! constructor
	AnimatedMesh(const eastl::shared_ptr<BaseMesh>& mesh=0, AnimatedMeshType type=AMT_UNKNOWN) : 
		BaseAnimatedMesh(), mFramesPerSecond(25.f), mType(type)
	{
		AddMesh(mesh);
	}

	//! destructor
	virtual ~AnimatedMesh()
	{
		// drop meshes
	}
	
	//! Gets the frame of the animated mesh.
	/** \return frames. If the amount is 1, it is a static, non animated mesh. */
	/*
	virtual const eastl::vector<eastl::shared_ptr<BaseMesh>> GetFrames() const
	{
		return mMeshes;
	}
	*/
	//! Gets the frame count of the animated mesh.
	/** \return Amount of frames. If the amount is 1, it is a static, non animated mesh. */
	virtual unsigned int GetFrameCount() const
	{
		return mMeshes.size();
	}

	//! Gets the default animation speed of the animated mesh.
	/** \return Amount of frames per second. If the amount is 0, it is a static, non animated mesh. */
	virtual float GetAnimationSpeed() const
	{
		return mFramesPerSecond;
	}

	//! Gets the frame count of the animated mesh.
	/** \param fps Frames per second to play the animation with. If the amount is 0, it is not animated.
	The actual speed is set in the scene node the mesh is instantiated in.*/
	virtual void SetAnimationSpeed(float fps)
	{
		mFramesPerSecond=fps;
	}

	//! Returns the IMesh interface for a frame.
	/** \param frame: Frame number as zero based index. The maximum frame number is
	getFrameCount() - 1;
	\param detailLevel: Level of detail. 0 is the lowest,
	255 the highest level of detail. Most meshes will ignore the detail level.
	\param startFrameLoop: start frame
	\param endFrameLoop: end frame
	\return The animated mesh based on a detail level. */
	virtual eastl::shared_ptr<BaseMesh> GetMesh(int frame, int detailLevel=255, int startFrameLoop=-1, int endFrameLoop=-1)
	{
		if (mMeshes.empty())
			return nullptr;

		return mMeshes[frame];
	}

	//! adds a Mesh
	void AddMesh(eastl::shared_ptr<BaseMesh> mesh)
	{
		if (mesh)
			mMeshes.push_back(mesh);
	}

	//! Returns the type of the animated mesh.
	virtual AnimatedMeshType GetMeshType() const
	{
		return mType;
	}

	//! returns amount of mesh buffers.
	virtual unsigned int GetMeshBufferCount() const
	{
		if (mMeshes.empty())
			return 0;

		return mMeshes.size();
	}

	//! returns pointer to a mesh buffer
	virtual eastl::shared_ptr<MeshBuffer> GetMeshBuffer(unsigned int nr) const
	{
		if (mMeshes.empty())
			return nullptr;

		if (nr < 0 || nr >= mMeshes.size())
			return nullptr;

		return mMeshes[nr]->GetMeshBuffer(0);
	}

	//! Returns pointer to a mesh buffer which fits a material
	/** \param material: material to search for
	\return Returns the pointer to the mesh buffer or
	NULL if there is no such mesh buffer. */
	virtual eastl::shared_ptr<MeshBuffer> GetMeshBuffer( const Material &material) const
	{
		for (auto mesh : mMeshes)
		{
			if (mesh->GetMeshBuffer(material) != nullptr)
				return mesh->GetMeshBuffer(material);
		}

		return nullptr;
	}

protected:

	//! All meshes defining the animated mesh
	eastl::vector<eastl::shared_ptr<BaseMesh>> mMeshes;

	//! Default animation speed of this mesh.
	float mFramesPerSecond;

	//! The type of the mesh.
	AnimatedMeshType mType;
};

#endif

