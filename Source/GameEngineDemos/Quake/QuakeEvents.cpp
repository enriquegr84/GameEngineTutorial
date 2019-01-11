//========================================================================
// QuakeEvents.cpp : defines game-specific events for Quake
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

#include "QuakeStd.h"
#include "QuakeEvents.h"

const BaseEventType QuakeEventDataFireWeapon::skEventType(0x1b15b6a7);
const BaseEventType QuakeEventDataJumpActor::skEventType(0xfeee009e);
const BaseEventType QuakeEventDataMoveActor::skEventType(0xeeaa0a40);
const BaseEventType QuakeEventDataFallActor::skEventType(0x47d33df3);
const BaseEventType QuakeEventDataRotateActor::skEventType(0xed6973fe);
const BaseEventType QuakeEventDataStartThrust::skEventType(0x1d62d48c);
const BaseEventType QuakeEventDataEndThrust::skEventType(0xe60f88a4);
const BaseEventType QuakeEventDataStartSteer::skEventType(0xf0b5b4fd);
const BaseEventType QuakeEventDataEndSteer::skEventType(0x176645ef);
const BaseEventType QuakeEventDataGameplayUIUpdate::skEventType(0x1002ded2);
const BaseEventType QuakeEventDataSetControlledActor::skEventType(0xbe5e3388);
