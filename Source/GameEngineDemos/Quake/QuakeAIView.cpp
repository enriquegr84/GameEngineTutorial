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
	mPitchTarget = 0.0f;

	mOrientation = 1;
	mStationaryTime = 0;
	mCurrentNodeTime = 0;

	mMaxJumpSpeed = 3.4f;
	mMaxFallSpeed = 240.0f;
	mMaxRotateSpeed = 180.0f;
	mMoveSpeed = 6.0f;
	mJumpSpeed = 3.4f;
	mJumpMoveSpeed = 10.0f;
	mFallSpeed = 0.0f;
	mRotateSpeed = 0.0f;

	mPathingGraph = GameLogic::Get()->GetAIManager()->GetPathingGraph();
	mCurrentNode = NULL;
	mGoalCluster = NULL;
	mPlayerAction = 0;
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
	//printf("distance stationary %f \n", Length(collision - position));
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
	//printf("distance cliff %f \n", abs(collision[2] - position[2]));
	if (abs(collision[2] - position[2]) > 60.f)
	{
		//Choose randomly which way too look for getting out the cliff
		int sign = Randomizer::Rand() % 2 ? 1 : -1;

		// Smoothly turn 110� and check raycasting until we meet a minimum distance
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
	//printf("distance smooth %f \n", Length(collision - position));
	if (Length(collision - position) < 50.f)
	{
		//Choose randomly which way too look for obstacles
		int sign = mOrientation;

		// Smoothly turn 90� and check raycasting until we meet a minimum distance
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
	//printf("distance smooth %f \n", Length(collision - position));
	if (Length(collision - position) < 80.f)
	{
		//Choose randomly which way too look for obstacles
		int sign = Randomizer::Rand() % 2 ? 1 : -1;

		// Smoothly turn 90� and check raycasting until we meet a minimum distance
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
	mCurrentNodeTime -= deltaMs / 1000.f;

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
						if (mPathingGraph)
						{
							Vector3<float> currentPosition = pTransformComponent->GetPosition();
							/*
							printf("\n current position %f %f %f \n", 
								currentPosition[0], currentPosition[1], currentPosition[2]);
							*/
							if (mCurrentNode == NULL)
								mCurrentNode = mPathingGraph->FindClosestNode(currentPosition);

							bool findNode = true;
							Vector3<float> diff = mCurrentNode->GetPos() - currentPosition;
							if (Length(diff) > PATHING_DEFAULT_NODE_TOLERANCE)
								findNode = false;

							if (mCurrentNodeTime <= 0.f)
							{
								mCurrentNode = mPathingGraph->FindClosestNode(currentPosition);
								mGoalCluster = NULL;
								mPlayerAction = 0;
								findNode = true;
							}

							if (findNode)
							{
								PathingNode* currentNode = mCurrentNode;
								if (mGoalCluster == NULL || currentNode == mGoalCluster)
								{
									//printf("new goal %f node time", mCurrentNodeTime);
									do
									{
										// choose a random cluster
										PathingClusterVec clusterNodes;
										currentNode->GetClusters(GAT_JUMP, clusterNodes);

										PathingClusterVec clusterActors;
										if (mCurrentNodeTime <= 0.f)
										{
											for (PathingCluster* clusterNode : clusterNodes)
												if (clusterNode->GetActor() != INVALID_ACTOR_ID)
													clusterActors.push_back(clusterNode);
										}

										if (clusterActors.empty())
										{
											//printf("cluster size %u \n", clusterNodes.size());
											if (!clusterNodes.empty())
											{
												unsigned int cluster = Randomizer::Rand() % clusterNodes.size();
												mGoalCluster = clusterNodes[cluster]->GetTarget();
											}
											else
											{
												mGoalCluster = NULL;
												break;
											}
										}
										else
										{
											//printf("cluster actors size %u \n", clusterActors.size());
											unsigned int cluster = Randomizer::Rand() % clusterActors.size();
											mGoalCluster = clusterActors[cluster]->GetTarget();
										}
									} while (currentNode == mGoalCluster || mGoalCluster->GetArcs().empty());
								}

								if (mGoalCluster != NULL)
								{
									float minPosDiff = FLT_MAX;
									PathingCluster* currentCluster = currentNode->FindCluster(GAT_JUMP, mGoalCluster);
									if (currentCluster != NULL)
									{
										PathingArc* clusterArc = currentNode->FindArc(currentCluster->GetNode());
										PathingNode* clusterNode = clusterArc->GetNode();
										unsigned int clusterArcType = clusterArc->GetType();
										/*
										Vector3<float> nextPos = clusterNode->GetPos();
										printf("next pos %f %f %f arc %u \n", 
											nextPos[0], nextPos[1], nextPos[2], clusterArc->GetType());

										printf("node %f %f %f node %u goal cluster %u\n",
											currentNode->GetPos()[0], currentNode->GetPos()[1], currentNode->GetPos()[2],
											currentNode->GetId(), mGoalCluster->GetId());
										*/
										mPlayerAction = clusterArcType;
										mCurrentNode = clusterArc->GetNode();
										mCurrentNodeTime = clusterArc->GetWeight() + 1.0f;
										Vector3<float> direction = clusterNode->GetPos() - currentPosition;
										Normalize(direction);
										mYaw = atan2(direction[1], direction[0]) * (float)GE_C_RAD_TO_DEG;

										printf("\n next nodes : ");
										do
										{
											currentCluster = currentNode->FindCluster(GAT_JUMP, mGoalCluster);
											clusterArc = currentNode->FindArc(currentCluster->GetNode());
											currentNode = clusterArc->GetNode();
											printf("%u ", currentNode->GetId());
										} while (currentNode != mGoalCluster);
									}
									else
									{
										//printf("arc not found \n");
										mCurrentNode = NULL;
										mGoalCluster = NULL;
									}
								}
								else
								{
									Stationary(deltaMs);
									Smooth(deltaMs);
									Cliff();
								}
							}
							else
							{
								if (mPlayerAction == GAT_JUMP ||
									mPlayerAction == GAT_PUSH || 
									mPlayerAction == GAT_TELEPORT)
								{
									if (mPlayerAction == GAT_JUMP)
									{
										pPlayerActor->GetAction().actionType |= ACTION_JUMP;

										Vector3<float> direction = mCurrentNode->GetPos() - currentPosition;
										Normalize(direction);
										mYaw = atan2(direction[1], direction[0]) * (float)GE_C_RAD_TO_DEG;
									}
									else
									{
										Stationary(deltaMs);
										Movement(deltaMs);
									}
									mPlayerAction = 0;
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
									if (collision == NULL)
									{
										/*
										printf("node approaching %f %f %f \n",
										mCurrentNode->GetPos()[0], mCurrentNode->GetPos()[1], mCurrentNode->GetPos()[2]);
										*/
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
						rotation = yawRotation;
						Matrix4x4<float> pitchRotation = Rotation<4, float>(
							AxisAngle<4, float>(Vector4<float>::Unit(1), mPitch * (float)GE_C_DEG_TO_RAD));
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

				// Calculate the new rotation matrix from the camera
				// yaw and pitch (zrotate and xrotate).
				Matrix4x4<float> yawRotation = Rotation<4, float>(
					AxisAngle<4, float>(Vector4<float>::Unit(2), mYaw * (float)GE_C_DEG_TO_RAD));
				rotation = yawRotation;

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