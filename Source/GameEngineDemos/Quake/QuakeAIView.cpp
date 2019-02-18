//========================================================================
// QuakeAIView.cpp : AI Controller class
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

#include "Core/OS/OS.h"

#include "Core/Logger/Logger.h"

#include "Core/Event/EventManager.h"

#include "Physic/PhysicEventListener.h"

#include "Actors/PlayerActor.h"
#include "Actors/PushTrigger.h"
#include "Actors/TeleporterTrigger.h"
#include "QuakePlayerController.h"
#include "QuakeAIView.h"
#include "QuakeEvents.h"
#include "QuakeApp.h"

///////////////////////////////////////////////////////////////////////////////
//
// QuakeAIView::QuakeAIView
//
QuakeAIView::QuakeAIView()
	: BaseGameView()
{
	mYaw = 0.0f;
	mPitchTarget = 0.0f;

	mOrientation = 1;
	mStationaryTime = 0;

	mMaxJumpSpeed = 3.4f;
	mMaxFallSpeed = 240.0f;
	mMaxRotateSpeed = 180.0f;
	mMoveSpeed = 6.0f;
	mJumpSpeed = 3.4f;
	mJumpMoveSpeed = 10.0f;
	mFallSpeed = 0.0f;
	mRotateSpeed = 0.0f;
}

//
// QuakeAIView::~QuakeAIView
//
QuakeAIView::~QuakeAIView(void)
{
	//LogInformation("AI Destroying QuakeAIView");
}

//  class QuakeAIView::OnAttach
void QuakeAIView::OnAttach(GameViewId vid, ActorId actorId)
{
	mViewId = vid;
	mPlayerId = actorId;

	eastl::shared_ptr<Actor> pGameActor(GameLogic::Get()->GetActor(mPlayerId).lock());
	eastl::shared_ptr<TransformComponent> pTransformComponent(
		pGameActor->GetComponent<TransformComponent>(TransformComponent::Name).lock());
	if (pTransformComponent)
	{
		EulerAngles<float> yawPitchRoll;
		yawPitchRoll.mAxis[1] = 1;
		yawPitchRoll.mAxis[2] = 2;
		pTransformComponent->GetTransform().GetRotation(yawPitchRoll);

		mYaw = yawPitchRoll.mAngle[YAW] * (float)GE_C_RAD_TO_DEG;
		mPitchTarget = yawPitchRoll.mAngle[ROLL] * (float)GE_C_RAD_TO_DEG;
	}
}

//Stationary movement
Vector4<float> QuakeAIView::Stationary(unsigned long deltaMs)
{
	Vector3<float> position = mAbsoluteTransform.GetTranslation();
	Matrix4x4<float> rotation = Rotation<4, float>(
		AxisAngle<4, float>(Vector4<float>::Unit(2), mYaw * (float)GE_C_DEG_TO_RAD));

	// This will give us the "look at" vector 
	// in world space - we'll use that to move.
	Vector4<float> atWorld = Vector4<float>::Unit(PITCH); // forward vector
#if defined(GE_USE_MAT_VEC)
	atWorld = rotation * atWorld;
#else
	atWorld = atWorld * rotation;
#endif

	Transform start;
	start.SetRotation(rotation);
	start.SetTranslation(mAbsoluteTransform.GetTranslationW1() + atWorld * 10.f);

	Transform end;
	end.SetRotation(rotation);
	end.SetTranslation(mAbsoluteTransform.GetTranslationW1() + atWorld * 500.f);

	Vector3<float> collision, collisionNormal;
	collision = end.GetTranslation();
	ActorId actorId = GameLogic::Get()->GetGamePhysics()->ConvexSweep(
		mPlayerId, start, end, collision, collisionNormal);
	if (Length(collision - position) < 5.f)
	{
		if (mStationaryTime > 100.f)
		{
			mStationaryTime = 0;

			//Choose randomly which way too look for obstacles
			int sign = Randomizer::Rand() % 2 ? 1 : -1;

			mYaw += 130.f * sign;
			rotation = Rotation<4, float>(
				AxisAngle<4, float>(Vector4<float>::Unit(2), mYaw * (float)GE_C_DEG_TO_RAD));

			atWorld = Vector4<float>::Unit(PITCH); // forward vector
#if defined(GE_USE_MAT_VEC)
			atWorld = rotation * atWorld;
#else
			atWorld = atWorld * rotation;
#endif
			return atWorld;
		}
		mStationaryTime += deltaMs;
	}
	else mStationaryTime = 0;

	return atWorld;
}

//Smooth movement
Vector4<float> QuakeAIView::Smooth() 
{
	Vector3<float> position = mAbsoluteTransform.GetTranslation();
	Matrix4x4<float> rotation = Rotation<4, float>(
		AxisAngle<4, float>(Vector4<float>::Unit(2), mYaw * (float)GE_C_DEG_TO_RAD));

	// This will give us the "look at" vector 
	// in world space - we'll use that to move.
	Vector4<float> atWorld = Vector4<float>::Unit(PITCH); // forward vector
#if defined(GE_USE_MAT_VEC)
	atWorld = rotation * atWorld;
#else
	atWorld = atWorld * rotation;
#endif

	Transform start;
	start.SetRotation(rotation);
	start.SetTranslation(mAbsoluteTransform.GetTranslationW1() + atWorld * 10.f);

	Transform end;
	end.SetRotation(rotation);
	end.SetTranslation(mAbsoluteTransform.GetTranslationW1() + atWorld * 500.f);

	Vector3<float> collision, collisionNormal;
	collision = end.GetTranslation();
	ActorId actorId = GameLogic::Get()->GetGamePhysics()->ConvexSweep(
		mPlayerId, start, end, collision, collisionNormal);
	if (Length(collision - position) < 40.f)
	{
		//Choose randomly which way too look for obstacles
		int sign = Randomizer::Rand() % 2 ? 1 : -1;

		// Smoothly turn 90º and check raycasting until we meet a minimum distance
		for (int angle = 1; angle <= 90; angle++)
		{
			rotation = Rotation<4, float>(
				AxisAngle<4, float>(Vector4<float>::Unit(2),
				(mYaw + angle * sign) * (float)GE_C_DEG_TO_RAD));

			atWorld = Vector4<float>::Unit(PITCH); // forward vector
#if defined(GE_USE_MAT_VEC)
			atWorld = rotation * atWorld;
#else
			atWorld = atWorld * rotation;
#endif

			start.SetRotation(rotation);
			end.SetRotation(rotation);
			end.SetTranslation(mAbsoluteTransform.GetTranslationW1() + atWorld * 500.f);

			collision = end.GetTranslation();
			ActorId actorId = GameLogic::Get()->GetGamePhysics()->ConvexSweep(
				mPlayerId, start, end, collision, collisionNormal);
			if (Length(collision - position) > 40.f)
			{
				mOrientation = Randomizer::Rand() % 2 ? 1 : -1;
				mYaw += angle * sign;
				return atWorld;
			}
		}

		//If we haven't find a way out we proceed exactly the same but in the opposite direction
		sign *= -1;
		for (int angle = 1; angle <= 90; angle++)
		{
			rotation = Rotation<4, float>(
				AxisAngle<4, float>(Vector4<float>::Unit(2),
				(mYaw + angle * sign) * (float)GE_C_DEG_TO_RAD));

			atWorld = Vector4<float>::Unit(PITCH); // forward vector
#if defined(GE_USE_MAT_VEC)
			atWorld = rotation * atWorld;
#else
			atWorld = atWorld * rotation;
#endif

			start.SetRotation(rotation);
			end.SetRotation(rotation);
			end.SetTranslation(mAbsoluteTransform.GetTranslationW1() + atWorld * 500.f);

			collision = end.GetTranslation();
			ActorId actorId = GameLogic::Get()->GetGamePhysics()->ConvexSweep(
				mPlayerId, start, end, collision, collisionNormal);
			if (Length(collision - position) > 40.f)
			{
				mOrientation = Randomizer::Rand() % 2 ? 1 : -1;
				mYaw += angle * sign;
				return atWorld;
			}
		}

		//if we couldnt find any way out the stationary function will take care of it.
		mOrientation = Randomizer::Rand() % 2 ? 1 : -1;
	}

	rotation = Rotation<4, float>(
		AxisAngle<4, float>(Vector4<float>::Unit(2), mYaw * (float)GE_C_DEG_TO_RAD));

	atWorld = Vector4<float>::Unit(PITCH); // forward vector
#if defined(GE_USE_MAT_VEC)
	atWorld = rotation * atWorld;
#else
	atWorld = atWorld * rotation;
#endif
	return atWorld;
}

// Cliff control
Vector4<float> QuakeAIView::Cliff()
{
	Vector3<float> position = mAbsoluteTransform.GetTranslation();

	Matrix4x4<float> yawRotation = Rotation<4, float>(
		AxisAngle<4, float>(Vector4<float>::Unit(2), mYaw * (float)GE_C_DEG_TO_RAD));
	Matrix4x4<float> pitchRotation = Rotation<4, float>(
		AxisAngle<4, float>(Vector4<float>::Unit(1), -60.f * (float)GE_C_DEG_TO_RAD));
	Matrix4x4<float> rotation = yawRotation * pitchRotation;

	// This will give us the "look at" vector 
	// in world space - we'll use that to move.
	Vector4<float> atWorld = Vector4<float>::Unit(PITCH); // forward vector
#if defined(GE_USE_MAT_VEC)
	atWorld = rotation * atWorld;
#else
	atWorld = atWorld * rotation;
#endif

	Transform start;
	start.SetRotation(rotation);
#if defined(GE_USE_MAT_VEC)
	start.SetTranslation(mAbsoluteTransform.GetTranslationW1() +
		yawRotation * Vector4<float>::Unit(0) * 10.f + Vector4<float>::Unit(2) * 30.f);
#else
	start.SetTranslation(mAbsoluteTransform.GetTranslationW1() +
		Vector4<float>::Unit(0) * yawRotation * 10.f + Vector4<float>::Unit(2) * 30.f);
#endif


	Transform end;
	end.SetRotation(rotation);
	end.SetTranslation(mAbsoluteTransform.GetTranslationW1() + atWorld * 500.f);

	Vector3<float> collision, collisionNormal;
	collision = end.GetTranslation();
	ActorId actorId = GameLogic::Get()->GetGamePhysics()->ConvexSweep(
		mPlayerId, start, end, collision, collisionNormal);

	//Check whether we are close to a cliff
	if (abs(collision[2] - position[2]) > 50.f)
	{
		//Choose randomly which way too look for getting out the cliff
		int sign = Randomizer::Rand() % 2 ? 1 : -1;

		// Smoothly turn 110º and check raycasting until we meet a minimum distance
		for (int angle = 1; angle <= 110; angle++)
		{
			rotation = Rotation<4, float>(
				AxisAngle<4, float>(Vector4<float>::Unit(2),
				(mYaw + angle * sign) * (float)GE_C_DEG_TO_RAD));

			atWorld = Vector4<float>::Unit(PITCH); // forward vector
#if defined(GE_USE_MAT_VEC)
			atWorld = rotation * atWorld;
#else
			atWorld = atWorld * rotation;
#endif

			start.SetRotation(rotation);
			end.SetRotation(rotation);
			end.SetTranslation(mAbsoluteTransform.GetTranslationW1() + atWorld * 500.f);

			collision = end.GetTranslation();
			ActorId actorId = GameLogic::Get()->GetGamePhysics()->ConvexSweep(
				mPlayerId, start, end, collision, collisionNormal);
			if (abs(collision[2] - position[2]) <= 50.f)
			{
				mOrientation = Randomizer::Rand() % 2 ? 1 : -1;
				mYaw += angle * sign;
				return atWorld;
			}
		}

		//If we haven't find a way out we proceed exactly the same but in the opposite direction
		sign *= -1;
		for (int angle = 1; angle <= 110; angle++)
		{
			rotation = Rotation<4, float>(
				AxisAngle<4, float>(Vector4<float>::Unit(2),
				(mYaw + angle * sign) * (float)GE_C_DEG_TO_RAD));

			atWorld = Vector4<float>::Unit(PITCH); // forward vector
#if defined(GE_USE_MAT_VEC)
			atWorld = rotation * atWorld;
#else
			atWorld = atWorld * rotation;
#endif

			start.SetRotation(rotation);
			end.SetRotation(rotation);
			end.SetTranslation(mAbsoluteTransform.GetTranslationW1() + atWorld * 500.f);

			collision = end.GetTranslation();
			ActorId actorId = GameLogic::Get()->GetGamePhysics()->ConvexSweep(
				mPlayerId, start, end, collision, collisionNormal);
			if (abs(collision[2] - position[2]) <= 50.f)
			{
				mOrientation = Randomizer::Rand() % 2 ? 1 : -1;
				mYaw += angle * sign;
				return atWorld;
			}
		}

		//if we couldnt find any way out then we make a hard turn
		mOrientation = Randomizer::Rand() % 2 ? 1 : -1;
		mYaw += 130.f * sign;
		rotation = Rotation<4, float>(
			AxisAngle<4, float>(Vector4<float>::Unit(2), mYaw * (float)GE_C_DEG_TO_RAD));

		atWorld = Vector4<float>::Unit(PITCH); // forward vector
#if defined(GE_USE_MAT_VEC)
		atWorld = rotation * atWorld;
#else
		atWorld = atWorld * rotation;
#endif
		return atWorld;
	}

	return atWorld;
}

//  class QuakeAIView::OnUpdate			- Chapter 10, page 283
void QuakeAIView::OnUpdate(unsigned int timeMs, unsigned long deltaMs)
{
	//Handling rotation as a result of mouse position
	Matrix4x4<float> rotation;

	eastl::shared_ptr<PlayerActor> pPlayerActor(
		eastl::dynamic_shared_pointer_cast<PlayerActor>(
		GameLogic::Get()->GetActor(mPlayerId).lock()));
	if (!pPlayerActor) return;

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
			pPlayerActor->GetAction().actionType |= ACTION_MOVEFORWARD;

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
				else
				{
					mPitchTarget = eastl::max(-85.f, eastl::min(85.f, mPitchTarget));
					mPitch = 90 * ((mPitchTarget + 85.f) / 170.f) - 45.f;

					eastl::shared_ptr<TransformComponent> pTransformComponent(
						pPlayerActor->GetComponent<TransformComponent>(TransformComponent::Name).lock());
					if (pTransformComponent)
					{
						// Calculate the new rotation matrix from the camera
						// yaw and pitch (zrotate and xrotate).
						Matrix4x4<float> yawRotation = Rotation<4, float>(
							AxisAngle<4, float>(Vector4<float>::Unit(2), mYaw * (float)GE_C_DEG_TO_RAD));
						rotation = -yawRotation;
						Matrix4x4<float> pitchRotation = Rotation<4, float>(
							AxisAngle<4, float>(Vector4<float>::Unit(1), mPitch * (float)GE_C_DEG_TO_RAD));
						mAbsoluteTransform.SetRotation(yawRotation * pitchRotation);
						mAbsoluteTransform.SetTranslation(pTransformComponent->GetPosition());

						Vector4<float> direction;
						direction = Stationary(deltaMs);
						direction = Smooth();
						direction = Cliff();
						Normalize(direction);

						direction *= mMoveSpeed;
						velocity = direction;

						// update node rotation matrix
						pitchRotation = Rotation<4, float>(
							AxisAngle<4, float>(Vector4<float>::Unit(1), mPitchTarget * (float)GE_C_DEG_TO_RAD));
						pTransformComponent->SetRotation(yawRotation * pitchRotation);
					}
				}
				pPlayerActor->GetAction().actionType |= ACTION_RUN;
			}
			else
			{
				mFallSpeed += deltaMs / (pPhysicComponent->GetJumpSpeed() * 0.5f);
				if (mFallSpeed > mMaxFallSpeed) mFallSpeed = mMaxFallSpeed;

				Vector4<float> direction = -Vector4<float>::Unit(YAW);
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
					eastl::make_shared<QuakeEventDataRotateActor>(mPlayerId, mAbsoluteTransform));

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
}