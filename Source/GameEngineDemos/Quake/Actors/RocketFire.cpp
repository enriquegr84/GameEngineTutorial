//========================================================================
// RocketFire.cpp - Firing rocket
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

#include "GameEngineStd.h"

#include "RocketFire.h"

#include "Game/GameLogic.h"
#include "Game/Actor/Actor.h"
#include "Game/Actor/TransformComponent.h"

#include "Core/Logger/Logger.h"

#include "Core/Event/EventManager.h"
#include "Core/Event/Event.h"

const char* RocketFire::Name = "RocketFire";

//---------------------------------------------------------------------------------------------------------------------
// RocketFire
//---------------------------------------------------------------------------------------------------------------------
RocketFire::RocketFire(void)
{
	mExplosionTime = 0.f;
}

bool RocketFire::Init(tinyxml2::XMLElement* pData)
{
	tinyxml2::XMLElement* pProjectile = pData->FirstChildElement("Projectile");
	if (pProjectile)
	{
		unsigned int temp = 0;
		mType = pProjectile->IntAttribute("type", temp);

		temp = 0;
		mCode = pProjectile->IntAttribute("code", temp);

		mExplosionTime = pProjectile->FloatAttribute("explosion");
	}

	return true;
}

tinyxml2::XMLElement* RocketFire::GenerateXml(void)
{
	tinyxml2::XMLDocument doc;

	// component element
	tinyxml2::XMLElement* pComponentElement = doc.NewElement(GetName());
	return pComponentElement;
}

void RocketFire::Apply(eastl::weak_ptr<Actor> pActor)
{
	eastl::shared_ptr<Actor> pStrongActor(pActor);
	if (pStrongActor)
	{
		LogInformation("Applying rocket firing to actor id " + eastl::to_string(pStrongActor->GetId()));
	}
}

void RocketFire::Update(float deltaMs)
{
	// drop timing counter
	if (mExplosionTime)
	{
		if (deltaMs >= mExplosionTime)
		{
			mExplosionTime = 0;

			eastl::shared_ptr<EventDataRequestDestroyActor>
				pRequestDestroyActorEvent(new EventDataRequestDestroyActor(mOwner->GetId()));
			BaseEventManager::Get()->QueueEvent(pRequestDestroyActorEvent);

			eastl::shared_ptr<TransformComponent> pTransformComponent =
				mOwner->GetComponent<TransformComponent>(TransformComponent::Name).lock();
			if (pTransformComponent)
			{
				Transform initTransform;
				initTransform.SetTranslation(pTransformComponent->GetTransform().GetTranslation());
				GameLogic::Get()->CreateActor("actors/quake/effects/rocketexplosion.xml", nullptr, &initTransform);
			}
		}
		else mExplosionTime -= deltaMs;
	}
}