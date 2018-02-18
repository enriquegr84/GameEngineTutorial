// Copyright (C) 2002-2012 Nikolaus Gebhardt
// This file is part of the "Irrlicht Engine".
// For conditions of distribution and use, see copyright notice in irrlicht.h

#ifndef CAMERANODE_H
#define CAMERANODE_H

#include "Graphic/Scene/Hierarchy/Node.h"
#include "Graphic/Scene/Hierarchy/ViewVolume.h"

#include "Mathematic/Geometric/Frustum.h"


////////////////////////////////////////////////////
//
// class CameraNode					- Chapter 16, page 548
//
//    A camera node controls the D3D view transform and holds the view
//    frustum definition
//
////////////////////////////////////////////////////

class CameraNode : public Node
{
public:

	//! constructor
	CameraNode(const ActorId actorid);

	//! Sets a custom view matrix affector.
	/** \param affector: The affector matrix. */
	void SetViewMatrixAffector(const Matrix4x4<float>& affector);

	//! Gets the custom view matrix affector.
	const Matrix4x4<float>& GetViewMatrixAffector() const;

	//! It is possible to send mouse and key events to the camera. Most cameras
	//! may ignore this input, but camera scene nodes which are created for
	//! example with ISceneManager::addMayaCameraSceneNode or
	//! ISceneManager::addMeshViewerCameraSceneNode, may want to get this input
	//! for changing their position, look at target or whatever.
	//virtual bool OnEvent(const Event& event);

	//! Sets the look at target of the camera
	/** If the camera's target and rotation are bound ( @see BindTargetAndRotation() )
	then calling this will also change the camera's scene node rotation to match the target.
	\param pos: Look at target of the camera. */
	void SetTarget(const eastl::shared_ptr<Node>& target) { mTarget = target; }

	//! Gets the current look at target of the camera
	/** \return The current look at target of the camera */
	eastl::shared_ptr<Node> GetTarget() { return mTarget; }

	//! Gets the current camera
	/** \return The current camera */
	eastl::shared_ptr<Camera> Get() { return mCamera; }

	void ClearTarget() { mTarget = 0; }

	//! PreRender event
	bool PreRender(Scene *pScene);
	bool Render(Scene *pScene);

	//! Update
	void UpdateMatrices();

	//! Returns if the input receiver of the camera is currently enabled.
	//bool IsInputReceiverEnabled() const;

protected:

	eastl::shared_ptr<Node>		mTarget;
	eastl::shared_ptr<Camera>	mCamera;

	//bool mInputReceiverEnabled;
	//bool mTargetAndRotationAreBound;

	Matrix4x4<float>	mAffector;

	bool			mActive;
	bool			mDebugCamera;
};

#endif

