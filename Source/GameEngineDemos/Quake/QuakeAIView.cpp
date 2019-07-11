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

#include "AI/AIManager.h"

#include "QuakePlayerController.h"
#include "QuakeAIManager.h"
#include "QuakeAIView.h"
#include "QuakeEvents.h"
#include "QuakeApp.h"
#include "Quake.h"

///////////////////////////////////////////////////////////////////////////////
//
// QuakeAIView::QuakeAIView
//

QuakeAIView::QuakeAIView()
	: BaseGameView()
{
	mYaw = 0.0f;
	mYawSmooth = 0.0f;
	mYawSmoothTime = 0.0f;
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

	mCurrentAction = 0;
	mCurrentActionTime = 0.f;
	mCurrentActor = INVALID_ACTOR_ID;

	mCurrentNode = NULL;
	mCurrentArc = NULL;
	mGoalNode= NULL;

	mViewId = INVALID_GAME_VIEW_ID;
	mPlayerId = INVALID_ACTOR_ID;
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

	mPathingGraph = GameLogic::Get()->GetAIManager()->GetPathingGraph();
	eastl::shared_ptr<Actor> pGameActor(GameLogic::Get()->GetActor(mPlayerId).lock());
	eastl::shared_ptr<TransformComponent> pTransformComponent(
		pGameActor->GetComponent<TransformComponent>(TransformComponent::Name).lock());
	if (pTransformComponent)
	{
		EulerAngles<float> yawPitchRoll;
		yawPitchRoll.mAxis[1] = 1;
		yawPitchRoll.mAxis[2] = 2;
		pTransformComponent->GetTransform().GetRotation(yawPitchRoll);

		mYaw = mYawSmooth = yawPitchRoll.mAngle[YAW] * (float)GE_C_RAD_TO_DEG;
		mPitchTarget = yawPitchRoll.mAngle[ROLL] * (float)GE_C_RAD_TO_DEG;

		mAbsoluteTransform.SetRotation(pTransformComponent->GetRotation());
		mAbsoluteTransform.SetTranslation(pTransformComponent->GetPosition());
	}
}

//Stationary movement
void QuakeAIView::Stationary(unsigned long deltaMs)
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

	Vector3<float> scale =
		GameLogic::Get()->GetGamePhysics()->GetScale(mPlayerId) / 2.f;

	Transform start;
	start.SetRotation(rotation);
	start.SetTranslation(mAbsoluteTransform.GetTranslationW1() 
		+ scale[YAW] * Vector4<float>::Unit(YAW));

	Transform end;
	end.SetRotation(rotation);
	end.SetTranslation(mAbsoluteTransform.GetTranslationW1() +
		atWorld * 500.f + scale[YAW] * Vector4<float>::Unit(YAW));

	Vector3<float> collision, collisionNormal;
	collision = end.GetTranslation();
	ActorId actorId = GameLogic::Get()->GetGamePhysics()->ConvexSweep(
		mPlayerId, start, end, collision, collisionNormal);
	if (Length(collision - position) < 50.f)
	{
		mStationaryTime += deltaMs;
		if (mStationaryTime > 100)
		{
			//Choose randomly which way too look for obstacles
			int sign = Randomizer::Rand() % 2 ? 1 : -1;
			mYaw += 130.f * sign;
		}
	}
	else mStationaryTime = 0;
}

// Cliff control
void QuakeAIView::Cliff()
{
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

	Vector3<float> position = HProject(
		mAbsoluteTransform.GetTranslationW1() + atWorld * 10.f);

	Transform start;
	start.SetRotation(rotation);
	start.SetTranslation(position);

	Transform end;
	end.SetRotation(rotation);
	end.SetTranslation(mAbsoluteTransform.GetTranslationW1() + 
		atWorld * 10.f - Vector4<float>::Unit(2) * 300.f);

	Vector3<float> collision, collisionNormal;
	collision = end.GetTranslation();
	ActorId actorId = GameLogic::Get()->GetGamePhysics()->CastRay(
		start.GetTranslation(), end.GetTranslation(), collision, collisionNormal);

	//Check whether we are close to a cliff
	if (abs(collision[2] - position[2]) > 60.f)
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
			end.SetTranslation(mAbsoluteTransform.GetTranslationW1() +
				atWorld * 100.f - Vector4<float>::Unit(2) * 300.f);

			collision = end.GetTranslation();
			ActorId actorId = GameLogic::Get()->GetGamePhysics()->CastRay(
				start.GetTranslation(), end.GetTranslation(), collision, collisionNormal);
			if (abs(collision[2] - position[2]) <= 60.f)
			{
				mOrientation = Randomizer::Rand() % 2 ? 1 : -1;
				mYaw += angle * sign;
				return;
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
			end.SetTranslation(mAbsoluteTransform.GetTranslationW1() +
				atWorld * 100.f - Vector4<float>::Unit(2) * 300.f);

			collision = end.GetTranslation();
			ActorId actorId = GameLogic::Get()->GetGamePhysics()->CastRay(
				start.GetTranslation(), end.GetTranslation(), collision, collisionNormal);
			if (abs(collision[2] - position[2]) <= 60.f)
			{
				mOrientation = Randomizer::Rand() % 2 ? 1 : -1;
				mYaw += angle * sign;
				return;
			}
		}

		//if we couldnt find any way out then we make a hard turn
		mOrientation = Randomizer::Rand() % 2 ? 1 : -1;
		mYaw += 130.f * sign;
	}
}

//Movement
void QuakeAIView::Movement(unsigned long deltaMs)
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

	Vector3<float> scale =
		GameLogic::Get()->GetGamePhysics()->GetScale(mPlayerId) / 2.f;

	Transform start;
	start.SetRotation(rotation);
	start.SetTranslation(mAbsoluteTransform.GetTranslationW1() 
		+ scale[YAW] * Vector4<float>::Unit(YAW));

	Transform end;
	end.SetRotation(rotation);
	end.SetTranslation(mAbsoluteTransform.GetTranslationW1() +
		atWorld * 500.f + scale[YAW] * Vector4<float>::Unit(YAW));

	Vector3<float> collision, collisionNormal;
	collision = end.GetTranslation();
	ActorId actorId = GameLogic::Get()->GetGamePhysics()->ConvexSweep(
		mPlayerId, start, end, collision, collisionNormal);
	if (Length(collision - position) < 50.f)
	{
		//Choose randomly which way too look for obstacles
		int sign = mOrientation;

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
			end.SetTranslation(mAbsoluteTransform.GetTranslationW1() +
				atWorld * 500.f + scale[YAW] * Vector4<float>::Unit(YAW));

			collision = end.GetTranslation();
			actorId = GameLogic::Get()->GetGamePhysics()->ConvexSweep(
				mPlayerId, start, end, collision, collisionNormal);
			if (Length(collision - position) > 50.f)
			{
				mYaw += angle * sign;
				return;
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
			end.SetTranslation(mAbsoluteTransform.GetTranslationW1() +
				atWorld * 500.f + scale[YAW] * Vector4<float>::Unit(YAW));

			collision = end.GetTranslation();
			actorId = GameLogic::Get()->GetGamePhysics()->ConvexSweep(
				mPlayerId, start, end, collision, collisionNormal);
			if (Length(collision - position) > 50.f)
			{
				mYaw += angle * sign;
				return;
			}
		}
	}
}

//Smooth movement
void QuakeAIView::Smooth(unsigned long deltaMs)
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

	Vector3<float> scale = 
		GameLogic::Get()->GetGamePhysics()->GetScale(mPlayerId) / 2.f;

	Transform start;
	start.SetRotation(rotation);
	start.SetTranslation(mAbsoluteTransform.GetTranslationW1() 
		+ scale[YAW] * Vector4<float>::Unit(YAW));

	Transform end;
	end.SetRotation(rotation);
	end.SetTranslation(mAbsoluteTransform.GetTranslationW1() + 
		atWorld * 500.f + scale[YAW] * Vector4<float>::Unit(YAW));

	Vector3<float> collision, collisionNormal;
	collision = end.GetTranslation();
	ActorId actorId = GameLogic::Get()->GetGamePhysics()->ConvexSweep(
		mPlayerId, start, end, collision, collisionNormal);
	if (Length(collision - position) < 80.f)
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
			end.SetTranslation(mAbsoluteTransform.GetTranslationW1() + 
				atWorld * 500.f + scale[YAW] * Vector4<float>::Unit(YAW));

			collision = end.GetTranslation();
			actorId = GameLogic::Get()->GetGamePhysics()->ConvexSweep(
				mPlayerId, start, end, collision, collisionNormal);
			if (Length(collision - position) > 80.f)
			{
				mOrientation = Randomizer::Rand() % 2 ? 1 : -1;
				mYaw += angle * sign;
				return;
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
			end.SetTranslation(mAbsoluteTransform.GetTranslationW1() + 
				atWorld * 500.f + scale[YAW] * Vector4<float>::Unit(YAW));

			collision = end.GetTranslation();
			actorId = GameLogic::Get()->GetGamePhysics()->ConvexSweep(
				mPlayerId, start, end, collision, collisionNormal);
			if (Length(collision - position) > 80.f)
			{
				mOrientation = Randomizer::Rand() % 2 ? 1 : -1;
				mYaw += angle * sign;
				return;
			}
		}

		//if we couldnt find any way out the stationary function will take care of it.
		mOrientation = Randomizer::Rand() % 2 ? 1 : -1;
	}
	else
	{
		mYaw += 0.03f * deltaMs * mOrientation;
	}
}


//  class QuakeAIView::OnUpdate			- Chapter 10, page 283
void QuakeAIView::OnUpdate(unsigned int timeMs, unsigned long deltaMs)
{
	QuakeAIManager* aiManager =
		dynamic_cast<QuakeAIManager*>(GameLogic::Get()->GetAIManager());
	if (!aiManager->IsEnable()) return;

	mYawSmoothTime += deltaMs / 1000.f;
	mCurrentActionTime -= deltaMs / 1000.f;

	eastl::shared_ptr<PlayerActor> pPlayerActor(
		eastl::dynamic_shared_pointer_cast<PlayerActor>(
		GameLogic::Get()->GetActor(mPlayerId).lock()));
	if (!pPlayerActor) return;

	eastl::shared_ptr<TransformComponent> pTransformComponent(
		pPlayerActor->GetComponent<TransformComponent>(TransformComponent::Name).lock());
	if (pTransformComponent)
	{
		if (mPathingGraph && mCurrentArc != NULL)
		{
			if (pPlayerActor->GetAction().triggerPush != INVALID_ACTOR_ID ||
				pPlayerActor->GetAction().triggerTeleporter != INVALID_ACTOR_ID)
			{
				if (pPlayerActor->GetAction().triggerPush == mCurrentArc->GetNode()->GetActorId() ||
					pPlayerActor->GetAction().triggerTeleporter == mCurrentArc->GetNode()->GetActorId())
				{
					Vector3<float> currentPosition = pTransformComponent->GetPosition();
					if (mCurrentPlan.size())
					{
						PathingArcVec::iterator itArc = mCurrentPlan.begin();

						do
						{
							mCurrentArc = (*itArc);
							mCurrentAction = mCurrentArc->GetType();
							mCurrentNode = mCurrentArc->GetNode();
							mCurrentActionTime = mCurrentArc->GetWeight() + 2.0f;
							Vector3<float> direction = mCurrentNode->GetPos() - currentPosition;
							Normalize(direction);
							mYaw = atan2(direction[1], direction[0]) * (float)GE_C_RAD_TO_DEG;

							if (mCurrentAction == GAT_PUSH || mCurrentAction == GAT_TELEPORT)
							{
								itArc++;
								if (itArc != mCurrentPlan.end())
								{
									mCurrentArc = (*itArc);
									mCurrentAction = mCurrentArc->GetType();
									mCurrentNode = mCurrentArc->GetNode();

									Vector3<float> direction = mCurrentNode->GetPos() - currentPosition;
									Normalize(direction);
									mYaw = atan2(direction[1], direction[0]) * (float)GE_C_RAD_TO_DEG;
								}
								break;
							}
							else itArc++;
						} while (itArc != mCurrentPlan.end());

						mCurrentPlan.erase(mCurrentPlan.begin(), itArc);
					}
					else if (mGoalNode != NULL)
					{
						do
						{
							PathingCluster* currentCluster = mCurrentNode->FindCluster(GAT_JUMP, mGoalNode);
							if (currentCluster != NULL)
							{
								PathingArc* clusterArc = mCurrentNode->FindArc(currentCluster->GetNode());
								PathingNode* clusterNode = clusterArc->GetNode();
								unsigned int clusterArcType = clusterArc->GetType();

								mCurrentArc = clusterArc;
								mCurrentAction = clusterArcType;
								mCurrentNode = clusterArc->GetNode();
								mCurrentActionTime = clusterArc->GetWeight() + 2.0f;
								Vector3<float> direction = clusterNode->GetPos() - currentPosition;
								Normalize(direction);
								mYaw = atan2(direction[1], direction[0]) * (float)GE_C_RAD_TO_DEG;
							}
							else mCurrentNode = mGoalNode;

							if (mCurrentAction == GAT_PUSH || mCurrentAction == GAT_TELEPORT)
							{
								if (mCurrentNode != mGoalNode)
								{
									currentCluster = mCurrentNode->FindCluster(GAT_JUMP, mGoalNode);
									mCurrentArc = mCurrentNode->FindArc(currentCluster->GetNode());
									mCurrentAction = mCurrentArc->GetType();
									mCurrentNode = mCurrentArc->GetNode();

									Vector3<float> direction = mCurrentNode->GetPos() - currentPosition;
									Normalize(direction);
									mYaw = atan2(direction[1], direction[0]) * (float)GE_C_RAD_TO_DEG;
								}
								break;
							}
						} while (mCurrentNode != mGoalNode);

						PathingNode* currentNode = mCurrentNode;
						if (currentNode != mGoalNode)
						{
							//printf("\n random trigger nodes %u : ", mPlayerId);
							do
							{
								PathingCluster* currentCluster = currentNode->FindCluster(GAT_JUMP, mGoalNode);
								PathingArc* clusterArc = currentNode->FindArc(currentCluster->GetNode());
								currentNode = clusterArc->GetNode();
								//printf("%u ", currentNode->GetId());
							} while (currentNode != mGoalNode);
						}
					}
				}
			}
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
			pTransformComponent->SetTransform(pTeleporterTrigger->GetTarget());

			mYaw = mYawSmooth = yawPitchRoll.mAngle[YAW] * (float)GE_C_RAD_TO_DEG;
			mPitchTarget = -yawPitchRoll.mAngle[ROLL] * (float)GE_C_RAD_TO_DEG;

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
				pPlayerActor->GetAction().actionType = 0;
				pPlayerActor->GetAction().actionType |= ACTION_MOVEFORWARD;

				if (aiManager->GetPlayerWeapon(mPlayerId) != WP_NONE)
					pPlayerActor->ChangeWeapon(aiManager->GetPlayerWeapon(mPlayerId));

				Vector4<float> velocity = Vector4<float>::Zero();
				if (pPhysicComponent->OnGround())
				{
					mFallSpeed = 0.0f;

					if (pPlayerActor->GetAction().triggerPush != INVALID_ACTOR_ID)
					{
						float push;
						Vector3<float> direction;
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

						direction[PITCH] *= push / 90.f;
						direction[ROLL] *= push / 90.f;
						direction[YAW] = push / 30.f;
						velocity = HLift(direction, 0.f);

						pPlayerActor->GetAction().actionType |= ACTION_JUMP;
					}
					else
					{
						mPitchTarget = 0.f;
						mPitchTarget = eastl::max(-85.f, eastl::min(85.f, mPitchTarget));
						mPitch = 90 * ((mPitchTarget + 85.f) / 170.f) - 45.f;

						if (mPathingGraph)
						{
							Vector3<float> currentPosition = pTransformComponent->GetPosition();
							PathingNode* currentNode = aiManager->GetPathingGraph()->FindClosestNode(currentPosition);
							if (mCurrentNode == NULL)
								mCurrentNode = currentNode;

							bool searchNode = false;
							if (mCurrentNode->GetActorId() != INVALID_ACTOR_ID)
							{
								eastl::shared_ptr<Actor> pItemActor(
									eastl::dynamic_shared_pointer_cast<Actor>(
									GameLogic::Get()->GetActor(mCurrentNode->GetActorId()).lock()));
								if (pItemActor->GetType() == "Trigger")
								{
									if (mCurrentAction == GAT_PUSH || mCurrentAction == GAT_TELEPORT)
									{
										eastl::shared_ptr<TransformComponent> pTriggerTransform =
											pItemActor->GetComponent<TransformComponent>(TransformComponent::Name).lock();
										Vector3<float> diff = pTriggerTransform->GetPosition() - currentPosition;
										diff[YAW] = 0.f;

										if (Length(diff) == 0.f)
										{
											searchNode = true;
											if (!mCurrentPlan.empty())
												mCurrentPlan.erase(mCurrentPlan.begin());
										}
									}
									else
									{
										Vector3<float> diff = mCurrentNode->GetPos() - currentPosition;
										if (Length(diff) <= 6.0f)
										{
											searchNode = true;
											if (!mCurrentPlan.empty())
												mCurrentPlan.erase(mCurrentPlan.begin());
										}
									}
								}
								else if (mCurrentActor != mCurrentNode->GetActorId())
								{
									eastl::shared_ptr<TransformComponent> pTransform =
										pItemActor->GetComponent<TransformComponent>(TransformComponent::Name).lock();
									Vector3<float> diff = pTransform->GetPosition() - currentPosition;
									diff[YAW] = 0.f;
									
									if (Length(diff) <= 6.0f)
									{
										searchNode = true;
										if (!mCurrentPlan.empty())
											mCurrentPlan.erase(mCurrentPlan.begin());
										mCurrentActor = mCurrentNode->GetActorId();
									}

								}
								else
								{
									Vector3<float> diff = mCurrentNode->GetPos() - currentPosition;
									if (Length(diff) <= 6.0f)
									{
										searchNode = true;
										if (!mCurrentPlan.empty())
											mCurrentPlan.erase(mCurrentPlan.begin());
									}
								}
							}
							else
							{
								Vector3<float> diff = mCurrentNode->GetPos() - currentPosition;
								if (Length(diff) <= 6.0f)
								{
									searchNode = true;
									if (!mCurrentPlan.empty())
										mCurrentPlan.erase(mCurrentPlan.begin());
								}
							}

							if (mCurrentActionTime <= 0.f)
							{
								printf("\n current decision making player %u : ", mPlayerId);
								for (PathingArc* pathArc : mCurrentPlan)
									printf("%u ", pathArc->GetNode()->GetId());

								if (mCurrentArc != NULL)
								{
									printf("\n fail arc id %u type %u node %u \n ",
										mCurrentArc->GetId(), mCurrentArc->GetType(), mCurrentArc->GetNode()->GetId());
									LogInformation("fail arc id " + eastl::to_string(mCurrentArc->GetId()) +
										" type " + eastl::to_string(mCurrentArc->GetType()) + " node " +
										eastl::to_string(mCurrentArc->GetNode()->GetId()));
									aiManager->mLogError << ("fail arc id " +
										eastl::to_string(mCurrentArc->GetId()) + " type " +
										eastl::to_string(mCurrentArc->GetType()) + " node " +
										eastl::to_string(mCurrentArc->GetNode()->GetId())).c_str() << std::endl;
								}
								else
								{
									printf("\n no arc \n ");
									LogInformation("no arc");
								}

								mCurrentPlan.clear();
								mCurrentNode = currentNode;

								mCurrentActor = INVALID_ACTOR_ID;
								mCurrentActionTime = 0.f;
								mCurrentAction = 0;
								mCurrentArc = 0;

								mGoalNode = NULL;
								searchNode = true;
							}

							if (searchNode)
							{
								PathingArcVec pathPlan;
								if (aiManager->IsPlayerUpdated(mPlayerId))
									aiManager->GetPlayerPath(mPlayerId, pathPlan);

								if (pathPlan.size())
								{
									PathingNodeVec searchNodes;
									for (PathingArc* pathArc : pathPlan)
										searchNodes.push_back(pathArc->GetNode());

									PathPlan* plan = aiManager->GetPathingGraph()->FindPath(mCurrentNode, searchNodes);
									if (plan)
									{
										plan->ResetPath();

										PathingArcVec path;
										if (!plan->CheckForEnd())
										{
											PathingNode* node = plan->GetArcs().back()->GetNode();
											for (PathingArc* planArc : plan->GetArcs())
												path.push_back(planArc);

											PathingArcVec::iterator itArc;
											PathingArcVec::iterator itPathArc = pathPlan.begin();
											for (itArc = pathPlan.begin(); itArc != pathPlan.end(); itArc++)
												if ((*itArc)->GetNode() == node)
													itPathArc = itArc;

											for (itPathArc++; itPathArc != pathPlan.end(); itPathArc++)
												path.push_back((*itPathArc));

											//printf("\n found new plan %u : ", mPlayerId);
											aiManager->SetPlayerUpdated(mPlayerId, false);
										}
										else if (eastl::find(searchNodes.begin(), searchNodes.end(), mCurrentNode))
										{
											PathingArcVec::iterator itArc;
											PathingArcVec::iterator itPathArc = pathPlan.begin();
											for (itArc = pathPlan.begin(); itArc != pathPlan.end(); itArc++)
												if ((*itArc)->GetNode() == mCurrentNode)
													itPathArc = itArc;

											for (itPathArc++; itPathArc != pathPlan.end(); itPathArc++)
												path.push_back((*itPathArc));

											//printf("\n found new plan %u : ", mPlayerId);
											aiManager->SetPlayerUpdated(mPlayerId, false);
										}

										mCurrentPlan = path;
										delete plan;
									}
								}

								if (!mCurrentPlan.empty())
								{
									PathingArcVec::iterator itArc = mCurrentPlan.begin();
	
									mCurrentArc = (*itArc);
									mCurrentAction = mCurrentArc->GetType();
									if (mCurrentAction != GAT_PUSH && mCurrentAction != GAT_TELEPORT)
										mCurrentNode = mCurrentArc->GetNode();
									mCurrentActionTime = mCurrentArc->GetWeight() + 1.0f;
									Vector3<float> direction = mCurrentNode->GetPos() - currentPosition;
									Normalize(direction);
									mYaw = atan2(direction[1], direction[0]) * (float)GE_C_RAD_TO_DEG;

									mGoalNode = NULL;
									searchNode = false;
								}

								if (searchNode)
								{
									PathingNode* currentNode = mCurrentNode;
									PathingClusterVec clusterNodes;
									currentNode->GetClusters(GAT_JUMP, clusterNodes);

									if (mGoalNode == NULL || mGoalNode == currentNode)
									{
										PathingArcVec pathPlan;
										aiManager->GetPlayerPath(mPlayerId, pathPlan);

										if (pathPlan.size())
										{
											/*
											printf("\n original plan player %u : ", mPlayerId);
											for (PathingArc* pathArc : pathPlan)
												printf("%u ", pathArc->GetNode()->GetId());
											*/
											PathingArcVec::iterator itPathPlan;
											for (itPathPlan = pathPlan.begin(); itPathPlan != pathPlan.end(); itPathPlan++)
											{
												PathingArc* pathArc = (*itPathPlan);
												for (PathingCluster* clusterNode : clusterNodes)
												{
													if (currentNode->GetCluster() != clusterNode->GetNode()->GetCluster() &&
														pathArc->GetNode()->GetCluster() == clusterNode->GetNode()->GetCluster())
													{
														//printf("\n plan node %u : ", mPlayerId);

														mGoalNode = clusterNode->GetTarget();

														PathingArcVec newPathPlan;
														for (itPathPlan++; itPathPlan != pathPlan.end(); itPathPlan++)
															newPathPlan.push_back((*itPathPlan));
														aiManager->SetPlayerPath(mPlayerId, newPathPlan);
														searchNode = false;
														break;
													}
												}

												if (!searchNode) break;
											}
										}
									}

									if (mGoalNode == NULL || mGoalNode == currentNode)
									{
										//printf("\n random node %u : ", mPlayerId);

										// choose a random cluster
										do
										{
											if (!clusterNodes.empty())
											{
												unsigned int cluster = Randomizer::Rand() % clusterNodes.size();
												mGoalNode = clusterNodes[cluster]->GetTarget();
											}
											else
											{
												mGoalNode = NULL;
												break;
											}
										} while (currentNode == mGoalNode || mGoalNode->GetArcs().empty());
									}

									if (mGoalNode != NULL)
									{
										float minPosDiff = FLT_MAX;
										PathingCluster* currentCluster = currentNode->FindCluster(GAT_JUMP, mGoalNode);
										if (currentCluster != NULL)
										{
											PathingArc* clusterArc = currentNode->FindArc(currentCluster->GetNode());
											PathingNode* clusterNode = clusterArc->GetNode();
											unsigned int clusterArcType = clusterArc->GetType();

											mCurrentArc = clusterArc;
											mCurrentAction = clusterArcType;
											if (mCurrentAction != GAT_PUSH && mCurrentAction != GAT_TELEPORT)
												mCurrentNode = clusterArc->GetNode();
											mCurrentActionTime = clusterArc->GetWeight() + 1.0f;
											Vector3<float> direction = clusterNode->GetPos() - currentPosition;
											Normalize(direction);
											mYaw = atan2(direction[1], direction[0]) * (float)GE_C_RAD_TO_DEG;

											//printf("\n new plan %u : ", mPlayerId);
											do
											{
												currentCluster = currentNode->FindCluster(GAT_JUMP, mGoalNode);
												clusterArc = currentNode->FindArc(currentCluster->GetNode());
												currentNode = clusterArc->GetNode();
												//printf("%u ", currentNode->GetId());
											} while (currentNode != mGoalNode);
										}
										else
										{
											mGoalNode = NULL;
											mCurrentNode = NULL;

											mCurrentActor = INVALID_ACTOR_ID;
											mCurrentActionTime = 0.f;
											mCurrentAction = 0;
											mCurrentArc = 0;
											mCurrentPlan.clear();
										}
									}
									else
									{
										Stationary(deltaMs);
										Smooth(deltaMs);
										Cliff();
									}
								}
							}
							else
							{
								if (mCurrentAction == GAT_JUMP || mCurrentAction == GAT_PUSH || mCurrentAction == GAT_TELEPORT)
								{
									eastl::shared_ptr<Actor> pItemActor(
										eastl::dynamic_shared_pointer_cast<Actor>(
										GameLogic::Get()->GetActor(mCurrentNode->GetActorId()).lock()));
									if (pItemActor && pItemActor->GetType() == "Trigger")
									{
										eastl::shared_ptr<TransformComponent> pTriggerTransform =
											pItemActor->GetComponent<TransformComponent>(TransformComponent::Name).lock();

										Vector3<float> direction = pTriggerTransform->GetPosition() - currentPosition;
										Normalize(direction);
										mYaw = atan2(direction[1], direction[0]) * (float)GE_C_RAD_TO_DEG;

										Stationary(deltaMs);
										Movement(deltaMs);
									}
									else
									{
										if (mCurrentAction == GAT_JUMP)
										{
											pPlayerActor->GetAction().actionType |= ACTION_JUMP;
											mCurrentAction = 0;
										}

										Vector3<float> direction = mCurrentNode->GetPos() - currentPosition;
										Normalize(direction);
										mYaw = atan2(direction[1], direction[0]) * (float)GE_C_RAD_TO_DEG;
									}
								}
								else
								{
									if (mCurrentNode->GetActorId() != INVALID_ACTOR_ID)
									{
										eastl::shared_ptr<Actor> pItemActor(
											eastl::dynamic_shared_pointer_cast<Actor>(
											GameLogic::Get()->GetActor(mCurrentNode->GetActorId()).lock()));

										if (mCurrentActor != mCurrentNode->GetActorId())
										{
											eastl::shared_ptr<TransformComponent> pTransform =
												pItemActor->GetComponent<TransformComponent>(TransformComponent::Name).lock();

											Vector3<float> direction = pTransform->GetPosition() - currentPosition;
											Normalize(direction);
											mYaw = atan2(direction[1], direction[0]) * (float)GE_C_RAD_TO_DEG;
										}
										else
										{
											Vector3<float> direction = mCurrentNode->GetPos() - currentPosition;
											Normalize(direction);
											mYaw = atan2(direction[1], direction[0]) * (float)GE_C_RAD_TO_DEG;
										}
									}
									else
									{
										Vector3<float> scale =
											GameLogic::Get()->GetGamePhysics()->GetScale(mPlayerId) / 2.f;

										Transform start;
										start.SetTranslation(currentPosition + scale[YAW] * Vector3<float>::Unit(YAW));
										Transform end;
										end.SetTranslation(mCurrentNode->GetPos() + scale[YAW] * Vector3<float>::Unit(YAW));

										Vector3<float> collision, collisionNormal;
										ActorId actorId = GameLogic::Get()->GetGamePhysics()->ConvexSweep(
											mPlayerId, start, end, collision, collisionNormal);
										if (collision == NULL || actorId != INVALID_ACTOR_ID)
										{
											Vector3<float> direction = mCurrentNode->GetPos() - currentPosition;
											Normalize(direction);
											mYaw = atan2(direction[1], direction[0]) * (float)GE_C_RAD_TO_DEG;
										}
										else
										{
											Stationary(deltaMs);
											Movement(deltaMs);
										}
									}
								}
							}
						}
						else
						{
							Stationary(deltaMs);
							Smooth(deltaMs);
							Cliff();
						}

						// Calculate the new rotation matrix from the camera
						// yaw and pitch (zrotate and xrotate).
						Matrix4x4<float> yawRotation = Rotation<4, float>(
							AxisAngle<4, float>(Vector4<float>::Unit(2), mYaw * (float)GE_C_DEG_TO_RAD));
						Matrix4x4<float> rotation = yawRotation;
						Matrix4x4<float> pitchRotation = Rotation<4, float>(
							AxisAngle<4, float>(Vector4<float>::Unit(1), mPitch * (float)GE_C_DEG_TO_RAD));

						//smoothing camera rotation
						if (abs(mYawSmooth - mYaw) < 90)
						{
							if (mYaw - mYawSmooth > 180)
								mYawSmooth--;
							else if (mYaw - mYawSmooth < -180)
								mYawSmooth++;
							else if (mYaw > mYawSmooth)
								mYawSmooth++;
							else if (mYaw < mYawSmooth)
								mYawSmooth--;
						}
						else if (mYawSmoothTime >= 0.5f)
						{
							mYawSmooth = mYaw;
							mYawSmoothTime = 0.f;
						}

						yawRotation = Rotation<4, float>(
							AxisAngle<4, float>(Vector4<float>::Unit(2), mYawSmooth * (float)GE_C_DEG_TO_RAD));

						mAbsoluteTransform.SetRotation(yawRotation * pitchRotation);
						mAbsoluteTransform.SetTranslation(pTransformComponent->GetPosition());

						// This will give us the "look at" vector 
						// in world space - we'll use that to move.
						Vector4<float> atWorld = Vector4<float>::Unit(PITCH); // forward vector
#if defined(GE_USE_MAT_VEC)
						atWorld = rotation * atWorld;
#else
						atWorld = atWorld * rotation;
#endif
						Normalize(atWorld);

						if (pPlayerActor->GetAction().actionType & ACTION_JUMP)
						{
							Vector4<float> upWorld = Vector4<float>::Unit(YAW);
							Vector4<float> direction = atWorld + upWorld;
							Normalize(direction);

							direction[PITCH] *= mJumpMoveSpeed;
							direction[ROLL] *= mJumpMoveSpeed;
							direction[YAW] *= mJumpSpeed;
							velocity = direction;
						}
						else
						{
							atWorld *= mMoveSpeed;
							velocity = atWorld;
						}

						ActorId playerTarget = aiManager->GetPlayerTarget(mPlayerId);
						if (playerTarget != INVALID_ACTOR_ID)
						{
							eastl::shared_ptr<PlayerActor> pPlayerTarget(
								eastl::dynamic_shared_pointer_cast<PlayerActor>(
								GameLogic::Get()->GetActor(playerTarget).lock()));

							if (pPlayerTarget->GetState().stats[STAT_HEALTH] > 0)
							{
								//set muzzle location relative to pivoting eye
								Vector3<float> playerPos = pTransformComponent->GetTransform().GetTranslation();
								playerPos += Vector3<float>::Unit(YAW) * (float)pPlayerActor->GetState().viewHeight;

								eastl::shared_ptr<TransformComponent> pTargetTransform(
									pPlayerTarget->GetComponent<TransformComponent>(TransformComponent::Name).lock());

								Vector3<float> targetPos = pTargetTransform->GetTransform().GetTranslation();
								//targetPos += Vector3<float>::Unit(YAW) * (float)pPlayerTarget->GetState().viewHeight;

								eastl::vector<ActorId> collisionActors;
								eastl::vector<Vector3<float>> collisions, collisionNormals;
								GameLogic::Get()->GetGamePhysics()->CastRay(
									playerPos, targetPos, collisionActors, collisions, collisionNormals);

								ActorId closestCollisionId = INVALID_ACTOR_ID;
								Vector3<float> closestCollision = targetPos;
								for (unsigned int i = 0; i < collisionActors.size(); i++)
								{
									if (collisionActors[i] != pPlayerActor->GetId())
									{
										if (closestCollision != NULL)
										{
											if (Length(closestCollision - playerPos) > Length(collisions[i] - playerPos))
											{
												closestCollisionId = collisionActors[i];
												closestCollision = collisions[i];
											}
										}
										else
										{
											closestCollisionId = collisionActors[i];
											closestCollision = collisions[i];
										}
									}
								}

								if (closestCollisionId == pPlayerTarget->GetId())
								{
									Vector3<float> direction = closestCollision - playerPos;
									float scale = Length(direction);
									Normalize(direction);

									mYaw = mYawSmooth = atan2(direction[1], direction[0]) * (float)GE_C_RAD_TO_DEG;
									mPitchTarget = -asin(direction[2]) * (float)GE_C_RAD_TO_DEG;

									mPitchTarget = eastl::max(-85.f, eastl::min(85.f, mPitchTarget));
									mPitch = 90 * ((mPitchTarget + 85.f) / 170.f) - 45.f;

									yawRotation = Rotation<4, float>(
										AxisAngle<4, float>(Vector4<float>::Unit(2), mYaw * (float)GE_C_DEG_TO_RAD));
									pitchRotation = Rotation<4, float>(
										AxisAngle<4, float>(Vector4<float>::Unit(1), mPitch * (float)GE_C_DEG_TO_RAD));
									mAbsoluteTransform.SetRotation(yawRotation * pitchRotation);

									pPlayerActor->GetAction().actionType |= ACTION_ATTACK;
								}
							}
						}

						// update node rotation matrix
						pitchRotation = Rotation<4, float>(
							AxisAngle<4, float>(Vector4<float>::Unit(1), mPitchTarget * (float)GE_C_DEG_TO_RAD));
						pTransformComponent->SetRotation(yawRotation * pitchRotation);
					}
					pPlayerActor->GetAction().actionType |= ACTION_RUN;
				}
				else
				{
					mFallSpeed += deltaMs / (pPhysicComponent->GetJumpSpeed() * 0.5f);
					if (mFallSpeed > mMaxFallSpeed) mFallSpeed = mMaxFallSpeed;

					// Calculate the new rotation matrix from the camera
					// yaw and pitch (zrotate and xrotate).
					Matrix4x4<float> yawRotation = Rotation<4, float>(
						AxisAngle<4, float>(Vector4<float>::Unit(2), mYaw * (float)GE_C_DEG_TO_RAD));
					Matrix4x4<float> rotation = yawRotation;
					Matrix4x4<float> pitchRotation = Rotation<4, float>(
						AxisAngle<4, float>(Vector4<float>::Unit(1), mPitch * (float)GE_C_DEG_TO_RAD));

					// This will give us the "look at" vector 
					// in world space - we'll use that to move.
					Vector4<float> atWorld = Vector4<float>::Unit(PITCH); // forward vector
#if defined(GE_USE_MAT_VEC)
					atWorld = rotation * atWorld;
#else
					atWorld = atWorld * rotation;
#endif
					Normalize(atWorld);

					Vector4<float> upWorld = -Vector4<float>::Unit(YAW);
					Vector4<float> direction = atWorld + upWorld;
					Normalize(direction);

					direction[PITCH] *= pPhysicComponent->GetJumpSpeed() * (mFallSpeed / 4.f);
					direction[ROLL] *= pPhysicComponent->GetJumpSpeed() * (mFallSpeed / 4.f);
					direction[YAW] = -pPhysicComponent->GetJumpSpeed() * mFallSpeed;
					velocity = direction;

					ActorId playerTarget = aiManager->GetPlayerTarget(mPlayerId);
					if (playerTarget != INVALID_ACTOR_ID)
					{
						eastl::shared_ptr<PlayerActor> pPlayerTarget(
							eastl::dynamic_shared_pointer_cast<PlayerActor>(
							GameLogic::Get()->GetActor(playerTarget).lock()));

						if (pPlayerTarget->GetState().stats[STAT_HEALTH] > 0)
						{
							//set muzzle location relative to pivoting eye
							Vector3<float> playerPos = pTransformComponent->GetTransform().GetTranslation();
							playerPos += Vector3<float>::Unit(YAW) * (float)pPlayerActor->GetState().viewHeight;

							eastl::shared_ptr<TransformComponent> pTargetTransform(
								pPlayerTarget->GetComponent<TransformComponent>(TransformComponent::Name).lock());

							Vector3<float> targetPos = pTargetTransform->GetTransform().GetTranslation();
							//targetPos += Vector3<float>::Unit(YAW) * (float)pPlayerTarget->GetState().viewHeight;

							eastl::vector<ActorId> collisionActors;
							eastl::vector<Vector3<float>> collisions, collisionNormals;
							GameLogic::Get()->GetGamePhysics()->CastRay(
								playerPos, targetPos, collisionActors, collisions, collisionNormals);

							ActorId closestCollisionId = INVALID_ACTOR_ID;
							Vector3<float> closestCollision = targetPos;
							for (unsigned int i = 0; i < collisionActors.size(); i++)
							{
								if (collisionActors[i] != pPlayerActor->GetId())
								{
									if (closestCollision != NULL)
									{
										if (Length(closestCollision - playerPos) > Length(collisions[i] - playerPos))
										{
											closestCollisionId = collisionActors[i];
											closestCollision = collisions[i];
										}
									}
									else
									{
										closestCollisionId = collisionActors[i];
										closestCollision = collisions[i];
									}
								}
							}

							if (closestCollisionId == pPlayerTarget->GetId())
							{
								Vector3<float> direction = closestCollision - playerPos;
								float scale = Length(direction);
								Normalize(direction);

								mYaw = mYawSmooth = atan2(direction[1], direction[0]) * (float)GE_C_RAD_TO_DEG;
								mPitchTarget = -asin(direction[2]) * (float)GE_C_RAD_TO_DEG;

								mPitchTarget = eastl::max(-85.f, eastl::min(85.f, mPitchTarget));
								mPitch = 90 * ((mPitchTarget + 85.f) / 170.f) - 45.f;

								yawRotation = Rotation<4, float>(
									AxisAngle<4, float>(Vector4<float>::Unit(2), mYaw * (float)GE_C_DEG_TO_RAD));
								pitchRotation = Rotation<4, float>(
									AxisAngle<4, float>(Vector4<float>::Unit(1), mPitch * (float)GE_C_DEG_TO_RAD));
								mAbsoluteTransform.SetRotation(yawRotation * pitchRotation);

								pPlayerActor->GetAction().actionType |= ACTION_ATTACK;

								// update node rotation matrix
								pitchRotation = Rotation<4, float>(
									AxisAngle<4, float>(Vector4<float>::Unit(1), mPitchTarget * (float)GE_C_DEG_TO_RAD));
								pTransformComponent->SetRotation(yawRotation * pitchRotation);
							}
						}
					}

					pPlayerActor->GetAction().actionType |= ACTION_FALLEN;
				}

				if (pPlayerActor->GetState().moveType == PM_DEAD)
				{
					mGoalNode = NULL;
					mCurrentNode = NULL;

					mCurrentActor = INVALID_ACTOR_ID;
					mCurrentActionTime = 0.f;
					mCurrentAction = 0;
					mCurrentArc = 0;
					mCurrentPlan.clear();

					pPlayerActor->PlayerSpawn();
					pPlayerActor->GetAction().actionType = 0;
				}
				else
				{
					EventManager::Get()->TriggerEvent(
						eastl::make_shared<QuakeEventDataRotateActor>(mPlayerId, mAbsoluteTransform));

					pPlayerActor->UpdateTimers(deltaMs);
					pPlayerActor->UpdateWeapon(deltaMs);
					pPlayerActor->UpdateMovement(HProject(velocity));
				}
			}
		}
	}
}