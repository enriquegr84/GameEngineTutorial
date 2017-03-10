//========================================================================
// HumanView.h : Defines the HumanView class of the GameEngine application
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

#ifndef HUMANVIEW_H
#define HUMANVIEW_H

#include "GameEngineStd.h"

#include "UI/UserInterface.h"

#include "Core/Process/ProcessManager.h"
#include "Core/Event/EventManager.h"

class ScreenElementScene;
class CameraSceneNode;
class BaseRenderer;

//
// class HumanView - Chapter 10, page 272
//
// This is a view onto the game - which includes anything visual or audible.
// It gets messages about the changes in game state.
//

class HumanView : public BaseGameView
{
	friend class GameEngineApp;

public:

	HumanView(const eastl::shared_ptr<BaseRenderer>& renderer);

	virtual ~HumanView();

	// Implement the BaseGameView interface, except for the VOnRender() method, which is renderer specific
	virtual bool OnRestore();
	virtual bool OnLostDevice();

	virtual void OnRender(double fTime, float fElapsedTime);
	virtual void OnUpdate(const int deltaMilliseconds );
	virtual void OnAnimate(unsigned int uTime);

	virtual GameViewType GetType() { return GameView_Human; }
	virtual GameViewId GetId() const { return m_ViewId; }

	virtual void OnAttach(GameViewId vid, ActorId aid)
	{
		m_ViewId = vid; 
		m_ActorId = aid;
	}
	virtual bool OnMsgProc( const Event& event );

	// Virtual methods to control the layering of interface elements
	virtual void PushElement(const eastl::shared_ptr<BaseScreenElement>& pElement);
	virtual void RemoveElement(const eastl::shared_ptr<BaseScreenElement>& pElement);

	void TogglePause(bool active);
    bool LoadGame(XMLElement* pLevelData);

	eastl::list<eastl::shared_ptr<BaseScreenElement>> m_ScreenElements; // a game screen entity

	// Audio
	bool InitAudio();
	ProcessManager* GetProcessManager() { return m_pProcessManager; }

	//Camera adjustments.
	//virtual void VSetActiveCameraOffset(const Vec4 & camOffset );

	// Added post press
	eastl::shared_ptr<ScreenElementScene> m_pScene;
	eastl::shared_ptr<CameraSceneNode> m_pCamera;

	void HandleGameState(BaseGameState newState);		

	// Added post press - this helps the network system attach views to the right actor.
	virtual void SetControlledActor(ActorId actorId) { m_ActorId = actorId; }

	// Event delegates
	void PlaySoundDelegate(BaseEventDataPtr pEventData);	
    void GameStateDelegate(BaseEventDataPtr pEventData);

public:
	// Class Console						- not described in the book
	//
	// This class offers a method for entering script commands at runtime.
	// Activate it by pressing the Tilde (~) key during runtime, and type
	// "exit" or "~" and pressing enter.
	class Console : public BaseUI
	{
	public:
		Console(void);

		~Console(void);

		virtual bool OnInit();
		virtual bool OnRestore() { return true; };
		virtual bool OnLostDevice() { return true; };

		virtual void OnUpdate(int deltaMilliseconds);
		virtual void OnAnimate(unsigned int uTime) { }

		//! draws all gui elements
		virtual bool OnRender(double fTime, float fElapsedTime);

		virtual bool OnEvent(const Event& evt) { return false; }
		virtual bool OnMsgProc(const Event& evt) { return false; }

		virtual int GetZOrder() const { return 1; }
		virtual void SetZOrder(int const zOrder) { }

		void AddDisplayText( const eastl::wstring & newText );
		void SetDisplayText( const eastl::wstring & newText );

		void SetActive( const bool bIsActive )  { m_bActive = bIsActive; }
		bool IsActive(void) const  { return m_bActive; }

		void HandleKeyboardInput( const unsigned int keyVal, 
			const unsigned int oemKeyVal, const bool bKeyDown );

	private:
		bool m_bActive;

		eastl::list<eastl::wstring> m_DisplayStrings;

		//eastl::wstring m_CurrentOutputString;	//What's the current output string?
		eastl::wstring m_CurrentInputString;	//What's the current input string?

		int m_ConsoleInputSize;	//Height of the input console window

		int m_CursorBlinkTimer;	//Countdown to toggle cursor blink state
		bool m_bCursorOn;	//Is the cursor currently displayed?

		bool m_bShiftDown;	//Is the shift button down?
		bool m_bCapsLockDown;	//Is the caps lock button down?

		//If this is true, we have a string to execute on our next update cycle.  
		//We have to do this because otherwise the user could interrupt in the midst
		//of an ::Update() call in another system.  This causes problems.
		bool m_bExecuteStringOnUpdate;
	};

	Console & GetConsole( void )
	{
		return m_Console;
	}

protected:

	GameViewId m_ViewId;
	ActorId m_ActorId;

	ProcessManager* m_pProcessManager;	// strictly for things like button animations, etc.
	bool m_runFullSpeed; // set to true if you want to run full speed

	BaseGameState m_BaseGameState;	// Added post-press - what is the current game state

	virtual void RenderText() { };

	virtual bool LoadGameDelegate(XMLElement* pLevelData);

	Console m_Console;

private:
	void RegisterAllDelegates(void);
	void RemoveAllDelegates(void);
};


#endif