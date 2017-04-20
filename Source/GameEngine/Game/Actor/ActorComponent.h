//========================================================================
// ActorComponent.h - Defines the base class for a component that can be attached to an actor
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

#ifndef ACTORCOMPONENT_H
#define ACTORCOMPONENT_H

#include "GameEngineStd.h"

#include "Core/Utility/StringUtil.h"

/*
	class ActorComponent describes any actor in the game. Components is the chosen architecture
	to define actors since they encapsulate specific features and are flexible enough to make 
	changes during the course of the game. This is the base class that the actor maintains a
	reference to and establishes a common interfaces for each component such as unique
	identified. Each component will have a particular responsability and also subclasses
	which can change actor's behavior at runtime
*/
class ActorComponent
{
	friend class ActorFactory;

protected:
	// It is important to have direct control over lifetime of actors. We need the ability to destroy
	// the actor objects at any time to free up memory
	eastl::shared_ptr<Actor> mOwner;

public:
	virtual ~ActorComponent(void) { mOwner.reset(); }

    // These functions are meant to be overridden by the implementation classes of the components.
	virtual bool Init(XMLElement* pData) = 0;
	virtual void PostInit(void) { }
	virtual void Update(int deltaMs) { }
	virtual void OnChanged(void) { }				// [mrmike] - added post-press

    // for the editor
    virtual XMLElement* GenerateXml(void) = 0;

    // This function should be overridden by the interface class.
	virtual ComponentId GetId(void) const { return GetIdFromName(GetName()); }
	virtual const char *GetName() const = 0;
    static ComponentId GetIdFromName(const char* componentStr)
	{
		return HashedString(componentStr).GetHashValue();
	}

private:
	void SetOwner(eastl::shared_ptr<Actor> pOwner) { mOwner = pOwner; }

};


#endif