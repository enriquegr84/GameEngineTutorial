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
#include "Core/IO/XmlResource.h"
#include "Core/Event/EventManager.h"
#include "Core/Event/Event.h"

#include "Physic/PhysicEventListener.h"

#include "QuakeEvents.h"
#include "QuakeView.h"
#include "QuakeApp.h"
#include "Quake.h"

QuakeAIManager::QuakeAIManager() : AIManager()
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
}   // QuakeAIManager

//-----------------------------------------------------------------------------

QuakeAIManager::~QuakeAIManager()
{

}   // ~QuakeAIManager


/////////////////////////////////////////////////////////////////////////////
// QuakeAIManager::LoadXml
//
//    Loads the AI pathing graph information from an XML file
//
void QuakeAIManager::LoadPathingGraph(const eastl::wstring& path)
{
	mPathingGraph = eastl::make_shared<PathingGraph>();

	// Load the physics config file and grab the root XML node
	tinyxml2::XMLElement* pRoot = XmlResourceLoader::LoadAndReturnRootXMLElement(path.c_str());
	LogAssert(pRoot, "AI xml doesn't exists");

	eastl::map<unsigned int, PathingNode*> pathingGraph;
	for (tinyxml2::XMLElement* pNode = pRoot->FirstChildElement(); pNode; pNode = pNode->NextSiblingElement())
	{
		int pathNodeId = 0;
		ActorId actorId = INVALID_ACTOR_ID;
		float tolerance = PATHING_DEFAULT_NODE_TOLERANCE;
		Vector3<float> position = Vector3<float>::Zero();

		pathNodeId = pNode->IntAttribute("id", pathNodeId);
		actorId = pNode->IntAttribute("actorid", actorId);
		tolerance = pNode->FloatAttribute("tolerance", tolerance);

		tinyxml2::XMLElement* pPositionElement = pNode->FirstChildElement("Position");
		if (pPositionElement)
		{
			float x = 0;
			float y = 0;
			float z = 0;
			x = pPositionElement->FloatAttribute("x", x);
			y = pPositionElement->FloatAttribute("y", y);
			z = pPositionElement->FloatAttribute("z", z);

			position = Vector3<float>{ x, y, z };
		}

		PathingNode* pathNode = new PathingNode(pathNodeId, actorId, position, tolerance);
		pathingGraph[pathNodeId] = pathNode;
		mPathingGraph->InsertNode(pathNode);
	}

	for (tinyxml2::XMLElement* pNode = pRoot->FirstChildElement(); pNode; pNode = pNode->NextSiblingElement())
	{
		int pathNodeId = 0;
		pathNodeId = pNode->IntAttribute("id", pathNodeId);
		PathingNode* pathNode = pathingGraph[pathNodeId];

		for (tinyxml2::XMLElement* pArc = pNode->FirstChildElement("Arc"); pArc; pArc = pArc->NextSiblingElement())
		{
			int arcType = 0;
			float weight = 0.f;
			PathingNode* links[2];
			Vector3<float> connection = NULL;

			arcType = pArc->IntAttribute("type", arcType);
			weight = pArc->FloatAttribute("weight", weight);

			tinyxml2::XMLElement* pLinkElement = pArc->FirstChildElement("Link");
			if (pLinkElement)
			{
				int nodeA = 0;
				int nodeB = 0;
				nodeA = pLinkElement->IntAttribute("a", nodeA);
				nodeB = pLinkElement->IntAttribute("b", nodeB);

				links[0] = pathingGraph[nodeA];
				links[1] = pathingGraph[nodeB];
			}

			tinyxml2::XMLElement* pConnectionElement = pArc->FirstChildElement("Connection");
			if (pConnectionElement)
			{
				float x = 0;
				float y = 0;
				float z = 0;
				x = pConnectionElement->FloatAttribute("x", x);
				y = pConnectionElement->FloatAttribute("y", y);
				z = pConnectionElement->FloatAttribute("z", z);

				connection = Vector3<float>{ x, y, z };
			}

			PathingArc* pathArc = new PathingArc(arcType, weight, connection);
			pathArc->LinkNodes(links[0], links[1]);
			pathNode->AddArc(pathArc);
		}
	}
	/*
	GameApplication* gameApp = (GameApplication*)Application::App;
	const GameViewList& gameViews = gameApp->GetGameViews();
	for (auto it = gameViews.begin(); it != gameViews.end(); ++it)
	{
		eastl::shared_ptr<BaseGameView> pView = *it;
		if (pView->GetType() == GV_HUMAN)
		{
			eastl::shared_ptr<QuakeHumanView> pHumanView =
				eastl::static_pointer_cast<QuakeHumanView, BaseGameView>(pView);
			mPlayerActor =
				eastl::dynamic_shared_pointer_cast<PlayerActor>(
				GameLogic::Get()->GetActor(pHumanView->GetActorId()).lock());
			break;
		}
	}

	QuakeLogic* game = static_cast<QuakeLogic *>(GameLogic::Get());
	game->GetGamePhysics()->SetTriggerCollision(true);
	game->RemoveAllDelegates();
	RegisterAllDelegates();

	eastl::shared_ptr<BaseGamePhysic> gamePhysics = GameLogic::Get()->GetGamePhysics();

	for (PathingNode* pathNode : mPathingGraph->GetNodes())
	{
		if (pathNode->GetActorId() != INVALID_ACTOR_ID)
		{
			eastl::shared_ptr<Actor> pItemActor(
				GameLogic::Get()->GetActor(pathNode->GetActorId()).lock());

			if (pItemActor->GetType() == "Trigger")
			{
				if (pItemActor->GetComponent<PushTrigger>(PushTrigger::Name).lock())
				{
					eastl::shared_ptr<PushTrigger> pPushTrigger =
						pItemActor->GetComponent<PushTrigger>(PushTrigger::Name).lock();

					Vector3<float> targetPosition = pPushTrigger->GetTarget().GetTranslation();
					SimulateTriggerPush(pathNode, targetPosition);
				}
				else if (pItemActor->GetComponent<TeleporterTrigger>(TeleporterTrigger::Name).lock())
				{
					eastl::shared_ptr<TeleporterTrigger> pTeleporterTrigger =
						pItemActor->GetComponent<TeleporterTrigger>(TeleporterTrigger::Name).lock();

					Vector3<float> targetPosition = pTeleporterTrigger->GetTarget().GetTranslation();
					SimulateTriggerTeleport(pathNode, targetPosition);
				}
			}
		}
	}

	eastl::string levelPath = "ai/quake/bloodrun - copia.xml";
	SavePathingGraph(FileSystem::Get()->GetPath(levelPath));
	*/
}

/////////////////////////////////////////////////////////////////////////////
// QuakeAIManager::SavePathingGraph
//
//    Saves the AI pathing graph information to an XML file
//
void QuakeAIManager::SavePathingGraph(const eastl::string& path)
{
	tinyxml2::XMLDocument doc;

	// base element
	tinyxml2::XMLElement* pBaseElement = doc.NewElement("PathingGraph");
	doc.InsertFirstChild(pBaseElement);

	for (PathingNode* pathNode : mPathingGraph->GetNodes())
	{
		tinyxml2::XMLElement* pNode = doc.NewElement("Node");
		pNode->SetAttribute("id", eastl::to_string(pathNode->GetId()).c_str());
		pNode->SetAttribute("actorid", eastl::to_string(pathNode->GetActorId()).c_str());
		pNode->SetAttribute("tolerance", eastl::to_string(pathNode->GetTolerance()).c_str());
		pBaseElement->LinkEndChild(pNode);

		tinyxml2::XMLElement* pPosition = doc.NewElement("Position");
		pPosition->SetAttribute("x", eastl::to_string((int)round(pathNode->GetPos()[0])).c_str());
		pPosition->SetAttribute("y", eastl::to_string((int)round(pathNode->GetPos()[1])).c_str());
		pPosition->SetAttribute("z", eastl::to_string((int)round(pathNode->GetPos()[2])).c_str());
		pNode->LinkEndChild(pPosition);

		for (PathingArc* pathArc : pathNode->GetArcs())
		{
			tinyxml2::XMLElement* pArc = doc.NewElement("Arc");
			pArc->SetAttribute("type", eastl::to_string(pathArc->GetType()).c_str());
			pArc->SetAttribute("weight", eastl::to_string(pathArc->GetWeight()).c_str());
			pNode->LinkEndChild(pArc);

			tinyxml2::XMLElement* pLink = doc.NewElement("Link");
			pLink->SetAttribute("a", eastl::to_string(pathArc->GetOrigin()->GetId()).c_str());
			pLink->SetAttribute("b", eastl::to_string(pathArc->GetNeighbor()->GetId()).c_str());
			pArc->LinkEndChild(pLink);

			if (pathArc->GetConnection() != NULL)
			{
				tinyxml2::XMLElement* pConnection = doc.NewElement("Connection");
				pConnection->SetAttribute("x", eastl::to_string((int)round(pathArc->GetConnection()[0])).c_str());
				pConnection->SetAttribute("y", eastl::to_string((int)round(pathArc->GetConnection()[1])).c_str());
				pConnection->SetAttribute("z", eastl::to_string((int)round(pathArc->GetConnection()[2])).c_str());
				pArc->LinkEndChild(pConnection);
			}
		}
	}

	doc.SaveFile(path.c_str());
}

//waypoint generation via physics simulation
void QuakeAIManager::CreateWaypoints(ActorId playerId)
{
	GameApplication* gameApp = (GameApplication*)Application::App;
	QuakeLogic* game = static_cast<QuakeLogic *>(GameLogic::Get());

	mPlayerActor = 
		eastl::dynamic_shared_pointer_cast<PlayerActor>(
		GameLogic::Get()->GetActor(playerId).lock());

	game->GetGamePhysics()->SetTriggerCollision(true);
	game->RemoveAllDelegates();
	RegisterAllDelegates();

	//first we store the most important points of the map
	mPathingGraph = eastl::make_shared<PathingGraph>();

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

	// we create the waypoint according to the character controller physics system. Every
	// simulation step, it will be generated new waypoints from different actions such as
	// movement, jumping or falling and its conections
	SimulateWaypoint();

	Level* level = game->GetLevelManager()->GetLevel(ToWideString(gameApp->mOption.mLevel.c_str()));
	eastl::string levelPath = "ai/quake/" + ToString(level->GetName().c_str()) + ".xml";
	SavePathingGraph(FileSystem::Get()->GetPath(levelPath));

	SimulateFiring();

	game->GetGamePhysics()->SetTriggerCollision(false);
	game->RegisterAllDelegates();

	RemoveAllDelegates();
}

void QuakeAIManager::SimulateFiring()
{
	for (PathingNode* pNode : mPathingGraph->GetNodes())
	{
		for (unsigned int weapon = 1; weapon <= 8; weapon++)
		{
			// fire the specific weapon
			switch (weapon)
			{
				case WP_GAUNTLET:
					SimulateGauntletAttack(pNode);
					break;
				case WP_SHOTGUN:
					SimulateShotgunFire(pNode);
					break;
				case WP_MACHINEGUN:
					SimulateBulletFire(pNode, MACHINEGUN_SPREAD, MACHINEGUN_DAMAGE);
					break;
				case WP_RAILGUN:
					SimulateRailgunFire(pNode);
					break;
				case WP_LIGHTNING:
					SimulateLightningFire(pNode);
					break;
				default:
					// FIXME Error( "Bad weapon" );
					break;
			}
		}
	}

	eastl::shared_ptr<Actor> pGameActor(
		GameLogic::Get()->CreateActor("actors/quake/effects/simulategrenadelauncherfire.xml", nullptr));
	mActorCollisions[pGameActor->GetId()] = false;
	eastl::shared_ptr<PhysicComponent> pPhysicalComponent =
		pGameActor->GetComponent<PhysicComponent>(PhysicComponent::Name).lock();
	if (pPhysicalComponent)
		pPhysicalComponent->SetIgnoreCollision(mPlayerActor->GetId(), true);
	for (PathingNode* pNode : mPathingGraph->GetNodes())
		SimulateGrenadeLauncherFire(pNode, pGameActor);
	EventManager::Get()->TriggerEvent(
		eastl::make_shared<EventDataRequestDestroyActor>(pGameActor->GetId()));

	pGameActor = GameLogic::Get()->CreateActor("actors/quake/effects/simulaterocketlauncherfire.xml", nullptr);
	mActorCollisions[pGameActor->GetId()] = false;
	pPhysicalComponent = pGameActor->GetComponent<PhysicComponent>(PhysicComponent::Name).lock();
	if (pPhysicalComponent)
	{
		pPhysicalComponent->SetIgnoreCollision(mPlayerActor->GetId(), true);
		pPhysicalComponent->SetGravity(Vector3<float>::Zero());
	}
	for (PathingNode* pNode : mPathingGraph->GetNodes())
		SimulateRocketLauncherFire(pNode, pGameActor);
	EventManager::Get()->TriggerEvent(
		eastl::make_shared<EventDataRequestDestroyActor>(pGameActor->GetId()));

	pGameActor = GameLogic::Get()->CreateActor("actors/quake/effects/simulateplasmagunfire.xml", nullptr);
	mActorCollisions[pGameActor->GetId()] = false;
	pPhysicalComponent = pGameActor->GetComponent<PhysicComponent>(PhysicComponent::Name).lock();
	if (pPhysicalComponent)
	{
		pPhysicalComponent->SetIgnoreCollision(mPlayerActor->GetId(), true);
		pPhysicalComponent->SetGravity(Vector3<float>::Zero());
	}
	for (PathingNode* pNode : mPathingGraph->GetNodes())
		SimulatePlasmagunFire(pNode, pGameActor);
	EventManager::Get()->TriggerEvent(
		eastl::make_shared<EventDataRequestDestroyActor>(pGameActor->GetId()));

	mActorCollisions.clear();
	mActorNodes.clear();
}

void QuakeAIManager::SimulateGauntletAttack(PathingNode* pNode)
{
	eastl::shared_ptr<BaseGamePhysic> gamePhysics = GameLogic::Get()->GetGamePhysics();

	Transform transform;
	for (PathingNode* pathNode : mPathingGraph->GetNodes())
	{
		// set aiming directions
		Vector3<float> direction = pathNode->GetPos() - pNode->GetPos();
		if (direction != Vector3<float>::Zero())
		{
			//set muzzle location relative to pivoting eye
			Vector3<float> muzzle = pNode->GetPos();
			muzzle[2] += mPlayerActor->GetState().viewHeight;
			muzzle -= Vector3<float>::Unit(ROLL) * 11.f;

			direction = pathNode->GetPos() - muzzle;
			float scale = Length(direction);
			Normalize(direction);

			Matrix4x4<float> yawRotation = Rotation<4, float>(
				AxisAngle<4, float>(Vector4<float>::Unit(2), atan2(direction[1], direction[0])));
			Matrix4x4<float> pitchRotation = Rotation<4, float>(
				AxisAngle<4, float>(Vector4<float>::Unit(1), -asin(direction[2])));
			Matrix4x4<float> rotation = yawRotation * pitchRotation;

			Vector3<float> forward = HProject(rotation * Vector4<float>::Unit(PITCH));
			Vector3<float> end = muzzle + forward * 32.f;

			transform.SetTranslation(pNode->GetPos());
			gamePhysics->SetTransform(mPlayerActor->GetId(), transform);

			Vector3<float> collision, collisionNormal;
			ActorId collisionId = gamePhysics->CastRay(muzzle, end, collision, collisionNormal);
			if (collisionId == mPlayerActor->GetId())
			{
				direction = collision - muzzle;
				Normalize(direction);
				mWeaponGroundDirection[WP_GAUNTLET - 1][pNode][pathNode] = direction;
				mWeaponGroundDamageTime[WP_GAUNTLET - 1][pNode][pathNode] = 0.f;
				mWeaponGroundDamage[WP_GAUNTLET - 1][pNode][pathNode] = 50;
			}
		}

		// handle jumping and falling nodes
		for (PathingArc* pathArc : pathNode->GetArcs())
		{
			if (pathArc->GetType() == AIAT_JUMP || pathArc->GetType() == AIAT_FALL)
			{
				direction = pathArc->GetConnection() - pNode->GetPos();
				if (direction != Vector3<float>::Zero())
				{
					//set muzzle location relative to pivoting eye
					Vector3<float> muzzle = pNode->GetPos();
					muzzle[2] += mPlayerActor->GetState().viewHeight;
					muzzle -= Vector3<float>::Unit(ROLL) * 11.f;

					direction = pathArc->GetConnection() - muzzle;
					float scale = Length(direction);
					Normalize(direction);

					Matrix4x4<float> yawRotation = Rotation<4, float>(
						AxisAngle<4, float>(Vector4<float>::Unit(2), atan2(direction[1], direction[0])));
					Matrix4x4<float> pitchRotation = Rotation<4, float>(
						AxisAngle<4, float>(Vector4<float>::Unit(1), -asin(direction[2])));
					Matrix4x4<float> rotation = yawRotation * pitchRotation;

					Vector3<float> forward = HProject(rotation * Vector4<float>::Unit(PITCH));
					Vector3<float> end = muzzle + forward * 32.f;

					transform.SetTranslation(pNode->GetPos());
					gamePhysics->SetTransform(mPlayerActor->GetId(), transform);

					Vector3<float> collision, collisionNormal;
					ActorId collisionId = gamePhysics->CastRay(muzzle, end, collision, collisionNormal);
					if (collisionId == mPlayerActor->GetId())
					{
						direction = collision - muzzle;
						Normalize(direction);
						mWeaponDirection[WP_GAUNTLET - 1][pNode][pathArc] = direction;
						mWeaponDamage[WP_GAUNTLET - 1][pNode][pathArc] = 50;
					}
				}
			}
		}
	}
}

void QuakeAIManager::SimulateBulletFire(PathingNode* pNode, float spread, int damage)
{
	eastl::shared_ptr<BaseGamePhysic> gamePhysics = GameLogic::Get()->GetGamePhysics();

	Transform transform;
	for (PathingNode* pathNode : mPathingGraph->GetNodes())
	{
		// set aiming directions
		Vector3<float> direction = pathNode->GetPos() - pNode->GetPos();
		if (direction != Vector3<float>::Zero())
		{
			//set muzzle location relative to pivoting eye
			Vector3<float> muzzle = pNode->GetPos();
			muzzle[2] += mPlayerActor->GetState().viewHeight;
			muzzle -= Vector3<float>::Unit(ROLL) * 11.f;

			direction = pathNode->GetPos() - muzzle;
			float scale = Length(direction);
			Normalize(direction);

			Matrix4x4<float> yawRotation = Rotation<4, float>(
				AxisAngle<4, float>(Vector4<float>::Unit(2), atan2(direction[1], direction[0])));
			Matrix4x4<float> pitchRotation = Rotation<4, float>(
				AxisAngle<4, float>(Vector4<float>::Unit(1), -asin(direction[2])));
			Matrix4x4<float> rotation = yawRotation * pitchRotation;

			Vector3<float> forward = HProject(rotation * Vector4<float>::Unit(PITCH));
			Vector3<float> right = HProject(rotation * Vector4<float>::Unit(ROLL));
			Vector3<float> up = HProject(rotation * Vector4<float>::Unit(YAW));

			float r = (Randomizer::Rand() & 0x7fff) / ((float)0x7fff) * (float)GE_C_PI * 2.f;
			float u = sin(r) * (2.f * ((Randomizer::Rand() & 0x7fff) / ((float)0x7fff) - 0.5f)) * spread  * 16.f;
			r = cos(r) * (2.f * ((Randomizer::Rand() & 0x7fff) / ((float)0x7fff) - 0.5f)) * spread  * 16.f;
			Vector3<float> end = muzzle + forward * 8192.f * 16.f;
			end += right * r;
			end += up * u;

			transform.SetTranslation(pNode->GetPos());
			gamePhysics->SetTransform(mPlayerActor->GetId(), transform);

			Vector3<float> collision, collisionNormal;
			ActorId collisionId = gamePhysics->CastRay(muzzle, end, collision, collisionNormal);
			if (collisionId == mPlayerActor->GetId())
			{
				direction = collision - muzzle;
				Normalize(direction);
				mWeaponGroundDirection[WP_MACHINEGUN - 1][pNode][pathNode] = direction;
				mWeaponGroundDamageTime[WP_MACHINEGUN - 1][pNode][pathNode] = 0.f;
				mWeaponGroundDamage[WP_MACHINEGUN - 1][pNode][pathNode] = damage;
			}
		}

		// handle jumping and falling nodes
		for (PathingArc* pathArc : pathNode->GetArcs())
		{
			if (pathArc->GetType() == AIAT_JUMP || pathArc->GetType() == AIAT_FALL)
			{
				direction = pathArc->GetConnection() - pNode->GetPos();
				if (direction != Vector3<float>::Zero())
				{
					//set muzzle location relative to pivoting eye
					Vector3<float> muzzle = pNode->GetPos();
					muzzle[2] += mPlayerActor->GetState().viewHeight;
					muzzle -= Vector3<float>::Unit(ROLL) * 11.f;

					direction = pathArc->GetConnection() - muzzle;
					float scale = Length(direction);
					Normalize(direction);

					Matrix4x4<float> yawRotation = Rotation<4, float>(
						AxisAngle<4, float>(Vector4<float>::Unit(2), atan2(direction[1], direction[0])));
					Matrix4x4<float> pitchRotation = Rotation<4, float>(
						AxisAngle<4, float>(Vector4<float>::Unit(1), -asin(direction[2])));
					Matrix4x4<float> rotation = yawRotation * pitchRotation;

					Vector3<float> forward = HProject(rotation * Vector4<float>::Unit(PITCH));
					Vector3<float> right = HProject(rotation * Vector4<float>::Unit(ROLL));
					Vector3<float> up = HProject(rotation * Vector4<float>::Unit(YAW));

					float r = (Randomizer::Rand() & 0x7fff) / ((float)0x7fff) * (float)GE_C_PI * 2.f;
					float u = sin(r) * (2.f * ((Randomizer::Rand() & 0x7fff) / ((float)0x7fff) - 0.5f)) * spread  * 16.f;
					r = cos(r) * (2.f * ((Randomizer::Rand() & 0x7fff) / ((float)0x7fff) - 0.5f)) * spread  * 16.f;
					Vector3<float> end = muzzle + forward * 8192.f * 16.f;
					end += right * r;
					end += up * u;

					transform.SetTranslation(pNode->GetPos());
					gamePhysics->SetTransform(mPlayerActor->GetId(), transform);

					Vector3<float> collision, collisionNormal;
					ActorId collisionId = gamePhysics->CastRay(muzzle, end, collision, collisionNormal);
					if (collisionId == mPlayerActor->GetId())
					{
						direction = collision - muzzle;
						Normalize(direction);
						mWeaponDirection[WP_MACHINEGUN - 1][pNode][pathArc] = direction;
						mWeaponDamage[WP_MACHINEGUN - 1][pNode][pathArc] = damage;
					}
				}
			}
		}
	}
}

// DEFAULT_SHOTGUN_SPREAD and DEFAULT_SHOTGUN_COUNT	are in bg_public.h, because
// client predicts same spreads
#define	DEFAULT_SHOTGUN_DAMAGE	10

void QuakeAIManager::SimulateShotgunFire(PathingNode* pNode)
{
	eastl::shared_ptr<BaseGamePhysic> gamePhysics = GameLogic::Get()->GetGamePhysics();

	Transform transform;
	for (PathingNode* pathNode : mPathingGraph->GetNodes())
	{
		// set aiming directions
		Vector3<float> direction = pathNode->GetPos() - pNode->GetPos();
		if (direction != Vector3<float>::Zero())
		{
			//set muzzle location relative to pivoting eye
			Vector3<float> muzzle = pNode->GetPos();
			muzzle[2] += mPlayerActor->GetState().viewHeight;
			muzzle -= Vector3<float>::Unit(ROLL) * 11.f;

			direction = pathNode->GetPos() - muzzle;
			float scale = Length(direction);
			Normalize(direction);

			Matrix4x4<float> yawRotation = Rotation<4, float>(
				AxisAngle<4, float>(Vector4<float>::Unit(2), atan2(direction[1], direction[0])));
			Matrix4x4<float> pitchRotation = Rotation<4, float>(
				AxisAngle<4, float>(Vector4<float>::Unit(1), -asin(direction[2])));
			Matrix4x4<float> rotation = yawRotation * pitchRotation;

			Vector3<float> forward = HProject(rotation * Vector4<float>::Unit(PITCH));
			Vector3<float> right = HProject(rotation * Vector4<float>::Unit(ROLL));
			Vector3<float> up = HProject(rotation * Vector4<float>::Unit(YAW));

			// generate the "random" spread pattern
			for (unsigned int i = 0; i < DEFAULT_SHOTGUN_COUNT; i++)
			{
				float r = (2.f * ((Randomizer::Rand() & 0x7fff) / ((float)0x7fff) - 0.5f)) * DEFAULT_SHOTGUN_SPREAD * 16.f;
				float u = (2.f * ((Randomizer::Rand() & 0x7fff) / ((float)0x7fff) - 0.5f)) * DEFAULT_SHOTGUN_SPREAD * 16.f;
				Vector3<float> end = muzzle + forward * 8192.f * 16.f;
				end += right * r;
				end += up * u;

				transform.SetTranslation(pNode->GetPos());
				gamePhysics->SetTransform(mPlayerActor->GetId(), transform);

				Vector3<float> collision, collisionNormal;
				ActorId collisionId = gamePhysics->CastRay(muzzle, end, collision, collisionNormal);
				if (collisionId == mPlayerActor->GetId())
				{
					direction = collision - muzzle;
					Normalize(direction);
					mWeaponGroundDirection[WP_SHOTGUN - 1][pNode][pathNode] = direction;
					mWeaponGroundDamageTime[WP_SHOTGUN - 1][pNode][pathNode] = 0.f;
					if (mWeaponGroundDamage[WP_SHOTGUN - 1].find(pNode) != 
						mWeaponGroundDamage[WP_SHOTGUN - 1].end())
					{
						if (mWeaponGroundDamage[WP_SHOTGUN - 1][pNode].find(pathNode) ==
							mWeaponGroundDamage[WP_SHOTGUN - 1][pNode].end())
						{
							mWeaponGroundDamage[WP_SHOTGUN - 1][pNode][pathNode] = 0;
						}
					}
					else mWeaponGroundDamage[WP_SHOTGUN - 1][pNode][pathNode] = 0;
					mWeaponGroundDamage[WP_SHOTGUN - 1][pNode][pathNode] += DEFAULT_SHOTGUN_DAMAGE;
				}
			}
		}

		// handle jumping and falling nodes
		for (PathingArc* pathArc : pathNode->GetArcs())
		{
			if (pathArc->GetType() == AIAT_JUMP || pathArc->GetType() == AIAT_FALL)
			{
				direction = pathArc->GetConnection() - pNode->GetPos();
				if (direction != Vector3<float>::Zero())
				{
					//set muzzle location relative to pivoting eye
					Vector3<float> muzzle = pNode->GetPos();
					muzzle[2] += mPlayerActor->GetState().viewHeight;
					muzzle -= Vector3<float>::Unit(ROLL) * 11.f;

					direction = pathArc->GetConnection() - muzzle;
					float scale = Length(direction);
					Normalize(direction);

					Matrix4x4<float> yawRotation = Rotation<4, float>(
						AxisAngle<4, float>(Vector4<float>::Unit(2), atan2(direction[1], direction[0])));
					Matrix4x4<float> pitchRotation = Rotation<4, float>(
						AxisAngle<4, float>(Vector4<float>::Unit(1), -asin(direction[2])));
					Matrix4x4<float> rotation = yawRotation * pitchRotation;

					Vector3<float> forward = HProject(rotation * Vector4<float>::Unit(PITCH));
					Vector3<float> right = HProject(rotation * Vector4<float>::Unit(ROLL));
					Vector3<float> up = HProject(rotation * Vector4<float>::Unit(YAW));

					// generate the "random" spread pattern
					for (unsigned int i = 0; i < DEFAULT_SHOTGUN_COUNT; i++)
					{
						float r = (2.f * ((Randomizer::Rand() & 0x7fff) / ((float)0x7fff) - 0.5f)) * DEFAULT_SHOTGUN_SPREAD * 16.f;
						float u = (2.f * ((Randomizer::Rand() & 0x7fff) / ((float)0x7fff) - 0.5f)) * DEFAULT_SHOTGUN_SPREAD * 16.f;
						Vector3<float> end = muzzle + forward * 8192.f * 16.f;
						end += right * r;
						end += up * u;

						transform.SetTranslation(pNode->GetPos());
						gamePhysics->SetTransform(mPlayerActor->GetId(), transform);

						Vector3<float> collision, collisionNormal;
						ActorId collisionId = gamePhysics->CastRay(muzzle, end, collision, collisionNormal);
						if (collisionId == mPlayerActor->GetId())
						{
							direction = collision - muzzle;
							Normalize(direction);
							mWeaponDirection[WP_SHOTGUN - 1][pNode][pathArc] = direction;
							if (mWeaponDamage[WP_SHOTGUN - 1].find(pNode) != 
								mWeaponDamage[WP_SHOTGUN - 1].end())
							{
								if (mWeaponDamage[WP_SHOTGUN - 1][pNode].find(pathArc) ==
									mWeaponDamage[WP_SHOTGUN - 1][pNode].end())
								{
									mWeaponDamage[WP_SHOTGUN - 1][pNode][pathArc] = 0;
								}
							}
							else mWeaponDamage[WP_SHOTGUN - 1][pNode][pathArc] = 0;
							mWeaponDamage[WP_SHOTGUN - 1][pNode][pathArc] += DEFAULT_SHOTGUN_DAMAGE;
						}
					}
				}
			}
		}
	}
}

void QuakeAIManager::SimulateGrenadeLauncherFire(PathingNode* pNode, eastl::shared_ptr<Actor> pGameActor)
{
	eastl::shared_ptr<BaseGamePhysic> gamePhysics = GameLogic::Get()->GetGamePhysics();

	Transform transform;
	Vector3<float> direction;
	for (int yawAngle = 0; yawAngle < 360; yawAngle += 20)
	{
		for (int pitchAngle = -80; pitchAngle <= 80; pitchAngle += 20)
		{
			Matrix4x4<float> yawRotation = Rotation<4, float>(
				AxisAngle<4, float>(Vector4<float>::Unit(2), yawAngle * (float)GE_C_DEG_TO_RAD));
			Matrix4x4<float> pitchRotation = Rotation<4, float>(
				AxisAngle<4, float>(Vector4<float>::Unit(1), -pitchAngle * (float)GE_C_DEG_TO_RAD));
			Matrix4x4<float> rotation = yawRotation * pitchRotation;

			// forward vector
#if defined(GE_USE_MAT_VEC)
			direction = HProject(rotation * Vector4<float>::Unit(PITCH));
#else
			direction = HProject(Vector4<float>::Unit(PITCH) * rotation);
#endif

			Vector3<float> forward = HProject(rotation * Vector4<float>::Unit(PITCH));
			Vector3<float> right = HProject(rotation * Vector4<float>::Unit(ROLL));
			Vector3<float> up = HProject(rotation * Vector4<float>::Unit(YAW));

			//set muzzle location relative to pivoting eye
			Vector3<float> muzzle = pNode->GetPos();
			muzzle[2] += mPlayerActor->GetState().viewHeight;
			muzzle -= Vector3<float>::Unit(ROLL) * 11.f;

			float r = (Randomizer::Rand() & 0x7fff) / ((float)0x7fff) * (float)GE_C_PI * 2.f;
			float u = sin(r) * (2.f * ((Randomizer::Rand() & 0x7fff) / ((float)0x7fff) - 0.5f)) * 16.f;
			r = cos(r) * (2.f * ((Randomizer::Rand() & 0x7fff) / ((float)0x7fff) - 0.5f)) * 16.f;
			Vector3<float> end = muzzle + forward * 8192.f * 16.f;
			end += right * r;
			end += up * u;

			direction[PITCH] *= 800000.f;
			direction[ROLL] *= 800000.f;
			direction[YAW] *= 500000.f;

			// projectile simulation
			transform.SetTranslation(muzzle);
			transform.SetRotation(rotation);
			gamePhysics->SetTransform(pGameActor->GetId(), transform);
			gamePhysics->ApplyForce(pGameActor->GetId(), direction);
			gamePhysics->OnUpdate(0.01f);

			float deltaTime = 0.f;
			while (deltaTime < 3.0f)
			{
				gamePhysics->OnUpdate(0.1f);
				deltaTime += 0.1f;
			}

			transform = gamePhysics->GetTransform(pGameActor->GetId());

			PathingNodeVec impactNodes;
			mPathingGraph->FindNodes(impactNodes, transform.GetTranslation(), 150);
			for (PathingNode* impactNode : impactNodes)
			{
				float dist = Length(transform.GetTranslation() - impactNode->GetPos());
				float damage = 100 * (1.f - dist / 150); // calculate radius damage

				direction = transform.GetTranslation() - muzzle;
				Normalize(direction);
				mWeaponGroundDirection[WP_GRENADE_LAUNCHER - 1][pNode][impactNode] = direction;
				mWeaponGroundDamageTime[WP_GRENADE_LAUNCHER - 1][pNode][impactNode] = deltaTime;
				mWeaponGroundDamage[WP_GRENADE_LAUNCHER - 1][pNode][impactNode] = damage;
			}
		}
	}
}

void QuakeAIManager::SimulateRocketLauncherFire(PathingNode* pNode, eastl::shared_ptr<Actor> pGameActor)
{
	eastl::shared_ptr<BaseGamePhysic> gamePhysics = GameLogic::Get()->GetGamePhysics();

	Transform transform;
	Vector3<float> direction;
	for (int yawAngle = 0; yawAngle < 360; yawAngle += 20)
	{
		for (int pitchAngle = -80; pitchAngle <= 80; pitchAngle += 20)
		{
			Matrix4x4<float> yawRotation = Rotation<4, float>(
				AxisAngle<4, float>(Vector4<float>::Unit(2), yawAngle * (float)GE_C_DEG_TO_RAD));
			Matrix4x4<float> pitchRotation = Rotation<4, float>(
				AxisAngle<4, float>(Vector4<float>::Unit(1), -pitchAngle * (float)GE_C_DEG_TO_RAD));
			Matrix4x4<float> rotation = yawRotation * pitchRotation;

			// forward vector
#if defined(GE_USE_MAT_VEC)
			direction = HProject(rotation * Vector4<float>::Unit(PITCH));
#else
			direction = HProject(Vector4<float>::Unit(PITCH) * rotation);
#endif

			Vector3<float> forward = HProject(rotation * Vector4<float>::Unit(PITCH));
			Vector3<float> right = HProject(rotation * Vector4<float>::Unit(ROLL));
			Vector3<float> up = HProject(rotation * Vector4<float>::Unit(YAW));

			//set muzzle location relative to pivoting eye
			Vector3<float> muzzle = pNode->GetPos();
			muzzle[2] += mPlayerActor->GetState().viewHeight;
			muzzle -= Vector3<float>::Unit(ROLL) * 11.f;

			float r = (Randomizer::Rand() & 0x7fff) / ((float)0x7fff) * (float)GE_C_PI * 2.f;
			float u = sin(r) * (2.f * ((Randomizer::Rand() & 0x7fff) / ((float)0x7fff) - 0.5f)) * 16.f;
			r = cos(r) * (2.f * ((Randomizer::Rand() & 0x7fff) / ((float)0x7fff) - 0.5f)) * 16.f;
			Vector3<float> end = muzzle + forward * 8192.f * 16.f;
			end += right * r;
			end += up * u;

			direction[PITCH] *= 1000000.f;
			direction[ROLL] *= 1000000.f;
			direction[YAW] *= 1000000.f;

			// projectile simulation
			transform.SetTranslation(muzzle);
			transform.SetRotation(rotation);
			gamePhysics->SetTransform(pGameActor->GetId(), transform);
			gamePhysics->ApplyForce(pGameActor->GetId(), direction);
			gamePhysics->OnUpdate(0.01f);

			float deltaTime = 0.f;
			while (!mActorCollisions[pGameActor->GetId()] || deltaTime < 10.0f)
			{
				gamePhysics->OnUpdate(0.1f);
				deltaTime += 0.1f;
			}
			mActorCollisions[pGameActor->GetId()] = false;

			transform = gamePhysics->GetTransform(pGameActor->GetId());

			PathingNodeVec impactNodes;
			mPathingGraph->FindNodes(impactNodes, transform.GetTranslation(), 120);
			for (PathingNode* impactNode : impactNodes)
			{
				float dist = Length(transform.GetTranslation() - impactNode->GetPos());
				float damage = 100 * (1.f - dist / 120.f); // calculate radius damage

				direction = transform.GetTranslation() - muzzle;
				Normalize(direction);
				mWeaponGroundDirection[WP_ROCKET_LAUNCHER - 1][pNode][impactNode] = direction;
				mWeaponGroundDamageTime[WP_ROCKET_LAUNCHER - 1][pNode][impactNode] = deltaTime;
				mWeaponGroundDamage[WP_ROCKET_LAUNCHER - 1][pNode][impactNode] = damage;
			}
		}
	}
}

void QuakeAIManager::SimulatePlasmagunFire(PathingNode* pNode, eastl::shared_ptr<Actor> pGameActor)
{
	eastl::shared_ptr<BaseGamePhysic> gamePhysics = GameLogic::Get()->GetGamePhysics();

	Transform transform;
	Vector3<float> direction;
	for (int yawAngle = 0; yawAngle < 360; yawAngle += 20)
	{
		for (int pitchAngle = -80; pitchAngle <= 80; pitchAngle += 20)
		{
			Matrix4x4<float> yawRotation = Rotation<4, float>(
				AxisAngle<4, float>(Vector4<float>::Unit(2), yawAngle * (float)GE_C_DEG_TO_RAD));
			Matrix4x4<float> pitchRotation = Rotation<4, float>(
				AxisAngle<4, float>(Vector4<float>::Unit(1), -pitchAngle * (float)GE_C_DEG_TO_RAD));
			Matrix4x4<float> rotation = yawRotation * pitchRotation;

			// forward vector
#if defined(GE_USE_MAT_VEC)
			direction = HProject(rotation * Vector4<float>::Unit(PITCH));
#else
			direction = HProject(Vector4<float>::Unit(PITCH) * rotation);
#endif

			Vector3<float> forward = HProject(rotation * Vector4<float>::Unit(PITCH));
			Vector3<float> right = HProject(rotation * Vector4<float>::Unit(ROLL));
			Vector3<float> up = HProject(rotation * Vector4<float>::Unit(YAW));

			//set muzzle location relative to pivoting eye
			Vector3<float> muzzle = pNode->GetPos();
			muzzle[2] += mPlayerActor->GetState().viewHeight;
			muzzle -= Vector3<float>::Unit(ROLL) * 11.f;

			float r = (Randomizer::Rand() & 0x7fff) / ((float)0x7fff) * (float)GE_C_PI * 2.f;
			float u = sin(r) * (2.f * ((Randomizer::Rand() & 0x7fff) / ((float)0x7fff) - 0.5f)) * 16.f;
			r = cos(r) * (2.f * ((Randomizer::Rand() & 0x7fff) / ((float)0x7fff) - 0.5f)) * 16.f;
			Vector3<float> end = muzzle + forward * 8192.f * 16.f;
			end += right * r;
			end += up * u;

			direction[PITCH] *= 4000.f;
			direction[ROLL] *= 4000.f;
			direction[YAW] *= 4000.f;

			// projectile simulation
			transform.SetTranslation(muzzle);
			transform.SetRotation(rotation);
			gamePhysics->SetTransform(pGameActor->GetId(), transform);
			gamePhysics->ApplyForce(pGameActor->GetId(), direction);
			gamePhysics->OnUpdate(0.01f);

			float deltaTime = 0.f;
			while (!mActorCollisions[pGameActor->GetId()] || deltaTime < 10.0f)
			{
				gamePhysics->OnUpdate(0.1f);
				deltaTime += 0.1f;
			}
			mActorCollisions[pGameActor->GetId()] = false;

			transform = gamePhysics->GetTransform(pGameActor->GetId());

			PathingNodeVec impactNodes;
			mPathingGraph->FindNodes(impactNodes, transform.GetTranslation(), 20);
			for (PathingNode* impactNode : impactNodes)
			{
				float dist = Length(transform.GetTranslation() - impactNode->GetPos());
				float damage = 15 * (1.f - dist / 20.f); // calculate radius damage

				direction = transform.GetTranslation() - muzzle;
				Normalize(direction);
				mWeaponGroundDirection[WP_PLASMAGUN - 1][pNode][impactNode] = direction;
				mWeaponGroundDamageTime[WP_PLASMAGUN - 1][pNode][impactNode] = deltaTime;
				mWeaponGroundDamage[WP_PLASMAGUN - 1][pNode][impactNode] = damage;
			}
		}
	}
}

void QuakeAIManager::SimulateRailgunFire(PathingNode* pNode)
{
	eastl::shared_ptr<BaseGamePhysic> gamePhysics = GameLogic::Get()->GetGamePhysics();

	Transform transform;
	for (PathingNode* pathNode : mPathingGraph->GetNodes())
	{
		// set aiming directions
		Vector3<float> direction = pathNode->GetPos() - pNode->GetPos();
		if (direction != Vector3<float>::Zero())
		{
			//set muzzle location relative to pivoting eye
			Vector3<float> muzzle = pNode->GetPos();
			muzzle[2] += mPlayerActor->GetState().viewHeight;
			muzzle -= Vector3<float>::Unit(ROLL) * 11.f;

			direction = pathNode->GetPos() - muzzle;
			float scale = Length(direction);
			Normalize(direction);

			Matrix4x4<float> yawRotation = Rotation<4, float>(
				AxisAngle<4, float>(Vector4<float>::Unit(2), atan2(direction[1], direction[0])));
			Matrix4x4<float> pitchRotation = Rotation<4, float>(
				AxisAngle<4, float>(Vector4<float>::Unit(1), -asin(direction[2])));
			Matrix4x4<float> rotation = yawRotation * pitchRotation;

			Vector3<float> forward = HProject(rotation * Vector4<float>::Unit(PITCH));
			Vector3<float> right = HProject(rotation * Vector4<float>::Unit(ROLL));
			Vector3<float> up = HProject(rotation * Vector4<float>::Unit(YAW));

			float r = (Randomizer::Rand() & 0x7fff) / ((float)0x7fff) * (float)GE_C_PI * 2.f;
			float u = sin(r) * (2.f * ((Randomizer::Rand() & 0x7fff) / ((float)0x7fff) - 0.5f)) * 16.f;
			r = cos(r) * (2.f * ((Randomizer::Rand() & 0x7fff) / ((float)0x7fff) - 0.5f)) * 16.f;
			Vector3<float> end = muzzle + forward * 8192.f * 16.f;
			end += right * r;
			end += up * u;

			transform.SetTranslation(pNode->GetPos());
			gamePhysics->SetTransform(mPlayerActor->GetId(), transform);

			Vector3<float> collision, collisionNormal;
			ActorId collisionId = gamePhysics->CastRay(muzzle, end, collision, collisionNormal);
			if (collisionId == mPlayerActor->GetId())
			{
				direction = collision - muzzle;
				Normalize(direction);
				mWeaponGroundDirection[WP_RAILGUN - 1][pNode][pathNode] = direction;
				mWeaponGroundDamageTime[WP_RAILGUN - 1][pNode][pathNode] = 0.f;
				mWeaponGroundDamage[WP_RAILGUN - 1][pNode][pathNode] = 100;
			}
		}

		// handle jumping and falling nodes
		for (PathingArc* pathArc : pathNode->GetArcs())
		{
			if (pathArc->GetType() == AIAT_JUMP || pathArc->GetType() == AIAT_FALL)
			{
				direction = pathArc->GetConnection() - pNode->GetPos();
				if (direction != Vector3<float>::Zero())
				{
					//set muzzle location relative to pivoting eye
					Vector3<float> muzzle = pNode->GetPos();
					muzzle[2] += mPlayerActor->GetState().viewHeight;
					muzzle -= Vector3<float>::Unit(ROLL) * 11.f;

					direction = pathArc->GetConnection() - muzzle;
					float scale = Length(direction);
					Normalize(direction);

					Matrix4x4<float> yawRotation = Rotation<4, float>(
						AxisAngle<4, float>(Vector4<float>::Unit(2), atan2(direction[1], direction[0])));
					Matrix4x4<float> pitchRotation = Rotation<4, float>(
						AxisAngle<4, float>(Vector4<float>::Unit(1), -asin(direction[2])));
					Matrix4x4<float> rotation = yawRotation * pitchRotation;

					Vector3<float> forward = HProject(rotation * Vector4<float>::Unit(PITCH));
					Vector3<float> right = HProject(rotation * Vector4<float>::Unit(ROLL));
					Vector3<float> up = HProject(rotation * Vector4<float>::Unit(YAW));

					float r = (Randomizer::Rand() & 0x7fff) / ((float)0x7fff) * (float)GE_C_PI * 2.f;
					float u = sin(r) * (2.f * ((Randomizer::Rand() & 0x7fff) / ((float)0x7fff) - 0.5f)) * 16.f;
					r = cos(r) * (2.f * ((Randomizer::Rand() & 0x7fff) / ((float)0x7fff) - 0.5f)) * 16.f;
					Vector3<float> end = muzzle + forward * 8192.f * 16.f;
					end += right * r;
					end += up * u;

					transform.SetTranslation(pNode->GetPos());
					gamePhysics->SetTransform(mPlayerActor->GetId(), transform);

					Vector3<float> collision, collisionNormal;
					ActorId collisionId = gamePhysics->CastRay(muzzle, end, collision, collisionNormal);
					if (collisionId == mPlayerActor->GetId())
					{
						direction = collision - muzzle;
						Normalize(direction);
						mWeaponDirection[WP_RAILGUN - 1][pNode][pathArc] = direction;
						mWeaponDamage[WP_RAILGUN - 1][pNode][pathArc] = 100;
					}
				}
			}
		}
	}
}

void QuakeAIManager::SimulateLightningFire(PathingNode* pNode)
{
	eastl::shared_ptr<BaseGamePhysic> gamePhysics = GameLogic::Get()->GetGamePhysics();

	Transform transform;
	for (PathingNode* pathNode : mPathingGraph->GetNodes())
	{
		// set aiming directions
		Vector3<float> direction = pathNode->GetPos() - pNode->GetPos();
		if (direction != Vector3<float>::Zero())
		{
			//set muzzle location relative to pivoting eye
			Vector3<float> muzzle = pNode->GetPos();
			muzzle[2] += mPlayerActor->GetState().viewHeight;
			muzzle -= Vector3<float>::Unit(ROLL) * 11.f;

			direction = pathNode->GetPos() - muzzle;
			float scale = Length(direction);
			Normalize(direction);

			Matrix4x4<float> yawRotation = Rotation<4, float>(
				AxisAngle<4, float>(Vector4<float>::Unit(2), atan2(direction[1], direction[0])));
			Matrix4x4<float> pitchRotation = Rotation<4, float>(
				AxisAngle<4, float>(Vector4<float>::Unit(1), -asin(direction[2])));
			Matrix4x4<float> rotation = yawRotation * pitchRotation;

			Vector3<float> forward = HProject(rotation * Vector4<float>::Unit(PITCH));
			Vector3<float> right = HProject(rotation * Vector4<float>::Unit(ROLL));
			Vector3<float> up = HProject(rotation * Vector4<float>::Unit(YAW));

			float r = (Randomizer::Rand() & 0x7fff) / ((float)0x7fff) * (float)GE_C_PI * 2.f;
			float u = sin(r) * (2.f * ((Randomizer::Rand() & 0x7fff) / ((float)0x7fff) - 0.5f)) * 16.f;
			r = cos(r) * (2.f * ((Randomizer::Rand() & 0x7fff) / ((float)0x7fff) - 0.5f)) * 16.f;
			Vector3<float> end = muzzle + forward * (float)LIGHTNING_RANGE;
			end += right * r;
			end += up * u;

			transform.SetTranslation(pNode->GetPos());
			gamePhysics->SetTransform(mPlayerActor->GetId(), transform);

			Vector3<float> collision, collisionNormal;
			ActorId collisionId = gamePhysics->CastRay(muzzle, end, collision, collisionNormal);
			if (collisionId == mPlayerActor->GetId())
			{
				direction = collision - muzzle;
				Normalize(direction);
				mWeaponGroundDirection[WP_LIGHTNING-1][pNode][pathNode] = direction;
				mWeaponGroundDamageTime[WP_LIGHTNING - 1][pNode][pathNode] = 0.f;
				mWeaponGroundDamage[WP_LIGHTNING - 1][pNode][pathNode] = 8;
			}
		}

		// handle jumping and falling nodes
		for (PathingArc* pathArc : pathNode->GetArcs())
		{
			if (pathArc->GetType() == AIAT_JUMP || pathArc->GetType() == AIAT_FALL)
			{
				direction = pathArc->GetConnection() - pNode->GetPos();
				if (direction != Vector3<float>::Zero())
				{
					//set muzzle location relative to pivoting eye
					Vector3<float> muzzle = pNode->GetPos();
					muzzle[2] += mPlayerActor->GetState().viewHeight;
					muzzle -= Vector3<float>::Unit(ROLL) * 11.f;

					direction = pathArc->GetConnection() - muzzle;
					float scale = Length(direction);
					Normalize(direction);

					Matrix4x4<float> yawRotation = Rotation<4, float>(
						AxisAngle<4, float>(Vector4<float>::Unit(2), atan2(direction[1], direction[0])));
					Matrix4x4<float> pitchRotation = Rotation<4, float>(
						AxisAngle<4, float>(Vector4<float>::Unit(1), -asin(direction[2])));
					Matrix4x4<float> rotation = yawRotation * pitchRotation;

					Vector3<float> forward = HProject(rotation * Vector4<float>::Unit(PITCH));
					Vector3<float> right = HProject(rotation * Vector4<float>::Unit(ROLL));
					Vector3<float> up = HProject(rotation * Vector4<float>::Unit(YAW));

					float r = (Randomizer::Rand() & 0x7fff) / ((float)0x7fff) * (float)GE_C_PI * 2.f;
					float u = sin(r) * (2.f * ((Randomizer::Rand() & 0x7fff) / ((float)0x7fff) - 0.5f)) * 16.f;
					r = cos(r) * (2.f * ((Randomizer::Rand() & 0x7fff) / ((float)0x7fff) - 0.5f)) * 16.f;
					Vector3<float> end = muzzle + forward * (float)LIGHTNING_RANGE;
					end += right * r;
					end += up * u;

					transform.SetTranslation(pNode->GetPos());
					gamePhysics->SetTransform(mPlayerActor->GetId(), transform);

					Vector3<float> collision, collisionNormal;
					ActorId collisionId = gamePhysics->CastRay(muzzle, end, collision, collisionNormal);
					if (collisionId == mPlayerActor->GetId())
					{
						direction = collision - muzzle;
						Normalize(direction);
						mWeaponDirection[WP_LIGHTNING - 1][pNode][pathArc] = direction;
						mWeaponDamage[WP_LIGHTNING - 1][pNode][pathArc] = 8;
					}
				}
			}
		}
	}
}


void QuakeAIManager::SimulateWaypoint()
{
	eastl::shared_ptr<BaseGamePhysic> gamePhysics = GameLogic::Get()->GetGamePhysics();

	mActorNodes.clear();
	while (!mOpenSet.empty())
	{
		// grab the candidate
		eastl::map<PathingNode*, bool>::iterator itOpenSet = mOpenSet.begin();
		PathingNode* pNode = itOpenSet->first;
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
		/*
		//check if its on ground
		if (itOpenSet->second)
			SimulateJump(pNode);
		*/
		// we have processed this node so remove it from the closed set
		mClosedSet.erase(itOpenSet);
	}

	//finally we process the item actors which we have met
	eastl::map<PathingNode*, ActorId>::iterator itActorNode;
	for (itActorNode = mActorNodes.begin(); itActorNode != mActorNodes.end(); itActorNode++)
	{
		Vector3<float> position = itActorNode->first->GetPos();
		eastl::shared_ptr<Actor> pItemActor(
			GameLogic::Get()->GetActor(itActorNode->second).lock());
		PathingNode* pClosestNode = mPathingGraph->FindClosestNode(position);
		if (pClosestNode != NULL)
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
	gamePhysics->OnUpdate(0.02f);

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

		totalTime += 0.02f;
		deltaTime += 0.02f;
		fallSpeed += (20.f / (jumpSpeed * 0.5f));
		if (fallSpeed > mMaxFallSpeed) fallSpeed = mMaxFallSpeed;

		PathingNode* pClosestNode =
			mPathingGraph->FindClosestNode(transform.GetTranslation());
		if (pClosestNode != NULL)
		{
			if (pClosestNode != pFallingNode)
				deltaTime = 0.02f;

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
		gamePhysics->OnUpdate(0.02f);

		transform = gamePhysics->GetTransform(mPlayerActor->GetId());
	}
	totalTime += 0.02f;

	Vector3<float> position = transform.GetTranslation();
	PathingNode* pEndNode = mPathingGraph->FindClosestNode(position);
	if (pNode != pEndNode)
	{
		if (pEndNode != NULL && pNode->FindArc(AIAT_TELEPORTTARGET, pEndNode) == NULL)
		{
			Vector3<float> diff = pEndNode->GetPos() - position;
			if (Length(diff) <= 6.f)
			{
				PathingArc* pArc = new PathingArc(AIAT_TELEPORTTARGET, totalTime);
				pArc->LinkNodes(pNode, pEndNode);
				pNode->AddArc(pArc);

				PathingNode* pCurrentNode = pNode;
				eastl::vector<PathingNode*>::iterator itNode;
				for (itNode = nodes.begin(); itNode != nodes.end(); itNode++)
				{
					pFallingNode = (*itNode);
					if (pCurrentNode->FindArc(AIAT_TELEPORT, pEndNode) == NULL)
					{
						PathingArc* pArc = new PathingArc(
							AIAT_TELEPORT, nodeTimes[pFallingNode], nodePositions[pFallingNode]);
						pArc->LinkNodes(pEndNode, pFallingNode);
						pCurrentNode->AddArc(pArc);
					}

					pCurrentNode = pFallingNode;
				}

				if (pCurrentNode != pEndNode)
				{
					if (pCurrentNode->FindArc(AIAT_TELEPORT, pEndNode) == NULL)
					{
						deltaTime = 0.02f;
						PathingArc* pArc = new PathingArc(AIAT_TELEPORT, deltaTime, pEndNode->GetPos());
						pArc->LinkNodes(pEndNode, pEndNode);
						pCurrentNode->AddArc(pArc);
					}
				}
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
	gamePhysics->OnUpdate(0.02f);

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

		totalTime += 0.02f;
		deltaTime += 0.02f;
		fallSpeed += (20.f / (jumpSpeed * 0.5f));
		if (fallSpeed > mMaxFallSpeed) fallSpeed = mMaxFallSpeed;

		PathingNode* pClosestNode =
			mPathingGraph->FindClosestNode(transform.GetTranslation());
		if (pClosestNode != NULL)
		{
			if (pClosestNode != pFallingNode)
				deltaTime = 0.02f;

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
		gamePhysics->OnUpdate(0.02f);

		transform = gamePhysics->GetTransform(mPlayerActor->GetId());
	}
	totalTime += 0.02f;

	//we store the jump if we find a landing node
	Vector3<float> position = transform.GetTranslation();
	PathingNode* pEndNode = mPathingGraph->FindClosestNode(position);
	if (pNode != pEndNode)
	{
		if (pEndNode != NULL && pNode->FindArc(AIAT_PUSHTARGET, pEndNode) == NULL)
		{
			Vector3<float> diff = pEndNode->GetPos() - position;
			if (Length(diff) <= 6.f)
			{
				PathingArc* pArc = new PathingArc(AIAT_PUSHTARGET, totalTime);
				pArc->LinkNodes(pNode, pEndNode);
				pNode->AddArc(pArc);

				PathingNode* pCurrentNode = pNode;
				eastl::vector<PathingNode*>::iterator itNode;
				for (itNode = nodes.begin(); itNode != nodes.end(); itNode++)
				{
					pFallingNode = (*itNode);
					if (pCurrentNode->FindArc(AIAT_PUSH, pEndNode) == NULL)
					{
						PathingArc* pArc = new PathingArc(
							AIAT_PUSH, nodeTimes[pFallingNode], nodePositions[pFallingNode]);
						pArc->LinkNodes(pEndNode, pFallingNode);
						pCurrentNode->AddArc(pArc);
					}

					pCurrentNode = pFallingNode;
				}

				if (pCurrentNode != pEndNode)
				{
					if (pCurrentNode->FindArc(AIAT_PUSH, pEndNode) == NULL)
					{
						deltaTime = 0.02f;
						PathingArc* pArc = new PathingArc(AIAT_PUSH, deltaTime, pEndNode->GetPos());
						pArc->LinkNodes(pEndNode, pEndNode);
						pCurrentNode->AddArc(pArc);
					}
				}
			}
		}
	}
}

float FindClosestMovement(eastl::vector<Vector3<float>>& movements, const Vector3<float>& pos)
{
	float length = FLT_MAX;
	eastl::vector<Vector3<float>>::iterator it;
	for (it = movements.begin(); it != movements.end(); it++)
	{
		Vector3<float> diff = pos - (*it);
		if (Length(diff) < length)
			length = Length(diff);
	}

	return length;
}

void QuakeAIManager::SimulateMovement(PathingNode* pNode)
{
	//lets move the character towards different directions
	eastl::shared_ptr<BaseGamePhysic> gamePhysics = GameLogic::Get()->GetGamePhysics();

	Transform transform;
	transform.SetTranslation(pNode->GetPos());
	gamePhysics->SetTransform(mPlayerActor->GetId(), transform);
	gamePhysics->SetVelocity(mPlayerActor->GetId(), Vector3<float>::Zero());

	// nodes closed to falling position
	eastl::vector<PathingNode*> nodes;
	eastl::map<PathingNode*, float> nodeTimes;
	eastl::map<PathingNode*, Vector3<float>> nodePositions;

	for (int angle = 0; angle < 360; angle += 20)
	{
		Matrix4x4<float> rotation = Rotation<4, float>(
			AxisAngle<4, float>(Vector4<float>::Unit(2), angle * (float)GE_C_DEG_TO_RAD));

		transform.SetTranslation(pNode->GetPos());
		transform.SetRotation(rotation);

		//create movements on the ground
		eastl::vector<Vector3<float>> movements;
		Vector3<float> position = pNode->GetPos();
		PathingNode* pEndNode = NULL;
		PathingNode* pCurrentNode = pNode;
		do
		{
			pEndNode = NULL;

			if (!gamePhysics->OnGround(mPlayerActor->GetId()))
			{
				float deltaTime = 0.f;

				if (!movements.empty())
				{
					eastl::vector<Vector3<float>>::iterator itMove;
					for (itMove = movements.begin(); itMove != movements.end(); itMove++)
					{
						deltaTime += 0.02f;
						PathingNode* pClosestNode = mPathingGraph->FindClosestNode((*itMove));
						if (pCurrentNode != pClosestNode)
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
							else if (Length(diff) <= 6.f)
							{
								if (pCurrentNode->FindArc(AIAT_MOVE, pClosestNode) == NULL)
								{
									PathingArc* pArc = new PathingArc(AIAT_MOVE, deltaTime);
									pArc->LinkNodes(pCurrentNode, pClosestNode);
									pCurrentNode->AddArc(pArc);

									pCurrentNode = pClosestNode;
								}

								deltaTime = 0.f;
							}
						}
					}
				}
				movements.clear();

				nodes.clear();
				nodeTimes.clear();
				nodePositions.clear();

				deltaTime = 0.f;
				float totalTime = 0.f, fallSpeed = 2.f;

				PathingNode* pFallingNode = pCurrentNode;
				do
				{
					float jumpSpeed = gamePhysics->GetJumpSpeed(mPlayerActor->GetId());

					totalTime += 0.02f;
					deltaTime += 0.02f;
					fallSpeed += (20.f / (jumpSpeed * 0.5f));
					if (fallSpeed > mMaxFallSpeed) fallSpeed = mMaxFallSpeed;

					PathingNode* pClosestNode =
						mPathingGraph->FindClosestNode(transform.GetTranslation());
					if (pClosestNode != NULL)
					{
						if (pClosestNode != pFallingNode)
							deltaTime = 0.02f;

						if (eastl::find(nodes.begin(), nodes.end(), pClosestNode) == nodes.end())
							nodes.push_back(pClosestNode);
						nodeTimes[pClosestNode] = deltaTime;
						nodePositions[pClosestNode] = transform.GetTranslation();
						pFallingNode = pClosestNode;
					}

					Vector3<float> direction; // forward vector
#if defined(GE_USE_MAT_VEC)
					direction = HProject(rotation * Vector4<float>::Unit(PITCH));
#else
					direction = HProject(Vector4<float>::Unit(PITCH) * rotation);
#endif
					direction[PITCH] *= jumpSpeed * (fallSpeed / 4.f);
					direction[ROLL] *= jumpSpeed * (fallSpeed / 4.f);
					direction[YAW] = -jumpSpeed * fallSpeed;

					gamePhysics->FallDirection(mPlayerActor->GetId(), direction);
					gamePhysics->OnUpdate(0.02f);

					transform = gamePhysics->GetTransform(mPlayerActor->GetId());
				} while (!gamePhysics->OnGround(mPlayerActor->GetId()));

				//we store the fall if we find a landing node
				Vector3<float> position = transform.GetTranslation();
				pEndNode = mPathingGraph->FindClosestNode(position);
				if (pCurrentNode != pEndNode)
				{
					if (pEndNode != NULL && pCurrentNode->FindArc(AIAT_FALLTARGET, pEndNode) == NULL)
					{
						Vector3<float> diff = pEndNode->GetPos() - position;
						if (Length(diff) >= 16.f || Length(diff) <= 6.f)
						{
							if (Length(diff) >= 16.f)
							{
								pEndNode = new PathingNode(GetNewNodeID(), INVALID_ACTOR_ID, position);
								mPathingGraph->InsertNode(pEndNode);
							}

							PathingArc* pArc = new PathingArc(AIAT_FALLTARGET, totalTime);
							pArc->LinkNodes(pCurrentNode, pEndNode);
							pCurrentNode->AddArc(pArc);

							eastl::vector<PathingNode*>::iterator itNode;
							for (itNode = nodes.begin(); itNode != nodes.end(); itNode++)
							{
								pFallingNode = (*itNode);
								if (pCurrentNode->FindArc(AIAT_FALL, pEndNode) == NULL)
								{
									PathingArc* pArc = new PathingArc(
										AIAT_FALL, nodeTimes[pFallingNode], nodePositions[pFallingNode]);
									pArc->LinkNodes(pEndNode, pFallingNode);
									pCurrentNode->AddArc(pArc);
								}

								pCurrentNode = pFallingNode;
							}

							if (pCurrentNode != pEndNode)
							{
								if (pCurrentNode->FindArc(AIAT_FALL, pEndNode) == NULL)
								{
									deltaTime = 0.02f;
									PathingArc* pArc = new PathingArc(AIAT_FALL, deltaTime);
									pArc->LinkNodes(pEndNode, pEndNode);
									pCurrentNode->AddArc(pArc);
								}
								pCurrentNode = pEndNode;
							}
						}
						else break;
					}
					else break;
				}
			}
			else
			{
				PathingNode* pClosestNode = mPathingGraph->FindClosestNode(position);
				if (pClosestNode != pCurrentNode)
				{
					// if we find a link to the closest node then we stop
					Vector3<float> diff = pClosestNode->GetPos() - position;
					if (Length(diff) <= 6.f)
					{
						if (pCurrentNode->FindArc(AIAT_MOVE, pClosestNode) != NULL)
						{
							pEndNode = pClosestNode;
							break;
						}
					}
				}
				movements.push_back(position);

				Vector3<float> direction; // forward vector
#if defined(GE_USE_MAT_VEC)
				direction = HProject(rotation * Vector4<float>::Unit(PITCH));
#else
				direction = HProject(Vector4<float>::Unit(PITCH) * rotation);
#endif

				gamePhysics->SetTransform(mPlayerActor->GetId(), transform);
				gamePhysics->WalkDirection(mPlayerActor->GetId(), direction * mMoveSpeed);
				gamePhysics->OnUpdate(0.02f);
			}

			transform = gamePhysics->GetTransform(mPlayerActor->GetId());
			position = transform.GetTranslation();

		} while (FindClosestMovement(movements, position) > 2.f); // stalling is a break condition

		if (!movements.empty())
		{
			float deltaTime = 0.f;
			eastl::vector<Vector3<float>>::iterator itMove;
			for (itMove = movements.begin(); itMove != movements.end(); itMove++)
			{
				deltaTime += 0.02f;
				PathingNode* pClosestNode = mPathingGraph->FindClosestNode((*itMove));
				if (pCurrentNode != pClosestNode)
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
					else if (Length(diff) <= 6.f)
					{
						if (pCurrentNode->FindArc(AIAT_MOVE, pClosestNode) == NULL)
						{
							PathingArc* pArc = new PathingArc(AIAT_MOVE, deltaTime);
							pArc->LinkNodes(pCurrentNode, pClosestNode);
							pCurrentNode->AddArc(pArc);

							pCurrentNode = pClosestNode;
						}
						else break;
					}
				}
			}
			deltaTime += 0.02f;

			if (pEndNode != NULL && pCurrentNode != pEndNode)
			{
				if (pCurrentNode->FindArc(AIAT_MOVE, pEndNode) == NULL)
				{
					PathingArc* pArc = new PathingArc(AIAT_MOVE, deltaTime);
					pArc->LinkNodes(pCurrentNode, pEndNode);
					pCurrentNode->AddArc(pArc);
				}
			}
		}
	}
}

void QuakeAIManager::SimulateJump(PathingNode* pNode)
{
	//lets move the character towards different directions
	eastl::shared_ptr<BaseGamePhysic> gamePhysics = GameLogic::Get()->GetGamePhysics();

	Transform transform;
	Vector3<float> direction;
	for (int angle = 0; angle < 360; angle += 20)
	{
		Matrix4x4<float> rotation = Rotation<4, float>(
			AxisAngle<4, float>(Vector4<float>::Unit(2), angle * (float)GE_C_DEG_TO_RAD));

		// forward vector
#if defined(GE_USE_MAT_VEC)
		direction = HProject(rotation * Vector4<float>::Unit(PITCH));
#else
		direction = HProject(Vector4<float>::Unit(PITCH) * rotation);
#endif
		direction[PITCH] *= mJumpMoveSpeed;
		direction[ROLL] *= mJumpMoveSpeed;
		direction[YAW] = mJumpSpeed;

		transform.SetTranslation(pNode->GetPos());
		transform.SetRotation(rotation);
		gamePhysics->SetTransform(mPlayerActor->GetId(), transform);
		gamePhysics->WalkDirection(mPlayerActor->GetId(), direction);
		gamePhysics->Jump(mPlayerActor->GetId(), direction);
		gamePhysics->OnUpdate(0.02f);

		transform = gamePhysics->GetTransform(mPlayerActor->GetId());

		// nodes closed to jump position
		eastl::vector<PathingNode*> nodes;
		eastl::map<PathingNode*, float> nodeTimes;
		eastl::map<PathingNode*, Vector3<float>> nodePositions;

		float fallSpeed = 0.f, deltaTime = 0.f, totalTime = 0.f;
		PathingNode* pFallingNode = pNode;

		// gravity falling simulation
		while (!gamePhysics->OnGround(mPlayerActor->GetId()))
		{
			float jumpSpeed = gamePhysics->GetJumpSpeed(mPlayerActor->GetId());

			totalTime += 0.02f;
			deltaTime += 0.02f;
			fallSpeed += (20.f / (jumpSpeed * 0.5f));
			if (fallSpeed > mMaxFallSpeed) fallSpeed = mMaxFallSpeed;

			PathingNode* pClosestNode =
				mPathingGraph->FindClosestNode(transform.GetTranslation());
			if (pClosestNode != NULL)
			{
				if (pClosestNode != pFallingNode)
					deltaTime = 0.02f;

				if (eastl::find(nodes.begin(), nodes.end(), pClosestNode) == nodes.end())
					nodes.push_back(pClosestNode);
				nodeTimes[pClosestNode] = deltaTime;
				nodePositions[pClosestNode] = transform.GetTranslation();
				pFallingNode = pClosestNode;
			}

			// forward vector
#if defined(GE_USE_MAT_VEC)
			direction = HProject(rotation * Vector4<float>::Unit(PITCH));
#else
			direction = HProject(Vector4<float>::Unit(PITCH) * rotation);
#endif
			direction[PITCH] *= jumpSpeed * (fallSpeed / 4.f);
			direction[ROLL] *= jumpSpeed * (fallSpeed / 4.f);
			direction[YAW] = -jumpSpeed * fallSpeed;

			gamePhysics->FallDirection(mPlayerActor->GetId(), direction);
			gamePhysics->OnUpdate(0.02f);

			transform = gamePhysics->GetTransform(mPlayerActor->GetId());
		}
		totalTime += 0.02f;

		//we store the jump if we find a landing node
		Vector3<float> position = transform.GetTranslation();
		PathingNode* pEndNode = mPathingGraph->FindClosestNode(position);
		if (pNode != pEndNode)
		{
			if (pEndNode != NULL && pNode->FindArc(AIAT_JUMPTARGET, pEndNode) == NULL)
			{
				Vector3<float> diff = pEndNode->GetPos() - position;
				if (Length(diff) <= 6.f)
				{
					PathingArc* pArc = new PathingArc(AIAT_JUMPTARGET, totalTime);
					pArc->LinkNodes(pNode, pEndNode);
					pNode->AddArc(pArc);

					PathingNode* pCurrentNode = pNode;
					eastl::vector<PathingNode*>::iterator itNode;
					for (itNode = nodes.begin(); itNode != nodes.end(); itNode++)
					{
						pFallingNode = (*itNode);
						if (pCurrentNode->FindArc(AIAT_JUMP, pEndNode) == NULL)
						{
							PathingArc* pArc = new PathingArc(
								AIAT_JUMP, nodeTimes[pFallingNode], nodePositions[pFallingNode]);
							pArc->LinkNodes(pEndNode, pFallingNode);
							pCurrentNode->AddArc(pArc);
						}

						pCurrentNode = pFallingNode;
					}

					if (pCurrentNode != pEndNode)
					{
						if (pCurrentNode->FindArc(AIAT_JUMP, pEndNode) == NULL)
						{
							deltaTime = 0.02f;
							PathingArc* pArc = new PathingArc(AIAT_JUMP, deltaTime, pEndNode->GetPos());
							pArc->LinkNodes(pEndNode, pEndNode);
							pCurrentNode->AddArc(pArc);
						}
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
			if (pItemActor->GetType() == "Weapon" ||
				pItemActor->GetType() == "Ammo" ||
				pItemActor->GetType() == "Armor" ||
				pItemActor->GetType() == "Health")
			{
				Vector3<float> position = pPhysicComponent->GetTransform().GetTranslation();
				PathingNode* pClosestNode = mPathingGraph->FindClosestNode(position);
				if (pClosestNode != NULL)
				{
					Vector3<float> diff = pClosestNode->GetPos() - position;
					if (Length(diff) <= 6.f)
						mActorNodes[pClosestNode] = pItemActor->GetId();
				}
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
		else
		{
			if (pGameActorA->GetType() == "Fire")
			{
				mActorCollisions[pGameActorA->GetId()] = true;
			}
			else if (pGameActorB->GetType() == "Fire")
			{
				mActorCollisions[pGameActorB->GetId()] = true;
			}
			return;
		}

		if (mPlayerActor->GetId() == pPlayerActor->GetId())
		{
			eastl::shared_ptr<PhysicComponent> pPhysicComponent(
				mPlayerActor->GetComponent<PhysicComponent>(PhysicComponent::Name).lock());
			if (pPhysicComponent->OnGround())
			{
				if (pItemActor->GetType() == "Trigger")
				{
					if (pItemActor->GetComponent<PushTrigger>(PushTrigger::Name).lock() ||
						pItemActor->GetComponent<TeleporterTrigger>(TeleporterTrigger::Name).lock())
					{
						Vector3<float> position = pPhysicComponent->GetTransform().GetTranslation();
						PathingNode* pClosestNode = mPathingGraph->FindClosestNode(position);
						if (pClosestNode != NULL)
						{
							Vector3<float> diff = pClosestNode->GetPos() - position;
							if (Length(diff) <= 6.f)
								mActorNodes[pClosestNode] = pItemActor->GetId();
						}
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