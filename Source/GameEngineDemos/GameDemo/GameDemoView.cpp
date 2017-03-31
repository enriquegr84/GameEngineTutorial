//========================================================================
// DemosView.cpp : source file for the sample game
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

#include "GameDemoStd.h"

#include "Game/GameOption.h"

#include "Audio/Audio.h"
#include "Audio/SoundProcess.h"
#include "Graphic/Renderer/Renderer.h"

#include "Graphic/Scene/Scene.h"
/*
#include "Graphic/Scene/SceneNodes.h"
#include "Graphic/Scene/Element/CameraSceneNode.h"
#include "Graphic/Scene/Controller/MovementController.h"
*/
#include "Core/Event/Event.h"
#include "Core/Event/EventManager.h"

#include "Physic/PhysicEventListener.h"

#include "Game/View/HumanView.h"
//#include "Game/View/UI/MessageBox.h"
#include "Game/Actor/Actor.h"
#include "Game/Actor/RenderComponent.h"
/*
#include "Game/View/UI/UIEngine.h"
#include "Game/View/UI/ScreenStateManager.h"
#include "Game/View/UI/Screens/ScreenMainMenu.h"

#include "Game/Config/User.h"
*/
#include "GameDemoView.h"
#include "GameDemoEvents.h"
#include "GameDemoManager.h"
#include "GameDemoNetwork.h"
#include "GameDemoController.h"

//========================================================================
//
// MainMenuUI & MainMenuView implementation
//
//
//========================================================================

#define CID_CREATE_GAME_RADIO			(2)
#define CID_JOIN_GAME_RADIO				(3)
#define CID_NUM_AI_SLIDER				(4)
#define CID_NUM_PLAYER_SLIDER			(5)
#define CID_HOST_LISTEN_PORT			(6)
#define CID_CLIENT_ATTACH_PORT			(7)
#define CID_START_BUTTON				(8)
#define CID_HOST_NAME					(9)
#define CID_NUM_AI_LABEL				(10)
#define CID_NUM_PLAYER_LABEL			(11)
#define CID_HOST_LISTEN_PORT_LABEL		(12)
#define CID_CLIENT_ATTACH_PORT_LABEL	(13)
#define CID_HOST_NAME_LABEL				(14)
#define CID_LEVEL_LABEL					(15)
#define CID_LEVEL_GUIListBox			(16)


const int SampleUIWidth = 600;
const int SampleUIHeight = 600;

MainMenuUI::MainMenuUI()
{

}

MainMenuUI::~MainMenuUI()
{ 
	//GUIEngine::DialogQueue::deallocate();

	//D3DRenderer::g_DialogResourceManager.UnregisterDialog(&m_SampleUI);
	//if(demos_manager)
	//	SAFE_DELETE ( demos_manager );

	//ScreenStateManager::Deallocate();
	//GUIEventHandler::Deallocate();
}

bool MainMenuUI::OnInit()
{
	BaseUI::OnInit();
	
	UIEngine::Init(this, NULL);
	UIEngine::AddLoadingIcon(
		gameDemoApp.m_pRenderer->GetTexture("art/gui/notes.png").get() );
	//gameDemoApp.m_pPlatform->OnPause(1000);
	UIEngine::AddLoadingIcon( 
		gameDemoApp.m_pRenderer->GetTexture("art/gui/cup_gold.png").get() );
	//gameDemoApp.m_pPlatform->OnPause(1000);
    UIEngine::AddLoadingIcon( 
		gameDemoApp.m_pRenderer->GetTexture("art/gui/options_video.png").get() );
	//gameDemoApp.m_pPlatform->OnPause(1000);
    UIEngine::AddLoadingIcon( 
		gameDemoApp.m_pRenderer->GetTexture("art/gui/gui_lock.png").get() );
	//gameDemoApp.m_pPlatform->OnPause(1000);
    UIEngine::AddLoadingIcon( 
		gameDemoApp.m_pRenderer->GetTexture("art/gui/gift.png").get() );
	//gameDemoApp.m_pPlatform->OnPause(1000);
    UIEngine::AddLoadingIcon( 
		gameDemoApp.m_pRenderer->GetTexture("art/gui/banana.png").get() );
	//gameDemoApp.m_pPlatform->OnPause(1000);
	/*
    if(!UserConfigParams::m_no_start_screen)
    {
        ScreenStateManager::Get()->PushScreen(ScreenMainMenu::GetInstance());
	}
	*/
	return true;
}

bool MainMenuUI::OnRestore()
{
    //mSampleUI.SetLocation( 
	//	(gameDemoApp->GetScreenSize().x - SampleUIWidth)/2, 
	//	(gameDemoApp->GetScreenSize().y - SampleUIHeight) / 2  );
    //mSampleUI.SetSize( SampleUIWidth, SampleUIHeight );
	return true;
}

bool MainMenuUI::OnRender(double fTime, float fElapsedTime)
{
    // If the resolution should be switched, do it now. This will delete the
    // old device and create a new one.
	/*
    if (mResolutionChanging!=RES_CHANGE_NONE)
    {
        ApplyResolutionSettings();
        if(mResolutionChanging==RES_CHANGE_YES)
        new DialogConfirmResolution();
        mResolutionChanging = RES_CHANGE_NONE;
    }
	*/

    {
		//g_SuperTuxKartApp.m_pRenderer->PreRender(
		//	/*backBuffer clear*/ true, /*zBuffer*/ true, Color(255,100,101,140));

		float dt = gameDemoApp.GetLimitedDt();
		UIEngine::Update(dt);
		UIEngine::Render(dt);

		//g_SuperTuxKartApp.m_pRenderer->PostRender();
    }

    //if (m_request_screenshot) doScreenShot();

    // Enable this next print statement to get render information printed
    // E.g. number of triangles rendered, culled etc. The stats is only
    // printed while the race is running and not while the in-game menu
    // is shown. This way the output can be studied by just opening the
    // menu.
    //if(World::getWorld() && World::getWorld()->isRacePhase())
    //    printRenderStats();

	return true;
};

bool MainMenuUI::OnMsgProc( const Event& evt )
{
	return BaseUI::OnMsgProc(evt);
	//return m_SampleUI.MsgProc( msg.m_hWnd, msg.m_uMsg, msg.m_wParam, msg.m_lParam );
}


//--------------------------------------------------------------------------------------
// Handles the GUI events
//--------------------------------------------------------------------------------------
//    Note: pUserContext added to comply with DirectX 9c - June 2005 Update
//
bool MainMenuUI::OnEvent(const Event& evt)
{
    if (evt.mEventType == ET_GUI_EVENT)
    {
		return UIEventHandler::Get()->OnEvent(evt);
    }

	return false;
}


MainMenuView::MainMenuView() : HumanView(eastl::shared_ptr<Renderer>())
{
	mMainMenuUI.reset(new MainMenuUI); 
	mMainMenuUI->OnInit();
	PushElement(mMainMenuUI);
}


MainMenuView::~MainMenuView()
{
}


void MainMenuView::RenderText()
{
	HumanView::RenderText();
}


void MainMenuView::OnUpdate(unsigned long deltaMs)
{
	HumanView::OnUpdate( deltaMs );
}


bool MainMenuView::OnMsgProc( const Event& evt )
{
	if (mMainMenuUI->IsVisible() )
	{
		if (HumanView::OnMsgProc(evt))
			return 1;
	}
	return 0;
}

//========================================================================
//
// StandardHUD implementation
//
//
//========================================================================

//--------------------------------------------------------------------------------------
// UI control IDs
//--------------------------------------------------------------------------------------
#define IDC_TOGGLEFULLSCREEN    1
#define IDC_TOGGLEREF           2
#define IDC_CHANGEDEVICE        3

StandardHUD::StandardHUD()
{

}


StandardHUD::~StandardHUD() 
{ 
//  [mrmike] - this causes a "memory written after freed error" so I commented it out.
//	D3DRenderer::g_DialogResourceManager.UnregisterDialog(&m_HUD); 
}

bool StandardHUD::OnInit()
{
	BaseUI::OnInit();

	/*
	To make the font a little bit nicer, we load an external font
	and set it as the new default font in the skin.
	To keep the standard font for tool tip text, we set it to
	the built-in font.
	
	shared_ptr<IGUIFont> font(GetFont("fontcourier.bmp"));
	if (font)
		GetSkin()->SetFont(font);

    // Initialize dialogs
	//m_HUD.Init( &D3DRenderer::g_DialogResourceManager );
    //m_HUD.SetCallback( OnGUIEvent ); 
	int iY = 10; 
	AddButton(Rect<s32>(35, iY, 160, iY + 22), 0, IDC_TOGGLEFULLSCREEN, L"Toggle full screen");
	iY += 24;
	AddButton(Rect<s32>(35, iY, 160, iY + 22), 0, IDC_TOGGLEREF, L"Toggle REF (F3)");
    //m_HUD.AddButton( IDC_CHANGEDEVICE, L"Change device (F2)", 35, iY += 24, 125, 22 );
	*/
	return true;
}

bool StandardHUD::OnRestore()
{
    //mHUD.SetLocation( gameApp->GetScreenSize().x - 170, 0 );
    //mHUD.SetSize( 170, 170 );
	return BaseUI::OnRestore();
}

bool StandardHUD::OnRender(double time, float elapsedTime)
{
	/*
	HRESULT hr;
	DXUT_BeginPerfEvent( DXUT_PERFEVENTCOLOR, L"StandardUI" ); // These events are to help PIX identify what the code is doing
	V( mHUD.OnRender( fElapsedTime ) );
	DXUT_EndPerfEvent();
	*/
	return BaseUI::OnRender(time, elapsedTime);
};


bool StandardHUD::OnMsgProc( const Event& evt )
{
	return BaseUI::OnMsgProc( evt );
    //return mHUD.MsgProc( msg.m_hWnd, msg.m_uMsg, msg.m_wParam, msg.m_lParam );
}

//--------------------------------------------------------------------------------------
// Handles the GUI events
//--------------------------------------------------------------------------------------
//    Note: pUserContext added to comply with DirectX 9c - June 2005 Update
//
bool StandardHUD::OnEvent(const Event& evt)
{
	if (evt.mEventType == ET_UI_EVENT)
	{
		int id = evt.mUIEvent.mCaller->GetID();

		switch(evt.mUIEvent.mEventType)
		{
			case GET_BUTTON_CLICKED:
				switch (id)
				{
				case IDC_TOGGLEFULLSCREEN: 
					gameDemoApp.m_pPlatform->SwitchToFullScreen(); break;
				case IDC_TOGGLEREF:        
					gameDemoApp.m_pPlatform->SetResizable(); break;
				}
				break;
		}
	}

	return false;
}

//========================================================================
//
// GameDemoHumanView Implementation
//
//========================================================================

//
// GameDemoHumanView::GameDemoHumanView	- Chapter 19, page 724
//
GameDemoHumanView::GameDemoHumanView(eastl::shared_ptr<Renderer> renderer) 
	: HumanView(renderer)
{ 
	mShowUI = true; 
    RegisterAllDelegates();
}


GameDemoHumanView::~GameDemoHumanView()
{
    RemoveAllDelegates();
}

//
// GameDemoHumanView::OnMsgProc				- Chapter 19, page 727
//
bool GameDemoHumanView::OnMsgProc( const Event& evt )
{
	if (HumanView::OnMsgProc(evt))
		return 1;

	switch(evt.mEventType)
	{
	case ET_UI_EVENT:
		// hey, why is the user sending gui events..?
		break;
	case ET_KEY_INPUT_EVENT:
		{
			if (evt.mKeyInput.mPressedDown)
			{
				switch (evt.mKeyInput.mKey)
				{
					case KEY_KEY_1:
					{
						mShowUI = !mShowUI;
						//mStandardHUD->VSetVisible(m_bShowUI);
						return true;
					}

					case KEY_KEY_2:
					{
						// test the picking API
						/*
						POINT ptCursor;
						GetCursorPos( &ptCursor );
						ScreenToClient( gameApp->GetHwnd(), &ptCursor );

						RayCast rayCast(ptCursor);
						mScene->Pick(&rayCast);
						rayCast.Sort();

						if (rayCast.m_NumIntersections)
						{
							// You can iterate through the intersections on the raycast.
							int a = 0;
						}
						*/
					}
					break;

					case KEY_KEY_3:
					{
						//extern void CreateThreads();
						//CreateThreads();

						extern void testThreading(ProcessManager *procMgr, bool runProtected);
						testThreading(mProcessManager, true);

						return true;

						Sleep(5000);
						testThreading(mProcessManager, false);
						Sleep(5000);

						extern void testRealtimeEvents(ProcessManager *procMgr);
						testRealtimeEvents(mProcessManager);

						extern void testRealtimeDecompression(ProcessManager *procMgr);
						testRealtimeDecompression(mProcessManager);
					}
					break;

					case KEY_KEY_4:
					{
						Resource resource("scripts\\test.lua");
						// this actually loads the Lua file from the zip file
						eastl::shared_ptr<ResHandle> pResourceHandle = 
							gameApp->m_ResCache->GetHandle(&resource);
					}
					break;

					case KEY_KEY_5:
					{
						/*
							eastl::shared_ptr<EventDataScriptEventTestToLua> pEvent(
								new EventDataScriptEventTestToLua);
							EventManager::Get()->QueueEvent(pEvent);
						*/
					}
					break;

					case KEY_KEY_6:
					{
						//LuaStateManager::Get()->ExecuteString("TestProcess()");
					}
					break;

					case KEY_KEY_7:
					{
						GameDemoLogic *twg = static_cast<DemosLogic *>(g_pGameApp->m_pGame);
						twg->ToggleRenderDiagnostics();
					}	
					break;

					case KEY_KEY_8:
					{
						m_KeyboardHandler = m_pDemoController;
						m_MouseHandler = m_pDemoController;
						m_pCamera->SetTarget(m_pTeapot);
						//m_pTeapot->SetAlpha(0.8f);
						//ReleaseCapture();
						return true;
					}

					case KEY_KEY_9:
					{
						m_KeyboardHandler = m_pFreeCameraController;
						m_MouseHandler = m_pFreeCameraController;
						m_pCamera->ClearTarget();
						//m_pTeapot->SetAlpha(fOPAQUE);
						//SetCapture((HWND)g_pGameApp->m_pPlatform->GetID());
						return true;
					}

					case KEY_ESCAPE:
						g_pGameApp->SetQuitting(true);
						return true;
				}
			}
		}
	}

	return false;
}

//
// GameDemoHumanView::RenderText				- Chapter 19, page 727
//
void GameDemoHumanView::RenderText()
{
	HumanView::RenderText();
	/*
	D3DRenderer::g_pTextHelper->Begin();

    // Gameplay UI (with shadow)....
    if (!m_gameplayText.empty())
    {
	    D3DRenderer::g_pTextHelper->SetInsertionPos( g_pApp->GetScreenSize().x/2, 5 );
	    D3DRenderer::g_pTextHelper->SetForegroundColor( D3DXCOLOR( 0.0f, 0.0f, 0.0f, 1.0f ) );
	    D3DRenderer::g_pTextHelper->DrawTextLine(m_gameplayText.c_str());
	    D3DRenderer::g_pTextHelper->SetInsertionPos( g_pApp->GetScreenSize().x/2-1, 5-1 );
	    D3DRenderer::g_pTextHelper->SetForegroundColor( D3DXCOLOR( 0.25f, 1.0f, 0.25f, 1.0f ) );
	    D3DRenderer::g_pTextHelper->DrawTextLine(m_gameplayText.c_str());
    }
	// ...Gameplay UI

	if( m_bShowUI )
	{
		// Output statistics...
		D3DRenderer::g_pTextHelper->SetInsertionPos( 5, 5 );
		D3DRenderer::g_pTextHelper->SetForegroundColor( D3DXCOLOR( 1.0f, 1.0f, 0.0f, 1.0f ) );
		D3DRenderer::g_pTextHelper->DrawTextLine( DXUTGetFrameStats() );
		D3DRenderer::g_pTextHelper->DrawTextLine( DXUTGetDeviceStats() );
		//...output statistics
		
		D3DRenderer::g_pTextHelper->SetForegroundColor( D3DXCOLOR( 0.0f, 0.0f, 0.0f, 0.5f ) );

		//Game State...
		switch (m_BaseGameState)
		{
			case BGS_Initializing:
				D3DRenderer::g_pTextHelper->DrawTextLine(g_pApp->GetString(_T("IDS_INITIALIZING")).c_str());
				break;

			case BGS_MainMenu:
				D3DRenderer::g_pTextHelper->DrawTextLine(L"Main Menu");
				break;

//			case BGS_SpawnAI:
//				D3DRenderer::g_pTextHelper->DrawTextLine(L"Spawn AI");
//				break;

			case BGS_WaitingForPlayers:
				D3DRenderer::g_pTextHelper->DrawTextLine(g_pApp->GetString(_T("IDS_WAITING")).c_str());
				break;

			case BGS_LoadingGameEnvironment:
				D3DRenderer::g_pTextHelper->DrawTextLine(g_pApp->GetString(_T("IDS_LOADING")).c_str());
				break;

			case BGS_Running:
#ifndef DISABLE_PHYSICS
				D3DRenderer::g_pTextHelper->DrawTextLine(g_pApp->GetString(_T("IDS_RUNNING")).c_str());
#else
				D3DRenderer::g_pTextHelper->DrawTextLine(g_pApp->GetString(_T("IDS_NOPHYSICS")).c_str());
#endif //!DISABLE_PHYSICS
				break;
		}
		//...Game State

		//Camera...
		fschar_t buffer[256];
		const fschar_t *s = NULL;
		matrix4 toWorld, fromWorld;
		if (m_pCamera)
		{	
			m_pCamera->VGet()->Transform(&toWorld, &fromWorld);
		}
		swprintf(buffer, g_pApp->GetString(_T("IDS_CAMERA_LOCATION")).c_str(), toWorld.m[3][0], toWorld.m[3][1], toWorld.m[3][2]);
		D3DRenderer::g_pTextHelper->DrawTextLine( buffer );
		//...Camera

		//Help text.  Right justified, lower right of screen.
		RECT helpRect;
		helpRect.left = 0;
		helpRect.right = g_pApp->GetScreenSize().x - 10;
		helpRect.top = g_pApp->GetScreenSize().y - 15*8;
		helpRect.bottom = g_pApp->GetScreenSize().y;
		D3DRenderer::g_pTextHelper->SetInsertionPos( helpRect.right, helpRect.top );
		D3DRenderer::g_pTextHelper->SetForegroundColor( D3DXCOLOR( 1.0f, 0.75f, 0.0f, 1.0f ) );
		D3DRenderer::g_pTextHelper->DrawTextLine( helpRect, DT_RIGHT, g_pApp->GetString(_T("IDS_CONTROLS_HEADER")).c_str() );
		helpRect.top = g_pApp->GetScreenSize().y-15*7;
		D3DRenderer::g_pTextHelper->DrawTextLine( helpRect, DT_RIGHT, g_pApp->GetString(_T("IDS_CONTROLS")).c_str() );
		//...Help
	}//end if (m_bShowUI)

	D3DRenderer::g_pTextHelper->End();
	*/
}


//
// GameDemoHumanView::OnUpdate				- Chapter 19, page 730
//
void GameDemoHumanView::OnUpdate(unsigned long deltaMs)
{
	HumanView::OnUpdate( deltaMs );

	if (m_pFreeCameraController)
	{
		m_pFreeCameraController->OnUpdate(deltaMs);
	}

	if (m_pDemoController)
	{
		m_pDemoController->OnUpdate(deltaMs);
	}

	//Send out a tick to listeners.
	shared_ptr<EvtData_Update_Tick> pTickEvent(new EvtData_Update_Tick(deltaMs));
    IEventManager::Get()->TriggerEvent(pTickEvent);
}

//
// GameDemoHumanView::OnAttach				- Chapter 19, page 731
//
void GameDemoHumanView::OnAttach(GameViewId vid, ActorId aid)
{
	HumanView::OnAttach(vid, aid);
}

bool GameDemoHumanView::LoadGameDelegate(TiXmlElement* pLevelData)
{
	if (!HumanView::LoadGameDelegate(pLevelData))
		return false;

    m_StandardHUD.reset(new StandardHUD);
	m_StandardHUD->OnInit();
    PushElement(m_StandardHUD);

    // A movement controller is going to control the camera, 
    // but it could be constructed with any of the objects you see in this
    // function. You can have your very own remote controlled sphere. What fun...
    m_pFreeCameraController.reset(new MovementController(m_pCamera, 0, 0, false));

    m_pScene->OnRestore();
    return true;
}

void GameDemoHumanView::SetControlledActor(ActorId actorId)
{ 
	m_pTeapot = m_pScene->GetSceneNode(actorId);
    if (!m_pTeapot)
    {
        GE_ERROR("Invalid teapot");
        return;
    }

	HumanView::SetControlledActor(actorId);

    m_pDemoController.reset(new DemoController(m_pTeapot));
    m_KeyboardHandler = m_pDemoController;
    m_MouseHandler = m_pDemoController;
    m_pCamera->SetTarget(m_pTeapot);
    //m_pTeapot->SetAlpha(0.8f);
}

void GameDemoHumanView::GameplayUiUpdateDelegate(BaseEventDataPtr pEventData)
{
    shared_ptr<EvtData_Gameplay_UI_Update> pCastEventData = 
		static_pointer_cast<EvtData_Gameplay_UI_Update>(pEventData);
    if (!pCastEventData->GetUiString().empty())
        m_gameplayText = pCastEventData->GetUiString();
    else
        m_gameplayText.clear();
}

void GameDemoHumanView::SetControlledActorDelegate(BaseEventDataPtr pEventData)
{
    shared_ptr<EvtData_Set_Controlled_Actor> pCastEventData = 
		static_pointer_cast<EvtData_Set_Controlled_Actor>(pEventData);
    //SetControlledActor(pCastEventData->GetActorId());

	HumanView::SetControlledActor(m_pFreeCameraController->GetTarget()->Get()->ActorId());
	m_KeyboardHandler = m_pFreeCameraController;
	m_MouseHandler = m_pFreeCameraController;

}

void GameDemoHumanView::RegisterAllDelegates(void)
{
	// [mrmike] Move, New, and Destroy actor events are now handled by the HumanView, as are the PlaySound and GameState events

    IEventManager* pGlobalEventManager = IEventManager::Get();
    pGlobalEventManager->AddListener(MakeDelegate(this, &DemosHumanView::GameplayUiUpdateDelegate), EvtData_Gameplay_UI_Update::sk_EventType);
    pGlobalEventManager->AddListener(MakeDelegate(this, &DemosHumanView::SetControlledActorDelegate), EvtData_Set_Controlled_Actor::sk_EventType);
}

void GameDemoHumanView::RemoveAllDelegates(void)
{
    IEventManager* pGlobalEventManager = IEventManager::Get();
    pGlobalEventManager->RemoveListener(MakeDelegate(this, &DemosHumanView::GameplayUiUpdateDelegate), EvtData_Gameplay_UI_Update::sk_EventType);
    pGlobalEventManager->RemoveListener(MakeDelegate(this, &DemosHumanView::SetControlledActorDelegate), EvtData_Set_Controlled_Actor::sk_EventType);
}

///////////////////////////////////////////////////////////////////////////////
//
// AITeapotView::AITeapotView					- Chapter 19, page 737
//
AITeapotView::AITeapotView(eastl::shared_ptr<PathingGraph> pPathingGraph) 
	: GameView(), mPathingGraph(pPathingGraph)
{
    //
}

//
// AITeapotView::~AITeapotView					- Chapter 19, page 737
//
AITeapotView::~AITeapotView(void)
{
    LogInformation("AI Destroying AITeapotView");
}