//========================================================================
// ActorFactory.h - Defines a factory for creating actors & components
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

#ifndef ACTORFACTORY_H
#define ACTORFACTORY_H

#include "GameEngineStd.h"

#include "Actor.h"

#include "Mathematic/Algebra/Transform.h"

/*
	Class ActorFactory. All actors are created using a factory. The factory's job is to
	take an XML resource, parse it, and return a fully initialized actor complete with
	all the appropriate components. It's important to understand how actors are built, 
	how to define a component configuration and any default values for that component.
*/
class ActorFactory
{
protected:
    GenericObjectFactory<ActorComponent, ComponentId> mComponentFactory;

public:
    ActorFactory(void);

    eastl::shared_ptr<Actor> CreateActor(
		const wchar_t* actorResource, tinyxml2::XMLElement* overrides,
		const Transform* initialTransform, const ActorId serversActorId);
	void ModifyActor(eastl::shared_ptr<Actor> pActor, tinyxml2::XMLElement* overrides);

//protected:
    // This function can be overridden by a subclass so you can create game-specific 
	// C++ components. If you do this, make sure you call the base-class version first.  
	// If it returns NULL, you know it's not an engine component.
    virtual eastl::shared_ptr<ActorComponent> CreateComponent(eastl::shared_ptr<Actor> pActor, tinyxml2::XMLElement* pData);
};


#endif
