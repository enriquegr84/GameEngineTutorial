//========================================================================
// QuakeAIProcess.cpp : Defines ai process that can run in a thread
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

#include "Quake.h"
#include "QuakeApp.h"
#include "QuakeEvents.h"
#include "QuakeAIManager.h"
#include "QuakeAIProcess.h"

#include <ppl.h> 

using namespace concurrency;

QuakeAIProcess::QuakeAIProcess()
  :  RealtimeProcess()
{

}

QuakeAIProcess::~QuakeAIProcess(void)
{

}

void QuakeAIProcess::Heuristic(NodeState& playerState, NodeState& otherPlayerState)
{
	//lets give priority to health(0.4), armor(0.2), weapon/ammo(0.4)
	float heuristic = 0.f;

	int maxHealth = 200;
	int maxArmor = 200;
	int maxAmmo = 200;
	int ammo = 0;
	for (int weapon = 1; weapon < MAX_WEAPONS; weapon++)
	{
		switch (weapon)
		{
			case WP_LIGHTNING:
				maxAmmo = 200;
				if (playerState.stats[STAT_WEAPONS] && (1 << weapon))
				{
					ammo = (playerState.ammo[weapon] < maxAmmo) ? playerState.ammo[weapon] : maxAmmo;
					heuristic += (ammo / maxAmmo) * 0.15f * 0.4f;
				}
				break;
			case WP_SHOTGUN:
				maxAmmo = 20;
				if (playerState.stats[STAT_WEAPONS] && (1 << weapon))
				{
					ammo = (playerState.ammo[weapon] < maxAmmo) ? playerState.ammo[weapon] : maxAmmo;
					heuristic += (ammo / maxAmmo) * 0.15f * 0.4f;
				}
				break;
			case WP_MACHINEGUN:
				maxAmmo = 200;
				if (playerState.stats[STAT_WEAPONS] && (1 << weapon))
				{
					ammo = (playerState.ammo[weapon] < maxAmmo) ? playerState.ammo[weapon] : maxAmmo;
					heuristic += (ammo / maxAmmo) * 0.1f * 0.4f;
				}
				break;
			case WP_PLASMAGUN:
				maxAmmo = 200;
				if (playerState.stats[STAT_WEAPONS] && (1 << weapon))
				{
					ammo = (playerState.ammo[weapon] < maxAmmo) ? playerState.ammo[weapon] : maxAmmo;
					heuristic += (ammo / maxAmmo) * 0.1f * 0.4f;
				}
				break;
			case WP_GRENADE_LAUNCHER:
				maxAmmo = 20;
				if (playerState.stats[STAT_WEAPONS] && (1 << weapon))
				{
					ammo = (playerState.ammo[weapon] < maxAmmo) ? playerState.ammo[weapon] : maxAmmo;
					heuristic += (ammo / maxAmmo) * 0.f * 0.4f;
				}
				break;
			case WP_ROCKET_LAUNCHER:
				maxAmmo = 20;
				if (playerState.stats[STAT_WEAPONS] && (1 << weapon))
				{
					ammo = (playerState.ammo[weapon] < maxAmmo) ? playerState.ammo[weapon] : maxAmmo;
					heuristic += (ammo / maxAmmo) * 0.2f * 0.4f;
				}
				break;
			case WP_RAILGUN:
				maxAmmo = 20;
				if (playerState.stats[STAT_WEAPONS] && (1 << weapon))
				{
					ammo = (playerState.ammo[weapon] < maxAmmo) ? playerState.ammo[weapon] : maxAmmo;
					heuristic += (ammo / maxAmmo) * 0.3f * 0.4f;
				}
				break;
		}
	}
	heuristic += (playerState.stats[STAT_HEALTH] / maxHealth) * 0.4f;
	heuristic += (playerState.stats[STAT_ARMOR] / maxArmor) * 0.2f;

	//the other part of the heuristic is the damage dealer
	int maxDamage = 0;
	for (int weapon = 1; weapon <= MAX_WEAPONS; weapon++)
	{
		if (playerState.damage[weapon - 1] > maxDamage)
		{
			playerState.weapon = (WeaponType)weapon;
			maxDamage = playerState.damage[weapon - 1];
		}
	}

	//damage threshold to the max hp/armor
	if (maxDamage > 400) maxDamage = 400;
	heuristic += (maxDamage / 400);

	//calculate the other player heuristic the same way but reversing the sign
	for (int weapon = 1; weapon < MAX_WEAPONS; weapon++)
	{
		switch (weapon)
		{
			case WP_LIGHTNING:
				maxAmmo = 200;
				if (otherPlayerState.stats[STAT_WEAPONS] && (1 << weapon))
				{
					ammo = (otherPlayerState.ammo[weapon] < maxAmmo) ? otherPlayerState.ammo[weapon] : maxAmmo;
					heuristic -= (ammo / maxAmmo) * 0.15f * 0.4f;
				}
				break;
			case WP_SHOTGUN:
				maxAmmo = 20;
				if (otherPlayerState.stats[STAT_WEAPONS] && (1 << weapon))
				{
					ammo = (otherPlayerState.ammo[weapon] < maxAmmo) ? otherPlayerState.ammo[weapon] : maxAmmo;
					heuristic -= (ammo / maxAmmo) * 0.15f * 0.4f;
				}
				break;
			case WP_MACHINEGUN:
				maxAmmo = 200;
				if (otherPlayerState.stats[STAT_WEAPONS] && (1 << weapon))
				{
					ammo = (otherPlayerState.ammo[weapon] < maxAmmo) ? otherPlayerState.ammo[weapon] : maxAmmo;
					heuristic -= (ammo / maxAmmo) * 0.1f * 0.4f;
				}
				break;
			case WP_PLASMAGUN:
				maxAmmo = 200;
				if (otherPlayerState.stats[STAT_WEAPONS] && (1 << weapon))
				{
					ammo = (otherPlayerState.ammo[weapon] < maxAmmo) ? otherPlayerState.ammo[weapon] : maxAmmo;
					heuristic -= (ammo / maxAmmo) * 0.1f * 0.4f;
				}
				break;
			case WP_GRENADE_LAUNCHER:
				maxAmmo = 20;
				if (otherPlayerState.stats[STAT_WEAPONS] && (1 << weapon))
				{
					ammo = (otherPlayerState.ammo[weapon] < maxAmmo) ? otherPlayerState.ammo[weapon] : maxAmmo;
					heuristic -= (ammo / maxAmmo) * 0.f * 0.4f;
				}
				break;
			case WP_ROCKET_LAUNCHER:
				maxAmmo = 20;
				if (otherPlayerState.stats[STAT_WEAPONS] && (1 << weapon))
				{
					ammo = (otherPlayerState.ammo[weapon] < maxAmmo) ? otherPlayerState.ammo[weapon] : maxAmmo;
					heuristic -= (ammo / maxAmmo) * 0.2f * 0.4f;
				}
				break;
			case WP_RAILGUN:
				maxAmmo = 20;
				if (otherPlayerState.stats[STAT_WEAPONS] && (1 << weapon))
				{
					ammo = (otherPlayerState.ammo[weapon] < maxAmmo) ? otherPlayerState.ammo[weapon] : maxAmmo;
					heuristic -= (ammo / maxAmmo) * 0.3f * 0.4f;
				}
				break;
		}
	}
	heuristic -= (otherPlayerState.stats[STAT_HEALTH] / maxHealth) * 0.4f;
	heuristic -= (otherPlayerState.stats[STAT_ARMOR] / maxArmor) * 0.2f;

	//the other part of the heuristic is the damage dealer
	maxDamage = 0;
	for (int weapon = 1; weapon <= MAX_WEAPONS; weapon++)
	{
		if (otherPlayerState.damage[weapon - 1] > maxDamage)
		{
			otherPlayerState.weapon = (WeaponType)weapon;
			maxDamage = otherPlayerState.damage[weapon - 1];
		}
	}

	//damage threshold to the max hp/armor
	if (maxDamage > 400) maxDamage = 400;
	heuristic -= (maxDamage / 400);
	playerState.heuristic = heuristic;
	otherPlayerState.heuristic = heuristic;
}

void QuakeAIProcess::Damage(NodeState& state, float visibleTime, float visibleDistance, float visibleHeight)
{
	for (int weapon = 1; weapon <= MAX_WEAPONS; weapon++)
	{
		if (weapon != WP_GAUNTLET)
		{
			if (state.ammo[weapon] && (state.stats[STAT_WEAPONS] & (1 << weapon)))
			{
				int damage = 0;
				int shotCount = 0;
				float fireTime = 0.f;
				float rangeDistance = 0;
				switch (weapon)
				{
					case WP_LIGHTNING:
						damage = 8;
						fireTime = 0.05f;
						state.damage[weapon - 1] = 0;
						shotCount = int(visibleTime / fireTime);
						shotCount = shotCount > state.ammo[weapon] ? state.ammo[weapon] : shotCount;
						if (visibleDistance <= LIGHTNING_RANGE)
							state.damage[weapon - 1] = damage * shotCount;
						break;
					case WP_SHOTGUN:
						damage = 120;
						fireTime = 1.0f;
						rangeDistance = visibleDistance > 600 ? visibleDistance : 600;
						shotCount = (int)round(visibleTime / fireTime);
						shotCount = shotCount > state.ammo[weapon] ? state.ammo[weapon] : shotCount;
						state.damage[weapon - 1] = (int)round(damage *
							(1.f - (visibleDistance / rangeDistance)) * shotCount);
						break;
					case WP_MACHINEGUN:
						damage = 7;
						fireTime = 0.1f;
						rangeDistance = visibleDistance > 800 ? visibleDistance : 800;
						shotCount = int(visibleTime / fireTime);
						shotCount = shotCount > state.ammo[weapon] ? state.ammo[weapon] : shotCount;
						state.damage[weapon - 1] = (int)round(damage *
							(1.f - (visibleDistance / rangeDistance)) * shotCount);
						break;
					case WP_GRENADE_LAUNCHER:
						damage = 100;
						fireTime = 0.8f;
						state.damage[weapon - 1] = 0;
						break;
					case WP_ROCKET_LAUNCHER:
						damage = 100;
						fireTime = 0.8f;
						rangeDistance = visibleDistance > 1200 ? visibleDistance : 1200;
						if (visibleHeight <= 20.f) rangeDistance = 600;
						shotCount = int(visibleTime / fireTime);
						shotCount = shotCount > state.ammo[weapon] ? state.ammo[weapon] : shotCount;
						state.damage[weapon - 1] = (int)round(damage *
							(1.f - (visibleDistance / rangeDistance)) * shotCount);
						break;
					case WP_PLASMAGUN:
						damage = 15;
						fireTime = 0.1f;
						rangeDistance = visibleDistance > 400 ? visibleDistance : 400;
						shotCount = int(visibleTime / fireTime);
						shotCount = shotCount > state.ammo[weapon] ? state.ammo[weapon] : shotCount;
						state.damage[weapon - 1] = (int)round(damage *
							(1.f - (visibleDistance / rangeDistance)) * shotCount);
						break;
					case WP_RAILGUN:
						damage = 100;
						fireTime = 1.5f;
						shotCount = int(visibleTime / fireTime);
						shotCount = shotCount > state.ammo[weapon] ? state.ammo[weapon] : shotCount;
						state.damage[weapon - 1] = damage * shotCount;
						break;
				}
			}
		}
		else
		{
			if (visibleDistance <= 30.f)
			{
				int damage = 50;
				float fireTime = 0.4f;
				int shotCount = int(visibleTime / fireTime);
				state.damage[weapon - 1] = damage * shotCount;
			}
		}
	}
}

/*
CanItemBeGrabbed
Returns false if the item should not be picked up.
*/
bool CanItemBeGrabbed(ActorId itemId, float itemTime, NodeState& playerState)
{
	eastl::shared_ptr<Actor> pItemActor(GameLogic::Get()->GetActor(itemId).lock());

	if (pItemActor->GetType() == "Weapon")
	{
		eastl::shared_ptr<WeaponPickup> pWeaponPickup =
			pItemActor->GetComponent<WeaponPickup>(WeaponPickup::Name).lock();
		if (pWeaponPickup->mRespawnTime - itemTime <= 0)
			return true;
	}
	else if (pItemActor->GetType() == "Ammo")
	{
		eastl::shared_ptr<AmmoPickup> pAmmoPickup =
			pItemActor->GetComponent<AmmoPickup>(AmmoPickup::Name).lock();
		if (pAmmoPickup->mRespawnTime - itemTime <= 0)
		{
			if (playerState.ammo[pAmmoPickup->GetCode()] >= 200)
				return false;		// can't hold any more

			return true;
		}
	}
	else if (pItemActor->GetType() == "Armor")
	{
		eastl::shared_ptr<ArmorPickup> pArmorPickup =
			pItemActor->GetComponent<ArmorPickup>(ArmorPickup::Name).lock();
		if (pArmorPickup->mRespawnTime - itemTime <= 0)
		{
			if (playerState.stats[STAT_ARMOR] >= playerState.stats[STAT_MAX_HEALTH] * 2)
				return false;

			return true;
		}
	}
	else if (pItemActor->GetType() == "Health")
	{
		eastl::shared_ptr<HealthPickup> pHealthPickup =
			pItemActor->GetComponent<HealthPickup>(HealthPickup::Name).lock();
		if (pHealthPickup->mRespawnTime - itemTime <= 0)
		{
			// small and mega healths will go over the max, otherwise
			// don't pick up if already at max
			if (pHealthPickup->GetAmount() == 5 || pHealthPickup->GetAmount() == 100)
			{
				if (playerState.stats[STAT_HEALTH] >= playerState.stats[STAT_MAX_HEALTH] * 2)
					return false;

				return true;
			}

			if (playerState.stats[STAT_HEALTH] >= playerState.stats[STAT_MAX_HEALTH])
				return false;

			return true;
		}
	}
	return false;
}

void QuakeAIProcess::PickupItem(NodeState& playerState, eastl::map<ActorId, float>& actors)
{
	for (auto actor : actors)
	{
		eastl::shared_ptr<Actor> pItemActor(
			GameLogic::Get()->GetActor(actor.first).lock());

		if (pItemActor->GetType() == "Weapon")
		{
			eastl::shared_ptr<WeaponPickup> pWeaponPickup =
				pItemActor->GetComponent<WeaponPickup>(WeaponPickup::Name).lock();
			if (pWeaponPickup->mRespawnTime - actor.second > 0)
				continue;

			// add the weapon
			playerState.stats[STAT_WEAPONS] |= (1 << pWeaponPickup->GetType());

			// add ammo
			playerState.ammo[pWeaponPickup->GetType()] += pWeaponPickup->GetAmmo();
			if (playerState.ammo[pWeaponPickup->GetType()] > 200)
				playerState.ammo[pWeaponPickup->GetType()] = 200;
		}
		else if (pItemActor->GetType() == "Ammo")
		{
			eastl::shared_ptr<AmmoPickup> pAmmoPickup =
				pItemActor->GetComponent<AmmoPickup>(AmmoPickup::Name).lock();
			if (pAmmoPickup->mRespawnTime - actor.second > 0)
				continue;

			playerState.ammo[pAmmoPickup->GetType()] += pAmmoPickup->GetAmount();
			if (playerState.ammo[pAmmoPickup->GetType()] > 200)
				playerState.ammo[pAmmoPickup->GetType()] = 200;
		}
		else if (pItemActor->GetType() == "Armor")
		{
			eastl::shared_ptr<ArmorPickup> pArmorPickup =
				pItemActor->GetComponent<ArmorPickup>(ArmorPickup::Name).lock();
			if (pArmorPickup->mRespawnTime - actor.second > 0)
				continue;

			playerState.stats[STAT_ARMOR] += pArmorPickup->GetAmount();
			if (playerState.stats[STAT_ARMOR] > playerState.stats[STAT_MAX_HEALTH] * 2)
				playerState.stats[STAT_ARMOR] = playerState.stats[STAT_MAX_HEALTH] * 2;
		}
		else if (pItemActor->GetType() == "Health")
		{
			eastl::shared_ptr<HealthPickup> pHealthPickup =
				pItemActor->GetComponent<HealthPickup>(HealthPickup::Name).lock();
			if (pHealthPickup->mRespawnTime - actor.second > 0)
				continue;

			int max;
			if (pHealthPickup->GetAmount() != 5 && pHealthPickup->GetAmount() != 100)
				max = playerState.stats[STAT_MAX_HEALTH];
			else
				max = playerState.stats[STAT_MAX_HEALTH] * 2;

			playerState.stats[STAT_HEALTH] += pHealthPickup->GetAmount();
			if (playerState.stats[STAT_HEALTH] > max)
				playerState.stats[STAT_HEALTH] = max;
		}

		if (pItemActor->GetType() == "Weapon")
		{
			eastl::shared_ptr<WeaponPickup> pWeaponPickup =
				pItemActor->GetComponent<WeaponPickup>(WeaponPickup::Name).lock();
			if (pWeaponPickup->mRespawnTime - actor.second > 0)
				continue;

			playerState.stats[STAT_WEAPONS] |= 1 << pWeaponPickup->GetCode();
			if (!playerState.ammo[pWeaponPickup->GetCode()])
				playerState.ammo[pWeaponPickup->GetCode()] = 1;
		}
	}
}

void QuakeAIProcess::Visibility(
	PathingNode* playerNode, PathingArcVec& playerPathPlan,
	float* visibleTime, float* visibleDistance, float* visibleHeight,
	PathingNode* otherPlayerNode, PathingArcVec& otherPlayerPathPlan,
	float* otherVisibleTime, float* otherVisibleDistance, float* otherVisibleHeight)
{
	float visibleWeight = 0.f;
	float totalTime = 0.f, totalArcTime = 0.f;
	unsigned int index = 0, otherIndex = 0, otherPathIndex = 0;

	PathingNode* currentNode = playerNode;
	PathingNode* otherCurrentNode = otherPlayerNode;
	PathingTransition* currentTransition = NULL;
	PathingTransition* otherCurrentTransition =
		otherCurrentNode->FindTransition(otherPlayerPathPlan[otherPathIndex]->GetId());
	for (PathingArc* currentArc : playerPathPlan)
	{
		index = 0;
		currentTransition = currentNode->FindTransition(currentArc->GetId());
		for (; index < currentTransition->GetNodes().size(); index++)
		{
			visibleWeight = currentTransition->GetWeights()[index];

			if (currentTransition->GetNodes()[index]->IsVisibleNode(
				otherCurrentTransition->GetNodes()[otherIndex]))
			{
				//we only take into consideration visibility from ground
				if (currentArc->GetType() == GAT_MOVE)
				{
					(*visibleDistance) += Length(
						otherCurrentTransition->GetConnections()[otherIndex] -
						currentTransition->GetConnections()[index]) * visibleWeight;
					(*visibleHeight) +=
						(currentTransition->GetConnections()[index][2] -
						otherCurrentTransition->GetConnections()[otherIndex][2]) * visibleWeight;
					(*visibleTime) += visibleWeight;
				}
				if (otherPlayerPathPlan[otherPathIndex]->GetType() == GAT_MOVE)
				{
					(*otherVisibleDistance) += Length(
						otherCurrentTransition->GetConnections()[otherIndex] -
						currentTransition->GetConnections()[index]) * visibleWeight;
					(*otherVisibleHeight) +=
						(otherCurrentTransition->GetConnections()[otherIndex][2] -
						currentTransition->GetConnections()[index][2]) * visibleWeight;
					(*otherVisibleTime) += visibleWeight;
				}
			}
			while (totalArcTime <= totalTime)
			{
				totalArcTime += otherCurrentTransition->GetWeights()[otherIndex];
				if (otherIndex + 1 >= otherCurrentTransition->GetNodes().size())
				{
					if (otherPathIndex + 1 < otherPlayerPathPlan.size())
					{
						otherIndex = 0;
						otherCurrentNode = otherPlayerPathPlan[otherPathIndex]->GetNode();

						otherPathIndex++;
						otherCurrentTransition = otherCurrentNode->FindTransition(
							otherPlayerPathPlan[otherPathIndex]->GetId());
					}
					else break;
				}
				else otherIndex++;
			}
			totalTime += visibleWeight;
		}
		currentNode = currentArc->GetNode();
	}

	if (currentTransition)
	{
		index--;
		otherCurrentTransition = otherCurrentNode->FindTransition(
			otherPlayerPathPlan[otherPathIndex]->GetId());
		for (; otherIndex < otherCurrentTransition->GetNodes().size(); otherIndex++)
		{
			visibleWeight = currentTransition->GetWeights()[index];

			if (currentTransition->GetNodes()[index]->IsVisibleNode(
				otherCurrentTransition->GetNodes()[otherIndex]))
			{
				if (playerPathPlan.back()->GetType() == GAT_MOVE)
				{
					(*visibleDistance) += Length(
						otherCurrentTransition->GetConnections()[otherIndex] -
						currentTransition->GetConnections()[index]) * visibleWeight;
					(*visibleHeight) +=
						(currentTransition->GetConnections()[index][2] -
						otherCurrentTransition->GetConnections()[otherIndex][2]) * visibleWeight;
					(*visibleTime) += visibleWeight;
				}
				if (otherPlayerPathPlan[otherPathIndex]->GetType() == GAT_MOVE)
				{
					(*otherVisibleDistance) += Length(
						otherCurrentTransition->GetConnections()[otherIndex] -
						currentTransition->GetConnections()[index]) * visibleWeight;
					(*otherVisibleHeight) +=
						(otherCurrentTransition->GetConnections()[otherIndex][2] -
						currentTransition->GetConnections()[index][2]) * visibleWeight;
					(*otherVisibleTime) += visibleWeight;
				}
			}
		}

		//lets put a minimum of potential visibility time
		otherIndex--;
		if (totalTime < 4.0f)
		{
			visibleWeight = 4.0f - totalTime;

			if (currentTransition->GetNodes()[index]->IsVisibleNode(
				otherCurrentTransition->GetNodes()[otherIndex]))
			{
				if (playerPathPlan.back()->GetType() == GAT_MOVE)
				{
					(*visibleDistance) += Length(
						otherCurrentTransition->GetConnections()[otherIndex] -
						currentTransition->GetConnections()[index]) * visibleWeight;
					(*visibleHeight) +=
						(currentTransition->GetConnections()[index][2] -
						otherCurrentTransition->GetConnections()[otherIndex][2]) * visibleWeight;
					(*visibleTime) += visibleWeight;
				}
			}
			if (currentTransition->GetNodes()[index]->IsVisibleNode(
				otherCurrentTransition->GetNodes()[otherIndex]))
			{
				if (otherPlayerPathPlan[otherPathIndex]->GetType() == GAT_MOVE)
				{
					(*otherVisibleDistance) += Length(
						otherCurrentTransition->GetConnections()[otherIndex] -
						currentTransition->GetConnections()[index]) * visibleWeight;
					(*otherVisibleHeight) +=
						(otherCurrentTransition->GetConnections()[otherIndex][2] -
							currentTransition->GetConnections()[index][2]) * visibleWeight;
					(*otherVisibleTime) += visibleWeight;
				}
			}
		}

		//average
		if ((*visibleTime) > 0.f)
		{
			(*visibleDistance) /= (*visibleTime);
			(*visibleHeight) /= (*visibleTime);
		}
		if ((*otherVisibleTime) > 0.f)
		{
			(*otherVisibleDistance) /= (*otherVisibleTime);
			(*otherVisibleHeight) /= (*otherVisibleTime);
		}
	}
}

void QuakeAIProcess::Visibility(
	PathingNode* playerNode, PathingArcVec& playerPathPlan,
	float* visibleTime, float* visibleDistance, float* visibleHeight,
	PathingNode* otherPlayerNode,
	float* otherVisibleTime, float* otherVisibleDistance, float* otherVisibleHeight)
{
	float visibleWeight = 0.f;
	float totalTime = 0.f;
	unsigned int index = 0;

	PathingNode* currentNode = playerNode;
	PathingTransition* currentTransition = NULL;
	for (PathingArc* currentArc : playerPathPlan)
	{
		index = 0;
		currentTransition = currentNode->FindTransition(currentArc->GetId());
		for (; index < currentTransition->GetNodes().size(); index++)
		{
			visibleWeight = currentTransition->GetWeights()[index];

			if (currentTransition->GetNodes()[index]->IsVisibleNode(otherPlayerNode))
			{
				//we only take into consideration visibility from ground
				if (currentArc->GetType() == GAT_MOVE)
				{
					(*visibleDistance) += Length(otherPlayerNode->GetPos() -
						currentTransition->GetConnections()[index]) * visibleWeight;
					(*visibleHeight) +=
						(currentTransition->GetConnections()[index][2] -
						otherPlayerNode->GetPos()[2]) * visibleWeight;
					(*visibleTime) += visibleWeight;
				}

				(*otherVisibleDistance) += Length(otherPlayerNode->GetPos() -
					currentTransition->GetConnections()[index]) * visibleWeight;
				(*otherVisibleHeight) += (otherPlayerNode->GetPos()[2] -
					currentTransition->GetConnections()[index][2]) * visibleWeight;
				(*otherVisibleTime) += visibleWeight;
			}
			totalTime += visibleWeight;
		}
		currentNode = currentArc->GetNode();
	}

	if (currentTransition)
	{
		index--;

		//lets put a minimum of potential visibility time
		if (totalTime < 4.0f)
		{
			visibleWeight = 4.0f - totalTime;

			if (currentTransition->GetNodes()[index]->IsVisibleNode(otherPlayerNode))
			{
				if (playerPathPlan.back()->GetType() == GAT_MOVE)
				{
					(*visibleDistance) += Length(otherPlayerNode->GetPos() -
						currentTransition->GetConnections()[index]) * visibleWeight;
					(*visibleHeight) +=
						(currentTransition->GetConnections()[index][2] -
						otherPlayerNode->GetPos()[2]) * visibleWeight;
					(*visibleTime) += visibleWeight;
				}
			}

			(*otherVisibleDistance) += Length(otherPlayerNode->GetPos() -
				currentTransition->GetConnections()[index]) * visibleWeight;
			(*otherVisibleHeight) += (otherPlayerNode->GetPos()[2] -
				currentTransition->GetConnections()[index][2]) * visibleWeight;
			(*otherVisibleTime) += visibleWeight;
		}

		//average
		if ((*visibleTime) > 0.f)
		{
			(*visibleDistance) /= (*visibleTime);
			(*visibleHeight) /= (*visibleTime);
		}
		if ((*otherVisibleTime) > 0.f)
		{
			(*otherVisibleDistance) /= (*otherVisibleTime);
			(*otherVisibleHeight) /= (*otherVisibleTime);
		}
	}
}

void QuakeAIProcess::Simulation(
	NodeState& playerState, eastl::vector<PathingArcVec>& playerPathPlans,
	NodeState& otherPlayerState, eastl::vector<PathingArcVec>& otherPlayerPathPlans)
{
	//visibility between pathing transitions
	PathingNode* playerNode = playerState.node;
	PathingNode* otherPlayerNode = otherPlayerState.node;
	eastl::map<PathingArcVec, eastl::map<PathingArcVec, NodeState>> playerStates, otherPlayerStates;
	for (PathingArcVec playerPathPlan : playerPathPlans)
	{
		float pathPlanWeight = 0.f;
		eastl::map<ActorId, float> actors;
		if (playerNode->GetActorId() != INVALID_ACTOR_ID)
			actors[playerNode->GetActorId()] = pathPlanWeight;
		for (PathingArc* playerArc : playerPathPlan)
		{
			pathPlanWeight += playerArc->GetWeight();
			if (playerArc->GetNode()->GetActorId() != INVALID_ACTOR_ID)
				actors[playerArc->GetNode()->GetActorId()] = pathPlanWeight;
		}

		for (PathingArcVec otherPlayerPathPlan : otherPlayerPathPlans)
		{
			float otherPathPlanWeight = 0.f;
			eastl::map<ActorId, float> otherActors;
			if (otherPlayerNode->GetActorId() != INVALID_ACTOR_ID)
				otherActors[otherPlayerNode->GetActorId()] = otherPathPlanWeight;
			for (PathingArc* otherPlayerArc : otherPlayerPathPlan)
			{
				otherPathPlanWeight += otherPlayerArc->GetWeight();
				if (otherPlayerArc->GetNode()->GetActorId() != INVALID_ACTOR_ID)
					otherActors[otherPlayerArc->GetNode()->GetActorId()] = otherPathPlanWeight;
			}

			float visibleTime = 0, visibleDistance = 0, visibleHeight = 0;
			float otherVisibleTime = 0, otherVisibleDistance = 0, otherVisibleHeight = 0;
			if (pathPlanWeight > otherPathPlanWeight)
			{
				Visibility(playerNode, playerPathPlan,
					&visibleTime, &visibleDistance, &visibleHeight,
					otherPlayerNode, otherPlayerPathPlan,
					&otherVisibleTime, &otherVisibleDistance, &otherVisibleHeight);
			}
			else
			{
				Visibility(otherPlayerNode, otherPlayerPathPlan, 
					&otherVisibleTime, &otherVisibleDistance, &otherVisibleHeight,
					playerNode, playerPathPlan, &visibleTime, &visibleDistance, &visibleHeight);
			}

			//calculate damage
			NodeState playerNodeState(playerState);
			playerNodeState.path = playerPathPlan;
			playerNodeState.node = playerPathPlan.back()->GetNode();
			Damage(playerNodeState, visibleTime, visibleDistance, visibleHeight);

			eastl::map<ActorId, float> pathActors;
			for (auto actor : actors)
			{
				if (CanItemBeGrabbed(actor.first, actor.second, playerState))
				{
					if (otherActors.find(actor.first) != otherActors.end())
					{
						if (!CanItemBeGrabbed(actor.first, actor.second, otherPlayerState) ||
							otherActors[actor.first] > actor.second)
						{
							pathActors[actor.first] = actor.second;
						}
					}
					else pathActors[actor.first] = actor.second;
				}
			}
			PickupItem(playerNodeState, pathActors);

			NodeState otherPlayerNodeState(otherPlayerState);
			otherPlayerNodeState.path = otherPlayerPathPlan;
			otherPlayerNodeState.node = otherPlayerPathPlan.back()->GetNode();
			Damage(otherPlayerNodeState, otherVisibleTime, otherVisibleDistance, otherVisibleHeight);
			
			eastl::map<ActorId, float> otherPathActors;
			for (auto otherActor : otherActors)
			{
				if (CanItemBeGrabbed(otherActor.first, otherActor.second, otherPlayerState))
				{
					if (actors.find(otherActor.first) != actors.end())
					{
						if (!CanItemBeGrabbed(otherActor.first, otherActor.second, playerState) ||
							actors[otherActor.first] > otherActor.second)
						{
							otherPathActors[otherActor.first] = otherActor.second;
						}
					}
					else otherPathActors[otherActor.first] = otherActor.second;
				}
			}
			PickupItem(otherPlayerNodeState, otherPathActors);

			//we calculate the heuristic
			Heuristic(playerNodeState, otherPlayerNodeState);
			playerStates[playerPathPlan][otherPlayerPathPlan] = playerNodeState;
			otherPlayerStates[otherPlayerPathPlan][playerPathPlan] = otherPlayerNodeState;
		}
	}

	//we proceed with the minimax algorithm between players
	NodeState bestPlayerState;
	bestPlayerState.heuristic = FLT_MIN;
	for (auto evaluatePlayerState : playerStates)
	{
		NodeState playerNodeState;
		playerNodeState.heuristic = FLT_MAX;
		for (auto evaluatePlayerNodeState : evaluatePlayerState.second)
			if (evaluatePlayerNodeState.second.heuristic < playerNodeState.heuristic)
				playerNodeState = evaluatePlayerNodeState.second;

		if (playerNodeState.heuristic > bestPlayerState.heuristic)
			bestPlayerState = playerNodeState;
	}

	//we do exactly the same for the other player
	NodeState bestOtherPlayerState;
	bestOtherPlayerState.heuristic = FLT_MAX;
	for (auto evaluateOtherPlayerState : otherPlayerStates)
	{
		NodeState otherPlayerNodeState;
		otherPlayerNodeState.heuristic = FLT_MIN;
		for (auto evaluateOtherPlayerNodeState : evaluateOtherPlayerState.second)
			if (evaluateOtherPlayerNodeState.second.heuristic > otherPlayerNodeState.heuristic)
				otherPlayerNodeState = evaluateOtherPlayerNodeState.second;

		if (otherPlayerNodeState.heuristic < bestOtherPlayerState.heuristic)
			bestOtherPlayerState = otherPlayerNodeState;
	}

	playerState.Copy(bestPlayerState);
	otherPlayerState.Copy(bestOtherPlayerState);
}

void QuakeAIProcess::Simulation(NodeState& playerState,
	NodeState& otherPlayerState, eastl::vector<PathingArcVec>& otherPlayerPathPlans)
{
	//visibility between pathing transitions
	PathingNode* playerNode = playerState.node;
	PathingNode* otherPlayerNode = otherPlayerState.node;

	eastl::map<ActorId, float> actors;
	if (playerNode->GetActorId() != INVALID_ACTOR_ID)
		actors[playerNode->GetActorId()] = 0.f;

	eastl::map<PathingArcVec, NodeState> playerStates, otherPlayerStates;
	for (PathingArcVec otherPlayerPathPlan : otherPlayerPathPlans)
	{
		float otherPathPlanWeight = 0.f;
		eastl::map<ActorId, float> otherActors;
		if (otherPlayerNode->GetActorId() != INVALID_ACTOR_ID)
			otherActors[otherPlayerNode->GetActorId()] = otherPathPlanWeight;
		for (PathingArc* otherPlayerArc : otherPlayerPathPlan)
		{
			otherPathPlanWeight += otherPlayerArc->GetWeight();
			if (otherPlayerArc->GetNode()->GetActorId() != INVALID_ACTOR_ID)
				otherActors[otherPlayerArc->GetNode()->GetActorId()] = otherPathPlanWeight;
		}

		float visibleTime = 0, visibleDistance = 0, visibleHeight = 0;
		float otherVisibleTime = 0, otherVisibleDistance = 0, otherVisibleHeight = 0;
		Visibility(otherPlayerNode, otherPlayerPathPlan,
			&otherVisibleTime, &otherVisibleDistance, &otherVisibleHeight,
			playerNode, &visibleTime, &visibleDistance, &visibleHeight);

		//calculate damage
		NodeState playerNodeState(playerState);
		playerNodeState.node = playerNode;
		Damage(playerNodeState, visibleTime, visibleDistance, visibleHeight);

		eastl::map<ActorId, float> pathActors;
		for (auto actor : actors)
		{
			if (CanItemBeGrabbed(actor.first, actor.second, playerState))
			{
				if (otherActors.find(actor.first) != otherActors.end())
				{
					if (!CanItemBeGrabbed(actor.first, actor.second, otherPlayerState) ||
						otherActors[actor.first] > actor.second)
					{
						pathActors[actor.first] = actor.second;
					}
				}
				else pathActors[actor.first] = actor.second;
			}
		}
		PickupItem(playerNodeState, pathActors);

		NodeState otherPlayerNodeState(otherPlayerState);
		otherPlayerNodeState.path = otherPlayerPathPlan;
		otherPlayerNodeState.node = otherPlayerPathPlan.back()->GetNode();
		Damage(otherPlayerNodeState, otherVisibleTime, otherVisibleDistance, otherVisibleHeight);

		eastl::map<ActorId, float> otherPathActors;
		for (auto otherActor : otherActors)
		{
			if (CanItemBeGrabbed(otherActor.first, otherActor.second, otherPlayerState))
			{
				if (actors.find(otherActor.first) != actors.end())
				{
					if (!CanItemBeGrabbed(otherActor.first, otherActor.second, playerState) ||
						actors[otherActor.first] > otherActor.second)
					{
						otherPathActors[otherActor.first] = otherActor.second;
					}
				}
				else otherPathActors[otherActor.first] = otherActor.second;
			}
		}
		PickupItem(otherPlayerNodeState, otherPathActors);

		//we calculate the heuristic
		Heuristic(playerNodeState, otherPlayerNodeState);
		playerStates[otherPlayerPathPlan] = playerNodeState;
		otherPlayerStates[otherPlayerPathPlan] = otherPlayerNodeState;
	}

	//we proceed with the minimax algorithm between players
	NodeState bestPlayerState, bestOtherPlayerState;
	bestOtherPlayerState.heuristic = FLT_MIN;
	for (auto evaluateOtherPlayerState : otherPlayerStates)
	{
		if (evaluateOtherPlayerState.second.heuristic > bestOtherPlayerState.heuristic)
		{
			bestPlayerState = playerStates[evaluateOtherPlayerState.first];
			bestOtherPlayerState = evaluateOtherPlayerState.second;
		}
	}

	playerState.Copy(bestPlayerState);
	otherPlayerState.Copy(bestOtherPlayerState);
}

void QuakeAIProcess::Simulation(NodeState& playerState, NodeState& otherPlayerState)
{
	//visibility between pathing transitions
	PathingNode* playerNode = playerState.node;
	PathingNode* otherPlayerNode = otherPlayerState.node;
	float visibleDistance = Length(otherPlayerNode->GetPos() - playerNode->GetPos());
	float visibleHeight = (playerNode->GetPos()[2] - otherPlayerNode->GetPos()[2]);
	float visibleTime = 4.0f;

	float otherVisibleDistance = Length(otherPlayerNode->GetPos() - playerNode->GetPos());
	float otherVisibleHeight = (otherPlayerNode->GetPos()[2] - playerNode->GetPos()[2]);
	float otherVisibleTime = 4.0f;

	//calculate damage
	playerState.node = playerNode;
	if (playerNode->IsVisibleNode(otherPlayerNode))
	{
		playerState.target = otherPlayerState.player;
		Damage(playerState, visibleTime, visibleDistance, visibleHeight);
	}

	eastl::map<ActorId, float> pathActors;
	if (playerNode->GetActorId() != INVALID_ACTOR_ID)
	{
		if (CanItemBeGrabbed(playerNode->GetActorId(), 0.f, playerState))
			pathActors[playerNode->GetActorId()] = 0.f;
	}
	PickupItem(playerState, pathActors);

	otherPlayerState.node = otherPlayerNode;
	if (otherPlayerNode->IsVisibleNode(playerNode))
	{
		otherPlayerState.target = playerState.player;
		Damage(otherPlayerState, otherVisibleTime, otherVisibleDistance, otherVisibleHeight);
	}

	eastl::map<ActorId, float> otherPathActors;
	if (otherPlayerNode->GetActorId() != INVALID_ACTOR_ID)
	{
		if (CanItemBeGrabbed(otherPlayerNode->GetActorId(), 0.f, otherPlayerState))
			otherPathActors[otherPlayerNode->GetActorId()] = 0.f;
	}
	PickupItem(otherPlayerState, otherPathActors);

	Heuristic(playerState, otherPlayerState);
}

void QuakeAIProcess::ConstructPath(PathingNode* playerClusterNode, unsigned int playerClusterType,
	eastl::map<PathingNode*, float>& playerVisibleNodes, eastl::vector<PathingArcVec>& playerPathPlan)
{
	//lets traverse the closest cluster nodes and actors from our current position
	//it is the best way to expand the area uniformely
	PathingClusterVec pathClusters;
	playerClusterNode->GetClusters(playerClusterType, 10, pathClusters);
	for (PathingCluster* pathCluster : pathClusters)
	{
		PathingArcVec pathPlan;
		PathingNode* currentNode = playerClusterNode;
		if (playerVisibleNodes.find(currentNode) != playerVisibleNodes.end())
			playerVisibleNodes.erase(currentNode);
		while (currentNode != pathCluster->GetTarget())
		{
			PathingCluster* currentCluster = 
				currentNode->FindCluster(playerClusterType, pathCluster->GetTarget());
			PathingArc* currentArc = currentNode->FindArc(currentCluster->GetNode());
			PathingTransition* currentTransition = currentNode->FindTransition(currentArc->GetId());
			for (PathingNode* nodeTransition : currentTransition->GetNodes())
				if (playerVisibleNodes.find(nodeTransition) != playerVisibleNodes.end())
					playerVisibleNodes.erase(nodeTransition);

			pathPlan.push_back(currentArc);
			currentNode = currentArc->GetNode();
			if (playerVisibleNodes.find(currentNode) != playerVisibleNodes.end())
				playerVisibleNodes.erase(currentNode);
		}
		playerPathPlan.push_back(pathPlan);
	}

	//if there are missing visible nodes we search deeper
	if (!playerVisibleNodes.empty())
	{
		unsigned int clusterSize = pathClusters.size();
		eastl::vector<PathingNodeArcMap> pathNodePlan = eastl::vector<PathingNodeArcMap>(clusterSize);
		for (unsigned int clusterIdx = 0; clusterIdx < clusterSize; clusterIdx++)
		{
			PathingCluster* pathCluster = pathClusters[clusterIdx];

			PathingArcVec pathPlan;
			PathingNode* currentNode = playerClusterNode;
			while (currentNode != pathCluster->GetTarget())
			{
				for (PathingArc* currentArc : currentNode->GetArcs())
				{
					if (pathNodePlan[clusterIdx].find(currentArc->GetNode()) == pathNodePlan[clusterIdx].end())
					{
						bool addPath = false;
						PathingTransition* currentTransition = currentNode->FindTransition(currentArc->GetId());
						for (PathingNode* nodeTransition : currentTransition->GetNodes())
							if (playerVisibleNodes.find(nodeTransition) != playerVisibleNodes.end())
								addPath = true;

						if (addPath || playerVisibleNodes.find(currentArc->GetNode()) != playerVisibleNodes.end())
						{
							for (PathingArcVec::iterator itPath = pathPlan.begin(); itPath != pathPlan.end(); itPath++)
								pathNodePlan[clusterIdx][currentArc->GetNode()].push_back(*itPath);

							pathNodePlan[clusterIdx][currentArc->GetNode()].push_back(currentArc);
						}
					}
				}
				PathingCluster* currentCluster = currentNode->FindCluster(playerClusterType, pathCluster->GetTarget());
				PathingArc* currentArc = currentNode->FindArc(currentCluster->GetNode());

				pathPlan.push_back(currentArc);
				currentNode = currentArc->GetNode();
			}
		}

		//select a minimum of missing visible nodes
		unsigned int minimumVisibleNodes = 0;
		bool foundVisibleNode = false;
		do
		{
			foundVisibleNode = false;
			for (unsigned int clusterIdx = 0; clusterIdx < clusterSize; clusterIdx++)
			{
				for (auto playerVisibleNode : playerVisibleNodes)
				{
					PathingNode* visibleNode = playerVisibleNode.first;
					if (pathNodePlan[clusterIdx].find(visibleNode) != pathNodePlan[clusterIdx].end())
					{
						minimumVisibleNodes++;
						foundVisibleNode = true;
						playerVisibleNodes.erase(visibleNode);

						PathingArcVec pathPlan = pathNodePlan[clusterIdx][visibleNode];
						playerPathPlan.push_back(pathPlan);
						break;
					}
				}
			}
			if (!foundVisibleNode) break;

		} while (minimumVisibleNodes < 10);
	}
}

void QuakeAIProcess::EvaluateNode(NodeState& playerState, NodeState& otherPlayerState)
{
	Simulation(playerState, otherPlayerState);
}

void QuakeAIProcess::EvaluateNode(
	NodeState& playerState, NodeState& otherPlayerState,
	PathingCluster* otherPlayerCluster, unsigned int otherPlayerClusterType)
{
	//first we find those nodes which contains actor or/and were visible
	eastl::map<PathingNode*, float> otherVisibleNodes;
	eastl::map<PathingNode*, float> otherNodes;
	otherNodes[otherPlayerCluster->GetNode()] = 0.f;

	PathingNode* currentNode = otherPlayerCluster->GetNode();
	while (currentNode != otherPlayerCluster->GetTarget())
	{
		for (PathingArc* currentArc : currentNode->GetArcs())
		{
			PathingTransition* currentTransition = currentNode->FindTransition(currentArc->GetId());
			for (PathingNode* nodeTransition : currentTransition->GetNodes())
				if (nodeTransition->IsVisibleNode(playerState.node))
					otherVisibleNodes[nodeTransition] = currentArc->GetWeight();

			otherNodes[currentArc->GetNode()] = currentArc->GetWeight();
			if (currentArc->GetNode()->IsVisibleNode(playerState.node))
				otherVisibleNodes[currentArc->GetNode()] = currentArc->GetWeight();
		}

		PathingCluster* currentCluster = 
			currentNode->FindCluster(otherPlayerClusterType, otherPlayerCluster->GetTarget());
		PathingArc* currentArc = currentNode->FindArc(currentCluster->GetNode());

		PathingTransition* currentTransition = currentNode->FindTransition(currentArc->GetId());
		for (PathingNode* nodeTransition : currentTransition->GetNodes())
			if (nodeTransition->IsVisibleNode(playerState.node))
				otherVisibleNodes[nodeTransition] = currentArc->GetWeight();

		otherNodes[currentArc->GetNode()] = currentArc->GetWeight();
		if (currentArc->GetNode()->IsVisibleNode(playerState.node))
			otherVisibleNodes[currentArc->GetNode()] = currentArc->GetWeight();

		currentNode = currentArc->GetNode();
	}

	// next we construct path
	eastl::vector<PathingArcVec> otherNodePathPlans;
	ConstructPath(otherPlayerCluster->GetNode(), 
		otherPlayerClusterType, otherVisibleNodes, otherNodePathPlans);

	otherPlayerState.node = otherPlayerCluster->GetNode();
	Simulation(playerState, otherPlayerState, otherNodePathPlans);
}

void QuakeAIProcess::EvaluateNode(
	NodeState& playerState, NodeState& otherPlayerState,
	PathingCluster* playerCluster, unsigned int playerClusterType,
	PathingCluster* otherPlayerCluster, unsigned int otherPlayerClusterType)
{
	//first we find those nodes which contains actor or/and were visible
	eastl::map<PathingNode*, float> nodes;
	nodes[playerCluster->GetNode()] = 0.f;

	PathingNode* currentNode = playerCluster->GetNode();
	while (currentNode != playerCluster->GetTarget())
	{
		for (PathingArc* currentArc : currentNode->GetArcs())
		{
			PathingTransition* currentTransition = 
				currentNode->FindTransition(currentArc->GetId());
			for (PathingNode* nodeTransition : currentTransition->GetNodes())
				nodes[nodeTransition] = currentArc->GetWeight();

			nodes[currentArc->GetNode()] = currentArc->GetWeight();
		}

		PathingCluster* currentCluster = 
			currentNode->FindCluster(playerClusterType, playerCluster->GetTarget());
		PathingArc* currentArc = currentNode->FindArc(currentCluster->GetNode());
		currentNode = currentArc->GetNode();
	}

	eastl::map<PathingNode*, float> visibleNodes;
	eastl::map<PathingNode*, float> otherVisibleNodes;
	currentNode = otherPlayerCluster->GetNode();
	while (currentNode != otherPlayerCluster->GetTarget())
	{
		for (PathingArc* currentArc : currentNode->GetArcs())
		{
			PathingTransition* currentTransition = currentNode->FindTransition(currentArc->GetId());
			for (auto playerNode : nodes)
			{
				for (PathingNode* nodeTransition : currentTransition->GetNodes())
				{
					if (nodeTransition->IsVisibleNode(playerNode.first))
					{
						visibleNodes[playerNode.first] = currentArc->GetWeight();
						otherVisibleNodes[currentArc->GetNode()] = currentArc->GetWeight();
					}
				}

				if (currentArc->GetNode()->IsVisibleNode(playerNode.first))
				{
					visibleNodes[playerNode.first] = currentArc->GetWeight();
					otherVisibleNodes[currentArc->GetNode()] = currentArc->GetWeight();
				}
			}
		}

		PathingCluster* currentCluster = 
			currentNode->FindCluster(otherPlayerClusterType, otherPlayerCluster->GetTarget());
		PathingArc* currentArc = currentNode->FindArc(currentCluster->GetNode());
		currentNode = currentArc->GetNode();
	}

	// next we construct path for those nodes which were visible
	eastl::vector<PathingArcVec> nodePathPlans, otherNodePathPlans;
	ConstructPath(playerCluster->GetNode(), 
		playerClusterType, visibleNodes, nodePathPlans);
	ConstructPath(otherPlayerCluster->GetNode(), 
		otherPlayerClusterType, otherVisibleNodes, otherNodePathPlans);

	playerState.node = playerCluster->GetNode();
	otherPlayerState.node = otherPlayerCluster->GetNode();
	Simulation(playerState, nodePathPlans, otherPlayerState, otherNodePathPlans);
}

void QuakeAIProcess::EvaluateCluster(
	NodeState& playerState, NodeState& otherPlayerState, unsigned int* iteration)
{
	(*iteration)++;
	printf("it %u \n", (*iteration));

	//single case playerNode - otherPlayerNode
	{
		NodeState state(playerState);
		NodeState otherState(otherPlayerState);
		EvaluateNode(state, otherState);

		mPlayerState.Copy(state);
		mOtherPlayerState.Copy(otherPlayerState);
	}

	PathingClusterVec playerClusters;
	unsigned int playerClusterType = GAT_JUMP;
	playerState.node->GetClusters(playerClusterType, 10, playerClusters);
	unsigned int clusterSize = playerClusters.size();

	eastl::map<PathingCluster*, NodeState> playerClusterStates;
	//for (unsigned int playerClusterIdx = 0; playerClusterIdx < clusterSize; playerClusterIdx++)
	parallel_for(size_t(0), clusterSize, [&](size_t playerClusterIdx)
	{
		PathingCluster* playerCluster = playerClusters[playerClusterIdx];

		NodeState state(playerState);
		NodeState otherState(otherPlayerState);
		EvaluateNode(otherState, state, playerCluster, playerClusterType);
		state.heuristic = -state.heuristic;
		otherState.heuristic = -otherState.heuristic;

		if (state.valid && otherState.valid)
			playerClusterStates[playerCluster] = state;
	});

	//minimax
	for (auto playerClusterState : playerClusterStates)
	{
		if (playerClusterState.second.heuristic > mPlayerState.heuristic)
			mPlayerState.Copy(playerClusterState.second);
	}

	PathingClusterVec otherPlayerClusters;
	unsigned int otherPlayerClusterType = GAT_JUMP;
	otherPlayerState.node->GetClusters(otherPlayerClusterType, 10, otherPlayerClusters);
	unsigned int otherClusterSize = otherPlayerClusters.size();

	eastl::map<PathingCluster*, NodeState> otherPlayerClusterStates;
	//for (unsigned int otherPlayerClusterIdx = 0; otherPlayerClusterIdx < otherClusterSize; otherPlayerClusterIdx++)
	parallel_for(size_t(0), otherClusterSize, [&](size_t otherPlayerClusterIdx)
	{
		PathingCluster* otherPlayerCluster = otherPlayerClusters[otherPlayerClusterIdx];

		NodeState state(playerState);
		NodeState otherState(otherPlayerState);
		EvaluateNode(state, otherState, otherPlayerCluster, otherPlayerClusterType);

		if (state.valid && otherState.valid)
			otherPlayerClusterStates[otherPlayerCluster] = otherState;
	});

	//minimax
	for (auto otherPlayerClusterState : otherPlayerClusterStates)
	{
		if (otherPlayerClusterState.second.heuristic < mOtherPlayerState.heuristic)
			mOtherPlayerState.Copy(otherPlayerClusterState.second);
	}

	eastl::map<PathingCluster*, eastl::map<PathingCluster*, NodeState>> playerClustersStates, otherPlayerClustersStates;
	//for (unsigned int playerClusterIdx = 0; playerClusterIdx < clusterSize; playerClusterIdx++)
	parallel_for(size_t(0), clusterSize, [&](size_t playerClusterIdx)
	{
		PathingCluster* playerCluster = playerClusters[playerClusterIdx];
		//for (unsigned int otherPlayerClusterIdx = 0; otherPlayerClusterIdx < otherClusterSize; otherPlayerClusterIdx++)
		parallel_for(size_t(0), otherClusterSize, [&](size_t otherPlayerClusterIdx)
		{
			PathingCluster* otherPlayerCluster = otherPlayerClusters[otherPlayerClusterIdx];

			NodeState state(playerState);
			NodeState otherState(otherPlayerState);
			EvaluateNode(state, otherState, playerCluster, playerClusterType, otherPlayerCluster, otherPlayerClusterType);

			if (state.valid && otherState.valid)
			{
				playerClustersStates[playerCluster][otherPlayerCluster] = state;
				otherPlayerClustersStates[otherPlayerCluster][playerCluster] = otherState;
			}
		});
	});

	//minimax
	for (auto playerClustersState : playerClustersStates)
	{
		NodeState playerNodeState;
		playerNodeState.heuristic = FLT_MAX;
		for (auto playerClusterState : playerClustersState.second)
			if (playerClusterState.second.heuristic < playerNodeState.heuristic)
				playerNodeState = playerClusterState.second;

		if (playerNodeState.heuristic > mPlayerState.heuristic)
			mPlayerState.Copy(playerNodeState);
	}

	for (auto otherPlayerClustersState : otherPlayerClustersStates)
	{
		NodeState otherPlayerNodeState;
		otherPlayerNodeState.heuristic = FLT_MIN;
		for (auto otherPlayerClusterState : otherPlayerClustersState.second)
			if (otherPlayerClusterState.second.heuristic > otherPlayerNodeState.heuristic)
				otherPlayerNodeState = otherPlayerClusterState.second;

		if (otherPlayerNodeState.heuristic < mOtherPlayerState.heuristic)
			mOtherPlayerState.Copy(otherPlayerNodeState);
	}
}

void QuakeAIProcess::ThreadProc( )
{
	unsigned int iteration = 0;
	while (true)
	{
		eastl::map<GameViewType, eastl::vector<eastl::shared_ptr<PlayerActor>>> players;

		GameApplication* gameApp = (GameApplication*)Application::App;
		const GameViewList& gameViews = gameApp->GetGameViews();
		for (auto it = gameViews.begin(); it != gameViews.end(); ++it)
		{
			eastl::shared_ptr<BaseGameView> pView = *it;
			if (pView->GetActorId() != INVALID_ACTOR_ID)
			{
				players[pView->GetType()].push_back(
					eastl::dynamic_shared_pointer_cast<PlayerActor>(
					GameLogic::Get()->GetActor(pView->GetActorId()).lock()));
			}
		}

		if (players.find(GV_HUMAN) != players.end() && players.find(GV_AI) != players.end())
		{
			eastl::shared_ptr<PathingGraph> pathingGraph =
				GameLogic::Get()->GetAIManager()->GetPathingGraph();

			eastl::map<eastl::shared_ptr<PlayerActor>, PathingNode*> playerNodes, aiNodes;
			for (eastl::shared_ptr<PlayerActor> pPlayerActor : players[GV_HUMAN])
			{
				eastl::shared_ptr<TransformComponent> pTransformComponent(
					pPlayerActor->GetComponent<TransformComponent>(TransformComponent::Name).lock());
				if (pTransformComponent)
				{
					playerNodes[pPlayerActor] = 
						pathingGraph->FindClosestNode(pTransformComponent->GetPosition());
				}
			}

			for (eastl::shared_ptr<PlayerActor> pPlayerActor : players[GV_AI])
			{
				eastl::shared_ptr<TransformComponent> pTransformComponent(
					pPlayerActor->GetComponent<TransformComponent>(TransformComponent::Name).lock());
				if (pTransformComponent)
				{
					aiNodes[pPlayerActor] =
						pathingGraph->FindClosestNode(pTransformComponent->GetPosition());
				}
			}

			for (auto playerNode : playerNodes)
			{
				NodeState playerState(playerNode.first);
				playerState.node = playerNode.second;
				for (auto aiNode : aiNodes)
				{
					NodeState aiPlayerState(aiNode.first);
					aiPlayerState.node = aiNode.second;
					EvaluateCluster(playerState, aiPlayerState, &iteration);
				}
			}
		}
		/*
		EventManager::Get()->QueueEvent(
			eastl::shared_ptr<QuakeEventDataAIDecisionMaking>(new QuakeEventDataAIDecisionMaking()));
		*/
	}

	Succeed();
}