//========================================================================
// QuakeActorFactory.cpp : Creates actors from components
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

 
#include "QuakeActorFactory.h"

#include "Actors/AmmoPickup.h"
#include "Actors/ItemPickup.h"
#include "Actors/ArmorPickup.h"
#include "Actors/HealthPickup.h"
#include "Actors/WeaponPickup.h"
#include "Actors/PushTrigger.h"
#include "Actors/TeleporterTrigger.h"
#include "Actors/DestinationTarget.h"
#include "Actors/SpeakerTarget.h"
#include "Actors/PlayerActor.h"

#include "Game/Actor/TransformComponent.h"
#include "Game/Actor/ActorComponent.h"
#include "Game/GameLogic.h"

#include "Core/IO/XmlResource.h"

//---------------------------------------------------------------------------------------------------------------------
QuakeActorFactory::QuakeActorFactory(void) : ActorFactory()
{
	mComponentFactory.Register<AmmoPickup>(ActorComponent::GetIdFromName(AmmoPickup::Name));
	mComponentFactory.Register<ItemPickup>(ActorComponent::GetIdFromName(ItemPickup::Name));
	mComponentFactory.Register<ArmorPickup>(ActorComponent::GetIdFromName(ArmorPickup::Name));
	mComponentFactory.Register<HealthPickup>(ActorComponent::GetIdFromName(HealthPickup::Name));
	mComponentFactory.Register<WeaponPickup>(ActorComponent::GetIdFromName(WeaponPickup::Name));
	mComponentFactory.Register<PushTrigger>(ActorComponent::GetIdFromName(PushTrigger::Name));
	mComponentFactory.Register<TeleporterTrigger>(ActorComponent::GetIdFromName(TeleporterTrigger::Name));
	mComponentFactory.Register<DestinationTarget>(ActorComponent::GetIdFromName(DestinationTarget::Name));
	mComponentFactory.Register<SpeakerTarget>(ActorComponent::GetIdFromName(SpeakerTarget::Name));
}

eastl::shared_ptr<ActorComponent> QuakeActorFactory::CreateComponent(
	eastl::shared_ptr<Actor> pActor, tinyxml2::XMLElement* pData)
{
	return ActorFactory::CreateComponent(pActor, pData);
}


eastl::shared_ptr<Actor> QuakeActorFactory::CreatePlayerActor(const wchar_t* actorResource,
	tinyxml2::XMLElement *overrides, const Transform *pInitialTransform, const ActorId serversActorId)
{
	// Grab the root XML node
	tinyxml2::XMLElement* pRoot = XmlResourceLoader::LoadAndReturnRootXMLElement(actorResource);
	if (!pRoot)
	{
		LogError(L"Failed to create actor from resource: " + eastl::wstring(actorResource));
		return eastl::shared_ptr<Actor>();
	}

	// create the actor instance
	ActorId nextActorId = serversActorId;
	if (nextActorId == INVALID_ACTOR_ID)
	{
		nextActorId = GameLogic::Get()->GetNewActorID();
	}
	eastl::shared_ptr<Actor> pActor(new PlayerActor(nextActorId));
	if (!pActor->Init(pRoot))
	{
		LogError(L"Failed to initialize actor: " + eastl::wstring(actorResource));
		return eastl::shared_ptr<Actor>();
	}

	bool initialTransformSet = false;
	tinyxml2::XMLElement* pNode = pRoot->FirstChildElement();

	// Loop through each child element and load the component
	for (; pNode; pNode = pNode->NextSiblingElement())
	{
		eastl::shared_ptr<ActorComponent> pComponent(CreateComponent(pActor, pNode));
		if (!pComponent)
		{
			//	If an error occurs, we kill the actor and bail.  We could keep going, 
			//	but the actor is will only be partially complete so it's not worth it.  
			//	Note that the pActor instance will be destroyed because it will fall 
			//	out of scope with nothing else pointing to it.
			return eastl::shared_ptr<Actor>();
		}
	}

	if (overrides)
	{
		ModifyActor(pActor, overrides);
	}

	//	This is a bit of a hack to get the initial transform of the transform component 
	//	set before the other components (like PhysicsComponent) read it.
	eastl::shared_ptr<TransformComponent> pTransformComponent(
		pActor->GetComponent<TransformComponent>(TransformComponent::Name).lock());
	if (pInitialTransform && pTransformComponent)
	{
		pTransformComponent->SetPosition(pInitialTransform->GetTranslation());
	}

	// Now that the actor has been fully created, run the post init phase
	pActor->PostInit();

	return pActor;
}