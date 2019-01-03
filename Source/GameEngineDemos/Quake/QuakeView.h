//========================================================================
// QuakeView.h : source file for the sample game
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

#include "Core/Event/EventManager.h"

#include "Game/View/HumanView.h"

//! Macro for save Dropping an Element
#define DropElement(x)	if (x) { x->Remove(); x = 0; }

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


class QuakeStandardHUD : public BaseUI
{

public:
	QuakeStandardHUD();
	virtual ~QuakeStandardHUD();

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

class BaseEventManager;
class SoundProcess;
class QuakePlayerController;
class QuakeCameraController;
class Node;

class QuakeMainMenuView : public HumanView
{
protected:
	eastl::shared_ptr<QuakeMainMenuUI> mQuakeMainMenuUI; 
public:

	QuakeMainMenuView(); 
	~QuakeMainMenuView(); 
	virtual bool OnMsgProc( const Event& evt );
	virtual void RenderText();	
	virtual void OnUpdate(unsigned int timeMs, unsigned long deltaMs);
};


class QuakeHumanView : public HumanView
{
protected:
	bool  mShowUI;					// If true, it renders the UI control text
	DebugMode mDebugMode;
    eastl::string mGameplayText;

	eastl::shared_ptr<QuakePlayerController> mGamePlayerController;
	eastl::shared_ptr<QuakeCameraController> mGameCameraController;
	eastl::shared_ptr<Node> mPlayer;
	eastl::shared_ptr<QuakeStandardHUD> mQuakeStandardHUD;

public:
	QuakeHumanView();
	virtual ~QuakeHumanView();

	virtual bool OnMsgProc( const Event& event );	
	virtual void RenderText();	
	virtual void OnUpdate(unsigned int timeMs, unsigned long deltaMs);
	virtual void OnAttach(GameViewId vid, ActorId aid);

	virtual void SetControlledActor(ActorId actorId);
	virtual bool LoadGameDelegate(tinyxml2::XMLElement* pLevelData) override;

    // event delegates
    void GameplayUiUpdateDelegate(BaseEventDataPtr pEventData);
    void SetControlledActorDelegate(BaseEventDataPtr pEventData);
	void MoveActorDelegate(BaseEventDataPtr pEventData);

private:
    void RegisterAllDelegates(void);
    void RemoveAllDelegates(void);
};

class QuakeAIPlayerView : public BaseGameView 
{
	friend class QuakeAIPlayerViewListener;

private:
	eastl::shared_ptr<PathingGraph> mPathingGraph;

protected:
	GameViewId	mViewId;
	ActorId mPlayerActorId;

public:
	QuakeAIPlayerView(eastl::shared_ptr<PathingGraph> pPathingGraph);
	virtual ~QuakeAIPlayerView();

	virtual bool OnRestore() { return true; }
	virtual void OnRender(double time, float elapsedTime) {}
	virtual bool OnLostDevice() { return true; }
	virtual GameViewType GetType() { return GV_AI; }
	virtual GameViewId GetId() const { return mViewId; }
	virtual void OnAttach(GameViewId vid, ActorId actorId) { mViewId = vid; mPlayerActorId = actorId; }
	virtual bool OnMsgProc( const Event& event ) {	return false; }
	virtual void OnUpdate(unsigned int timeMs, unsigned long deltaMs) {}
	
	eastl::shared_ptr<PathingGraph> GetPathingGraph(void) const { return mPathingGraph; }
};

#endif