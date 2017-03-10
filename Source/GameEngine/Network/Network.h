//========================================================================
//
// Network.h - the core classes for creating a multiplayer game
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

#ifndef NETWORK_H
#define NETWORK_H

#include "GameEngineStd.h"

#include "Game/Game.h"
#include "Core/Event/EventManager.h"

#include <sys/types.h>
#include <Winsock2.h>

#define MAX_PACKET_SIZE (256)
#define RECV_BUFFER_SIZE (MAX_PACKET_SIZE * 512)
#define MAX_QUEUE_PER_PLAYER (10000)

#define MAGIC_NUMBER		(0x1f2e3d4c)
#define IPMANGLE(a,b,c,d) (((a)<<24)|((b)<<16)|((c)<<8)|((d)))
#define INVALID_SOCKET_ID (-1)

class NetSocket;

////////////////////////////////////////////////////
//
// BasePacket Description
//
//    The interface class that defines a public API for 
//    packet objects - data that is either about to be
//    sent to or just been recieved from the network
//
////////////////////////////////////////////////////

class BasePacket
{
public:
	virtual char const * const GetType() const=0;
	virtual char const * const GetData() const=0;
	virtual u_long GetSize() const =0;
	virtual ~BasePacket() { }
};

////////////////////////////////////////////////////
// class BinaryPacket							- Chapter 19, page 665
//
//   A packet object that can be constructed all at once,
//   or with repeated calls to MemCpy
////////////////////////////////////////////////////

class BinaryPacket : public BasePacket
{
protected:
	char *m_Data;

public:
	inline BinaryPacket(char const * const data, u_long size);
	inline BinaryPacket(u_long size);
	virtual ~BinaryPacket() { delete m_Data; }
	virtual char const * const GetType() const { return g_Type; }
	virtual char const * const GetData() const { return m_Data; }
	virtual u_long GetSize() const { return ntohl(*(u_long *)m_Data); }
	inline void MemCpy(char const *const data, size_t size, int destOffset);

	static const char *g_Type;
};

///////////////////////////////////////////////////////////////////////////////
// BinaryPacket::BinaryPacket					- Chapter 19, page 666
//
inline BinaryPacket::BinaryPacket(char const * const data, u_long size)
{
	m_Data = new char[size + sizeof(u_long)];
	LogAssert(m_Data);
	*(u_long *)m_Data = htonl(size+sizeof(u_long));
	memcpy(m_Data+sizeof(u_long), data, size);
}

inline BinaryPacket::BinaryPacket(u_long size)
{
	m_Data = new char[size + sizeof(u_long)];
	LogAssert(m_Data);
	*(u_long *)m_Data = htonl(size+sizeof(u_long));
}

///////////////////////////////////////////////////////////////////////////////
// BinaryPacket::MemCpy							- Chapter 19, page 666
//
inline void BinaryPacket::MemCpy(char const *const data, size_t size, int destOffset)
{
	LogAssert(size+destOffset <= GetSize()-sizeof(u_long));
	memcpy(m_Data + destOffset + sizeof(u_long), data, size);
}

////////////////////////////////////////////////////
// TextPacket Description						- not described in the book
//
//   A packet object that takes a text string.
////////////////////////////////////////////////////

class TextPacket : public BinaryPacket
{
public:
	TextPacket(char const * const text);
	virtual char const * const GetType() const { return g_Type; }

	static const char *g_Type;
};


///////////////////////////////////////////////////////////////////////////////
// NetSocket Description						- Chapter 19, page 666
//
//   A base class for a socket connection.
////////////////////////////////////////////////////

class NetSocket 
{
	friend class BaseSocketManager;
	typedef eastl::list<eastl::shared_ptr<BasePacket>> PacketList;

public:
	NetSocket();											// clients use this to initialize a NetSocket prior to calling Connect.
	NetSocket(SOCKET new_sock, unsigned int hostIP);		// servers use this when new clients attach.
	virtual ~NetSocket();

	bool Connect(unsigned int ip, unsigned int port, bool forceCoalesce = 0);
	void SetBlocking(bool blocking);
	void Send(eastl::shared_ptr<BasePacket> pkt, bool clearTimeOut=1);

	virtual int  HasOutput() { return !m_OutList.empty(); }
	virtual void HandleOutput();
	virtual void HandleInput();
	virtual void TimeOut() { m_timeOut=0; }

	void HandleException() { m_deleteFlag |= 1; }

	void SetTimeOut(unsigned int ms=45*1000) { m_timeOut = g_pGameApp->GetTime() + ms; }

	int GetIpAddress() { return m_ipaddr; }

protected:
    SOCKET m_sock;
	int m_id;				// a unique ID given by the socket manager

	// note: if deleteFlag has bit 2 set, exceptions only close the
	//   socket and set to INVALID_SOCKET, and do not delete the NetSocket
    int m_deleteFlag;
 
	PacketList m_OutList;
	PacketList m_InList;

	char m_recvBuf[RECV_BUFFER_SIZE];
	unsigned int m_recvOfs, m_recvBegin;
	bool m_bBinaryProtocol;

    int m_sendOfs;
	unsigned int m_timeOut;
	unsigned int m_ipaddr;

	int m_internal;
	int m_timeCreated;
};


//
// class BaseSocketManager						- Chapter 19, page 676
//
class BaseSocketManager
{
protected:
	WSADATA m_WsaData;

	typedef eastl::list<NetSocket *> SocketList;
	typedef eastl::map<int, NetSocket *> SocketIdMap;

	SocketList m_SockList;
	SocketIdMap m_SockMap;

	int m_NextSocketId;
	unsigned int m_Inbound;
	unsigned int m_Outbound;
	unsigned int m_MaxOpenSockets;
	unsigned int m_SubnetMask;
	unsigned int m_Subnet;

	NetSocket *FindSocket(int sockId);

public:
	BaseSocketManager();
	virtual ~BaseSocketManager() { Shutdown(); }

	void DoSelect(int pauseMicroSecs, bool handleInput = true);

	bool Init();
	void Shutdown();
	void PrintError();

	int AddSocket(NetSocket *socket); 
	void RemoveSocket(NetSocket *socket);

	unsigned int GetHostByName(const eastl::string &hostName);
	const char *GetHostByAddr(unsigned int ip);

	int GetIpAddress(int sockId);

	void SetSubnet(unsigned int subnet, unsigned int subnetMask)
	{
		m_Subnet = subnet;
		m_SubnetMask = subnetMask;
	}
	bool IsInternal(unsigned int ipaddr);

	bool Send(int sockId, eastl::shared_ptr<BasePacket> packet);

	void AddToOutbound(int rc) { m_Outbound += rc; }
	void AddToInbound(int rc) { m_Inbound += rc; }

};

extern BaseSocketManager *g_pSocketManager;


//
// class ClientSocketManager					- Chapter 19, page 684
//
class ClientSocketManager : public BaseSocketManager
{
	eastl::string m_HostName;
	unsigned int m_Port;

public:
	ClientSocketManager(const eastl::string &hostName, unsigned int port)
	{
		m_HostName = hostName;
		m_Port = port;
	}

	bool Connect();
};





//
// class NetListenSocket						- Chapter 19, page 673
//
class NetListenSocket: public NetSocket
{
public:
	NetListenSocket() { };
	NetListenSocket(int portnum);

	void Init(int portnum);
	void InitScan(int portnum_min, int portnum_max);
	SOCKET AcceptConnection(unsigned int *pAddr);

	unsigned short port;
};


//
// class GameServerListenSocket					- Chapter 19, page 685
//
class GameServerListenSocket: public NetListenSocket 
{
public:
	GameServerListenSocket(int portnum) { Init(portnum); }

	virtual void HandleInput();
};

//
// class RemoteEventSocket						- Chapter 19, page 688
//
class RemoteEventSocket: public NetSocket 
{
public:
	enum
	{
		NetMsg_Event,
		NetMsg_PlayerLoginOk,
	};

	// server accepting a client
	RemoteEventSocket(SOCKET new_sock, unsigned int hostIP)		
	: NetSocket(new_sock, hostIP)
	{
	}

	// client attach to server
	RemoteEventSocket() { };										

	virtual void HandleInput();

protected:
	void CreateEvent(std::istrstream &in);
};



//
// class NetworkEventForwarder					- Chapter 19, page 690
//
// The only thing a network game view does is 
// act as an intermediary between the server and the 'real' remote view
// it listens to the same messages as a a game view
// and sends them along via TCP/IP

class NetworkEventForwarder
{
public:
	NetworkEventForwarder(int sockId) { m_SockId = sockId;  }

    // Delegate that forwards events through the network.  The game layer must register objects of this class for 
    // the events it wants.  See TeapotWarsApp::VCreateGameAndView() and TeapotWarsLogic::RemoteClientDelegate()
    // for examples of this happening.
    void ForwardEvent(BaseEventDataPtr pEventData);

protected:
	int m_SockId;
};


//
// class NetworkGameView						- Chapter 19, page 691
//
class NetworkGameView : public BaseGameView
{

public:
	// BaseGameView Implementation - everything is stubbed out.
	virtual bool OnRestore() { return true; }
	virtual void OnRender(double fTime, float fElapsedTime) { }
	virtual void OnAnimate(unsigned int uTime) { }
	virtual bool OnLostDevice() { return true; }
	virtual GameViewType GetType() { return GameView_Remote; }
	virtual GameViewId GetId() const { return m_ViewId; }
	virtual void OnAttach(GameViewId vid, ActorId aid);
	virtual bool OnMsgProc( const Event& event ) { return false; }
	virtual void OnUpdate(unsigned long deltaMs);

	void NewActorDelegate(BaseEventDataPtr pEventData);

	void SetPlayerActorId(ActorId actorId) { m_ActorId = actorId; }
	void AttachRemotePlayer(int sockID);

	int HasRemotePlayerAttached() { return m_SockId != INVALID_SOCKET_ID; }

	NetworkGameView();

protected:
	GameViewId m_ViewId;
	ActorId m_ActorId;
	int m_SockId;
};

#endif






