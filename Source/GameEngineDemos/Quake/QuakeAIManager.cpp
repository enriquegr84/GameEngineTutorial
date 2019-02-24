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
	mLastNodeId = 0;

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
			PathingNode* pNode = new PathingNode(
				GetNewNodeID(), ammo->GetId(), pTransformComponent->GetPosition());
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
			PathingNode* pNode = new PathingNode(
				GetNewNodeID(), weapon->GetId(), pTransformComponent->GetPosition());
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
			PathingNode* pNode = new PathingNode(
				GetNewNodeID(), health->GetId(), pTransformComponent->GetPosition());
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
			PathingNode* pNode = new PathingNode(
				GetNewNodeID(), armor->GetId(), pTransformComponent->GetPosition());
			mPathingGraph->InsertNode(pNode);
			mOpenSet[pNode] = true;
		}
	}
	for (auto target : game->GetTargetActors())
	{
		eastl::shared_ptr<TransformComponent> pTransformComponent(
			target->GetComponent<TransformComponent>(TransformComponent::Name).lock());
		if (pTransformComponent)
		{
			PathingNode* pNode = new PathingNode(
				GetNewNodeID(), target->GetId(), pTransformComponent->GetPosition());
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

	mActorNodes.clear();
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

	//finally we process the item actors which we have met
	eastl::map<Vector3<float>, ActorId>::iterator itActorNode;
	for (itActorNode = mActorNodes.begin(); itActorNode != mActorNodes.end(); itActorNode++)
	{
		Vector3<float> position = itActorNode->first;
		eastl::shared_ptr<Actor> pItemActor(
			GameLogic::Get()->GetActor(itActorNode->second).lock());
		PathingNode* pClosestNode = mPathingGraph->FindClosestNode(position);
		if (pClosestNode != NULL && Length(pClosestNode->GetPos() - position) <= 6.f)
		{
			pClosestNode->SetActorId(pItemActor->GetId());

			if (pItemActor->GetType() == "Trigger")
			{
				if (pItemActor->GetComponent<PushTrigger>(PushTrigger::Name).lock())
				{
					eastl::shared_ptr<PushTrigger> pPushTrigger =
						pItemActor->GetComponent<PushTrigger>(PushTrigger::Name).lock();

					Vector3<float> targetPosition = pPushTrigger->GetTarget().GetTranslation();
					SimulateTriggerPush(pClosestNode, targetPosition);
				}
				else if (pItemActor->GetComponent<TeleporterTrigger>(TeleporterTrigger::Name).lock())
				{
					eastl::shared_ptr<TeleporterTrigger> pTeleporterTrigger =
						pItemActor->GetComponent<TeleporterTrigger>(TeleporterTrigger::Name).lock();

					Vector3<float> targetPosition = pTeleporterTrigger->GetTarget().GetTranslation();
					SimulateTriggerTeleport(pClosestNode, targetPosition);
				}
			}
		}
	}
	mActorNodes.clear();
}

void QuakeAIManager::SimulateTriggerTeleport(PathingNode* pNode, const Vector3<float>& target)
{
	//lets move the character towards different directions
	eastl::shared_ptr<BaseGamePhysic> gamePhysics = GameLogic::Get()->GetGamePhysics();

	Vector3<float> direction = Vector3<float>::Unit(YAW); //up vector

	Transform transform;
	transform.SetTranslation(target);
	gamePhysics->SetTransform(mPlayerActor->GetId(), transform);
	gamePhysics->OnUpdate(0.01f);

	// nodes closed to teleport position
	eastl::vector<PathingNode*> nodes;
	eastl::map<PathingNode*, float> nodeTimes;
	eastl::map<PathingNode*, Vector3<float>> nodePositions;

	// gravity falling simulation
	transform = gamePhysics->GetTransform(mPlayerActor->GetId());

	PathingNode* pFallingNode = pNode;
	float totalTime = 0.f, deltaTime = 0.f, fallSpeed = 0.f;
	while (!gamePhysics->OnGround(mPlayerActor->GetId()))
	{
		float jumpSpeed = gamePhysics->GetJumpSpeed(mPlayerActor->GetId());

		totalTime += 0.01f;
		deltaTime += 0.01f;
		fallSpeed += (10.f / (jumpSpeed * 0.5f));
		if (fallSpeed > mMaxFallSpeed) fallSpeed = mMaxFallSpeed;

		PathingNode* pClosestNode =
			mPathingGraph->FindClosestNode(transform.GetTranslation());
		if (pClosestNode != NULL)
		{
			if (pClosestNode != pFallingNode)
				deltaTime = 0.01f;

			if (eastl::find(nodes.begin(), nodes.end(), pClosestNode) == nodes.end())
				nodes.push_back(pClosestNode);
			nodeTimes[pClosestNode] = deltaTime;
			nodePositions[pClosestNode] = transform.GetTranslation();
			pFallingNode = pClosestNode;
		}

		Normalize(direction);
		direction[PITCH] *= jumpSpeed * (fallSpeed / 4.f);
		direction[ROLL] *= jumpSpeed * (fallSpeed / 4.f);
		direction[YAW] = -jumpSpeed * fallSpeed;

		gamePhysics->FallDirection(mPlayerActor->GetId(), direction);
		gamePhysics->OnUpdate(0.01f);

		transform = gamePhysics->GetTransform(mPlayerActor->GetId());
	}
	totalTime += 0.01f;

	Vector3<float> position = transform.GetTranslation();
	PathingNode* pEndNode = mPathingGraph->FindClosestNode(position);
	if (pEndNode != NULL && pNode->FindArc(AIAT_TELEPORTTARGET, pEndNode) == NULL)
	{
		Vector3<float> diff = pEndNode->GetPos() - position;
		if (Length(diff) <= 6.f)
		{
			PathingNode* pCurrentNode = pNode;
			eastl::vector<PathingNode*>::iterator itNode;
			for (itNode = nodes.begin(); itNode != nodes.end(); itNode++)
			{
				PathingNode* pTeleportNode = (*itNode);
				if (pCurrentNode == pNode)
				{
					PathingArc* pArc = new PathingArc(AIAT_TELEPORTTARGET, totalTime);
					pArc->LinkNodes(pEndNode, pTeleportNode);
					pCurrentNode->AddArc(pArc);
				}
				PathingArc* pArc = new PathingArc(
					AIAT_TELEPORT, nodeTimes[pFallingNode], nodePositions[pFallingNode]);
				pArc->LinkNodes(pEndNode, pTeleportNode);
				pCurrentNode->AddArc(pArc);

				pCurrentNode = pTeleportNode;
			}

			if (pCurrentNode != pEndNode)
			{
				deltaTime = 0.01f;
				PathingArc* pArc = new PathingArc(AIAT_TELEPORT, deltaTime);
				pArc->LinkNodes(pEndNode, pEndNode);
				pCurrentNode->AddArc(pArc);
			}
		}
	}
}

void QuakeAIManager::SimulateTriggerPush(PathingNode* pNode, const Vector3<float>& target)
{
	//lets move the character towards different directions
	eastl::shared_ptr<BaseGamePhysic> gamePhysics = GameLogic::Get()->GetGamePhysics();

	Vector3<float> direction = target - pNode->GetPos();
	float push = Length(target - pNode->GetPos());
	Normalize(direction);

	direction[PITCH] *= push / 90.f;
	direction[ROLL] *= push / 90.f;
	direction[YAW] = push / 30.f;

	Transform transform;
	transform.SetTranslation(pNode->GetPos());
	gamePhysics->SetTransform(mPlayerActor->GetId(), transform);
	gamePhysics->WalkDirection(mPlayerActor->GetId(), direction);
	gamePhysics->Jump(mPlayerActor->GetId(), direction);
	gamePhysics->OnUpdate(0.01f);

	// nodes closed to jump position
	eastl::vector<PathingNode*> nodes;
	eastl::map<PathingNode*, float> nodeTimes;
	eastl::map<PathingNode*, Vector3<float>> nodePositions;

	// gravity falling simulation
	transform = gamePhysics->GetTransform(mPlayerActor->GetId());

	PathingNode* pFallingNode = pNode;
	float totalTime = 0.f, deltaTime = 0.f, fallSpeed = 0.f;
	while (!gamePhysics->OnGround(mPlayerActor->GetId()))
	{
		float jumpSpeed = gamePhysics->GetJumpSpeed(mPlayerActor->GetId());

		totalTime += 0.01f;
		deltaTime += 0.01f;
		fallSpeed += (10.f / (jumpSpeed * 0.5f));
		if (fallSpeed > mMaxFallSpeed) fallSpeed = mMaxFallSpeed;

		PathingNode* pClosestNode =
			mPathingGraph->FindClosestNode(transform.GetTranslation());
		if (pClosestNode != NULL)
		{
			if (pClosestNode != pFallingNode)
				deltaTime = 0.01f;

			if (eastl::find(nodes.begin(), nodes.end(), pClosestNode) == nodes.end())
				nodes.push_back(pClosestNode);
			nodeTimes[pClosestNode] = deltaTime;
			nodePositions[pClosestNode] = transform.GetTranslation();
			pFallingNode = pClosestNode;
		}

		Normalize(direction);
		direction[PITCH] *= jumpSpeed * (fallSpeed / 4.f);
		direction[ROLL] *= jumpSpeed * (fallSpeed / 4.f);
		direction[YAW] = -jumpSpeed * fallSpeed;

		gamePhysics->FallDirection(mPlayerActor->GetId(), direction);
		gamePhysics->OnUpdate(0.01f);

		transform = gamePhysics->GetTransform(mPlayerActor->GetId());
	}
	totalTime += 0.01f;

	//we store the jump if we find a landing node
	if (!nodes.empty())
	{
		Vector3<float> position = transform.GetTranslation();
		PathingNode* pEndNode = mPathingGraph->FindClosestNode(position);
		if (pEndNode != NULL && pNode->FindArc(AIAT_PUSHTARGET, pEndNode) == NULL)
		{
			Vector3<float> diff = pEndNode->GetPos() - position;
			if (Length(diff) <= 6.f)
			{
				PathingNode* pCurrentNode = pNode;
				eastl::vector<PathingNode*>::iterator itNode;
				for (itNode = nodes.begin(); itNode != nodes.end(); itNode++)
				{
					PathingNode* pJumpNode = (*itNode);
					if (pCurrentNode == pNode)
					{
						PathingArc* pArc = new PathingArc(AIAT_PUSHTARGET, totalTime);
						pArc->LinkNodes(pEndNode, pJumpNode);
						pCurrentNode->AddArc(pArc);
					}
					PathingArc* pArc = new PathingArc(
						AIAT_PUSH, nodeTimes[pFallingNode], nodePositions[pFallingNode]);
					pArc->LinkNodes(pEndNode, pJumpNode);
					pCurrentNode->AddArc(pArc);

					pCurrentNode = pJumpNode;
				}

				if (pCurrentNode != pEndNode)
				{
					deltaTime = 0.01f;
					PathingArc* pArc = new PathingArc(AIAT_PUSH, deltaTime);
					pArc->LinkNodes(pEndNode, pEndNode);
					pCurrentNode->AddArc(pArc);
				}
			}
		}
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
			if (Length(diff) <= 3.f)
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
			eastl::vector<Vector3<float>>::iterator itMove;
			for (itMove = movements.begin(); itMove != movements.end(); itMove++)
			{
				deltaTime += 0.02f;
				PathingNode* pClosestNode = mPathingGraph->FindClosestNode((*itMove));
				if (pClosestNode != NULL)
				{
					Vector3<float> diff = pClosestNode->GetPos() - (*itMove);
					if (Length(diff) >= 16.f)
					{
						PathingNode* pNewNode = new PathingNode(
							GetNewNodeID(), INVALID_ACTOR_ID, (*itMove));
						mPathingGraph->InsertNode(pNewNode);
						PathingArc* pArc = new PathingArc(AIAT_MOVE, deltaTime);
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
					if (pCurrentNode != pEndNode)
					{
						Vector3<float> diff = pEndNode->GetPos() - position;
						if (Length(diff) >= 16.f)
						{
							PathingNode* pNewNode = new PathingNode(
								GetNewNodeID(), INVALID_ACTOR_ID, position);
							mPathingGraph->InsertNode(pNewNode);
							PathingArc* pArc = new PathingArc(AIAT_MOVE, deltaTime);
							pArc->LinkNodes(pCurrentNode, pNewNode);
							pCurrentNode->AddArc(pArc);

							mOpenSet[pNewNode] = onGround;
						}
						else if (Length(diff) <= 6.f)
						{
							PathingArc* pArc = new PathingArc(AIAT_MOVE, deltaTime);
							pArc->LinkNodes(pCurrentNode, pEndNode);
							pCurrentNode->AddArc(pArc);
						}
					}
				}
			}
			else if (pCurrentNode != pEndNode)
			{
				PathingArc* pArc = new PathingArc(AIAT_MOVE, deltaTime);
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

	// nodes closed to jump position
	eastl::vector<PathingNode*> nodes;
	eastl::map<PathingNode*, float> nodeTimes;
	eastl::map<PathingNode*, Vector3<float>> nodePositions;

	// gravity falling simulation
	transform = gamePhysics->GetTransform(mPlayerActor->GetId());

	float fallSpeed = 0.f, deltaTime = 0.f, totalTime = 0.f;
	PathingNode* pFallingNode = pNode;
	while (!gamePhysics->OnGround(mPlayerActor->GetId()))
	{
		float jumpSpeed = gamePhysics->GetJumpSpeed(mPlayerActor->GetId());

		totalTime += 0.01f;
		deltaTime += 0.01f;
		fallSpeed += (10.f / (jumpSpeed * 0.5f));
		if (fallSpeed > mMaxFallSpeed) fallSpeed = mMaxFallSpeed;

		PathingNode* pClosestNode = 
			mPathingGraph->FindClosestNode(transform.GetTranslation());
		if (pClosestNode != NULL)
		{
			if (pClosestNode != pFallingNode)
				deltaTime = 0.01f;

			if (eastl::find(nodes.begin(), nodes.end(), pClosestNode) == nodes.end())
				nodes.push_back(pClosestNode);
			nodeTimes[pClosestNode] = deltaTime;
			nodePositions[pClosestNode] = transform.GetTranslation();
			pFallingNode = pClosestNode;
		}

		Vector3<float> direction = Vector3<float>::Unit(YAW); // jump up vector
		direction[YAW] *= -jumpSpeed * fallSpeed;

		gamePhysics->FallDirection(mPlayerActor->GetId(), direction);
		gamePhysics->OnUpdate(0.01f);

		transform = gamePhysics->GetTransform(mPlayerActor->GetId());
	}
	totalTime += 0.01f;

	//we store the jump if we find a landing node
	if (!nodes.empty())
	{
		Vector3<float> position = transform.GetTranslation();
		PathingNode* pEndNode = mPathingGraph->FindClosestNode(position);
		if (pEndNode != NULL && pNode->FindArc(AIAT_JUMPTARGET, pEndNode) == NULL)
		{
			Vector3<float> diff = pEndNode->GetPos() - position;
			if (Length(diff) <= 6.f)
			{
				PathingNode* pCurrentNode = pNode;
				eastl::vector<PathingNode*>::iterator itNode;
				for (itNode = nodes.begin(); itNode != nodes.end(); itNode++)
				{
					PathingNode* pJumpNode = (*itNode);
					if (pCurrentNode == pNode)
					{
						PathingArc* pArc = new PathingArc(AIAT_JUMPTARGET, totalTime);
						pArc->LinkNodes(pEndNode, pJumpNode);
						pCurrentNode->AddArc(pArc);
					}
					PathingArc* pArc = new PathingArc(
						AIAT_JUMP, nodeTimes[pFallingNode], nodePositions[pFallingNode]);
					pArc->LinkNodes(pEndNode, pJumpNode);
					pCurrentNode->AddArc(pArc);

					pCurrentNode = pJumpNode;
				}

				if (pCurrentNode != pEndNode)
				{
					deltaTime = 0.01f;
					PathingArc* pArc = new PathingArc(AIAT_JUMP, deltaTime);
					pArc->LinkNodes(pEndNode, pEndNode);
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
		nodePositions.clear();
		nodes.clear();
		totalTime = 0.f;
		deltaTime = 0.f;
		fallSpeed = 0.f;
		pFallingNode = pNode;
		while (!gamePhysics->OnGround(mPlayerActor->GetId()))
		{
			float jumpSpeed = gamePhysics->GetJumpSpeed(mPlayerActor->GetId());

			totalTime += 0.01f;
			deltaTime += 0.01f;
			fallSpeed += (10.f / (jumpSpeed * 0.5f));
			if (fallSpeed > mMaxFallSpeed) fallSpeed = mMaxFallSpeed;

			PathingNode* pClosestNode =
				mPathingGraph->FindClosestNode(transform.GetTranslation());
			if (pClosestNode != NULL)
			{
				if (pClosestNode != pFallingNode)
					deltaTime = 0.01f;

				if (eastl::find(nodes.begin(), nodes.end(), pClosestNode) == nodes.end())
					nodes.push_back(pClosestNode);
				nodeTimes[pClosestNode] = deltaTime;
				nodePositions[pClosestNode] = transform.GetTranslation();
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
		totalTime += 0.01f;

		//we store the jump if we find a landing node
		if (!nodes.empty())
		{
			Vector3<float> position = transform.GetTranslation();
			PathingNode* pEndNode = mPathingGraph->FindClosestNode(position);
			if (pEndNode != NULL && pNode->FindArc(AIAT_JUMPTARGET, pEndNode) == NULL)
			{
				Vector3<float> diff = pEndNode->GetPos() - position;
				if (Length(diff) <= 6.f)
				{
					PathingNode* pCurrentNode = pNode;
					eastl::vector<PathingNode*>::iterator itNode;
					for (itNode = nodes.begin(); itNode != nodes.end(); itNode++)
					{
						PathingNode* pJumpNode = (*itNode);
						if (pCurrentNode == pNode)
						{
							PathingArc* pArc = new PathingArc(AIAT_JUMPTARGET, totalTime);
							pArc->LinkNodes(pEndNode, pJumpNode);
							pCurrentNode->AddArc(pArc);
						}
						PathingArc* pArc = new PathingArc(
							AIAT_JUMP, nodeTimes[pFallingNode], nodePositions[pFallingNode]);
						pArc->LinkNodes(pEndNode, pJumpNode);
						pCurrentNode->AddArc(pArc);

						pCurrentNode = pJumpNode;
					}

					if (pCurrentNode != pEndNode)
					{
						deltaTime = 0.01f;
						PathingArc* pArc = new PathingArc(AIAT_JUMP, deltaTime);
						pArc->LinkNodes(pEndNode, pEndNode);
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

	float fallSpeed = 2.f, deltaTime = 0.f, totalTime = 0.f;
	Vector3<float> direction = Vector3<float>::Unit(YAW); // jump up vector
	direction[YAW] *= -fallSpeed;

	Transform transform;
	transform.SetTranslation(pNode->GetPos());
	gamePhysics->SetTransform(mPlayerActor->GetId(), transform);
	gamePhysics->SetVelocity(mPlayerActor->GetId(), Vector3<float>::Zero());
	gamePhysics->FallDirection(mPlayerActor->GetId(), direction);
	gamePhysics->OnUpdate(0.01f);

	// nodes closed to falling position
	eastl::vector<PathingNode*> nodes;
	eastl::map<PathingNode*, float> nodeTimes;
	eastl::map<PathingNode*, Vector3<float>> nodePositions;

	// gravity falling simulation
	transform = gamePhysics->GetTransform(mPlayerActor->GetId());

	PathingNode* pFallingNode = pNode;
	while (!gamePhysics->OnGround(mPlayerActor->GetId()))
	{
		float jumpSpeed = gamePhysics->GetJumpSpeed(mPlayerActor->GetId());

		totalTime += 0.01f;
		deltaTime += 0.01f;
		fallSpeed += (10.f / (jumpSpeed * 0.5f));
		if (fallSpeed > mMaxFallSpeed) fallSpeed = mMaxFallSpeed;

		PathingNode* pClosestNode =
			mPathingGraph->FindClosestNode(transform.GetTranslation());
		if (pClosestNode != NULL)
		{
			if (pClosestNode != pFallingNode)
				deltaTime = 0.01f;

			if (eastl::find(nodes.begin(), nodes.end(), pClosestNode) == nodes.end())
				nodes.push_back(pClosestNode);
			nodeTimes[pClosestNode] = deltaTime;
			nodePositions[pClosestNode] = transform.GetTranslation();
			pFallingNode = pClosestNode;
		}

		Vector3<float> direction = Vector3<float>::Unit(YAW); // jump up vector
		direction[YAW] *= -jumpSpeed * fallSpeed;

		gamePhysics->FallDirection(mPlayerActor->GetId(), direction);
		gamePhysics->OnUpdate(0.01f);

		transform = gamePhysics->GetTransform(mPlayerActor->GetId());
	}
	totalTime += 0.01f;

	//we store the fall if we find a landing node
	if (!nodes.empty())
	{
		Vector3<float> position = transform.GetTranslation();
		PathingNode* pEndNode = mPathingGraph->FindClosestNode(position);
		if (pEndNode != NULL && pNode->FindArc(AIAT_FALLTARGET, pEndNode) == NULL)
		{
			Vector3<float> diff = pEndNode->GetPos() - position;
			if (Length(diff) <= 6.f)
			{
				PathingNode* pCurrentNode = pNode;
				eastl::vector<PathingNode*>::iterator itNode;
				for (itNode = nodes.begin(); itNode != nodes.end(); itNode++)
				{
					PathingNode* pFallingNode = (*itNode);
					if (pCurrentNode == pNode)
					{
						PathingArc* pArc = new PathingArc(AIAT_FALLTARGET, totalTime);
						pArc->LinkNodes(pEndNode, pFallingNode);
						pCurrentNode->AddArc(pArc);
					}
					PathingArc* pArc = new PathingArc(
						AIAT_FALL, nodeTimes[pFallingNode], nodePositions[pFallingNode]);
					pArc->LinkNodes(pEndNode, pFallingNode);
					pCurrentNode->AddArc(pArc);

					pCurrentNode = pFallingNode;
				}

				if (pCurrentNode != pEndNode)
				{
					deltaTime = 0.01f;
					PathingArc* pArc = new PathingArc(AIAT_FALL, deltaTime);
					pArc->LinkNodes(pEndNode, pEndNode);
					pCurrentNode->AddArc(pArc);
				}
			}
		}
	}

	for (int angle = 0; angle < 360; angle += 20)
	{
		totalTime = 0.f;
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

		nodePositions.clear();
		nodes.clear();
		pFallingNode = pNode;
		while (!gamePhysics->OnGround(mPlayerActor->GetId()))
		{
			float jumpSpeed = gamePhysics->GetJumpSpeed(mPlayerActor->GetId());

			totalTime += 0.01f;
			deltaTime += 0.01f;
			fallSpeed += (10.f / (jumpSpeed * 0.5f));
			if (fallSpeed > mMaxFallSpeed) fallSpeed = mMaxFallSpeed;

			PathingNode* pClosestNode =
				mPathingGraph->FindClosestNode(transform.GetTranslation());
			if (pClosestNode != NULL)
			{
				if (pClosestNode != pFallingNode)
					deltaTime = 0.01f;

				if (eastl::find(nodes.begin(), nodes.end(), pClosestNode) == nodes.end())
					nodes.push_back(pClosestNode);
				nodeTimes[pClosestNode] = deltaTime;
				nodePositions[pClosestNode] = transform.GetTranslation();
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
		totalTime += 0.01f;

		//we store the fall if we find a landing node
		if (!nodes.empty())
		{
			Vector3<float> position = transform.GetTranslation();
			PathingNode* pEndNode = mPathingGraph->FindClosestNode(position);
			if (pEndNode != NULL && pNode->FindArc(AIAT_FALLTARGET, pEndNode) == NULL)
			{
				Vector3<float> diff = pEndNode->GetPos() - position;
				if (Length(diff) <= 6.f)
				{
					PathingNode* pCurrentNode = pNode;
					eastl::vector<PathingNode*>::iterator itNode;
					for (itNode = nodes.begin(); itNode != nodes.end(); itNode++)
					{
						PathingNode* pFallingNode = (*itNode);
						if (pCurrentNode == pNode)
						{
							PathingArc* pArc = new PathingArc(AIAT_FALLTARGET, totalTime);
							pArc->LinkNodes(pEndNode, pFallingNode);
							pCurrentNode->AddArc(pArc);
						}
						PathingArc* pArc = new PathingArc(
							AIAT_FALL, nodeTimes[pFallingNode], nodePositions[pFallingNode]);
						pArc->LinkNodes(pEndNode, pFallingNode);
						pCurrentNode->AddArc(pArc);

						pCurrentNode = pFallingNode;
					}

					if (pCurrentNode != pEndNode)
					{
						deltaTime = 0.01f;
						PathingArc* pArc = new PathingArc(AIAT_FALL, deltaTime);
						pArc->LinkNodes(pEndNode, pEndNode);
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
		eastl::shared_ptr<PhysicComponent> pPhysicComponent(
			mPlayerActor->GetComponent<PhysicComponent>(PhysicComponent::Name).lock());
		if (pPhysicComponent->OnGround())
		{
			if (pItemActor->GetType() == "Weapon")
			{
				eastl::shared_ptr<WeaponPickup> pWeaponPickup =
					pItemActor->GetComponent<WeaponPickup>(WeaponPickup::Name).lock();
				mActorNodes[pPhysicComponent->GetTransform().GetTranslation()] = pItemActor->GetId();
			}
			else if (pItemActor->GetType() == "Ammo")
			{
				eastl::shared_ptr<AmmoPickup> pAmmoPickup =
					pItemActor->GetComponent<AmmoPickup>(AmmoPickup::Name).lock();
				mActorNodes[pPhysicComponent->GetTransform().GetTranslation()] = pItemActor->GetId();
			}
			else if (pItemActor->GetType() == "Armor")
			{
				eastl::shared_ptr<ArmorPickup> pArmorPickup =
					pItemActor->GetComponent<ArmorPickup>(ArmorPickup::Name).lock();
				mActorNodes[pPhysicComponent->GetTransform().GetTranslation()] = pItemActor->GetId();
			}
			else if (pItemActor->GetType() == "Health")
			{
				eastl::shared_ptr<HealthPickup> pHealthPickup =
					pItemActor->GetComponent<HealthPickup>(HealthPickup::Name).lock();
				mActorNodes[pPhysicComponent->GetTransform().GetTranslation()] = pItemActor->GetId();
			}
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
			eastl::shared_ptr<PhysicComponent> pPhysicComponent(
				mPlayerActor->GetComponent<PhysicComponent>(PhysicComponent::Name).lock());
			if (pPhysicComponent->OnGround())
			{
				if (pItemActor->GetType() == "Trigger")
				{
					if (pItemActor->GetComponent<PushTrigger>(PushTrigger::Name).lock())
					{
						mActorNodes[pPhysicComponent->GetTransform().GetTranslation()] = pItemActor->GetId();
					}
					else if (pItemActor->GetComponent<TeleporterTrigger>(TeleporterTrigger::Name).lock())
					{
						mActorNodes[pPhysicComponent->GetTransform().GetTranslation()] = pItemActor->GetId();
					}
				}
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