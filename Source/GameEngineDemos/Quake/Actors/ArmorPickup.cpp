//========================================================================
// ArmorPickup.cpp - An armor pickup
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

#include "ArmorPickup.h"

#include "Graphic/Graphic.h"

#include "Application/GameApplication.h"

#include "Core/Logger/Logger.h"
#include "Game/Actor/Actor.h"

const char* ArmorPickup::Name = "ArmorPickup";

//---------------------------------------------------------------------------------------------------------------------
// ArmorPickup
//---------------------------------------------------------------------------------------------------------------------
ArmorPickup::ArmorPickup(void)
{
	mRespawnTime = 0.f;
}

bool ArmorPickup::Init(tinyxml2::XMLElement* pData)
{
	tinyxml2::XMLElement* pArmor = pData->FirstChildElement("Armor");
	if (pArmor)
	{
		unsigned int temp = 0;
		mType = pArmor->IntAttribute("type", temp);

		temp = 0;
		mCode = pArmor->IntAttribute("code", temp);

		temp = 0;
		mWait = pArmor->IntAttribute("wait", temp);

		temp = 0;
		mAmount = pArmor->IntAttribute("amount", temp);

		temp = 0;
		mMaximum = pArmor->IntAttribute("maximum", temp);
	}

	return true;
}

tinyxml2::XMLElement* ArmorPickup::GenerateXml(void)
{
	tinyxml2::XMLDocument doc;

	// component element
	tinyxml2::XMLElement* pComponentElement = doc.NewElement(GetName());
	return pComponentElement;
}

void ArmorPickup::Apply(eastl::weak_ptr<Actor> pActor)
{
	eastl::shared_ptr<Actor> pStrongActor(pActor);
	if (pStrongActor)
	{
		LogInformation("Applying armor pickup to actor id " + eastl::to_string(pStrongActor->GetId()));
	}
}

void ArmorPickup::Update(float deltaMs)
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

