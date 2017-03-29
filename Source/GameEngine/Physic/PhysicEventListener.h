//========================================================================
// PhysicEventListener.cpp : implements the events sent FROM the phsyics system
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

#ifndef PHYSICEVENTLISTENER_H
#define PHYSICEVENTLISTENER_H

#include "GameEngineStd.h"

#include "Core/Event/EventManager.h"

#include "Mathematic/Algebra/Vector3.h"

//
// Physics event implementation 
//

class EventDataPhysTriggerEnter : public EventData
{
	int mTriggerID;
    ActorId mOther;

public:
	static const BaseEventType skEventType;

	virtual const BaseEventType & GetEventType( void ) const
	{
		return skEventType;
	}

	EventDataPhysTriggerEnter()
	{
		mTriggerID = -1;
		mOther = INVALID_ACTOR_ID;
	}

	explicit EventDataPhysTriggerEnter( int triggerID, ActorId other )
		: mTriggerID(triggerID), mOther(other)
	{}

	BaseEventDataPtr Copy() const 
	{
		return BaseEventDataPtr(new EventDataPhysTriggerEnter(mTriggerID, mOther));
	}

    virtual const char* GetName(void) const
    {
        return "EventDataPhysTriggerEnter";
    }

    int GetTriggerId(void) const
    {
        return mTriggerID;
    }

    ActorId GetOtherActor(void) const
    {
        return mOther;
    }
};

class EventDataPhysTriggerLeave : public EventData
{
	int mTriggerID;
    ActorId mOther;

public:
	static const BaseEventType skEventType;

	virtual const BaseEventType & GetEventType( void ) const
	{
		return skEventType;
	}

	EventDataPhysTriggerLeave()
	{
		mTriggerID = -1;
		mOther = INVALID_ACTOR_ID;
	}

	explicit EventDataPhysTriggerLeave( int triggerID, ActorId other )
		: mTriggerID(triggerID),
		  mOther(other)
	{}

	virtual BaseEventDataPtr Copy() const
	{
		return BaseEventDataPtr ( new EventDataPhysTriggerLeave(mTriggerID, mOther) );
	}

    virtual const char* GetName(void) const
    {
        return "EventDataPhysTriggerLeave";
    }

    int GetTriggerId(void) const
    {
        return mTriggerID;
    }
    
    ActorId GetOtherActor(void) const
    {
        return mOther;
    }
};

class EventDataPhysCollision : public EventData
{
	ActorId mActorA;
    ActorId mActorB;
	Vector3<float> mSumNormalForce;
    Vector3<float> mSumFrictionForce;
    eastl::list<Vector3<float>> mCollisionPoints;

public:
	static const BaseEventType skEventType;

	virtual const BaseEventType & GetEventType( void ) const
	{
		return skEventType;
	}

	EventDataPhysCollision()
	{
		mActorA = INVALID_ACTOR_ID;
		mActorB = INVALID_ACTOR_ID;
		mSumNormalForce.MakeZero();
		mSumFrictionForce.MakeZero();
	}

	explicit EventDataPhysCollision(ActorId actorA, ActorId actorB,
		Vector3<float> sumNormalForce, Vector3<float> sumFrictionForce, eastl::list<Vector3<float>> collisionPoints )
	:	mActorA(actorA),
		mActorB(actorB),
		mSumNormalForce(sumNormalForce),
		mSumFrictionForce(sumFrictionForce),
		mCollisionPoints(collisionPoints)
	{}

	virtual BaseEventDataPtr Copy() const
	{
		return BaseEventDataPtr ( new EventDataPhysCollision(
			mActorA, mActorB, mSumNormalForce, mSumFrictionForce, mCollisionPoints));
	}

    virtual const char* GetName(void) const
    {
        return "EventDataPhysCollision";
    }

    ActorId GetActorA(void) const
    {
        return mActorA;
    }

    ActorId GetActorB(void) const
    {
        return mActorB;
    }

    const Vector3<float>& GetSumNormalForce(void) const
    {
        return mSumNormalForce;
    }

    const Vector3<float>& GetSumFrictionForce(void) const
    {
        return mSumFrictionForce;
    }

    const eastl::list<Vector3<float>>& GetCollisionPoints(void) const
    {
        return mCollisionPoints;
    }
};


class EventDataPhysSeparation : public EventData
{
	ActorId mActorA;
    ActorId mActorB;

public:
	static const BaseEventType skEventType;

	virtual const BaseEventType & GetEventType( void ) const
	{
		return skEventType;
	}

	EventDataPhysSeparation()
	{

		mActorA = INVALID_ACTOR_ID;
		mActorB = INVALID_ACTOR_ID;
	}

	explicit EventDataPhysSeparation(ActorId actorA, ActorId actorB)
		: mActorA(actorA)
		, mActorB(actorB)
	{}

	virtual BaseEventDataPtr Copy() const
	{
		return BaseEventDataPtr ( new EventDataPhysSeparation(mActorA, mActorB) );
	}

    virtual const char* GetName(void) const
    {
        return "EventDataPhysSeparation";
    }

    ActorId GetActorA(void) const
    {
        return mActorA;
    }

    ActorId GetActorB(void) const
    {
        return mActorB;
    }
};


#endif