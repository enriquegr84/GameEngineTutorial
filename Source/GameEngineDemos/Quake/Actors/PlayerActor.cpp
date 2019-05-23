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

#include "PlayerActor.h"

#include "Quake/QuakeEvents.h"

#include "Core/Logger/Logger.h"

//---------------------------------------------------------------------------------------------------------------------
// PlayerActor
//---------------------------------------------------------------------------------------------------------------------
PlayerActor::PlayerActor(ActorId id) : Actor(id)
{
	memset(&mState, 0, sizeof(mState));
	memset(&mAction, 0, sizeof(mAction));
}

PlayerActor::~PlayerActor()
{

}


void PlayerActor::StartTorsoAnim(int anim)
{
	if (mState.moveType >= PM_DEAD)
		return;

	mState.torsoAnim = anim;
}

void PlayerActor::StartLegsAnim(int anim)
{
	if (mState.moveType >= PM_DEAD)
		return;

	if (mState.legsTimer > 0)
		return;		// a high priority animation is running

	mState.legsAnim = anim;
}

void PlayerActor::ContinueLegsAnim(int anim)
{
	if (mState.legsAnim == anim)
		return;

	if (mState.legsTimer > 0)
		return;		// a high priority animation is running

	StartLegsAnim(anim);
}

void PlayerActor::ContinueTorsoAnim(int anim)
{
	if (mState.torsoAnim == anim)
		return;

	if (mState.torsoTimer > 0)
		return;		// a high priority animation is running

	StartTorsoAnim(anim);
}

void PlayerActor::ForceLegsAnim(int anim)
{
	mState.legsTimer = 0;
	StartLegsAnim(anim);
}

void PlayerActor::BeginWeaponChange()
{
	if (mState.weapon <= WP_NONE || mState.weapon > MAX_WEAPONS)
		return;

	if (!(mState.stats[STAT_WEAPONS] & (1 << mState.weapon)))
		return;

	if (mState.weaponState == WEAPON_DROPPING)
		return;

	mState.weaponState = WEAPON_DROPPING;
	mState.weaponTime += 200;
	StartTorsoAnim(TORSO_DROP);
	EventManager::Get()->TriggerEvent(
		eastl::make_shared<QuakeEventDataChangeWeapon>(GetId()));
}

void PlayerActor::FinishWeaponChange()
{
	int weapon = mAction.weaponSelect;
	if (weapon < WP_NONE || weapon > MAX_WEAPONS)
		weapon = WP_NONE;

	if (!(mState.stats[STAT_WEAPONS] & (1 << weapon)))
		weapon = WP_NONE;

	mState.weapon = weapon;
	mState.weaponState = WEAPON_RAISING;
	mState.weaponTime += 250;
	StartTorsoAnim(TORSO_RAISE);
	EventManager::Get()->TriggerEvent(
		eastl::make_shared<QuakeEventDataChangeWeapon>(GetId()));
}

bool PlayerActor::WeaponSelectable(int i)
{
	if (!mState.ammo[i])
		return false;

	if (!(mState.stats[STAT_WEAPONS] & (1 << i)))
		return false;

	return true;
}

void PlayerActor::NextWeapon()
{
	//mAction.weaponSelectTime = cg.time;
	int i;
	int original = mAction.weaponSelect;
	for (i=1; i <= MAX_WEAPONS; i++)
	{
		mAction.weaponSelect++;
		if (mAction.weaponSelect > MAX_WEAPONS)
			mAction.weaponSelect = 1;

		if (mAction.weaponSelect == WP_GAUNTLET)
			break;		// never cycle to gauntlet

		if (WeaponSelectable(mAction.weaponSelect))
			break;
	}
	if (i > MAX_WEAPONS)
		mAction.weaponSelect = original;
}

void PlayerActor::PreviousWeapon()
{
	//mAction.weaponSelectTime = cg.time;
	int i;
	int original = mAction.weaponSelect;
	for (i=1; i <= MAX_WEAPONS; i++)
	{
		mAction.weaponSelect--;
		if (mAction.weaponSelect == -1)
			mAction.weaponSelect = MAX_WEAPONS;

		if (mAction.weaponSelect == WP_GAUNTLET)
			break;		// never cycle to gauntlet

		if (WeaponSelectable(mAction.weaponSelect))
			break;
	}
	if (i > MAX_WEAPONS)
		mAction.weaponSelect = original;
}

void PlayerActor::Weapon()
{
	int num = 0;// = atoi(Argv(1));
	if (num < 1 || num > MAX_WEAPONS)
		return;

	//mAction.weaponSelectTime = cg.time;

	if (!(mState.stats[STAT_WEAPONS] & (1 << num)))
		return;		// don't have the weapon

	mAction.weaponSelect = num;
}

void PlayerActor::OutOfAmmoChange()
{
	//mAction.weaponSelectTime = cg.time;
	for (int i = MAX_WEAPONS; i > 0; i--)
	{
		if (WeaponSelectable(i))
		{
			mAction.weaponSelect = i;
			break;
		}
	}
}

void PlayerActor::PlayerSpawn()
{
	// clear everything
	int accuracyHits = mState.accuracyHits;
	int accuracyShots = mState.accuracyShots;
	int persistant[MAX_PERSISTANT];
	for (int i = 0; i < MAX_PERSISTANT; i++)
		persistant[i] = mState.persistant[i];

	memset(&mState, 0, sizeof(mState));
	memset(&mAction, 0, sizeof(mAction));

	//	client->areabits = savedAreaBits;
	mState.accuracyHits = accuracyHits;
	mState.accuracyShots = accuracyShots;

	for (int i = 0; i < MAX_PERSISTANT; i++)
		mState.persistant[i] = persistant[i];

	// increment the spawncount so the client will detect the respawn
	mState.persistant[PERS_SPAWN_COUNT]++;
	//mState.persistant[PERS_TEAM] = sessionTeam;
	mState.stats[STAT_MAX_HEALTH] = 100;
	mState.takeDamage = true;
	mState.viewHeight = DEFAULT_VIEWHEIGHT;
	mState.stats[STAT_WEAPONS] |= (1 << WP_MACHINEGUN);
	mState.ammo[WP_MACHINEGUN] = 100;
	mState.stats[STAT_WEAPONS] |= (1 << WP_GAUNTLET);

	// health will count down towards max_health
	mState.stats[STAT_HEALTH] = mState.stats[STAT_MAX_HEALTH] + 25;

	// force the base weapon up
	mAction.weaponSelect = WP_MACHINEGUN;
	mAction.weaponSelectTime = 0;

	mState.weapon = WP_MACHINEGUN;
	mState.weaponState = WEAPON_READY;

	// don't allow full run speed for a bit
	//mState.moveFlags |= PMF_TIME_KNOCKBACK;
	//mState.respawnTime = level.time;

	// set default animations
	mState.torsoAnim = TORSO_STAND;
	mState.legsAnim = LEGS_IDLE;

	// fire the targets of the spawn point
	EventManager::Get()->TriggerEvent(
		eastl::make_shared<QuakeEventDataSpawnActor>(GetId()));
}

void PlayerActor::UpdateWeapon(unsigned long deltaMs)
{
	int addTime;

	// don't allow attack until all buttons are up
	if (mAction.actionType & ACTION_RESPAWN)
		return;

	// ignore if spectator
	/*
	if (mState.persistant[PERS_TEAM] == TEAM_SPECTATOR)
		return;
	*/

	// check for dead player
	if (mState.stats[STAT_HEALTH] <= 0)
	{
		mState.weapon = WP_NONE;
		return;
	}

	// make weapon function
	if (mState.weaponTime > 0)
		mState.weaponTime -= deltaMs;

	// check for weapon change
	// can't change if weapon is firing, but can change
	// again if lowering or raising
	if (mState.weaponTime <= 0 ||
		mState.weaponState != WEAPON_FIRING)
	{
		if (mState.weapon != mAction.weaponSelect)
			BeginWeaponChange();
	}

	if (mState.weaponTime > 0)
		return;

	// change weapon if time
	if (mState.weaponState == WEAPON_DROPPING)
	{
		FinishWeaponChange();
		return;
	}

	if (mState.weaponState == WEAPON_RAISING)
	{
		mState.weaponState = WEAPON_READY;
		if (mState.weapon == WP_GAUNTLET)
			StartTorsoAnim(TORSO_STAND2);
		else
			StartTorsoAnim(TORSO_STAND);
		return;
	}

	// check for fire
	if (!(mAction.actionType & ACTION_ATTACK))
	{
		mState.weaponTime = 0;
		mState.weaponState = WEAPON_READY;
		return;
	}

	// start the animation even if out of ammo
	if (mState.weapon == WP_GAUNTLET)
	{
		// the guantlet only "fires" when it actually hits something
		/*
		if (!mState.gauntletHit)
		{
			mState.weaponTime = 0;
			mState.weaponState = WEAPON_READY;
			return;
		}
		*/
		StartTorsoAnim(TORSO_ATTACK2);
	}
	else
	{
		StartTorsoAnim(TORSO_ATTACK);
	}

	mState.weaponState = WEAPON_FIRING;

	// check for out of ammo
	if (!mState.ammo[mState.weapon])
	{
		OutOfAmmoChange();
		mState.weaponTime += 500;
		return;
	}

	// take an ammo away if not infinite
	if (mState.ammo[mState.weapon] != -1)
		mState.ammo[mState.weapon]--;

	// fire weapon
	EventManager::Get()->TriggerEvent(
		eastl::make_shared<QuakeEventDataFireWeapon>(GetId()));

	switch (mState.weapon)
	{
		case WP_GAUNTLET:
			addTime = 400;
			break;
		case WP_LIGHTNING:
			addTime = 50;
			break;
		case WP_SHOTGUN:
			addTime = 1000;
			break;
		case WP_MACHINEGUN:
			addTime = 100;
			break;
		case WP_GRENADE_LAUNCHER:
			addTime = 800;
			break;
		case WP_ROCKET_LAUNCHER:
			addTime = 800;
			break;
		case WP_PLASMAGUN:
			addTime = 100;
			break;
		case WP_RAILGUN:
			addTime = 1500;
			break;
		default:
			break;
	}

	mState.weaponTime += addTime;
}

void PlayerActor::UpdateMovement(const Vector3<float>& velocity)
{
	if (mState.weaponState == WEAPON_READY)
	{
		if (mState.weapon == WP_GAUNTLET)
			ContinueTorsoAnim(TORSO_STAND2);
		else
			ContinueTorsoAnim(TORSO_STAND);
	}

	if (mAction.actionType & ACTION_JUMP)
	{
		if (mAction.actionType & ACTION_MOVEBACK)
			ForceLegsAnim(LEGS_JUMPB);
		else
			ForceLegsAnim(LEGS_JUMP);

		EventManager::Get()->TriggerEvent(
			eastl::make_shared<QuakeEventDataJumpActor>(GetId(), velocity));
	}

	if (mAction.actionType & ACTION_RUN)
	{
		// if not trying to move
		if (!(mAction.actionType & ACTION_MOVEFORWARD) &&
			!(mAction.actionType & ACTION_MOVEBACK) &&
			!(mAction.actionType & ACTION_MOVELEFT) &&
			!(mAction.actionType & ACTION_MOVERIGHT))
		{
			ContinueLegsAnim(LEGS_IDLE);
		}
		else
		{
			if (mAction.actionType & ACTION_MOVEBACK)
				ContinueLegsAnim(LEGS_BACK);
			else
				ContinueLegsAnim(LEGS_RUN);
		}

		EventManager::Get()->TriggerEvent(
			eastl::make_shared<QuakeEventDataMoveActor>(GetId(), velocity));
	}
	else
	{
		EventManager::Get()->TriggerEvent(
			eastl::make_shared<QuakeEventDataFallActor>(GetId(), velocity));
	}
}

void PlayerActor::UpdateTimers(unsigned long deltaMs)
{
	// drop misc timing counter
	if (mState.moveTime)
	{
		if (deltaMs >= mState.moveTime)
			mState.moveTime = 0;
		else
			mState.moveTime -= deltaMs;
	}

	if (mState.jumpTime)
	{
		if (deltaMs >= mState.jumpTime)
			mState.jumpTime = 0;
		else
			mState.jumpTime -= deltaMs;
	}

	// drop animation counter
	if (mState.legsTimer > 0)
	{
		mState.legsTimer -= deltaMs;
		if (mState.legsTimer < 0)
			mState.legsTimer = 0;
	}

	if (mState.torsoTimer > 0)
	{
		mState.torsoTimer -= deltaMs;
		if (mState.torsoTimer < 0)
			mState.torsoTimer = 0;
	}
}

