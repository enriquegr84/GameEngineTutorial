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

#ifndef SCREENELEMENT_H
#define SCREENELEMENT_H

#include "GameEngineStd.h"

#include "Application/System/EventSystem.h"

/*
	A screen element is anything that draws and accepts input. It could be anything from a UI
	to your rendered 3D world. Screen elements can be hierarchical, for example, a dialog box
	can have buttons attached to it. Screen elements in various configurations create the user
	interface for the game, such as a menu, inventory screen. Some run on top of the main game
	screen, but others might completely overlay the main view and even pause the game. In
	addition to acting as a container for controls, screens parse user input messages from the
	application layer and translate them into game messages.

	The BaseScreenElement interface shows that a screen knows how to restore itself when it 
	needs to be rebuilt, render itself when it's time to draw, how it should be ordered in the
	master draw list, and whether it is visible. The OnMsgProce method accepts messages from
	the application layer, but translates them into a structure to simplify the call signature
	of anything that will accept these messages.
*/
class BaseScreenElement
{
public:
	virtual bool OnInit() = 0;
	virtual bool OnRestore() = 0;
	virtual bool OnLostDevice() = 0;

	virtual bool OnRender(double time, float elapsedTime) = 0;
	virtual void OnUpdate(unsigned int timeMs, unsigned long deltaMs) = 0;

	virtual int GetZOrder() const = 0;
	virtual void SetZOrder(int const zOrder) = 0;
	virtual bool IsVisible() = 0;
	virtual void SetVisible(bool visible) = 0;

	virtual bool OnMsgProc(const Event& event) = 0;

	virtual ~BaseScreenElement() { };
	virtual bool const operator <(BaseScreenElement const &other) 
	{ return GetZOrder() < other.GetZOrder(); }
};

#endif