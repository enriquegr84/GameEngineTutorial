//========================================================================
// Quake.h : source file for the sample game
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

#ifndef QUAKE_H
#define QUAKE_H

#include "Game/Game.h"

#include "Actors/PlayerActor.h"
#include "Actors/AmmoPickup.h"
#include "Actors/ItemPickup.h"
#include "Actors/ArmorPickup.h"
#include "Actors/HealthPickup.h"
#include "Actors/WeaponPickup.h"
#include "Actors/PushTrigger.h"
#include "Actors/TeleporterTrigger.h"
#include "Actors/LocationTarget.h"
#include "Actors/SpeakerTarget.h"
#include "Actors/GrenadeFire.h"
#include "Actors/PlasmaFire.h"
#include "Actors/RocketFire.h"

class BaseEventManager;
class NetworkEventForwarder;

#define	MAX_SPAWN_POINTS	128
#define	DEFAULT_GRAVITY		800
#define	GIB_HEALTH			-40
#define	ARMOR_PROTECTION	0.66

#define	GENTITYNUM_BITS		10		// don't need to send any more
#define	MAX_GENTITIES		(1<<GENTITYNUM_BITS)

// entitynums are communicated with GENTITY_BITS, so any reserved
// values that are going to be communcated over the net need to
// also be in this range
#define	ENTITYNUM_NONE		(MAX_GENTITIES-1)
#define	ENTITYNUM_WORLD		(MAX_GENTITIES-2)
#define	ENTITYNUM_MAX_NORMAL	(MAX_GENTITIES-2)

#define	MAX_MODELS			256		// these are sent over the net as 8 bits
#define	MAX_SOUNDS			256		// so they cannot be blindly increased

#define	CARNAGE_REWARD_TIME	3000
#define REWARD_SPRITE_TIME	2000

// contents flags are seperate bits
// a given brush can contribute multiple content bits

#define	CONTENTS_SOLID			1		// an eye is never valid in a solid
#define	CONTENTS_LAVA			8
#define	CONTENTS_SLIME			16
#define	CONTENTS_WATER			32
#define	CONTENTS_FOG			64

#define CONTENTS_NOTTEAM1		0x0080
#define CONTENTS_NOTTEAM2		0x0100
#define CONTENTS_NOBOTCLIP		0x0200

#define	CONTENTS_AREAPORTAL		0x8000

#define	CONTENTS_PLAYERCLIP		0x10000
#define	CONTENTS_MONSTERCLIP	0x20000
//bot specific contents types
#define	CONTENTS_TELEPORTER		0x40000
#define	CONTENTS_JUMPPAD		0x80000
#define CONTENTS_CLUSTERPORTAL	0x100000
#define CONTENTS_DONOTENTER		0x200000
#define CONTENTS_BOTCLIP		0x400000
#define CONTENTS_MOVER			0x800000

#define	CONTENTS_ORIGIN			0x1000000	// removed before bsping an entity

#define	CONTENTS_BODY			0x2000000	// should never be on a brush, only in game
#define	CONTENTS_CORPSE			0x4000000
#define	CONTENTS_DETAIL			0x8000000	// brushes not used for the bsp
#define	CONTENTS_STRUCTURAL		0x10000000	// brushes used for the bsp
#define	CONTENTS_TRANSLUCENT	0x20000000	// don't consume surface fragments inside
#define	CONTENTS_TRIGGER		0x40000000
#define	CONTENTS_NODROP			0x80000000	// don't leave bodies or items (death fog, lava)

// Entity Flgas
#define	EF_DEAD				0x00000001		// don't draw a foe marker over players with EF_DEAD
#define	EF_TELEPORT_BIT		0x00000004		// toggled every time the origin abruptly changes
#define	EF_AWARD_EXCELLENT	0x00000008		// draw an excellent sprite
#define EF_PLAYER_EVENT		0x00000010
#define	EF_BOUNCE			0x00000010		// for missiles
#define	EF_BOUNCE_HALF		0x00000020		// for missiles
#define	EF_AWARD_GAUNTLET	0x00000040		// draw a gauntlet sprite
#define	EF_NODRAW			0x00000080		// may have an event, but no model (unspawned items)
#define	EF_FIRING			0x00000100		// for lightning gun
#define	EF_KAMIKAZE			0x00000200
#define	EF_MOVER_STOP		0x00000400		// will push otherwise
#define EF_AWARD_CAP		0x00000800		// draw the capture sprite
#define	EF_TALK				0x00001000		// draw a talk balloon
#define	EF_CONNECTION		0x00002000		// draw a connection trouble sprite
#define	EF_VOTED			0x00004000		// already cast a vote
#define	EF_AWARD_IMPRESSIVE	0x00008000		// draw an impressive sprite
#define	EF_AWARD_DEFEND		0x00010000		// draw a defend sprite
#define	EF_AWARD_ASSIST		0x00020000		// draw a assist sprite
#define EF_AWARD_DENIED		0x00040000		// denied
#define EF_TEAMVOTED		0x00080000		// already cast a team vote

#define	LIGHTNING_RANGE		768

#define MACHINEGUN_SPREAD	200
#define	MACHINEGUN_DAMAGE	4
#define	MACHINEGUN_TEAM_DAMAGE	5		// wimpier MG in teamplay

#define DEFAULT_SHOTGUN_SPREAD	700
#define DEFAULT_SHOTGUN_COUNT	11

enum ItemType 
{
	IT_WEAPON,				// EFX: rotate + upscale + minlight
	IT_AMMO,				// EFX: rotate
	IT_ARMOR,				// EFX: rotate + minlight
	IT_HEALTH,				// EFX: static external sphere + rotating internal
	IT_POWERUP,				// instant on, timer based
							// EFX: rotate + external ring that rotates
	IT_HOLDABLE,			// single use, holdable item
							// EFX: rotate + bob
	IT_COUNT
};


enum PowerupType 
{
	PW_NONE,

	PW_QUAD,
	PW_BATTLESUIT,
	PW_HASTE,
	PW_INVIS,
	PW_REGEN,
	PW_FLIGHT,

	PW_REDFLAG,
	PW_BLUEFLAG,
	PW_NEUTRALFLAG,

	PW_SCOUT,
	PW_GUARD,
	PW_DOUBLER,
	PW_AMMOREGEN,
	PW_INVULNERABILITY,

	PW_NUM_POWERUPS
};

enum EntityType
{
	ET_GENERAL,
	ET_PLAYER,
	ET_ITEM,
	ET_MISSILE,
	ET_MOVER,
	ET_BEAM,
	ET_PORTAL,
	ET_SPEAKER,
	ET_PUSH_TRIGGER,
	ET_TELEPORT_TRIGGER,
	ET_INVISIBLE,
	ET_GRAPPLE,				// grapple hooked on wall
	ET_TEAM,

	ET_EVENTS				// any of the EV_* events can be added freestanding
							// by setting eType to ET_EVENTS + eventNum
							// this avoids having to set eFlags and eventNum
};

//---------------------------------------------------------------------------------------------------------------------
// QuakeLogic class                        - Chapter 21, page 723
//---------------------------------------------------------------------------------------------------------------------
class QuakeLogic : public GameLogic
{
	friend class QuakeAIManager;

public:
	QuakeLogic();
	virtual ~QuakeLogic();

	// Update
	virtual void SetProxy();

	virtual void SyncActor(const ActorId id, Transform const &transform);

	virtual void ResetViewType();
	virtual void UpdateViewType(const eastl::shared_ptr<BaseGameView>& pView, bool add);

	// Overloads
	virtual void ChangeState(BaseGameState newState);
	virtual eastl::shared_ptr<BaseGamePhysic> GetGamePhysics(void) { return mPhysics; }

	// Quake Actors
	eastl::shared_ptr<Actor> GetRandomActor();
	void GetAmmoActors(eastl::vector<eastl::shared_ptr<Actor>>& ammo);
	void GetWeaponActors(eastl::vector<eastl::shared_ptr<Actor>>& weapon);
	void GetHealthActors(eastl::vector<eastl::shared_ptr<Actor>>& health);
	void GetArmorActors(eastl::vector<eastl::shared_ptr<Actor>>& armor);
	void GetTriggerActors(eastl::vector<eastl::shared_ptr<Actor>>& trigger);
	void GetTargetActors(eastl::vector<eastl::shared_ptr<Actor>>& target);

	//Quake Spawn Points
	const eastl::shared_ptr<Actor>& SelectRandomSpawnPoint();
	const eastl::shared_ptr<Actor>& SelectNearestSpawnPoint(const Vector3<float>& from);
	void SelectSpawnPoint(const Vector3<float>& avoidPoint, Transform& transform);
	void SelectRandomFurthestSpawnPoint(const Vector3<float>& avoidPoint, Transform& transform);
	void SelectInitialSpawnPoint(Transform& transform);

	// event delegates
	void RequestStartGameDelegate(BaseEventDataPtr pEventData);
	void RemoteClientDelegate(BaseEventDataPtr pEventData);
	void NetworkPlayerActorAssignmentDelegate(BaseEventDataPtr pEventData);
	void PhysicsTriggerEnterDelegate(BaseEventDataPtr pEventData);
	void PhysicsTriggerLeaveDelegate(BaseEventDataPtr pEventData);
	void PhysicsCollisionDelegate(BaseEventDataPtr pEventData);
	void PhysicsSeparationDelegate(BaseEventDataPtr pEventData);
	void EnvironmentLoadedDelegate(BaseEventDataPtr pEventData);

	void SplashDamageDelegate(BaseEventDataPtr pEventData);
	void FireWeaponDelegate(BaseEventDataPtr pEventData);
	void SpawnActorDelegate(BaseEventDataPtr pEventData);
	void PushActorDelegate(BaseEventDataPtr pEventData);
	void JumpActorDelegate(BaseEventDataPtr pEventData);
	void MoveActorDelegate(BaseEventDataPtr pEventData);
	void FallActorDelegate(BaseEventDataPtr pEventData);
	void RotateActorDelegate(BaseEventDataPtr pEventData);
	void StartThrustDelegate(BaseEventDataPtr pEventData);
	void EndThrustDelegate(BaseEventDataPtr pEventData);
	void StartSteerDelegate(BaseEventDataPtr pEventData);
	void EndSteerDelegate(BaseEventDataPtr pEventData);

protected:

	// event registers
	void RegisterAllDelegates(void);
	void RemoveAllDelegates(void);
	void CreateNetworkEventForwarder(const int socketId);
	void DestroyAllNetworkEventForwarders(void);

	eastl::list<NetworkEventForwarder*> mNetworkEventForwarders;

	eastl::shared_ptr<PlayerActor> CreatePlayerActor(const eastl::string &actorResource,
		tinyxml2::XMLElement *overrides, const Transform *initialTransform = NULL,
		const ActorId serversActorId = INVALID_ACTOR_ID);

	virtual ActorFactory* CreateActorFactory(void);
	virtual LevelManager* CreateLevelManager(void);
	virtual AIManager* CreateAIManager(void);

	virtual bool LoadGameDelegate(tinyxml2::XMLElement* pLevelData);

private:

	bool SpotTelefrag(const eastl::shared_ptr<Actor>& spot);

	bool RadiusDamage(float damage, float radius, int mod,
		Vector3<float> origin, const eastl::shared_ptr<PlayerActor>& attacker);

	void GauntletAttack(const eastl::shared_ptr<PlayerActor>& player,
		const Vector3<float>& origin, const Vector3<float>& forward);
	void BulletFire(const eastl::shared_ptr<PlayerActor>& player,
		const Vector3<float>& muzzle, const Vector3<float>& forward,
		const Vector3<float>& right, const Vector3<float>& up, 
		float spread, int damage);
	void ShotgunFire(const eastl::shared_ptr<PlayerActor>& player,
		const Vector3<float>& muzzle, const Vector3<float>& forward,
		const Vector3<float>& right, const Vector3<float>& up);
	bool ShotgunPellet(const eastl::shared_ptr<PlayerActor>& player,
		const Vector3<float>& forward, const Vector3<float>& start, const Vector3<float>& end);
	void GrenadeLauncherFire(
		const eastl::shared_ptr<PlayerActor>& player, const Vector3<float>& muzzle, 
		const Vector3<float>& forward, const EulerAngles<float>& angles);
	void RocketLauncherFire(
		const eastl::shared_ptr<PlayerActor>& player, const Vector3<float>& muzzle,
		const Vector3<float>& forward, const EulerAngles<float>& angles);
	void PlasmagunFire(
		const eastl::shared_ptr<PlayerActor>& player, const Vector3<float>& muzzle, 
		const Vector3<float>& forward, const EulerAngles<float>& angles);
	void RailgunFire(const eastl::shared_ptr<PlayerActor>& player, 
		const Vector3<float>& muzzle, const Vector3<float>& forward);
	void LightningFire(const eastl::shared_ptr<PlayerActor>& player, 
		const Vector3<float>& muzzle, const Vector3<float>& forward);
};

#endif