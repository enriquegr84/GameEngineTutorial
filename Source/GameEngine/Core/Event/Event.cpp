//========================================================================
// Events.cpp : defines common game events
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
//    http://code.google.com/p/GameEngine/6
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

 

#include "Event.h"
#include "Physic/PhysicEventListener.h"

// To define a new event - you need a 32-bit UID. 
// In Visual Studio, go to Tools->Create UID and grab the first bit.

const BaseEventType EvtData_Environment_Loaded::sk_EventType(0xa3814acd);
const BaseEventType EvtData_Remote_Environment_Loaded::sk_EventType(0x8E2AD6E6);
const BaseEventType EvtData_New_Actor::sk_EventType(0xe86c7c31);
const BaseEventType EvtData_Move_Actor::sk_EventType(0xeeaa0a40);
const BaseEventType EvtData_Destroy_Actor::sk_EventType(0x77dd2b3a);
//const BaseEventType EvtData_New_Render_Component::sk_EventType(0xaf4aff75);
const BaseEventType EvtData_Modified_Render_Component::sk_EventType(0x80fe9766);
const BaseEventType EvtData_Request_Start_Game::sk_EventType(0x11f2b19d);
const BaseEventType EvtData_Remote_Client::sk_EventType(0x301693d5);
const BaseEventType EvtData_Network_Player_Actor_Assignment::sk_EventType(0xa7c92f11);
const BaseEventType EvtData_Update_Tick::sk_EventType(0xf0f5d183);
const BaseEventType EvtData_Decompress_Request::sk_EventType(0xc128a129);
const BaseEventType EvtData_Decompression_Progress::sk_EventType(0x68de1f28);
const BaseEventType EvtData_Request_New_Actor::sk_EventType(0x40378c64);
const BaseEventType EvtData_Request_Destroy_Actor::sk_EventType(0xf5395770);
const BaseEventType EvtData_PlaySound::sk_EventType(0x3d8118ee);