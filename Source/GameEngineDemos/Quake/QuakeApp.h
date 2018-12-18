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

#ifndef QUAKEAPP_H
#define QUAKEAPP_H

#include "Application/GameApplication.h"

//---------------------------------------------------------------------------------------------------------------------
// QuakeApp class                          - Chapter 21, page 722
// Note: Don't put anything in this class that needs to be destructed
//---------------------------------------------------------------------------------------------------------------------
class QuakeApp : public GameApplication
{
public:
	// Abstract base class.
	QuakeApp();
	virtual ~QuakeApp();

	virtual void CreateGameAndView();

	virtual void AddView(const eastl::shared_ptr<BaseGameView>& pView, ActorId actorId = INVALID_ACTOR_ID);
	virtual void RemoveView(const eastl::shared_ptr<BaseGameView>& pView);
	virtual void RemoveViews();
	virtual void RemoveView();

	virtual eastl::wstring GetGameTitle() { return L"Game Quake"; }
	virtual eastl::wstring GetGameAppDirectory() { return L"GameEngineQuake\\Quake"; }
	//virtual HICON GetIcon();

protected:
    virtual void RegisterGameEvents(void);
    virtual void CreateNetworkEventForwarder(void);
    virtual void DestroyNetworkEventForwarder(void);
};

#endif