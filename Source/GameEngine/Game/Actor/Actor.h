//========================================================================
// Actor.h - Defines the Actor class
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

#ifndef ACTOR_H
#define ACTOR_H

#include "Game/GameStd.h"

typedef eastl::string ActorType;

/*
	Actor class. A game actor is an object that represents a single entity in your game world.
	Its entiry purpose is to manage and mantain components to make them as flexible and reusable 
	as possible.
*/
class Actor
{
    friend class ActorFactory;

public:

    typedef eastl::map<ComponentId, eastl::shared_ptr<ActorComponent>> ActorComponents;

private:
    ActorId mID;					// unique id for the actor
    ActorType mType;

	// all components this actor has
	ActorComponents mComponents;

	// [mrmike] - these were added post press as editor helpers, 
	// but will also be great for save game files if we ever make them.
	// the XML file from which this actor was initialized (considered the "Archetype" file)
	eastl::string mResource;

public:
    explicit Actor(ActorId id);
    ~Actor(void);

    bool Init(XMLElement* pData);
    void PostInit(void);
    void Destroy(void);
    void Update(int deltaMs);

    // editor functions
	//[mrmike] - we can delete this.
    //bool SaveActorFromEditor(const char* path);
	eastl::string Actor::ToXML();

    // accessors
    ActorId GetId(void) const { return mID; }
    ActorType GetType(void) const { return mType; }

    // template function for retrieving components.
    template <class ComponentType>
    eastl::weak_ptr<ComponentType> GetComponent(ComponentId id)
    {
        ActorComponents::iterator findIt = mComponents.find(id);
        if (findIt != mComponents.end())
        {
            eastl::shared_ptr<ActorComponent> pBase(findIt->second);
			// cast to subclass version of the pointer
            eastl::shared_ptr<ComponentType> pSub(
				eastl::static_pointer_cast<ComponentType>(pBase));
			// convert strong pointer to weak pointer
            eastl::weak_ptr<ComponentType> pWeakSub(pSub);
            return pWeakSub;  // return the weak pointer
        }
        else return eastl::weak_ptr<ComponentType>();
    }

    template <class ComponentType>
    eastl::weak_ptr<ComponentType> GetComponent(const char *name)
    {
		ComponentId id = ActorComponent::GetIdFromName(name);
        ActorComponents::iterator findIt = mComponents.find(id);
        if (findIt != mComponents.end())
        {
            eastl::shared_ptr<ActorComponent> pBase(findIt->second);
			// cast to subclass version of the pointer
            eastl::shared_ptr<ComponentType> pSub(
				eastl::static_pointer_cast<ComponentType>(pBase));
			// convert strong pointer to weak pointer
            eastl::weak_ptr<ComponentType> pWeakSub(pSub);  
            return pWeakSub;  // return the weak pointer
        }
        else return eastl::weak_ptr<ComponentType>();
    }

	const ActorComponents* GetComponents() { return &mComponents; }

    void AddComponent(eastl::shared_ptr<ActorComponent> pComponent);
};

#endif