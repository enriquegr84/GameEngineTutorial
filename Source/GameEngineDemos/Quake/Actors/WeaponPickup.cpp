//========================================================================
// WeaponPickup.cpp - A weapon pickup
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

#include "WeaponPickup.h"

#include "Graphic/Graphic.h"

#include "Application/GameApplication.h"

#include "Core/Logger/Logger.h"
#include "Game/Actor/Actor.h"

const char* WeaponPickup::Name = "WeaponPickup";

//---------------------------------------------------------------------------------------------------------------------
// WeaponPickup
//---------------------------------------------------------------------------------------------------------------------
WeaponPickup::WeaponPickup(void)
{
	mRespawnTime = 0.f;
}

bool WeaponPickup::Init(tinyxml2::XMLElement* pData)
{
	tinyxml2::XMLElement* pWeapon = pData->FirstChildElement("Weapon");
	if (pWeapon)
	{
		unsigned int temp = 0;
		mType = pWeapon->IntAttribute("type", temp);

		temp = 0;
		mCode = pWeapon->IntAttribute("code", temp);

		temp = 0;
		mWait = pWeapon->IntAttribute("wait", temp);

		temp = 0;
		mAmmo = pWeapon->IntAttribute("ammo", temp);
	}

	return true;
}

tinyxml2::XMLElement* WeaponPickup::GenerateXml(void)
{
	tinyxml2::XMLDocument doc;

	// component element
	tinyxml2::XMLElement* pComponentElement = doc.NewElement(GetName());
	return pComponentElement;
}

void WeaponPickup::Apply(eastl::weak_ptr<Actor> pActor)
{
	eastl::shared_ptr<Actor> pStrongActor(pActor);
	if (pStrongActor)
	{
		LogInformation("Applying ammo pickup to actor id " + eastl::to_string(pStrongActor->GetId()));
	}
}

void WeaponPickup::Update(float deltaMs)
{
	// drop misc timing counter
	if (mRespawnTime)
	{
		GameApplication* gameApp = (GameApplication*)Application::App;
		const eastl::shared_ptr<ScreenElementScene>& pScene = gameApp->GetHumanView()->mScene;

		if (deltaMs >= mRespawnTime)
		{
			mRespawnTime = 0;

			eastl::shared_ptr<Node> node = pScene->GetSceneNode(mOwner->GetId());
			node->SetVisible(true);
		}
		else
		{
			mRespawnTime -= deltaMs;

			eastl::shared_ptr<Node> node = pScene->GetSceneNode(mOwner->GetId());
			node->SetVisible(false);
		}
	}
}

