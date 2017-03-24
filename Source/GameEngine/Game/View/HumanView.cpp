//========================================================================
// HumanView.cpp - Implements the class HumanView, which provides a Human interface into the game
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

#include "HumanView.h"

#include "Game/Actor/AudioComponent.h"
#include "Game/Actor/RenderComponentInterface.h"

//events related
#include "Audio/DirectSoundAudio.h"
#include "Audio/SoundProcess.h"

#include "Application/GameApplication.h"
#include "Application/System/System.h"

#include "Graphic/Scene/Scene.h"
#include "Graphic/Renderer/Renderer.h"

#include "Core/Event/Event.h"
#include "Core/Event/EventManager.h"
#include "Core/Process/Process.h"

const unsigned int SCREEN_REFRESH_RATE(1000/60);
const GameViewId InvalidGameViewId = 0xffffffff;

template<class T>
struct SortBy_SharedPtr_Content
{
	bool operator()(const eastl::shared_ptr<T> &lhs, const eastl::shared_ptr<T> &rhs) const
	{
		return *lhs < *rhs;
	}
};

//
// HumanView::HumanView - Chapter 10, page 272
//
HumanView::HumanView()
{
	InitAudio(); 

	m_pProcessManager = new ProcessManager;

	m_runFullSpeed = true;
	m_ViewId = InvalidGameViewId;

	// Added post press for move, new, and destroy actor events and others
	RegisterAllDelegates();
	m_BaseGameState = BGS_Initializing;		// what is the current game state

	GameApplication* gameApp = (GameApplication*)Application::App;
	if (gameApp->mRenderer)
	{
		// Moved to the HumanView class post press
		m_pScene.reset(new ScreenElementScene());
		/*
		m_pCamera.reset(new CameraSceneNode(
			gameApp->mGame->GetNewActorID(), m_pScene.get(), Matrix4x4<float>::Identity));
		LogAssert(m_pScene && m_pCamera, "Out of memory");
		m_pCamera->SetFarValue(20000.f); // this increase a shadow visible range.

		m_pScene->AddChild(m_pCamera->Get()->GetId(), m_pCamera);
		m_pScene->SetActiveCamera(m_pCamera);
		*/
	}
}


HumanView::~HumanView()
{
	// [mrmike] - RemoveAllDelegates was added post press to handle move, new, and destroy actor events.
	RemoveAllDelegates();

	while (!m_ScreenElements.empty())
		m_ScreenElements.pop_front();

	delete m_pProcessManager;
	delete g_pAudio;
}


bool HumanView::LoadGameDelegate(XMLElement* pLevelData) 
{ 
	PushElement(m_pScene);  
	return true; 
}

bool HumanView::LoadGame(XMLElement* pLevelData)
{
    // call the delegate method
    return LoadGameDelegate(pLevelData);
}

/*
	OnRender method is responsible for rendering the view at either a clamped maximum refresh rate 
	or at full speed, depending on the value of the local variables. If the view is ready to draw,
	it calls the application renderer's PreRender() method. 
	The loop iterates through the screen layers and if it is visible, it calls its own render method.
	This implies that the only thing the view really draws for itself is the following RenderText().
	Everything else should be drawn because it belongs to the list of screens.
	Next it is rendered any text applied directly to the screen.
	PostRender() method finalizes the render process and presents the screen to the viewer
*/
void HumanView::OnRender(double fTime, float fElapsedTime )
{
	int deltaMilliseconds = int(fElapsedTime * 1000.0f);

	// It is time to draw ?
	GameApplication* gameApp = (GameApplication*)Application::App;
	//if (gameApp->mRenderer->PreRender())
	{
		if( m_runFullSpeed || ( deltaMilliseconds > SCREEN_REFRESH_RATE) )
		{
			m_ScreenElements.sort(SortBy_SharedPtr_Content<BaseScreenElement>());

			for(eastl::list<eastl::shared_ptr<BaseScreenElement>>::iterator it =
				m_ScreenElements.begin(); it!=m_ScreenElements.end(); ++it)
			{
				if ((*it)->IsVisible())
				{
					(*it)->OnRender(fTime, fElapsedTime);
				}
			}

            RenderText();

			// Let the console render.
			m_Console.OnRender(fTime, fElapsedTime);

			//gameApp->mGame->RenderDiagnostics();
		}
    }
	//gameApp->mRenderer->PostRender();
}

/*
	OnRestore method is responsible for recreating anything that might be lost while the game
	is running. It typically happens as a result of the operating system responding to something
	application wide, such as restoring the application from a sleep mode or changing the screen
	resolution while the game is running. OnRestore method gets called just after the class is
	instantiated, so this method is just as useful for initialization as it is for restoring lost
	objects. These objects include all of the attached screens
*/
bool HumanView::OnRestore()
{
	bool hr = true;
	for(eastl::list<eastl::shared_ptr<BaseScreenElement>>::iterator it =
		m_ScreenElements.begin(); it != m_ScreenElements.end(); ++it)
	{
		return ( (*it)->OnRestore() );
	}

	return hr;
}

/*
	OnLostDevice will be called prior to OnRestore(), so it is used to chain the "on lost device"
	event to other objects or simply release the objects so they will be re-created in the call
	OnRestore(). Recursively calls OnLostDevice for everything attached to the HumanView. 
*/
bool HumanView::OnLostDevice() 
{
	for(eastl::list<eastl::shared_ptr<BaseScreenElement>>::iterator it =
		m_ScreenElements.begin(); it!=m_ScreenElements.end(); ++it)
	{
		return ( (*it)->OnLostDevice() );
	}
	return true;
}



//
// HumanView::InitAudio							- Chapter X, page Y
//
bool HumanView::InitAudio()
{
	if (!g_pAudio)
	{
		g_pAudio = new DirectSoundAudio();		// use this line for DirectSound
	}

	if (!g_pAudio)
		return false;

	GameApplication* gameApp = (GameApplication*)Application::App;
	if (!g_pAudio->Initialize(gameApp->mSystem->GetID()))
		return false;

	return true;
}



void HumanView::TogglePause(bool active)
{
	// Pause or resume audio	
	if ( active )
	{
		if (g_pAudio) 
			g_pAudio->PauseAllSounds();
	}
	else
	{
		if (g_pAudio)
			g_pAudio->ResumeAllSounds();
	}
}

/*
	HumanView do process device messages from the application layer. Any conceivable message that
	the game views would want to see should be translated into the generic message form and passed
	on to all the game views. If the game view returns true it means that it has completely consumed
	the message, and no other view should see it. The HumanView class can contain multiple screens,
	but instead of being layered, they will sit side by side. The HumanView class will still grab
	input from all the devices and translate it into game commands, just as you are about to see, but
	in this case, each device will be treated as input for a different player.
	The OnMsgProc iterates through the list of screens attached to it, forward the message on the
	visible ones, and if they don't eat the message, then ask the handlers if they can consume it.
	We can hook own device input device implementation by using the handlers. The existence of the
	handler is always checked before the message is sent to it.
	It is used a reverse iterator for the screens because otherwise the screen on top is going to be
	the last one drawn. User input should always be processed in order of the screens from top to 
	bottom, or reverse order.
*/
bool HumanView::OnMsgProc( const Event& evt )
{
	// Iterate through the screen layers first
	// In reverse order since we'll send input messages to the 
	// screen on top
	for(eastl::list<eastl::shared_ptr<BaseScreenElement>>::reverse_iterator it =
		m_ScreenElements.rbegin(); it!=m_ScreenElements.rend(); ++it)
	{
		if ( (*it)->IsVisible() )
		{
			if ( (*it)->OnMsgProc( evt ) )
			{
				return true;
			}
		}
	}

	bool result = 0;
	switch (evt.mEventType) 
	{
		case ET_KEY_INPUT_EVENT:
			if (m_Console.IsActive())
			{	
				//See if it was the console key.
				if (L'º' == evt.mKeyInput.mChar)
				{
					m_Console.SetActive(false);
				}
				else
				{
					const unsigned int oemScan = 
						int( evt.mKeyInput.mChar & ( 0xff << 16 ) ) >> 16;
					m_Console.HandleKeyboardInput( 
						evt.mKeyInput.mChar, 
						MapVirtualKey( oemScan, 1 ), 
						evt.mKeyInput.mPressedDown );
				}
			}
			else
			{
				//See if it was the console key.
				if (L'º' == evt.mKeyInput.mChar)
				{
					m_Console.SetActive(true);
				}
				else if (evt.mKeyInput.mPressedDown)
				{
					if (mKeyboardHandler)
						result = mKeyboardHandler->OnKeyDown(evt.mKeyInput.mKey);
				}
				else
				{
					if (mKeyboardHandler)
						result = mKeyboardHandler->OnKeyUp(evt.mKeyInput.mKey);
				}
			}
			break;
		case ET_MOUSE_INPUT_EVENT:
			if (mMouseHandler)
			{
				switch (evt.mMouseInput.mEvent)
				{
					case MIE_MOUSE_MOVED:
						result = mMouseHandler->OnMouseMove(
							Vector2<int>{evt.mMouseInput.X, evt.mMouseInput.Y}, 1);
						break;
					case MIE_LMOUSE_PRESSED_DOWN:
						result = mMouseHandler->OnMouseButtonDown(
							Vector2<int>{evt.mMouseInput.X, evt.mMouseInput.Y}, 1, "PointerLeft");
						break;
					case MIE_LMOUSE_LEFT_UP:
						result = mMouseHandler->OnMouseButtonUp(
							Vector2<int>{evt.mMouseInput.X, evt.mMouseInput.Y}, 1, "PointerLeft");
						break;
					case MIE_RMOUSE_PRESSED_DOWN:
						result = mMouseHandler->OnMouseButtonDown(
							Vector2<int>{evt.mMouseInput.X, evt.mMouseInput.Y}, 1, "PointerRight");
						break;
					case MIE_RMOUSE_LEFT_UP:
						result = mMouseHandler->OnMouseButtonUp(
							Vector2<int>{evt.mMouseInput.X, evt.mMouseInput.Y}, 1, "PointerRight");
						break;
					case MIE_MMOUSE_PRESSED_DOWN:
						result = mMouseHandler->OnMouseButtonDown(
							Vector2<int>{evt.mMouseInput.X, evt.mMouseInput.Y}, 1, "PointerMiddle");
						break;
					case MIE_MMOUSE_LEFT_UP:
						result = mMouseHandler->OnMouseButtonUp(
							Vector2<int>{evt.mMouseInput.X, evt.mMouseInput.Y}, 1, "PointerMiddle");
						break;
					case MIE_MOUSE_WHEEL:
						if (evt.mMouseInput.mWheel > 0)
							result = mMouseHandler->OnWheelRollUp();
						else
							result = mMouseHandler->OnWheelRollDown();
						break;
					default:
						break;
				}
			} // if
			break;

		default:
			return false;
	}
	return result;
}

/*
	OnUpdate is called once per frame by the application layer so that it can perform non-rendering
	update tasks. The OnUpdate() chain is called as quickly as the game loops and is used to update
	my object attached to the human view. In this case, the ProcessManager is updated, as well as
	any of the screen elements attached to the human view, such as updating the objects in the 3D
	scene, which is itself a screen element. A game object that exists in the game universe and is
	affected by game rules, like physics, belongs to the game logic. Whenever the game object moves
	or change state, events are generated that eventually make their way to the game views, where
	they update their internal representations of these objects. There is a different set of objects
	that only exist visually and have no real effect on the world themselves, such as particle
	effects, which is updated in this function. Since the game logic knows nothing about them, they
	are completely contained in the human view and need some way to be updated if they are animating.
	The audio system is another example of what human perceives but the game logic does not.
	Background music and ambient sound effects have no effect on the game logic per se and therefore
	can safely belong to the human view. The audio system is actually managed as a Process object that
	is attached to the ProcessManager contained in the human view.
*/
void HumanView::OnUpdate(const int deltaMilliseconds)
{
	m_pProcessManager->UpdateProcesses(deltaMilliseconds);

	m_Console.OnUpdate(deltaMilliseconds);

	// This section of code was added post-press. It runs through the screenlist
	// and calls OnUpdate. Some screen elements need to update every frame, one 
	// example of this is a 3D scene attached to the human view.
	//
	for(eastl::list<eastl::shared_ptr<BaseScreenElement>>::iterator i =
		m_ScreenElements.begin(); i!=m_ScreenElements.end(); ++i)
	{
		(*i)->OnUpdate(deltaMilliseconds);
	}
}

//
// HumanView::OnAnimate						- Chapter 10, page 277
//
void HumanView::OnAnimate(unsigned int uTime)
{

	// This section of code was added post-press. It runs through the screenlist
	// and calls OnAnimate. Some screen elements need to update every frame, one 
	// example of this is a 3D scene attached to the human view.
	//
	for(eastl::list<eastl::shared_ptr<BaseScreenElement>>::iterator i =
		m_ScreenElements.begin(); i!=m_ScreenElements.end(); ++i)
	{
		(*i)->OnAnimate(uTime);
	}
}

//
// HumanView::VPushElement						- Chapter 10, page 274
//
void HumanView::PushElement(const eastl::shared_ptr<BaseScreenElement>& pElement)
{
	m_ScreenElements.push_front(pElement);
}

//
// HumanView::VPopElement						- Chapter 10, page 274
//
//   
//
void HumanView::RemoveElement(const eastl::shared_ptr<BaseScreenElement>& pElement)
{
	m_ScreenElements.remove(pElement);
}


//
// HumanView::RegisterAllDelegates, HumanView::RemoveAllDelegates		- not described in the book
//
//   Aggregates calls to manage event listeners for the HumanView class.
//
void HumanView::RegisterAllDelegates(void)
{

    BaseEventManager* pGlobalEventManager = BaseEventManager::Get();
//	pGlobalEventManager->AddListener(
//		MakeDelegate(this, &HumanView::GameStateDelegate), EvtData_Game_State::sk_EventType);
    pGlobalEventManager->AddListener(
		MakeDelegate(this, &HumanView::PlaySoundDelegate), EvtData_PlaySound::sk_EventType);
}

void HumanView::RemoveAllDelegates(void)
{
    BaseEventManager* pGlobalEventManager = BaseEventManager::Get();
//	pGlobalEventManager->RemoveListener(
//		MakeDelegate(this, &HumanView::GameStateDelegate), EvtData_Game_State::sk_EventType);
    pGlobalEventManager->RemoveListener(
		MakeDelegate(this, &HumanView::PlaySoundDelegate), EvtData_PlaySound::sk_EventType);
}


//
// HumanView::PlaySoundDelegate							- Chapter X, page Y
//
void HumanView::PlaySoundDelegate(BaseEventDataPtr pEventData)
{
    eastl::shared_ptr<EvtData_PlaySound> pCastEventData = 
		eastl::static_pointer_cast<EvtData_PlaySound>(pEventData);
	/*
    // play the sound a bullet makes when it hits a teapot
	GameApplication* gameApp = (GameApplication*)Application::App;
    BaseResource resource(pCastEventData->GetResource().c_str());
    eastl::shared_ptr<ResHandle> srh = 
		eastl::static_pointer_cast<ResHandle>(gameApp->mResCache->GetHandle(&resource));
    shared_ptr<SoundProcess> sfx(new SoundProcess(srh, 100, false));
    mProcessManager->AttachProcess(sfx);
	*/
}

//
// HumanView::GameStateDelegate							- Chapter X, page Y
//
void HumanView::GameStateDelegate(BaseEventDataPtr pEventData)
{
//    eastl::shared_ptr<EvtData_Game_State> pCastEventData = 
//		static_pointer_cast<EvtData_Game_State>(pEventData);
//    m_BaseGameState = pCastEventData->GetGameState(); 
}


//==============================================================
// HumanView::Console - defines the a class to manage a console to type in commands
//
//   Not described in the book. 
//
const int kCursorBlinkTimeMS = 500;

char const * const kExitString = "exit";
char const * const kClearString = "clear";

HumanView::Console::Console()
: m_bActive( false ), m_bExecuteStringOnUpdate( false )
{
	m_ConsoleInputSize = 48;

	m_CurrentInputString = L"";

	m_CursorBlinkTimer = kCursorBlinkTimeMS;
	m_bCursorOn = true;

	Console::OnInit();
}

HumanView::Console::~Console()
{
}

void HumanView::Console::AddDisplayText( const eastl::wstring & newText )
{
	/*
	BaseUIStaticText* text = 
		(BaseUIStaticText*)GetRootUIElement()->GetElementFromId(1).get();
	text->SetText(newText.c_str());
	m_CurrentOutputString += newText;
	m_CurrentOutputString += '\n';
	*/
}

void HumanView::Console::SetDisplayText( const eastl::wstring & newText )
{
	/*
	BaseUIStaticText* text = 
		(BaseUIStaticText*)GetRootUIElement()->GetElementFromId(1).get();
	text->SetText(newText.c_str());
	m_CurrentOutputString = newText;
	*/
}

bool HumanView::Console::OnInit( )
{
	BaseUI::OnInit();
	/*
	unsigned int width = m_pRenderer->GetScreenSize().Width;
	unsigned int height = m_pRenderer->GetScreenSize().Height;
	shared_ptr<BaseUIStaticText> consoleText(
		AddStaticText(L">", RectangleBase<2, int>(0, height, width, height-10), false, true, 0, 1, true));
	consoleText->SetOverrideColor(Color(1.0f, 1.0f, 1.0f, 1.0f)); //white font
	consoleText->SetBackgroundColor(Color(1.0f, 0.0f, 0.0f, 0.0f)); //black background
	*/
	return true;
}

void HumanView::Console::OnUpdate( const int deltaMilliseconds )
{
	//Don't do anything if not active.
	if ( !m_bActive )
	{
		return;	//Bail!
	}
	/*
	//Do we have a string to execute?
	if (m_bExecuteStringOnUpdate)
	{
		const eastl::string renderedInputString = m_CurrentInputString;
		if ( m_CurrentInputString == kExitString )
		{
			SetActive( false );
			m_CurrentInputString = eastl::wstring(">") ;
		}
		else if ( m_CurrentInputString == kClearString )
		{
			m_CurrentInputString = eastl::wstring(">") ;	//clear
		}
		else
		{
			//Put the input string into the output window.
			AddDisplayText( renderedInputString );

			const int retVal = true;

			//Attempt to execute the current input string...
            if (!m_CurrentInputString.empty())
			{
				LuaStateManager::Get()->ExecuteString(
					eastl::string(m_CurrentInputString.c_str()).c_str());
			}

			//Clear the input string
			m_CurrentInputString.clear();
		}

		//We're accepting input again.
		m_bExecuteStringOnUpdate = false;
        SetActive(false);
	}
	*/
	//Update the cursor blink timer...
	m_CursorBlinkTimer -= deltaMilliseconds;

	if ( m_CursorBlinkTimer < 0 )
	{
		m_CursorBlinkTimer = 0;

		m_bCursorOn = !m_bCursorOn;

		m_CursorBlinkTimer = kCursorBlinkTimeMS;
	}
}

bool HumanView::Console::OnRender(double fTime, float fElapsedTime)
{
	//Don't do anything if not active.
	if ( !m_bActive )
	{
		return false;	//Bail!
	}

	return BaseUI::OnRender(fTime, fElapsedTime);
}

void HumanView::Console::HandleKeyboardInput( const unsigned int keyVal, 
	const unsigned int oemKeyVal, const bool bKeyDown )
{
	if ( true == m_bExecuteStringOnUpdate )
	{
		//We've already got a string to execute; ignore.
		return;
	}

	//See if it's a valid key press that we care about.
	switch( oemKeyVal )
	{
	    case VK_BACK:
		{
			const size_t strSize = m_CurrentInputString.size();
			if ( strSize > 0 )
			{
				m_CurrentInputString.erase(strSize - 1);
			}
            break;
		}

    	case VK_RETURN:
		{
			m_bExecuteStringOnUpdate = true;	//Execute this eastl::string.
            break;
		}

    	default:
        {
		    m_CurrentInputString += (wchar_t)keyVal;
		    break;
        }
	}
}
