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

#include "Mathematic/Algebra/Vector3.h"

// bit field limits
#define	MAX_STATS				16
#define	MAX_PERSISTANT			16
#define	MAX_POWERUPS			16
#define	MAX_WEAPONS				8

#define	DEFAULT_VIEWHEIGHT	26
#define CROUCH_VIEWHEIGHT	12
#define	DEAD_VIEWHEIGHT		-16

// reward sounds (stored in ps->persistant[PERS_PLAYEREVENTS])
#define	PLAYEREVENT_DENIEDREWARD		0x0001
#define	PLAYEREVENT_GAUNTLETREWARD		0x0002
#define PLAYEREVENT_HOLYSHIT			0x0004

// player flags
#define	FL_GODMODE				0x00000010
#define	FL_NOTARGET				0x00000020
#define	FL_TEAMSLAVE			0x00000400	// not the first on the team
#define FL_NO_KNOCKBACK			0x00000800
#define FL_DROPPED_ITEM			0x00001000
#define FL_NO_BOTS				0x00002000	// spawn point not for bot use
#define FL_NO_HUMANS			0x00004000	// spawn point just for bots
#define FL_FORCE_GESTURE		0x00008000	// force gesture on client

// damage flags
#define DAMAGE_RADIUS				0x00000001	// damage was indirect
#define DAMAGE_NO_ARMOR				0x00000002	// armour does not protect from this damage
#define DAMAGE_NO_KNOCKBACK			0x00000004	// do not affect velocity, just view angles
#define DAMAGE_NO_PROTECTION		0x00000008  // armor, shields, invulnerability, and godmode have no effect

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


enum WeaponType
{
	WP_NONE,

	WP_SHOTGUN,
	WP_ROCKET_LAUNCHER,
	WP_RAILGUN,
	WP_PLASMAGUN,
	WP_MACHINEGUN,
	WP_LIGHTNING,
	WP_GRENADE_LAUNCHER,
	WP_GAUNTLET
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

// means of death
enum MeansOfDeath
{
	MOD_UNKNOWN,
	MOD_SHOTGUN,
	MOD_GAUNTLET,
	MOD_MACHINEGUN,
	MOD_GRENADE,
	MOD_GRENADE_SPLASH,
	MOD_ROCKET,
	MOD_ROCKET_SPLASH,
	MOD_PLASMA,
	MOD_PLASMA_SPLASH,
	MOD_RAILGUN,
	MOD_LIGHTNING,
	MOD_BFG,
	MOD_BFG_SPLASH,
	MOD_WATER,
	MOD_SLIME,
	MOD_LAVA,
	MOD_CRUSH,
	MOD_TELEFRAG,
	MOD_FALLING,
	MOD_SUICIDE,
	MOD_TARGET_LASER,
	MOD_TRIGGER_HURT,
	MOD_GRAPPLE
};

// This enumeration stores all the animations in order from the config file (.cfg).
enum AnimationType
{
	// If one model is set to one of the BOTH_* animations, the other one should be too,
	// otherwise it looks really bad and confusing.

	BOTH_DEATH1 = 0,		// The first twirling death animation
	BOTH_DEAD1,				// The end of the first twirling death animation
	BOTH_DEATH2,			// The second twirling death animation
	BOTH_DEAD2,				// The end of the second twirling death animation
	BOTH_DEATH3,			// The back flip death animation
	BOTH_DEAD3,				// The end of the back flip death animation

	// The next block is the animations that the upper body performs

	TORSO_GESTURE,			// The torso's gesturing animation
	
	TORSO_ATTACK,			// The torso's attack1 animation
	TORSO_ATTACK2,			// The torso's attack2 animation

	TORSO_DROP,				// The torso's weapon drop animation
	TORSO_RAISE,			// The torso's weapon pickup animation

	TORSO_STAND,			// The torso's idle stand animation
	TORSO_STAND2,			// The torso's idle stand2 animation

	// The final block is the animations that the legs perform

	LEGS_WALKCR = 6,		// The legs's crouching walk animation
	LEGS_WALK,				// The legs's walk animation
	LEGS_RUN,				// The legs's run animation
	LEGS_BACK,				// The legs's running backwards animation
	LEGS_SWIM,				// The legs's swimming animation
	
	LEGS_JUMP,				// The legs's jumping animation
	LEGS_LAND,				// The legs's landing animation

	LEGS_JUMPB,				// The legs's jumping back animation
	LEGS_LANDB,				// The legs's landing back animation

	LEGS_IDLE,				// The legs's idle stand animation
	LEGS_IDLECR,			// The legs's idle crouching animation

	LEGS_TURN,				// The legs's turn animation

	//! Not an animation, but amount of animation types.
	ANIMATION_COUNT = 25
};

enum ActionType
{
	ACTION_ATTACK = 0x0000001,
	ACTION_RESPAWN = 0x0000002,
	ACTION_CROUCH = 0x0000008,
	ACTION_JUMP = 0x0000010,
	ACTION_RUN = 0x0000020,
	ACTION_MOVEFORWARD = 0x0000080,
	ACTION_MOVEBACK = 0x0000100,
	ACTION_MOVELEFT = 0x0000200,
	ACTION_MOVERIGHT = 0x0000800,
	ACTION_FALLEN = 0x0001000,
	ACTION_KNOCKBACK = 0x0002000
};

// PlayerAction is the information from the character controller
struct PlayerAction
{
	int actionType;

	int weaponSelectTime;
	int weaponSelect;
};


// PlayerState is the information from its current state
struct PlayerState
{
	int contents;

	bool takeDamage;

	int weapon;
	int weaponState;
	int weaponTime;

	int viewHeight;

	int accurateCount;		// for "impressive" reward sound

	int accuracyShots;		// total number of shots
	int accuracyHits;		// total number of hits

	int rewardTime;
	int respawnTime;		// can respawn when time > this, force after forcerespwan
	int lastKillTime;

	int lastKilled;	// last client that this client killed
	int lastHurt;	// last client that damaged this client
	int lastHurtMod; // type of damage the client did

	int	eType;			// entity type
	int	eFlags;			// entity flags

	int moveType;
	int moveTime;

	//animation
	int legsTimer;		// don't change low priority animations until this runs out
	int legsAnim;		// mask off ANIM_TOGGLEBIT

	int torsoTimer;		// don't change low priority animations until this runs out
	int torsoAnim;		// mask off ANIM_TOGGLEBIT

	// damage feedback
	int damageEvent;	// when it changes, latch the other parms
	int damageYaw;
	int damagePitch;
	int damageCount;

	// sum up damage over an entire frame, so
	// shotgun blasts give a single big kick
	int damageArmor;				// damage absorbed by armor
	int damageBlood;				// damage taken out of health
	int damageKnockback;			// impact damage
	Vector3<float> damageFrom;		// origin for vector calculation
	bool damageFromWorld;			// if true, don't use the damage_from vector

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

public:
    explicit PlayerActor(ActorId id);
    virtual ~PlayerActor(void);

	PlayerState& GetState() { return mState; }
	PlayerAction& GetAction() { return mAction; }

	void NextWeapon();
	void PrevWeapon();

	void UpdateTimers(unsigned long deltaMs);
	void UpdateWeapon(unsigned long deltaMs);
	void UpdateMovement(const Vector3<float>& velocity);

private:

	PlayerState mState;
	PlayerAction mAction;

	void StartLegsAnim(int anim);
	void StartTorsoAnim(int anim);
	void ContinueLegsAnim(int anim);
	void ContinueTorsoAnim(int anim);
	void ForceLegsAnim(int anim);

	void FinishWeaponChange();
	void BeginWeaponChange();

	void OutOfAmmoChange();
	bool WeaponSelectable(int i);
	void Weapon();
};

#endif