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

#include "GameView.h"
#include "Game/GameLogic.h"

#include "Graphic/UI/UserInterface.h"
#include "Graphic/ScreenElement.h"

#include "Core/Process/ProcessManager.h"
#include "Core/Event/EventManager.h"

class ScreenElementScene;
class CameraNode;
class Renderer;
class System;


/*
	Class  HumanView. It is a view onto the game, which includes anything visual or audible.
	It is a platform-independent class which gets message of the game state or inputs
	and translate it into changes on the graphics system, which will draw the game
	world and the audio system
*/
class HumanView : public BaseGameView
{
	friend class GameApplication;

public:

	HumanView();

	virtual ~HumanView();

	// Implement the BaseGameView interface, except for the VOnRender() method, which is renderer specific
	virtual bool OnRestore();
	virtual bool OnLostDevice();

	virtual void OnRender(double time, float elapsedTime);
	virtual void OnUpdate(const int deltaMilliseconds );
	virtual void OnAnimate(unsigned int uTime);

	virtual GameViewType GetType() { return GV_HUMAN; }
	virtual GameViewId GetId() const { return mViewId; }

	virtual void OnAttach(GameViewId vid, ActorId aid)
	{
		mViewId = vid; 
		mActorId = aid;
	}
	virtual bool OnMsgProc( const Event& event );

	// Virtual methods to control the layering of interface elements
	virtual void PushElement(const eastl::shared_ptr<BaseScreenElement>& pElement);
	virtual void RemoveElement(const eastl::shared_ptr<BaseScreenElement>& pElement);

	void TogglePause(bool active);
    bool LoadGame(XMLElement* pLevelData);

	/*
		list of pointers to objects that implement the BaseScreenElement interface. A screen
		element is a strictly user interface thing and is a container for user interface controls 
		like buttons and textboxes
	*/
	eastl::list<eastl::shared_ptr<BaseScreenElement>> mScreenElements; // a game screen entity
																		
	// Interface sensitive objects
	/*
		mouse and keyboard handler interpret device messages into game commands
	*/
	eastl::shared_ptr<BaseMouseHandler> mMouseHandler;
	eastl::shared_ptr<BaseKeyboardHandler> mKeyboardHandler;

	// Initialize audio system
	bool InitAudio();
	ProcessManager* GetProcessManager() { return mProcessManager; }

	//Camera adjustments.
	//virtual void VSetActiveCameraOffset(const Vec4 & camOffset );

	// Added post press
	eastl::shared_ptr<ScreenElementScene> mScene;
	eastl::shared_ptr<CameraNode> mCamera;

	// Added post press - this helps the network system attach views to the right actor.
	virtual void SetControlledActor(ActorId actorId) { mActorId = actorId; }

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
		Console();

		~Console();

		virtual bool OnInit();
		virtual bool OnRestore() { return true; };
		virtual bool OnLostDevice() { return true; };

		virtual void OnUpdate(int deltaMilliseconds);
		virtual void OnAnimate(unsigned int uTime) { }

		//! draws all gui elements
		virtual bool OnRender(double time, float elapsedTime);

		virtual bool OnEvent(const Event& evt) { return false; }
		virtual bool OnMsgProc(const Event& evt) { return false; }

		virtual int GetZOrder() const { return 1; }
		virtual void SetZOrder(int const zOrder) { }

		void AddDisplayText( const eastl::wstring & newText );
		void SetDisplayText( const eastl::wstring & newText );

		void SetActive( const bool bIsActive )  { mIsActive = bIsActive; }
		bool IsActive(void) const  { return mIsActive; }

		void HandleKeyboardInput( const unsigned int keyVal, 
			const unsigned int oemKeyVal, const bool bKeyDown );

	private:
		bool mIsActive;

		eastl::list<eastl::wstring> mDisplayStrings;

		//eastl::wstring mCurrentOutputString;	//What's the current output string?
		eastl::wstring mCurrentInputString;	//What's the current input string?

		int mConsoleInputSize;	//Height of the input console window

		int mCursorBlinkTimer;	//Countdown to toggle cursor blink state
		bool mIsCursorOn;	//Is the cursor currently displayed?

		bool mIsShiftDown;	//Is the shift button down?
		bool mIsCapsLockDown;	//Is the caps lock button down?

		//If this is true, we have a string to execute on our next update cycle.  
		//We have to do this because otherwise the user could interrupt in the midst
		//of an ::Update() call in another system.  This causes problems.
		bool mIsExecuteStringOnUpdate;
	};

	Console & GetConsole( void )
	{
		return mConsole;
	}

protected:

	/*
		ViewId and ActorId makes easy for the game logic to determine if a view is attached
		to a particular actor in the game universe
	*/
	GameViewId mViewId;
	ActorId mActorId;

	/*
		ProcessManager is a convenient manager for anything that takes multiple game loops
		to accomplish, such as playing a sound effect or running an animation
	*/
	ProcessManager* mProcessManager;
	bool mRunFullSpeed; // set to true if you want to run full speed

	BaseGameState mGameState;	// Added post-press - what is the current game state
	
	/*
		This member, once overloaded in an inherited class, is what is called when text-specific
		elements need to be drawn by the view.
	*/
	virtual void RenderText() { };

	/*
		It is reponsible for creating view-specific elements from an XML file that defines all the
		elements in the game. It might include a background music track, which could be appreciated
		by the human playing but is inconsequential for the game logic
	*/
	virtual bool LoadGameDelegate(XMLElement* pLevelData);

	Console mConsole;

private:
	void RegisterAllDelegates(void);
	void RemoveAllDelegates(void);
};


#endif