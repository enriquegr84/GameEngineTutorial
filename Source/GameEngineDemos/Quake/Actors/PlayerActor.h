//========================================================================
// Actor.h - Defines the Actor class
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

#ifndef PLAYERACTOR_H
#define PLAYERACTOR_H

#include "Quake/QuakeStd.h"

#include "Game/Actor/Actor.h"

// bit field limits
#define	MAX_STATS				16
#define	MAX_PERSISTANT			16
#define	MAX_POWERUPS			16
#define	MAX_WEAPONS				16

enum PlayerMoveType
{
	PM_NORMAL,		// can accelerate and turn
	PM_NOCLIP,		// noclip movement
	PM_SPECTATOR,	// still run into walls
	PM_DEAD,		// no acceleration or turning, but free falling
	PM_FREEZE,		// stuck in place with no control
	PM_INTERMISSION,	// no movement or status bar
	PM_SPINTERMISSION	// no movement or status bar
};

enum WeaponState
{
	WEAPON_READY,
	WEAPON_RAISING,
	WEAPON_DROPPING,
	WEAPON_FIRING
};

enum StatType
{
	STAT_HEALTH,
	STAT_HOLDABLE_ITEM,
	STAT_WEAPONS,					// 16 bit fields
	STAT_ARMOR,
	STAT_DEAD_YAW,					// look this direction when dead (FIXME: get rid of?)
	STAT_CLIENTS_READY,				// bit mask of clients wishing to exit the intermission (FIXME: configstring?)
	STAT_MAX_HEALTH					// health / armor limit, changable by handicap
};

enum PersistantType
{
	PERS_SCORE,						// !!! MUST NOT CHANGE, SERVER AND GAME BOTH REFERENCE !!!
	PERS_HITS,						// total points damage inflicted so damage beeps can sound on change
	PERS_RANK,						// player rank or team rank
	PERS_TEAM,						// player team
	PERS_SPAWN_COUNT,				// incremented every respawn
	PERS_PLAYEREVENTS,				// 16 bits that can be flipped for events
	PERS_ATTACKER,					// clientnum of last damage inflicter
	PERS_ATTACKEE_ARMOR,			// health/armor of last person we attacked
	PERS_KILLED,					// count of the number of times you died
									// player awards tracking
	PERS_IMPRESSIVE_COUNT,			// two railgun hits in a row
	PERS_EXCELLENT_COUNT,			// two successive kills in a short amount of time
	PERS_DEFEND_COUNT,				// defend awards
	PERS_ASSIST_COUNT,				// assist awards
	PERS_GAUNTLET_FRAG_COUNT,		// kills with the guantlet
	PERS_CAPTURES					// captures
};


// PlayerState is the information needed to predict player state
struct PlayerState
{
	int moveType;

	// damage feedback
	int damageEvent;	// when it changes, latch the other parms
	int damageYaw;
	int damagePitch;
	int damageCount;

	int stats[MAX_STATS];
	int persistant[MAX_PERSISTANT];	// stats that aren't cleared on death
	int powerups[MAX_POWERUPS];	// level.time that the powerup runs out
	int ammo[MAX_WEAPONS];
};


/*
	Player Actor class. Quake player actor is a character (NPC or human) in your game world.
	Its entiry purpose is to manage characters data and and mantain components to make them 
	as flexible and reusable as possible.
*/
class PlayerActor : public Actor
{

private:
	PlayerState mState;

public:
    explicit PlayerActor(ActorId id);
    virtual ~PlayerActor(void);

	PlayerState GetState() { return mState; }
};

#endif