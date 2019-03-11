// QuakePlayerController.cpp - Controller class for the player 
//
// Part of the GameEngine Application
//
// GameEngine is the sample application that encapsulates much of the source code
// discussed in "Game Coding Complete - 4th Edition" by Mike McShaffry and David
// "Rez" Graham, published by Charles River Media. 
// ISBN-10: 1133776574 | ISBN-13: 978-1133776574
//
// If this source code has found it's way to you, and you think it has helped you
// in any way, do the authors a favor and buy a new copy of the book - there are 
// detailed explanations in it that compliment this code well. Buy a copy at Amazon.com
// by clicking here: 
//    http://www.amazon.com/gp/product/1133776574/ref=olp_product_details?ie=UTF8&me=&seller=
//
// There's a companion web site at http://www.mcshaffry.com/GameCode/
// 
// The source code is managed and maintained through Google Code: 
//    http://code.google.com/p/GameEngine/
//
// (c) Copyright 2012 Michael L. McShaffry and David Graham
//
// This program is free software; you can redistribute it and/or
// modify it under the terms of the GNU Lesser GPL v3
// as published by the Free Software Foundation.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See 
// http://www.gnu.org/licenses/lgpl-3.0.txt for more details.
//
// You should have received a copy of the GNU Lesser GPL v3
// along with this program; if not, write to the Free Software
// Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
//
//========================================================================

#include "QuakeStd.h"

#include "Core/Logger/Logger.h"

#include "Core/Event/EventManager.h"

#include "Physic/PhysicEventListener.h"

#include "Actors/PlayerActor.h"
#include "Actors/PushTrigger.h"
#include "Actors/TeleporterTrigger.h"
#include "QuakePlayerController.h"
#include "QuakeEvents.h"
#include "QuakeApp.h"

////////////////////////////////////////////////////
// QuakePlayerController Implementation
////////////////////////////////////////////////////


QuakePlayerController::QuakePlayerController(
	const eastl::shared_ptr<Node>& target, float initialYaw, float initialPitch)
	: mTarget(target)
{
	mYaw = initialYaw;
	mPitchTarget = -initialPitch;

	mMaxJumpSpeed = 3.4f;
	mMaxFallSpeed = 240.0f;
	mMaxRotateSpeed = 180.0f;
	mMoveSpeed = 6.0f;
	mJumpSpeed = 3.4f;
	mJumpMoveSpeed = 10.0f;
	mFallSpeed = 0.0f;
	mRotateSpeed = 0.0f;

	//Point cursor;
	System* system = System::Get();
	system->GetCursorControl()->SetPosition(0.5f, 0.5f);
	Vector2<unsigned int> cursorPosition = system->GetCursorControl()->GetPosition();
	mLastMousePos[0] = cursorPosition[0];
	mLastMousePos[1] = cursorPosition[1];

	memset(mKey, 0x00, sizeof(mKey));

	mMouseUpdate = true;
	mMouseRButtonDown = false;
	mMouseLButtonDown = false;
	mWheelRollDown = false;
	mWheelRollUp = false;
}

//
// QuakePlayerController::OnMouseButtonDown		- Chapter 10, page 282
//
bool QuakePlayerController::OnMouseButtonDown(
	const Vector2<int> &mousePos, const int radius, const eastl::string &buttonName)
{
	if (buttonName == "PointerLeft")
	{
		mMouseLButtonDown = true;

		// We want mouse movement to be relative to the position
		// the cursor was at when the user first presses down on
		// the left button
		mLastMousePos = mousePos;
		return true;
	}
	else if (buttonName == "PointerRight")
	{
		mMouseRButtonDown = true;

		// We want mouse movement to be relative to the position
		// the cursor was at when the user first presses down on
		// the right button
		mLastMousePos = mousePos;
		return true;
	}
	return false;
}

bool QuakePlayerController::OnMouseButtonUp(
	const Vector2<int> &mousePos, const int radius, const eastl::string &buttonName)
{
	if (buttonName == "PointerLeft")
	{
		mMouseLButtonDown = false;
		return true;
	}
	else if (buttonName == "PointerRight")
	{
		mMouseRButtonDown = false;
		return true;
	}
	return false;
}


//  class QuakePlayerController::OnMouseMove		- Chapter 10, page 282

bool QuakePlayerController::OnMouseMove(const Vector2<int> &mousePos, const int radius)
{
	if (mMouseUpdate)
	{
		// rotate the view
		if (mLastMousePos != mousePos)
		{
			mRotateSpeed = mMaxRotateSpeed;

			System* system = System::Get();
			mYaw += ((mLastMousePos[0] - mousePos[0]) / (float)system->GetWidth()) * mRotateSpeed;
			mPitchTarget += ((mousePos[1] - mLastMousePos[1]) / (float)system->GetHeight()) * mRotateSpeed;
			mLastMousePos = mousePos;
		}
	}

	return true;
}

//  class QuakePlayerController::OnUpdate			- Chapter 10, page 283

void QuakePlayerController::OnUpdate(unsigned int timeMs, unsigned long deltaMs)
{
	// Special case, mouse is whipped outside of window before it can update.
	if (mEnabled)
	{
		System* system = System::Get();
		Vector2<unsigned int> cursorPosition = system->GetCursorControl()->GetPosition();
		Vector2<int> mousePosition{ (int)cursorPosition[0], (int)cursorPosition[1] };

		Renderer* renderer = Renderer::Get();
		Vector2<unsigned int> screenSize(renderer->GetScreenSize());
		RectangleShape<2, int> screenRectangle;
		screenRectangle.mCenter[0] = screenSize[0] / 2;
		screenRectangle.mCenter[1] = screenSize[1] / 2;
		screenRectangle.mExtent[0] = (int)screenSize[0];
		screenRectangle.mExtent[1] = (int)screenSize[1];

		// Only if we are moving outside quickly.
		bool reset = !screenRectangle.IsPointInside(mousePosition);

		if (reset)
		{
			// Force a reset.
			mMouseUpdate = false;
			system->GetCursorControl()->SetPosition(0.5f, 0.5f);
			cursorPosition = system->GetCursorControl()->GetPosition();
			mLastMousePos[0] = cursorPosition[0];
			mLastMousePos[1] = cursorPosition[1];
		}
		else mMouseUpdate = true;
	}
	//Handling rotation as a result of mouse position
	Matrix4x4<float> rotation;

	const ActorId actorId = mTarget->GetId();
	eastl::shared_ptr<PlayerActor> pPlayerActor(
		eastl::dynamic_shared_pointer_cast<PlayerActor>(
		GameLogic::Get()->GetActor(actorId).lock()));
	if (pPlayerActor)
	{
		mPitchTarget = eastl::max(-85.f, eastl::min(85.f, mPitchTarget));
		mPitch = 90 * ((mPitchTarget + 85.f) / 170.f) - 45.f;

		// Calculate the new rotation matrix from the camera
		// yaw and pitch (zrotate and xrotate).
		Matrix4x4<float> yawRotation = Rotation<4, float>(
			AxisAngle<4, float>(Vector4<float>::Unit(2), mYaw * (float)GE_C_DEG_TO_RAD));
		rotation = -yawRotation;
		Matrix4x4<float> pitchRotation = Rotation<4, float>(
			AxisAngle<4, float>(Vector4<float>::Unit(1), mPitch * (float)GE_C_DEG_TO_RAD));
		mAbsoluteTransform.SetRotation(yawRotation * pitchRotation);

		// update node rotation matrix
		eastl::shared_ptr<TransformComponent> pTransformComponent(
			pPlayerActor->GetComponent<TransformComponent>(TransformComponent::Name).lock());
		if (pTransformComponent)
		{
			pitchRotation = Rotation<4, float>(
				AxisAngle<4, float>(Vector4<float>::Unit(1), mPitchTarget * (float)GE_C_DEG_TO_RAD));
			pTransformComponent->SetRotation(yawRotation * pitchRotation);
			/*
			Vector3<float> pos = pTransformComponent->GetPosition();
			printf("pos %f %f %f\n", pos[0], pos[1], pos[2]);
			*/
		}
	}

	Vector4<float> atWorld = Vector4<float>::Zero();
	Vector4<float> rightWorld = Vector4<float>::Zero();
	Vector4<float> upWorld = Vector4<float>::Zero();

	if (mKey[KEY_KEY_W] || mKey[KEY_KEY_S])
	{
		// This will give us the "look at" vector 
		// in world space - we'll use that to move
		// the camera.
		atWorld = Vector4<float>::Unit(PITCH); // forward vector
#if defined(GE_USE_MAT_VEC)
		atWorld = rotation * atWorld;
#else
		atWorld = atWorld * rotation;
#endif

		if (mKey[KEY_KEY_W])
			atWorld *= -1.f;
	}

	if (mKey[KEY_KEY_A] || mKey[KEY_KEY_D])
	{
		// This will give us the "look right" vector 
		// in world space - we'll use that to move
		// the camera.
		rightWorld = Vector4<float>::Unit(ROLL); // right vector
#if defined(GE_USE_MAT_VEC)
		rightWorld = rotation * rightWorld;
#else
		rightWorld = rightWorld * rotation;
#endif

		if (mKey[KEY_KEY_A])
			rightWorld *= -1.f;
	}

	if (pPlayerActor->GetAction().triggerTeleporter != INVALID_ACTOR_ID)
	{
		eastl::shared_ptr<Actor> pItemActor(
			eastl::dynamic_shared_pointer_cast<Actor>(
			GameLogic::Get()->GetActor(pPlayerActor->GetAction().triggerTeleporter).lock()));
		eastl::shared_ptr<TeleporterTrigger> pTeleporterTrigger =
			pItemActor->GetComponent<TeleporterTrigger>(TeleporterTrigger::Name).lock();
		pPlayerActor->GetAction().triggerTeleporter = INVALID_ACTOR_ID;

		EulerAngles<float> yawPitchRoll;
		yawPitchRoll.mAxis[1] = 1;
		yawPitchRoll.mAxis[2] = 2;
		pTeleporterTrigger->GetTarget().GetRotation(yawPitchRoll);
		mYaw = yawPitchRoll.mAngle[YAW] * (float)GE_C_RAD_TO_DEG;
		mPitchTarget = -yawPitchRoll.mAngle[ROLL] * (float)GE_C_RAD_TO_DEG;

		eastl::shared_ptr<TransformComponent> pTransformComponent =
			pPlayerActor->GetComponent<TransformComponent>(TransformComponent::Name).lock();
		if (pTransformComponent)
			pTransformComponent->SetTransform(pTeleporterTrigger->GetTarget());

		eastl::shared_ptr<PhysicComponent> pPhysicalComponent =
			pPlayerActor->GetComponent<PhysicComponent>(PhysicComponent::Name).lock();
		if (pPhysicalComponent)
			pPhysicalComponent->SetTransform(pTeleporterTrigger->GetTarget());

		// play teleporter sound
		EventManager::Get()->TriggerEvent(
			eastl::make_shared<EventDataPlaySound>("audio/quake/sound/world/teleout.ogg"));
	}
	else
	{
		eastl::shared_ptr<PhysicComponent> pPhysicComponent(
			pPlayerActor->GetComponent<PhysicComponent>(PhysicComponent::Name).lock());
		if (pPhysicComponent)
		{
			int prevActionType = pPlayerActor->GetAction().actionType;

			pPlayerActor->GetAction().actionType = 0;
			if (mMouseLButtonDown)
				pPlayerActor->GetAction().actionType |= ACTION_ATTACK;
			if (mWheelRollDown)
				pPlayerActor->PreviousWeapon();
			if (mWheelRollUp)
				pPlayerActor->NextWeapon();

			if (mKey[KEY_KEY_S])
				pPlayerActor->GetAction().actionType |= ACTION_MOVEBACK;
			else if (mKey[KEY_KEY_W])
				pPlayerActor->GetAction().actionType |= ACTION_MOVEFORWARD;
			else if (mKey[KEY_KEY_A])
				pPlayerActor->GetAction().actionType |= ACTION_MOVELEFT;
			else if (mKey[KEY_KEY_D])
				pPlayerActor->GetAction().actionType |= ACTION_MOVERIGHT;

			Vector4<float> velocity = Vector4<float>::Zero();
			if (pPhysicComponent->OnGround())
			{
				mFallSpeed = 0.0f;

				if (pPlayerActor->GetAction().triggerPush != INVALID_ACTOR_ID)
				{
					float push;
					Vector3<float> direction;
					eastl::shared_ptr<TransformComponent> pTransformComponent =
						pPlayerActor->GetComponent<TransformComponent>(TransformComponent::Name).lock();
					if (pTransformComponent)
					{
						eastl::shared_ptr<Actor> pItemActor(
							eastl::dynamic_shared_pointer_cast<Actor>(
							GameLogic::Get()->GetActor(pPlayerActor->GetAction().triggerPush).lock()));
						eastl::shared_ptr<PushTrigger> pPushTrigger =
							pItemActor->GetComponent<PushTrigger>(PushTrigger::Name).lock();

						Vector3<float> targetPosition = pPushTrigger->GetTarget().GetTranslation();
						Vector3<float> playerPosition = pTransformComponent->GetPosition();
						direction = targetPosition - playerPosition;
						push = Length(direction);
						Normalize(direction);
					}

					direction[PITCH] *= push / 90.f;
					direction[ROLL] *= push / 90.f;
					direction[YAW] = push / 30.f;
					velocity = HLift(direction, 0.f);

					pPlayerActor->GetAction().actionType |= ACTION_JUMP;
				}
				else if (mEnabled)
				{
					if (mMouseRButtonDown)
					{
						upWorld = Vector4<float>::Unit(YAW);
						Vector4<float> direction = atWorld + rightWorld + upWorld;
						Normalize(direction);

						direction[PITCH] *= mJumpMoveSpeed;
						direction[ROLL] *= mJumpMoveSpeed;
						direction[YAW] *= mJumpSpeed;
						velocity = direction;

						pPlayerActor->GetAction().actionType |= ACTION_JUMP;
					}
					else
					{
						Vector4<float> direction = atWorld + rightWorld + upWorld;
						Normalize(direction);

						direction *= mMoveSpeed;
						velocity = direction;
					}
				}
				pPlayerActor->GetAction().actionType |= ACTION_RUN;
			}
			else
			{
				mFallSpeed += deltaMs/ (pPhysicComponent->GetJumpSpeed() * 0.5f);
				if (mFallSpeed > mMaxFallSpeed) mFallSpeed = mMaxFallSpeed;

				upWorld = -Vector4<float>::Unit(YAW);
				Vector4<float> direction = atWorld + rightWorld + upWorld;
				Normalize(direction);
				
				direction[PITCH] *= pPhysicComponent->GetJumpSpeed() * (mFallSpeed / 4.f);
				direction[ROLL] *= pPhysicComponent->GetJumpSpeed() * (mFallSpeed / 4.f);
				direction[YAW] = -pPhysicComponent->GetJumpSpeed() * mFallSpeed;
				velocity = direction;

				pPlayerActor->GetAction().actionType |= ACTION_FALLEN;
			}

			if (pPlayerActor->GetState().moveType != PM_DEAD)
			{
				EventManager::Get()->TriggerEvent(
					eastl::make_shared<QuakeEventDataRotateActor>(actorId, mAbsoluteTransform));

				pPlayerActor->UpdateTimers(deltaMs);
				pPlayerActor->UpdateWeapon(deltaMs);
				pPlayerActor->UpdateMovement(HProject(velocity));
			}
			else if (!(prevActionType & ACTION_ATTACK))
			{
				if (pPlayerActor->GetAction().actionType & ACTION_ATTACK)
					pPlayerActor->PlayerSpawn();
			}
		}
	}

	mWheelRollDown = false;
	mWheelRollUp = false;
}