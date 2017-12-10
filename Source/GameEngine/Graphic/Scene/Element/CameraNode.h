// Copyright (C) 2002-2012 Nikolaus Gebhardt
// This file is part of the "Irrlicht Engine".
// For conditions of distribution and use, see copyright notice in irrlicht.h

#ifndef CAMERANODE_H
#define CAMERANODE_H

#include "Graphic/Scene/Hierarchy/Node.h"

#include "Mathematic/ViewFrustum.h"


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
	CameraNode(ActorId actorid, Scene* pScene);

	
	/*
	void SetActiveCameraOffset( const Vector4<float> & cameraOffset ) { mCamOffsetVector = cameraOffset; }
	*/

	//! Sets the projection matrix of the camera.
	/** The matrix4 class has some methods
	to build a projection matrix. e.g: matrix4::buildProjectionMatrixPerspectiveFovLH.
	Note that the matrix will only stay as set by this method until one of
	the following Methods are called: setNearValue, setFarValue, setAspectRatio, setFOV.
	\param projection The new projection matrix of the camera.
	\param isOrthogonal Set this to true if the matrix is an orthogonal one (e.g.
	from matrix4::buildProjectionMatrixOrthoLH(). */
	void SetProjectionMatrix(const Matrix4x4& projection, bool isOrthogonal = false);

	//! Gets the current projection matrix of the camera
	//! \return Returns the current projection matrix of the camera.
	const Matrix4x4& GetProjectionMatrix() const;

	//! Gets the current view matrix of the camera
	//! \return Returns the current view matrix of the camera.
	const Matrix4x4& GetViewMatrix() const;

	//! Sets a custom view matrix affector.
	/** \param affector: The affector matrix. */
	void SetViewMatrixAffector(const Matrix4x4& affector);

	//! Gets the custom view matrix affector.
	const Matrix4x4& GetViewMatrixAffector() const;

	//! It is possible to send mouse and key events to the camera. Most cameras
	//! may ignore this input, but camera scene nodes which are created for
	//! example with ISceneManager::addMayaCameraSceneNode or
	//! ISceneManager::addMeshViewerCameraSceneNode, may want to get this input
	//! for changing their position, look at target or whatever.
	//virtual bool OnEvent(const Event& event);

	//! Checks if a camera is orthogonal.
	bool IsOrthogonal() const { return mIsOrthogonal; }

	//! Sets the look at target of the camera
	/** If the camera's target and rotation are bound ( @see BindTargetAndRotation() )
	then calling this will also change the camera's scene node rotation to match the target.
	\param pos: Look at target of the camera. */
	void SetTarget(const eastl::shared_ptr<SceneNode>& target) { mTarget = target; }

	//! Gets the current look at target of the camera
	/** \return The current look at target of the camera */
	eastl::shared_ptr<SceneNode> GetTarget() { return mTarget; }

	void ClearTarget() { mTarget = 0; }

	//! Sets the up vector of the camera.
	//! \param pos: New upvector of the camera.
	void SetUpVector(const Vector3<float>& pos);

	//! Gets the up vector of the camera.
	//! \return Returns the up vector of the camera.
	const Vector3<float>& GetUpVector() const;

	//! Gets distance from the camera to the near plane.
	//! \return Value of the near plane of the camera.
	float GetNearValue() const;

	//! Gets the distance from the camera to the far plane.
	//! \return Value of the far plane of the camera.
	float GetFarValue() const;

	//! Get the aspect ratio of the camera.
	//! \return The aspect ratio of the camera.
	float GetAspectRatio() const;

	//! Gets the field of view of the camera.
	//! \return Field of view of the camera
	float GetFOV() const;

	//! Sets the value of the near clipping plane. (default: 1.0f)
	void SetNearValue(float zn);

	//! Sets the value of the far clipping plane (default: 2000.0f)
	void SetFarValue(float zf);

	//! Sets the aspect ratio (default: 4.0f / 3.0f)
	void SetAspectRatio(float aspect);

	//! Sets the field of view (Default: PI / 3.5f)
	void SetFOV(float fovy);

	//! PreRender event
	bool PreRender(Scene *pScene);
	bool Render(Scene *pScene);

	Matrix4x4 GetWorldViewProjection(Scene *pScene);
	bool SetViewTransform(Scene *pScene);

	//! Update
	void UpdateMatrices();

	//! Returns the view area. Sometimes needed by bsp or lod render nodes.
	const ViewFrustum& GetViewFrustum() const;

	const AABBox3<float>& GetBoundingBox() const;

	//! Returns if the input receiver of the camera is currently enabled.
	//bool IsInputReceiverEnabled() const;

protected:

	void RecalculateProjectionMatrix();
	void RecalculateViewArea();

	eastl::shared_ptr<Node> mTarget;
	Vector3<float> UpVector;

	float Fovy;	// Field of view, in radians.
	float Aspect;	// Aspect ratio.
	float ZNear;	// value of the near view-plane.
	float ZFar;	// Z-value of the far view-plane.

	ViewFrustum ViewArea;
	Matrix4x4 Affector;

	//bool InputReceiverEnabled;
	//bool TargetAndRotationAreBound;

	ViewFrustum		mFrustum;
	Matrix4x4		mProjection;
	Matrix4x4		mView;
	bool			mbActive;
	bool			mDebugCamera;
	//Vector4<float>mCamOffsetVector;	//Direction of camera relative to target.

	bool mIsOrthogonal;
};

#endif

