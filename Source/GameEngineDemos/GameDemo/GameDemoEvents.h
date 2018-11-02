//========================================================================
// DemosEvents.h : defines game-specific events for Demos
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


#ifndef GAMEDEMOEVENTS_H
#define GAMEDEMOEVENTS_H

#include "Core/Event/EventManager.h"
#include "Core/Event/Event.h"


//---------------------------------------------------------------------------------------------------------------------
// class EventDataFireWeapon
//---------------------------------------------------------------------------------------------------------------------
class EventDataFireWeapon : public EventData
{
    ActorId mId;

public:
    static const BaseEventType skEventType;
    virtual const BaseEventType & GetEventType() const
    {
        return skEventType;
    }

    EventDataFireWeapon(void)
    {
        mId = INVALID_ACTOR_ID;
    }

    EventDataFireWeapon( ActorId id )
	: mId( id )
    {
    }

    virtual BaseEventDataPtr Copy() const
    {
        return BaseEventDataPtr(new EventDataFireWeapon(mId));
    }

    virtual void Serialize( std::ostrstream & out ) const
    {
        out << mId << " ";
    }

    virtual void Deserialize( std::istrstream & in )
    {
        in >> mId;
    }

    virtual const char* GetName(void) const
    {
        return "EventDataFireWeapon";
    }

	ActorId GetActorId(void) const
    {
        return mId;
    }
};

//---------------------------------------------------------------------------------------------------------------------
// EventDataMoveActor - sent when actors are moved
//---------------------------------------------------------------------------------------------------------------------
class EventDataMoveActor : public EventData
{
	ActorId mId;
	Transform mTransform;

public:
	static const BaseEventType skEventType;

	virtual const BaseEventType& GetEventType(void) const
	{
		return skEventType;
	}

	EventDataMoveActor(void)
	{
		mId = INVALID_ACTOR_ID;
	}

	EventDataMoveActor(ActorId id, const Transform& trans)
		: mId(id), mTransform(trans)
	{
		//
	}

	virtual void Serialize(std::ostrstream &out) const
	{
		out << mId << " ";
		for (int i = 0; i<4; ++i)
			for (int j = 0; j<4; ++j)
				out << mTransform.GetMatrix()(i, j) << " ";
	}

	virtual void Deserialize(std::istrstream& in)
	{
		in >> mId;

		Matrix4x4<float> transform = mTransform.GetMatrix();
		for (int i = 0; i<4; ++i)
			for (int j = 0; j<4; ++j)
				in >> transform(i, j);
	}

	virtual BaseEventDataPtr Copy() const
	{
		return BaseEventDataPtr(new EventDataMoveActor(mId, mTransform));
	}

	virtual const char* GetName(void) const
	{
		return "EventDataMoveActor";
	}

	ActorId GetId(void) const
	{
		return mId;
	}

	const Transform& GetTransform(void) const
	{
		return mTransform;
	}
};

//---------------------------------------------------------------------------------------------------------------------
// class EventDataStartThrust				        - Chapter 10, 279
//---------------------------------------------------------------------------------------------------------------------
class EventDataStartThrust : public EventData
{
    ActorId mId;
    float mAcceleration;

public:
    static const BaseEventType skEventType;
    virtual const BaseEventType& GetEventType( void ) const
    {
        return skEventType;
    }

    EventDataStartThrust(void) 
	: mId(INVALID_ACTOR_ID), mAcceleration(0)
    {
        //
    }

    EventDataStartThrust(ActorId id, float acceleration) 
	: mId(id), mAcceleration(acceleration)
    {
        //
    }

    virtual BaseEventDataPtr Copy() const
    {
        return BaseEventDataPtr(new EventDataStartThrust(mId, mAcceleration));
    }

    virtual void Serialize(std::ostrstream & out) const
    {
        out << mId << " ";
        out << mAcceleration << " ";
    }

    virtual void Deserialize(std::istrstream& in)
    {
        in >> mId;
        in >> mAcceleration;
    }

    virtual const char* GetName(void) const
    {
        return "EventDataStartThrust";
    }

    ActorId GetActorId(void) const
    {
        return mId;
    }

    float GetAcceleration(void) const
    {
        return mAcceleration;
    }

    void Set(ActorId id)
    {
        mId = id;
    }
};


//---------------------------------------------------------------------------------------------------------------------
// class EventDataEndThrust				        - Chapter 10, 279
//---------------------------------------------------------------------------------------------------------------------
class EventDataEndThrust : public EventData
{
    ActorId mId;

public:
    static const BaseEventType skEventType;
    virtual const BaseEventType & GetEventType( void ) const
    {
        return skEventType;
    }

    EventDataEndThrust(void) :
        mId(INVALID_ACTOR_ID)
    {
        //
    }

    EventDataEndThrust(ActorId id) :
        mId(id)
    {
        //
    }

    virtual BaseEventDataPtr Copy() const
    {
        return BaseEventDataPtr(new EventDataEndThrust(mId));
    }

    virtual void Serialize(std::ostrstream & out) const
    {
        out << mId << " ";
    }

    virtual void Deserialize(std::istrstream& in)
    {
        in >> mId;
    }

    virtual const char* GetName(void) const
    {
        return "EventDataEndThrust";
    }

    ActorId GetActorId(void) const
    {
        return mId;
    }

    void Set(ActorId id)
    {
        mId = id;
    }
};


//---------------------------------------------------------------------------------------------------------------------
// class EventDataStartSteer				             - Chapter 10, 279
//---------------------------------------------------------------------------------------------------------------------
class EventDataStartSteer : public EventData
{
    ActorId mId;
    float mAcceleration;

public:
    static const BaseEventType skEventType;
    virtual const BaseEventType & GetEventType( void ) const
    {
        return skEventType;
    }

    EventDataStartSteer(void) 
	: mId(INVALID_ACTOR_ID), mAcceleration(0)
    {
        //
    }

    EventDataStartSteer(ActorId id, float acceleration) 
	: mId(id), mAcceleration(acceleration)
    {
        //
    }

    virtual BaseEventDataPtr Copy() const
    {
        return BaseEventDataPtr(new EventDataStartSteer(mId, mAcceleration));
    }

    virtual void Serialize(std::ostrstream & out) const
    {
        out << mId << " ";
        out << mAcceleration << " ";
    }

    virtual void Deserialize(std::istrstream& in)
    {
        in >> mId;
        in >> mAcceleration;
    }

    virtual const char* GetName(void) const
    {
        return "EventDataStartSteer";
    }

    ActorId GetActorId(void) const
    {
        return mId;
    }

    float GetAcceleration(void) const
    {
        return mAcceleration;
    }

    void Set(ActorId id)
    {
        mId = id;
    }
};


//---------------------------------------------------------------------------------------------------------------------
// class EventDataEndSteer				        - Chapter 10, 279
//---------------------------------------------------------------------------------------------------------------------
class EventDataEndSteer : public EventData
{
    ActorId mId;

public:
    static const BaseEventType skEventType;
    virtual const BaseEventType & GetEventType( void ) const
    {
        return skEventType;
    }

    EventDataEndSteer(void) :
        mId(INVALID_ACTOR_ID)
    {
        //
    }

    EventDataEndSteer(ActorId id) :
        mId(id)
    {
        //
    }

    virtual BaseEventDataPtr Copy() const
    {
        return BaseEventDataPtr(new EventDataEndSteer(mId));
    }

    virtual void Serialize(std::ostrstream & out) const
    {
        out << mId << " ";
    }

    virtual void Deserialize(std::istrstream& in)
    {
        in >> mId;
    }

    virtual const char* GetName(void) const
    {
        return "EventDataEndSteer";
    }

    ActorId GetActorId(void) const
    {
        return mId;
    }

    void Set(ActorId id)
    {
        mId = id;
    }
};


//---------------------------------------------------------------------------------------------------------------------
// class EventDataGameplayUIUpdate				- Chapter 10, 279
//---------------------------------------------------------------------------------------------------------------------
class EventDataGameplayUIUpdate : public EventData
{
    eastl::string mGameplayUiString;

public:
    static const BaseEventType skEventType;

    EventDataGameplayUIUpdate(void) { }
    EventDataGameplayUIUpdate(const eastl::string& gameplayUiString)
	: mGameplayUiString(gameplayUiString)
    {
    }

    virtual const BaseEventType& GetEventType(void) const
    {
        return skEventType;
    }

    virtual BaseEventDataPtr Copy() const
    {
        return BaseEventDataPtr(new EventDataGameplayUIUpdate(mGameplayUiString));
    }

    virtual void Serialize(std::ostrstream& out) const
    {
		out << mGameplayUiString.c_str();
    }

    virtual void Deserialize(std::istrstream& in)
    {
		in >> std::string(mGameplayUiString.c_str());
    }

    const eastl::string& GetUiString(void) const
    {
        return mGameplayUiString;
    }

    virtual const char* GetName(void) const
    {
        return "EventDataGameplayUIUpdate";
    }
};


//---------------------------------------------------------------------------------------------------------------------
// class EventDataSetControlledActor				- Chapter 10, 279
//---------------------------------------------------------------------------------------------------------------------
class EventDataSetControlledActor : public EventData
{
    ActorId mId;

public:
    static const BaseEventType skEventType;

    EventDataSetControlledActor(void) { }
    EventDataSetControlledActor(ActorId actorId)
        : mId(actorId)
    {
    }

    virtual const BaseEventType& GetEventType(void) const
    {
        return skEventType;
    }

    virtual BaseEventDataPtr Copy() const
    {
        return BaseEventDataPtr(new EventDataSetControlledActor(mId));
    }

    virtual void Serialize(std::ostrstream& out) const
    {
        out << mId;
    }

    virtual void Deserialize(std::istrstream& in)
    {
        in >> mId;
    }

    const ActorId& GetActorId(void) const
    {
        return mId;
    }

    virtual const char* GetName(void) const
    {
        return "EventDataSetControlledActor";
    }
};

#endif