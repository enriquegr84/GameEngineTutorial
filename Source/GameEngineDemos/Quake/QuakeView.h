//========================================================================
// QuakeView.h : Game View Class
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

#ifndef QUAKEVIEW_H
#define QUAKEVIEW_H

#include "QuakeStd.h"

#include "AI/Pathing.h"

#include "Core/Event/EventManager.h"

#include "Game/View/HumanView.h"

//! Macro for save Dropping an Element
#define DropElement(x)	if (x) { x->Remove(); x = 0; }

#define STAT_MINUS			10	// num frame for '-' stats digit

#define	ICON_SIZE			48
#define	CHAR_WIDTH			32
#define	CHAR_HEIGHT			48
#define	TEXT_ICON_SPACE		4

#define	NUM_CROSSHAIRS		10

enum FootStep 
{
	FOOTSTEP_NORMAL,
	FOOTSTEP_BOOT,
	FOOTSTEP_FLESH,
	FOOTSTEP_MECH,
	FOOTSTEP_ENERGY,
	FOOTSTEP_METAL,
	FOOTSTEP_SPLASH,

	FOOTSTEP_TOTAL
};

enum ImpactSound
{
	IMPACTSOUND_DEFAULT,
	IMPACTSOUND_METAL,
	IMPACTSOUND_FLESH
};

enum LeType
{
	LE_MARK,
	LE_EXPLOSION,
	LE_SPRITE_EXPLOSION,
	LE_FRAGMENT,
	LE_MOVE_SCALE_FADE,
	LE_FALL_SCALE_FADE,
	LE_FADE_RGB,
	LE_SCALE_FADE,
	LE_SCOREPLUM
};

enum LeFlag
{
	LEF_PUFF_DONT_SCALE = 0x0001,	// do not scale size over time
	LEF_TUMBLE = 0x0002,			// tumble over time, used for ejecting shells
	LEF_SOUND1 = 0x0004,			// sound 1 for kamikaze
	LEF_SOUND2 = 0x0008				// sound 2 for kamikaze
};

enum LeMarkType
{
	LEMT_NONE,
	LEMT_BURN,
	LEMT_BLOOD
};			// fragment local entities can leave marks on walls

enum LeBounceSoundType
{
	LEBS_NONE,
	LEBS_BLOOD,
	LEBS_BRASS
};	// fragment local entities can make sounds on impacts

// each weapon enum has an associated resource
// that contains media references necessary to present the
// weapon and its effects
struct WeaponResource 
{
	eastl::shared_ptr<MD3Mesh> handsModel;			// the hands don't actually draw, they just position the weapon
	eastl::shared_ptr<MD3Mesh> barrelModel;
	eastl::shared_ptr<MD3Mesh> flashModel;

	Vector3<float> weaponMidpoint;		// so it will rotate centered instead of by tag

	float flashDlight;
	eastl::wstring flashSound[4];		// fast firing weapons randomly choose
	eastl::array<float, 4U> flashDlightColor;

	eastl::wstring weaponIcon;
	eastl::wstring weaponModel;
	eastl::wstring ammoIcon;
	eastl::wstring ammoModel;

	eastl::wstring missileModel;
	eastl::wstring missileSound;

	float missileDlight;
	int missileRenderfx;
	eastl::array<float, 4U>	missileDlightColor;

	float trailRadius;
	float wiTrailTime;

	eastl::wstring readySound;
	eastl::wstring firingSound;

	bool loopFireSound;
};


// all of the model, shader, and sound references that are
// loaded at game load time are stored in Media
struct MediaResource
{
	eastl::wstring healthIcon;
	eastl::wstring armorIcon;

	eastl::wstring deferShader;

	// gib explosions
	eastl::wstring gibAbdomen;
	eastl::wstring gibArm;
	eastl::wstring gibChest;
	eastl::wstring gibFist;
	eastl::wstring gibFoot;
	eastl::wstring gibForearm;
	eastl::wstring gibIntestine;
	eastl::wstring gibLeg;
	eastl::wstring gibSkull;
	eastl::wstring gibBrain;

	eastl::wstring smoke2;

	eastl::wstring machinegunBrassModel;
	eastl::wstring shotgunBrassModel;

	eastl::wstring railRingsShader;
	eastl::wstring railCoreShader;

	eastl::wstring lightningShader;

	eastl::wstring balloonShader;
	eastl::wstring connectionShader;

	eastl::wstring selectShader;
	eastl::wstring viewBloodShader;
	eastl::wstring tracerShader;
	eastl::wstring crosshairShader[NUM_CROSSHAIRS];
	eastl::wstring lagometerShader;
	eastl::wstring backTileShader;
	eastl::wstring noammoShader;

	eastl::wstring smokePuffShader;
	eastl::wstring smokePuffRageProShader;
	eastl::wstring shotgunSmokePuffShader;
	eastl::wstring plasmaBallShader;
	eastl::wstring bloodTrailShader;

	eastl::wstring numberShaders[11];

	eastl::wstring shadowMarkShader;

	eastl::wstring botSkillShaders[5];

	// wall mark shaders
	eastl::wstring bloodMarkShader;
	eastl::wstring bulletMarkShader;
	eastl::wstring burnMarkShader;
	eastl::wstring holeMarkShader;
	eastl::wstring energyMarkShader;

	// weapon effect models
	eastl::wstring bulletFlashModel;
	eastl::wstring ringFlashModel;
	eastl::wstring dishFlashModel;
	eastl::wstring lightningExplosionModel;

	// weapon effect shaders
	eastl::wstring plasmaExplosionShader;
	eastl::wstring railExplosionShader[5];
	eastl::wstring bulletExplosionShader[5];
	eastl::wstring rocketExplosionShader[8];
	eastl::wstring grenadeExplosionShader[4];
	eastl::wstring bloodExplosionShader[5];

	// special effects models
	eastl::wstring teleportEffectModel;
	eastl::wstring teleportEffectShader;

	// scoreboard headers
	eastl::wstring scoreboardName;
	eastl::wstring scoreboardPing;
	eastl::wstring scoreboardScore;
	eastl::wstring scoreboardTime;

	// medals shown during gameplay
	eastl::wstring medalImpressive;
	eastl::wstring medalExcellent;
	eastl::wstring medalGauntlet;
	eastl::wstring medalDefend;
	eastl::wstring medalAssist;
	eastl::wstring medalCapture;

	// sounds
	eastl::wstring quadSound;
	eastl::wstring tracerSound;
	eastl::wstring selectSound;
	eastl::wstring useNothingSound;
	eastl::wstring wearOffSound;
	eastl::wstring footsteps[FOOTSTEP_TOTAL][4];
	eastl::wstring sfxLghit1;
	eastl::wstring sfxLghit2;
	eastl::wstring sfxLghit3;
	eastl::wstring sfxRic1;
	eastl::wstring sfxRic2;
	eastl::wstring sfxRic3;
	eastl::wstring sfxRailg;
	eastl::wstring sfxRockexp;
	eastl::wstring sfxPlasmaexp;

	eastl::wstring gibSound;
	eastl::wstring gibBounce1Sound;
	eastl::wstring gibBounce2Sound;
	eastl::wstring gibBounce3Sound;
	eastl::wstring teleInSound;
	eastl::wstring teleOutSound;
	eastl::wstring noAmmoSound;
	eastl::wstring respawnSound;
	eastl::wstring talkSound;
	eastl::wstring landSound;
	eastl::wstring fallSound;
	eastl::wstring jumpPadSound;

	//player sound
	eastl::wstring playerDeath1Sound;
	eastl::wstring playerDeath2Sound;
	eastl::wstring playerDeath3Sound;
	eastl::wstring playerDrownSound;
	eastl::wstring playerFallSound;
	eastl::wstring playerFallingSound;
	eastl::wstring playerGaspSound;
	eastl::wstring playerJumpSound;
	eastl::wstring playerLandSound;
	eastl::wstring playerPainSound[4];
	eastl::wstring playerTalkSound;
	eastl::wstring playerTauntSound;

	eastl::wstring oneMinuteSound;
	eastl::wstring fiveMinuteSound;
	eastl::wstring suddenDeathSound;

	eastl::wstring threeFragSound;
	eastl::wstring twoFragSound;
	eastl::wstring oneFragSound;

	eastl::wstring hitSound;
	eastl::wstring hitSoundHighArmor;
	eastl::wstring hitSoundLowArmor;
	eastl::wstring hitTeamSound;
	eastl::wstring impressiveSound;
	eastl::wstring excellentSound;
	eastl::wstring deniedSound;
	eastl::wstring humiliationSound;
	eastl::wstring assistSound;
	eastl::wstring defendSound;
	eastl::wstring firstImpressiveSound;
	eastl::wstring firstExcellentSound;
	eastl::wstring firstHumiliationSound;

	eastl::wstring weaponHoverSound;

	// tournament sounds
	eastl::wstring count3Sound;
	eastl::wstring count2Sound;
	eastl::wstring count1Sound;
	eastl::wstring countFightSound;
	eastl::wstring countPrepareSound;

	eastl::wstring cursor;
	eastl::wstring selectCursor;
	eastl::wstring sizeCursor;

	eastl::wstring nhealthSound;
	eastl::wstring hgrenb1aSound;
	eastl::wstring hgrenb2aSound;
	eastl::wstring wstbimplSound;
	eastl::wstring wstbimpmSound;
	eastl::wstring wstbimpdSound;
	eastl::wstring wstbactvSound;
};

class QuakeMainMenuUI : public BaseUI
{
protected:

	bool mCreatingGame;

	void Set();
	void SetUIActive(int command);

public:
	QuakeMainMenuUI();
	virtual ~QuakeMainMenuUI();

	// IScreenElement Implementation
	virtual bool OnInit();
	virtual bool OnRestore();
	virtual bool OnLostDevice() { return true; }

	virtual void OnUpdate(int deltaMilliseconds) { }

	//! draws all gui elements
	virtual bool OnRender(double time, float elapsedTime);

	virtual bool OnEvent(const Event& evt);
	virtual bool OnMsgProc(const Event& evt);

	virtual int GetZOrder() const { return 1; }
	virtual void SetZOrder(int const zOrder) { }
};

//forwarding
class BaseEventManager;
class SoundProcess;
class QuakeHumanView;
class QuakePlayerController;
class QuakeCameraController;
class PlayerActor;
class Node;

class QuakeStandardHUD : public BaseUI
{
public:
	QuakeStandardHUD(const eastl::shared_ptr<QuakeHumanView>& view);
	virtual ~QuakeStandardHUD();

	// IScreenElement Implementation
	virtual bool OnInit();
	virtual bool OnRestore();
	virtual bool OnLostDevice() { return true; }

	virtual void OnUpdate(unsigned int timeMs, unsigned long deltaMs);

	//! draws all gui elements
	virtual bool OnRender(double time, float elapsedTime);

	virtual bool OnEvent(const Event& evt);
	virtual bool OnMsgProc(const Event& evt);

	virtual int GetZOrder() const { return 1; }
	virtual void SetZOrder(int const zOrder) { }

protected:
	eastl::shared_ptr<QuakeHumanView> mGameView;

	eastl::shared_ptr<BaseUIImage> mCrosshair;
	eastl::shared_ptr<BaseUIImage> mAmmoIcon, mArmorIcon, mHealthIcon;
	eastl::vector<eastl::shared_ptr<BaseUIImage>> mAmmo, mArmor, mHealth;
	eastl::vector<eastl::shared_ptr<BaseUIStaticText>> mScore;

	void UpdatePickupItem();
	void UpdateScores();
	void UpdateStatusBar(const eastl::shared_ptr<PlayerActor>& player);
};

class QuakeMainMenuView : public HumanView
{

public:

	QuakeMainMenuView(); 
	~QuakeMainMenuView(); 
	virtual bool OnMsgProc( const Event& evt );
	virtual void RenderText();	
	virtual void OnUpdate(unsigned int timeMs, unsigned long deltaMs);

protected:
	eastl::shared_ptr<QuakeMainMenuUI> mQuakeMainMenuUI;
};


class QuakeHumanView : public HumanView, public eastl::enable_shared_from_this<QuakeHumanView>
{
	friend class QuakeStandardHUD;

public:
	QuakeHumanView();
	virtual ~QuakeHumanView();

	virtual bool OnMsgProc( const Event& event );	
	virtual void RenderText();	
	virtual void OnUpdate(unsigned int timeMs, unsigned long deltaMs);
	virtual void OnAttach(GameViewId vid, ActorId aid);

	virtual void SetControlledActor(ActorId actorId);
	virtual bool LoadGameDelegate(tinyxml2::XMLElement* pLevelData) override;

    void GameplayUiUpdateDelegate(BaseEventDataPtr pEventData);
    void SetControlledActorDelegate(BaseEventDataPtr pEventData);
	
	void FireWeaponDelegate(BaseEventDataPtr pEventData);
	void ChangeWeaponDelegate(BaseEventDataPtr pEventData);

	void DeadActorDelegate(BaseEventDataPtr pEventData);
	void SpawnActorDelegate(BaseEventDataPtr pEventData);
	void JumpActorDelegate(BaseEventDataPtr pEventData);
	void MoveActorDelegate(BaseEventDataPtr pEventData);

protected:

	bool  mShowUI;					// If true, it renders the UI control text
	DebugMode mDebugMode;
	eastl::string mGameplayText;

	// media
	MediaResource mMedia;
	WeaponResource mWeaponMedia[8];

	eastl::shared_ptr<QuakePlayerController> mGamePlayerController;
	eastl::shared_ptr<QuakeCameraController> mGameCameraController;

	eastl::shared_ptr<QuakeStandardHUD> mGameStandardHUD;
	eastl::shared_ptr<Node> mPlayer;

private:

	void RegisterSound();
	void RegisterMedia();
	void RegisterWeapon(unsigned int weapon);

    void RegisterAllDelegates(void);
    void RemoveAllDelegates(void);
};

#endif