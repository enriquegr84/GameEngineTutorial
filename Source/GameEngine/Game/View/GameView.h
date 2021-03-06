//========================================================================
// GameView.h : Defines the GameView class of the GameEngine application
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

#ifndef GAMEVIEW_H
#define GAMEVIEW_H

#include "GameEngineStd.h"

#include "Application/System/EventSystem.h"

class BaseGameView
{
public:
	virtual bool OnRestore() = 0;
	virtual void OnRender(double time, float elapsedTime) = 0;
	virtual void OnUpdate(unsigned int timeMs, unsigned long deltaMs) = 0;

	virtual bool OnLostDevice() = 0;
	virtual GameViewType GetType() = 0;
	virtual GameViewId GetId() const = 0;
	virtual ActorId GetActorId() const = 0;
	virtual void OnAttach(GameViewId vid, ActorId aid) = 0;

	virtual bool OnMsgProc(const Event& event) = 0;

	virtual ~BaseGameView() { };
};

typedef eastl::list<eastl::shared_ptr<BaseGameView>> GameViewList;

#endif