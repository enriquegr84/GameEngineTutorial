//========================================================================
// AmmoPickup.h - An ammo pickup
//
// Part of the GameCode4 Application
//
// GameCode4 is the sample application that encapsulates much of the source code
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
//    http://code.google.com/p/gamecode4/
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

#ifndef AMMOPICKUP_H
#define AMMOPICKUP_H

#include "BasePickup.h"

//---------------------------------------------------------------------------------------------------------------------
// AmmoPickup implementation class.
//---------------------------------------------------------------------------------------------------------------------
class AmmoPickup : public BasePickup
{
	unsigned int mCode;
	unsigned int mType;

	unsigned int mWait;
	unsigned int mAmount;

public:
	float mRespawnTime;

	static const char* Name;
	virtual const char* GetName() const { return Name; }
	unsigned int GetCode(void) const { return mCode; }
	unsigned int GetType() const { return mType; }

	unsigned int GetWait() const { return mWait; }
	unsigned int GetAmount() const { return mAmount; }

	AmmoPickup(void);

	virtual bool Init(tinyxml2::XMLElement* pData) override;
	virtual tinyxml2::XMLElement* GenerateXml(void) override;
	virtual void Apply(eastl::weak_ptr<Actor> pActor) override;
	virtual void Update(float deltaMs) override;
};

#endif