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

#include "Mathematic/Algebra/Transform.h"


//---------------------------------------------------------------------------------------------------------------------
// EvtData_New_Actor - This event is sent out when an actor is *actually* created.
//---------------------------------------------------------------------------------------------------------------------
class EvtData_New_Actor : public EventData
{
	ActorId m_actorId;
    GameViewId m_viewId;

public:
	static const BaseEventType sk_EventType;

	EvtData_New_Actor(void) 
	{
		m_actorId = INVALID_ACTOR_ID;
		m_viewId = InvalidGameViewId;
	}

    explicit EvtData_New_Actor(ActorId actorId, GameViewId viewId = InvalidGameViewId) 
        : m_actorId(actorId),
          m_viewId(viewId)
	{
	}

    virtual void Deserialize(std::istrstream& in)
    {
        in >> m_actorId;
		in >> m_viewId;
    }

	virtual const BaseEventType& GetEventType(void) const
	{
		return sk_EventType;
	}

	virtual BaseEventDataPtr Copy(void) const
	{
		return BaseEventDataPtr(new EvtData_New_Actor(m_actorId, m_viewId));
	}

	virtual void Serialize(std::ostrstream& out) const
	{
		out << m_actorId << " ";
		out << m_viewId << " ";
	}


    virtual const char* GetName(void) const
    {
        return "EvtData_New_Actor";
    }

	const ActorId GetActorId(void) const
	{
		return m_actorId;
	}

    GameViewId GetViewId(void) const
    {
        return m_viewId;
    }
};


//---------------------------------------------------------------------------------------------------------------------
// EvtData_Destroy_Actor - sent when actors are destroyed	
//---------------------------------------------------------------------------------------------------------------------
class EvtData_Destroy_Actor : public EventData
{
    ActorId m_id;

public:
	static const BaseEventType sk_EventType;

    explicit EvtData_Destroy_Actor(ActorId id = INVALID_ACTOR_ID)
        : m_id(id)
    {
        //
    }

	virtual const BaseEventType& GetEventType(void) const
	{
		return sk_EventType;
	}

	virtual BaseEventDataPtr Copy(void) const
	{
		return BaseEventDataPtr ( new EvtData_Destroy_Actor ( m_id ) );
	}

	virtual void Serialize(std::ostrstream &out) const
	{
		out << m_id;
	}

    virtual void Deserialize(std::istrstream& in)
    {
        in >> m_id;
    }

    virtual const char* GetName(void) const
    {
        return "EvtData_Destroy_Actor";
    }

    ActorId GetId(void) const { return m_id; }
};


//---------------------------------------------------------------------------------------------------------------------
// EvtData_Move_Actor - sent when actors are moved
//---------------------------------------------------------------------------------------------------------------------
class EvtData_Move_Actor : public EventData
{
    ActorId m_id;
    Matrix4x4<float> m_matrix;

public:
	static const BaseEventType sk_EventType;

	virtual const BaseEventType& GetEventType(void) const
	{
		return sk_EventType;
	}

    EvtData_Move_Actor(void)
    {
        m_id = INVALID_ACTOR_ID;
    }

	EvtData_Move_Actor(ActorId id, const Matrix4x4<float>& mat)
        : m_id(id), m_matrix(mat)
	{
        //
	}

	virtual void Serialize(std::ostrstream &out) const
	{
		out << m_id << " ";
		for (int i=0; i<4; ++i)
		{
			for (int j=0; j<4; ++j)
			{
				out << m_matrix(i,j) << " ";
			}
		}
	}

    virtual void Deserialize(std::istrstream& in)
    {
        in >> m_id;
        for (int i=0; i<4; ++i)
        {
            for (int j=0; j<4; ++j)
            {
                in >> m_matrix(i,j);
            }
        }
    }

	virtual BaseEventDataPtr Copy() const
	{
		return BaseEventDataPtr(new EvtData_Move_Actor(m_id, m_matrix));
	}

    virtual const char* GetName(void) const
    {
        return "EvtData_Move_Actor";
    }

    ActorId GetId(void) const
    {
        return m_id;
    }

    const Matrix4x4<float>& GetMatrix(void) const
    {
        return m_matrix;
    }
};


//---------------------------------------------------------------------------------------------------------------------
// EvtData_New_Render_Component - This event is sent out when an actor is *actually* created.
//---------------------------------------------------------------------------------------------------------------------
/*
class EvtData_New_Render_Component : public EventData
{
    ActorId m_actorId;
    eastl::shared_ptr<SceneNode> m_pSceneNode;

public:
    static const Matrix4x4<float> sk_EventType;

    EvtData_New_Render_Component(void) 
    {
        m_actorId = INVALID_ACTOR_ID;
    }

    explicit EvtData_New_Render_Component(ActorId actorId, const eastl::shared_ptr<SceneNode>& pSceneNode) 
        : m_actorId(actorId),
          m_pSceneNode(pSceneNode)
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
        return sk_EventType;
    }

    virtual BaseEventDataPtr Copy(void) const
    {
        return BaseEventDataPtr(new EvtData_New_Render_Component(m_actorId, m_pSceneNode));
    }

    virtual const char* GetName(void) const
    {
        return "EvtData_New_Render_Component";
    }

    const ActorId GetActorId(void) const
    {
        return m_actorId;
    }

    const eastl::shared_ptr<SceneNode>& GetSceneNode(void) const
    {
        return m_pSceneNode;
    }
};
*/

//---------------------------------------------------------------------------------------------------------------------
// EvtData_Modified_Render_Component - This event is sent out when a render component is changed
//   NOTE: This class is not described in the book!
//---------------------------------------------------------------------------------------------------------------------
class EvtData_Modified_Render_Component : public EventData
{
    ActorId m_id;

public:
	static const BaseEventType sk_EventType;

	virtual const BaseEventType& GetEventType(void) const
	{
		return sk_EventType;
	}

    EvtData_Modified_Render_Component(void)
    {
        m_id = INVALID_ACTOR_ID;
    }

	EvtData_Modified_Render_Component(ActorId id)
        : m_id(id)
	{
	}

	virtual void Serialize(std::ostrstream &out) const
	{
		out << m_id;
	}

    virtual void Deserialize(std::istrstream& in)
    {
        in >> m_id;
    }

	virtual BaseEventDataPtr Copy() const
	{
		return BaseEventDataPtr(new EvtData_Modified_Render_Component(m_id));
	}

    virtual const char* GetName(void) const
    {
        return "EvtData_Modified_Render_Component";
    }

    ActorId GetActorId(void) const
    {
        return m_id;
    }
};



//---------------------------------------------------------------------------------------------------------------------
// EvtData_Environment_Loaded - this event is sent when a new game is started
//---------------------------------------------------------------------------------------------------------------------
class EvtData_Environment_Loaded : public EventData
{
public:
	static const BaseEventType sk_EventType;

    EvtData_Environment_Loaded(void) { }
	virtual const BaseEventType& GetEventType(void) const	{ return sk_EventType; }
	virtual BaseEventDataPtr Copy(void) const { return BaseEventDataPtr( new EvtData_Environment_Loaded( ) ); }
    virtual const char* GetName(void) const  { return "EvtData_Environment_Loaded";  }
};


//---------------------------------------------------------------------------------------------------------------------
// EvtData_Environment_Loaded - this event is sent when a client has loaded its environment
//   This is special because we only want this event to go from client to server, and stop there. The
//   EvtData_Environment_Loaded is received by server and proxy logics alike. Thy to do this with just the above 
//   event and you'll get into an endless loop of the EvtData_Environment_Loaded event making infinite round trips
//   from client to server.
//
// FUTURE_WORK: It would be an interesting idea to add a "Private" type of event that is addressed only to a specific 
//              listener. Of course, that might be a really dumb idea too - someone will have to try it!
//---------------------------------------------------------------------------------------------------------------------
class EvtData_Remote_Environment_Loaded : public EventData
{
public:
	static const BaseEventType sk_EventType;

    EvtData_Remote_Environment_Loaded(void) { }
	virtual const BaseEventType& GetEventType(void) const	{ return sk_EventType; }
	virtual BaseEventDataPtr Copy(void) const { return BaseEventDataPtr( new EvtData_Remote_Environment_Loaded( ) ); }
    virtual const char* GetName(void) const  { return "EvtData_Remote_Environment_Loaded";  }
};


//---------------------------------------------------------------------------------------------------------------------
// EvtData_Request_Start_Game - this is sent by the authoritative game logic to all views so they will load a game level.
//---------------------------------------------------------------------------------------------------------------------
class EvtData_Request_Start_Game : public EventData
{

public:
	static const BaseEventType sk_EventType;

    EvtData_Request_Start_Game(void) { }

	virtual const BaseEventType& GetEventType(void) const
	{
		return sk_EventType;
	}

	virtual BaseEventDataPtr Copy() const
	{
		return BaseEventDataPtr( new EvtData_Request_Start_Game( ) );
	}

    virtual const char* GetName(void) const
    {
        return "EvtData_Request_Start_Game";
    }
};

//---------------------------------------------------------------------------------------------------------------------
// EvtData_Remote_Client						- Chapter 19, page 687
// 
//   Sent whenever a new client attaches to a game logic acting as a server				
//---------------------------------------------------------------------------------------------------------------------
class EvtData_Remote_Client : public EventData
{
    int m_socketId;
    int m_ipAddress;

public:
	static const BaseEventType sk_EventType;

    EvtData_Remote_Client(void)
    {
        m_socketId = 0;
        m_ipAddress = 0;
    }

	EvtData_Remote_Client( const int socketid, const int ipaddress )
		: m_socketId( socketid ), m_ipAddress(ipaddress)
	{
	}

    virtual const BaseEventType & GetEventType( void ) const
    {
        return sk_EventType;
    }

	virtual BaseEventDataPtr Copy() const
	{
		return BaseEventDataPtr( new EvtData_Remote_Client( m_socketId, m_ipAddress ) );
	}

    virtual const char* GetName(void) const
    {
        return "EvtData_Remote_Client";
    }

	virtual void Serialize(std::ostrstream &out) const
	{
		out << m_socketId << " ";
		out << m_ipAddress;
	}

    virtual void Deserialize( std::istrstream &in )
    {
        in >> m_socketId;
        in >> m_ipAddress;
    }

    int GetSocketId(void) const
    {
        return m_socketId;
    }

    int GetIpAddress(void) const
    {
        return m_ipAddress;
    }
};


//---------------------------------------------------------------------------------------------------------------------
// EvtData_Update_Tick - sent by the game logic each game tick
//---------------------------------------------------------------------------------------------------------------------
class EvtData_Update_Tick : public EventData
{
    int m_DeltaMilliseconds;

public:
	static const BaseEventType sk_EventType;

    explicit EvtData_Update_Tick( const int deltaMilliseconds )
        : m_DeltaMilliseconds( deltaMilliseconds )
    {
    }

	virtual const BaseEventType& GetEventType( void ) const
	{
		return sk_EventType;
	}

	virtual BaseEventDataPtr Copy() const
	{
		return BaseEventDataPtr (new EvtData_Update_Tick ( m_DeltaMilliseconds ) );
	}

	virtual void Serialize( std::ostrstream & out )
	{
		LogError("You should not be serializing update ticks!");
	}

    virtual const char* GetName(void) const
    {
        return "EvtData_Update_Tick";
    }
};


//---------------------------------------------------------------------------------------------------------------------
// EvtData_Network_Player_Actor_Assignment - sent by the server to the clients when a network view is assigned a player number
//---------------------------------------------------------------------------------------------------------------------
class EvtData_Network_Player_Actor_Assignment : public EventData
{
    ActorId m_ActorId;
    int m_SocketId;

public:
	static const BaseEventType sk_EventType;

	EvtData_Network_Player_Actor_Assignment()
	{
		m_ActorId = INVALID_ACTOR_ID;
		m_SocketId = -1;
	}

    explicit EvtData_Network_Player_Actor_Assignment( const ActorId actorId, const int socketId )
        : m_ActorId( actorId ), m_SocketId(socketId)

    {
    }

	virtual const BaseEventType & GetEventType( void ) const
	{
		return sk_EventType;
	}

	virtual BaseEventDataPtr Copy() const
	{
		 return BaseEventDataPtr( new EvtData_Network_Player_Actor_Assignment( m_ActorId, m_SocketId ) ) ;
	}

    virtual const char* GetName(void) const
    {
        return "EvtData_Network_Player_Actor_Assignment";
    }


	virtual void Serialize(std::ostrstream &out) const
	{
		out << m_ActorId << " ";
		out << m_SocketId;
	}

    virtual void Deserialize( std::istrstream &in )
    {
        in >> m_ActorId;
        in >> m_SocketId;
    }

    ActorId GetActorId(void) const
    {
        return m_ActorId;
    }

    ActorId GetSocketId(void) const
    {
        return m_SocketId;
    }
};


//---------------------------------------------------------------------------------------------------------------------
// EvtData_Decompress_Request - sent to a multithreaded game event listener to decompress something in the resource file
//---------------------------------------------------------------------------------------------------------------------
class EvtData_Decompress_Request : public EventData
{
    eastl::wstring m_zipFileName;
    eastl::string m_fileName;

public:
	static const BaseEventType sk_EventType;

    explicit EvtData_Decompress_Request( eastl::wstring zipFileName, eastl::string filename  )
        : m_zipFileName( zipFileName ),
        m_fileName ( filename )
    {
    }

	virtual const BaseEventType& GetEventType( void ) const
	{
		return sk_EventType;
	}

	virtual BaseEventDataPtr Copy() const
	{
		return BaseEventDataPtr (new EvtData_Decompress_Request ( m_zipFileName, m_fileName ) );
	}

	virtual void Serialize( std::ostrstream & out )
	{
		LogError( "You should not be serializing decompression requests!" );
	}

    const eastl::wstring& GetZipFilename(void) const
    {
        return m_zipFileName;
    }

    const eastl::string& GetFilename(void) const
    {
        return m_fileName;
    }
    virtual const char* GetName(void) const
    {
        return "EvtData_Decompress_Request";
    }
};


//---------------------------------------------------------------------------------------------------------------------
// EvtData_Decompression_Progress - sent by the decompression thread to report progress
//---------------------------------------------------------------------------------------------------------------------
class EvtData_Decompression_Progress : public EventData
{
    int m_progress;
    eastl::wstring m_zipFileName;
    eastl::string m_fileName;
    void *m_buffer;

public:
	static const BaseEventType sk_EventType;

    EvtData_Decompression_Progress( int progress, eastl::wstring zipFileName, eastl::string filename, void *buffer )
        : m_progress(progress),
        m_zipFileName( zipFileName ),
        m_fileName ( filename ),
        m_buffer (buffer)
    {
    }

	virtual const BaseEventType & GetEventType( void ) const
	{
		return sk_EventType;
	}

	virtual BaseEventDataPtr Copy() const
	{
		return BaseEventDataPtr (new EvtData_Decompression_Progress ( m_progress, m_zipFileName, m_fileName, m_buffer ) );
	}

	virtual void Serialize( std::ostrstream & out )
	{
		LogError( "You should not be serializing decompression progress events!" );
	}

    virtual const char* GetName(void) const
    {
        return "EvtData_Decompression_Progress";
    }

};


//---------------------------------------------------------------------------------------------------------------------
// class EvtData_Request_New_Actor				
// This event is sent by a server asking Client proxy logics to create new actors from their local resources.
// It can be sent from script or via code.
// This event is also sent from the server game logic to client logics AFTER it has created a new actor. The logics will allow follow suit to stay in sync.
//---------------------------------------------------------------------------------------------------------------------
class EvtData_Request_New_Actor : public EventData
{
    eastl::string m_actorResource;
	bool m_hasInitialTransform;
	Transform m_initialTransform;
	ActorId m_serverActorId;
    GameViewId m_viewId;

public:
	static const BaseEventType sk_EventType;

	EvtData_Request_New_Actor()
	{
		m_actorResource = "";
		m_hasInitialTransform = false;
		m_initialTransform.MakeIdentity();
		m_serverActorId = -1;
		m_viewId = InvalidGameViewId;
	}

    explicit EvtData_Request_New_Actor(const eastl::string &actorResource, const Transform *initialTransform=NULL, 
		const ActorId serverActorId = INVALID_ACTOR_ID, const GameViewId viewId = InvalidGameViewId)
    {
        m_actorResource = actorResource;
		if (initialTransform)
		{
			m_hasInitialTransform = true;
			m_initialTransform = *initialTransform;
		}
		else
			m_hasInitialTransform = false;

		m_serverActorId = serverActorId;
        m_viewId = viewId;
    }

	virtual const BaseEventType& GetEventType(void) const
	{
		return sk_EventType;
	}

	virtual void Deserialize( std::istrstream & in )
	{
		std::string actorResource;
		in >> actorResource;
		m_actorResource = eastl::string(actorResource.c_str());
		in >> m_hasInitialTransform;
		if (m_hasInitialTransform)
		{
			Matrix4x4<float> transform = m_initialTransform.GetMatrix();
			for (int i=0; i<4; ++i)
				for (int j=0; j<4; ++j)
					in >> transform(i,j);
		}
		in >> m_serverActorId;
		in >> m_viewId;
	}

	virtual BaseEventDataPtr Copy() const
	{	 
		return BaseEventDataPtr (new EvtData_Request_New_Actor(
			m_actorResource, (m_hasInitialTransform) ? &m_initialTransform : NULL, m_serverActorId, m_viewId));
	}

	virtual void Serialize( std::ostrstream & out ) const
	{
		out << std::string(m_actorResource.c_str()) << " ";
		out << m_hasInitialTransform << " ";
		if (m_hasInitialTransform)
		{
			Matrix4x4<float> transform = m_initialTransform.GetMatrix();
			for (int i=0; i<4; ++i)
				for (int j=0; j<4; ++j)
					out << transform(i,j) << " ";
		}
		out << m_serverActorId << " ";
		out << m_viewId << " ";
	}

    virtual const char* GetName(void) const { return "EvtData_Request_New_Actor";  }

    const eastl::string &GetActorResource(void) const { return m_actorResource;  }
	const Transform *GetInitialTransform(void) const { return (m_hasInitialTransform) ? &m_initialTransform : NULL; }
	const ActorId GetServerActorId(void) const 	{ return m_serverActorId; }
    GameViewId GetViewId(void) const { return m_viewId; }
};


//---------------------------------------------------------------------------------------------------------------------
// EvtData_Request_Destroy_Actor - sent by any system requesting that the game logic destroy an actor	
//---------------------------------------------------------------------------------------------------------------------
class EvtData_Request_Destroy_Actor : public EventData
{
    ActorId m_actorId;

public:
    static const BaseEventType sk_EventType;

    EvtData_Request_Destroy_Actor()
    {
        m_actorId = INVALID_ACTOR_ID;
    }

    EvtData_Request_Destroy_Actor(ActorId actorId)
    {
        m_actorId = actorId;
    }

    virtual const BaseEventType& GetEventType(void) const
    {
        return sk_EventType;
    }

    virtual void Deserialize( std::istrstream & in )
    {
        in >> m_actorId;
    }

    virtual BaseEventDataPtr Copy() const
    {
        return BaseEventDataPtr (new EvtData_Request_Destroy_Actor(m_actorId));
    }

    virtual void Serialize( std::ostrstream & out ) const
    {
        out << m_actorId;
    }

    virtual const char* GetName(void) const
    {
        return "EvtData_Request_Destroy_Actor";
    }

    ActorId GetActorId(void) const
    {
        return m_actorId;
    }
};


//---------------------------------------------------------------------------------------------------------------------
// EvtData_PlaySound - sent by any system wishing for a HumanView to play a sound
//---------------------------------------------------------------------------------------------------------------------
class EvtData_PlaySound : public EventData
{
    eastl::string m_soundResource;

public:
    static const BaseEventType sk_EventType;

    EvtData_PlaySound(void) { }
    EvtData_PlaySound(const eastl::string& soundResource)
        : m_soundResource(soundResource)
    {
    }

    virtual const BaseEventType& GetEventType(void) const
    {
        return sk_EventType;
    }

    virtual BaseEventDataPtr Copy() const
    {
        return BaseEventDataPtr(new EvtData_PlaySound(m_soundResource));
    }

    virtual void Serialize(std::ostrstream& out) const
    {
        out << std::string(m_soundResource.c_str());
    }

    virtual void Deserialize(std::istrstream& in)
    {
		std::string soundResource;
        in >> soundResource;
		m_soundResource = eastl::string(soundResource.c_str());
    }

    const eastl::string& GetResource(void) const
    {
        return m_soundResource;
    }

    virtual const char* GetName(void) const
    {
        return "EvtData_PlaySound";
    }
};


#endif