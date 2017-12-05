// DemoController.cpp - Controller class for the player teapots
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

#include "GameDemoStd.h"

#include "Core/Logger/Logger.h"

#include "Core/Event/EventManager.h"

#include "Physic/PhysicEventListener.h"

#include "GameDemoController.h"
#include "GameDemoEvents.h"

const float ACTOR_ACCELERATION = 6.5f * 8.0f;
const float ACTOR_ANGULAR_ACCELERATION = 22.0f;


////////////////////////////////////////////////////
// GameDemoController Implementation
////////////////////////////////////////////////////


//
// GameDemoController::GameDemoController
//
GameDemoController::GameDemoController(const eastl::shared_ptr<Node>& object)
: mObject(object)
{
	memset(mKey, 0x00, sizeof(mKey));
}

//
// DemoController::OnMouseButtonDown				- Chapter 19, page 735
//
bool GameDemoController::OnMouseButtonDown(const Vector2<int> &mousePos, const int radius, const eastl::string &buttonName)
{
	if (buttonName != "PointerLeft")
		return false;
	ActorId actorId;// = mObject->Get()->ActorId();
	LogAssert(actorId != INVALID_ACTOR_ID, "The teapot controller isn't attached to a valid actor!");
    eastl::shared_ptr<EventDataFireWeapon> pFireEvent(new EventDataFireWeapon(actorId));
    BaseEventManager::Get()->QueueEvent(pFireEvent);
	return true;
}

//
// DemoController::OnUpdate				- Chapter 19, page 736
//
void GameDemoController::OnUpdate(unsigned long const deltaTime)  
{
    //
}

bool GameDemoController::OnKeyDown(const KeyCode c)
{
    // update the key table
    mKey[c] = true;

    // send a thrust event if necessary
    if (mKey[KEY_KEY_W] || mKey[KEY_KEY_S])
    {
		const ActorId actorId = INVALID_ACTOR_ID;// = mObject->Get()->ActorId();
        eastl::shared_ptr<EventDataStartThrust> pEvent(new EventDataStartThrust(
			actorId, (c == 'W' ? ACTOR_ACCELERATION : (-ACTOR_ACCELERATION))));
        EventManager::Get()->QueueEvent(pEvent);
    }

    // send a steer event if necessary
    if (mKey[KEY_KEY_A] || mKey[KEY_KEY_D])
    {
		const ActorId actorId = INVALID_ACTOR_ID;// = mObject->Get()->ActorId();
        eastl::shared_ptr<EventDataStartSteer> pEvent(new EventDataStartSteer(
			actorId, (c == 'D' ? ACTOR_ANGULAR_ACCELERATION : (-ACTOR_ANGULAR_ACCELERATION))));
        EventManager::Get()->QueueEvent(pEvent);
    }

    return true;
}

bool GameDemoController::OnKeyUp(const KeyCode c)
{
    // update the key table
    mKey[c] = false;

    // send an end thrust event if necessary
    if (mKey[KEY_KEY_W] || mKey[KEY_KEY_S])
    {
		const ActorId actorId = INVALID_ACTOR_ID;// = mObject->Get()->ActorId();
        eastl::shared_ptr<EventDataEndThrust> pEvent(new EventDataEndThrust(actorId));
        EventManager::Get()->QueueEvent(pEvent);
    }

    // send an end steer event if necessary
    if (mKey[KEY_KEY_A] || mKey[KEY_KEY_D])
    {
		const ActorId actorId = INVALID_ACTOR_ID;// = mObject->Get()->ActorId();
        eastl::shared_ptr<EventDataEndSteer> pEvent(new EventDataEndSteer(actorId));
        EventManager::Get()->QueueEvent(pEvent);
    }

    return true;
}
