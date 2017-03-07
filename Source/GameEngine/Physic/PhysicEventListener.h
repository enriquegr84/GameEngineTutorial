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

#include "Events/EventManager.h"
#include "GameEngine/GameEngine.h"
#include "Scripting/ScriptEvent.h"

//
// Physics event implementation 
//

class EvtData_PhysTrigger_Enter : public BaseEventData
{
	int m_triggerID;
    ActorId m_other;

public:
	static const EventType sk_EventType;

	virtual const EventType & GetEventType( void ) const
	{
		return sk_EventType;
	}

	EvtData_PhysTrigger_Enter()
	{
		m_triggerID = -1;
		m_other = INVALID_ACTOR_ID;
	}

	explicit EvtData_PhysTrigger_Enter( int triggerID, ActorId other )
		: m_triggerID(triggerID),
		  m_other(other)
	{}

	BaseEventDataPtr Copy() const 
	{
		return BaseEventDataPtr(new EvtData_PhysTrigger_Enter(m_triggerID, m_other));
	}

    virtual const char* GetName(void) const
    {
        return "EvtData_PhysTrigger_Enter";
    }

    int GetTriggerId(void) const
    {
        return m_triggerID;
    }

    ActorId GetOtherActor(void) const
    {
        return m_other;
    }
};

class EvtData_PhysTrigger_Leave : public BaseEventData
{
	int m_triggerID;
    ActorId m_other;

public:
	static const EventType sk_EventType;

	virtual const EventType & GetEventType( void ) const
	{
		return sk_EventType;
	}

	EvtData_PhysTrigger_Leave()
	{
		m_triggerID = -1;
		m_other = INVALID_ACTOR_ID;
	}

	explicit EvtData_PhysTrigger_Leave( int triggerID, ActorId other )
		: m_triggerID(triggerID),
		  m_other(other)
	{}

	virtual BaseEventDataPtr Copy() const
	{
		return BaseEventDataPtr ( new EvtData_PhysTrigger_Leave(m_triggerID, m_other) );
	}

    virtual const char* GetName(void) const
    {
        return "EvtData_PhysTrigger_Leave";
    }

    int GetTriggerId(void) const
    {
        return m_triggerID;
    }
    
    ActorId GetOtherActor(void) const
    {
        return m_other;
    }
};

class EvtData_PhysCollision : public BaseEventData
{
	ActorId m_ActorA;
    ActorId m_ActorB;
	Vector3f m_SumNormalForce;
    Vector3f m_SumFrictionForce;
    eastl::list<Vector3f> m_CollisionPoints;

public:
	static const EventType sk_EventType;

	virtual const EventType & GetEventType( void ) const
	{
		return sk_EventType;
	}

	EvtData_PhysCollision()
	{
		m_ActorA = INVALID_ACTOR_ID;
		m_ActorB = INVALID_ACTOR_ID;
		m_SumNormalForce = Vector3f(0.0f, 0.0f, 0.0f);
		m_SumFrictionForce = Vector3f(0.0f, 0.0f, 0.0f);
	}

	explicit EvtData_PhysCollision(ActorId actorA,
								ActorId actorB,
								Vector3f sumNormalForce,
								Vector3f sumFrictionForce,
								eastl::list<Vector3f> collisionPoints )
	:	m_ActorA(actorA),
		m_ActorB(actorB),
		m_SumNormalForce(sumNormalForce),
		m_SumFrictionForce(sumFrictionForce),
		m_CollisionPoints(collisionPoints)
	{}

	virtual BaseEventDataPtr Copy() const
	{
		return BaseEventDataPtr ( new EvtData_PhysCollision(m_ActorA, m_ActorB, m_SumNormalForce, m_SumFrictionForce, m_CollisionPoints));
	}

    virtual const char* GetName(void) const
    {
        return "EvtData_PhysCollision";
    }

    ActorId GetActorA(void) const
    {
        return m_ActorA;
    }

    ActorId GetActorB(void) const
    {
        return m_ActorB;
    }

    const Vector3f& GetSumNormalForce(void) const
    {
        return m_SumNormalForce;
    }

    const Vector3f& GetSumFrictionForce(void) const
    {
        return m_SumFrictionForce;
    }

    const eastl::list<Vector3f>& GetCollisionPoints(void) const
    {
        return m_CollisionPoints;
    }

    virtual void BuildEventData(void);

    EXPORT_FOR_SCRIPT_EVENT(EvtData_PhysCollision);
};


class EvtData_PhysSeparation : public BaseEventData
{
	ActorId m_ActorA;
    ActorId m_ActorB;

public:
	static const EventType sk_EventType;

	virtual const EventType & GetEventType( void ) const
	{
		return sk_EventType;
	}

	EvtData_PhysSeparation()
	{

		m_ActorA = INVALID_ACTOR_ID;
		m_ActorB = INVALID_ACTOR_ID;
	}

	explicit EvtData_PhysSeparation(ActorId actorA, ActorId actorB)
		: m_ActorA(actorA)
		, m_ActorB(actorB)
	{}

	virtual BaseEventDataPtr Copy() const
	{
		return BaseEventDataPtr ( new EvtData_PhysSeparation(m_ActorA, m_ActorB) );
	}

    virtual const char* GetName(void) const
    {
        return "EvtData_PhysSeparation";
    }

    ActorId GetActorA(void) const
    {
        return m_ActorA;
    }

    ActorId GetActorB(void) const
    {
        return m_ActorB;
    }
};


#endif