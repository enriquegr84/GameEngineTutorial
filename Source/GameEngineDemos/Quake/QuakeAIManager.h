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
	AT_MOVE,
	AT_JUMP,
	AT_FALL,
	AT_PUSH,
	AT_TELEPORT,

	AT_COUNT
};

class QuakeAIManager : public AIManager
{

public:
	QuakeAIManager();
	~QuakeAIManager();

	virtual void CreateWaypoints(ActorId playerId);

protected:

	void SimulateJump(PathingNode* pNode);
	void SimulateMovement(PathingNode* pNode);
	void SimulateFall(PathingNode* pNode);

	void SimulateWaypoints();

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
	//we simulate doing the same rotations, so we store them
	//in order to not recalculate it every time
	eastl::map<int, Vector3<float>> mAngleDirection;
	eastl::map<int, Matrix4x4<float>> mRotationDirection;

	//open set of nodes to be analized and also to
	//inform whether they are on ground or not
	eastl::map<PathingNode*, bool> mOpenSet, mClosedSet;

	eastl::shared_ptr<PathingGraph> mPathingGraph;

	void RegisterAllDelegates(void);
	void RemoveAllDelegates(void);

};   // QuakeAIManager

#endif
