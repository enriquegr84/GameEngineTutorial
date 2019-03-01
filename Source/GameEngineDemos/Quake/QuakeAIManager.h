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
#include "AI/Pathing.h"

#include "Actors/PlayerActor.h"
#include "Core/Event/EventManager.h"

#include "Physic/PhysicEventListener.h"
#include "Mathematic/Algebra/Matrix4x4.h"

enum AIActionType
{
	AIAT_MOVE,
	AIAT_PUSH,
	AIAT_PUSHTARGET,
	AIAT_TELEPORT,
	AIAT_TELEPORTTARGET,
	AIAT_FALL,
	AIAT_FALLTARGET,
	AIAT_JUMP,
	AIAT_JUMPTARGET,

	AIAT_COUNT
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
	void SimulateFall(PathingNode* pNode);
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

	unsigned int GetNewArcID(void)
	{
		return ++mLastArcId;
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

	unsigned int mLastArcId;
	unsigned int mLastNodeId;

	//open set of nodes to be analized and also to
	//inform whether they are on ground or not
	eastl::map<PathingNode*, bool> mOpenSet, mClosedSet;

	//pathing nodes which contains actors from game
	eastl::map<ActorId, bool> mActorCollisions;
	eastl::map<Vector3<float>, ActorId> mActorNodes;
	PathingNodeDoubleMap mWeaponGroundDamage[MAX_WEAPONS];
	PathingNodeDoubleMap mWeaponGroundDamageTime[MAX_WEAPONS];
	PathingNodeDirectionMap mWeaponGroundDirection[MAX_WEAPONS];
	PathingNodeArcDoubleMap mWeaponDamage[MAX_WEAPONS];
	PathingNodeArcDirectionMap mWeaponDirection[MAX_WEAPONS];

	eastl::shared_ptr<PathingGraph> mPathingGraph;

	void RegisterAllDelegates(void);
	void RemoveAllDelegates(void);

};   // QuakeAIManager

#endif
