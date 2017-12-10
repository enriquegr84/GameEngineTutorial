// Copyright (C) 2002-2012 Nikolaus Gebhardt
// This file is part of the "Irrlicht Engine".
// For conditions of distribution and use, see copyright notice in irrlicht.h

#ifndef ANIMATEDMESH_H
#define ANIMATEDMESH_H

#include "Mesh.h"

//! Possible types of (animated) meshes.
enum AnimatedMeshType
{
	//! Unknown animated mesh type.
	AMT_UNKNOWN = 0,

	//! Quake 2 MD2 model file
	AMT_MD2,

	//! Quake 3 MD3 model file
	AMT_MD3,

	//! Maya .obj static model
	AMT_OBJ,

	//! Quake 3 .bsp static Map
	AMT_BSP,

	//! 3D Studio .3ds file
	AMT_3DS,

	//! My3D Mesh, the file format by Zhuck Dimitry
	AMT_MY3D,

	//! Pulsar LMTools .lmts file. This Irrlicht loader was written by Jonas Petersen
	AMT_LMTS,

	//! Cartography Shop .csm file. This loader was created by Saurav Mohapatra.
	AMT_CSM,

	//! .oct file for Paul Nette's FSRad or from Murphy McCauley's Blender .oct exporter.
	/** The oct file format contains 3D geometry and lightmaps and
	can be loaded directly by Irrlicht */
	AMT_OCT,

	//! Halflife MDL model file
	AMT_MDL_HALFLIFE,

	//! generic skinned mesh
	AMT_SKINNED
};

//! Interface for an animated mesh.
/** There are already simple implementations of this interface available so
you don't have to implement this interface on your own if you need to:
You might want to use AnimatedMesh, Mesh, MeshBuffer etc. */
class BaseAnimatedMesh : public BaseMesh
{
public:

	//! Gets the frame count of the animated mesh.
	/** \return The amount of frames. If the amount is 1,
	it is a static, non animated mesh. */
	virtual unsigned int GetFrameCount() const = 0;

	//! Gets the animation speed of the animated mesh.
	/** \return The number of frames per second to play the
	animation with by default. If the amount is 0,
	it is a static, non animated mesh. */
	virtual float GetAnimationSpeed() const = 0;

	//! Sets the animation speed of the animated mesh.
	/** \param fps Number of frames per second to play the
	animation with by default. If the amount is 0,
	it is not animated. The actual speed is set in the
	scene node the mesh is instantiated in.*/
	virtual void SetAnimationSpeed(float fps) = 0;

	//! Returns the IMesh interface for a frame.
	/** \param frame: Frame number as zero based index. The maximum
	frame number is getFrameCount() - 1;
	\param detailLevel: Level of detail. 0 is the lowest, 255 the
	highest level of detail. Most meshes will ignore the detail level.
	\param startFrameLoop: Because some animated meshes (.MD2) are
	blended between 2 static frames, and maybe animated in a loop,
	the startFrameLoop and the endFrameLoop have to be defined, to
	prevent the animation to be blended between frames which are
	outside of this loop.
	If startFrameLoop and endFrameLoop are both -1, they are ignored.
	\param endFrameLoop: see startFrameLoop.
	\return Returns the animated mesh based on a detail level. */
	virtual eastl::shared_ptr<Mesh> GetMesh(int frame, int detailLevel = 255,
		int startFrameLoop = -1, int endFrameLoop = -1) = 0;

	//! Returns the type of the animated mesh.
	/** In most cases it is not neccessary to use this method.
	This is useful for making a safe downcast. For example,
	if getMeshType() returns EAMT_MD2 it's safe to cast the
	IAnimatedMesh to IAnimatedMeshMD2.
	\returns Type of the mesh. */
	virtual AnimatedMeshType GetMeshType() const
	{
		return AMT_UNKNOWN;
	}
};



//! Simple implementation of the IAnimatedMesh interface.
struct AnimatedMesh : public BaseAnimatedMesh
{
	//! constructor
	AnimatedMesh(const eastl::shared_ptr<Mesh>& mesh=0, AnimatedMeshType type=AMT_UNKNOWN) : 
		BaseAnimatedMesh(), mFramesPerSecond(25.f), mType(type)
	{
		#ifdef _DEBUG
		//setDebugName("AnimatedMesh");
		#endif
		AddMesh(mesh);
		RecalculateBoundingBox();
	}

	//! destructor
	virtual ~AnimatedMesh()
	{
		// drop meshes
	}

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
	virtual eastl::shared_ptr<IMesh> GetMesh(int frame, int detailLevel=255, int startFrameLoop=-1, int endFrameLoop=-1)
	{
		if (mMeshes.empty())
			return 0;

		return mMeshes[frame];
	}

	//! adds a Mesh
	void AddMesh(eastl::shared_ptr<Mesh> mesh)
	{
		if (mesh)
			mMeshes.push_back(mesh);
	}

	//! Returns an axis aligned bounding box of the mesh.
	/** \return A bounding box of this mesh is returned. */
	virtual const AABBox3<float>& GetBoundingBox() const
	{
		return mBox;
	}

	//! set user axis aligned bounding box
	virtual void SetBoundingBox(const AABBox3f& box)
	{
		mBox = box;
	}

	//! Recalculates the bounding box.
	void RecalculateBoundingBox()
	{
		mBox.Reset(0,0,0);

		if (mMeshes.empty())
			return;

		mBox = mMeshes[0]->GetBoundingBox();

		for (unsigned int i=1; i<mMeshes.size(); ++i)
			mBox.AddInternalBox(mMeshes[i]->GetBoundingBox());
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

		return mMeshes[0]->GetMeshBufferCount();
	}

	//! returns pointer to a mesh buffer
	virtual eastl::shared_ptr<IMeshBuffer> GetMeshBuffer(unsigned int nr) const
	{
		if (mMeshes.empty())
			return 0;

		return mMeshes[0]->GetMeshBuffer(nr);
	}

	//! Returns pointer to a mesh buffer which fits a material
	/** \param material: material to search for
	\return Returns the pointer to the mesh buffer or
	NULL if there is no such mesh buffer. */
	virtual eastl::shared_ptr<IMeshBuffer> GetMeshBuffer( const Material &material) const
	{
		if (mMeshes.empty())
			return 0;

		return mMeshes[0]->GetMeshBuffer(material);
	}

	//! Set a material flag for all meshbuffers of this mesh.
	virtual void SetMaterialFlag(E_MATERIAL_FLAG flag, bool newvalue)
	{
		for (unsigned int i=0; i<mMeshes.size(); ++i)
			mMeshes[i]->SetMaterialFlag(flag, newvalue);
	}

	//! set the hardware mapping hint, for driver
	virtual void SetHardwareMappingHint( E_HARDWARE_MAPPING newMappingHint, E_BUFFER_TYPE buffer=EBT_VERTEX_AND_INDEX )
	{
		for (unsigned int i=0; i<mMeshes.size(); ++i)
			mMeshes[i]->SetHardwareMappingHint(newMappingHint, buffer);
	}

	//! flags the meshbuffer as changed, reloads hardware buffers
	virtual void SetDirty(E_BUFFER_TYPE buffer=EBT_VERTEX_AND_INDEX)
	{
		for (unsigned int i=0; i<mMeshes.size(); ++i)
			mMeshes[i]->SetDirty(buffer);
	}

	//! All meshes defining the animated mesh
	eastl::vector<eastl::shared_ptr<Mesh>> mMeshes;

	//! The bounding box of this mesh
	AABBox3<float> mBox;

	//! Default animation speed of this mesh.
	float mFramesPerSecond;

	//! The type of the mesh.
	AnimatedMeshType mType;
};

#endif

