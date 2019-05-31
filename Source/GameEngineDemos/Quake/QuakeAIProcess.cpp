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
	mFile = fopen("test.txt", "w");
}

QuakeAIProcess::~QuakeAIProcess(void)
{
	fclose(mFile);
}

void QuakeAIProcess::Heuristic(NodeState& playerState, NodeState& otherPlayerState)
{
	//lets give priority to damage, health, armor, weapon/ammo
	float heuristic = 0.f;

	float maxDistance = 4.0f;
	float distance = 0.f;
	int maxAmmo = 0;
	int ammo = 0;

	//picked up items
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
					if (playerState.stats[STAT_WEAPONS] & (1 << pWeaponPickup->GetCode()))
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
					if (playerState.stats[STAT_WEAPONS] & (1 << pWeaponPickup->GetCode()))
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
					if (playerState.stats[STAT_WEAPONS] & (1 << pWeaponPickup->GetCode()))
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
					if (playerState.stats[STAT_WEAPONS] & (1 << pWeaponPickup->GetCode()))
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
					if (playerState.stats[STAT_WEAPONS] & (1 << pWeaponPickup->GetCode()))
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
					if (playerState.stats[STAT_WEAPONS] & (1 << pWeaponPickup->GetCode()))
					{
						distance = (playerState.itemDistance[item] < maxDistance) ?
							playerState.itemDistance[item] : maxDistance;
						ammo = (playerState.itemAmount[item] < maxAmmo) ?
							playerState.itemAmount[item] : maxAmmo;

						//relation based on amount gained and distance travelled
						heuristic += (ammo / (float)maxAmmo) * (1.0f - (distance / (float)maxDistance)) * 0.2f;
					}
					break;
				case WP_RAILGUN:
					maxAmmo = 10;
					if (playerState.stats[STAT_WEAPONS] & (1 << pWeaponPickup->GetCode()))
					{
						distance = (playerState.itemDistance[item] < maxDistance) ?
							playerState.itemDistance[item] : maxDistance;
						ammo = (playerState.itemAmount[item] < maxAmmo) ?
							playerState.itemAmount[item] : maxAmmo;

						//relation based on amount gained and distance travelled
						heuristic += (ammo / (float)maxAmmo) * (1.0f - (distance / (float)maxDistance)) * 0.2f;
					}
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
						heuristic += (ammo / (float)maxAmmo) * (1.0f - (distance / (float)maxDistance)) * 0.2f;
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
						heuristic += (ammo / (float)maxAmmo) * (1.0f - (distance / (float)maxDistance)) * 0.2f;
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

	//player status calculation
	for (int weapon = 1; weapon < MAX_WEAPONS; weapon++)
	{
		switch (weapon)
		{
			case WP_LIGHTNING:
				maxAmmo = 200;
				if (playerState.stats[STAT_WEAPONS] & (1 << weapon))
				{
					ammo = (playerState.ammo[weapon] < maxAmmo) ? playerState.ammo[weapon] : maxAmmo;
					heuristic += (ammo / (float)maxAmmo) * 0.1f;
				}
				break;
			case WP_SHOTGUN:
				maxAmmo = 20;
				if (playerState.stats[STAT_WEAPONS] & (1 << weapon))
				{
					ammo = (playerState.ammo[weapon] < maxAmmo) ? playerState.ammo[weapon] : maxAmmo;
					heuristic += (ammo / (float)maxAmmo) * 0.1f;
				}
				break;
			case WP_MACHINEGUN:
				maxAmmo = 200;
				if (playerState.stats[STAT_WEAPONS] & (1 << weapon))
				{
					ammo = (playerState.ammo[weapon] < maxAmmo) ? playerState.ammo[weapon] : maxAmmo;
					heuristic += (ammo / (float)maxAmmo) * 0.05f;
				}
				break;
			case WP_PLASMAGUN:
				maxAmmo = 200;
				if (playerState.stats[STAT_WEAPONS] & (1 << weapon))
				{
					ammo = (playerState.ammo[weapon] < maxAmmo) ? playerState.ammo[weapon] : maxAmmo;
					heuristic += (ammo / (float)maxAmmo) * 0.05f;
				}
				break;
			case WP_GRENADE_LAUNCHER:
				maxAmmo = 20;
				if (playerState.stats[STAT_WEAPONS] & (1 << weapon))
				{
					ammo = (playerState.ammo[weapon] < maxAmmo) ? playerState.ammo[weapon] : maxAmmo;
					heuristic += (ammo / (float)maxAmmo) * 0.f;
				}
				break;
			case WP_ROCKET_LAUNCHER:
				maxAmmo = 20;
				if (playerState.stats[STAT_WEAPONS] & (1 << weapon))
				{
					ammo = (playerState.ammo[weapon] < maxAmmo) ? playerState.ammo[weapon] : maxAmmo;
					heuristic += (ammo / (float)maxAmmo) * 0.2f;
				}
				break;
			case WP_RAILGUN:
				maxAmmo = 20;
				if (playerState.stats[STAT_WEAPONS] & (1 << weapon))
				{
					ammo = (playerState.ammo[weapon] < maxAmmo) ? playerState.ammo[weapon] : maxAmmo;
					heuristic += (ammo / (float)maxAmmo) * 0.2f;
				}
				break;
		}	
	}

	//health and armor
	{
		unsigned int maxHealth = 100;
		unsigned int maxArmor = 100;
		heuristic += (playerState.stats[STAT_HEALTH] / (float)maxHealth) * 0.3f;
		heuristic += (playerState.stats[STAT_ARMOR] / (float)maxArmor) * 0.3f;
	}

	//the other part of the heuristic is the damage dealer
	int maxDamage = 0;
	for (int weapon = 1; weapon <= MAX_WEAPONS; weapon++)
	{
		if (playerState.damage[weapon - 1] > maxDamage)
		{
			playerState.target = otherPlayerState.player;
			playerState.weapon = (WeaponType)weapon;
			maxDamage = playerState.damage[weapon - 1];
		}
	}

	//damage threshold to the max hp/armor
	if (maxDamage > 400) maxDamage = 400;
	heuristic += (maxDamage / 400.f) * 0.4f;

	//calculate the other player heuristic the same way but reversing the sign
	for (eastl::shared_ptr<Actor> item : otherPlayerState.items)
	{
		if (item->GetType() == "Weapon")
		{
			eastl::shared_ptr<WeaponPickup> pWeaponPickup =
				item->GetComponent<WeaponPickup>(WeaponPickup::Name).lock();

			switch (pWeaponPickup->GetCode())
			{
				case WP_LIGHTNING:
					maxAmmo = 60;
					if (otherPlayerState.stats[STAT_WEAPONS] & (1 << pWeaponPickup->GetCode()))
					{
						distance = (otherPlayerState.itemDistance[item] < maxDistance) ?
							otherPlayerState.itemDistance[item] : maxDistance;
						ammo = (otherPlayerState.itemAmount[item] < maxAmmo) ?
							otherPlayerState.itemAmount[item] : maxAmmo;

						//relation based on amount gained and distance travelled
						heuristic -= (ammo / (float)maxAmmo) * (1.0f - (distance / (float)maxDistance)) * 0.1f;
					}
					break;
				case WP_SHOTGUN:
					maxAmmo = 10;
					if (otherPlayerState.stats[STAT_WEAPONS] & (1 << pWeaponPickup->GetCode()))
					{
						distance = (otherPlayerState.itemDistance[item] < maxDistance) ?
							otherPlayerState.itemDistance[item] : maxDistance;
						ammo = (otherPlayerState.itemAmount[item] < maxAmmo) ?
							otherPlayerState.itemAmount[item] : maxAmmo;

						//relation based on amount gained and distance travelled
						heuristic -= (ammo / (float)maxAmmo) * (1.0f - (distance / (float)maxDistance)) * 0.1f;
					}
					break;
				case WP_MACHINEGUN:
					maxAmmo = 50;
					if (otherPlayerState.stats[STAT_WEAPONS] & (1 << pWeaponPickup->GetCode()))
					{
						distance = (otherPlayerState.itemDistance[item] < maxDistance) ?
							otherPlayerState.itemDistance[item] : maxDistance;
						ammo = (otherPlayerState.itemAmount[item] < maxAmmo) ?
							otherPlayerState.itemAmount[item] : maxAmmo;

						//relation based on amount gained and distance travelled
						heuristic -= (ammo / (float)maxAmmo) * (1.0f - (distance / (float)maxDistance)) * 0.05f;
					}
					break;
				case WP_PLASMAGUN:
					maxAmmo = 30;
					if (otherPlayerState.stats[STAT_WEAPONS] & (1 << pWeaponPickup->GetCode()))
					{
						distance = (otherPlayerState.itemDistance[item] < maxDistance) ?
							otherPlayerState.itemDistance[item] : maxDistance;
						ammo = (otherPlayerState.itemAmount[item] < maxAmmo) ?
							otherPlayerState.itemAmount[item] : maxAmmo;

						//relation based on amount gained and distance travelled
						heuristic -= (ammo / (float)maxAmmo) * (1.0f - (distance / (float)maxDistance)) * 0.05f;
					}
					break;
				case WP_GRENADE_LAUNCHER:
					maxAmmo = 5;
					if (otherPlayerState.stats[STAT_WEAPONS] & (1 << pWeaponPickup->GetCode()))
					{
						distance = (otherPlayerState.itemDistance[item] < maxDistance) ?
							otherPlayerState.itemDistance[item] : maxDistance;
						ammo = (otherPlayerState.itemAmount[item] < maxAmmo) ?
							otherPlayerState.itemAmount[item] : maxAmmo;

						//relation based on amount gained and distance travelled
						heuristic -= (ammo / (float)maxAmmo) * (1.0f - (distance / (float)maxDistance)) * 0.f;
					}
					break;
				case WP_ROCKET_LAUNCHER:
					maxAmmo = 5;
					if (otherPlayerState.stats[STAT_WEAPONS] & (1 << pWeaponPickup->GetCode()))
					{
						distance = (otherPlayerState.itemDistance[item] < maxDistance) ?
							otherPlayerState.itemDistance[item] : maxDistance;
						ammo = (otherPlayerState.itemAmount[item] < maxAmmo) ?
							otherPlayerState.itemAmount[item] : maxAmmo;

						//relation based on amount gained and distance travelled
						heuristic -= (ammo / (float)maxAmmo) * (1.0f - (distance / (float)maxDistance)) * 0.2f;
					}
					break;
				case WP_RAILGUN:
					maxAmmo = 10;
					if (otherPlayerState.stats[STAT_WEAPONS] & (1 << pWeaponPickup->GetCode()))
					{
						distance = (otherPlayerState.itemDistance[item] < maxDistance) ?
							otherPlayerState.itemDistance[item] : maxDistance;
						ammo = (otherPlayerState.itemAmount[item] < maxAmmo) ?
							otherPlayerState.itemAmount[item] : maxAmmo;

						//relation based on amount gained and distance travelled
						heuristic -= (ammo / (float)maxAmmo) * (1.0f - (distance / (float)maxDistance)) * 0.2f;
					}
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
					if (otherPlayerState.stats[STAT_WEAPONS] & (1 << pAmmoPickup->GetCode()))
					{
						distance = (otherPlayerState.itemDistance[item] < maxDistance) ?
							otherPlayerState.itemDistance[item] : maxDistance;
						ammo = (otherPlayerState.itemAmount[item] < maxAmmo) ?
							otherPlayerState.itemAmount[item] : maxAmmo;

						//relation based on amount gained and distance travelled
						heuristic -= (ammo / (float)maxAmmo) * (1.0f - (distance / (float)maxDistance)) * 0.1f;
					}
					break;
				case WP_SHOTGUN:
					maxAmmo = 10;
					if (otherPlayerState.stats[STAT_WEAPONS] & (1 << pAmmoPickup->GetCode()))
					{
						distance = (otherPlayerState.itemDistance[item] < maxDistance) ?
							otherPlayerState.itemDistance[item] : maxDistance;
						ammo = (otherPlayerState.itemAmount[item] < maxAmmo) ?
							otherPlayerState.itemAmount[item] : maxAmmo;

						//relation based on amount gained and distance travelled
						heuristic -= (ammo / (float)maxAmmo) * (1.0f - (distance / (float)maxDistance)) * 0.1f;
					}
					break;
				case WP_MACHINEGUN:
					maxAmmo = 50;
					if (otherPlayerState.stats[STAT_WEAPONS] & (1 << pAmmoPickup->GetCode()))
					{
						distance = (otherPlayerState.itemDistance[item] < maxDistance) ?
							otherPlayerState.itemDistance[item] : maxDistance;
						ammo = (otherPlayerState.itemAmount[item] < maxAmmo) ?
							otherPlayerState.itemAmount[item] : maxAmmo;

						//relation based on amount gained and distance travelled
						heuristic -= (ammo / (float)maxAmmo) * (1.0f - (distance / (float)maxDistance)) * 0.05f;
					}
					break;
				case WP_PLASMAGUN:
					maxAmmo = 30;
					if (otherPlayerState.stats[STAT_WEAPONS] & (1 << pAmmoPickup->GetCode()))
					{
						distance = (otherPlayerState.itemDistance[item] < maxDistance) ?
							otherPlayerState.itemDistance[item] : maxDistance;
						ammo = (otherPlayerState.itemAmount[item] < maxAmmo) ?
							otherPlayerState.itemAmount[item] : maxAmmo;

						//relation based on amount gained and distance travelled
						heuristic -= (ammo / (float)maxAmmo) * (1.0f - (distance / (float)maxDistance)) * 0.05f;
					}
					break;
				case WP_GRENADE_LAUNCHER:
					maxAmmo = 5;
					if (otherPlayerState.stats[STAT_WEAPONS] & (1 << pAmmoPickup->GetCode()))
					{
						distance = (otherPlayerState.itemDistance[item] < maxDistance) ?
							otherPlayerState.itemDistance[item] : maxDistance;
						ammo = (otherPlayerState.itemAmount[item] < maxAmmo) ?
							otherPlayerState.itemAmount[item] : maxAmmo;

						//relation based on amount gained and distance travelled
						heuristic -= (ammo / (float)maxAmmo) * (1.0f - (distance / (float)maxDistance)) * 0.f;
					}
					break;
				case WP_ROCKET_LAUNCHER:
					maxAmmo = 5;
					if (otherPlayerState.stats[STAT_WEAPONS] & (1 << pAmmoPickup->GetCode()))
					{
						distance = (otherPlayerState.itemDistance[item] < maxDistance) ?
							otherPlayerState.itemDistance[item] : maxDistance;
						ammo = (otherPlayerState.itemAmount[item] < maxAmmo) ?
							otherPlayerState.itemAmount[item] : maxAmmo;

						//relation based on amount gained and distance travelled
						heuristic -= (ammo / (float)maxAmmo) * (1.0f - (distance / (float)maxDistance)) * 0.2f;
					}
					break;
				case WP_RAILGUN:
					maxAmmo = 10;
					if (otherPlayerState.stats[STAT_WEAPONS] & (1 << pAmmoPickup->GetCode()))
					{
						distance = (otherPlayerState.itemDistance[item] < maxDistance) ?
							otherPlayerState.itemDistance[item] : maxDistance;
						ammo = (otherPlayerState.itemAmount[item] < maxAmmo) ?
							otherPlayerState.itemAmount[item] : maxAmmo;

						//relation based on amount gained and distance travelled
						heuristic -= (ammo / (float)maxAmmo) * (1.0f - (distance / (float)maxDistance)) * 0.2f;
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

			distance = (otherPlayerState.itemDistance[item] < maxDistance) ?
				otherPlayerState.itemDistance[item] : maxDistance;
			armor = (otherPlayerState.itemAmount[item] < maxArmor) ?
				otherPlayerState.itemAmount[item] : maxArmor;

			//relation based on amount gained and distance travelled
			heuristic -= (armor / (float)maxArmor) * (1.0f - (distance / (float)maxDistance)) * 0.3f;

		}
		else if (item->GetType() == "Health")
		{
			eastl::shared_ptr<HealthPickup> pHealthPickup =
				item->GetComponent<HealthPickup>(HealthPickup::Name).lock();

			int health = 0;
			int maxHealth = 100;

			distance = (otherPlayerState.itemDistance[item] < maxDistance) ?
				otherPlayerState.itemDistance[item] : maxDistance;
			health = (otherPlayerState.itemAmount[item] < maxHealth) ?
				otherPlayerState.itemAmount[item] : maxHealth;

			//relation based on amount gained and distance travelled
			heuristic -= (health / (float)maxHealth) * (1.0f - (distance / (float)maxDistance)) * 0.3f;
		}
	}

	//player status calculation
	for (int weapon = 1; weapon < MAX_WEAPONS; weapon++)
	{
		switch (weapon)
		{
			case WP_LIGHTNING:
				maxAmmo = 200;
				if (otherPlayerState.stats[STAT_WEAPONS] & (1 << weapon))
				{
					ammo = (otherPlayerState.ammo[weapon] < maxAmmo) ? otherPlayerState.ammo[weapon] : maxAmmo;
					heuristic -= (ammo / (float)maxAmmo) * 0.1f;
				}
				break;
			case WP_SHOTGUN:
				maxAmmo = 20;
				if (otherPlayerState.stats[STAT_WEAPONS] & (1 << weapon))
				{
					ammo = (otherPlayerState.ammo[weapon] < maxAmmo) ? otherPlayerState.ammo[weapon] : maxAmmo;
					heuristic -= (ammo / (float)maxAmmo) * 0.1f;
				}
				break;
			case WP_MACHINEGUN:
				maxAmmo = 200;
				if (otherPlayerState.stats[STAT_WEAPONS] & (1 << weapon))
				{
					ammo = (otherPlayerState.ammo[weapon] < maxAmmo) ? otherPlayerState.ammo[weapon] : maxAmmo;
					heuristic -= (ammo / (float)maxAmmo) * 0.05f;
				}
				break;
			case WP_PLASMAGUN:
				maxAmmo = 200;
				if (otherPlayerState.stats[STAT_WEAPONS] & (1 << weapon))
				{
					ammo = (otherPlayerState.ammo[weapon] < maxAmmo) ? otherPlayerState.ammo[weapon] : maxAmmo;
					heuristic -= (ammo / (float)maxAmmo) * 0.05f;
				}
				break;
			case WP_GRENADE_LAUNCHER:
				maxAmmo = 20;
				if (otherPlayerState.stats[STAT_WEAPONS] & (1 << weapon))
				{
					ammo = (otherPlayerState.ammo[weapon] < maxAmmo) ? otherPlayerState.ammo[weapon] : maxAmmo;
					heuristic -= (ammo / (float)maxAmmo) * 0.f;
				}
				break;
			case WP_ROCKET_LAUNCHER:
				maxAmmo = 20;
				if (otherPlayerState.stats[STAT_WEAPONS] & (1 << weapon))
				{
					ammo = (otherPlayerState.ammo[weapon] < maxAmmo) ? otherPlayerState.ammo[weapon] : maxAmmo;
					heuristic -= (ammo / (float)maxAmmo) * 0.2f;
				}
				break;
			case WP_RAILGUN:
				maxAmmo = 20;
				if (otherPlayerState.stats[STAT_WEAPONS] & (1 << weapon))
				{
					ammo = (otherPlayerState.ammo[weapon] < maxAmmo) ? otherPlayerState.ammo[weapon] : maxAmmo;
					heuristic -= (ammo / (float)maxAmmo) * 0.2f;
				}
				break;
		}
	}

	//health and armor
	{
		unsigned int maxHealth = 100;
		unsigned int maxArmor = 100;
		heuristic -= (otherPlayerState.stats[STAT_HEALTH] / (float)maxHealth) * 0.3f;
		heuristic -= (otherPlayerState.stats[STAT_ARMOR] / (float)maxArmor) * 0.3f;
	}

	//the other part of the heuristic is the damage dealer
	maxDamage = 0;
	for (int weapon = 1; weapon <= MAX_WEAPONS; weapon++)
	{
		if (otherPlayerState.damage[weapon - 1] > maxDamage)
		{
			otherPlayerState.target = playerState.player;
			otherPlayerState.weapon = (WeaponType)weapon;
			maxDamage = otherPlayerState.damage[weapon - 1];
		}
	}

	//damage threshold to the max hp/armor
	if (maxDamage > 400) maxDamage = 400;
	heuristic -= (maxDamage / 400.f) * 0.4f;

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
						damage = 4;
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
						damage = 4;
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
						if (visibleHeight <= 20.f)
							rangeDistance = visibleDistance > 600 ? visibleDistance : 600;
						else 
							rangeDistance = visibleDistance > 1200 ? visibleDistance : 1200;
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
			if (pAmmoPickup->mRespawnTime - actor.second > 0)
				continue;

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
			if (pArmorPickup->mRespawnTime - actor.second > 0)
				continue;

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
			if (pHealthPickup->mRespawnTime - actor.second > 0)
				continue;

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

		PathingNode* currentNode = otherPlayerNode;
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
	bestOtherPlayerState.heuristic = FLT_MAX;
	for (auto evaluateOtherPlayerState : otherPlayerStates)
	{
		if (evaluateOtherPlayerState.second.heuristic < bestOtherPlayerState.heuristic)
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

void QuakeAIProcess::ConstructPath(PathingNode* playerNode, 
	eastl::map<unsigned int, PathingCluster*>& playerClusters,
	eastl::vector<PathingArcVec>& playerPathPlan)
{
	for (auto playerCluster : playerClusters)
	{
		eastl::vector<ActorId> actors;
		PathingArcVec clusterPathPlan;
		PathingNode* currentNode = playerNode;
		while (currentNode != playerCluster.second->GetTarget())
		{
			for (PathingArc* pathArc : currentNode->GetArcs())
			{
				if (pathArc->GetNode()->GetActorId() != INVALID_ACTOR_ID)
				{
					if (eastl::find(actors.begin(), actors.end(), pathArc->GetNode()->GetActorId()) == actors.end())
					{
						actors.push_back(pathArc->GetNode()->GetActorId());

						PathingArcVec actorPathPlan;
						for (PathingArc* pathArc : clusterPathPlan)
							actorPathPlan.push_back(pathArc);

						actorPathPlan.push_back(pathArc);
						playerPathPlan.push_back(actorPathPlan);
					}
				}
			}

			PathingCluster* currentCluster = 
				currentNode->FindCluster(playerCluster.first, playerCluster.second->GetTarget());
			PathingArc* currentArc = currentNode->FindArc(currentCluster->GetNode());
			PathingTransition* currentTransition = currentNode->FindTransition(currentArc->GetId());
			for (PathingNode* nodeTransition : currentTransition->GetNodes())
			{
				if (nodeTransition->GetActorId() != INVALID_ACTOR_ID)
					if (eastl::find(actors.begin(), actors.end(), nodeTransition->GetActorId()) == actors.end())
						actors.push_back(nodeTransition->GetActorId());
			}
			clusterPathPlan.push_back(currentArc);

			currentNode = currentArc->GetNode();
			if (currentNode->GetActorId() != INVALID_ACTOR_ID)
				if (eastl::find(actors.begin(), actors.end(), currentNode->GetActorId()) == actors.end())
					actors.push_back(currentNode->GetActorId());
		}
		playerPathPlan.push_back(clusterPathPlan);

		currentNode = playerNode;
		clusterPathPlan = PathingArcVec();
		while (currentNode != playerCluster.second->GetTarget())
		{
			PathingClusterVec actorClusters;
			currentNode->GetClusterActors(playerCluster.first, actorClusters);
			for (PathingCluster* actorCluster : actorClusters)
			{
				if (eastl::find(actors.begin(), actors.end(), actorCluster->GetActor()) == actors.end())
				{
					PathingArcVec actorPathPlan;
					for (PathingArc* pathArc : clusterPathPlan)
						actorPathPlan.push_back(pathArc);

					PathingNode* actorNode = currentNode;
					while (actorNode != actorCluster->GetTarget())
					{
						PathingCluster* currentCluster =
							actorNode->FindCluster(actorCluster->GetType(), actorCluster->GetTarget());
						PathingArc* currentArc = actorNode->FindArc(currentCluster->GetNode());
						PathingTransition* currentTransition = actorNode->FindTransition(currentArc->GetId());
						for (PathingNode* nodeTransition : currentTransition->GetNodes())
						{
							if (nodeTransition->GetActorId() != INVALID_ACTOR_ID)
								if (eastl::find(actors.begin(), actors.end(), nodeTransition->GetActorId()) == actors.end())
									actors.push_back(nodeTransition->GetActorId());
						}
						actorPathPlan.push_back(currentArc);

						actorNode = currentArc->GetNode();
						if (actorNode->GetActorId() != INVALID_ACTOR_ID)
							if (eastl::find(actors.begin(), actors.end(), actorNode->GetActorId()) == actors.end())
								actors.push_back(actorNode->GetActorId());
					}

					playerPathPlan.push_back(actorPathPlan);
				}
			}

			PathingCluster* currentCluster =
				currentNode->FindCluster(playerCluster.first, playerCluster.second->GetTarget());
			PathingArc* currentArc = currentNode->FindArc(currentCluster->GetNode());
			clusterPathPlan.push_back(currentArc);

			currentNode = currentArc->GetNode();
		}
	}
}

void QuakeAIProcess::EvaluateNode(NodeState& playerState, NodeState& otherPlayerState)
{
	Simulation(playerState, otherPlayerState);
}

void QuakeAIProcess::EvaluateNode(
	NodeState& playerState, NodeState& otherPlayerState, 
	eastl::map<unsigned int, PathingCluster*>& otherPlayerClusters)
{
	//construct path
	eastl::vector<PathingArcVec> otherNodePathPlans;
	ConstructPath(otherPlayerState.node, otherPlayerClusters, otherNodePathPlans);

	Simulation(playerState, otherPlayerState, otherNodePathPlans);
}

void QuakeAIProcess::EvaluateNode(
	NodeState& playerState, NodeState& otherPlayerState,
	eastl::map<unsigned int, PathingCluster*>& playerClusters, 
	eastl::map<unsigned int, PathingCluster*>& otherPlayerClusters)
{
	//construct path
	eastl::vector<PathingArcVec> nodePathPlans, otherNodePathPlans;
	ConstructPath(playerState.node, playerClusters, nodePathPlans);
	ConstructPath(otherPlayerState.node, otherPlayerClusters, otherNodePathPlans);

	Simulation(playerState, nodePathPlans, otherPlayerState, otherNodePathPlans);
}

void QuakeAIProcess::EvaluateCluster(NodeState& playerState, NodeState& otherPlayerState)
{
	fprintf(mFile, "\n\n ITERATION \n\n");

	//single case playerNode - otherPlayerNode
	NodeState playerNodeState(playerState); 
	NodeState otherPlayerNodeState(otherPlayerState);
	EvaluateNode(playerNodeState, otherPlayerNodeState);

	PathingClusterVec playerClusters;
	playerState.node->GetClusters(playerClusters, 30);
	unsigned int clusterSize = playerClusters.size();

	//fprintf(mFile, "\n playerCluster - otherPlayerNode \n");
	eastl::map<PathingCluster*, NodeState> playerClusterNodeStates, otherPlayerNodeClusterStates;
	for (unsigned int playerClusterIdx = 0; playerClusterIdx < clusterSize; playerClusterIdx++)
	//parallel_for(size_t(0), clusterSize, [&](size_t playerClusterIdx)
	{
		PathingCluster* playerCluster = playerClusters[playerClusterIdx];
		/*
		fprintf(mFile, "player cluster type %u actor %u target %u pos %f %f %f length %f \n",
			playerCluster->GetType(), playerCluster->GetActor(), playerCluster->GetTarget()->GetCluster(),
			playerCluster->GetTarget()->GetPos()[0],
			playerCluster->GetTarget()->GetPos()[1],
			playerCluster->GetTarget()->GetPos()[2],
			Length(playerState.node->GetPos() - playerCluster->GetTarget()->GetPos()));
		*/
		eastl::map<unsigned int, PathingCluster*> clusters;
		clusters[playerCluster->GetType()] = playerCluster;
		for (PathingCluster* cluster : playerState.node->GetClusters())
		{
			if (cluster->GetActor() == INVALID_ACTOR_ID)
				if (clusters.find(cluster->GetType()) == clusters.end())
					if (playerCluster->GetTarget()->GetCluster() == cluster->GetTarget()->GetCluster())
						clusters[cluster->GetType()] = cluster;
		}

		NodeState state(playerState);
		NodeState otherState(otherPlayerState);
		EvaluateNode(otherState, state, clusters);
		state.heuristic = -state.heuristic;
		otherState.heuristic = -otherState.heuristic;

		if (state.valid && otherState.valid)
		{
			/*
			PathingNode* pathNode = state.node;
			PathingNode* otherPathNode = otherState.node;
			fprintf(mFile, "player pos %f %f %f, id %u, heuristic %f, target %u, weapon %u, damage %u, paths %u \n other player pos %f %f %f, id %u, heuristic %f, target %u, weapon %u, damage %u, paths %u \n",
				pathNode->GetPos()[0], pathNode->GetPos()[1], pathNode->GetPos()[2],
				pathNode->GetId(), state.heuristic, state.target, state.weapon, 
				state.weapon > 0 ? state.damage[state.weapon - 1] : 0, state.path.size(),
				otherPathNode->GetPos()[0], otherPathNode->GetPos()[1], otherPathNode->GetPos()[2],
				otherPathNode->GetId(), otherState.heuristic, otherState.target, otherState.weapon, 
				otherState.weapon > 0 ? otherState.damage[otherState.weapon - 1] : 0, otherState.path.size());
			*/
			playerClusterNodeStates[playerCluster] = state;
			otherPlayerNodeClusterStates[playerCluster] = otherState;
		}
	}

	PathingClusterVec otherPlayerClusters;
	otherPlayerState.node->GetClusters(otherPlayerClusters, 30);
	unsigned int otherClusterSize = otherPlayerClusters.size();

	//fprintf(mFile, "\n playerNode - otherPlayerCluster \n");
	eastl::map<PathingCluster*, NodeState> playerNodeClusterStates, otherPlayerClusterNodeStates;
	for (unsigned int otherPlayerClusterIdx = 0; otherPlayerClusterIdx < otherClusterSize; otherPlayerClusterIdx++)
	//parallel_for(size_t(0), otherClusterSize, [&](size_t otherPlayerClusterIdx)
	{
		PathingCluster* otherPlayerCluster = otherPlayerClusters[otherPlayerClusterIdx];
		/*
		fprintf(mFile, "other player cluster type %u actor %u target %u pos %f %f %f length %f \n",
			otherPlayerCluster->GetType(), otherPlayerCluster->GetActor(), otherPlayerCluster->GetTarget()->GetCluster(),
			otherPlayerCluster->GetTarget()->GetPos()[0],
			otherPlayerCluster->GetTarget()->GetPos()[1],
			otherPlayerCluster->GetTarget()->GetPos()[2],
			Length(otherPlayerState.node->GetPos() - otherPlayerCluster->GetTarget()->GetPos()));
		*/
		eastl::map<unsigned int, PathingCluster*> otherClusters;
		otherClusters[otherPlayerCluster->GetType()] = otherPlayerCluster;
		for (PathingCluster* otherCluster : otherPlayerState.node->GetClusters())
		{
			if (otherCluster->GetActor() == INVALID_ACTOR_ID)
				if (otherClusters.find(otherCluster->GetType()) == otherClusters.end())
					if (otherPlayerCluster->GetTarget()->GetCluster() == otherCluster->GetTarget()->GetCluster())
						otherClusters[otherCluster->GetType()] = otherCluster;
		}

		NodeState state(playerState);
		NodeState otherState(otherPlayerState);
		EvaluateNode(state, otherState, otherClusters);

		if (state.valid && otherState.valid)
		{
			/*
			PathingNode* pathNode = state.node;
			PathingNode* otherPathNode = otherState.node;
			fprintf(mFile, "player pos %f %f %f, id %u, heuristic %f, target %u, weapon %u, damage %u, paths %u \n other player pos %f %f %f, id %u, heuristic %f, target %u, weapon %u, damage %u, paths %u \n",
				pathNode->GetPos()[0], pathNode->GetPos()[1], pathNode->GetPos()[2],
				pathNode->GetId(), state.heuristic, state.target, state.weapon,
				state.weapon > 0 ? state.damage[state.weapon - 1] : 0, state.path.size(),
				otherPathNode->GetPos()[0], otherPathNode->GetPos()[1], otherPathNode->GetPos()[2],
				otherPathNode->GetId(), otherState.heuristic, otherState.target, otherState.weapon,
				otherState.weapon > 0 ? otherState.damage[otherState.weapon - 1] : 0, otherState.path.size());
			*/
			playerNodeClusterStates[otherPlayerCluster] = state;
			otherPlayerClusterNodeStates[otherPlayerCluster] = otherState;
		}
	}

	//fprintf(mFile, "\n playerCluster - otherPlayerCluster \n");
	eastl::map<PathingCluster*, eastl::map<PathingCluster*, NodeState>> playerClustersStates, otherPlayerClustersStates;
	for (unsigned int playerClusterIdx = 0; playerClusterIdx < clusterSize; playerClusterIdx++)
	//parallel_for(size_t(0), clusterSize, [&](size_t playerClusterIdx)
	{
		PathingCluster* playerCluster = playerClusters[playerClusterIdx];
		/*
		fprintf(mFile, "player cluster type %u actor %u target %u pos %f %f %f length %f \n",
			playerCluster->GetType(), playerCluster->GetActor(), playerCluster->GetTarget()->GetCluster(),
			playerCluster->GetTarget()->GetPos()[0],
			playerCluster->GetTarget()->GetPos()[1],
			playerCluster->GetTarget()->GetPos()[2],
			Length(playerState.node->GetPos() - playerCluster->GetTarget()->GetPos()));
		*/
		eastl::map<unsigned int, PathingCluster*> clusters;
		clusters[playerCluster->GetType()] = playerCluster;
		for (PathingCluster* cluster : playerState.node->GetClusters())
		{
			if (cluster->GetActor() == INVALID_ACTOR_ID)
				if (clusters.find(cluster->GetType()) == clusters.end())
					if (playerCluster->GetTarget()->GetCluster() == cluster->GetTarget()->GetCluster())
						clusters[cluster->GetType()] = cluster;
		}

		for (unsigned int otherPlayerClusterIdx = 0; otherPlayerClusterIdx < otherClusterSize; otherPlayerClusterIdx++)
		//parallel_for(size_t(0), otherClusterSize, [&](size_t otherPlayerClusterIdx)
		{
			PathingCluster* otherPlayerCluster = otherPlayerClusters[otherPlayerClusterIdx];
			/*
			fprintf(mFile, "other player cluster type %u actor %u target %u pos %f %f %f length %f \n",
				otherPlayerCluster->GetType(), otherPlayerCluster->GetActor(), otherPlayerCluster->GetTarget()->GetCluster(),
				otherPlayerCluster->GetTarget()->GetPos()[0],
				otherPlayerCluster->GetTarget()->GetPos()[1],
				otherPlayerCluster->GetTarget()->GetPos()[2],
				Length(otherPlayerState.node->GetPos() - otherPlayerCluster->GetTarget()->GetPos()));
			*/
			eastl::map<unsigned int, PathingCluster*> otherClusters;
			otherClusters[otherPlayerCluster->GetType()] = otherPlayerCluster;
			/*
			for (PathingCluster* otherCluster : otherPlayerState.node->GetClusters())
			{
				if (otherCluster->GetActor() == INVALID_ACTOR_ID)
					if (otherClusters.find(otherCluster->GetType()) == otherClusters.end())
						if (otherPlayerCluster->GetTarget()->GetCluster() == otherCluster->GetTarget()->GetCluster())
							otherClusters[otherCluster->GetType()] = otherCluster;
			}
			*/

			NodeState state(playerState);
			NodeState otherState(otherPlayerState);
			EvaluateNode(state, otherState, clusters, otherClusters);

			if (state.valid && otherState.valid)
			{
				/*
				PathingNode* pathNode = state.node;
				PathingNode* otherPathNode = otherState.node;
				fprintf(mFile, "player pos %f %f %f, id %u, heuristic %f, target %u, weapon %u, damage %u, paths %u \n other player pos %f %f %f, id %u, heuristic %f, target %u, weapon %u, damage %u, paths %u \n",
					pathNode->GetPos()[0], pathNode->GetPos()[1], pathNode->GetPos()[2],
					pathNode->GetId(), state.heuristic, state.target, state.weapon,
					state.weapon > 0 ? state.damage[state.weapon - 1] : 0, state.path.size(),
					otherPathNode->GetPos()[0], otherPathNode->GetPos()[1], otherPathNode->GetPos()[2],
					otherPathNode->GetId(), otherState.heuristic, otherState.target, otherState.weapon,
					otherState.weapon > 0 ? otherState.damage[otherState.weapon - 1] : 0, otherState.path.size());
				*/
				playerClustersStates[playerCluster][otherPlayerCluster] = state;
				otherPlayerClustersStates[otherPlayerCluster][playerCluster] = otherState;
			}
		}
	}

	//minimax
	fprintf(mFile, "\n minimax player \n");
	fprintf(mFile, "node : ");
	for (auto playerNodeClusterState : playerNodeClusterStates)
	{
		fprintf(mFile, "%f ", playerNodeClusterState.second.heuristic);

		eastl::vector<ActorId> actors;
		PathingArcVec::iterator itArc = playerNodeClusterState.second.path.begin();
		for (; itArc != playerNodeClusterState.second.path.end(); itArc++)
		{
			//printf("%u ", (*itArc)->GetNode()->GetId());
			if ((*itArc)->GetNode()->GetActorId() != INVALID_ACTOR_ID)
				actors.push_back((*itArc)->GetNode()->GetActorId());
		}

		fprintf(mFile, " actors : ");
		for (ActorId actor : actors)
		{
			eastl::shared_ptr<Actor> pItemActor(
				eastl::dynamic_shared_pointer_cast<Actor>(
					GameLogic::Get()->GetActor(actor).lock()));
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

		if (abs(playerNodeClusterState.second.heuristic - playerNodeState.heuristic) <= GE_ROUNDING_ERROR)
		{
			if (playerNodeState.weapon != WP_NONE && playerNodeClusterState.second.weapon != WP_NONE)
			{
				if (playerNodeClusterState.second.damage[playerNodeClusterState.second.weapon - 1] >
					playerNodeState.damage[playerNodeState.weapon - 1])
				{
					playerNodeState = playerNodeClusterState.second;
				}
			}
			else if (playerNodeClusterState.second.weapon != WP_NONE)
			{
				playerNodeState = playerNodeClusterState.second;
			}
		}
		else if (playerNodeClusterState.second.heuristic < playerNodeState.heuristic)
			playerNodeState = playerNodeClusterState.second;
	}
	if (playerNodeState.valid)
	{
		fprintf(mFile, " min %f ", playerNodeState.heuristic);
		mPlayerState.Copy(playerNodeState);

		eastl::vector<ActorId> actors;
		PathingArcVec::iterator itArc = mPlayerState.path.begin();
		for (; itArc != mPlayerState.path.end(); itArc++)
		{
			//printf("%u ", (*itArc)->GetNode()->GetId());
			if ((*itArc)->GetNode()->GetActorId() != INVALID_ACTOR_ID)
				actors.push_back((*itArc)->GetNode()->GetActorId());
		}

		fprintf(mFile, " actors : ");
		for (ActorId actor : actors)
		{
			eastl::shared_ptr<Actor> pItemActor(
				eastl::dynamic_shared_pointer_cast<Actor>(
					GameLogic::Get()->GetActor(actor).lock()));
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

	for (auto playerClustersState : playerClustersStates)
	{
		playerNodeState = playerClusterNodeStates[playerClustersState.first];
		fprintf(mFile, "\n cluster : %f ", playerNodeState.heuristic);
		for (auto playerClusterState : playerClustersState.second)
		{
			fprintf(mFile, "%f ", playerClusterState.second.heuristic);

			eastl::vector<ActorId> actors;
			PathingArcVec::iterator itArc = playerClusterState.second.path.begin();
			for (; itArc != playerClusterState.second.path.end(); itArc++)
			{
				//printf("%u ", (*itArc)->GetNode()->GetId());
				if ((*itArc)->GetNode()->GetActorId() != INVALID_ACTOR_ID)
					actors.push_back((*itArc)->GetNode()->GetActorId());
			}

			fprintf(mFile, " actors : ");
			for (ActorId actor : actors)
			{
				eastl::shared_ptr<Actor> pItemActor(
					eastl::dynamic_shared_pointer_cast<Actor>(
						GameLogic::Get()->GetActor(actor).lock()));
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
			fprintf(mFile, " min %f ", playerNodeState.heuristic);

			eastl::vector<ActorId> actors;
			PathingArcVec::iterator itArc = playerNodeState.path.begin();
			for (; itArc != playerNodeState.path.end(); itArc++)
			{
				//printf("%u ", (*itArc)->GetNode()->GetId());
				if ((*itArc)->GetNode()->GetActorId() != INVALID_ACTOR_ID)
					actors.push_back((*itArc)->GetNode()->GetActorId());
			}

			fprintf(mFile, " actors : ");
			for (ActorId actor : actors)
			{
				eastl::shared_ptr<Actor> pItemActor(
					eastl::dynamic_shared_pointer_cast<Actor>(
						GameLogic::Get()->GetActor(actor).lock()));
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
		fprintf(mFile, " max %f ", mPlayerState.heuristic);

		eastl::vector<ActorId> actors;
		PathingArcVec::iterator itArc = mPlayerState.path.begin();
		for (; itArc != mPlayerState.path.end(); itArc++)
		{
			//printf("%u ", (*itArc)->GetNode()->GetId());
			if ((*itArc)->GetNode()->GetActorId() != INVALID_ACTOR_ID)
				actors.push_back((*itArc)->GetNode()->GetActorId());
		}

		fprintf(mFile, " actors : ");
		for (ActorId actor : actors)
		{
			eastl::shared_ptr<Actor> pItemActor(
				eastl::dynamic_shared_pointer_cast<Actor>(
					GameLogic::Get()->GetActor(actor).lock()));
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

	fprintf(mFile, "\n minimax otherPlayer \n");
	fprintf(mFile, "node : ");
	for (auto otherPlayerNodeClusterState : otherPlayerNodeClusterStates)
	{
		fprintf(mFile, "%f ", otherPlayerNodeClusterState.second.heuristic);

		//printf("nodes : ");
		eastl::vector<ActorId> actors;
		PathingArcVec::iterator itArc = otherPlayerNodeClusterState.second.path.begin();
		for (; itArc != otherPlayerNodeClusterState.second.path.end(); itArc++)
		{
			//printf("%u ", (*itArc)->GetNode()->GetId());
			if ((*itArc)->GetNode()->GetActorId() != INVALID_ACTOR_ID)
				actors.push_back((*itArc)->GetNode()->GetActorId());
		}

		fprintf(mFile, " actors : ");
		for (ActorId actor : actors)
		{
			eastl::shared_ptr<Actor> pItemActor(
				eastl::dynamic_shared_pointer_cast<Actor>(
					GameLogic::Get()->GetActor(actor).lock()));
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

		if (abs(otherPlayerNodeClusterState.second.heuristic - otherPlayerNodeState.heuristic) <= GE_ROUNDING_ERROR)
		{
			if (otherPlayerNodeState.weapon != WP_NONE && otherPlayerNodeClusterState.second.weapon != WP_NONE)
			{
				if (otherPlayerNodeClusterState.second.damage[otherPlayerNodeClusterState.second.weapon - 1] >
					otherPlayerNodeState.damage[otherPlayerNodeState.weapon - 1])
				{
					otherPlayerNodeState = otherPlayerNodeClusterState.second;
				}
			}
			else if (otherPlayerNodeClusterState.second.weapon != WP_NONE)
			{
				otherPlayerNodeState = otherPlayerNodeClusterState.second;
			}
		}
		else if (otherPlayerNodeClusterState.second.heuristic > otherPlayerNodeState.heuristic)
		{
			otherPlayerNodeState = otherPlayerNodeClusterState.second;
		}
	}
	if (otherPlayerNodeState.valid)
	{
		fprintf(mFile, " max %f ", otherPlayerNodeState.heuristic);
		mOtherPlayerState.Copy(otherPlayerNodeState);

		//printf("nodes : ");
		eastl::vector<ActorId> actors;
		PathingArcVec::iterator itArc = mOtherPlayerState.path.begin();
		for (; itArc != mOtherPlayerState.path.end(); itArc++)
		{
			//printf("%u ", (*itArc)->GetNode()->GetId());
			if ((*itArc)->GetNode()->GetActorId() != INVALID_ACTOR_ID)
				actors.push_back((*itArc)->GetNode()->GetActorId());
		}

		fprintf(mFile, " actors : ");
		for (ActorId actor : actors)
		{
			eastl::shared_ptr<Actor> pItemActor(
				eastl::dynamic_shared_pointer_cast<Actor>(
					GameLogic::Get()->GetActor(actor).lock()));
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

	for (auto otherPlayerClustersState : otherPlayerClustersStates)
	{
		otherPlayerNodeState = otherPlayerClusterNodeStates[otherPlayerClustersState.first];
		fprintf(mFile, "\n cluster : %f ", otherPlayerNodeState.heuristic);
		for (auto otherPlayerClusterState : otherPlayerClustersState.second)
		{
			fprintf(mFile, "%f ", otherPlayerClusterState.second.heuristic);
			
			//printf("nodes : ");
			eastl::vector<ActorId> actors;
			PathingArcVec::iterator itArc = otherPlayerClusterState.second.path.begin();
			for (; itArc != otherPlayerClusterState.second.path.end(); itArc++)
			{
				//printf("%u ", (*itArc)->GetNode()->GetId());
				if ((*itArc)->GetNode()->GetActorId() != INVALID_ACTOR_ID)
					actors.push_back((*itArc)->GetNode()->GetActorId());
			}

			fprintf(mFile, " actors : ");
			for (ActorId actor : actors)
			{
				eastl::shared_ptr<Actor> pItemActor(
					eastl::dynamic_shared_pointer_cast<Actor>(
						GameLogic::Get()->GetActor(actor).lock()));
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
				else if (otherPlayerClusterState.second.path.size() > otherPlayerNodeState.path.size())
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
			fprintf(mFile, " max %f ", otherPlayerNodeState.heuristic);

			//printf("nodes : ");
			eastl::vector<ActorId> actors;
			PathingArcVec::iterator itArc = otherPlayerNodeState.path.begin();
			for (; itArc != otherPlayerNodeState.path.end(); itArc++)
			{
				//printf("%u ", (*itArc)->GetNode()->GetId());
				if ((*itArc)->GetNode()->GetActorId() != INVALID_ACTOR_ID)
					actors.push_back((*itArc)->GetNode()->GetActorId());
			}

			fprintf(mFile, " actors : ");
			for (ActorId actor : actors)
			{
				eastl::shared_ptr<Actor> pItemActor(
					eastl::dynamic_shared_pointer_cast<Actor>(
						GameLogic::Get()->GetActor(actor).lock()));
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
			else if (otherPlayerNodeState.path.size() > mOtherPlayerState.path.size())
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
		fprintf(mFile, " min %f ", mOtherPlayerState.heuristic);
		//printf("nodes : ");
		eastl::vector<ActorId> actors;
		PathingArcVec::iterator itArc = mOtherPlayerState.path.begin();
		for (; itArc != mOtherPlayerState.path.end(); itArc++)
		{
			//printf("%u ", (*itArc)->GetNode()->GetId());
			if ((*itArc)->GetNode()->GetActorId() != INVALID_ACTOR_ID)
				actors.push_back((*itArc)->GetNode()->GetActorId());
		}

		fprintf(mFile, " actors : ");
		for (ActorId actor : actors)
		{
			eastl::shared_ptr<Actor> pItemActor(
				eastl::dynamic_shared_pointer_cast<Actor>(
					GameLogic::Get()->GetActor(actor).lock()));
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
	while (true)
	{
		if (GameLogic::Get()->GetState() == BGS_RUNNING)
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
				QuakeAIManager* aiManager = dynamic_cast<QuakeAIManager*>(GameLogic::Get()->GetAIManager());

				eastl::vector<eastl::shared_ptr<PlayerActor>>::iterator itAIPlayer;
				for (itAIPlayer = players[GV_AI].begin(); itAIPlayer != players[GV_AI].end(); )
				{
					bool removeAIPlayer = false;
					eastl::shared_ptr<PlayerActor> pAIPlayer = (*itAIPlayer);
					for (eastl::shared_ptr<PlayerActor> pHumanPlayer : players[GV_HUMAN])
						if (pHumanPlayer->GetId() == pAIPlayer->GetId())
							removeAIPlayer = true;

					if (removeAIPlayer)
						itAIPlayer = players[GV_AI].erase(itAIPlayer);
					else
						itAIPlayer++;
				}

				eastl::map<eastl::shared_ptr<PlayerActor>, PathingNode*> playerNodes, aiNodes;
				for (eastl::shared_ptr<PlayerActor> pHumanPlayer : players[GV_HUMAN])
					playerNodes[pHumanPlayer] = aiManager->GetPlayerNode(pHumanPlayer->GetId());
				for (eastl::shared_ptr<PlayerActor> pAIPlayer : players[GV_AI])
					aiNodes[pAIPlayer] = aiManager->GetPlayerNode(pAIPlayer->GetId());

				for (auto playerNode : playerNodes)
				{
					NodeState playerState(playerNode.first);
					playerState.node = playerNode.second;
					for (auto aiNode : aiNodes)
					{
						NodeState aiPlayerState(aiNode.first);
						aiPlayerState.node = aiNode.second;
						EvaluateCluster(playerState, aiPlayerState);
					}
				}

				for (eastl::shared_ptr<PlayerActor> pPlayerActor : players[GV_HUMAN])
				{
					eastl::shared_ptr<TransformComponent> pTransformComponent(
						pPlayerActor->GetComponent<TransformComponent>(TransformComponent::Name).lock());
					if (pTransformComponent)
					{
						PathingNode* pathNode =
							aiManager->GetPathingGraph()->FindClosestNode(pTransformComponent->GetPosition());
						if (mPlayerState.path.size())
						{
							/*
							printf("\n player pos %f %f %f, id %u, heuristic %f, target %u, weapon %u, damage %u, paths %u \n",
							mPlayerState.node->GetPos()[0], mPlayerState.node->GetPos()[1], mPlayerState.node->GetPos()[2],
							mPlayerState.node->GetId(), mPlayerState.heuristic, mPlayerState.target, mPlayerState.weapon,
							mPlayerState.weapon > 0 ? mPlayerState.damage[mPlayerState.weapon - 1] : 0, mPlayerState.path.size());
							*/
							//printf("\n player nodes %u : ", mPlayerState.player);
							eastl::vector<ActorId> actors;
							PathingArcVec::iterator itArc = mPlayerState.path.begin();
							for (; itArc != mPlayerState.path.end(); itArc++)
							{
								//printf("%u ", (*itArc)->GetNode()->GetId());
								if ((*itArc)->GetNode()->GetActorId() != INVALID_ACTOR_ID)
									actors.push_back((*itArc)->GetNode()->GetActorId());
							}

							fprintf(mFile, "\n player actors  %f : ", mPlayerState.heuristic);
							//printf("\n player actors  %u : ", mPlayerState.player);
							for (ActorId actor : actors)
							{
								eastl::shared_ptr<Actor> pItemActor(
									eastl::dynamic_shared_pointer_cast<Actor>(
										GameLogic::Get()->GetActor(actor).lock()));
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

							PathingNodeVec searchNodes;
							for (PathingArc* pathArc : mPlayerState.path)
								searchNodes.push_back(pathArc->GetNode());
							PathPlan* plan = aiManager->GetPathingGraph()->FindPath(pathNode, searchNodes);

							if (plan)
							{
								plan->ResetPath();

								PathingArcVec path;
								PathingArcVec::iterator itArc = mPlayerState.path.begin();
								if (!plan->CheckForEnd())
								{
									PathingNode* node = plan->GetArcs().back()->GetNode();
									for (PathingArc* planArc : plan->GetArcs())
										path.push_back(planArc);

									for (; itArc != mPlayerState.path.end(); itArc++)
										if ((*itArc)->GetNode() == node)
											break;
								}

								for (itArc++; itArc != mPlayerState.path.end(); itArc++)
									path.push_back((*itArc));
								mPlayerState.path = path;
							}

							aiManager->SetPlayerHeuristic(mPlayerState.player, -mPlayerState.heuristic);
							aiManager->SetPlayerTarget(mPlayerState.player, mPlayerState.target);
							aiManager->SetPlayerWeapon(mPlayerState.player, mPlayerState.weapon);
							aiManager->SetPlayerPath(mPlayerState.player, mPlayerState.path);
						}
					}
				}

				for (eastl::shared_ptr<PlayerActor> pPlayerActor : players[GV_AI])
				{
					eastl::shared_ptr<TransformComponent> pTransformComponent(
						pPlayerActor->GetComponent<TransformComponent>(TransformComponent::Name).lock());
					if (pTransformComponent)
					{
						PathingNode* pathNode =
							aiManager->GetPathingGraph()->FindClosestNode(pTransformComponent->GetPosition());
						if (mOtherPlayerState.path.size())
						{
							/*
							printf("\n other player pos %f %f %f, id %u, heuristic %f, target %u, weapon %u, damage %u, paths %u \n",
							mOtherPlayerState.node->GetPos()[0], mOtherPlayerState.node->GetPos()[1], mOtherPlayerState.node->GetPos()[2],
							mOtherPlayerState.node->GetId(), mOtherPlayerState.heuristic, mOtherPlayerState.target, mOtherPlayerState.weapon,
							mOtherPlayerState.weapon > 0 ? mOtherPlayerState.damage[mOtherPlayerState.weapon - 1] : 0, mOtherPlayerState.path.size());
							*/

							//printf("\n other player nodes %u : ", mOtherPlayerState.player);
							eastl::vector<ActorId> actors;
							PathingArcVec::iterator itArc = mOtherPlayerState.path.begin();
							for (; itArc != mOtherPlayerState.path.end(); itArc++)
							{
								//printf("%u ", (*itArc)->GetNode()->GetId());
								if ((*itArc)->GetNode()->GetActorId() != INVALID_ACTOR_ID)
									actors.push_back((*itArc)->GetNode()->GetActorId());
							}

							fprintf(mFile, "\n other player actors %f : ", mOtherPlayerState.heuristic);
							printf("\n other player actors %u : ", mOtherPlayerState.player);
							for (ActorId actor : actors)
							{
								eastl::shared_ptr<Actor> pItemActor(
									eastl::dynamic_shared_pointer_cast<Actor>(
										GameLogic::Get()->GetActor(actor).lock()));
								if (pItemActor->GetType() == "Weapon")
								{
									eastl::shared_ptr<WeaponPickup> pWeaponPickup =
										pItemActor->GetComponent<WeaponPickup>(WeaponPickup::Name).lock();
									fprintf(mFile, "weapon %u ", pWeaponPickup->GetCode());
									printf("weapon %u ", pWeaponPickup->GetCode());
								}
								else if (pItemActor->GetType() == "Ammo")
								{
									eastl::shared_ptr<AmmoPickup> pAmmoPickup =
										pItemActor->GetComponent<AmmoPickup>(AmmoPickup::Name).lock();
									fprintf(mFile, "ammo %u ", pAmmoPickup->GetCode());
									printf("ammo %u ", pAmmoPickup->GetCode());
								}
								else if (pItemActor->GetType() == "Armor")
								{
									eastl::shared_ptr<ArmorPickup> pArmorPickup =
										pItemActor->GetComponent<ArmorPickup>(ArmorPickup::Name).lock();
									fprintf(mFile, "armor %u ", pArmorPickup->GetCode());
									printf("armor %u ", pArmorPickup->GetCode());
								}
								else if (pItemActor->GetType() == "Health")
								{
									eastl::shared_ptr<HealthPickup> pHealthPickup =
										pItemActor->GetComponent<HealthPickup>(HealthPickup::Name).lock();
									fprintf(mFile, "health %u ", pHealthPickup->GetCode());
									printf("health %u ", pHealthPickup->GetCode());
								}
							}

							PathingNodeVec searchNodes;
							for (PathingArc* pathArc : mOtherPlayerState.path)
								searchNodes.push_back(pathArc->GetNode());
							PathPlan* plan = aiManager->GetPathingGraph()->FindPath(pathNode, searchNodes);

							if (plan)
							{
								plan->ResetPath();

								PathingArcVec path;
								PathingArcVec::iterator itArc = mOtherPlayerState.path.begin();
								if (!plan->CheckForEnd())
								{
									PathingNode* node = plan->GetArcs().back()->GetNode();
									for (PathingArc* planArc : plan->GetArcs())
										path.push_back(planArc);

									for (; itArc != mOtherPlayerState.path.end(); itArc++)
										if ((*itArc)->GetNode() == node)
											break;
								}

								for (itArc++; itArc != mOtherPlayerState.path.end(); itArc++)
									path.push_back((*itArc));
								mOtherPlayerState.path = path;
							}

							aiManager->SetPlayerHeuristic(mOtherPlayerState.player, mOtherPlayerState.heuristic);
							aiManager->SetPlayerTarget(mOtherPlayerState.player, mOtherPlayerState.target);
							aiManager->SetPlayerWeapon(mOtherPlayerState.player, mOtherPlayerState.weapon);
							aiManager->SetPlayerPath(mOtherPlayerState.player, mOtherPlayerState.path);
						}
					}
				}
			}
		}
	}

	Succeed();
}