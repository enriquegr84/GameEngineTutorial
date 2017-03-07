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

#include "GameEngine/GameEngine.h"

#include "Actors/AudioComponent.h"
#include "Actors/RenderComponentInterface.h"

//events related
#include "Audio/DirectSoundAudio.h"
#include "Audio/SoundProcess.h"
#include "Events/Events.h"
#include "Events/EventManagerImpl.h"

#include "Graphics/IRenderer.h"
#include "Scenes/Scene.h"
#include "Process/Process.h"
#include "Scripting/LUAScripting/LuaStateManager.h"

const unsigned int SCREEN_REFRESH_RATE(1000/60);
const GameViewId InvalidGameViewId = 0xffffffff;

//
// HumanView::HumanView - Chapter 10, page 272
//
HumanView::HumanView(const shared_ptr<IRenderer>& renderer)
{
	InitAudio(); 

	m_pProcessManager = new ProcessManager;

	m_runFullSpeed = true;
	m_PointerRadius = 1;	// we assume we are on a mouse enabled machine - if this were a tablet we should detect it here.
	m_ViewId = InvalidGameViewId;

	// Added post press for move, new, and destroy actor events and others
	RegisterAllDelegates();
	m_BaseGameState = BGS_Initializing;		// what is the current game state

	if (renderer)
	{
		// Moved to the HumanView class post press
		m_pScene.reset(new ScreenElementScene(renderer));

		m_pCamera.reset(new CameraSceneNode(
			g_pGameApp->GetGameLogic()->GetNewActorID(), m_pScene.get(), &g_IdentityMatrix4));
		GE_ASSERT(m_pScene && m_pCamera && _GE_TEXT("Out of memory"));
		m_pCamera->SetFarValue(20000.f); // this increase a shadow visible range.

		m_pScene->AddChild(m_pCamera->Get()->GetId(), m_pCamera);
		m_pScene->SetActiveCamera(m_pCamera);
	}
}


HumanView::~HumanView()
{
	// [mrmike] - RemoveAllDelegates was added post press to handle move, new, and destroy actor events.
	RemoveAllDelegates();

	while (!m_ScreenElements.empty())
	{
		m_ScreenElements.pop_front();		
	}

	SAFE_DELETE(m_pProcessManager);

	SAFE_DELETE(g_pAudio);
}


bool HumanView::LoadGameDelegate(XmlElement* pLevelData) 
{ 
	PushElement(m_pScene);  
	return true; 
}

bool HumanView::LoadGame(XmlElement* pLevelData)
{
    // call the delegate method
    return LoadGameDelegate(pLevelData);
}

//
// HumanView::OnRender							- Chapter 10, page 274
//
void HumanView::OnRender(double fTime, float fElapsedTime )
{
	int deltaMilliseconds = int(fElapsedTime * 1000.0f);

	// It is time to draw ?
	if (g_pGameApp->m_pRenderer->PreRender())
	{
		if( m_runFullSpeed || ( deltaMilliseconds > SCREEN_REFRESH_RATE) )
		{
			m_ScreenElements.sort(SortBy_SharedPtr_Content<BaseScreenElement>());

			for(ScreenElementList::iterator i=m_ScreenElements.begin(); i!=m_ScreenElements.end(); ++i)
			{
				if ( (*i)->IsVisible() )
				{
					(*i)->OnRender(fTime, fElapsedTime);
				}
			}

            RenderText();

			// Let the console render.
			m_Console.OnRender(fTime, fElapsedTime);

			//g_pGameApp->GetGameLogic()->RenderDiagnostics();
		}
    }
	g_pGameApp->m_pRenderer->PostRender();
}


//
// HumanView::OnRestore						- Chapter 10, page 275
//
bool HumanView::OnRestore()
{
	bool hr = true;
	for(ScreenElementList::iterator i=m_ScreenElements.begin(); i!=m_ScreenElements.end(); ++i)
	{
		return ( (*i)->OnRestore() );
	}

	return hr;
}


//
// HumanView::OnLostDevice						- not described in the book
//
//    Recursively calls VOnLostDevice for everything attached to the HumanView. 

bool HumanView::OnLostDevice() 
{
	for(ScreenElementList::iterator i=m_ScreenElements.begin(); i!=m_ScreenElements.end(); ++i)
	{
		return ( (*i)->OnLostDevice() );
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

	if (!g_pAudio->Initialize(g_pGameApp->m_pPlatform->GetID()))
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



//
// HumanView::OnMsgProc						- Chapter 10, page 279
//
bool HumanView::OnMsgProc( const Event& evt )
{
	// Iterate through the screen layers first
	// In reverse order since we'll send input messages to the 
	// screen on top
	for(ScreenElementList::reverse_iterator i=m_ScreenElements.rbegin(); i!=m_ScreenElements.rend(); ++i)
	{
		if ( (*i)->IsVisible() )
		{
			if ( (*i)->OnMsgProc( evt ) )
			{
				return 1;
			}
		}
	}

	bool result = 0;
	switch (evt.m_EventType) 
	{
		case EET_KEY_INPUT_EVENT:
			if (m_Console.IsActive())
			{	
				//See if it was the console key.
				if (L'º' == evt.m_KeyInput.m_Char)
				{
					m_Console.SetActive(false);
				}
				else
				{
					const unsigned int oemScan = int( evt.m_KeyInput.m_Char & ( 0xff << 16 ) ) >> 16;
					m_Console.HandleKeyboardInput( evt.m_KeyInput.m_Char, MapVirtualKey( oemScan, 1 ), evt.m_KeyInput.m_bPressedDown );
				}
			}
			else
			{
				//See if it was the console key.
				if (L'º' == evt.m_KeyInput.m_Char)
				{
					m_Console.SetActive(true);
				}
				else if (evt.m_KeyInput.m_bPressedDown)
				{
					if (m_KeyboardHandler)
						result = m_KeyboardHandler->OnKeyDown(evt.m_KeyInput.m_Key);
				}
				else
				{
					if (m_KeyboardHandler)
						result = m_KeyboardHandler->OnKeyUp(evt.m_KeyInput.m_Key);
				}
			}
			break;
		case EET_MOUSE_INPUT_EVENT:
			if (m_MouseHandler)
			{
				switch (evt.m_MouseInput.m_Event)
				{
					case EMIE_MOUSE_MOVED:
						result = m_MouseHandler->OnMouseMove(
							Vector2i(evt.m_MouseInput.X,evt.m_MouseInput.Y),1);
						break;
					case EMIE_LMOUSE_PRESSED_DOWN:
						result = m_MouseHandler->OnMouseButtonDown(
							Vector2i(evt.m_MouseInput.X,evt.m_MouseInput.Y), 1, "PointerLeft");
						break;
					case EMIE_LMOUSE_LEFT_UP:
						result = m_MouseHandler->OnMouseButtonUp(
							Vector2i(evt.m_MouseInput.X,evt.m_MouseInput.Y), 1, "PointerLeft");
						break;
					case EMIE_RMOUSE_PRESSED_DOWN:
						result = m_MouseHandler->OnMouseButtonDown(
							Vector2i(evt.m_MouseInput.X,evt.m_MouseInput.Y), 1, "PointerRight");
						break;
					case EMIE_RMOUSE_LEFT_UP:
						result = m_MouseHandler->OnMouseButtonUp(
							Vector2i(evt.m_MouseInput.X,evt.m_MouseInput.Y), 1, "PointerRight");
						break;
					case EMIE_MMOUSE_PRESSED_DOWN:
						result = m_MouseHandler->OnMouseButtonDown(
							Vector2i(evt.m_MouseInput.X,evt.m_MouseInput.Y), 1, "PointerMiddle");
						break;
					case EMIE_MMOUSE_LEFT_UP:
						result = m_MouseHandler->OnMouseButtonUp(
							Vector2i(evt.m_MouseInput.X,evt.m_MouseInput.Y), 1, "PointerMiddle");
						break;
					case EMIE_MOUSE_WHEEL:
						if (evt.m_MouseInput.m_Wheel > 0)
							result = m_MouseHandler->OnWheelRollUp();
						else
							result = m_MouseHandler->OnWheelRollDown();
						break;
					default:
						break;
				}
			} // if
			break;

		default:
			return 0;
	}
	return result;
}

//
// HumanView::OnUpdate						- Chapter 10, page 277
//
void HumanView::OnUpdate(const int deltaMilliseconds)
{
	m_pProcessManager->UpdateProcesses(deltaMilliseconds);

	m_Console.OnUpdate(deltaMilliseconds);

	// This section of code was added post-press. It runs through the screenlist
	// and calls VOnUpdate. Some screen elements need to update every frame, one 
	// example of this is a 3D scene attached to the human view.
	//
	for(ScreenElementList::iterator i=m_ScreenElements.begin(); i!=m_ScreenElements.end(); ++i)
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
	for(ScreenElementList::iterator i=m_ScreenElements.begin(); i!=m_ScreenElements.end(); ++i)
	{
		(*i)->OnAnimate(uTime);
	}
}

//
// HumanView::VPushElement						- Chapter 10, page 274
//
void HumanView::PushElement(const shared_ptr<BaseScreenElement>& pElement)
{
	m_ScreenElements.push_front(pElement);
}

//
// HumanView::VPopElement						- Chapter 10, page 274
//
//   
//
void HumanView::RemoveElement(const shared_ptr<BaseScreenElement>& pElement)
{
	m_ScreenElements.remove(pElement);
}

//
// HumanView::SetActiveCameraOffset					- not described in the book
//
//   Sets a camera offset, useful for making a 1st person or 3rd person game
//
/*
void HumanView::SetActiveCameraOffset(const Vec4 & camOffset )
{
	GE_ASSERT(m_pCamera);
	if (m_pCamera)
	{
		m_pCamera->SetActiveCameraOffset( camOffset );
	}
}
*/


//
// HumanView::RegisterAllDelegates, HumanView::RemoveAllDelegates		- not described in the book
//
//   Aggregates calls to manage event listeners for the HumanView class.
//
void HumanView::RegisterAllDelegates(void)
{

    BaseEventManager* pGlobalEventManager = BaseEventManager::Get();
//	pGlobalEventManager->AddListener(MakeDelegate(this, &HumanView::GameStateDelegate), EvtData_Game_State::sk_EventType);
    pGlobalEventManager->AddListener(MakeDelegate(this, &HumanView::PlaySoundDelegate), EvtData_PlaySound::sk_EventType);
}

void HumanView::RemoveAllDelegates(void)
{
    BaseEventManager* pGlobalEventManager = BaseEventManager::Get();
//	pGlobalEventManager->RemoveListener(MakeDelegate(this, &HumanView::GameStateDelegate), EvtData_Game_State::sk_EventType);
    pGlobalEventManager->RemoveListener(MakeDelegate(this, &HumanView::PlaySoundDelegate), EvtData_PlaySound::sk_EventType);
}


//
// HumanView::PlaySoundDelegate							- Chapter X, page Y
//
void HumanView::PlaySoundDelegate(BaseEventDataPtr pEventData)
{
    shared_ptr<EvtData_PlaySound> pCastEventData = static_pointer_cast<EvtData_PlaySound>(pEventData);

    // play the sound a bullet makes when it hits a teapot
    Resource resource(pCastEventData->GetResource().c_str());
    shared_ptr<ResHandle> srh = static_pointer_cast<ResHandle>(g_pGameApp->m_ResCache->GetHandle(&resource));
    shared_ptr<SoundProcess> sfx(new SoundProcess(srh, 100, false));
    m_pProcessManager->AttachProcess(sfx);
}

//
// HumanView::GameStateDelegate							- Chapter X, page Y
//
void HumanView::GameStateDelegate(BaseEventDataPtr pEventData)
{
//    shared_ptr<EvtData_Game_State> pCastEventData = static_pointer_cast<EvtData_Game_State>(pEventData);
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

	m_CurrentInputString = eastl::string("");

	m_CursorBlinkTimer = kCursorBlinkTimeMS;
	m_bCursorOn = true;

	Console::OnInit();
}

HumanView::Console::~Console()
{
}

void HumanView::Console::AddDisplayText( const eastl::wstring & newText )
{
	BaseUIStaticText* text = 
		(BaseUIStaticText*)GetRootUIElement()->GetElementFromId(1).get();
	text->SetText(newText.c_str());
	//m_CurrentOutputString += newText;
	//m_CurrentOutputString += '\n';
}

void HumanView::Console::SetDisplayText( const eastl::wstring & newText )
{
	BaseUIStaticText* text = 
		(BaseUIStaticText*)GetRootUIElement()->GetElementFromId(1).get();
	text->SetText(newText.c_str());
	//m_CurrentOutputString = newText;
}

bool HumanView::Console::OnInit( )
{
	BaseUI::OnInit();

	u32 width = g_pGameApp->m_pRenderer->GetScreenSize().Width;
	u32 height = g_pGameApp->m_pRenderer->GetScreenSize().Height;
	shared_ptr<BaseUIStaticText> consoleText(
		AddStaticText(L">", Rect<s32>(0, height, width, height-10), false, true, 0, 1, true));
	consoleText->SetOverrideColor(Color(1.0f, 1.0f, 1.0f, 1.0f)); //white font
	consoleText->SetBackgroundColor(Color(1.0f, 0.0f, 0.0f, 0.0f)); //black background

	return true;
}

void HumanView::Console::OnUpdate( const int deltaMilliseconds )
{
	//Don't do anything if not active.
	if ( !m_bActive )
	{
		return;	//Bail!
	}

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
