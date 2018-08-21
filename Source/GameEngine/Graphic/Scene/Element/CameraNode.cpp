// Copyright (C) 2002-2012 Nikolaus Gebhardt
// This file is part of the "Irrlicht Engine".
// For conditions of distribution and use, see copyright notice in irrlicht.h

#include "CameraNode.h"

#include "Graphic/Renderer/Renderer.h"
#include "Graphic/Scene/Scene.h"

//! constructor
CameraNode::CameraNode(const ActorId actorid)
	: Node(actorid, WeakBaseRenderComponentPtr(), RP_NONE, NT_CAMERA), 
	mActive(true), mTarget(0), mCamera(eastl::make_shared<Camera>(true, true))
{
	mAffector = Matrix4x4<float>::Identity();

	float aspectRatio = 4.0f / 3.0f;	// Aspect ratio.
	if (Renderer::Get())
	{
		aspectRatio =
			(float)Renderer::Get()->GetScreenSize()[0] /
			(float)Renderer::Get()->GetScreenSize()[1];
	}
	mCamera->SetFrustum((float)GE_C_PI, aspectRatio, 1.0f, 3000.0f);
}


//! Sets a custom view matrix affector. The matrix passed here, will be
//! multiplied with the view matrix when it gets updated.
//! This allows for custom camera setups like, for example, a reflection camera.
/** \param affector: The affector matrix. */
void CameraNode::SetViewMatrixAffector(const Matrix4x4<float>& affector)
{
	mAffector = affector;
}


//! Gets the custom view matrix affector.
const Matrix4x4<float>& CameraNode::GetViewMatrixAffector() const
{
	return mAffector;
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

	SceneNodeAnimatorList::Iterator ait = mAnimators.begin();

	for (; ait != mAnimators.end(); ++ait)
		if ((*ait)->EventReceiverEnabled() && (*ait)->OnEvent(event))
			return true;

	// if nobody processed the event, return false
	return false;
}
*/

//! update
void CameraNode::UpdateMatrices()
{
	Vector4<float> pos = mWorldTransform.GetTranslationW1();
	Vector4<float> target = mCamera->GetPosition() + mCamera->GetDVector();
	if (mTarget)
		target = mTarget->GetAbsoluteTransform().GetTranslationW1();

	Vector4<float> direction = target - pos;
	Normalize(direction);

	Vector4<float> up = mCamera->GetUVector();
	Normalize(up);

	float dp = Dot(direction, up);
	if ( Function<float>::Equals(fabs(dp), 1.f) )
		up[0] += 0.5f;

	Vector4<float> right = Cross(direction, up);

#if defined(GE_USE_MAT_VEC)
	mCamera->SetPosition( mAffector * pos );
	mCamera->SetAxes(
		mAffector * direction,
		mAffector * up,
		mAffector * right);
#else
	mCamera->SetPosition(pos * mAffector);
	mCamera->SetAxes(
		direction * mAffector,
		up * mAffector,
		right * mAffector);
#endif
}

//! prerender
bool CameraNode::PreRender(Scene *pScene)
{
	if (pScene->GetActiveCamera() == shared_from_this())
	{
		// register according to material types counted
		pScene->AddToRenderQueue(RP_CAMERA, shared_from_this());
	}

	return Node::PreRender(pScene);
}

//
// CameraSceneNode::Render					- Chapter 16, page 550
//
bool CameraNode::Render(Scene *pScene)
{
	UpdateMatrices();
	/*
	if (Renderer::Get())
	{
		Renderer::Get()->SetTransform(TS_PROJECTION, ViewArea.GetTransform ( TS_PROJECTION) );
		Renderer::Get()->SetTransform(TS_VIEW, ViewArea.GetTransform ( TS_VIEW) );
	}
	*/
	return true;
}
