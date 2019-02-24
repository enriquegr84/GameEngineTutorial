//
//  This program is free software; you can redistribute it and/or
//  modify it under the terms of the GNU General Public License
//  as published by the Free Software Foundation; either version 3
//  of the License, or (at your option) any later version.
//
//  This program is distributed in the hope that it will be useful,
//  but WITHOUT ANY WARRANTY; without even the implied warranty of
//  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//  GNU General Public License for more details.
//
//  You should have received a copy of the GNU General Public License
//  along with this program; if not, write to the Free Software
//  Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.

#include "QuakeAIManager.h"

#include "Core/OS/OS.h"
#include "Core/Logger/Logger.h"
#include "Core/Event/EventManager.h"

#include "Physic/PhysicEventListener.h"

#include "QuakeEvents.h"
#include "QuakeApp.h"
#include "Quake.h"

QuakeAIManager::QuakeAIManager()
	: AIManager()
{
	mYaw = 0.0f;
	mPitchTarget = 0.0f;

	mMaxJumpSpeed = 3.4f;
	mMaxFallSpeed = 240.0f;
	mMaxRotateSpeed = 180.0f;
	mMoveSpeed = 6.0f;
	mJumpSpeed = 3.4f;
	mJumpMoveSpeed = 10.0f;
	mFallSpeed = 0.0f;
	mRotateSpeed = 0.0f;

	mPathingGraph = eastl::make_shared<PathingGraph>();
}   // QuakeAIManager

//-----------------------------------------------------------------------------

QuakeAIManager::~QuakeAIManager()
{

}   // ~QuakeAIManager


	//waypoint generation via physics simulation
void QuakeAIManager::CreateWaypoints(ActorId playerId)
{
	mPlayerActor = 
		eastl::dynamic_shared_pointer_cast<PlayerActor>(
		GameLogic::Get()->GetActor(playerId).lock());

	QuakeLogic* game = static_cast<QuakeLogic *>(GameLogic::Get());
	game->GetGamePhysics()->SetTriggerCollision(true);
	game->RemoveAllDelegates();
	RegisterAllDelegates();

	//first we store the most important points of the map
	for (auto ammo : game->GetAmmoActors())
	{
		eastl::shared_ptr<TransformComponent> pTransformComponent(
			ammo->GetComponent<TransformComponent>(TransformComponent::Name).lock());
		if (pTransformComponent)
		{
			PathingNode* pNode = new PathingNode(pTransformComponent->GetPosition());
			mPathingGraph->InsertNode(pNode);

			mOpenSet[pNode] = true;
		}
	}
	for (auto weapon : game->GetWeaponActors())
	{
		eastl::shared_ptr<TransformComponent> pTransformComponent(
			weapon->GetComponent<TransformComponent>(TransformComponent::Name).lock());
		if (pTransformComponent)
		{
			PathingNode* pNode = new PathingNode(pTransformComponent->GetPosition());
			mPathingGraph->InsertNode(pNode);

			mOpenSet[pNode] = true;
		}
	}
	for (auto health : game->GetHealthActors())
	{
		eastl::shared_ptr<TransformComponent> pTransformComponent(
			health->GetComponent<TransformComponent>(TransformComponent::Name).lock());
		if (pTransformComponent)
		{
			PathingNode* pNode = new PathingNode(pTransformComponent->GetPosition());
			mPathingGraph->InsertNode(pNode);

			mOpenSet[pNode] = true;
		}
	}
	for (auto armor : game->GetArmorActors())
	{
		eastl::shared_ptr<TransformComponent> pTransformComponent(
			armor->GetComponent<TransformComponent>(TransformComponent::Name).lock());
		if (pTransformComponent)
		{
			PathingNode* pNode = new PathingNode(pTransformComponent->GetPosition());
			mPathingGraph->InsertNode(pNode);

			mOpenSet[pNode] = true;
		}
	}
	for (auto trigger : game->GetTriggerActors())
	{
		if (trigger->GetComponent<TeleporterTrigger>(TeleporterTrigger::Name).lock())
		{
			eastl::shared_ptr<TeleporterTrigger> pTeleporterTrigger(
				trigger->GetComponent<TeleporterTrigger>(TeleporterTrigger::Name).lock());
			if (pTeleporterTrigger)
			{
				PathingNode* pNode = new PathingNode(pTeleporterTrigger->GetTarget().GetTranslation());
				mPathingGraph->InsertNode(pNode);

				mOpenSet[pNode] = false;
			}
		}
		else if (trigger->GetComponent<PushTrigger>(PushTrigger::Name).lock())
		{
			eastl::shared_ptr<PushTrigger> pPushTrigger(
				trigger->GetComponent<PushTrigger>(PushTrigger::Name).lock());
			if (pPushTrigger)
			{
				PathingNode* pNode = new PathingNode(pPushTrigger->GetTarget().GetTranslation());
				mPathingGraph->InsertNode(pNode);

				mOpenSet[pNode] = false;
			}
		}
	}
	for (auto target : game->GetTargetActors())
	{
		eastl::shared_ptr<TransformComponent> pTransformComponent(
			target->GetComponent<TransformComponent>(TransformComponent::Name).lock());
		if (pTransformComponent)
		{
			PathingNode* pNode = new PathingNode(pTransformComponent->GetPosition());
			mPathingGraph->InsertNode(pNode);
			mOpenSet[pNode] = true;
		}
	}

	mAngleDirection.clear();
	mRotationDirection.clear();
	for (int angle = 0; angle < 360; angle += 20)
	{
		Matrix4x4<float> rotation = Rotation<4, float>(
			AxisAngle<4, float>(Vector4<float>::Unit(2), angle * (float)GE_C_DEG_TO_RAD));

		Vector4<float> atWorld = Vector4<float>::Unit(PITCH); // forward vector
#if defined(GE_USE_MAT_VEC)
		atWorld = rotation * atWorld;
#else
		atWorld = atWorld * rotation;
#endif

		mAngleDirection[angle] = HProject(atWorld);
		mRotationDirection[angle] = rotation;
	}

	// we create the waypoint according to the character controller physics system. Every
	// step, it will be generated recursively new waypoints and its conections as result of 
	// the simulation
	SimulateWaypoints();

	game->GetGamePhysics()->SetTriggerCollision(false);
	game->RegisterAllDelegates();

	RemoveAllDelegates();
}

void QuakeAIManager::SimulateWaypoints()
{
	eastl::shared_ptr<BaseGamePhysic> gamePhysics = GameLogic::Get()->GetGamePhysics();

	while (!mOpenSet.empty())
	{
		// grab the candidate
		eastl::map<PathingNode*, bool>::iterator itOpenSet = mOpenSet.begin();
		PathingNode* pNode = itOpenSet->first;

		//check if its on ground
		if (itOpenSet->second)
			SimulateMovement(pNode);

		// we have processed this node so remove it from the open set
		mClosedSet[pNode] = itOpenSet->second;
		mOpenSet.erase(itOpenSet);
	}

	while (!mClosedSet.empty())
	{
		// grab the candidate
		eastl::map<PathingNode*, bool>::iterator itOpenSet = mClosedSet.begin();
		PathingNode* pNode = itOpenSet->first;

		//check if its on ground
		if (itOpenSet->second)
			SimulateJump(pNode);
		else 
			SimulateFall(pNode);

		// we have processed this node so remove it from the closed set
		mClosedSet.erase(itOpenSet);
	}
}

void QuakeAIManager::SimulateMovement(PathingNode* pNode)
{
	//lets move the character towards different directions
	eastl::shared_ptr<BaseGamePhysic> gamePhysics = GameLogic::Get()->GetGamePhysics();

	Transform transform;
	for (int angle = 0; angle < 360; angle += 20)
	{
		Vector3<float> direction = mAngleDirection[angle];

		transform.SetTranslation(pNode->GetPos());
		transform.SetRotation(mRotationDirection[angle]);
		gamePhysics->SetTransform(mPlayerActor->GetId(), transform);
		gamePhysics->WalkDirection(mPlayerActor->GetId(), direction * mMoveSpeed);
		gamePhysics->OnUpdate(0.02f);

		//create movements on the ground
		eastl::vector<Vector3<float>> movements;

		bool onGround = gamePhysics->OnGround(mPlayerActor->GetId());
		transform = gamePhysics->GetTransform(mPlayerActor->GetId());
		Vector3<float> position = pNode->GetPos();
		PathingNode* pEndNode = NULL;
		while (onGround)
		{
			// stalling is a break condition
			Vector3<float> diff = position - transform.GetTranslation();
			if (Length(diff) <= 2.f)
				break;

			position = transform.GetTranslation();
			PathingNode* pClosestNode = mPathingGraph->FindClosestNode(position);
			if (pClosestNode != pNode)
			{
				// if we find another close node it is also a break condition
				diff = pClosestNode->GetPos() - position;
				if (Length(diff) <= 6.f)
				{ 
					pEndNode = pClosestNode;
					break;
				}
			}
			movements.push_back(position);

			gamePhysics->OnUpdate(0.02f);
			onGround = gamePhysics->OnGround(mPlayerActor->GetId());
			transform = gamePhysics->GetTransform(mPlayerActor->GetId());
		}

		if (!movements.empty())
		{
			float deltaTime = 0.f;
			PathingNode* pCurrentNode = pNode;
			for (auto movement : movements)
			{
				deltaTime += 0.02f;
				PathingNode* pClosestNode = mPathingGraph->FindClosestNode(movement);
				if (pClosestNode != NULL)
				{
					Vector3<float> diff = pClosestNode->GetPos() - movement;
					if (Length(diff) >= 16.f)
					{
						PathingNode* pNewNode = new PathingNode(movement);
						mPathingGraph->InsertNode(pNewNode);
						PathingArc* pArc = new PathingArc(AT_MOVE, deltaTime);
						pArc->LinkNodes(pCurrentNode, pNewNode);
						pCurrentNode->AddArc(pArc);

						mOpenSet[pNewNode] = true;
						pCurrentNode = pNewNode;

						deltaTime = 0.f;
					}
				}
			}
			deltaTime += 0.02f;

			if (pEndNode == NULL)
			{
				if (!onGround)
				{
					position = transform.GetTranslation();
					pEndNode = mPathingGraph->FindClosestNode(position);
					if (pEndNode != NULL)
					{
						Vector3<float> diff = pEndNode->GetPos() - position;
						if (Length(diff) >= 16.f)
						{
							PathingNode* pNewNode = new PathingNode(position);
							mPathingGraph->InsertNode(pNewNode);
							PathingArc* pArc = new PathingArc(AT_MOVE, deltaTime);
							pArc->LinkNodes(pCurrentNode, pNewNode);
							pCurrentNode->AddArc(pArc);

							mOpenSet[pNewNode] = onGround;
						}
						else if (Length(diff) <= 6.f)
						{
							PathingArc* pArc = new PathingArc(AT_MOVE, deltaTime);
							pArc->LinkNodes(pCurrentNode, pEndNode);
							pCurrentNode->AddArc(pArc);
						}
					}
				}
			}
			else
			{
				PathingArc* pArc = new PathingArc(AT_MOVE, deltaTime);
				pArc->LinkNodes(pCurrentNode, pEndNode);
				pCurrentNode->AddArc(pArc);
			}
		}
	}
}

void QuakeAIManager::SimulateJump(PathingNode* pNode)
{
	//lets move the character towards different directions
	eastl::shared_ptr<BaseGamePhysic> gamePhysics = GameLogic::Get()->GetGamePhysics();

	Vector3<float> direction = Vector3<float>::Unit(YAW); // jump up vector
	direction[YAW] *= mJumpSpeed;

	Transform transform;
	transform.SetTranslation(pNode->GetPos());
	gamePhysics->SetTransform(mPlayerActor->GetId(), transform);
	gamePhysics->WalkDirection(mPlayerActor->GetId(), direction);
	gamePhysics->Jump(mPlayerActor->GetId(), direction);
	gamePhysics->OnUpdate(0.01f);

	//nodes closed to jump position
	eastl::map<PathingNode*, float> nodes;

	// gravity falling simulation
	transform = gamePhysics->GetTransform(mPlayerActor->GetId());

	float fallSpeed = 0.f;
	float deltaTime = 0.f;
	PathingNode* pFallingNode = pNode;
	while (!gamePhysics->OnGround(mPlayerActor->GetId()))
	{
		float jumpSpeed = gamePhysics->GetJumpSpeed(mPlayerActor->GetId());

		deltaTime += 0.01f;
		fallSpeed += (10.f / (jumpSpeed * 0.5f));
		if (fallSpeed > mMaxFallSpeed) fallSpeed = mMaxFallSpeed;

		PathingNode* pClosestNode = 
			mPathingGraph->FindClosestNode(transform.GetTranslation());
		if (pClosestNode != NULL)
		{
			if (pClosestNode != pFallingNode)
				deltaTime = 0.01f;
			nodes[pClosestNode] = deltaTime;
			pFallingNode = pClosestNode;
		}

		Vector3<float> direction = Vector3<float>::Unit(YAW); // jump up vector
		direction[YAW] *= -jumpSpeed * fallSpeed;

		gamePhysics->FallDirection(mPlayerActor->GetId(), direction);
		gamePhysics->OnUpdate(0.01f);

		transform = gamePhysics->GetTransform(mPlayerActor->GetId());
	}

	//we store the jump if we find a landing node
	if (!nodes.empty())
	{
		Vector3<float> position = transform.GetTranslation();
		PathingNode* pEndNode = mPathingGraph->FindClosestNode(position);
		if (pEndNode != NULL)
		{
			Vector3<float> diff = pEndNode->GetPos() - position;
			if (Length(diff) <= 6.f)
			{
				PathingNode* pCurrentNode = pNode;
				for (auto node : nodes)
				{
					PathingNode* pJumpNode = node.first;
					PathingArc* pArc = new PathingArc(AT_JUMP, node.second);
					pArc->LinkNodes(pCurrentNode, pJumpNode);
					pCurrentNode->AddArc(pArc);

					pCurrentNode = pJumpNode;
				}

				if (pCurrentNode != pEndNode)
				{
					deltaTime = 0.01f;
					PathingArc* pArc = new PathingArc(AT_JUMP, deltaTime);
					pArc->LinkNodes(pCurrentNode, pEndNode);
					pCurrentNode->AddArc(pArc);
				}
			}
		}
	}

	for (int angle = 0; angle < 360; angle += 20)
	{
		direction = mAngleDirection[angle];
		direction[PITCH] *= mJumpMoveSpeed;
		direction[ROLL] *= mJumpMoveSpeed;
		direction[YAW] = mJumpSpeed;

		transform.SetTranslation(pNode->GetPos());
		transform.SetRotation(mRotationDirection[angle]);
		gamePhysics->SetTransform(mPlayerActor->GetId(), transform);
		gamePhysics->WalkDirection(mPlayerActor->GetId(), direction);
		gamePhysics->Jump(mPlayerActor->GetId(), direction);
		gamePhysics->OnUpdate(0.01f);

		transform = gamePhysics->GetTransform(mPlayerActor->GetId());

		// gravity falling simulation
		nodes.clear();
		deltaTime = 0.f;
		fallSpeed = 0.f;
		pFallingNode = pNode;
		while (!gamePhysics->OnGround(mPlayerActor->GetId()))
		{
			float jumpSpeed = gamePhysics->GetJumpSpeed(mPlayerActor->GetId());

			deltaTime += 0.01f;
			fallSpeed += (10.f / (jumpSpeed * 0.5f));
			if (fallSpeed > mMaxFallSpeed) fallSpeed = mMaxFallSpeed;

			PathingNode* pClosestNode =
				mPathingGraph->FindClosestNode(transform.GetTranslation());
			if (pClosestNode != NULL)
			{
				if (pClosestNode != pFallingNode)
					deltaTime = 0.01f;
				nodes[pClosestNode] = deltaTime;
				pFallingNode = pClosestNode;
			}

			direction = mAngleDirection[angle];
			direction[PITCH] *= jumpSpeed * (fallSpeed / 4.f);
			direction[ROLL] *= jumpSpeed * (fallSpeed / 4.f);
			direction[YAW] = -jumpSpeed * fallSpeed;

			gamePhysics->FallDirection(mPlayerActor->GetId(), direction);
			gamePhysics->OnUpdate(0.01f);

			transform = gamePhysics->GetTransform(mPlayerActor->GetId());
		}

		//we store the jump if we find a landing node
		if (!nodes.empty())
		{
			Vector3<float> position = transform.GetTranslation();
			PathingNode* pEndNode = mPathingGraph->FindClosestNode(position);
			if (pEndNode != NULL)
			{
				Vector3<float> diff = pEndNode->GetPos() - position;
				if (Length(diff) <= 6.f)
				{
					PathingNode* pCurrentNode = pNode;
					for (auto node : nodes)
					{
						PathingNode* pJumpNode = node.first;
						PathingArc* pArc = new PathingArc(AT_JUMP, node.second);
						pArc->LinkNodes(pCurrentNode, pJumpNode);
						pCurrentNode->AddArc(pArc);

						pCurrentNode = pJumpNode;
					}

					if (pCurrentNode != pEndNode)
					{
						deltaTime = 0.01f;
						PathingArc* pArc = new PathingArc(AT_JUMP, deltaTime);
						pArc->LinkNodes(pCurrentNode, pEndNode);
						pCurrentNode->AddArc(pArc);
					}
				}
			}
		}
	}
}

void QuakeAIManager::SimulateFall(PathingNode* pNode)
{
	//lets move the character towards different direction
	eastl::shared_ptr<BaseGamePhysic> gamePhysics = GameLogic::Get()->GetGamePhysics();

	float fallSpeed = 2.f, deltaTime = 0.f;
	Vector3<float> direction = Vector3<float>::Unit(YAW); // jump up vector
	direction[YAW] *= -fallSpeed;

	Transform transform;
	transform.SetTranslation(pNode->GetPos());
	gamePhysics->SetTransform(mPlayerActor->GetId(), transform);
	gamePhysics->SetVelocity(mPlayerActor->GetId(), Vector3<float>::Zero());
	gamePhysics->FallDirection(mPlayerActor->GetId(), direction);
	gamePhysics->OnUpdate(0.01f);

	//nodes closed to falling position
	eastl::map<PathingNode*, float> nodes;

	// gravity falling simulation
	transform = gamePhysics->GetTransform(mPlayerActor->GetId());

	PathingNode* pFallingNode = pNode;
	while (!gamePhysics->OnGround(mPlayerActor->GetId()))
	{
		float jumpSpeed = gamePhysics->GetJumpSpeed(mPlayerActor->GetId());

		deltaTime += 0.01f;
		fallSpeed += (10.f / (jumpSpeed * 0.5f));
		if (fallSpeed > mMaxFallSpeed) fallSpeed = mMaxFallSpeed;

		PathingNode* pClosestNode =
			mPathingGraph->FindClosestNode(transform.GetTranslation());
		if (pClosestNode != NULL)
		{
			if (pClosestNode != pFallingNode)
				deltaTime = 0.01f;
			nodes[pClosestNode] = deltaTime;
			pFallingNode = pClosestNode;
		}

		Vector3<float> direction = Vector3<float>::Unit(YAW); // jump up vector
		direction[YAW] *= -jumpSpeed * fallSpeed;

		gamePhysics->FallDirection(mPlayerActor->GetId(), direction);
		gamePhysics->OnUpdate(0.01f);

		transform = gamePhysics->GetTransform(mPlayerActor->GetId());
	}

	//we store the fall if we find a landing node
	if (!nodes.empty())
	{
		Vector3<float> position = transform.GetTranslation();
		PathingNode* pEndNode = mPathingGraph->FindClosestNode(position);
		if (pEndNode != NULL)
		{
			Vector3<float> diff = pEndNode->GetPos() - position;
			if (Length(diff) <= 6.f)
			{
				PathingNode* pCurrentNode = pNode;
				for (auto node : nodes)
				{
					PathingNode* pJumpNode = node.first;
					PathingArc* pArc = new PathingArc(AT_FALL, node.second);
					pArc->LinkNodes(pCurrentNode, pJumpNode);
					pCurrentNode->AddArc(pArc);

					pCurrentNode = pJumpNode;
				}

				if (pCurrentNode != pEndNode)
				{
					deltaTime = 0.01f;
					PathingArc* pArc = new PathingArc(AT_FALL, deltaTime);
					pArc->LinkNodes(pCurrentNode, pEndNode);
					pCurrentNode->AddArc(pArc);
				}
			}
		}
	}

	for (int angle = 0; angle < 360; angle += 20)
	{
		deltaTime = 0.f;
		fallSpeed = 2.f;
		if (fallSpeed > mMaxFallSpeed) fallSpeed = mMaxFallSpeed;

		direction = mAngleDirection[angle];
		direction[PITCH] *= 0.5f;
		direction[ROLL] *= 0.5f;
		direction[YAW] = -2.f;

		transform.SetTranslation(pNode->GetPos());
		transform.SetRotation(mRotationDirection[angle]);
		gamePhysics->SetTransform(mPlayerActor->GetId(), transform);
		gamePhysics->SetVelocity(mPlayerActor->GetId(), Vector3<float>::Zero());
		gamePhysics->FallDirection(mPlayerActor->GetId(), direction);
		gamePhysics->OnUpdate(0.01f);

		// gravity falling simulation
		transform = gamePhysics->GetTransform(mPlayerActor->GetId());

		nodes.clear();
		pFallingNode = pNode;
		while (!gamePhysics->OnGround(mPlayerActor->GetId()))
		{
			float jumpSpeed = gamePhysics->GetJumpSpeed(mPlayerActor->GetId());

			deltaTime += 0.01f;
			fallSpeed += (10.f / (jumpSpeed * 0.5f));
			if (fallSpeed > mMaxFallSpeed) fallSpeed = mMaxFallSpeed;

			PathingNode* pClosestNode =
				mPathingGraph->FindClosestNode(transform.GetTranslation());
			if (pClosestNode != NULL)
			{
				if (pClosestNode != pFallingNode)
					deltaTime = 0.01f;
				nodes[pClosestNode] = deltaTime;
				pFallingNode = pClosestNode;
			}

			direction = mAngleDirection[angle];
			direction[PITCH] *= jumpSpeed * (fallSpeed / 4.f);
			direction[ROLL] *= jumpSpeed * (fallSpeed / 4.f);
			direction[YAW] = -jumpSpeed * fallSpeed;

			gamePhysics->FallDirection(mPlayerActor->GetId(), direction);
			gamePhysics->OnUpdate(0.01f);

			transform = gamePhysics->GetTransform(mPlayerActor->GetId());
		}

		//we store the fall if we find a landing node
		if (!nodes.empty())
		{
			Vector3<float> position = transform.GetTranslation();
			PathingNode* pEndNode = mPathingGraph->FindClosestNode(position);
			if (pEndNode != NULL)
			{
				Vector3<float> diff = pEndNode->GetPos() - position;
				if (Length(diff) <= 6.f)
				{
					PathingNode* pCurrentNode = pNode;
					for (auto node : nodes)
					{
						PathingNode* pJumpNode = node.first;
						PathingArc* pArc = new PathingArc(AT_FALL, node.second);
						pArc->LinkNodes(pCurrentNode, pJumpNode);
						pCurrentNode->AddArc(pArc);

						pCurrentNode = pJumpNode;
					}

					if (pCurrentNode != pEndNode)
					{
						deltaTime = 0.01f;
						PathingArc* pArc = new PathingArc(AT_FALL, deltaTime);
						pArc->LinkNodes(pCurrentNode, pEndNode);
						pCurrentNode->AddArc(pArc);
					}
				}
			}
		}
	}
}

void QuakeAIManager::PhysicsTriggerEnterDelegate(BaseEventDataPtr pEventData)
{
	eastl::shared_ptr<EventDataPhysTriggerEnter> pCastEventData =
		eastl::static_pointer_cast<EventDataPhysTriggerEnter>(pEventData);

	eastl::shared_ptr<Actor> pItemActor(
		GameLogic::Get()->GetActor(pCastEventData->GetTriggerId()).lock());

	if (mPlayerActor->GetId() == pCastEventData->GetOtherActor())
	{
		// dead players
		if (mPlayerActor->GetState().stats[STAT_HEALTH] <= 0)
			return;

		if (pItemActor->GetType() == "Weapon")
		{
			eastl::shared_ptr<WeaponPickup> pWeaponPickup =
				pItemActor->GetComponent<WeaponPickup>(WeaponPickup::Name).lock();

		}
		else if (pItemActor->GetType() == "Ammo")
		{
			eastl::shared_ptr<AmmoPickup> pAmmoPickup =
				pItemActor->GetComponent<AmmoPickup>(AmmoPickup::Name).lock();

		}
		else if (pItemActor->GetType() == "Armor")
		{
			eastl::shared_ptr<ArmorPickup> pArmorPickup =
				pItemActor->GetComponent<ArmorPickup>(ArmorPickup::Name).lock();

		}
		else if (pItemActor->GetType() == "Health")
		{
			eastl::shared_ptr<HealthPickup> pHealthPickup =
				pItemActor->GetComponent<HealthPickup>(HealthPickup::Name).lock();

		}

		if (pItemActor->GetType() == "Weapon")
		{
			eastl::shared_ptr<WeaponPickup> pWeaponPickup =
				pItemActor->GetComponent<WeaponPickup>(WeaponPickup::Name).lock();

		}
	}
}


void QuakeAIManager::PhysicsTriggerLeaveDelegate(BaseEventDataPtr pEventData)
{
	eastl::shared_ptr<EventDataPhysTriggerLeave> pCastEventData =
		eastl::static_pointer_cast<EventDataPhysTriggerLeave>(pEventData);
}


void QuakeAIManager::PhysicsCollisionDelegate(BaseEventDataPtr pEventData)
{
	eastl::shared_ptr<EventDataPhysCollision> pCastEventData =
		eastl::static_pointer_cast<EventDataPhysCollision>(pEventData);

	eastl::shared_ptr<Actor> pGameActorA(
		GameLogic::Get()->GetActor(pCastEventData->GetActorA()).lock());
	eastl::shared_ptr<Actor> pGameActorB(
		GameLogic::Get()->GetActor(pCastEventData->GetActorB()).lock());
	if (pGameActorA && pGameActorB)
	{
		eastl::shared_ptr<Actor> pItemActor;
		eastl::shared_ptr<PlayerActor> pPlayerActor;
		if (eastl::dynamic_shared_pointer_cast<PlayerActor>(pGameActorA) &&
			!eastl::dynamic_shared_pointer_cast<PlayerActor>(pGameActorB))
		{
			pPlayerActor = eastl::dynamic_shared_pointer_cast<PlayerActor>(pGameActorA);
			pItemActor = pGameActorB;

		}
		else if (!eastl::dynamic_shared_pointer_cast<PlayerActor>(pGameActorA) &&
			eastl::dynamic_shared_pointer_cast<PlayerActor>(pGameActorB))
		{
			pPlayerActor = eastl::dynamic_shared_pointer_cast<PlayerActor>(pGameActorB);
			pItemActor = pGameActorA;
		}
		else return;

		if (mPlayerActor->GetId() == pPlayerActor->GetId())
		{
			// dead players
			if (pPlayerActor->GetState().stats[STAT_HEALTH] <= 0)
				return;

			if (pItemActor->GetType() == "Trigger")
			{
				if (pItemActor->GetComponent<PushTrigger>(PushTrigger::Name).lock())
				{
					pPlayerActor->GetAction().triggerPush = pItemActor->GetId();
				}
				else if (pItemActor->GetComponent<TeleporterTrigger>(TeleporterTrigger::Name).lock())
				{
					pPlayerActor->GetAction().triggerTeleporter = pItemActor->GetId();
				}
			}
			else if (pItemActor->GetType() == "Fire")
			{
				if (pItemActor->GetComponent<RocketFire>(RocketFire::Name).lock())
				{
					eastl::shared_ptr<RocketFire> pRocketFire =
						pItemActor->GetComponent<RocketFire>(RocketFire::Name).lock();
				}
				else if (pItemActor->GetComponent<PlasmaFire>(PlasmaFire::Name).lock())
				{
					eastl::shared_ptr<PlasmaFire> pPlasmaFire =
						pItemActor->GetComponent<PlasmaFire>(PlasmaFire::Name).lock();
				}
			}
		}
	}
	else if (pGameActorA || pGameActorB)
	{
		eastl::shared_ptr<Actor> pItemActor;
		if (pGameActorA)
			pItemActor = eastl::dynamic_shared_pointer_cast<Actor>(pGameActorA);
		else
			pItemActor = eastl::dynamic_shared_pointer_cast<Actor>(pGameActorB);
		if (pItemActor->GetType() == "Fire")
		{
			if (pItemActor->GetComponent<RocketFire>(RocketFire::Name).lock())
			{
				eastl::shared_ptr<RocketFire> pRocketFire =
					pItemActor->GetComponent<RocketFire>(RocketFire::Name).lock();
			}
			else if (pItemActor->GetComponent<PlasmaFire>(PlasmaFire::Name).lock())
			{
				eastl::shared_ptr<PlasmaFire> pPlasmaFire =
					pItemActor->GetComponent<PlasmaFire>(PlasmaFire::Name).lock();
			}
		}
	}
}

void QuakeAIManager::PhysicsSeparationDelegate(BaseEventDataPtr pEventData)
{
	eastl::shared_ptr<EventDataPhysSeparation> pCastEventData =
		eastl::static_pointer_cast<EventDataPhysSeparation>(pEventData);

	eastl::shared_ptr<Actor> pGameActorA(
		GameLogic::Get()->GetActor(pCastEventData->GetActorA()).lock());
	eastl::shared_ptr<Actor> pGameActorB(
		GameLogic::Get()->GetActor(pCastEventData->GetActorB()).lock());
	if (pGameActorA && pGameActorB)
	{
		eastl::shared_ptr<Actor> pItemActor;
		eastl::shared_ptr<PlayerActor> pPlayerActor;
		if (eastl::dynamic_shared_pointer_cast<PlayerActor>(pGameActorA) &&
			!eastl::dynamic_shared_pointer_cast<PlayerActor>(pGameActorB))
		{
			pPlayerActor = eastl::dynamic_shared_pointer_cast<PlayerActor>(pGameActorA);
			pItemActor = pGameActorB;

		}
		else if (!eastl::dynamic_shared_pointer_cast<PlayerActor>(pGameActorA) &&
			eastl::dynamic_shared_pointer_cast<PlayerActor>(pGameActorB))
		{
			pPlayerActor = eastl::dynamic_shared_pointer_cast<PlayerActor>(pGameActorB);
			pItemActor = pGameActorA;
		}
		else return;
	}
}

void QuakeAIManager::RegisterAllDelegates(void)
{
	BaseEventManager* pGlobalEventManager = BaseEventManager::Get();
	pGlobalEventManager->AddListener(
		MakeDelegate(this, &QuakeAIManager::PhysicsTriggerEnterDelegate),
		EventDataPhysTriggerEnter::skEventType);
	pGlobalEventManager->AddListener(
		MakeDelegate(this, &QuakeAIManager::PhysicsTriggerLeaveDelegate),
		EventDataPhysTriggerLeave::skEventType);
	pGlobalEventManager->AddListener(
		MakeDelegate(this, &QuakeAIManager::PhysicsCollisionDelegate),
		EventDataPhysCollision::skEventType);
	pGlobalEventManager->AddListener(
		MakeDelegate(this, &QuakeAIManager::PhysicsSeparationDelegate),
		EventDataPhysSeparation::skEventType);
}

void QuakeAIManager::RemoveAllDelegates(void)
{
	BaseEventManager* pGlobalEventManager = BaseEventManager::Get();
	pGlobalEventManager->RemoveListener(
		MakeDelegate(this, &QuakeAIManager::PhysicsTriggerEnterDelegate),
		EventDataPhysTriggerEnter::skEventType);
	pGlobalEventManager->RemoveListener(
		MakeDelegate(this, &QuakeAIManager::PhysicsTriggerLeaveDelegate),
		EventDataPhysTriggerLeave::skEventType);
	pGlobalEventManager->RemoveListener(
		MakeDelegate(this, &QuakeAIManager::PhysicsCollisionDelegate),
		EventDataPhysCollision::skEventType);
	pGlobalEventManager->RemoveListener(
		MakeDelegate(this, &QuakeAIManager::PhysicsSeparationDelegate),
		EventDataPhysSeparation::skEventType);
}