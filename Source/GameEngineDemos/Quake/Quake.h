//========================================================================
// Quake.h : source file for the sample game
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

#ifndef QUAKE_H
#define QUAKE_H

#include "Game/Game.h"

class BaseEventManager;
class NetworkEventForwarder;

//---------------------------------------------------------------------------------------------------------------------
// QuakeLogic class                        - Chapter 21, page 723
//---------------------------------------------------------------------------------------------------------------------
class QuakeLogic : public GameLogic
{
protected:
	eastl::list<NetworkEventForwarder*> mNetworkEventForwarders;

public:
	QuakeLogic();
	virtual ~QuakeLogic();

	// Quake Methods

	// Update
	virtual void SetProxy();

	virtual void SyncActor(const ActorId id, Transform const &transform);

	virtual void ResetViewType();
	virtual void UpdateViewType(const eastl::shared_ptr<BaseGameView>& pView, bool add);

	// Overloads
	virtual void ChangeState(BaseGameState newState);
	virtual eastl::shared_ptr<BaseGamePhysic> GetGamePhysics(void) { return mPhysics; }

	// event delegates
	void RequestStartGameDelegate(BaseEventDataPtr pEventData);
	void RemoteClientDelegate(BaseEventDataPtr pEventData);
	void NetworkPlayerActorAssignmentDelegate(BaseEventDataPtr pEventData);
	void EnvironmentLoadedDelegate(BaseEventDataPtr pEventData);
	void JumpActorDelegate(BaseEventDataPtr pEventData);
	void MoveActorDelegate(BaseEventDataPtr pEventData);
	void RotateActorDelegate(BaseEventDataPtr pEventData);
	void StartThrustDelegate(BaseEventDataPtr pEventData);
	void EndThrustDelegate(BaseEventDataPtr pEventData);
	void StartSteerDelegate(BaseEventDataPtr pEventData);
	void EndSteerDelegate(BaseEventDataPtr pEventData);

protected:

	virtual ActorFactory* CreateActorFactory(void);

	virtual bool LoadGameDelegate(tinyxml2::XMLElement* pLevelData);

private:
	void RegisterAllDelegates(void);
	void RemoveAllDelegates(void);
	void CreateNetworkEventForwarder(const int socketId);
	void DestroyAllNetworkEventForwarders(void);
};

#endif