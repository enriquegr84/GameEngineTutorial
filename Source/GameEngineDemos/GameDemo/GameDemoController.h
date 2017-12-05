//========================================================================
// File: DemoController.h
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

#ifndef GAMEDEMOCONTROLLER_H
#define GAMEDEMOCONTROLLER_H

#include "GameEngineStd.h"

#include "Application/System/EventSystem.h"

class Node;

class GameDemoController : public BaseMouseHandler, public BaseKeyboardHandler
{
protected:
	BYTE mKey[256];			// Which keys are up and down
	eastl::shared_ptr<Node> mObject;

public:
	GameDemoController(const eastl::shared_ptr<Node>& object);
	//void SetObject(const eastl::shared_ptr<SceneNode>& newObject);

	void OnUpdate(unsigned long const elapsedTime);

public:
	bool OnMouseMove(const Vector2<int> &mousePos, const int radius) { return true; }
	bool OnMouseButtonDown(const Vector2<int> &mousePos, const int radius, const eastl::string &buttonName);
	bool OnMouseButtonUp(const Vector2<int> &mousePos, const int radius, const eastl::string &buttonName)
	{ return (buttonName == "PointerLeft"); }

	bool OnKeyDown(const KeyCode c);
	bool OnKeyUp(const KeyCode c);

	bool OnWheelRollDown() { return true; }
	bool OnWheelRollUp() { return true; }

};

#endif