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

#include "GameDemo.h"
#include "GameDemoApp.h"
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

#define CID_DEMO_WINDOW					(1)
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

	// set a nicer font
	const eastl::shared_ptr<BaseUIFont>& font = GetFont("fontlucida.png");
	if (font) GetSkin()->SetFont(font);

	GetSkin()->SetColor(DC_BUTTON_TEXT, 
		eastl::array<float, 4U>{240/ 255.f, 170 / 255.f, 170 / 255.f, 170 / 255.f});
	GetSkin()->SetColor(DC_3D_HIGH_LIGHT, 
		eastl::array<float, 4U>{240 / 255.f, 34 / 255.f, 34 / 255.f, 34 / 255.f});
	GetSkin()->SetColor(DC_3D_FACE, 
		eastl::array<float, 4U>{240 / 255.f, 68 / 255.f, 68 / 255.f, 68 / 255.f});
	GetSkin()->SetColor(DC_EDITABLE, 
		eastl::array<float, 4U>{240 / 255.f, 68 / 255.f, 68 / 255.f, 68 / 255.f});
	GetSkin()->SetColor(DC_FOCUSED_EDITABLE, 
		eastl::array<float, 4U>{240 / 255.f, 84 / 255.f, 84 / 255.f, 84 / 255.f});
	GetSkin()->SetColor(DC_WINDOW, 
		eastl::array<float, 4U>{240 / 255.f, 102 / 255.f, 102 / 255.f, 102 / 255.f});

	//gui size
	Renderer* renderer = Renderer::Get();
	Vector2<unsigned int> screenSize(renderer->GetScreenSize());
	RectangleBase<2, int> screenRectangle;
	screenRectangle.extent[0] = (int)screenSize[0];
	screenRectangle.extent[1] = (int)screenSize[1];

	eastl::shared_ptr<BaseUIWindow> window = AddWindow(
		screenRectangle, false, L"Quake3 Explorer", 0, CID_DEMO_WINDOW);
	window->SetToolTipText(L"Quake3Explorer. Loads and show various BSP File Format and Shaders.");
	window->GetCloseButton()->SetToolTipText(L"Quit Quake3 Explorer");

	// add a status line help text
	RectangleBase<2, int> statusRectangle;
	statusRectangle.extent[0] = (int)screenSize[0];
	statusRectangle.extent[1] = 50;
	mStatusLine = AddStaticText(L"", statusRectangle, false, false, window, -1, true);

	/*
	UIEngine::Init(this, NULL);
	UIEngine::AddLoadingIcon(
		Renderer::Get()->GetTexture("art/gui/notes.png").get() );
	//System::Get()->OnPause(1000);
	UIEngine::AddLoadingIcon( 
		Renderer::Get()->GetTexture("art/gui/cup_gold.png").get() );
	//System::Get()->OnPause(1000);
    UIEngine::AddLoadingIcon( 
		Renderer::Get()->GetTexture("art/gui/options_video.png").get() );
	//System::Get()->OnPause(1000);
    UIEngine::AddLoadingIcon( 
		Renderer::Get()->GetTexture("art/gui/gui_lock.png").get() );
	//System::Get()->OnPause(1000);
    UIEngine::AddLoadingIcon( 
		Renderer::Get()->GetTexture("art/gui/gift.png").get() );
	//System::Get()->OnPause(1000);
    UIEngine::AddLoadingIcon( 
		Renderer::Get()->GetTexture("art/gui/banana.png").get() );
	//System::Get()->OnPause(1000);

    if(!UserConfigParams::m_no_start_screen)
    {
        ScreenStateManager::Get()->PushScreen(ScreenMainMenu::GetInstance());
	}

	int iY = 10;
	int iX = 35;
	int iX2 = g_SampleUIWidth / 2;
	int width = (g_SampleUIWidth / 2) - 10;
	int height = 25;
	int lineHeight = height + 2;

	// grab defaults from the game options.
	m_NumAIs = g_pApp->m_Options.m_numAIs;
	m_NumPlayers = g_pApp->m_Options.m_expectedPlayers;
	m_HostName = g_pApp->m_Options.m_gameHost;
	m_HostListenPort = g_pApp->m_Options.m_listenPort;
	m_ClientAttachPort = g_pApp->m_Options.m_listenPort;

	m_bCreatingGame = true;

	D3DCOLOR color = 0x50505050;
	m_SampleUI.SetBackgroundColors(color);

	m_SampleUI.SetFont(0, L"Ariel", height, 0);

	m_SampleUI.AddStatic(0, L"Teapot Wars Main Menu", iX - 20, iY, g_SampleUIWidth, height * 2);
	iY += (lineHeight * 3);

	m_SampleUI.AddRadioButton(CID_CREATE_GAME_RADIO, 1, L"Create Game", iX, iY, g_SampleUIWidth, height);
	iY += lineHeight;

	m_SampleUI.AddStatic(CID_LEVEL_LABEL, L"Level", iX, iY, width, height);
	m_SampleUI.AddListBox(CID_LEVEL_LISTBOX, iX2, iY, width, lineHeight * 5);
	std::vector<Level> levels = g_pApp->GetGameLogic()->GetLevelManager()->GetLevels();
	m_Levels.reserve(levels.size());
	int count = 0;
	for (std::vector<Level>::iterator i = levels.begin(); i != levels.end(); ++i, ++count)
	{
		m_Levels.push_back(s2ws(*i));
		m_SampleUI.GetListBox(CID_LEVEL_LISTBOX)->AddItem(m_Levels[count].c_str(), NULL);
	}
	iY += (lineHeight * 5);
	//m_SampleUI.GetListBox(CID_LEVEL_LISTBOX)->GetElement(0)->SetFont(0, 0x0);

	m_SampleUI.AddStatic(CID_NUM_AI_LABEL, L"", iX, iY, width, height);
	m_SampleUI.AddSlider(CID_NUM_AI_SLIDER, iX2, iY, width, height);
	m_SampleUI.GetSlider(CID_NUM_AI_SLIDER)->SetRange(0, g_pApp->m_Options.m_maxAIs);
	m_SampleUI.GetSlider(CID_NUM_AI_SLIDER)->SetValue(m_NumAIs); // should be ai options default
	iY += lineHeight;

	m_SampleUI.AddStatic(CID_NUM_PLAYER_LABEL, L"", iX, iY, width, height);
	m_SampleUI.AddSlider(CID_NUM_PLAYER_SLIDER, iX2, iY, width, height);
	m_SampleUI.GetSlider(CID_NUM_PLAYER_SLIDER)->SetRange(1, g_pApp->m_Options.m_maxPlayers);
	m_SampleUI.GetSlider(CID_NUM_PLAYER_SLIDER)->SetValue(m_NumPlayers);  // should be player options default
	iY += lineHeight;

	m_SampleUI.AddStatic(CID_HOST_LISTEN_PORT_LABEL, L"Host Listen Port", iX, iY, width, height);
	m_SampleUI.AddEditBox(CID_HOST_LISTEN_PORT, L"57", iX2, iY, width, height * 2);
	CDXUTEditBox *eb = m_SampleUI.GetEditBox(CID_HOST_LISTEN_PORT);
	eb->SetVisible(false);
	iY += lineHeight * 3;

	m_SampleUI.AddRadioButton(CID_JOIN_GAME_RADIO, 1, L"Join Game", iX, iY, width, height);
	m_SampleUI.GetRadioButton(CID_JOIN_GAME_RADIO)->SetChecked(true);
	iY += lineHeight;

	m_SampleUI.AddStatic(CID_CLIENT_ATTACH_PORT_LABEL, L"Host Attach Port", iX, iY, width, height);
	m_SampleUI.AddEditBox(CID_CLIENT_ATTACH_PORT, L"57", iX2, iY, width, height * 2);
	iY += lineHeight * 3;


	m_SampleUI.AddStatic(CID_HOST_NAME_LABEL, L"Host Name", iX, iY, width, height);
	m_SampleUI.AddEditBox(CID_HOST_NAME, L"sunshine", iX2, iY, width, height * 2);
	iY += lineHeight;

	m_SampleUI.AddButton(CID_START_BUTTON, L"Start Game", (g_SampleUIWidth - (width / 2)) / 2, iY += lineHeight, width / 2, height);

	m_SampleUI.GetRadioButton(CID_CREATE_GAME_RADIO)->SetChecked(true);

	Set();
	*/
	SetUIActive(1);
	return true;
}


// enable GUI elements
void MainMenuUI::SetUIActive(int command)
{
	bool inputState = false;

	//ICameraSceneNode * camera = Game->Device->getSceneManager()->getActiveCamera ();
	int guiActive = -1;
	switch (command)
	{
		case 0: guiActive = 0; inputState = !guiActive; break;
		case 1: guiActive = 1; inputState = !guiActive; break;
		case 2: guiActive ^= 1; inputState = !guiActive; break;
		case 3:
			//	if ( camera )
			//		inputState = !camera->isInputReceiverEnabled();
		break;
	}
	/*
	if ( camera )
	{
	camera->setInputReceiverEnabled ( inputState );
	game->Device->getCursorControl()->setVisible( !inputState );
	}
	*/
	const eastl::shared_ptr<BaseUIElement>& root = GetRootUIElement();
	const eastl::shared_ptr<BaseUIElement>& window = root->GetElementFromId(CID_DEMO_WINDOW);
	if (window)
		window->SetVisible(guiActive != 0);
	/*
	IGUITreeView* sceneTree = (IGUITreeView*)root->GetElementFromId(CID_SCENETREE_VIEW).get();
	if (guiActive && sceneTree && GetFocus().get() != sceneTree)
	{
		sceneTree->GetRoot()->ClearChildren();
		AddSceneTreeItem(
			g_DemosApp.GetHumanView()->m_pScene->GetRootSceneNode().get(), sceneTree->GetRoot().get());
	}
	*/
	SetFocus(guiActive ? window : 0);
}

void MainMenuUI::Set()
{
	/*
	WCHAR buffer[256];
	CHAR ansiBuffer[256];

	m_LevelIndex = m_SampleUI.GetListBox(CID_LEVEL_LISTBOX)->GetSelectedIndex();
	m_SampleUI.GetListBox(CID_LEVEL_LISTBOX)->SetVisible(m_bCreatingGame);

	m_NumAIs = m_SampleUI.GetSlider(CID_NUM_AI_SLIDER)->GetValue();
	m_SampleUI.GetSlider(CID_NUM_AI_SLIDER)->SetVisible(m_bCreatingGame);

	wsprintf(buffer, _T("%s: %d\n"), L"Number of AIs", m_NumAIs);
	m_SampleUI.GetStatic(CID_NUM_AI_LABEL)->SetText(buffer);
	m_SampleUI.GetStatic(CID_NUM_AI_LABEL)->SetVisible(m_bCreatingGame);

	m_NumPlayers = m_SampleUI.GetSlider(CID_NUM_PLAYER_SLIDER)->GetValue();
	m_SampleUI.GetSlider(CID_NUM_PLAYER_SLIDER)->SetVisible(m_bCreatingGame);
	wsprintf(buffer, _T("%s: %d\n"), L"Number of Players", m_NumPlayers);
	m_SampleUI.GetStatic(CID_NUM_PLAYER_LABEL)->SetText(buffer);
	m_SampleUI.GetStatic(CID_NUM_PLAYER_LABEL)->SetVisible(m_bCreatingGame);

	m_SampleUI.GetStatic(CID_HOST_LISTEN_PORT_LABEL)->SetVisible(m_NumPlayers>1 && m_bCreatingGame);
	m_SampleUI.GetEditBox(CID_HOST_LISTEN_PORT)->SetVisible(m_NumPlayers>1 && m_bCreatingGame);
	if (m_bCreatingGame)
	{
		WideToAnsiCch(ansiBuffer, m_SampleUI.GetEditBox(CID_HOST_LISTEN_PORT)->GetText(), 256);
		m_HostListenPort = ansiBuffer;
	}

	m_SampleUI.GetStatic(CID_HOST_NAME_LABEL)->SetVisible(!m_bCreatingGame);
	m_SampleUI.GetEditBox(CID_HOST_NAME)->SetVisible(!m_bCreatingGame);

	WideToAnsiCch(ansiBuffer, m_SampleUI.GetEditBox(CID_HOST_NAME)->GetText(), 256);
	m_HostName = ansiBuffer;

	m_SampleUI.GetStatic(CID_CLIENT_ATTACH_PORT_LABEL)->SetVisible(!m_bCreatingGame);
	m_SampleUI.GetEditBox(CID_CLIENT_ATTACH_PORT)->SetVisible(!m_bCreatingGame);
	if (!m_bCreatingGame)
	{
		WideToAnsiCch(ansiBuffer, m_SampleUI.GetEditBox(CID_CLIENT_ATTACH_PORT)->GetText(), 256);
		m_ClientAttachPort = ansiBuffer;
	}
	*/
}

bool MainMenuUI::OnRestore()
{
    //mSampleUI.SetLocation( 
	//	(System::Get()->GetScreenSize().x - SampleUIWidth)/2, 
	//	(System::Get()->GetScreenSize().y - SampleUIHeight) / 2  );
    //mSampleUI.SetSize( SampleUIWidth, SampleUIHeight );
	return true;
}

bool MainMenuUI::OnRender(double time, float elapsedTime)
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

		//float dt = gameApp->GetLimitedDt();
		//UIEngine::Update(dt);
		//UIEngine::Render(dt);

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
	/*
	HRESULT hr;
	DXUT_BeginPerfEvent(DXUT_PERFEVENTCOLOR, L"TeapotWarsHUD"); // These events are to help PIX identify what the code is doing
	V(m_SampleUI.OnRender(elapsedTime));
	DXUT_EndPerfEvent();
	return S_OK;
	*/
	wchar_t msg[128];
	swprintf(msg, 128,
		L"%03f fps, F1 GUI on/off, F2 respawn, F3-F6 toggle Nodes, F7 Collision on/off"
		L", F8 Gravity on/off, Right Mouse Toggle GUI", elapsedTime);
	if (mStatusLine)
		mStatusLine->SetText(msg);

	return BaseUI::OnRender(time, elapsedTime);
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
	/*
    if (evt.mEventType == ET_UI_EVENT)
    {
		return UIEventHandler::Get()->OnEvent(evt);
    }

	switch (nControlID)
	{
		case CID_CREATE_GAME_RADIO:
		{
			m_bCreatingGame = true;
			break;
		}

		case CID_JOIN_GAME_RADIO:
		{
			m_bCreatingGame = false;
			break;
		}

		case CID_LEVEL_LISTBOX:
		case CID_NUM_AI_SLIDER:
		case CID_NUM_PLAYER_SLIDER:
		case CID_HOST_LISTEN_PORT:
		case CID_CLIENT_ATTACH_PORT:
		case CID_HOST_NAME:
		{
			break;
		}

		case CID_START_BUTTON:
		{
			g_pApp->m_Options.m_numAIs = m_NumAIs;
			g_pApp->m_Options.m_expectedPlayers = m_NumPlayers;
			if (m_bCreatingGame)
			{
				if (m_LevelIndex == -1)
				{
					// FUTURE WORK - AN ERROR DIALOG WOULD BE GOOD HERE, OR JUST DEFALT THE SELECTION TO SOMETHING VALID
					return;
				}
				g_pApp->m_Options.m_Level = ws2s(m_Levels[m_LevelIndex]);
				g_pApp->m_Options.m_gameHost = "";
				g_pApp->m_Options.m_listenPort = atoi(m_HostListenPort.c_str());
			}
			else
			{
				g_pApp->m_Options.m_gameHost = m_HostName;
				g_pApp->m_Options.m_listenPort = atoi(m_ClientAttachPort.c_str());
			}

			VSetVisible(false);

			shared_ptr<EvtData_Request_Start_Game> pRequestStartGameEvent(GCC_NEW EvtData_Request_Start_Game());
			IEventManager::Get()->VQueueEvent(pRequestStartGameEvent);

			break;
		}

		default:
		{
			GCC_ERROR("Unknown control.");
		}
	}
	*/
	return BaseUI::OnEvent(evt);
}


MainMenuView::MainMenuView() : HumanView()
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


void MainMenuView::OnUpdate(unsigned long deltaTime)
{
	HumanView::OnUpdate( deltaTime );
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
	/*
	// Initialize dialogs
	m_HUD.Init(&D3DRenderer::g_DialogResourceManager);
	m_HUD.SetCallback(OnGUIEvent); int iY = 10;
	m_HUD.AddButton(IDC_TOGGLEFULLSCREEN, L"Toggle full screen", 35, iY, 125, 22);
	m_HUD.AddButton(IDC_TOGGLEREF, L"Toggle REF (F3)", 35, iY += 24, 125, 22);
	//m_HUD.AddButton( IDC_CHANGEDEVICE, L"Change device (F2)", 35, iY += 24, 125, 22 );
	*/
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
	
	eastl::shared_ptr<BaseUIFont> font(GetFont("fontcourier.bmp"));
	if (font)
		GetSkin()->SetFont(font);

    // Initialize dialogs
	//mHUD.Init( &D3DRenderer::DialogResourceManager );
    //mHUD.SetCallback( OnUIEvent ); 
	int iY = 10; 
	AddButton(RectangleBase<int>(35, iY, 160, iY + 22), 0, IDC_TOGGLEFULLSCREEN, L"Toggle full screen");
	iY += 24;
	AddButton(RectangleBase<int>(35, iY, 160, iY + 22), 0, IDC_TOGGLEREF, L"Toggle REF (F3)");
    //mHUD.AddButton( IDC_CHANGEDEVICE, L"Change device (F2)", 35, iY += 24, 125, 22 );
	*/
	return true;
}

bool StandardHUD::OnRestore()
{
	/*
    mHUD.SetLocation( System::Get()->GetScreenSize().x - 170, 0 );
    mHUD.SetSize( 170, 170 );
	*/
	return BaseUI::OnRestore();
}

bool StandardHUD::OnRender(double time, float elapsedTime)
{
	/*
	HRESULT hr;
	DXUT_BeginPerfEvent( DXUT_PERFEVENTCOLOR, L"StandardUI" ); // These events are to help PIX identify what the code is doing
	V( mHUD.OnRender( elapsedTime ) );
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
			case UIEVT_BUTTON_CLICKED:
				switch (id)
				{
				case IDC_TOGGLEFULLSCREEN: 
					System::Get()->SwitchToFullScreen(); break;
				case IDC_TOGGLEREF:        
					System::Get()->SetResizable(); break;
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
GameDemoHumanView::GameDemoHumanView() 
	: HumanView()
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
						//mStandardHUD->SetVisible(m_bShowUI);
						return true;
					}

					case KEY_KEY_2:
					{
						// test the picking API
						/*
						POINT ptCursor;
						GetCursorPos( &ptCursor );
						ScreenToClient( System::Get()->GetHwnd(), &ptCursor );

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
					}
					break;

					case KEY_KEY_4:
					{
						BaseResource resource(L"scripts\\test.lua");
						// this actually loads the Lua file from the zip file
						eastl::shared_ptr<ResHandle> pResourceHandle = 
							ResCache::Get()->GetHandle(&resource);
					}
					break;

					case KEY_KEY_5:
					{

					}
					break;

					case KEY_KEY_6:
					{

					}
					break;

					case KEY_KEY_7:
					{
						GameDemoLogic* twg = static_cast<GameDemoLogic *>(GameLogic::Get());
						twg->ToggleRenderDiagnostics();
					}	
					break;

					case KEY_KEY_8:
					{
						mKeyboardHandler = mGameDemoController;
						mMouseHandler = mGameDemoController;
						//mCamera->SetTarget(mTeapot);
						//mTeapot->SetAlpha(0.8f);
						//ReleaseCapture();
						return true;
					}

					case KEY_KEY_9:
					{
						//mKeyboardHandler = mFreeCameraController;
						//mMouseHandler = mFreeCameraController;
						//mCamera->ClearTarget();
						//mTeapot->SetAlpha(fOPAQUE);
						//SetCapture((HWND)System::Get()->GetID());
						return true;
					}

					case KEY_ESCAPE:
						GameApplication* gameApp = (GameApplication*)Application::App;
						gameApp->SetQuitting(true);
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
	D3DRenderer::TextHelper->Begin();

    // Gameplay UI (with shadow)....
    if (!mGameplayText.empty())
    {
	    D3DRenderer::TextHelper->SetInsertionPos( Renderer::Get()->GetScreenSize().x/2, 5 );
	    D3DRenderer::TextHelper->SetForegroundColor( D3DXCOLOR( 0.0f, 0.0f, 0.0f, 1.0f ) );
	    D3DRenderer::TextHelper->DrawTextLine(mGameplayText.c_str());
	    D3DRenderer::TextHelper->SetInsertionPos( Renderer::Get()->GetScreenSize().x/2-1, 5-1 );
	    D3DRenderer::TextHelper->SetForegroundColor( D3DXCOLOR( 0.25f, 1.0f, 0.25f, 1.0f ) );
	    D3DRenderer::TextHelper->DrawTextLine(mGameplayText.c_str());
    }
	// ...Gameplay UI

	if( mIsShowUI )
	{
		// Output statistics...
		D3DRenderer::TextHelper->SetInsertionPos( 5, 5 );
		D3DRenderer::TextHelper->SetForegroundColor( D3DXCOLOR( 1.0f, 1.0f, 0.0f, 1.0f ) );
		D3DRenderer::TextHelper->DrawTextLine( DXUTGetFrameStats() );
		D3DRenderer::TextHelper->DrawTextLine( DXUTGetDeviceStats() );
		//...output statistics
		
		D3DRenderer::TextHelper->SetForegroundColor( D3DXCOLOR( 0.0f, 0.0f, 0.0f, 0.5f ) );

		//Game State...
		switch (mBaseGameState)
		{
			case BGS_INITIALIZING:
				D3DRenderer::TextHelper->DrawTextLine(gameApp->GetString(_T("IDS_INITIALIZING")).c_str());
				break;

			case BGS_MAINMENU:
				D3DRenderer::TextHelper->DrawTextLine(L"Main Menu");
				break;

//			case BGS_SPAWNAI:
//				D3DRenderer::TextHelper->DrawTextLine(L"Spawn AI");
//				break;

			case BGS_WAITINGFORPLAYERS:
				D3DRenderer::TextHelper->DrawTextLine(gameApp->GetString(_T("IDS_WAITING")).c_str());
				break;

			case BGS_LOADINGGAMEENVIRONMENT:
				D3DRenderer::TextHelper->DrawTextLine(gameApp->GetString(_T("IDS_LOADING")).c_str());
				break;

			case BGS_RUNNING:
#ifndef DISABLE_PHYSICS
				D3DRenderer::TextHelper->DrawTextLine(gameApp->GetString(_T("IDS_RUNNING")).c_str());
#else
				D3DRenderer::TextHelper->DrawTextLine(gameApp->GetString(_T("IDS_NOPHYSICS")).c_str());
#endif //!DISABLE_PHYSICS
				break;
		}
		//...Game State

		//Camera...
		fschar_t buffer[256];
		const fschar_t *s = NULL;
		matrix4 toWorld, fromWorld;
		if (mCamera)
		{	
			mCamera->Get()->Transform(&toWorld, &fromWorld);
		}
		swprintf(buffer, gameApp->GetString(_T("IDS_CAMERA_LOCATION")).c_str(), toWorld.m[3][0], toWorld.m[3][1], toWorld.m[3][2]);
		D3DRenderer::TextHelper->DrawTextLine( buffer );
		//...Camera

		//Help text.  Right justified, lower right of screen.
		RECT helpRect;
		helpRect.left = 0;
		helpRect.right = gameApp->GetScreenSize().x - 10;
		helpRect.top = gameApp->GetScreenSize().y - 15*8;
		helpRect.bottom = gameApp->GetScreenSize().y;
		D3DRenderer::TextHelper->SetInsertionPos( helpRect.right, helpRect.top );
		D3DRenderer::TextHelper->SetForegroundColor( D3DXCOLOR( 1.0f, 0.75f, 0.0f, 1.0f ) );
		D3DRenderer::TextHelper->DrawTextLine( helpRect, DT_RIGHT, g_pApp->GetString(_T("IDS_CONTROLS_HEADER")).c_str() );
		helpRect.top = gameApp->GetScreenSize().y-15*7;
		D3DRenderer::TextHelper->DrawTextLine( helpRect, DT_RIGHT, g_pApp->GetString(_T("IDS_CONTROLS")).c_str() );
		//...Help
	}//end if (m_bShowUI)

	D3DRenderer::TextHelper->End();
	*/
}


//
// GameDemoHumanView::OnUpdate				- Chapter 19, page 730
//
void GameDemoHumanView::OnUpdate(unsigned long deltaTime)
{
	HumanView::OnUpdate( deltaTime );

	if (mFreeCameraController)
	{
		//mFreeCameraController->OnUpdate(deltaMs);
	}

	if (mGameDemoController)
	{
		mGameDemoController->OnUpdate(deltaTime);
	}

	//Send out a tick to listeners.
	eastl::shared_ptr<EventDataUpdateTick> pTickEvent(new EventDataUpdateTick(deltaTime));
    BaseEventManager::Get()->TriggerEvent(pTickEvent);
}

//
// GameDemoHumanView::OnAttach				- Chapter 19, page 731
//
void GameDemoHumanView::OnAttach(GameViewId vid, ActorId aid)
{
	HumanView::OnAttach(vid, aid);
}

bool GameDemoHumanView::LoadGameDelegate(XMLElement* pLevelData)
{
	if (!HumanView::LoadGameDelegate(pLevelData))
		return false;

    mStandardHUD.reset(new StandardHUD);
	mStandardHUD->OnInit();
    PushElement(mStandardHUD);

    // A movement controller is going to control the camera, 
    // but it could be constructed with any of the objects you see in this
    // function. You can have your very own remote controlled sphere. What fun...
    //mFreeCameraController.reset(new MovementController(mCamera, 0, 0, false));

    mScene->OnRestore();
    return true;
}

void GameDemoHumanView::SetControlledActor(ActorId actorId)
{ 
	mTeapot = mScene->GetSceneNode(actorId);
    if (!mTeapot)
    {
        LogError("Invalid teapot");
        return;
    }

	HumanView::SetControlledActor(actorId);

    mGameDemoController.reset(new GameDemoController(mTeapot));
    mKeyboardHandler = mGameDemoController;
    mMouseHandler = mGameDemoController;
    //mCamera->SetTarget(mTeapot);
    //mTeapot->SetAlpha(0.8f);
}

void GameDemoHumanView::GameplayUiUpdateDelegate(BaseEventDataPtr pEventData)
{
    eastl::shared_ptr<EventDataGameplayUIUpdate> pCastEventData = 
		eastl::static_pointer_cast<EventDataGameplayUIUpdate>(pEventData);
    if (!pCastEventData->GetUiString().empty())
        mGameplayText = pCastEventData->GetUiString();
    else
		mGameplayText.clear();
}

void GameDemoHumanView::SetControlledActorDelegate(BaseEventDataPtr pEventData)
{
    eastl::shared_ptr<EventDataSetControlledActor> pCastEventData = 
		eastl::static_pointer_cast<EventDataSetControlledActor>(pEventData);
    SetControlledActor(pCastEventData->GetActorId());

	//HumanView::SetControlledActor(mFreeCameraController->GetTarget()->Get()->ActorId());
	//mKeyboardHandler = mFreeCameraController;
	//mMouseHandler = mFreeCameraController;

}

void GameDemoHumanView::RegisterAllDelegates(void)
{
	// [mrmike] Move, New, and Destroy actor events are now handled by the HumanView, as are the PlaySound and GameState events

    BaseEventManager* pGlobalEventManager = BaseEventManager::Get();
    pGlobalEventManager->AddListener(
		MakeDelegate(this, &GameDemoHumanView::GameplayUiUpdateDelegate), 
		EventDataGameplayUIUpdate::skEventType);
    pGlobalEventManager->AddListener(
		MakeDelegate(this, &GameDemoHumanView::SetControlledActorDelegate), 
		EventDataSetControlledActor::skEventType);
}

void GameDemoHumanView::RemoveAllDelegates(void)
{
	BaseEventManager* pGlobalEventManager = BaseEventManager::Get();
    pGlobalEventManager->RemoveListener(
		MakeDelegate(this, &GameDemoHumanView::GameplayUiUpdateDelegate), 
		EventDataGameplayUIUpdate::skEventType);
    pGlobalEventManager->RemoveListener(
		MakeDelegate(this, &GameDemoHumanView::SetControlledActorDelegate), 
		EventDataSetControlledActor::skEventType);
}

///////////////////////////////////////////////////////////////////////////////
//
// AITeapotView::AITeapotView					- Chapter 19, page 737
//
AITeapotView::AITeapotView(eastl::shared_ptr<PathingGraph> pPathingGraph) 
	: BaseGameView(), mPathingGraph(pPathingGraph)
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