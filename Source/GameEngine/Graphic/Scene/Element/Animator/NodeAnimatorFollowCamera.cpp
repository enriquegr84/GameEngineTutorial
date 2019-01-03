// Copyright (C) 2002-2012 Nikolaus Gebhardt
// This file is part of the "Irrlicht Engine".
// For conditions of distribution and use, see copyright notice in irrlicht.h

#include "NodeAnimatorFollowCamera.h"

#include "Core/OS/OS.h"

#include "Application/GameApplication.h"
#include "Application/System/System.h"

#include "Graphic/Scene/Scene.h"
#include "Graphic/Scene/Element/CameraNode.h"

//! constructor
NodeAnimatorFollowCamera::NodeAnimatorFollowCamera(float rotateSpeed, float moveSpeed)
: mLastAnimationTime(0), mMaxVerticalAngle(88.0f), mMoveSpeed(moveSpeed), mRotateSpeed(rotateSpeed)
{
	System* system = System::Get();
	system->GetCursorControl()->SetPosition(0.5f, 0.5f);
	mCursorPosition = mCenterCursor = system->GetCursorControl()->GetRelativePosition();

	mLastTargetPosition.MakeZero();
}

//! animates a scene node
void NodeAnimatorFollowCamera::AnimateNode(Scene* pScene, Node* node, unsigned int timeMs)
{
	if (!node || node->GetType() != NT_CAMERA)
		return;

	// get time
	float timeDiff = (float)(timeMs - mLastAnimationTime);
	mLastAnimationTime = timeMs;

	CameraNode* camera = reinterpret_cast<CameraNode*>(node);
	if (pScene->GetActiveCamera().get() != camera || !camera->GetTarget())
		return;

	eastl::shared_ptr<Actor> pGameActor(
		GameLogic::Get()->GetActor(camera->GetTarget()->GetId()).lock());
	eastl::shared_ptr<PhysicComponent> pPhysicComponent(
		pGameActor->GetComponent<PhysicComponent>(PhysicComponent::Name).lock());
	if (pPhysicComponent)
	{
		float yaw = 180 + pPhysicComponent->GetOrientationOffset()[2];

		// set the camera behind the node and project it towards node direction
		Matrix4x4<float> yawRotation = Rotation<4, float>(
			AxisAngle<4, float>(Vector4<float>::Unit(2), yaw * (float)GE_C_DEG_TO_RAD));
		Transform targetTransform = pPhysicComponent->GetTransform();
		camera->GetAbsoluteTransform().SetRotation(targetTransform.GetRotation() * yawRotation);
		Vector4<float> translation = targetTransform.GetTranslationW1();
		Vector4<float> direction = Vector4<float>::Unit(1); // forward vector
#if defined(GE_USE_MAT_VEC)
		direction = camera->GetAbsoluteTransform() * direction;
#else
		direction = direction * camera->GetAbsoluteTransform();
#endif

		float scale = 80;
		Vector4<float> offset{ 0, 0, 60, 0 };
		camera->GetAbsoluteTransform().SetTranslation(translation - direction * scale + offset);
	}
}

//! Sets the rotation speed
void NodeAnimatorFollowCamera::SetRotateSpeed(float speed)
{
	mRotateSpeed = speed;
}


//! Sets the movement speed
void NodeAnimatorFollowCamera::SetMoveSpeed(float speed)
{
	mMoveSpeed = speed;
}

//! Gets the rotation speed
float NodeAnimatorFollowCamera::GetRotateSpeed() const
{
	return mRotateSpeed;
}


// Gets the movement speed
float NodeAnimatorFollowCamera::GetMoveSpeed() const
{
	return mMoveSpeed;
}

NodeAnimator* NodeAnimatorFollowCamera::CreateClone(Node* node)
{
	NodeAnimatorFollowCamera* newAnimator =
		new NodeAnimatorFollowCamera(mRotateSpeed, mMoveSpeed);

	return newAnimator;
}