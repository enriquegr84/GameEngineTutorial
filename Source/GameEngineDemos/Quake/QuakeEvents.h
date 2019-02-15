//========================================================================
// QuakeEvents.h : defines game-specific events for Quake
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


#ifndef QUAKEEVENTS_H
#define QUAKEEVENTS_H

#include "Core/Event/EventManager.h"
#include "Core/Event/Event.h"

//---------------------------------------------------------------------------------------------------------------------
// class QuakeEventDataFireWeapon
//---------------------------------------------------------------------------------------------------------------------
class QuakeEventDataFireWeapon : public EventData
{
    ActorId mId;

public:
    static const BaseEventType skEventType;
    virtual const BaseEventType & GetEventType() const
    {
        return skEventType;
    }

	QuakeEventDataFireWeapon(void)
    {
        mId = INVALID_ACTOR_ID;
    }

	QuakeEventDataFireWeapon( ActorId id )
	: mId( id )
    {
    }

    virtual BaseEventDataPtr Copy() const
    {
        return BaseEventDataPtr(new QuakeEventDataFireWeapon(mId));
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
        return "QuakeEventDataFireWeapon";
    }

	ActorId GetId(void) const
    {
        return mId;
    }
};


//---------------------------------------------------------------------------------------------------------------------
// class QuakeEventDataChangeWeapon
//---------------------------------------------------------------------------------------------------------------------
class QuakeEventDataChangeWeapon : public EventData
{
	ActorId mId;

public:
	static const BaseEventType skEventType;
	virtual const BaseEventType & GetEventType() const
	{
		return skEventType;
	}

	QuakeEventDataChangeWeapon(void)
	{
		mId = INVALID_ACTOR_ID;
	}

	QuakeEventDataChangeWeapon(ActorId id)
		: mId(id)
	{
	}

	virtual BaseEventDataPtr Copy() const
	{
		return BaseEventDataPtr(new QuakeEventDataChangeWeapon(mId));
	}

	virtual void Serialize(std::ostrstream & out) const
	{
		out << mId << " ";
	}

	virtual void Deserialize(std::istrstream & in)
	{
		in >> mId;
	}

	virtual const char* GetName(void) const
	{
		return "QuakeEventDataChangeWeapon";
	}

	ActorId GetId(void) const
	{
		return mId;
	}
};

//---------------------------------------------------------------------------------------------------------------------
// class QuakeEventDataSplashDamage
//---------------------------------------------------------------------------------------------------------------------
class QuakeEventDataSplashDamage : public EventData
{
	ActorId mId;
	Vector3<float> mOrigin;

public:
	static const BaseEventType skEventType;
	virtual const BaseEventType & GetEventType() const
	{
		return skEventType;
	}

	QuakeEventDataSplashDamage(void)
	{
		mId = INVALID_ACTOR_ID;
	}

	QuakeEventDataSplashDamage(ActorId id, const Vector3<float>& origin)
		: mId(id), mOrigin(origin)
	{
	}

	virtual BaseEventDataPtr Copy() const
	{
		return BaseEventDataPtr(new QuakeEventDataSplashDamage(mId, mOrigin));
	}

	virtual void Serialize(std::ostrstream &out) const
	{
		out << mId << " ";
		for (int i = 0; i<3; ++i)
			out << mOrigin[i] << " ";
	}

	virtual void Deserialize(std::istrstream& in)
	{
		in >> mId;
		for (int i = 0; i<3; ++i)
			in >> mOrigin[i];
	}

	virtual const char* GetName(void) const
	{
		return "QuakeEventDataSplashDamage";
	}

	ActorId GetId(void) const
	{
		return mId;
	}

	const Vector3<float>& GetOrigin(void) const
	{
		return mOrigin;
	}
};



//---------------------------------------------------------------------------------------------------------------------
// class QuakeEventDataDeadActor
//---------------------------------------------------------------------------------------------------------------------
class QuakeEventDataDeadActor : public EventData
{
	ActorId mId;

public:
	static const BaseEventType skEventType;
	virtual const BaseEventType & GetEventType() const
	{
		return skEventType;
	}

	QuakeEventDataDeadActor(void)
	{
		mId = INVALID_ACTOR_ID;
	}

	QuakeEventDataDeadActor(ActorId id)
		: mId(id)
	{
	}

	virtual BaseEventDataPtr Copy() const
	{
		return BaseEventDataPtr(new QuakeEventDataDeadActor(mId));
	}

	virtual void Serialize(std::ostrstream & out) const
	{
		out << mId << " ";
	}

	virtual void Deserialize(std::istrstream & in)
	{
		in >> mId;
	}

	virtual const char* GetName(void) const
	{
		return "QuakeEventDataDeadActor";
	}

	ActorId GetId(void) const
	{
		return mId;
	}
};


//---------------------------------------------------------------------------------------------------------------------
// QuakeEventDataSpawnActor - sent when actors are spawned
//---------------------------------------------------------------------------------------------------------------------
class QuakeEventDataSpawnActor : public EventData
{
	ActorId mId;

public:
	static const BaseEventType skEventType;

	virtual const BaseEventType& GetEventType(void) const
	{
		return skEventType;
	}

	QuakeEventDataSpawnActor(void)
	{
		mId = INVALID_ACTOR_ID;
	}

	QuakeEventDataSpawnActor(ActorId id)
		: mId(id)
	{
		//
	}

	virtual void Serialize(std::ostrstream &out) const
	{
		out << mId << " ";
	}

	virtual void Deserialize(std::istrstream& in)
	{
		in >> mId;
	}

	virtual BaseEventDataPtr Copy() const
	{
		return BaseEventDataPtr(new QuakeEventDataSpawnActor(mId));
	}

	virtual const char* GetName(void) const
	{
		return "QuakeEventDataSpawnActor";
	}

	ActorId GetId(void) const
	{
		return mId;
	}
};

//---------------------------------------------------------------------------------------------------------------------
// QuakeEventDataPushActor - sent when actor is pushed
//---------------------------------------------------------------------------------------------------------------------
class QuakeEventDataPushActor : public EventData
{
	ActorId mId;
	Vector3<float> mDirection;

public:
	static const BaseEventType skEventType;

	virtual const BaseEventType& GetEventType(void) const
	{
		return skEventType;
	}

	QuakeEventDataPushActor(void)
	{
		mId = INVALID_ACTOR_ID;
	}

	QuakeEventDataPushActor(ActorId id, const Vector3<float>& dir)
		: mId(id), mDirection(dir)
	{
		//
	}

	virtual void Serialize(std::ostrstream &out) const
	{
		out << mId << " ";
		for (int i = 0; i<3; ++i)
			out << mDirection[i] << " ";
	}

	virtual void Deserialize(std::istrstream& in)
	{
		in >> mId;
		for (int i = 0; i<3; ++i)
			in >> mDirection[i];
	}

	virtual BaseEventDataPtr Copy() const
	{
		return BaseEventDataPtr(new QuakeEventDataPushActor(mId, mDirection));
	}

	virtual const char* GetName(void) const
	{
		return "QuakeEventDataPushActor";
	}

	ActorId GetId(void) const
	{
		return mId;
	}

	const Vector3<float>& GetDirection(void) const
	{
		return mDirection;
	}
};

//---------------------------------------------------------------------------------------------------------------------
// QuakeEventDataJumpActor - sent when actor jumps
//---------------------------------------------------------------------------------------------------------------------
class QuakeEventDataJumpActor : public EventData
{
	ActorId mId;
	Vector3<float> mDirection;

public:
	static const BaseEventType skEventType;

	virtual const BaseEventType& GetEventType(void) const
	{
		return skEventType;
	}

	QuakeEventDataJumpActor(void)
	{
		mId = INVALID_ACTOR_ID;
	}

	QuakeEventDataJumpActor(ActorId id, const Vector3<float>& dir)
		: mId(id), mDirection(dir)
	{
		//
	}

	virtual void Serialize(std::ostrstream &out) const
	{
		out << mId << " ";
		for (int i = 0; i<3; ++i)
			out << mDirection[i] << " ";
	}

	virtual void Deserialize(std::istrstream& in)
	{
		in >> mId;
		for (int i = 0; i<3; ++i)
			in >> mDirection[i];
	}

	virtual BaseEventDataPtr Copy() const
	{
		return BaseEventDataPtr(new QuakeEventDataJumpActor(mId, mDirection));
	}

	virtual const char* GetName(void) const
	{
		return "QuakeEventDataJumpActor";
	}

	ActorId GetId(void) const
	{
		return mId;
	}

	const Vector3<float>& GetDirection(void) const
	{
		return mDirection;
	}
};

//---------------------------------------------------------------------------------------------------------------------
// QuakeEventDataMoveActor - sent when actors are moved
//---------------------------------------------------------------------------------------------------------------------
class QuakeEventDataMoveActor : public EventData
{
	ActorId mId;
	Vector3<float> mDirection;

public:
	static const BaseEventType skEventType;

	virtual const BaseEventType& GetEventType(void) const
	{
		return skEventType;
	}

	QuakeEventDataMoveActor(void)
	{
		mId = INVALID_ACTOR_ID;
	}

	QuakeEventDataMoveActor(ActorId id, const Vector3<float>& dir)
		: mId(id), mDirection(dir)
	{
		//
	}

	virtual void Serialize(std::ostrstream &out) const
	{
		out << mId << " ";
		for (int i = 0; i<3; ++i)
			out << mDirection[i] << " ";
	}

	virtual void Deserialize(std::istrstream& in)
	{
		in >> mId;
		for (int i = 0; i<3; ++i)
			in >> mDirection[i];
	}

	virtual BaseEventDataPtr Copy() const
	{
		return BaseEventDataPtr(new QuakeEventDataMoveActor(mId, mDirection));
	}

	virtual const char* GetName(void) const
	{
		return "QuakeEventDataMoveActor";
	}

	ActorId GetId(void) const
	{
		return mId;
	}

	const Vector3<float>& GetDirection(void) const
	{
		return mDirection;
	}
};

//---------------------------------------------------------------------------------------------------------------------
// QuakeEventDataFallActor - sent when actors are falling
//---------------------------------------------------------------------------------------------------------------------
class QuakeEventDataFallActor : public EventData
{
	ActorId mId;
	Vector3<float> mDirection;

public:
	static const BaseEventType skEventType;

	virtual const BaseEventType& GetEventType(void) const
	{
		return skEventType;
	}

	QuakeEventDataFallActor(void)
	{
		mId = INVALID_ACTOR_ID;
	}

	QuakeEventDataFallActor(ActorId id, const Vector3<float>& dir)
		: mId(id), mDirection(dir)
	{
		//
	}

	virtual void Serialize(std::ostrstream &out) const
	{
		out << mId << " ";
		for (int i = 0; i<3; ++i)
			out << mDirection[i] << " ";
	}

	virtual void Deserialize(std::istrstream& in)
	{
		in >> mId;
		for (int i = 0; i<3; ++i)
			in >> mDirection[i];
	}

	virtual BaseEventDataPtr Copy() const
	{
		return BaseEventDataPtr(new QuakeEventDataFallActor(mId, mDirection));
	}

	virtual const char* GetName(void) const
	{
		return "QuakeEventDataFallActor";
	}

	ActorId GetId(void) const
	{
		return mId;
	}

	const Vector3<float>& GetDirection(void) const
	{
		return mDirection;
	}
};

//---------------------------------------------------------------------------------------------------------------------
// QuakeEventDataRotateActor - sent when actors are moved
//---------------------------------------------------------------------------------------------------------------------
class QuakeEventDataRotateActor : public EventData
{
	ActorId mId;
	Transform mTransform;

public:
	static const BaseEventType skEventType;

	virtual const BaseEventType& GetEventType(void) const
	{
		return skEventType;
	}

	QuakeEventDataRotateActor(void)
	{
		mId = INVALID_ACTOR_ID;
	}

	QuakeEventDataRotateActor(ActorId id, const Transform& trans)
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
		return BaseEventDataPtr(new QuakeEventDataRotateActor(mId, mTransform));
	}

	virtual const char* GetName(void) const
	{
		return "QuakeEventDataRotateActor";
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
// class QuakeEventDataStartThrust				        - Chapter 10, 279
//---------------------------------------------------------------------------------------------------------------------
class QuakeEventDataStartThrust : public EventData
{
    ActorId mId;
    float mAcceleration;

public:
    static const BaseEventType skEventType;
    virtual const BaseEventType& GetEventType( void ) const
    {
        return skEventType;
    }

	QuakeEventDataStartThrust(void)
	: mId(INVALID_ACTOR_ID), mAcceleration(0)
    {
        //
    }

	QuakeEventDataStartThrust(ActorId id, float acceleration)
	: mId(id), mAcceleration(acceleration)
    {
        //
    }

    virtual BaseEventDataPtr Copy() const
    {
        return BaseEventDataPtr(new QuakeEventDataStartThrust(mId, mAcceleration));
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
        return "QuakeEventDataStartThrust";
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
// class QuakeEventDataEndThrust				        - Chapter 10, 279
//---------------------------------------------------------------------------------------------------------------------
class QuakeEventDataEndThrust : public EventData
{
    ActorId mId;

public:
    static const BaseEventType skEventType;
    virtual const BaseEventType & GetEventType( void ) const
    {
        return skEventType;
    }

	QuakeEventDataEndThrust(void) :
        mId(INVALID_ACTOR_ID)
    {
        //
    }

	QuakeEventDataEndThrust(ActorId id) :
        mId(id)
    {
        //
    }

    virtual BaseEventDataPtr Copy() const
    {
        return BaseEventDataPtr(new QuakeEventDataEndThrust(mId));
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
        return "QuakeEventDataEndThrust";
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
// class QuakeEventDataStartSteer				             - Chapter 10, 279
//---------------------------------------------------------------------------------------------------------------------
class QuakeEventDataStartSteer : public EventData
{
    ActorId mId;
    float mAcceleration;

public:
    static const BaseEventType skEventType;
    virtual const BaseEventType & GetEventType( void ) const
    {
        return skEventType;
    }

	QuakeEventDataStartSteer(void)
	: mId(INVALID_ACTOR_ID), mAcceleration(0)
    {
        //
    }

	QuakeEventDataStartSteer(ActorId id, float acceleration)
	: mId(id), mAcceleration(acceleration)
    {
        //
    }

    virtual BaseEventDataPtr Copy() const
    {
        return BaseEventDataPtr(new QuakeEventDataStartSteer(mId, mAcceleration));
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
        return "QuakeEventDataStartSteer";
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
// class QuakeEventDataEndSteer				        - Chapter 10, 279
//---------------------------------------------------------------------------------------------------------------------
class QuakeEventDataEndSteer : public EventData
{
    ActorId mId;

public:
    static const BaseEventType skEventType;
    virtual const BaseEventType & GetEventType( void ) const
    {
        return skEventType;
    }

	QuakeEventDataEndSteer(void) :
        mId(INVALID_ACTOR_ID)
    {
        //
    }

	QuakeEventDataEndSteer(ActorId id) :
        mId(id)
    {
        //
    }

    virtual BaseEventDataPtr Copy() const
    {
        return BaseEventDataPtr(new QuakeEventDataEndSteer(mId));
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
        return "QuakeEventDataEndSteer";
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
// class QuakeEventDataGameplayUIUpdate				- Chapter 10, 279
//---------------------------------------------------------------------------------------------------------------------
class QuakeEventDataGameplayUIUpdate : public EventData
{
    eastl::string mGameplayUiString;

public:
    static const BaseEventType skEventType;

	QuakeEventDataGameplayUIUpdate(void) { }
	QuakeEventDataGameplayUIUpdate(const eastl::string& gameplayUiString)
	: mGameplayUiString(gameplayUiString)
    {
    }

    virtual const BaseEventType& GetEventType(void) const
    {
        return skEventType;
    }

    virtual BaseEventDataPtr Copy() const
    {
        return BaseEventDataPtr(new QuakeEventDataGameplayUIUpdate(mGameplayUiString));
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
        return "QuakeEventDataGameplayUIUpdate";
    }
};


//---------------------------------------------------------------------------------------------------------------------
// class QuakeEventDataSetControlledActor				- Chapter 10, 279
//---------------------------------------------------------------------------------------------------------------------
class QuakeEventDataSetControlledActor : public EventData
{
    ActorId mId;

public:
    static const BaseEventType skEventType;

	QuakeEventDataSetControlledActor(void) { }
	QuakeEventDataSetControlledActor(ActorId actorId)
        : mId(actorId)
    {
    }

    virtual const BaseEventType& GetEventType(void) const
    {
        return skEventType;
    }

    virtual BaseEventDataPtr Copy() const
    {
        return BaseEventDataPtr(new QuakeEventDataSetControlledActor(mId));
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
        return "QuakeEventDataSetControlledActor";
    }
};

#endif