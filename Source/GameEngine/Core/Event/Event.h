//========================================================================
// Events.h : defines common game events
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

#ifndef EVENT_H
#define EVENT_H

#include "EventManager.h"

#include "Core/Logger/Logger.h"
#include "Graphic/Scene/Hierarchy/Node.h"
#include "Mathematic/Algebra/Transform.h"


//---------------------------------------------------------------------------------------------------------------------
// EventDataNewActor - This event is sent out when an actor is *actually* created.
//---------------------------------------------------------------------------------------------------------------------
class EventDataNewActor : public EventData
{
	ActorId mActorId;
    GameViewId mViewId;

public:
	static const BaseEventType skEventType;

	EventDataNewActor(void) 
	{
		mActorId = INVALID_ACTOR_ID;
		mViewId = INVALID_GAME_VIEW_ID;
	}

    explicit EventDataNewActor(ActorId actorId, GameViewId viewId = INVALID_GAME_VIEW_ID) 
        : mActorId(actorId), mViewId(viewId)
	{
	}

    virtual void Deserialize(std::istrstream& in)
    {
        in >> mActorId;
		in >> mViewId;
    }

	virtual const BaseEventType& GetEventType(void) const
	{
		return skEventType;
	}

	virtual BaseEventDataPtr Copy(void) const
	{
		return BaseEventDataPtr(new EventDataNewActor(mActorId, mViewId));
	}

	virtual void Serialize(std::ostrstream& out) const
	{
		out << mActorId << " ";
		out << mViewId << " ";
	}


    virtual const char* GetName(void) const
    {
        return "EventDataNewActor";
    }

	const ActorId GetActorId(void) const
	{
		return mActorId;
	}

    GameViewId GetViewId(void) const
    {
        return mViewId;
    }
};


//---------------------------------------------------------------------------------------------------------------------
// EventDataDestroyActor - sent when actors are destroyed	
//---------------------------------------------------------------------------------------------------------------------
class EventDataDestroyActor : public EventData
{
    ActorId mId;

public:
	static const BaseEventType skEventType;

    explicit EventDataDestroyActor(ActorId id = INVALID_ACTOR_ID)
        : mId(id)
    {
        //
    }

	virtual const BaseEventType& GetEventType(void) const
	{
		return skEventType;
	}

	virtual BaseEventDataPtr Copy(void) const
	{
		return BaseEventDataPtr ( new EventDataDestroyActor ( mId ) );
	}

	virtual void Serialize(std::ostrstream &out) const
	{
		out << mId;
	}

    virtual void Deserialize(std::istrstream& in)
    {
        in >> mId;
    }

    virtual const char* GetName(void) const
    {
        return "EventDataDestroyActor";
    }

    ActorId GetId(void) const { return mId; }
};


//---------------------------------------------------------------------------------------------------------------------
// EventDataSyncActor - sent when actors transform needs to be synchronized
//---------------------------------------------------------------------------------------------------------------------
class EventDataSyncActor : public EventData
{
	ActorId mId;
	Transform mTransform;

public:
	static const BaseEventType skEventType;

	virtual const BaseEventType& GetEventType(void) const
	{
		return skEventType;
	}

	EventDataSyncActor(void)
	{
		mId = INVALID_ACTOR_ID;
	}

	EventDataSyncActor(ActorId id, const Transform& trans)
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
		return BaseEventDataPtr(new EventDataSyncActor(mId, mTransform));
	}

	virtual const char* GetName(void) const
	{
		return "EventDataSyncActor";
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
// EventDataNewRenderComponent - This event is sent out when an actor is *actually* created.
//---------------------------------------------------------------------------------------------------------------------
class EventDataNewRenderComponent : public EventData
{
    ActorId mActorId;
    eastl::shared_ptr<Node> mSceneNode;

public:
	static const BaseEventType skEventType;

    EventDataNewRenderComponent(void) 
    {
        mActorId = INVALID_ACTOR_ID;
    }

    explicit EventDataNewRenderComponent(ActorId actorId, const eastl::shared_ptr<Node>& sceneNode)
        : mActorId(actorId),
          mSceneNode(sceneNode)
    {
    }

    virtual void Serialize(std::ostrstream& out) const
    {
        LogError(eastl::string(GetName()) + eastl::string(" should not be serialzied!"));
    }

    virtual void Deserialize(std::istrstream& in)
    {
        LogError(eastl::string(GetName()) + eastl::string(" should not be serialzied!"));
    }

    virtual const BaseEventType& GetEventType(void) const
    {
        return skEventType;
    }

    virtual BaseEventDataPtr Copy(void) const
    {
        return BaseEventDataPtr(new EventDataNewRenderComponent(mActorId, mSceneNode));
    }

    virtual const char* GetName(void) const
    {
        return "EventDataNewRenderComponent";
    }

    const ActorId GetActorId(void) const
    {
        return mActorId;
    }

    const eastl::shared_ptr<Node>& GetSceneNode(void) const
    {
        return mSceneNode;
    }
};

//---------------------------------------------------------------------------------------------------------------------
// EventDataModifiedRenderComponent - This event is sent out when a render component is changed
//   NOTE: This class is not described in the book!
//---------------------------------------------------------------------------------------------------------------------
class EventDataModifiedRenderComponent : public EventData
{
    ActorId mId;

public:
	static const BaseEventType skEventType;

	virtual const BaseEventType& GetEventType(void) const
	{
		return skEventType;
	}

    EventDataModifiedRenderComponent(void)
    {
        mId = INVALID_ACTOR_ID;
    }

	EventDataModifiedRenderComponent(ActorId id)
        : mId(id)
	{
	}

	virtual void Serialize(std::ostrstream &out) const
	{
		out << mId;
	}

    virtual void Deserialize(std::istrstream& in)
    {
        in >> mId;
    }

	virtual BaseEventDataPtr Copy() const
	{
		return BaseEventDataPtr(new EventDataModifiedRenderComponent(mId));
	}

    virtual const char* GetName(void) const
    {
        return "EventDataModifiedRenderComponent";
    }

    ActorId GetActorId(void) const
    {
        return mId;
    }
};


//---------------------------------------------------------------------------------------------------------------------
// EventDataEnvironmentLoaded - this event is sent when a new game is started
//---------------------------------------------------------------------------------------------------------------------
class EventDataEnvironmentLoaded : public EventData
{
public:
	static const BaseEventType skEventType;

    EventDataEnvironmentLoaded(void) { }
	virtual const BaseEventType& GetEventType(void) const	{ return skEventType; }
	virtual BaseEventDataPtr Copy(void) const { return BaseEventDataPtr( new EventDataEnvironmentLoaded( ) ); }
    virtual const char* GetName(void) const  { return "EventDataEnvironmentLoaded";  }
};


//---------------------------------------------------------------------------------------------------------------------
// EventtDataEnvironmentLoaded - this event is sent when a client has loaded its environment
//   This is special because we only want this event to go from client to server, and stop there. The
//   EventtDataEnvironmentLoaded is received by server and proxy logics alike. Thy to do this with just the above 
//   event and you'll get into an endless loop of the EventtDataEnvironmentLoaded event making infinite round trips
//   from client to server.
//
// FUTURE_WORK: It would be an interesting idea to add a "Private" type of event that is addressed only to a specific 
//              listener. Of course, that might be a really dumb idea too - someone will have to try it!
//---------------------------------------------------------------------------------------------------------------------
class EventDataRemoteEnvironmentLoaded : public EventData
{
public:
	static const BaseEventType skEventType;

    EventDataRemoteEnvironmentLoaded(void) { }
	virtual const BaseEventType& GetEventType(void) const	{ return skEventType; }
	virtual BaseEventDataPtr Copy(void) const { return BaseEventDataPtr( new EventDataRemoteEnvironmentLoaded( ) ); }
    virtual const char* GetName(void) const  { return "EventDataRemoteEnvironmentLoaded";  }
};


//---------------------------------------------------------------------------------------------------------------------
// EventDataRequestStartGame - this is sent by the authoritative game logic to all views so they will load a game level.
//---------------------------------------------------------------------------------------------------------------------
class EventDataRequestStartGame : public EventData
{

public:
	static const BaseEventType skEventType;

    EventDataRequestStartGame(void) { }

	virtual const BaseEventType& GetEventType(void) const
	{
		return skEventType;
	}

	virtual BaseEventDataPtr Copy() const
	{
		return BaseEventDataPtr( new EventDataRequestStartGame( ) );
	}

    virtual const char* GetName(void) const
    {
        return "EventDataRequestStartGame";
    }
};

//---------------------------------------------------------------------------------------------------------------------
// EventDataRemoteClient						- Chapter 19, page 687
// 
//   Sent whenever a new client attaches to a game logic acting as a server				
//---------------------------------------------------------------------------------------------------------------------
class EventDataRemoteClient : public EventData
{
    int mSocketId;
    int mIPAddress;

public:
	static const BaseEventType skEventType;

    EventDataRemoteClient(void)
    {
        mSocketId = 0;
        mIPAddress = 0;
    }

	EventDataRemoteClient( const int socketid, const int ipaddress )
		: mSocketId( socketid ), mIPAddress(ipaddress)
	{
	}

    virtual const BaseEventType & GetEventType( void ) const
    {
        return skEventType;
    }

	virtual BaseEventDataPtr Copy() const
	{
		return BaseEventDataPtr( new EventDataRemoteClient( mSocketId, mIPAddress ) );
	}

    virtual const char* GetName(void) const
    {
        return "EventDataRemoteClient";
    }

	virtual void Serialize(std::ostrstream &out) const
	{
		out << mSocketId << " ";
		out << mIPAddress;
	}

    virtual void Deserialize( std::istrstream &in )
    {
        in >> mSocketId;
        in >> mIPAddress;
    }

    int GetSocketId(void) const
    {
        return mSocketId;
    }

    int GetIpAddress(void) const
    {
        return mIPAddress;
    }
};


//---------------------------------------------------------------------------------------------------------------------
// EventDataUpdateTick - sent by the game logic each game tick
//---------------------------------------------------------------------------------------------------------------------
class EventDataUpdateTick : public EventData
{
    int mDeltaTime;

public:
	static const BaseEventType skEventType;

    explicit EventDataUpdateTick( const int dTime )
        : mDeltaTime( dTime )
    {
    }

	virtual const BaseEventType& GetEventType( void ) const
	{
		return skEventType;
	}

	virtual BaseEventDataPtr Copy() const
	{
		return BaseEventDataPtr (new EventDataUpdateTick ( mDeltaTime ) );
	}

	virtual void Serialize( std::ostrstream & out )
	{
		LogError("You should not be serializing update ticks!");
	}

    virtual const char* GetName(void) const
    {
        return "EventDataUpdateTick";
    }
};


//---------------------------------------------------------------------------------------------------------------------
// EventDataNetworkPlayerActorAssignment - sent by the server to the clients when a network view is assigned a player number
//---------------------------------------------------------------------------------------------------------------------
class EventDataNetworkPlayerActorAssignment : public EventData
{
    ActorId mActorId;
    int mSocketId;

public:
	static const BaseEventType skEventType;

	EventDataNetworkPlayerActorAssignment()
	{
		mActorId = INVALID_ACTOR_ID;
		mSocketId = -1;
	}

    explicit EventDataNetworkPlayerActorAssignment( const ActorId actorId, const int socketId )
        : mActorId( actorId ), mSocketId(socketId)

    {
    }

	virtual const BaseEventType & GetEventType( void ) const
	{
		return skEventType;
	}

	virtual BaseEventDataPtr Copy() const
	{
		 return BaseEventDataPtr( new EventDataNetworkPlayerActorAssignment( mActorId, mSocketId ) ) ;
	}

    virtual const char* GetName(void) const
    {
        return "EventDataNetworkPlayerActorAssignment";
    }


	virtual void Serialize(std::ostrstream &out) const
	{
		out << mActorId << " ";
		out << mSocketId;
	}

    virtual void Deserialize( std::istrstream &in )
    {
        in >> mActorId;
        in >> mSocketId;
    }

    ActorId GetActorId(void) const
    {
        return mActorId;
    }

    ActorId GetSocketId(void) const
    {
        return mSocketId;
    }
};


//---------------------------------------------------------------------------------------------------------------------
// EventDataDecompressRequest - sent to a multithreaded game event listener to decompress something in the resource file
//---------------------------------------------------------------------------------------------------------------------
class EventDataDecompressRequest : public EventData
{
    eastl::wstring mZipFileName;
    eastl::string mFileName;

public:
	static const BaseEventType skEventType;

    explicit EventDataDecompressRequest( eastl::wstring zipFileName, eastl::string filename  )
        : mZipFileName( zipFileName ), mFileName( filename )
    {
    }

	virtual const BaseEventType& GetEventType( void ) const
	{
		return skEventType;
	}

	virtual BaseEventDataPtr Copy() const
	{
		return BaseEventDataPtr (new EventDataDecompressRequest ( mZipFileName, mFileName ) );
	}

	virtual void Serialize( std::ostrstream & out )
	{
		LogError( "You should not be serializing decompression requests!" );
	}

    const eastl::wstring& GetZipFilename(void) const
    {
        return mZipFileName;
    }

    const eastl::string& GetFilename(void) const
    {
        return mFileName;
    }
    virtual const char* GetName(void) const
    {
        return "EventDataDecompressRequest";
    }
};


//---------------------------------------------------------------------------------------------------------------------
// EventDataDecompressionProgress - sent by the decompression thread to report progress
//---------------------------------------------------------------------------------------------------------------------
class EventDataDecompressionProgress : public EventData
{
    int mProgress;
    eastl::wstring mZipFileName;
    eastl::string mFileName;
    void *mBuffer;

public:
	static const BaseEventType skEventType;

    EventDataDecompressionProgress( int progress, eastl::wstring zipFileName, eastl::string filename, void *buffer )
        : mProgress(progress),
        mZipFileName( zipFileName ),
        mFileName ( filename ),
        mBuffer (buffer)
    {
    }

	virtual const BaseEventType & GetEventType( void ) const
	{
		return skEventType;
	}

	virtual BaseEventDataPtr Copy() const
	{
		return BaseEventDataPtr (new EventDataDecompressionProgress ( mProgress, mZipFileName, mFileName, mBuffer ) );
	}

	virtual void Serialize( std::ostrstream & out )
	{
		LogError( "You should not be serializing decompression progress events!" );
	}

    virtual const char* GetName(void) const
    {
        return "EventDataDecompressionProgress";
    }

};


//---------------------------------------------------------------------------------------------------------------------
// class EventDataRequestNewActor				
// This event is sent by a server asking Client proxy logics to create new actors from their local resources.
// It can be sent from script or via code.
// This event is also sent from the server game logic to client logics AFTER it has created a new actor. The logics will allow follow suit to stay in sync.
//---------------------------------------------------------------------------------------------------------------------
class EventDataRequestNewActor : public EventData
{
    eastl::string mActorResource;
	bool mIsInitialTransform;
	Transform mInitialTransform;
	ActorId mServerActorId;
    GameViewId mViewId;

public:
	static const BaseEventType skEventType;

	EventDataRequestNewActor()
	{
		mActorResource = "";
		mIsInitialTransform = false;
		mServerActorId = -1;
		mViewId = INVALID_GAME_VIEW_ID;
	}

    explicit EventDataRequestNewActor(const eastl::string &actorResource, const Transform *initialTransform=NULL, 
		const ActorId serverActorId = INVALID_ACTOR_ID, const GameViewId viewId = INVALID_GAME_VIEW_ID)
    {
        mActorResource = actorResource;
		if (initialTransform)
		{
			mIsInitialTransform = true;
			mInitialTransform = *initialTransform;
		}
		else
			mIsInitialTransform = false;

		mServerActorId = serverActorId;
        mViewId = viewId;
    }

	virtual const BaseEventType& GetEventType(void) const
	{
		return skEventType;
	}

	virtual void Deserialize( std::istrstream & in )
	{
		std::string actorResource;
		in >> actorResource;
		mActorResource = eastl::string(actorResource.c_str());
		in >> mIsInitialTransform;
		if (mIsInitialTransform)
		{
			Matrix4x4<float> transform = mInitialTransform.GetMatrix();
			for (int i=0; i<4; ++i)
				for (int j=0; j<4; ++j)
					in >> transform(i,j);
		}
		in >> mServerActorId;
		in >> mViewId;
	}

	virtual BaseEventDataPtr Copy() const
	{	 
		return BaseEventDataPtr (new EventDataRequestNewActor(
			mActorResource, (mIsInitialTransform) ? &mInitialTransform : NULL, mServerActorId, mViewId));
	}

	virtual void Serialize( std::ostrstream & out ) const
	{
		out << mActorResource.c_str() << " ";
		out << mIsInitialTransform << " ";
		if (mIsInitialTransform)
		{
			Matrix4x4<float> transform = mInitialTransform.GetMatrix();
			for (int i=0; i<4; ++i)
				for (int j=0; j<4; ++j)
					out << transform(i,j) << " ";
		}
		out << mServerActorId << " ";
		out << mViewId << " ";
	}

    virtual const char* GetName(void) const { return "EventDataRequestNewActor";  }

    const eastl::string &GetActorResource(void) const { return mActorResource;  }
	const Transform *GetInitialTransform(void) const { return (mIsInitialTransform) ? &mInitialTransform : NULL; }
	const ActorId GetServerActorId(void) const 	{ return mServerActorId; }
    GameViewId GetViewId(void) const { return mViewId; }
};


//---------------------------------------------------------------------------------------------------------------------
// EventDataRequestDestroyActor - sent by any system requesting that the game logic destroy an actor	
//---------------------------------------------------------------------------------------------------------------------
class EventDataRequestDestroyActor : public EventData
{
    ActorId mActorId;

public:
    static const BaseEventType skEventType;

    EventDataRequestDestroyActor()
    {
        mActorId = INVALID_ACTOR_ID;
    }

    EventDataRequestDestroyActor(ActorId actorId)
    {
        mActorId = actorId;
    }

    virtual const BaseEventType& GetEventType(void) const
    {
        return skEventType;
    }

    virtual void Deserialize( std::istrstream & in )
    {
        in >> mActorId;
    }

    virtual BaseEventDataPtr Copy() const
    {
        return BaseEventDataPtr (new EventDataRequestDestroyActor(mActorId));
    }

    virtual void Serialize( std::ostrstream & out ) const
    {
        out << mActorId;
    }

    virtual const char* GetName(void) const
    {
        return "EventDataRequestDestroyActor";
    }

    ActorId GetActorId(void) const
    {
        return mActorId;
    }
};


//---------------------------------------------------------------------------------------------------------------------
// EventDataPlaySound - sent by any system wishing for a HumanView to play a sound
//---------------------------------------------------------------------------------------------------------------------
class EventDataPlaySound : public EventData
{
    eastl::string mSoundResource;

public:
    static const BaseEventType skEventType;

    EventDataPlaySound(void) { }
    EventDataPlaySound(const eastl::string& soundResource)
        : mSoundResource(soundResource)
    {
    }

    virtual const BaseEventType& GetEventType(void) const
    {
        return skEventType;
    }

    virtual BaseEventDataPtr Copy() const
    {
        return BaseEventDataPtr(new EventDataPlaySound(mSoundResource));
    }

    virtual void Serialize(std::ostrstream& out) const
    {
        out << mSoundResource.c_str();
    }

    virtual void Deserialize(std::istrstream& in)
    {
		std::string soundResource;
        in >> soundResource;
		mSoundResource = eastl::string(soundResource.c_str());
    }

    const eastl::string& GetResource(void) const
    {
        return mSoundResource;
    }

    virtual const char* GetName(void) const
    {
        return "EventDataPlaySound";
    }
};

#endif