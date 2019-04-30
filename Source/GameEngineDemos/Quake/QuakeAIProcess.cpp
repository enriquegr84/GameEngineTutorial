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

QuakeAIProcess::QuakeAIProcess()
  :  RealtimeProcess()
{

}

QuakeAIProcess::~QuakeAIProcess(void)
{

}

float QuakeAIProcess::Heuristic(NodeState& playerState, NodeState& otherPlayerState)
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
			maxDamage = playerState.damage[weapon - 1];
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
			maxDamage = otherPlayerState.damage[weapon - 1];
	}

	//damage threshold to the max hp/armor
	if (maxDamage > 400) maxDamage = 400;
	heuristic -= (maxDamage / 400);
	return heuristic;
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
	PathingNode* otherPlayerNode, PathingArcVec& otherPlayerPathPlan,
	float* visibleTime, float* visibleDistance, float* visibleHeight,
	float* otherVisibleTime, float* otherVisibleDistance, float* otherVisibleHeight)
{
	float visibleWeight = 0.f;
	float totalTime = 0.f, totalArcTime = 0.f;
	unsigned int index = 0, otherIndex = 0, otherPathIndex = 0;

	PathingTransition* playerTransition = NULL;
	PathingTransition* otherPlayerTransition =
		otherPlayerNode->FindTransition(otherPlayerPathPlan[otherPathIndex]->GetId());
	for (PathingArc* playerArc : playerPathPlan)
	{
		playerTransition = playerNode->FindTransition(playerArc->GetId());
		for (; index < playerTransition->GetNodes().size(); index++)
		{
			visibleWeight = playerTransition->GetWeights()[index];

			if (playerTransition->GetNodes()[index]->IsVisibleNode(
				otherPlayerTransition->GetNodes()[otherIndex]))
			{
				//we only take into consideration visibility from ground
				if (playerArc->GetType() == GAT_MOVE)
				{
					(*visibleDistance) += Length(
						otherPlayerTransition->GetConnections()[otherIndex] -
						playerTransition->GetConnections()[index]) * visibleWeight;
					(*visibleHeight) +=
						(playerTransition->GetConnections()[index][2] -
						otherPlayerTransition->GetConnections()[otherIndex][2]) * visibleWeight;
					(*visibleTime) += visibleWeight;
				}
				if (otherPlayerPathPlan[otherPathIndex]->GetType() == GAT_MOVE)
				{
					(*otherVisibleDistance) += Length(
						otherPlayerTransition->GetConnections()[otherIndex] -
						playerTransition->GetConnections()[index]) * visibleWeight;
					(*otherVisibleHeight) +=
						(otherPlayerTransition->GetConnections()[otherIndex][2] -
						playerTransition->GetConnections()[index][2]) * visibleWeight;
					(*otherVisibleTime) += visibleWeight;
				}
			}
			while (totalArcTime <= totalTime)
			{
				totalArcTime += otherPlayerTransition->GetWeights()[otherIndex];
				if (otherIndex + 1 >= otherPlayerTransition->GetNodes().size())
				{
					if (otherPathIndex < otherPlayerPathPlan.size())
					{
						otherIndex = 0;
						otherPathIndex++;
						otherPlayerNode = otherPlayerPathPlan[otherPathIndex]->GetNode();
						otherPlayerTransition = otherPlayerNode->FindTransition(
							otherPlayerPathPlan[otherPathIndex]->GetId());
					}
					else break;
				}
				else otherIndex++;
			}
			totalTime += visibleWeight;
		}
		playerNode = playerArc->GetNode();
	}

	if (playerTransition)
	{
		index--;
		otherPlayerTransition =
			otherPlayerNode->FindTransition(otherPlayerPathPlan[otherPathIndex]->GetId());
		for (; otherIndex < otherPlayerTransition->GetNodes().size(); otherIndex++)
		{
			visibleWeight = playerTransition->GetWeights()[index];

			if (playerTransition->GetNodes()[index]->IsVisibleNode(
				otherPlayerTransition->GetNodes()[otherIndex]))
			{
				if (playerPathPlan.back()->GetType() == GAT_MOVE)
				{
					(*visibleDistance) += Length(
						otherPlayerTransition->GetConnections()[otherIndex] -
						playerTransition->GetConnections()[index]) * visibleWeight;
					(*visibleHeight) +=
						(playerTransition->GetConnections()[index][2] -
						otherPlayerTransition->GetConnections()[otherIndex][2]) * visibleWeight;
					(*visibleTime) += visibleWeight;
				}
				if (otherPlayerPathPlan[otherPathIndex]->GetType() == GAT_MOVE)
				{
					(*otherVisibleDistance) += Length(
						otherPlayerTransition->GetConnections()[otherIndex] -
						playerTransition->GetConnections()[index]) * visibleWeight;
					(*otherVisibleHeight) +=
						(otherPlayerTransition->GetConnections()[otherIndex][2] -
						playerTransition->GetConnections()[index][2]) * visibleWeight;
					(*otherVisibleTime) += visibleWeight;
				}
			}
		}

		//lets put a minimum of potential visibility time
		otherIndex--;
		if (totalTime < 4.0f)
		{
			visibleWeight = 4.0f - totalTime;

			if (playerTransition->GetNodes()[index]->IsVisibleNode(
				otherPlayerTransition->GetNodes()[otherIndex]))
			{
				if (playerPathPlan.back()->GetType() == GAT_MOVE)
				{
					(*visibleDistance) += Length(
						otherPlayerTransition->GetConnections()[otherIndex] -
						playerTransition->GetConnections()[index]) * visibleWeight;
					(*visibleHeight) +=
						(playerTransition->GetConnections()[index][2] -
						otherPlayerTransition->GetConnections()[otherIndex][2]) * visibleWeight;
					(*visibleTime) += visibleWeight;
				}
			}
			if (playerTransition->GetNodes()[index]->IsVisibleNode(
				otherPlayerTransition->GetNodes()[otherIndex]))
			{
				if (otherPlayerPathPlan[otherPathIndex]->GetType() == GAT_MOVE)
				{
					(*otherVisibleDistance) += Length(
						otherPlayerTransition->GetConnections()[otherIndex] -
						playerTransition->GetConnections()[index]) * visibleWeight;
					(*otherVisibleHeight) +=
						(otherPlayerTransition->GetConnections()[otherIndex][2] -
							playerTransition->GetConnections()[index][2]) * visibleWeight;
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

void QuakeAIProcess::Visibility(PathingNode* otherPlayerNode, 
	PathingNode* playerNode, PathingArcVec& playerPathPlan,
	float* visibleTime, float* visibleDistance, float* visibleHeight,
	float* otherVisibleTime, float* otherVisibleDistance, float* otherVisibleHeight)
{
	float visibleWeight = 0.f;
	float totalTime = 0.f;
	unsigned int index = 0;

	PathingTransition* playerTransition = NULL;
	for (PathingArc* playerArc : playerPathPlan)
	{
		playerTransition = playerNode->FindTransition(playerArc->GetId());
		for (; index < playerTransition->GetNodes().size(); index++)
		{
			visibleWeight = playerTransition->GetWeights()[index];

			if (playerTransition->GetNodes()[index]->IsVisibleNode(otherPlayerNode))
			{
				//we only take into consideration visibility from ground
				if (playerArc->GetType() == GAT_MOVE)
				{
					(*visibleDistance) += Length(otherPlayerNode->GetPos() -
						playerTransition->GetConnections()[index]) * visibleWeight;
					(*visibleHeight) +=
						(playerTransition->GetConnections()[index][2] -
						otherPlayerNode->GetPos()[2]) * visibleWeight;
					(*visibleTime) += visibleWeight;
				}

				(*otherVisibleDistance) += Length(otherPlayerNode->GetPos() -
					playerTransition->GetConnections()[index]) * visibleWeight;
				(*otherVisibleHeight) += (otherPlayerNode->GetPos()[2] -
					playerTransition->GetConnections()[index][2]) * visibleWeight;
				(*otherVisibleTime) += visibleWeight;
			}
			totalTime += visibleWeight;
		}
		playerNode = playerArc->GetNode();
	}

	if (playerTransition)
	{
		index--;

		//lets put a minimum of potential visibility time
		if (totalTime < 4.0f)
		{
			visibleWeight = 4.0f - totalTime;

			if (playerTransition->GetNodes()[index]->IsVisibleNode(otherPlayerNode))
			{
				if (playerPathPlan.back()->GetType() == GAT_MOVE)
				{
					(*visibleDistance) += Length(otherPlayerNode->GetPos() -
						playerTransition->GetConnections()[index]) * visibleWeight;
					(*visibleHeight) +=
						(playerTransition->GetConnections()[index][2] -
						otherPlayerNode->GetPos()[2]) * visibleWeight;
					(*visibleTime) += visibleWeight;
				}
			}

			(*otherVisibleDistance) += Length(otherPlayerNode->GetPos() -
				playerTransition->GetConnections()[index]) * visibleWeight;
			(*otherVisibleHeight) += (otherPlayerNode->GetPos()[2] -
				playerTransition->GetConnections()[index][2]) * visibleWeight;
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
	eastl::map<PathingArcVec, eastl::map<NodeState*, float>> playerHeuristics, otherPlayerHeuristics;
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
				Visibility(playerNode, playerPathPlan, otherPlayerNode, otherPlayerPathPlan, 
					&visibleTime, &visibleDistance, &visibleHeight,
					&otherVisibleTime, &otherVisibleDistance, &otherVisibleHeight);
			}
			else
			{
				Visibility(otherPlayerNode, otherPlayerPathPlan, playerNode, playerPathPlan, 
					&otherVisibleTime, &otherVisibleDistance, &otherVisibleHeight,
					&visibleTime, &visibleDistance, &visibleHeight);
			}

			//calculate damage
			NodeState playerNodeState(playerState);
			playerNodeState.arc = playerPathPlan.back();
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
			otherPlayerNodeState.arc = otherPlayerPathPlan.back();
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

			float heuristic = Heuristic(playerNodeState, otherPlayerNodeState);
			playerHeuristics[playerPathPlan][&playerNodeState] = heuristic;
			otherPlayerHeuristics[otherPlayerPathPlan][&otherPlayerNodeState] = heuristic;
		}
	}

	eastl::map<NodeState*, float> playerNodeStates;
	for (auto playerHeuristic : playerHeuristics)
	{
		float playerHeuristicAverage = 0.f;
		for (auto playerNodeHeuristic : playerHeuristic.second)
			playerHeuristicAverage += playerNodeHeuristic.second;

		//we have all path combinations between players which occurs simultaneously, we will calculate 
		//the average from all the combination and choose the closest one as the most representative
		NodeState playerHeuristicState;
		float playerHeuristicMinimum = FLT_MAX;
		playerHeuristicAverage /= playerHeuristic.second.size();
		for (auto playerNodeHeuristic : playerHeuristic.second)
		{
			if (abs(playerNodeHeuristic.second - playerHeuristicAverage) < playerHeuristicMinimum)
			{
				playerHeuristicMinimum = playerNodeHeuristic.second;
				playerHeuristicState = (*playerNodeHeuristic.first);
			}
		}
		playerNodeStates[&playerHeuristicState] = playerHeuristicMinimum;
	}

	//finally we choose the best of all the representative nodes
	float heuristicMinimum = FLT_MIN;
	for (auto playerNodeState : playerNodeStates)
	{
		if (playerNodeState.second > heuristicMinimum)
		{
			playerState = (*playerNodeState.first);
			heuristicMinimum = playerNodeState.second;
		}
	}

	//we do exactly the same for the other player
	eastl::map<NodeState*, float> otherPlayerNodeStates;
	for (auto otherPlayerHeuristic : otherPlayerHeuristics)
	{
		float otherPlayerHeuristicAverage = 0.f;
		for (auto otherPlayerNodeHeuristic : otherPlayerHeuristic.second)
			otherPlayerHeuristicAverage += otherPlayerNodeHeuristic.second;

		//we have all path combinations between players which occurs simultaneously, we will calculate 
		//the average from all the combination and choose the closest one as the most representative
		NodeState otherPlayerHeuristicState;
		float otherPlayerHeuristicMinimum = FLT_MAX;
		otherPlayerHeuristicAverage /= otherPlayerHeuristic.second.size();
		for (auto otherPlayerNodeHeuristic : otherPlayerHeuristic.second)
		{
			if (abs(otherPlayerNodeHeuristic.second - otherPlayerHeuristicAverage) < otherPlayerHeuristicMinimum)
			{
				otherPlayerHeuristicMinimum = otherPlayerNodeHeuristic.second;
				otherPlayerHeuristicState = (*otherPlayerNodeHeuristic.first);
			}
		}
		otherPlayerNodeStates[&otherPlayerHeuristicState] = otherPlayerHeuristicMinimum;
	}

	//finally we choose the best of all the representative nodes
	float otherHeuristicMinimum = FLT_MAX;
	for (auto otherPlayerNodeState : otherPlayerNodeStates)
	{
		if (otherPlayerNodeState.second < otherHeuristicMinimum)
		{
			otherPlayerState = (*otherPlayerNodeState.first);
			otherHeuristicMinimum = otherPlayerNodeState.second;
		}
	}
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

	eastl::map<NodeState*, float> playerHeuristics, otherPlayerHeuristics;
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
		Visibility(playerNode, otherPlayerNode, otherPlayerPathPlan,
			&otherVisibleTime, &otherVisibleDistance, &otherVisibleHeight,
			&visibleTime, &visibleDistance, &visibleHeight);

		//calculate damage
		NodeState playerNodeState(playerState);
		playerNodeState.arc = NULL;
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
		otherPlayerNodeState.arc = otherPlayerPathPlan.back();
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

		float heuristic = Heuristic(playerNodeState, otherPlayerNodeState);
		playerHeuristics[&playerNodeState] = heuristic;
		otherPlayerHeuristics[&otherPlayerNodeState] = heuristic;
	}

	if (!playerHeuristics.empty())
	{
		float playerHeuristicAverage = 0.f;
		for (auto playerHeuristic : playerHeuristics)
			playerHeuristicAverage += playerHeuristic.second;

		//we have all path combinations between players which occurs simultaneously, we will calculate 
		//the average from all the combination and choose the closest one as the most representative
		float playerHeuristicMinimum = FLT_MAX;
		playerHeuristicAverage /= playerHeuristics.size();
		for (auto playerHeuristic : playerHeuristics)
		{
			if (abs(playerHeuristic.second - playerHeuristicAverage) < playerHeuristicMinimum)
			{
				playerState = (*playerHeuristic.first);
				playerHeuristicMinimum = playerHeuristic.second;
			}
		}
	}

	//this time we don't need to calculate the average since there is only one state therefore
	//we only have to choose the best one
	float otherHeuristicMinimum = FLT_MAX;
	for (auto otherPlayerHeuristic : otherPlayerHeuristics)
	{
		if (otherPlayerHeuristic.second < otherHeuristicMinimum)
		{
			otherPlayerState = (*otherPlayerHeuristic.first);
			otherHeuristicMinimum = otherPlayerHeuristic.second;
		}
	}
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
	playerState.arc = NULL;
	playerState.node = playerNode;
	if (playerNode->IsVisibleNode(otherPlayerNode))
		Damage(playerState, visibleTime, visibleDistance, visibleHeight);

	eastl::map<ActorId, float> pathActors;
	if (playerNode->GetActorId() != INVALID_ACTOR_ID)
	{
		if (CanItemBeGrabbed(playerNode->GetActorId(), 0.f, playerState))
			pathActors[playerNode->GetActorId()] = 0.f;
	}
	PickupItem(playerState, pathActors);

	otherPlayerState.arc = NULL;
	otherPlayerState.node = otherPlayerNode;
	if (otherPlayerNode->IsVisibleNode(playerNode))
		Damage(otherPlayerState, otherVisibleTime, otherVisibleDistance, otherVisibleHeight);

	eastl::map<ActorId, float> otherPathActors;
	if (otherPlayerNode->GetActorId() != INVALID_ACTOR_ID)
	{
		if (CanItemBeGrabbed(otherPlayerNode->GetActorId(), 0.f, otherPlayerState))
			otherPathActors[otherPlayerNode->GetActorId()] = 0.f;
	}
	PickupItem(otherPlayerState, otherPathActors);
}

void QuakeAIProcess::ConstructPath(PathingNode* playerClusterNode, unsigned int playerClusterType,
	eastl::map<PathingNode*, float>& playerVisibleNodes, eastl::vector<PathingArcVec>& playerPathPlan)
{
	//lets traverse the closest cluster nodes and actors from our current position
	//it is the best way to expand the area uniformly
	PathingClusterVec playerClusters;
	playerClusterNode->GetClusters(playerClusterType, playerClusters);
	unsigned int clusterSize = playerClusters.size() <= 30 ? playerClusters.size() : 30;
	for (unsigned int clusterIdx = 0; clusterIdx < clusterSize; clusterIdx++)
	{
		PathingCluster* pathCluster = playerClusters[clusterIdx];

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
		eastl::vector<PathingNodeArcMap> pathNodePlan = eastl::vector<PathingNodeArcMap>(clusterSize);
		for (unsigned int clusterIdx = 0; clusterIdx < clusterSize; clusterIdx++)
		{
			PathingCluster* pathCluster = playerClusters[clusterIdx];

			PathingArcVec pathPlan;
			PathingNode* currentNode = playerClusterNode;
			while (currentNode != pathCluster->GetTarget())
			{
				for (PathingArc* currentArc : currentNode->GetArcs())
				{
					bool addPath = false;
					PathingTransition* currentTransition = currentNode->FindTransition(currentArc->GetId());
					for (PathingNode* nodeTransition : currentTransition->GetNodes())
						if (playerVisibleNodes.find(nodeTransition) != playerVisibleNodes.end())
							addPath = true;

					if (addPath || playerVisibleNodes.find(currentArc->GetNode()) != playerVisibleNodes.end())
						for (PathingArcVec::iterator itPath = pathPlan.begin(); itPath != pathPlan.end(); itPath++)
							pathNodePlan[clusterIdx][currentArc->GetNode()].push_back(*itPath);
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
						foundVisibleNode = true;
						playerVisibleNodes.erase(visibleNode);
						playerPathPlan.push_back(pathNodePlan[clusterIdx][visibleNode]);
						break;
					}
				}
			}
			if (!foundVisibleNode) break;

		} while (minimumVisibleNodes < 10);
	}
}

void QuakeAIProcess::EvaluateNode(PathingNode* playerNode, PathingNode* otherPlayerNode)
{
	NodeState playerState;
	playerState.node = playerNode;
	NodeState otherPlayerState;
	otherPlayerState.node = otherPlayerNode;
	Simulation(playerState, otherPlayerState);
	mAllies.push_back(playerState);
	mEnemies.push_back(otherPlayerState);
}

void QuakeAIProcess::EvaluateNode(bool isPlayerNode, PathingNode* playerNode,
	PathingCluster* otherPlayerCluster, unsigned int otherPlayerClusterType)
{
	//first we find those nodes which contains actor or/and were visible
	eastl::map<PathingNode*, float> otherPlayerVisibleNodes;
	eastl::map<PathingNode*, float> otherPlayerNodes;
	otherPlayerNodes[otherPlayerCluster->GetNode()] = 0.f;

	PathingNode* currentNode = otherPlayerCluster->GetNode();
	while (currentNode != otherPlayerCluster->GetTarget())
	{
		for (PathingArc* currentArc : currentNode->GetArcs())
		{
			PathingTransition* currentTransition = currentNode->FindTransition(currentArc->GetId());
			for (PathingNode* nodeTransition : currentTransition->GetNodes())
				if (nodeTransition->IsVisibleNode(playerNode))
					otherPlayerVisibleNodes[nodeTransition] = currentArc->GetWeight();

			otherPlayerNodes[currentArc->GetNode()] = currentArc->GetWeight();
			if (currentArc->GetNode()->IsVisibleNode(playerNode))
				otherPlayerVisibleNodes[currentArc->GetNode()] = currentArc->GetWeight();
		}

		PathingCluster* currentCluster = 
			currentNode->FindCluster(otherPlayerClusterType, otherPlayerCluster->GetTarget());
		PathingArc* currentArc = currentNode->FindArc(currentCluster->GetNode());

		PathingTransition* currentTransition = currentNode->FindTransition(currentArc->GetId());
		for (PathingNode* nodeTransition : currentTransition->GetNodes())
			if (nodeTransition->IsVisibleNode(playerNode))
				otherPlayerVisibleNodes[nodeTransition] = currentArc->GetWeight();

		otherPlayerNodes[currentArc->GetNode()] = currentArc->GetWeight();
		if (currentArc->GetNode()->IsVisibleNode(playerNode))
			otherPlayerVisibleNodes[currentArc->GetNode()] = currentArc->GetWeight();

		currentNode = currentArc->GetNode();
	}

	// next we construct path
	eastl::vector<PathingArcVec> otherPlayerPathPlans;
	ConstructPath(otherPlayerCluster->GetNode(), 
		otherPlayerClusterType, otherPlayerVisibleNodes, otherPlayerPathPlans);

	NodeState playerState;
	playerState.node = playerNode;
	NodeState otherPlayerState;
	otherPlayerState.node = otherPlayerCluster->GetNode();
	Simulation(playerState, otherPlayerState, otherPlayerPathPlans);
	if (isPlayerNode)
	{
		mAllies.push_back(playerState);
		mEnemies.push_back(otherPlayerState);
	}
	else
	{
		mAllies.push_back(otherPlayerState);
		mEnemies.push_back(playerState);
	}
}

void QuakeAIProcess::EvaluateNode(
	PathingCluster* playerCluster, unsigned int playerClusterType,
	PathingCluster* otherPlayerCluster, unsigned int otherPlayerClusterType)
{
	//first we find those nodes which contains actor or/and were visible
	eastl::map<PathingNode*, float> playerVisibleNodes;
	eastl::map<PathingNode*, float> playerNodes;
	playerNodes[playerCluster->GetNode()] = 0.f;

	PathingNode* currentNode = playerCluster->GetNode();
	while (currentNode != playerCluster->GetTarget())
	{
		for (PathingArc* currentArc : currentNode->GetArcs())
		{
			PathingTransition* currentTransition = 
				currentNode->FindTransition(currentArc->GetId());
			for (PathingNode* nodeTransition : currentTransition->GetNodes())
				playerNodes[nodeTransition] = currentArc->GetWeight();

			playerNodes[currentArc->GetNode()] = currentArc->GetWeight();
		}

		PathingCluster* currentCluster = 
			currentNode->FindCluster(playerClusterType, playerCluster->GetTarget());
		PathingArc* currentArc = currentNode->FindArc(currentCluster->GetNode());
		currentNode = currentArc->GetNode();
	}

	eastl::map<PathingNode*, float> otherPlayerVisibleNodes;
	eastl::map<PathingNode*, float> otherPlayerNodes;
	otherPlayerNodes[otherPlayerCluster->GetNode()] = 0.f;

	currentNode = otherPlayerCluster->GetNode();
	while (currentNode != otherPlayerCluster->GetTarget())
	{
		for (PathingArc* currentArc : currentNode->GetArcs())
		{
			PathingTransition* currentTransition = currentNode->FindTransition(currentArc->GetId());
			for (PathingNode* nodeTransition : currentTransition->GetNodes())
				otherPlayerNodes[nodeTransition] = currentArc->GetWeight();

			otherPlayerNodes[currentArc->GetNode()] = currentArc->GetWeight();
			for (auto playerNode : playerNodes)
			{
				for (PathingNode* nodeTransition : currentTransition->GetNodes())
				{
					if (nodeTransition->IsVisibleNode(playerNode.first))
					{
						playerVisibleNodes[playerNode.first] = currentArc->GetWeight();
						otherPlayerVisibleNodes[currentArc->GetNode()] = currentArc->GetWeight();
					}
				}

				if (currentArc->GetNode()->IsVisibleNode(playerNode.first))
				{
					playerVisibleNodes[playerNode.first] = currentArc->GetWeight();
					otherPlayerVisibleNodes[currentArc->GetNode()] = currentArc->GetWeight();
				}
			}
		}

		PathingCluster* currentCluster = 
			currentNode->FindCluster(otherPlayerClusterType, otherPlayerCluster->GetTarget());
		PathingArc* currentArc = currentNode->FindArc(currentCluster->GetNode());
		currentNode = currentArc->GetNode();
	}

	// next we construct path for those nodes which were visible
	eastl::vector<PathingArcVec> playerPathPlans, otherPlayerPathPlans;
	ConstructPath(playerCluster->GetNode(), 
		playerClusterType, playerVisibleNodes, playerPathPlans);
	ConstructPath(otherPlayerCluster->GetNode(), 
		otherPlayerClusterType, otherPlayerVisibleNodes, otherPlayerPathPlans);

	NodeState playerState;
	playerState.node = playerCluster->GetNode();
	NodeState otherPlayerState;
	otherPlayerState.node = otherPlayerCluster->GetNode();
	Simulation(playerState, playerPathPlans, otherPlayerState, otherPlayerPathPlans);
	mAllies.push_back(playerState);
	mEnemies.push_back(otherPlayerState);
}

void QuakeAIProcess::EvaluateCluster(
	PathingNode* playerNode, PathingNode* otherPlayerNode, unsigned int* iteration)
{
	(*iteration)++;
	printf("it %u \n", (*iteration));

	//single case playerNode - otherPlayerNode
	EvaluateNode(playerNode, otherPlayerNode);

	PathingClusterVec playerClusters;
	unsigned int playerClusterType = GAT_JUMP;
	playerNode->GetClusters(playerClusterType, playerClusters);
	unsigned int clusterSize = playerClusters.size() <= 30 ? playerClusters.size() : 30;
	for (unsigned int playerClusterIdx = 0; playerClusterIdx < clusterSize; playerClusterIdx++)
	{
		PathingCluster* playerCluster = playerClusters[playerClusterIdx];
		EvaluateNode(false, otherPlayerNode, playerCluster, playerClusterType);

		//EvaluateCluster(playerCluster->GetTarget(), otherPlayerNode, iteration);
	}

	PathingClusterVec otherPlayerClusters;
	unsigned int otherPlayerClusterType = GAT_JUMP;
	otherPlayerNode->GetClusters(otherPlayerClusterType, otherPlayerClusters);
	unsigned int otherClusterSize = otherPlayerClusters.size() <= 30 ? otherPlayerClusters.size() : 30;
	for (unsigned int otherPlayerClusterIdx = 0; otherPlayerClusterIdx < otherClusterSize; otherPlayerClusterIdx++)
	{
		PathingCluster* otherPlayerCluster = otherPlayerClusters[otherPlayerClusterIdx];
		EvaluateNode(true, playerNode, otherPlayerCluster, otherPlayerClusterType);

		//EvaluateCluster(playerNode, otherPlayerCluster->GetTarget(), iteration);
	}

	for (unsigned int playerClusterIdx = 0; playerClusterIdx < clusterSize; playerClusterIdx++)
	{
		PathingCluster* playerCluster = playerClusters[playerClusterIdx];
		for (unsigned int otherPlayerClusterIdx = 0; otherPlayerClusterIdx < otherClusterSize; otherPlayerClusterIdx++)
		{
			PathingCluster* otherPlayerCluster = otherPlayerClusters[otherPlayerClusterIdx];
			EvaluateNode(playerCluster, playerClusterType, otherPlayerCluster, otherPlayerClusterType);

			//EvaluateCluster(playerCluster->GetTarget(), otherPlayerCluster->GetTarget(), iteration);
		}
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
				for (auto aiNode : aiNodes)
				{
					EvaluateCluster(playerNode.second, aiNode.second, &iteration);
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