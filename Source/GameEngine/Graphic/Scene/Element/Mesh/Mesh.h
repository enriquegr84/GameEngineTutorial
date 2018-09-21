// Copyright (C) 2002-2012 Nikolaus Gebhardt
// This file is part of the "Irrlicht Engine".
// For conditions of distribution and use, see copyright notice in irrlicht.h

#ifndef BASEMESH_H
#define BASEMESH_H

#include "GameEngineStd.h"

#include "Graphic/Effect/Material.h"
#include "Graphic/Resource/Buffer/MeshBuffer.h"

#include "Mathematic/Geometric/AlignedBox.h"

//! Possible types of (animated) meshes.
enum GRAPHIC_ITEM AnimatedMeshType
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

//! Class which holds the geometry of an object.
/** An IMesh is nothing more than a collection of some mesh buffers
(MeshBuffer). SMesh is a simple implementation of an IMesh.
A mesh is usually added to an IMeshSceneNode in order to be rendered. */
class BaseMesh : public eastl::enable_shared_from_this<BaseMesh>
{
public:

	//! Get the amount of mesh buffers.
	/** \return Amount of mesh buffers (IMeshBuffer) in this mesh. */
	virtual unsigned int GetMeshBufferCount() const = 0;

	//! Get pointer to a mesh buffer.
	/** \param nr: Zero based index of the mesh buffer. The maximum value is
	GetMeshBufferCount() - 1;
	\return Pointer to the mesh buffer or 0 if there is no such
	mesh buffer. */
	virtual eastl::shared_ptr<MeshBuffer> GetMeshBuffer(unsigned int nr) const = 0;

	//! Get pointer to a mesh buffer which fits a material
	/** \param material: material to search for
	\return Pointer to the mesh buffer or 0 if there is no such
	mesh buffer. */
	virtual eastl::shared_ptr<MeshBuffer> GetMeshBuffer(const Material &material) const = 0;

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
	virtual eastl::shared_ptr<BaseMesh> GetMesh(int frame, int detailLevel = 255,
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

#endif