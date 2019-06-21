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
#include "QuakeAIProcess.h"

#include "Core/OS/OS.h"

QuakeAIProcess::QuakeAIProcess()
  :  RealtimeProcess()
{
	mFile = fopen("test.txt", "w");

	mAIManager = dynamic_cast<QuakeAIManager*>(
		GameLogic::Get()->GetAIManager());
}

QuakeAIProcess::~QuakeAIProcess(void)
{
	fclose(mFile);
}

float QuakeAIProcess::HeuristicPlayerItems(NodeState& playerState)
{
	float heuristic = 0.f;
	float maxDistance = 4.0f;
	float distance = 0.f;
	int maxAmmo = 0;
	int ammo = 0;

	//heuristic from picked up items
	for (eastl::shared_ptr<Actor> item : playerState.items)
	{
		if (item->GetType() == "Weapon")
		{
			eastl::shared_ptr<WeaponPickup> pWeaponPickup =
				item->GetComponent<WeaponPickup>(WeaponPickup::Name).lock();

			switch (pWeaponPickup->GetCode())
			{
				case WP_LIGHTNING:
					maxAmmo = 60;

					distance = (playerState.itemDistance[item] < maxDistance) ?
						playerState.itemDistance[item] : maxDistance;
					ammo = (playerState.itemAmount[item] < maxAmmo) ?
						playerState.itemAmount[item] : maxAmmo;

					//relation based on amount gained and distance travelled
					heuristic += (ammo / (float)maxAmmo) * (1.0f - (distance / (float)maxDistance)) * 0.2f;
					break;
				case WP_SHOTGUN:
					maxAmmo = 10;

					distance = (playerState.itemDistance[item] < maxDistance) ?
						playerState.itemDistance[item] : maxDistance;
					ammo = (playerState.itemAmount[item] < maxAmmo) ?
						playerState.itemAmount[item] : maxAmmo;

					//relation based on amount gained and distance travelled
					heuristic += (ammo / (float)maxAmmo) * (1.0f - (distance / (float)maxDistance)) * 0.2f;
					break;
				case WP_MACHINEGUN:
					maxAmmo = 50;

					distance = (playerState.itemDistance[item] < maxDistance) ?
						playerState.itemDistance[item] : maxDistance;
					ammo = (playerState.itemAmount[item] < maxAmmo) ?
						playerState.itemAmount[item] : maxAmmo;

					//relation based on amount gained and distance travelled
					heuristic += (ammo / (float)maxAmmo) * (1.0f - (distance / (float)maxDistance)) * 0.1f;
					break;
				case WP_PLASMAGUN:
					maxAmmo = 30;

					distance = (playerState.itemDistance[item] < maxDistance) ?
						playerState.itemDistance[item] : maxDistance;
					ammo = (playerState.itemAmount[item] < maxAmmo) ?
						playerState.itemAmount[item] : maxAmmo;

					//relation based on amount gained and distance travelled
					heuristic += (ammo / (float)maxAmmo) * (1.0f - (distance / (float)maxDistance)) * 0.1f;
					break;
				case WP_GRENADE_LAUNCHER:
					maxAmmo = 5;

					distance = (playerState.itemDistance[item] < maxDistance) ?
						playerState.itemDistance[item] : maxDistance;
					ammo = (playerState.itemAmount[item] < maxAmmo) ?
						playerState.itemAmount[item] : maxAmmo;

					//relation based on amount gained and distance travelled
					heuristic += (ammo / (float)maxAmmo) * (1.0f - (distance / (float)maxDistance)) * 0.f;
					break;
				case WP_ROCKET_LAUNCHER:
					maxAmmo = 5;

					distance = (playerState.itemDistance[item] < maxDistance) ?
						playerState.itemDistance[item] : maxDistance;
					ammo = (playerState.itemAmount[item] < maxAmmo) ?
						playerState.itemAmount[item] : maxAmmo;

					//relation based on amount gained and distance travelled
					heuristic += (ammo / (float)maxAmmo) * (1.0f - (distance / (float)maxDistance)) * 0.2f;
					break;
				case WP_RAILGUN:
					maxAmmo = 10;

					distance = (playerState.itemDistance[item] < maxDistance) ?
						playerState.itemDistance[item] : maxDistance;
					ammo = (playerState.itemAmount[item] < maxAmmo) ?
						playerState.itemAmount[item] : maxAmmo;

					//relation based on amount gained and distance travelled
					heuristic += (ammo / (float)maxAmmo) * (1.0f - (distance / (float)maxDistance)) * 0.2f;
					break;
			}
		}
		else if (item->GetType() == "Ammo")
		{
			eastl::shared_ptr<AmmoPickup> pAmmoPickup =
				item->GetComponent<AmmoPickup>(AmmoPickup::Name).lock();

			switch (pAmmoPickup->GetCode())
			{
				case WP_LIGHTNING:
					maxAmmo = 60;
					if (playerState.stats[STAT_WEAPONS] & (1 << pAmmoPickup->GetCode()))
					{
						distance = (playerState.itemDistance[item] < maxDistance) ?
							playerState.itemDistance[item] : maxDistance;
						ammo = (playerState.itemAmount[item] < maxAmmo) ?
							playerState.itemAmount[item] : maxAmmo;

						//relation based on amount gained and distance travelled
						heuristic += (ammo / (float)maxAmmo) * (1.0f - (distance / (float)maxDistance)) * 0.1f;
					}
					break;
				case WP_SHOTGUN:
					maxAmmo = 10;
					if (playerState.stats[STAT_WEAPONS] & (1 << pAmmoPickup->GetCode()))
					{
						distance = (playerState.itemDistance[item] < maxDistance) ?
							playerState.itemDistance[item] : maxDistance;
						ammo = (playerState.itemAmount[item] < maxAmmo) ?
							playerState.itemAmount[item] : maxAmmo;

						//relation based on amount gained and distance travelled
						heuristic += (ammo / (float)maxAmmo) * (1.0f - (distance / (float)maxDistance)) * 0.1f;
					}
					break;
				case WP_MACHINEGUN:
					maxAmmo = 50;
					if (playerState.stats[STAT_WEAPONS] & (1 << pAmmoPickup->GetCode()))
					{
						distance = (playerState.itemDistance[item] < maxDistance) ?
							playerState.itemDistance[item] : maxDistance;
						ammo = (playerState.itemAmount[item] < maxAmmo) ?
							playerState.itemAmount[item] : maxAmmo;

						//relation based on amount gained and distance travelled
						heuristic += (ammo / (float)maxAmmo) * (1.0f - (distance / (float)maxDistance)) * 0.05f;
					}
					break;
				case WP_PLASMAGUN:
					maxAmmo = 30;
					if (playerState.stats[STAT_WEAPONS] & (1 << pAmmoPickup->GetCode()))
					{
						distance = (playerState.itemDistance[item] < maxDistance) ?
							playerState.itemDistance[item] : maxDistance;
						ammo = (playerState.itemAmount[item] < maxAmmo) ?
							playerState.itemAmount[item] : maxAmmo;

						//relation based on amount gained and distance travelled
						heuristic += (ammo / (float)maxAmmo) * (1.0f - (distance / (float)maxDistance)) * 0.05f;
					}
					break;
				case WP_GRENADE_LAUNCHER:
					maxAmmo = 5;
					if (playerState.stats[STAT_WEAPONS] & (1 << pAmmoPickup->GetCode()))
					{
						distance = (playerState.itemDistance[item] < maxDistance) ?
							playerState.itemDistance[item] : maxDistance;
						ammo = (playerState.itemAmount[item] < maxAmmo) ?
							playerState.itemAmount[item] : maxAmmo;

						//relation based on amount gained and distance travelled
						heuristic += (ammo / (float)maxAmmo) * (1.0f - (distance / (float)maxDistance)) * 0.f;
					}
					break;
				case WP_ROCKET_LAUNCHER:
					maxAmmo = 5;
					if (playerState.stats[STAT_WEAPONS] & (1 << pAmmoPickup->GetCode()))
					{
						distance = (playerState.itemDistance[item] < maxDistance) ?
							playerState.itemDistance[item] : maxDistance;
						ammo = (playerState.itemAmount[item] < maxAmmo) ?
							playerState.itemAmount[item] : maxAmmo;

						//relation based on amount gained and distance travelled
						heuristic += (ammo / (float)maxAmmo) * (1.0f - (distance / (float)maxDistance)) * 0.1f;
					}
					break;
				case WP_RAILGUN:
					maxAmmo = 10;
					if (playerState.stats[STAT_WEAPONS] & (1 << pAmmoPickup->GetCode()))
					{
						distance = (playerState.itemDistance[item] < maxDistance) ?
							playerState.itemDistance[item] : maxDistance;
						ammo = (playerState.itemAmount[item] < maxAmmo) ?
							playerState.itemAmount[item] : maxAmmo;

						//relation based on amount gained and distance travelled
						heuristic += (ammo / (float)maxAmmo) * (1.0f - (distance / (float)maxDistance)) * 0.1f;
					}
					break;
			}
		}
		else if (item->GetType() == "Armor")
		{
			eastl::shared_ptr<ArmorPickup> pArmorPickup =
				item->GetComponent<ArmorPickup>(ArmorPickup::Name).lock();

			int armor = 0;
			int maxArmor = 100;

			distance = (playerState.itemDistance[item] < maxDistance) ?
				playerState.itemDistance[item] : maxDistance;
			armor = (playerState.itemAmount[item] < maxArmor) ?
				playerState.itemAmount[item] : maxArmor;

			//relation based on amount gained and distance travelled
			heuristic += (armor / (float)maxArmor) * (1.0f - (distance / (float)maxDistance)) * 0.3f;

		}
		else if (item->GetType() == "Health")
		{
			eastl::shared_ptr<HealthPickup> pHealthPickup =
				item->GetComponent<HealthPickup>(HealthPickup::Name).lock();

			int health = 0;
			int maxHealth = 100;

			distance = (playerState.itemDistance[item] < maxDistance) ?
				playerState.itemDistance[item] : maxDistance;
			health = (playerState.itemAmount[item] < maxHealth) ?
				playerState.itemAmount[item] : maxHealth;

			//relation based on amount gained and distance travelled
			heuristic += (health / (float)maxHealth) * (1.0f - (distance / (float)maxDistance)) * 0.3f;
		}
	}

	return heuristic;
}

void QuakeAIProcess::Heuristic(NodeState& playerState, NodeState& otherPlayerState)
{
	//lets give priority to damage, health, armor, weapon/ammo
	float heuristic = 0.f;

	//heuristic from picked up items
	heuristic += HeuristicPlayerItems(playerState);
	heuristic -= HeuristicPlayerItems(otherPlayerState);

	//heuristic from players status and damage dealer
	unsigned int maxHealth = 200;
	unsigned int maxArmor = 200;
	heuristic += (playerState.stats[STAT_HEALTH] / (float)maxHealth) * 0.3f;
	heuristic += (playerState.stats[STAT_ARMOR] / (float)maxArmor) * 0.3f;

	heuristic -= (otherPlayerState.stats[STAT_HEALTH] / (float)maxHealth) * 0.3f;
	heuristic -= (otherPlayerState.stats[STAT_ARMOR] / (float)maxArmor) * 0.3f;

	int ammo = 0, maxAmmo = 0;
	for (int weapon = 1; weapon < MAX_WEAPONS; weapon++)
	{
		switch (weapon)
		{
			case WP_LIGHTNING:
				maxAmmo = 200;
				if (playerState.stats[STAT_WEAPONS] & (1 << weapon))
				{
					ammo = (playerState.ammo[weapon] < maxAmmo) ? playerState.ammo[weapon] : maxAmmo;
					heuristic += (ammo / (float)maxAmmo) * 0.2f;
				}
				if (otherPlayerState.stats[STAT_WEAPONS] & (1 << weapon))
				{
					ammo = (otherPlayerState.ammo[weapon] < maxAmmo) ? otherPlayerState.ammo[weapon] : maxAmmo;
					heuristic -= (ammo / (float)maxAmmo) * 0.2f;
				}
				break;
			case WP_SHOTGUN:
				maxAmmo = 20;
				if (playerState.stats[STAT_WEAPONS] & (1 << weapon))
				{
					ammo = (playerState.ammo[weapon] < maxAmmo) ? playerState.ammo[weapon] : maxAmmo;
					heuristic += (ammo / (float)maxAmmo) * 0.2f;
				}
				if (otherPlayerState.stats[STAT_WEAPONS] & (1 << weapon))
				{
					ammo = (otherPlayerState.ammo[weapon] < maxAmmo) ? otherPlayerState.ammo[weapon] : maxAmmo;
					heuristic -= (ammo / (float)maxAmmo) * 0.2f;
				}
				break;
			case WP_MACHINEGUN:
				maxAmmo = 200;
				if (playerState.stats[STAT_WEAPONS] & (1 << weapon))
				{
					ammo = (playerState.ammo[weapon] < maxAmmo) ? playerState.ammo[weapon] : maxAmmo;
					heuristic += (ammo / (float)maxAmmo) * 0.1f;
				}
				if (otherPlayerState.stats[STAT_WEAPONS] & (1 << weapon))
				{
					ammo = (otherPlayerState.ammo[weapon] < maxAmmo) ? otherPlayerState.ammo[weapon] : maxAmmo;
					heuristic -= (ammo / (float)maxAmmo) * 0.1f;
				}
				break;
			case WP_PLASMAGUN:
				maxAmmo = 200;
				if (playerState.stats[STAT_WEAPONS] & (1 << weapon))
				{
					ammo = (playerState.ammo[weapon] < maxAmmo) ? playerState.ammo[weapon] : maxAmmo;
					heuristic += (ammo / (float)maxAmmo) * 0.1f;
				}
				if (otherPlayerState.stats[STAT_WEAPONS] & (1 << weapon))
				{
					ammo = (otherPlayerState.ammo[weapon] < maxAmmo) ? otherPlayerState.ammo[weapon] : maxAmmo;
					heuristic -= (ammo / (float)maxAmmo) * 0.1f;
				}
				break;
			case WP_GRENADE_LAUNCHER:
				maxAmmo = 20;
				if (playerState.stats[STAT_WEAPONS] & (1 << weapon))
				{
					ammo = (playerState.ammo[weapon] < maxAmmo) ? playerState.ammo[weapon] : maxAmmo;
					heuristic += (ammo / (float)maxAmmo) * 0.f;
				}
				if (otherPlayerState.stats[STAT_WEAPONS] & (1 << weapon))
				{
					ammo = (otherPlayerState.ammo[weapon] < maxAmmo) ? otherPlayerState.ammo[weapon] : maxAmmo;
					heuristic -= (ammo / (float)maxAmmo) * 0.f;
				}
				break;
			case WP_ROCKET_LAUNCHER:
				maxAmmo = 20;
				if (playerState.stats[STAT_WEAPONS] & (1 << weapon))
				{
					ammo = (playerState.ammo[weapon] < maxAmmo) ? playerState.ammo[weapon] : maxAmmo;
					heuristic += (ammo / (float)maxAmmo) * 0.2f;
				}
				if (otherPlayerState.stats[STAT_WEAPONS] & (1 << weapon))
				{
					ammo = (otherPlayerState.ammo[weapon] < maxAmmo) ? otherPlayerState.ammo[weapon] : maxAmmo;
					heuristic -= (ammo / (float)maxAmmo) * 0.2f;
				}
				break;
			case WP_RAILGUN:
				maxAmmo = 20;
				if (playerState.stats[STAT_WEAPONS] & (1 << weapon))
				{
					ammo = (playerState.ammo[weapon] < maxAmmo) ? playerState.ammo[weapon] : maxAmmo;
					heuristic += (ammo / (float)maxAmmo) * 0.2f;
				}
				if (otherPlayerState.stats[STAT_WEAPONS] & (1 << weapon))
				{
					ammo = (otherPlayerState.ammo[weapon] < maxAmmo) ? otherPlayerState.ammo[weapon] : maxAmmo;
					heuristic -= (ammo / (float)maxAmmo) * 0.2f;
				}
				break;
		}
	}

	int playerMaxDamage = 0, otherPlayerMaxDamage = 0;
	for (int weapon = 1; weapon <= MAX_WEAPONS; weapon++)
	{
		if (playerState.damage[weapon - 1] > playerMaxDamage)
		{
			playerState.target = otherPlayerState.player;
			playerState.weapon = (WeaponType)weapon;
			playerMaxDamage = playerState.damage[weapon - 1];
		}

		if (otherPlayerState.damage[weapon - 1] > otherPlayerMaxDamage)
		{
			otherPlayerState.target = playerState.player;
			otherPlayerState.weapon = (WeaponType)weapon;
			otherPlayerMaxDamage = otherPlayerState.damage[weapon - 1];
		}
	}
	
	int maxDamage = playerMaxDamage > otherPlayerMaxDamage ? playerMaxDamage : otherPlayerMaxDamage;
	if (maxDamage > 0)
	{
		if (maxDamage < 100) maxDamage = 100;

		heuristic += (playerMaxDamage / maxDamage) * 0.5f;
		heuristic -= (otherPlayerMaxDamage / maxDamage) * 0.5f;
	}
	
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
						damage = 5;
						fireTime = 0.05f;
						state.damage[weapon - 1] = 0;
						shotCount = int(visibleTime / fireTime);
						shotCount = shotCount > state.ammo[weapon] ? state.ammo[weapon] : shotCount;
						if (visibleDistance <= 600)
							state.damage[weapon - 1] = damage * shotCount;
						break;
					case WP_SHOTGUN:
						damage = 120;
						fireTime = 1.0f;
						rangeDistance = visibleDistance > 800 ? visibleDistance : 800;
						shotCount = (int)round(visibleTime / fireTime);
						shotCount = shotCount > state.ammo[weapon] ? state.ammo[weapon] : shotCount;
						state.damage[weapon - 1] = (int)round(damage *
							(1.f - (visibleDistance / rangeDistance)) * shotCount);
						break;
					case WP_MACHINEGUN:
						damage = 5;
						fireTime = 0.1f;
						rangeDistance = visibleDistance > 300 ? visibleDistance : 300;
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
						if (visibleHeight <= 30.f)
							rangeDistance = visibleDistance > 700 ? visibleDistance : 700;
						else 
							rangeDistance = visibleDistance > 1000 ? visibleDistance : 1000;
						shotCount = int(visibleTime / fireTime);
						shotCount = shotCount > state.ammo[weapon] ? state.ammo[weapon] : shotCount;
						state.damage[weapon - 1] = (int)round(damage *
							(1.f - (visibleDistance / rangeDistance)) * shotCount);
						break;
					case WP_PLASMAGUN:
						damage = 10;
						fireTime = 0.1f;
						rangeDistance = visibleDistance > 300 ? visibleDistance : 300;
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
			if (visibleDistance <= 20.f)
			{
				int damage = 50;
				float fireTime = 1.5f;
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
	if (pItemActor)
	{
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
	}
	return false;
}

void QuakeAIProcess::PickupItems(NodeState& playerState, eastl::map<ActorId, float>& actors)
{
	for (auto actor : actors)
	{
		eastl::shared_ptr<Actor> pItemActor(
			GameLogic::Get()->GetActor(actor.first).lock());
		if (pItemActor)
		{
			if (pItemActor->GetType() == "Weapon")
			{
				eastl::shared_ptr<WeaponPickup> pWeaponPickup =
					pItemActor->GetComponent<WeaponPickup>(WeaponPickup::Name).lock();
				if (mExcludeActors.find(actor.first) != mExcludeActors.end())
				{
					if (mExcludeActors[actor.first] - actor.second > 0)
						continue;
				}
				else
				{
					if (pWeaponPickup->mRespawnTime - actor.second > 0)
						continue;
				}

				// add the weapon
				playerState.stats[STAT_WEAPONS] |= (1 << pWeaponPickup->GetCode());

				// add ammo
				playerState.ammo[pWeaponPickup->GetCode()] += pWeaponPickup->GetAmmo();
				if (playerState.ammo[pWeaponPickup->GetCode()] > 200)
				{
					//add amount and distance
					playerState.items.push_back(pItemActor);
					playerState.itemDistance[pItemActor] = actor.second;
					playerState.itemAmount[pItemActor] = pWeaponPickup->GetAmmo() -
						(playerState.ammo[pWeaponPickup->GetCode()] - 200);

					playerState.ammo[pWeaponPickup->GetCode()] = 200;
				}
				else
				{
					//add amount and distance
					playerState.items.push_back(pItemActor);
					playerState.itemDistance[pItemActor] = actor.second;
					playerState.itemAmount[pItemActor] = pWeaponPickup->GetAmmo();
				}
			}
			else if (pItemActor->GetType() == "Ammo")
			{
				eastl::shared_ptr<AmmoPickup> pAmmoPickup =
					pItemActor->GetComponent<AmmoPickup>(AmmoPickup::Name).lock();
				if (mExcludeActors.find(actor.first) != mExcludeActors.end())
				{
					if (mExcludeActors[actor.first] - actor.second > 0)
						continue;
				}
				else
				{
					if (pAmmoPickup->mRespawnTime - actor.second > 0)
						continue;
				}

				playerState.ammo[pAmmoPickup->GetCode()] += pAmmoPickup->GetAmount();
				if (playerState.ammo[pAmmoPickup->GetCode()] > 200)
				{
					//add ammunt and distance
					playerState.items.push_back(pItemActor);
					playerState.itemDistance[pItemActor] = actor.second;
					playerState.itemAmount[pItemActor] = pAmmoPickup->GetAmount() -
						(playerState.ammo[pAmmoPickup->GetCode()] - 200);

					playerState.ammo[pAmmoPickup->GetCode()] = 200;
				}
				else
				{
					//add amount and distance
					playerState.items.push_back(pItemActor);
					playerState.itemDistance[pItemActor] = actor.second;
					playerState.itemAmount[pItemActor] = pAmmoPickup->GetAmount();
				}
			}
			else if (pItemActor->GetType() == "Armor")
			{
				eastl::shared_ptr<ArmorPickup> pArmorPickup =
					pItemActor->GetComponent<ArmorPickup>(ArmorPickup::Name).lock();
				if (mExcludeActors.find(actor.first) != mExcludeActors.end())
				{
					if (mExcludeActors[actor.first] - actor.second > 0)
						continue;
				}
				else
				{
					if (pArmorPickup->mRespawnTime - actor.second > 0)
						continue;
				}

				playerState.stats[STAT_ARMOR] += pArmorPickup->GetAmount();
				if (playerState.stats[STAT_ARMOR] > playerState.stats[STAT_MAX_HEALTH] * 2)
				{
					//add ammount and distance
					playerState.items.push_back(pItemActor);
					playerState.itemDistance[pItemActor] = actor.second;
					playerState.itemAmount[pItemActor] = pArmorPickup->GetAmount() -
						(playerState.stats[STAT_ARMOR] - playerState.stats[STAT_MAX_HEALTH] * 2);

					playerState.stats[STAT_ARMOR] = playerState.stats[STAT_MAX_HEALTH] * 2;
				}
				else
				{
					//add ammount and distance
					playerState.items.push_back(pItemActor);
					playerState.itemDistance[pItemActor] = actor.second;
					playerState.itemAmount[pItemActor] = pArmorPickup->GetAmount();
				}

			}
			else if (pItemActor->GetType() == "Health")
			{
				eastl::shared_ptr<HealthPickup> pHealthPickup =
					pItemActor->GetComponent<HealthPickup>(HealthPickup::Name).lock();
				if (mExcludeActors.find(actor.first) != mExcludeActors.end())
				{
					if (mExcludeActors[actor.first] - actor.second > 0)
						continue;
				}
				else
				{
					if (pHealthPickup->mRespawnTime - actor.second > 0)
						continue;
				}

				int max;
				if (pHealthPickup->GetAmount() != 5 && pHealthPickup->GetAmount() != 100)
					max = playerState.stats[STAT_MAX_HEALTH];
				else
					max = playerState.stats[STAT_MAX_HEALTH] * 2;

				playerState.stats[STAT_HEALTH] += pHealthPickup->GetAmount();
				if (playerState.stats[STAT_HEALTH] > max)
				{
					//add ammount and distance
					playerState.items.push_back(pItemActor);
					playerState.itemDistance[pItemActor] = actor.second;
					playerState.itemAmount[pItemActor] = pHealthPickup->GetAmount() -
						(playerState.stats[STAT_HEALTH] - max);

					playerState.stats[STAT_HEALTH] = max;
				}
				else
				{
					//add ammount and distance
					playerState.items.push_back(pItemActor);
					playerState.itemDistance[pItemActor] = actor.second;
					playerState.itemAmount[pItemActor] = pHealthPickup->GetAmount();
				}
			}
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

			if (mAIManager->GetPathingGraph()->IsVisibleCluster(
				currentTransition->GetNodes()[index]->GetCluster(),
				otherCurrentTransition->GetNodes()[otherIndex]->GetCluster()))
			{
				if (currentTransition->GetNodes()[index]->IsVisibleNode(
					otherCurrentTransition->GetNodes()[otherIndex]))
				{
					(*visibleDistance) += Length(
						otherCurrentTransition->GetConnections()[otherIndex] -
						currentTransition->GetConnections()[index]) * visibleWeight;
					(*visibleHeight) +=
						(currentTransition->GetConnections()[index][2] -
							otherCurrentTransition->GetConnections()[otherIndex][2]) * visibleWeight;
					(*visibleTime) += visibleWeight;

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

			if (mAIManager->GetPathingGraph()->IsVisibleCluster(
				currentTransition->GetNodes()[index]->GetCluster(), 
				otherCurrentTransition->GetNodes()[otherIndex]->GetCluster()))
			{
				if (currentTransition->GetNodes()[index]->IsVisibleNode(
					otherCurrentTransition->GetNodes()[otherIndex]))
				{
					(*visibleDistance) += Length(
						otherCurrentTransition->GetConnections()[otherIndex] -
						currentTransition->GetConnections()[index]) * visibleWeight;
					(*visibleHeight) +=
						(currentTransition->GetConnections()[index][2] -
							otherCurrentTransition->GetConnections()[otherIndex][2]) * visibleWeight;
					(*visibleTime) += visibleWeight;

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
		if (totalTime < 3.0f)
		{
			visibleWeight = 3.0f - totalTime;

			if (mAIManager->GetPathingGraph()->IsVisibleCluster(
				currentTransition->GetNodes()[index]->GetCluster(),
				otherCurrentTransition->GetNodes()[otherIndex]->GetCluster()))
			{
				if (currentTransition->GetNodes()[index]->IsVisibleNode(
					otherCurrentTransition->GetNodes()[otherIndex]))
				{
					(*visibleDistance) += Length(
						otherCurrentTransition->GetConnections()[otherIndex] -
						currentTransition->GetConnections()[index]) * visibleWeight;
					(*visibleHeight) +=
						(currentTransition->GetConnections()[index][2] -
							otherCurrentTransition->GetConnections()[otherIndex][2]) * visibleWeight;
					(*visibleTime) += visibleWeight;

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

void QuakeAIProcess::Simulation(
	NodeState& playerState, eastl::vector<PathingArcVec>& playerPathPlans,
	NodeState& otherPlayerState, eastl::vector<PathingArcVec>& otherPlayerPathPlans)
{
	PathingNode* playerNode = playerState.node;
	PathingNode* otherPlayerNode = otherPlayerState.node;
	eastl::map<PathingArcVec, eastl::map<PathingArcVec, NodeState>> playerStates, otherPlayerStates;
	for (PathingArcVec playerPathPlan : playerPathPlans)
	{
		float pathPlanWeight = 0.f;
		eastl::map<ActorId, float> actors;
		if (playerNode->GetActorId() != INVALID_ACTOR_ID)
			actors[playerNode->GetActorId()] = pathPlanWeight;

		PathingNode* currentNode = playerNode;
		for (PathingArc* playerArc : playerPathPlan)
		{
			pathPlanWeight += playerArc->GetWeight();
			if (playerArc->GetNode()->GetActorId() != INVALID_ACTOR_ID)
				if (actors.find(playerArc->GetNode()->GetActorId()) == actors.end())
					actors[playerArc->GetNode()->GetActorId()] = pathPlanWeight;

			currentNode = playerArc->GetNode();
		}

		for (PathingArcVec otherPlayerPathPlan : otherPlayerPathPlans)
		{
			float otherPathPlanWeight = 0.f;
			eastl::map<ActorId, float> otherActors;
			if (otherPlayerNode->GetActorId() != INVALID_ACTOR_ID)
				otherActors[otherPlayerNode->GetActorId()] = otherPathPlanWeight;

			currentNode = otherPlayerNode;
			for (PathingArc* otherPlayerArc : otherPlayerPathPlan)
			{
				otherPathPlanWeight += otherPlayerArc->GetWeight();
				if (otherPlayerArc->GetNode()->GetActorId() != INVALID_ACTOR_ID)
					if (otherActors.find(otherPlayerArc->GetNode()->GetActorId()) == otherActors.end())
						otherActors[otherPlayerArc->GetNode()->GetActorId()] = otherPathPlanWeight;

				currentNode = otherPlayerArc->GetNode();
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
			eastl::map<ActorId, float>::iterator itActor;
			for (itActor = actors.begin(); itActor != actors.end(); itActor++)
			{
				if (CanItemBeGrabbed((*itActor).first, (*itActor).second, playerState))
				{
					if (otherActors.find((*itActor).first) != otherActors.end())
					{
						if (!CanItemBeGrabbed((*itActor).first, (*itActor).second, otherPlayerState) ||
							otherActors[(*itActor).first] > (*itActor).second)
						{
							pathActors[(*itActor).first] = (*itActor).second;
						}
					}
					else pathActors[(*itActor).first] = (*itActor).second;
				}
			}
			PickupItems(playerNodeState, pathActors);

			NodeState otherPlayerNodeState(otherPlayerState);
			otherPlayerNodeState.path = otherPlayerPathPlan;
			otherPlayerNodeState.node = otherPlayerPathPlan.back()->GetNode();
			Damage(otherPlayerNodeState, otherVisibleTime, otherVisibleDistance, otherVisibleHeight);
			
			eastl::map<ActorId, float> otherPathActors;
			eastl::map<ActorId, float>::iterator itOtherActor;
			for (itOtherActor = otherActors.begin(); itOtherActor != otherActors.end(); itOtherActor++)
			{
				if (CanItemBeGrabbed((*itOtherActor).first, (*itOtherActor).second, otherPlayerState))
				{
					if (actors.find((*itOtherActor).first) != actors.end())
					{
						if (!CanItemBeGrabbed((*itOtherActor).first, (*itOtherActor).second, playerState) ||
							actors[(*itOtherActor).first] > (*itOtherActor).second)
						{
							otherPathActors[(*itOtherActor).first] = (*itOtherActor).second;
						}
					}
					else otherPathActors[(*itOtherActor).first] = (*itOtherActor).second;
				}
			}
			PickupItems(otherPlayerNodeState, otherPathActors);

			//we calculate the heuristic
			Heuristic(playerNodeState, otherPlayerNodeState);
			playerStates[playerPathPlan][otherPlayerPathPlan] = playerNodeState;
			otherPlayerStates[otherPlayerPathPlan][playerPathPlan] = otherPlayerNodeState;
		}
	}

	//we proceed with the minimax algorithm between players
	NodeState bestPlayerState;
	bestPlayerState.heuristic = -FLT_MAX;
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
		otherPlayerNodeState.heuristic = -FLT_MAX;
		for (auto evaluateOtherPlayerNodeState : evaluateOtherPlayerState.second)
			if (evaluateOtherPlayerNodeState.second.heuristic > otherPlayerNodeState.heuristic)
				otherPlayerNodeState = evaluateOtherPlayerNodeState.second;

		if (otherPlayerNodeState.heuristic < bestOtherPlayerState.heuristic)
			bestOtherPlayerState = otherPlayerNodeState;
	}

	playerState.Copy(bestPlayerState);
	otherPlayerState.Copy(bestOtherPlayerState);
}

void QuakeAIProcess::ConstructActorPath(NodeState& playerState, 
	PathingCluster* playerCluster, PathingNode* currentNode, float maxPathDistance, 
	eastl::vector<ActorId>& actors, float* actorHeuristic, PathingArcVec& actorPathPlan,
	eastl::map<ActorId, float>& currentActors, float currentDistance, PathingArcVec& currentPlan)
{
	if (currentDistance <= maxPathDistance)
	{
		PathingArcVec clusterPathPlan;
		eastl::vector<ActorId> clusterActors;
		for (ActorId actor : actors) clusterActors.push_back(actor);
		for (PathingArc* pathArc : currentPlan) clusterPathPlan.push_back(pathArc);

		while (currentNode != playerCluster->GetTarget())
		{
			PathingClusterVec actorClusters;
			currentNode->GetClusterActors(GAT_JUMP, actorClusters);
			for (PathingCluster* actorCluster : actorClusters)
			{
				ActorId actorId = actorCluster->GetActor();
				if (eastl::find(clusterActors.begin(), clusterActors.end(), actorId) != clusterActors.end())
				{
					if (CanItemBeGrabbed(actorId, currentDistance, playerState))
					{
						PathingArcVec pathPlan;
						for (PathingArc* pathArc : clusterPathPlan) pathPlan.push_back(pathArc);

						float distance = currentDistance;
						PathingNode* actorNode = currentNode;
						while (actorNode != actorCluster->GetTarget())
						{
							PathingCluster* currentCluster =
								actorNode->FindCluster(actorCluster->GetType(), actorCluster->GetTarget());
							PathingArc* currentArc = actorNode->FindArc(currentCluster->GetNode());
							distance += currentArc->GetWeight();
							pathPlan.push_back(currentArc);

							actorNode = currentArc->GetNode();
						}

						eastl::vector<ActorId> pathActors;
						eastl::map<ActorId, float> clusterPathActors;
						for (ActorId actor : actors) pathActors.push_back(actor);
						for (auto actor : currentActors) clusterPathActors[actor.first] = actor.second;

						clusterPathActors[actorId] = distance;
						pathActors.erase(eastl::find(pathActors.begin(), pathActors.end(), actorId));

						eastl::map<unsigned int, PathingCluster*> playerClusters;
						actorNode->GetClusters(playerCluster->GetTarget()->GetCluster(), playerClusters);
						if (playerClusters.size())
						{
							PathingCluster* playerActorCluster = (*playerClusters.begin()).second;
							if (playerClusters.find(playerCluster->GetType()) != playerClusters.end())
								playerActorCluster = playerClusters[playerCluster->GetType()];

							ConstructActorPath(playerState, playerActorCluster, actorNode, maxPathDistance, 
								pathActors, actorHeuristic, actorPathPlan, clusterPathActors, distance, pathPlan);
						}
					}
					clusterActors.erase(eastl::find(clusterActors.begin(), clusterActors.end(), actorId));
				}
			}

			PathingCluster* currentCluster =
				currentNode->FindCluster(playerCluster->GetType(), playerCluster->GetTarget());
			PathingArc* currentArc = currentNode->FindArc(currentCluster->GetNode());
			currentDistance += currentArc->GetWeight();
			clusterPathPlan.push_back(currentArc);

			currentNode = currentArc->GetNode();
		}
	}

	if (currentNode->GetCluster() == playerCluster->GetTarget()->GetCluster())
	{
		if (currentActors.size() > 0 && currentDistance <= maxPathDistance)
		{
			NodeState currentPlayerState(playerState);
			PickupItems(currentPlayerState, currentActors);
			float currentHeuristic = HeuristicPlayerItems(currentPlayerState);
			/*
			//printf("\n path plan size %u", pathPlan.size());
			char status[64];
			eastl::string playerStatus;

			sprintf(status, "player   %u heuristic %f cluster %u : actors : ", 
				currentPlayerState.player, currentHeuristic, currentNode->GetCluster());
			playerStatus = status;
			for (eastl::shared_ptr<Actor> pItemActor : currentPlayerState.items)
			{
				if (pItemActor->GetType() == "Weapon")
				{
					eastl::shared_ptr<WeaponPickup> pWeaponPickup =
						pItemActor->GetComponent<WeaponPickup>(WeaponPickup::Name).lock();
					sprintf(status, "weapon %u ", pWeaponPickup->GetCode());
					playerStatus += status;
				}
				else if (pItemActor->GetType() == "Ammo")
				{
					eastl::shared_ptr<AmmoPickup> pAmmoPickup =
						pItemActor->GetComponent<AmmoPickup>(AmmoPickup::Name).lock();
					sprintf(status, "ammo %u ", pAmmoPickup->GetCode());
					playerStatus += status;
				}
				else if (pItemActor->GetType() == "Armor")
				{
					eastl::shared_ptr<ArmorPickup> pArmorPickup =
						pItemActor->GetComponent<ArmorPickup>(ArmorPickup::Name).lock();
					sprintf(status, "armor %u ", pArmorPickup->GetCode());
					playerStatus += status;
				}
				else if (pItemActor->GetType() == "Health")
				{
					eastl::shared_ptr<HealthPickup> pHealthPickup =
						pItemActor->GetComponent<HealthPickup>(HealthPickup::Name).lock();
					sprintf(status, "health %u ", pHealthPickup->GetCode());
					playerStatus += status;
				}
			}
			printf("\n");
			printf(playerStatus.c_str());
			LogInformation(playerStatus);
			*/
			if (currentHeuristic > (*actorHeuristic))
			{
				actorPathPlan = currentPlan;
				(*actorHeuristic) = currentHeuristic;
			}
		}
	}
}

void QuakeAIProcess::ConstructPath(NodeState& playerState,
	PathingCluster* playerCluster, eastl::vector<PathingArcVec>& playerPathPlan)
{
	eastl::vector<ActorId> actors;
	PathingArcVec clusterPathPlan;
	PathingNode* currentNode = playerState.node;
	float maxPathDistance = 0.f;
	for (PathingCluster* actorCluster : currentNode->GetClusterActors())
		if (eastl::find(actors.begin(), actors.end(), actorCluster->GetActor()) == actors.end())
			actors.push_back(actorCluster->GetActor());
	while (currentNode != playerCluster->GetTarget())
	{
		PathingCluster* currentCluster = 
			currentNode->FindCluster(playerCluster->GetType(), playerCluster->GetTarget());
		PathingArc* currentArc = currentNode->FindArc(currentCluster->GetNode());
		clusterPathPlan.push_back(currentArc);

		currentNode = currentArc->GetNode();
		maxPathDistance += currentArc->GetWeight();
		for (PathingCluster* actorCluster : currentNode->GetClusterActors())
			if (eastl::find(actors.begin(), actors.end(), actorCluster->GetActor()) == actors.end())
				actors.push_back(actorCluster->GetActor());
	}
	if (clusterPathPlan.size()) playerPathPlan.push_back(clusterPathPlan);

	if (actors.size())
	{
		PathingArcVec currentPathPlan, actorPathPlan;
		float currentDistance = 0.f, actorHeuristic = 0.f;
		eastl::map<ActorId, float> currentActors;

		maxPathDistance += maxPathDistance * 0.5f;
		ConstructActorPath(playerState, playerCluster, playerState.node, maxPathDistance, actors,
			&actorHeuristic, actorPathPlan, currentActors, currentDistance, currentPathPlan);

		if (actorPathPlan.size())playerPathPlan.push_back(actorPathPlan);
	}
}

void QuakeAIProcess::EvaluatePlayers(NodeState& playerState, NodeState& otherPlayerState)
{
	eastl::map<PathingCluster*, eastl::vector<PathingArcVec>> playerPathPlans, otherPlayerPathPlans;

	PathingClusterVec pathingClusters[2];
	playerState.node->GetClusters(GAT_MOVE, pathingClusters[0], 20);
	playerState.node->GetClusters(GAT_JUMP, pathingClusters[1], 60);

	PathingClusterVec playerClusters;
	for (unsigned int clusterType = 0; clusterType < 2; clusterType++)
		for (PathingCluster* pathingCluster : pathingClusters[clusterType])
			playerClusters.push_back(pathingCluster);

	//fprintf(mFile, "\n playerCluster - otherPlayerNode \n");
	unsigned int clusterSize = playerClusters.size();
	unsigned int count = 0;
	for (unsigned int playerClusterIdx = 0; playerClusterIdx < clusterSize; playerClusterIdx++)
	//parallel_for(size_t(0), clusterSize, [&](size_t playerClusterIdx)
	{
		PathingCluster* playerCluster = playerClusters[playerClusterIdx];

		//construct path
		playerPathPlans[playerCluster] = eastl::vector<PathingArcVec>();
		ConstructPath(playerState, playerCluster, playerPathPlans[playerCluster]);
		count += playerPathPlans[playerCluster].size();
	}

	PathingClusterVec otherPathingClusters[2];
	otherPlayerState.node->GetClusters(GAT_MOVE, otherPathingClusters[0], 20);
	otherPlayerState.node->GetClusters(GAT_JUMP, otherPathingClusters[1], 60);

	PathingClusterVec otherPlayerClusters;
	for (unsigned int clusterType = 0; clusterType < 2; clusterType++)
		for (PathingCluster* otherPathingCluster : otherPathingClusters[clusterType])
			otherPlayerClusters.push_back(otherPathingCluster);

	//fprintf(mFile, "\n playerNode - otherPlayerCluster \n");
	unsigned int otherClusterSize = otherPlayerClusters.size();
	for (unsigned int otherPlayerClusterIdx = 0; otherPlayerClusterIdx < otherClusterSize; otherPlayerClusterIdx++)
	//parallel_for(size_t(0), otherClusterSize, [&](size_t otherPlayerClusterIdx)
	{
		PathingCluster* otherPlayerCluster = otherPlayerClusters[otherPlayerClusterIdx];

		//construct path
		otherPlayerPathPlans[otherPlayerCluster] = eastl::vector<PathingArcVec>();
		ConstructPath(otherPlayerState, otherPlayerCluster, otherPlayerPathPlans[otherPlayerCluster]);
		count += otherPlayerPathPlans[otherPlayerCluster].size();
	}
	printf("\n count  %u ", count);

	//fprintf(mFile, "\n playerCluster - otherPlayerCluster \n");
	eastl::map<PathingCluster*, eastl::map<PathingCluster*, NodeState>> playerClustersStates, otherPlayerClustersStates;
	for (unsigned int playerClusterIdx = 0; playerClusterIdx < clusterSize; playerClusterIdx++)
	//parallel_for(size_t(0), clusterSize, [&](size_t playerClusterIdx)
	{
		PathingCluster* playerCluster = playerClusters[playerClusterIdx];

		for (unsigned int otherPlayerClusterIdx = 0; otherPlayerClusterIdx < otherClusterSize; otherPlayerClusterIdx++)
		//parallel_for(size_t(0), otherClusterSize, [&](size_t otherPlayerClusterIdx)
		{
			PathingCluster* otherPlayerCluster = otherPlayerClusters[otherPlayerClusterIdx];

			NodeState state(playerState);
			NodeState otherState(otherPlayerState);
			Simulation(state, playerPathPlans[playerCluster], otherState, otherPlayerPathPlans[otherPlayerCluster]);

			if (state.valid && otherState.valid)
			{
				playerClustersStates[playerCluster][otherPlayerCluster] = state;
				otherPlayerClustersStates[otherPlayerCluster][playerCluster] = otherState;
			}
		}
	}

	//minimax
	mPlayerState.Copy(playerState);
	mPlayerState.heuristic = -FLT_MAX;
	fprintf(mFile, "\n minimax player \n");
	for (auto playerClustersState : playerClustersStates)
	{
		fprintf(mFile, "\n player cluster : %u ",
			playerClustersState.first->GetTarget()->GetCluster());

		NodeState playerNodeState(playerState);
		playerNodeState.heuristic = FLT_MAX;
		for (auto playerClusterState : playerClustersState.second)
		{
			if (playerClusterState.second.weapon != WP_NONE)
			{
				fprintf(mFile, " weapon : %u ", playerClusterState.second.weapon);
				fprintf(mFile, " damage : %i ", 
					playerClusterState.second.damage[playerClusterState.second.weapon - 1]);
			}

			PathingArcVec::iterator itArc = playerClusterState.second.path.begin();
			for (; itArc != playerClusterState.second.path.end(); itArc++)
			{
				//printf("%u ", (*itArc)->GetNode()->GetId());
			}

			if (!playerClusterState.second.items.empty())
			{
				fprintf(mFile, "other player cluster : %u ",
					playerClusterState.first->GetTarget()->GetCluster());
				fprintf(mFile, "heuristic : %f ", playerClusterState.second.heuristic);

				fprintf(mFile, " actors : ");
			}
			for (eastl::shared_ptr<Actor> pItemActor : playerClusterState.second.items)
			{
				if (pItemActor->GetType() == "Weapon")
				{
					eastl::shared_ptr<WeaponPickup> pWeaponPickup =
						pItemActor->GetComponent<WeaponPickup>(WeaponPickup::Name).lock();
					fprintf(mFile, "weapon %u ", pWeaponPickup->GetCode());
				}
				else if (pItemActor->GetType() == "Ammo")
				{
					eastl::shared_ptr<AmmoPickup> pAmmoPickup =
						pItemActor->GetComponent<AmmoPickup>(AmmoPickup::Name).lock();
					fprintf(mFile, "ammo %u ", pAmmoPickup->GetCode());
				}
				else if (pItemActor->GetType() == "Armor")
				{
					eastl::shared_ptr<ArmorPickup> pArmorPickup =
						pItemActor->GetComponent<ArmorPickup>(ArmorPickup::Name).lock();
					fprintf(mFile, "armor %u ", pArmorPickup->GetCode());
				}
				else if (pItemActor->GetType() == "Health")
				{
					eastl::shared_ptr<HealthPickup> pHealthPickup =
						pItemActor->GetComponent<HealthPickup>(HealthPickup::Name).lock();
					fprintf(mFile, "health %u ", pHealthPickup->GetCode());
				}
			}

			if (abs(playerClusterState.second.heuristic - playerNodeState.heuristic) <= GE_ROUNDING_ERROR)
			{
				if (playerNodeState.weapon != WP_NONE && playerClusterState.second.weapon != WP_NONE)
				{
					if (playerClusterState.second.damage[playerClusterState.second.weapon - 1] >
						playerNodeState.damage[playerNodeState.weapon - 1])
					{
						playerNodeState = playerClusterState.second;
					}
				}
				else if (playerClusterState.second.weapon != WP_NONE)
				{
					playerNodeState = playerClusterState.second;
				}
			}
			else if (playerClusterState.second.heuristic < playerNodeState.heuristic)
			{
				playerNodeState = playerClusterState.second;
			}
		}
		if (playerNodeState.valid)
		{
			fprintf(mFile, "\n min heuristic : %f ", playerNodeState.heuristic);
			if (playerNodeState.weapon != WP_NONE)
			{
				fprintf(mFile, " weapon : %u ", playerNodeState.weapon);
				fprintf(mFile, " damage : %i ", playerNodeState.damage[playerNodeState.weapon - 1]);
			}
			else
			{
				fprintf(mFile, " weapon : 0 ");
				fprintf(mFile, " damage : 0 ");
			}

			PathingArcVec::iterator itArc = playerNodeState.path.begin();
			for (; itArc != playerNodeState.path.end(); itArc++)
			{
				//printf("%u ", (*itArc)->GetNode()->GetId());
			}

			if (!playerNodeState.items.empty()) fprintf(mFile, " actors : ");
			for (eastl::shared_ptr<Actor> pItemActor : playerNodeState.items)
			{
				if (pItemActor->GetType() == "Weapon")
				{
					eastl::shared_ptr<WeaponPickup> pWeaponPickup =
						pItemActor->GetComponent<WeaponPickup>(WeaponPickup::Name).lock();
					fprintf(mFile, "weapon %u ", pWeaponPickup->GetCode());
				}
				else if (pItemActor->GetType() == "Ammo")
				{
					eastl::shared_ptr<AmmoPickup> pAmmoPickup =
						pItemActor->GetComponent<AmmoPickup>(AmmoPickup::Name).lock();
					fprintf(mFile, "ammo %u ", pAmmoPickup->GetCode());
				}
				else if (pItemActor->GetType() == "Armor")
				{
					eastl::shared_ptr<ArmorPickup> pArmorPickup =
						pItemActor->GetComponent<ArmorPickup>(ArmorPickup::Name).lock();
					fprintf(mFile, "armor %u ", pArmorPickup->GetCode());
				}
				else if (pItemActor->GetType() == "Health")
				{
					eastl::shared_ptr<HealthPickup> pHealthPickup =
						pItemActor->GetComponent<HealthPickup>(HealthPickup::Name).lock();
					fprintf(mFile, "health %u ", pHealthPickup->GetCode());
				}
			}
		}

		if (abs(playerNodeState.heuristic - mPlayerState.heuristic) <= GE_ROUNDING_ERROR)
		{
			if (mPlayerState.weapon != WP_NONE && playerNodeState.weapon != WP_NONE)
			{
				if (playerNodeState.damage[playerNodeState.weapon - 1] >
					mPlayerState.damage[mPlayerState.weapon - 1])
				{
					mPlayerState.Copy(playerNodeState);
				}
			}
			else if (playerNodeState.weapon != WP_NONE)
			{
				mPlayerState.Copy(playerNodeState);
			}
		}
		else if (playerNodeState.heuristic > mPlayerState.heuristic)
		{
			mPlayerState.Copy(playerNodeState);
		}
	}
	if (mPlayerState.valid)
	{
		fprintf(mFile, "\n max heuristic : %f ", mPlayerState.heuristic);
		if (mPlayerState.weapon != WP_NONE)
		{
			fprintf(mFile, " weapon : %u ", mPlayerState.weapon);
			fprintf(mFile, " damage : %i ", mPlayerState.damage[mPlayerState.weapon - 1]);
		}
		else
		{
			fprintf(mFile, " weapon : 0 ");
			fprintf(mFile, " damage : 0 ");
		}

		PathingArcVec::iterator itArc = mPlayerState.path.begin();
		for (; itArc != mPlayerState.path.end(); itArc++)
		{
			//printf("%u ", (*itArc)->GetNode()->GetId());
		}

		if (!mPlayerState.items.empty()) fprintf(mFile, " actors : ");
		for (eastl::shared_ptr<Actor> pItemActor : mPlayerState.items)
		{
			if (pItemActor->GetType() == "Weapon")
			{
				eastl::shared_ptr<WeaponPickup> pWeaponPickup =
					pItemActor->GetComponent<WeaponPickup>(WeaponPickup::Name).lock();
				fprintf(mFile, "weapon %u ", pWeaponPickup->GetCode());
			}
			else if (pItemActor->GetType() == "Ammo")
			{
				eastl::shared_ptr<AmmoPickup> pAmmoPickup =
					pItemActor->GetComponent<AmmoPickup>(AmmoPickup::Name).lock();
				fprintf(mFile, "ammo %u ", pAmmoPickup->GetCode());
			}
			else if (pItemActor->GetType() == "Armor")
			{
				eastl::shared_ptr<ArmorPickup> pArmorPickup =
					pItemActor->GetComponent<ArmorPickup>(ArmorPickup::Name).lock();
				fprintf(mFile, "armor %u ", pArmorPickup->GetCode());
			}
			else if (pItemActor->GetType() == "Health")
			{
				eastl::shared_ptr<HealthPickup> pHealthPickup =
					pItemActor->GetComponent<HealthPickup>(HealthPickup::Name).lock();
				fprintf(mFile, "health %u ", pHealthPickup->GetCode());
			}
		}
	}

	mOtherPlayerState.Copy(otherPlayerState);
	mOtherPlayerState.heuristic = FLT_MAX;
	fprintf(mFile, "\n minimax otherPlayer \n");
	for (auto otherPlayerClustersState : otherPlayerClustersStates)
	{
		fprintf(mFile, "\n other player cluster : %u ",
			otherPlayerClustersState.first->GetTarget()->GetCluster());

		NodeState otherPlayerNodeState(otherPlayerState);
		otherPlayerNodeState.heuristic = -FLT_MAX;
		for (auto otherPlayerClusterState : otherPlayerClustersState.second)
		{
			if (otherPlayerClusterState.second.weapon != WP_NONE)
			{
				fprintf(mFile, " weapon : %u ", otherPlayerClusterState.second.weapon);
				fprintf(mFile, " damage : %i ",
					otherPlayerClusterState.second.damage[
					otherPlayerClusterState.second.weapon - 1]);
			}

			//printf("nodes : ");
			PathingArcVec::iterator itArc = otherPlayerClusterState.second.path.begin();
			for (; itArc != otherPlayerClusterState.second.path.end(); itArc++)
			{
				//printf("%u ", (*itArc)->GetNode()->GetId());
			}

			if (!otherPlayerClusterState.second.items.empty())
			{
				fprintf(mFile, "player cluster : %u ",
					otherPlayerClusterState.first->GetTarget()->GetCluster());
				fprintf(mFile, "heuristic : %f ", otherPlayerClusterState.second.heuristic);


				fprintf(mFile, " actors : ");
			}
			for (eastl::shared_ptr<Actor> pItemActor : otherPlayerClusterState.second.items)
			{
				if (pItemActor->GetType() == "Weapon")
				{
					eastl::shared_ptr<WeaponPickup> pWeaponPickup =
						pItemActor->GetComponent<WeaponPickup>(WeaponPickup::Name).lock();
					fprintf(mFile, "weapon %u ", pWeaponPickup->GetCode());
				}
				else if (pItemActor->GetType() == "Ammo")
				{
					eastl::shared_ptr<AmmoPickup> pAmmoPickup =
						pItemActor->GetComponent<AmmoPickup>(AmmoPickup::Name).lock();
					fprintf(mFile, "ammo %u ", pAmmoPickup->GetCode());
				}
				else if (pItemActor->GetType() == "Armor")
				{
					eastl::shared_ptr<ArmorPickup> pArmorPickup =
						pItemActor->GetComponent<ArmorPickup>(ArmorPickup::Name).lock();
					fprintf(mFile, "armor %u ", pArmorPickup->GetCode());
				}
				else if (pItemActor->GetType() == "Health")
				{
					eastl::shared_ptr<HealthPickup> pHealthPickup =
						pItemActor->GetComponent<HealthPickup>(HealthPickup::Name).lock();
					fprintf(mFile, "health %u ", pHealthPickup->GetCode());
				}
			}

			if (abs(otherPlayerClusterState.second.heuristic - otherPlayerNodeState.heuristic) <= GE_ROUNDING_ERROR)
			{
				if (otherPlayerNodeState.weapon != WP_NONE && otherPlayerClusterState.second.weapon != WP_NONE)
				{
					if (otherPlayerClusterState.second.damage[otherPlayerClusterState.second.weapon - 1] >
						otherPlayerNodeState.damage[otherPlayerNodeState.weapon - 1])
					{
						otherPlayerNodeState = otherPlayerClusterState.second;
					}
				}
				else if (otherPlayerClusterState.second.weapon != WP_NONE)
				{
					otherPlayerNodeState = otherPlayerClusterState.second;
				}
			}
			else if (otherPlayerClusterState.second.heuristic > otherPlayerNodeState.heuristic)
			{
				otherPlayerNodeState = otherPlayerClusterState.second;
			}
		}

		if (otherPlayerNodeState.valid)
		{
			fprintf(mFile, "\n max heuristic : %f ", otherPlayerNodeState.heuristic);
			if (otherPlayerNodeState.weapon != WP_NONE)
			{
				fprintf(mFile, " weapon : %u ", otherPlayerNodeState.weapon);
				fprintf(mFile, " damage : %i ",
					otherPlayerNodeState.damage[otherPlayerNodeState.weapon - 1]);
			}
			else
			{
				fprintf(mFile, " weapon : 0 ");
				fprintf(mFile, " damage : 0 ");
			}

			//printf("nodes : ");
			PathingArcVec::iterator itArc = otherPlayerNodeState.path.begin();
			for (; itArc != otherPlayerNodeState.path.end(); itArc++)
			{
				//printf("%u ", (*itArc)->GetNode()->GetId());
			}

			if (!otherPlayerNodeState.items.empty()) fprintf(mFile, " actors : ");
			for (eastl::shared_ptr<Actor> pItemActor : otherPlayerNodeState.items)
			{
				if (pItemActor->GetType() == "Weapon")
				{
					eastl::shared_ptr<WeaponPickup> pWeaponPickup =
						pItemActor->GetComponent<WeaponPickup>(WeaponPickup::Name).lock();
					fprintf(mFile, "weapon %u ", pWeaponPickup->GetCode());
				}
				else if (pItemActor->GetType() == "Ammo")
				{
					eastl::shared_ptr<AmmoPickup> pAmmoPickup =
						pItemActor->GetComponent<AmmoPickup>(AmmoPickup::Name).lock();
					fprintf(mFile, "ammo %u ", pAmmoPickup->GetCode());
				}
				else if (pItemActor->GetType() == "Armor")
				{
					eastl::shared_ptr<ArmorPickup> pArmorPickup =
						pItemActor->GetComponent<ArmorPickup>(ArmorPickup::Name).lock();
					fprintf(mFile, "armor %u ", pArmorPickup->GetCode());
				}
				else if (pItemActor->GetType() == "Health")
				{
					eastl::shared_ptr<HealthPickup> pHealthPickup =
						pItemActor->GetComponent<HealthPickup>(HealthPickup::Name).lock();
					fprintf(mFile, "health %u ", pHealthPickup->GetCode());
				}
			}
		}

		if (abs(otherPlayerNodeState.heuristic - mOtherPlayerState.heuristic) <= GE_ROUNDING_ERROR)
		{
			if (mOtherPlayerState.weapon != WP_NONE && otherPlayerNodeState.weapon != WP_NONE)
			{
				if (otherPlayerNodeState.damage[otherPlayerNodeState.weapon - 1] >
					mOtherPlayerState.damage[mOtherPlayerState.weapon - 1])
				{
					mOtherPlayerState.Copy(otherPlayerNodeState);
				}
			}
			else if (otherPlayerNodeState.weapon != WP_NONE)
			{
				mOtherPlayerState.Copy(otherPlayerNodeState);
			}
		}
		else if (otherPlayerNodeState.heuristic < mOtherPlayerState.heuristic)
		{
			mOtherPlayerState.Copy(otherPlayerNodeState);
		}
	}
	if (mOtherPlayerState.valid)
	{
		fprintf(mFile, "\n min heuristic : %f ", mOtherPlayerState.heuristic);
		if (mOtherPlayerState.weapon != WP_NONE)
		{
			fprintf(mFile, " weapon : %u ", mOtherPlayerState.weapon);
			fprintf(mFile, " damage : %i ",
				mOtherPlayerState.damage[mOtherPlayerState.weapon - 1]);
		}
		else
		{
			fprintf(mFile, " weapon : 0 ");
			fprintf(mFile, " damage : 0 ");
		}

		//printf("nodes : ");
		PathingArcVec::iterator itArc = mOtherPlayerState.path.begin();
		for (; itArc != mOtherPlayerState.path.end(); itArc++)
		{
			//printf("%u ", (*itArc)->GetNode()->GetId());
		}

		if (!mOtherPlayerState.items.empty()) fprintf(mFile, " actors : ");
		for (eastl::shared_ptr<Actor> pItemActor : mOtherPlayerState.items)
		{
			if (pItemActor->GetType() == "Weapon")
			{
				eastl::shared_ptr<WeaponPickup> pWeaponPickup =
					pItemActor->GetComponent<WeaponPickup>(WeaponPickup::Name).lock();
				fprintf(mFile, "weapon %u ", pWeaponPickup->GetCode());
			}
			else if (pItemActor->GetType() == "Ammo")
			{
				eastl::shared_ptr<AmmoPickup> pAmmoPickup =
					pItemActor->GetComponent<AmmoPickup>(AmmoPickup::Name).lock();
				fprintf(mFile, "ammo %u ", pAmmoPickup->GetCode());
			}
			else if (pItemActor->GetType() == "Armor")
			{
				eastl::shared_ptr<ArmorPickup> pArmorPickup =
					pItemActor->GetComponent<ArmorPickup>(ArmorPickup::Name).lock();
				fprintf(mFile, "armor %u ", pArmorPickup->GetCode());
			}
			else if (pItemActor->GetType() == "Health")
			{
				eastl::shared_ptr<HealthPickup> pHealthPickup =
					pItemActor->GetComponent<HealthPickup>(HealthPickup::Name).lock();
				fprintf(mFile, "health %u ", pHealthPickup->GetCode());
			}
		}
	}
}

void QuakeAIProcess::ThreadProc( )
{
	unsigned int iteration = 0;

	while (true)
	{
		if (GameLogic::Get()->GetState() == BGS_RUNNING)
		{
			eastl::map<GameViewType, eastl::vector<ActorId>> players;

			GameApplication* gameApp = (GameApplication*)Application::App;
			const GameViewList& gameViews = gameApp->GetGameViews();
			for (auto it = gameViews.begin(); it != gameViews.end(); ++it)
			{
				eastl::shared_ptr<BaseGameView> pView = *it;
				if (pView->GetActorId() != INVALID_ACTOR_ID)
					players[pView->GetType()].push_back(pView->GetActorId());
			}

			if (players.find(GV_HUMAN) != players.end() && players.find(GV_AI) != players.end())
			{
				eastl::vector<ActorId>::iterator itAIPlayer;
				for (itAIPlayer = players[GV_AI].begin(); itAIPlayer != players[GV_AI].end(); )
				{
					bool removeAIPlayer = false;
					for (ActorId player : players[GV_HUMAN])
						if (player == (*itAIPlayer))
							removeAIPlayer = true;

					if (removeAIPlayer)
						itAIPlayer = players[GV_AI].erase(itAIPlayer);
					else
						itAIPlayer++;
				}

				iteration++;
				printf("\n ITERATION %u \n", iteration);
				fprintf(mFile, "\n\n ITERATION %u \n\n", iteration);

				for (ActorId player : players[GV_HUMAN])
				{
					eastl::shared_ptr<PlayerActor> pHumanPlayer =
						eastl::dynamic_shared_pointer_cast<PlayerActor>(
						GameLogic::Get()->GetActor(player).lock());
					eastl::shared_ptr<TransformComponent> pTransformComponent(
						pHumanPlayer->GetComponent<TransformComponent>(TransformComponent::Name).lock());

					NodeState playerState(pHumanPlayer);
					playerState.node =
						mAIManager->GetPathingGraph()->FindClosestNode(pTransformComponent->GetPosition());
					for (ActorId aiPlayer : players[GV_AI])
					{
						NodeState aiPlayerState;
						mAIManager->GetPlayerGuessState(aiPlayer, aiPlayerState);
						aiPlayerState.node = mAIManager->GetPlayerGuessNode(aiPlayer);
						aiPlayerState.ResetItems();

						mExcludeActors.clear();
						mAIManager->GetPlayerGuessItems(aiPlayerState.player, mExcludeActors);
						EvaluatePlayers(playerState, aiPlayerState);
					}
				}

				/*
				printf("\n blue player pos %f %f %f, id %u, heuristic %f, target %u, weapon %u, damage %u, paths %u \n",
				mPlayerState.node->GetPos()[0], mPlayerState.node->GetPos()[1], mPlayerState.node->GetPos()[2],
				mPlayerState.node->GetId(), mPlayerState.heuristic, mPlayerState.target, mPlayerState.weapon,
				mPlayerState.weapon > 0 ? mPlayerState.damage[mPlayerState.weapon - 1] : 0, mPlayerState.path.size());
				*/
				//printf("\n blue player nodes %u : ", mPlayerState.player);
				PathingArcVec::iterator itArc = mPlayerState.path.begin();
				for (; itArc != mPlayerState.path.end(); itArc++)
				{
					//printf("%u ", (*itArc)->GetNode()->GetId());
				}

				fprintf(mFile, "\n blue player   %f", mPlayerState.heuristic);
				//printf("\n blue player actors  %u : ", mPlayerState.player);
				if (!mPlayerState.items.empty()) fprintf(mFile, " actors : ");
				for (eastl::shared_ptr<Actor> pItemActor : mPlayerState.items)
				{
					if (pItemActor->GetType() == "Weapon")
					{
						eastl::shared_ptr<WeaponPickup> pWeaponPickup =
							pItemActor->GetComponent<WeaponPickup>(WeaponPickup::Name).lock();
						fprintf(mFile, "weapon %u ", pWeaponPickup->GetCode());
						//printf("weapon %u ", pWeaponPickup->GetCode());
					}
					else if (pItemActor->GetType() == "Ammo")
					{
						eastl::shared_ptr<AmmoPickup> pAmmoPickup =
							pItemActor->GetComponent<AmmoPickup>(AmmoPickup::Name).lock();
						fprintf(mFile, "ammo %u ", pAmmoPickup->GetCode());
						//printf("ammo %u ", pAmmoPickup->GetCode());
					}
					else if (pItemActor->GetType() == "Armor")
					{
						eastl::shared_ptr<ArmorPickup> pArmorPickup =
							pItemActor->GetComponent<ArmorPickup>(ArmorPickup::Name).lock();
						fprintf(mFile, "armor %u ", pArmorPickup->GetCode());
						//printf("armor %u ", pArmorPickup->GetCode());
					}
					else if (pItemActor->GetType() == "Health")
					{
						eastl::shared_ptr<HealthPickup> pHealthPickup =
							pItemActor->GetComponent<HealthPickup>(HealthPickup::Name).lock();
						fprintf(mFile, "health %u ", pHealthPickup->GetCode());
						//printf("health %u ", pHealthPickup->GetCode());
					}
				}
				if (mPlayerState.weapon != WP_NONE)
				{
					fprintf(mFile, " weapon : %u ", mPlayerState.weapon);
					fprintf(mFile, " damage : %i ", mPlayerState.damage[mPlayerState.weapon - 1]);
				}
				else
				{
					fprintf(mFile, " weapon : 0 ");
					fprintf(mFile, " damage : 0 ");
				}

				NodeState otherPlayerGuessState; 
				mAIManager->GetPlayerGuessState(mOtherPlayerState.player, otherPlayerGuessState);
				otherPlayerGuessState.CopyItems(mOtherPlayerState);
				mAIManager->SetPlayerGuessState(mOtherPlayerState.player, otherPlayerGuessState);
				mAIManager->SetPlayerGuessPath(mOtherPlayerState.player, mOtherPlayerState.path);
				mAIManager->SetPlayerGuessUpdated(mOtherPlayerState.player, true);

				mAIManager->SetPlayerTarget(mPlayerState.player, mPlayerState.target);
				mAIManager->SetPlayerWeapon(mPlayerState.player, mPlayerState.weapon);
				mAIManager->SetPlayerState(mPlayerState.player, mPlayerState);
				mAIManager->SetPlayerUpdated(mPlayerState.player, true);

				for (ActorId aiPlayer : players[GV_AI])
				{
					eastl::shared_ptr<PlayerActor> pAIPlayer =
						eastl::dynamic_shared_pointer_cast<PlayerActor>(
						GameLogic::Get()->GetActor(aiPlayer).lock());
					eastl::shared_ptr<TransformComponent> pTransformComponent(
						pAIPlayer->GetComponent<TransformComponent>(TransformComponent::Name).lock());

					NodeState aiPlayerState(pAIPlayer);
					aiPlayerState.node =
						mAIManager->GetPathingGraph()->FindClosestNode(pTransformComponent->GetPosition());
					for (ActorId player : players[GV_HUMAN])
					{
						NodeState playerState;
						mAIManager->GetPlayerGuessState(player, playerState);
						playerState.node = mAIManager->GetPlayerGuessNode(player);
						playerState.ResetItems();

						mExcludeActors.clear();
						mAIManager->GetPlayerGuessItems(playerState.player, mExcludeActors);
						EvaluatePlayers(playerState, aiPlayerState);
					}
				}

				/*
				printf("\n red player pos %f %f %f, id %u, heuristic %f, target %u, weapon %u, damage %u, paths %u \n",
				mOtherPlayerState.node->GetPos()[0], mOtherPlayerState.node->GetPos()[1], mOtherPlayerState.node->GetPos()[2],
				mOtherPlayerState.node->GetId(), mOtherPlayerState.heuristic, mOtherPlayerState.target, mOtherPlayerState.weapon,
				mOtherPlayerState.weapon > 0 ? mOtherPlayerState.damage[mOtherPlayerState.weapon - 1] : 0, mOtherPlayerState.path.size());
				*/
				//printf("\n red player nodes %u : ", mOtherPlayerState.player);
				itArc = mOtherPlayerState.path.begin();
				for (; itArc != mOtherPlayerState.path.end(); itArc++)
				{
					//printf("%u ", (*itArc)->GetNode()->GetId());
				}

				fprintf(mFile, "\n red player   %f", mOtherPlayerState.heuristic);
				//printf("\n red player actors %u : ", mOtherPlayerState.player);
				if (!mOtherPlayerState.items.empty()) fprintf(mFile, " actors : ");
				for (eastl::shared_ptr<Actor> pItemActor : mOtherPlayerState.items)
				{
					if (pItemActor->GetType() == "Weapon")
					{
						eastl::shared_ptr<WeaponPickup> pWeaponPickup =
							pItemActor->GetComponent<WeaponPickup>(WeaponPickup::Name).lock();
						fprintf(mFile, "weapon %u ", pWeaponPickup->GetCode());
						//printf("weapon %u ", pWeaponPickup->GetCode());
					}
					else if (pItemActor->GetType() == "Ammo")
					{
						eastl::shared_ptr<AmmoPickup> pAmmoPickup =
							pItemActor->GetComponent<AmmoPickup>(AmmoPickup::Name).lock();
						fprintf(mFile, "ammo %u ", pAmmoPickup->GetCode());
						//printf("ammo %u ", pAmmoPickup->GetCode());
					}
					else if (pItemActor->GetType() == "Armor")
					{
						eastl::shared_ptr<ArmorPickup> pArmorPickup =
							pItemActor->GetComponent<ArmorPickup>(ArmorPickup::Name).lock();
						fprintf(mFile, "armor %u ", pArmorPickup->GetCode());
						//printf("armor %u ", pArmorPickup->GetCode());
					}
					else if (pItemActor->GetType() == "Health")
					{
						eastl::shared_ptr<HealthPickup> pHealthPickup =
							pItemActor->GetComponent<HealthPickup>(HealthPickup::Name).lock();
						fprintf(mFile, "health %u ", pHealthPickup->GetCode());
						//printf("health %u ", pHealthPickup->GetCode());
					}
				}
				if (mOtherPlayerState.weapon != WP_NONE)
				{
					fprintf(mFile, " weapon : %u ", mOtherPlayerState.weapon);
					fprintf(mFile, " damage : %i ", mOtherPlayerState.damage[mOtherPlayerState.weapon - 1]);
				}
				else
				{
					fprintf(mFile, " weapon : 0 ");
					fprintf(mFile, " damage : 0 ");
				}

				NodeState playerGuessState;
				mAIManager->GetPlayerGuessState(mPlayerState.player, playerGuessState);
				playerGuessState.CopyItems(mPlayerState);
				mAIManager->SetPlayerGuessState(mPlayerState.player, playerGuessState);
				mAIManager->SetPlayerGuessPath(mPlayerState.player, mPlayerState.path);
				mAIManager->SetPlayerGuessUpdated(mPlayerState.player, true);

				mAIManager->SetPlayerTarget(mOtherPlayerState.player, mOtherPlayerState.target);
				mAIManager->SetPlayerWeapon(mOtherPlayerState.player, mOtherPlayerState.weapon);
				mAIManager->SetPlayerState(mOtherPlayerState.player, mOtherPlayerState);
				mAIManager->SetPlayerUpdated(mOtherPlayerState.player, true);
			}

			mAIManager->SetEnable(true);
		}
	}

	Succeed();
}