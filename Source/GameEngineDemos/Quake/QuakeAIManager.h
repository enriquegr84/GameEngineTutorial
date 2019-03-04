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

#ifndef QUAKEAIMANAGER_H
#define QUAKEAIMANAGER_H

#include "QuakeStd.h"

#include "AI/AIManager.h"

#include "Actors/PlayerActor.h"
#include "Core/Event/EventManager.h"

#include "Physic/PhysicEventListener.h"
#include "Mathematic/Algebra/Matrix4x4.h"

enum AIActionType
{
	AIAT_MOVE = 0x0000000,
	AIAT_PUSH = 0x0000001,
	AIAT_PUSHTARGET = 0x0000002,
	AIAT_TELEPORT = 0x0000005,
	AIAT_TELEPORTTARGET = 0x0000006,
	AIAT_FALL = 0x0000009,
	AIAT_FALLTARGET = 0x000000A,
	AIAT_JUMP = 0x000000D,
	AIAT_JUMPTARGET = 0x00000E
};

class QuakeAIManager : public AIManager
{

public:
	QuakeAIManager();
	~QuakeAIManager();

	virtual void CreateWaypoints(ActorId playerId);

	virtual void SavePathingGraph(const eastl::string& path);
	virtual void LoadPathingGraph(const eastl::wstring& path);

protected:

	void SimulateJump(PathingNode* pNode);
	void SimulateMovement(PathingNode* pNode);
	void SimulateTriggerPush(PathingNode* pNode, const Vector3<float>& target);
	void SimulateTriggerTeleport(PathingNode* pNode, const Vector3<float>& target);

	void SimulateWaypoint();

	void SimulateGauntletAttack(PathingNode* pNode);
	void SimulateBulletFire(PathingNode* pNode, float spread, int damage);
	void SimulateShotgunFire(PathingNode* pNode);
	void SimulateRailgunFire(PathingNode* pNode);
	void SimulateLightningFire(PathingNode* pNode);
	void SimulateGrenadeLauncherFire(PathingNode* pNode, eastl::shared_ptr<Actor> pGameActor);
	void SimulateRocketLauncherFire(PathingNode* pNode, eastl::shared_ptr<Actor> pGameActor);
	void SimulatePlasmagunFire(PathingNode* pNode, eastl::shared_ptr<Actor> pGameActor);

	void SimulateFiring();

	unsigned int GetNewNodeID(void)
	{
		return ++mLastNodeId;
	}

	// event delegates
	void PhysicsTriggerEnterDelegate(BaseEventDataPtr pEventData);
	void PhysicsTriggerLeaveDelegate(BaseEventDataPtr pEventData);
	void PhysicsCollisionDelegate(BaseEventDataPtr pEventData);
	void PhysicsSeparationDelegate(BaseEventDataPtr pEventData);

	eastl::shared_ptr<PlayerActor> mPlayerActor;

	// Orientation Controls
	float		mYaw;
	float		mPitch;
	float		mPitchTarget;
	float		mPitchOnDown;
	float		mYawOnDown;

	// Speed Controls
	float		mMaxFallSpeed;
	float		mMaxJumpSpeed;
	float		mMaxRotateSpeed;
	float		mFallSpeed;
	float		mJumpSpeed;
	float		mJumpMoveSpeed;
	float		mMoveSpeed;
	float		mRotateSpeed;


private:

	unsigned int mLastNodeId;

	//open set of nodes to be analized and also to
	//inform whether they are on ground or not
	eastl::map<PathingNode*, bool> mOpenSet, mClosedSet;

	//pathing nodes which contains actors from game
	eastl::map<ActorId, bool> mActorCollisions;
	eastl::map<PathingNode*, ActorId> mActorNodes;
	PathingNodeDoubleMap mWeaponGroundDamage[MAX_WEAPONS];
	PathingNodeDoubleMap mWeaponGroundDamageTime[MAX_WEAPONS];
	PathingNodeDirectionMap mWeaponGroundDirection[MAX_WEAPONS];
	PathingNodeArcDoubleMap mWeaponDamage[MAX_WEAPONS];
	PathingNodeArcDirectionMap mWeaponDirection[MAX_WEAPONS];

	void RegisterAllDelegates(void);
	void RemoveAllDelegates(void);

};   // QuakeAIManager

#endif
