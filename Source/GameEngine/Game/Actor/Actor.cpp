//========================================================================
// Actor.cpp - Implements the Actor class
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

#include "GameEngineStd.h"

#include "Actor.h"
#include "ActorComponent.h"

#include "Core/Logger/Logger.h"


//---------------------------------------------------------------------------------------------------------------------
// Actor
// Chapter 6, page 165
//---------------------------------------------------------------------------------------------------------------------
Actor::Actor(ActorId id)
{
    mID = id;
    mType = "Unknown";

	// [mrmike] added post press - this is an editor helper
	mResource = "Unknown";
}

Actor::~Actor(void)
{
    LogInformation("Destroying Actor " + eastl::to_string(mID));
	// [rez] if this assert fires, the actor was destroyed without calling Actor::Destroy()
    LogAssert(mComponents.empty(), "components are empty");
}

bool Actor::Init(tinyxml2::XMLElement* pData)
{
	mType = pData->Attribute("type");
	mResource = pData->Attribute("resource");
    LogInformation("Initializing Actor " + eastl::to_string(mID) + " " + mType);

    return true;
}

void Actor::PostInit(void)
{
	ActorComponents::iterator it = mComponents.begin();
    for (; it != mComponents.end(); ++it)
    {
        it->second->PostInit();
    }
}

void Actor::Destroy(void)
{
    mComponents.clear();
}

void Actor::Update(float deltaMs)
{
	ActorComponents::iterator it = mComponents.begin();
    for (; it != mComponents.end(); ++it)
    {
        it->second->Update(deltaMs);
    }
}

eastl::string Actor::ToXML()
{
    tinyxml2::XMLDocument outDoc;

    // Actor element
	tinyxml2::XMLElement* pActorElement = outDoc.NewElement("Actor");
    pActorElement->SetAttribute("type", mType.c_str());
	pActorElement->SetAttribute("resource", mResource.c_str());

    // components
    for (auto it = mComponents.begin(); it != mComponents.end(); ++it)
    {
        eastl::shared_ptr<ActorComponent> pComponent = it->second;
		tinyxml2::XMLElement* pComponentElement = pComponent->GenerateXml();
        pActorElement->LinkEndChild(pComponentElement);
    }

    outDoc.LinkEndChild(pActorElement);
	tinyxml2::XMLPrinter printer;
	outDoc.Accept(&printer);

	return printer.CStr();
}


void Actor::AddComponent(eastl::shared_ptr<ActorComponent> pComponent)
{
    eastl::pair<ActorComponents::iterator, bool> success = 
		mComponents.insert(eastl::make_pair(pComponent->GetId(), pComponent));
    LogAssert(success.second, "error add component");
}

