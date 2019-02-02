//========================================================================
// ActorFactory.cpp : Creates actors from components
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

 
#include "ActorFactory.h"

#include "ActorComponent.h"
#include "AudioComponent.h"
#include "TransformComponent.h"
#include "RenderComponent.h"
#include "PhysicComponent.h"

#include "Game/GameLogic.h"

#include "Core/IO/XmlResource.h"


//---------------------------------------------------------------------------------------------------------------------
// Factory class definition
// Chapter 6, page 161
//---------------------------------------------------------------------------------------------------------------------
ActorFactory::ActorFactory(void)
{
    mComponentFactory.Register<TransformComponent>(ActorComponent::GetIdFromName(TransformComponent::Name));
	mComponentFactory.Register<MeshRenderComponent>(ActorComponent::GetIdFromName(MeshRenderComponent::Name));
    mComponentFactory.Register<SphereRenderComponent>(ActorComponent::GetIdFromName(SphereRenderComponent::Name));
	mComponentFactory.Register<CubeRenderComponent>(ActorComponent::GetIdFromName(CubeRenderComponent::Name));
    mComponentFactory.Register<GridRenderComponent>(ActorComponent::GetIdFromName(GridRenderComponent::Name));
    mComponentFactory.Register<ParticleEffectRenderComponent>(ActorComponent::GetIdFromName(ParticleEffectRenderComponent::Name));
	mComponentFactory.Register<BillboardRenderComponent>(ActorComponent::GetIdFromName(BillboardRenderComponent::Name));
    mComponentFactory.Register<LightRenderComponent>(ActorComponent::GetIdFromName(LightRenderComponent::Name));
    mComponentFactory.Register<SkyRenderComponent>(ActorComponent::GetIdFromName(SkyRenderComponent::Name));
    mComponentFactory.Register<AudioComponent>(ActorComponent::GetIdFromName(AudioComponent::Name));
	mComponentFactory.Register<PhysicComponent>(ActorComponent::GetIdFromName(PhysicComponent::Name));
}

eastl::shared_ptr<Actor> ActorFactory::CreateActor(const wchar_t* actorResource, 
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
    eastl::shared_ptr<Actor> pActor(new Actor(nextActorId));
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
		pTransformComponent->SetRotation(pInitialTransform->GetRotation());
		pTransformComponent->SetPosition(pInitialTransform->GetTranslation());
	}

    // Now that the actor has been fully created, run the post init phase
    pActor->PostInit();

    return pActor;
}

eastl::shared_ptr<ActorComponent> ActorFactory::CreateComponent(
	eastl::shared_ptr<Actor> pActor, tinyxml2::XMLElement* pData)
{
    const char* name = pData->Value();
    eastl::shared_ptr<ActorComponent> pComponent(
		mComponentFactory.Create(ActorComponent::GetIdFromName(name)));

    // initialize the component if we found one
    if (pComponent)
    {
        if (!pComponent->Init(pData))
        {
            LogError(eastl::string("Component failed to initialize: ") + eastl::string(name));
            return eastl::shared_ptr<ActorComponent>();
        }

		pActor->AddComponent(pComponent);
		pComponent->SetOwner(pActor);
    }
    else
    {
        LogError(eastl::string("Couldn't find ActorComponent named ") + eastl::string(name));
        return eastl::shared_ptr<ActorComponent>();  // fail
    }

    //	pComponent will be NULL if the component wasn't found.  This isn't necessarily 
	//	an error since you might have a custom CreateComponent() function in a sub class.
    return pComponent;
}


void ActorFactory::ModifyActor(eastl::shared_ptr<Actor> pActor, tinyxml2::XMLElement* overrides)
{
	// Loop through each child element and load the component
	tinyxml2::XMLElement* pNode = overrides->FirstChildElement();
	for (; pNode; pNode = pNode->NextSiblingElement())
	{
		ComponentId componentId = ActorComponent::GetIdFromName(pNode->Value());
		eastl::shared_ptr<ActorComponent> pComponent(
			pActor->GetComponent<ActorComponent>(componentId).lock());
		if (pComponent)
		{
			pComponent->Init(pNode);

			// added post press to ensure that components that need it have
			// Events generated that can notify subsystems when changes happen.
			// This was done to have SceneNode derived classes respond to RenderComponent
			// changes.
			pComponent->OnChanged();		
		}
		else pComponent = CreateComponent(pActor, pNode);
	}		
}