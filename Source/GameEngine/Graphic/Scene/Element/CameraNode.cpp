// Copyright (C) 2002-2012 Nikolaus Gebhardt
// This file is part of the "Irrlicht Engine".
// For conditions of distribution and use, see copyright notice in irrlicht.h

#include "CameraNode.h"

#include "Graphic/Renderer/Renderer.h"
#include "Graphic/Scene/Scene.h"

//! constructor
CameraNode::CameraNode(const ActorId actorid)
	: Node(actorid, WeakBaseRenderComponentPtr(), NT_CAMERA), 
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
	mCamera->SetFrustum(60.0f, aspectRatio, 0.1f, 3000.0f);
	Vector4<float> camPosition{ 0.0f, 50.0f, 0.0f, 1.0f };
	Vector4<float> camDVector{ 0.0f, 1.0f, 0.0f, 0.0f };
	Vector4<float> camUVector{ 0.0f, 0.0f, 1.0f, 0.0f };
	Vector4<float> camRVector = Cross(camDVector, camUVector);
	mCamera->SetFrame(camPosition, camDVector, camUVector, camRVector);

	mLocalTransform.SetTranslation(camPosition);
	mWorldTransform.SetTranslation(camPosition);
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

//! update
void CameraNode::UpdateMatrices()
{
	Matrix4x4<float> rotation = mWorldTransform.GetRotation();
	Vector4<float> position = mWorldTransform.GetTranslationW1();
	Vector4<float> direction;
#if defined(GE_USE_MAT_VEC)
	direction = rotation.GetCol(1);
#else
	direction = rotation.GetRow(1);
#endif

	Vector4<float> up;
#if defined(GE_USE_MAT_VEC)
	up = rotation.GetCol(2);
#else
	up = rotation.GetRow(2);
#endif
	/*
	float dp = Dot(direction, Vector4<float>::Unit(2)); // up vector
	if (Function<float>::Equals(fabs(dp), 1.f))
		up[0] += 0.5f;
	*/
	Vector4<float> right = Cross(direction, up);

#if defined(GE_USE_MAT_VEC)
	mCamera->SetFrame(
		mAffector * position,
		mAffector * direction,
		mAffector * up,
		mAffector * right);
#else
	mCamera->SetFrame(
		position * mAffector,
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
	return true;
}
