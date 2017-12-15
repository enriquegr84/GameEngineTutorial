// Copyright (C) 2002-2012 Nikolaus Gebhardt
// This file is part of the "Irrlicht Engine".
// For conditions of distribution and use, see copyright notice in irrlicht.h

#include "CameraNode.h"

#include "Graphic/Scene/Scene.h"

//! constructor
CameraNode::CameraNode(ActorId actorid, Scene* pScene)
	: Node(actorid, WeakBaseRenderComponentPtr(), ERP_NONE, ESNT_CAMERA),
	UpVector(0.0f, 1.0f, 0.0f), ZNear(1.0f), ZFar(3000.0f), mActive(true), mTarget(0), mIsOrthogonal(false)
{
	#ifdef _DEBUG
	//setDebugName("CameraSceneNode");
	#endif

	// set default projection
	Fovy = PI / 2.5f;	// Field of view, in radians.

	const eastl::shared_ptr<Renderer>& renderer = pScene? pScene->GetRenderer() : 0;
	if (renderer)
	{
		Aspect = (float)renderer->GetCurrentRenderTargetSize().Width /
			(float)renderer->GetCurrentRenderTargetSize().Height;
	}
	else Aspect = 4.0f / 3.0f;	// Aspect ratio.

	RecalculateProjectionMatrix();
	RecalculateViewArea();
}


//! Sets the projection matrix of the camera.
/** The matrix4 class has some methods
to build a projection matrix. e.g: matrix4::buildProjectionMatrixPerspectiveFovLH
\param projection: The new projection matrix of the camera. */
void CameraNode::SetProjectionMatrix(const Matrix4x4<float>& projection, bool isOrthogonal)
{
	mIsOrthogonal = isOrthogonal;
	ViewArea.GetTransform ( ETS_PROJECTION ) = projection;
}


//! Gets the current projection matrix of the camera
//! \return Returns the current projection matrix of the camera.
const Matrix4x4<float>& CameraNode::GetProjectionMatrix() const
{
	return ViewArea.GetTransform ( ETS_PROJECTION );
}


//! Gets the current view matrix of the camera
//! \return Returns the current view matrix of the camera.
const Matrix4x4<float>& CameraNode::GetViewMatrix() const
{
	return ViewArea.GetTransform ( ETS_VIEW );
}


//! Sets a custom view matrix affector. The matrix passed here, will be
//! multiplied with the view matrix when it gets updated.
//! This allows for custom camera setups like, for example, a reflection camera.
/** \param affector: The affector matrix. */
void CameraNode::SetViewMatrixAffector(const Matrix4x4<float>& affector)
{
	Affector = affector;
}


//! Gets the custom view matrix affector.
const Matrix4x4<float>& CameraNode::GetViewMatrixAffector() const
{
	return Affector;
}


//! It is possible to send mouse and key events to the camera. Most cameras
//! may ignore this input, but camera scene nodes which are created for
//! example with ISceneManager::addMayaCameraSceneNode or
//! ISceneManager::addFPSCameraSceneNode, may want to get this input
//! for changing their position, look at target or whatever.
/*
bool CameraNode::OnEvent(const Event& event)
{
	if (!InputReceiverEnabled)
		return false;

	// send events to event receiving animators

	SceneNodeAnimatorList::Iterator ait = Animators.begin();

	for (; ait != Animators.end(); ++ait)
		if ((*ait)->iEventReceiverEnabled() && (*ait)->OnEvent(event))
			return true;

	// if nobody processed the event, return false
	return false;
}
*/


//! sets the up vector of the camera
//! \param pos: New upvector of the camera.
void CameraNode::SetUpVector(const Vector3<float>& pos)
{
	UpVector = pos;
}


//! Gets the up vector of the camera.
//! \return Returns the up vector of the camera.
const Vector3<float>& CameraNode::GetUpVector() const
{
	return UpVector;
}


float CameraNode::GetNearValue() const
{
	return ZNear;
}


float CameraNode::GetFarValue() const
{
	return ZFar;
}


float CameraNode::GetAspectRatio() const
{
	return Aspect;
}


float CameraNode::GetFOV() const
{
	return Fovy;
}


void CameraNode::SetNearValue(float f)
{
	ZNear = f;
	RecalculateProjectionMatrix();
}


void CameraNode::SetFarValue(float f)
{
	ZFar = f;
	RecalculateProjectionMatrix();
}


void CameraNode::SetAspectRatio(float f)
{
	Aspect = f;
	RecalculateProjectionMatrix();
}


void CameraNode::SetFOV(float f)
{
	Fovy = f;
	RecalculateProjectionMatrix();
}


void CameraNode::RecalculateProjectionMatrix()
{
	ViewArea.GetTransform ( ETS_PROJECTION ).BuildProjectionMatrixPerspectiveFovLH(Fovy, Aspect, ZNear, ZFar);
}


//! update
void CameraNode::UpdateMatrices()
{
	Matrix4x4<float> toWorld, fromWorld;
	Get()->Transform(&toWorld, &fromWorld);

	Vector3<float> pos = toWorld.GetTranslation();
	Vector3<float> target = toWorld.GetTranslation() + toWorld.GetRotationDegrees().RotationToDirection();

	if (mTarget)
		target = m_pTarget->Get()->ToWorld().GetTranslation();

	Vector3<float> tgtv = target - pos;
	tgtv.Normalize();

	Vector3<float> up = UpVector;
	up.Normalize();

	float dp = tgtv.DotProduct(up);

	if ( eastl::equals(eastl::abs<Vector3<float>>(dp), 1.f) )
	{
		up.X += 0.5f;
	}

	ViewArea.GetTransform(ETS_VIEW).BuildCameraLookAtMatrixLH(pos, target, up);
	ViewArea.GetTransform(ETS_VIEW) *= Affector;
	RecalculateViewArea();
}

void CameraNode::RecalculateViewArea()
{
	Matrix4x4<float> toWorld, fromWorld;
	Get()->Transform(&toWorld, &fromWorld);

	ViewArea.CameraPosition = toWorld.GetTranslation();

	Matrix4x4<float> m(Matrix4x4<float>::EM4CONST_NOTHING);
	m.setbyproductnocheck(	ViewArea.GetTransform(ETS_PROJECTION),
							ViewArea.GetTransform(ETS_VIEW));
	ViewArea.SetFrom(m);
}

//! prerender
bool CameraNode::PreRender(Scene *pScene)
{
	if (pScene->GetActiveCamera() == shared_from_this())
	{
		// register according to material types counted
		pScene->AddToRenderQueue(ERP_CAMERA, shared_from_this());
	}

	return SceneNode::PreRender(pScene);
}

//
// CameraSceneNode::Render					- Chapter 16, page 550
//
bool CameraNode::Render(Scene *pScene)
{
	UpdateMatrices();

	if ( pScene->GetRenderer() )
	{
		pScene->GetRenderer()->SetTransform(ETS_PROJECTION, ViewArea.GetTransform ( ETS_PROJECTION) );
		pScene->GetRenderer()->SetTransform(ETS_VIEW, ViewArea.GetTransform ( ETS_VIEW) );
	}

	return true;
}


//
// CameraNode::SetView					- Chapter 16, page 550
//
//    Note: this is incorrectly called CameraNode::SetView in the book
//
bool CameraNode::SetViewTransform(Scene *pScene)
{
	/*
	//If there is a target, make sure the camera is
	//rigidly attached right behind the target
	if(mTarget)
	{
		Matrix4x4<float> mat = mTarget->Get()->ToWorld();
		Vector4<float> at = mCamOffsetVector;
		Vector4<float> atWorld = mat.Xform(at);
		Vector3<float> pos = mat.GetPosition() + Vector3<float>(atWorld);
		mat.SetPosition(pos);
		SetTransform(&mat);
	}

	mView = Get()->FromWorld();

	pScene->GetRenderer()->SetViewTransform(&mView);
	*/
	return true;
}

//
// CameraSceneNode::GetWorldViewProjection			- not described in the book
//
//    Returns the concatenation of the world and view projection, which is generally sent into vertex shaders
//
Matrix4x4<float> CameraNode::GetWorldViewProjection(Scene *pScene)
{ 
	Matrix4x4<float> world = Get()->ToWorld();
	Matrix4x4<float> view = Get()->FromWorld();
	Matrix4x4<float> worldView = world * view;
	return worldView * m_Projection;
}

//! returns the axis aligned bounding box of this node
const AlignedBox3<float>& CameraNode::GetBoundingBox() const
{
	// should never be used.
	return ViewArea.GetBoundingBox();
}


//! returns the view frustum. needed sometimes by bsp or lod render nodes.
const ViewFrustum& CameraNode::GetViewFrustum() const
{
	return ViewArea;
}
