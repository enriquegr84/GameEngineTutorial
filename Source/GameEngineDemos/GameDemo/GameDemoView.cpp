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
#include "Graphic/Image/ImageResource.h"

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

#include "Game/Actor/Actor.h"
#include "Game/Actor/RenderComponent.h"

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
#define CID_SET_GAME_RADIO				(3)
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
#define CID_LEVEL_LISTBOX				(16)
#define CID_STATUS_LABEL				(17)


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

	GameApplication* gameApp = (GameApplication*)Application::App;
	System* system = System::Get();
	system->GetCursorControl()->SetVisible(true);

	// set a nicer font
	const eastl::shared_ptr<BaseUIFont>& font = GetFont(L"DefaultFont");
	if (font) GetSkin()->SetFont(font);

	GetSkin()->SetColor(DC_BUTTON_TEXT, 
		eastl::array<float, 4U>{170 / 255.f, 170 / 255.f, 170 / 255.f, 240 / 255.f});
	GetSkin()->SetColor(DC_3D_HIGH_LIGHT, 
		eastl::array<float, 4U>{34 / 255.f, 34 / 255.f, 34 / 255.f, 240 / 255.f});
	GetSkin()->SetColor(DC_3D_FACE, 
		eastl::array<float, 4U>{68 / 255.f, 68 / 255.f, 68 / 255.f, 240 / 255.f});
	GetSkin()->SetColor(DC_EDITABLE, 
		eastl::array<float, 4U>{68 / 255.f, 68 / 255.f, 68 / 255.f, 240 / 255.f});
	GetSkin()->SetColor(DC_FOCUSED_EDITABLE, 
		eastl::array<float, 4U>{84 / 255.f, 84 / 255.f, 84 / 255.f, 240 / 255.f});
	GetSkin()->SetColor(DC_WINDOW, 
		eastl::array<float, 4U>{102 / 255.f, 102 / 255.f, 102 / 255.f, 240 / 255.f});

	//gui size
	Renderer* renderer = Renderer::Get();
	Vector2<unsigned int> screenSize(renderer->GetScreenSize());
	RectangleBase<2, int> screenRectangle;
	screenRectangle.center[0] = screenSize[0] / 2;
	screenRectangle.center[1] = screenSize[1] / 2;
	screenRectangle.extent[0] = (int)screenSize[0];
	screenRectangle.extent[1] = (int)screenSize[1];

	eastl::shared_ptr<BaseUIWindow> window = AddWindow(
		screenRectangle, false, L"Teapot Wars", 0, CID_DEMO_WINDOW);
	window->GetCloseButton()->SetToolTipText(L"Quit Teapot Wars");

	// add a options line
	RectangleBase<2, int> playerOptionsRectangle;
	playerOptionsRectangle.center[0] = 50;
	playerOptionsRectangle.extent[0] = 90;
	playerOptionsRectangle.center[1] = 42;
	playerOptionsRectangle.extent[1] = 16;
	eastl::shared_ptr<BaseUIStaticText> playerOptionsLine =
		AddStaticText(L"AI Player:", playerOptionsRectangle, false, false, window, CID_NUM_AI_LABEL, true);
	playerOptionsLine->SetTextAlignment(UIA_UPPERLEFT, UIA_CENTER);

	playerOptionsRectangle.center[0] = 250;
	playerOptionsRectangle.extent[0] = 250;
	playerOptionsRectangle.center[1] = 40;
	playerOptionsRectangle.extent[1] = 20;
	eastl::shared_ptr<BaseUIScrollBar> gameAI = 
		AddScrollBar(true, playerOptionsRectangle, window, CID_NUM_AI_SLIDER);
	gameAI->SetMin(0);
	gameAI->SetMax(gameApp->mOption.mMaxAIs);
	gameAI->SetSmallStep(1);
	gameAI->SetLargeStep(1);
	gameAI->SetPos(gameApp->mOption.mNumAIs);
	gameAI->SetToolTipText(L"Set the AI players");

	playerOptionsRectangle.center[0] = 50;
	playerOptionsRectangle.extent[0] = 90;
	playerOptionsRectangle.center[1] = 82;
	playerOptionsRectangle.extent[1] = 16;
	playerOptionsLine =
		AddStaticText(L"Human Player:", playerOptionsRectangle, false, false, window, CID_NUM_PLAYER_LABEL, false);
	playerOptionsLine->SetTextAlignment(UIA_UPPERLEFT, UIA_CENTER);

	playerOptionsRectangle.center[0] = 250;
	playerOptionsRectangle.extent[0] = 250;
	playerOptionsRectangle.center[1] = 80;
	playerOptionsRectangle.extent[1] = 20;
	eastl::shared_ptr<BaseUIScrollBar> gamePlayer = 
		AddScrollBar(true, playerOptionsRectangle, window, CID_NUM_PLAYER_SLIDER);
	gamePlayer->SetMin(0);
	gamePlayer->SetMax(gameApp->mOption.mMaxPlayers);
	gamePlayer->SetSmallStep(1);
	gamePlayer->SetLargeStep(1);
	gamePlayer->SetPos(gameApp->mOption.mExpectedPlayers);
	gamePlayer->SetToolTipText(L"Set the Human players");

	playerOptionsRectangle.center[0] = 50;
	playerOptionsRectangle.extent[0] = 90;
	playerOptionsRectangle.center[1] = 122;
	playerOptionsRectangle.extent[1] = 16;
	playerOptionsLine =
		AddStaticText(L"Game Host:", playerOptionsRectangle, false, false, window, CID_HOST_NAME_LABEL, false);
	playerOptionsLine->SetTextAlignment(UIA_UPPERLEFT, UIA_CENTER);

	playerOptionsRectangle.center[0] = 220;
	playerOptionsRectangle.extent[0] = 190;
	playerOptionsRectangle.center[1] = 120;
	playerOptionsRectangle.extent[1] = 20;
	eastl::shared_ptr<BaseUIEditBox> gameHost = AddEditBox(
		eastl::wstring(gameApp->mOption.mGameHost.c_str()).c_str(), playerOptionsRectangle, true, window, CID_HOST_NAME);

	playerOptionsRectangle.center[0] = 350;
	playerOptionsRectangle.extent[0] = 50;
	playerOptionsRectangle.center[1] = 120;
	playerOptionsRectangle.extent[1] = 20;
	eastl::shared_ptr<BaseUIButton> gameStart = 
		AddButton(playerOptionsRectangle, window, CID_START_BUTTON, L"Start");
	gameStart->SetToolTipText(L"Start Game");

	// add a status line help text
	RectangleBase<2, int> statusRectangle;
	statusRectangle.center[0] = screenSize[0] / 2 + 5;
	statusRectangle.extent[0] = screenSize[0] - 10;
	statusRectangle.center[1] = screenSize[1] - 20;
	statusRectangle.extent[1] = 20;
	eastl::shared_ptr<BaseUIStaticText> statusLine = AddStaticText(L"", statusRectangle, false, false, window, CID_STATUS_LABEL, true);
	statusLine->SetTextAlignment(UIA_UPPERLEFT, UIA_CENTER);

	RectangleBase<2, int> videoRectangle;
	videoRectangle.center[0] = screenSize[0] - 355;
	videoRectangle.extent[0] = 90;
	videoRectangle.center[1] = 42;
	videoRectangle.extent[1] = 16;
	eastl::shared_ptr<BaseUIStaticText> videoDriverLine = 
		AddStaticText(L"VideoDriver:", videoRectangle, false, false, window, -1, true);
	videoDriverLine->SetTextAlignment(UIA_UPPERLEFT, UIA_CENTER);
	
	videoRectangle.center[0] = screenSize[0] - 155;
	videoRectangle.extent[0] = 290;
	videoRectangle.center[1] = 40;
	videoRectangle.extent[1] = 20;
	eastl::shared_ptr<BaseUIComboBox> videoDriver = AddComboBox(videoRectangle, window);
	videoDriver->AddItem(L"Direct3D 11", RT_DIRECT3D11);
	videoDriver->AddItem(L"OpenGL", RT_OPENGL);
	videoDriver->AddItem(L"Software Renderer", RT_SOFTWARE);
	videoDriver->SetSelected(videoDriver->GetIndexForItemData(gameApp->mOption.mRendererType));
	videoDriver->SetToolTipText(L"Use a VideoDriver");

	videoRectangle.center[0] = screenSize[0] - 355;
	videoRectangle.extent[0] = 90;
	videoRectangle.center[1] = 82;
	videoRectangle.extent[1] = 16;
	eastl::shared_ptr<BaseUIStaticText> videoModeLine =
		AddStaticText(L"VideoMode:", videoRectangle, false, false, window, -1, false);
	videoModeLine->SetTextAlignment(UIA_UPPERLEFT, UIA_CENTER);

	videoRectangle.center[0] = screenSize[0] - 155;
	videoRectangle.extent[0] = 290;
	videoRectangle.center[1] = 80;
	videoRectangle.extent[1] = 20;
	eastl::shared_ptr<BaseUIComboBox> videoMode = AddComboBox(videoRectangle, window);
	eastl::vector<Vector2<unsigned int>> videoResolutions = gameApp->mSystem->GetVideoResolutions();
	for (int i = 0; i != videoResolutions.size(); ++i)
	{
		unsigned int w = videoResolutions[i][0];
		unsigned int h = videoResolutions[i][1];
		unsigned int val = w << 16 | h;

		if (videoMode->GetIndexForItemData(val) >= 0)
			continue;

		float aspect = (float)w / (float)h;
		const wchar_t *a = L"";
		if (Function<float>::Equals(aspect, 1.3333333333f)) a = L"4:3";
		else if (Function<float>::Equals(aspect, 1.6666666f)) a = L"15:9 widescreen";
		else if (Function<float>::Equals(aspect, 1.7777777f)) a = L"16:9 widescreen";
		else if (Function<float>::Equals(aspect, 1.6f)) a = L"16:10 widescreen";
		else if (Function<float>::Equals(aspect, 2.133333f)) a = L"20:9 widescreen";

		wchar_t buf[256];
		swprintf(buf, sizeof(buf), L"%d x %d, %s", w, h, a);
		videoMode->AddItem(buf, val);
	}
	videoMode->SetSelected(videoMode->GetIndexForItemData(
		gameApp->mOption.mScreenSize[0] << 16 | gameApp->mOption.mScreenSize[1]));
	videoMode->SetToolTipText(L"Supported Screenmodes");

	screenRectangle.center[0] = screenSize[0] - 350;
	screenRectangle.extent[0] = 100;
	screenRectangle.center[1] = 120;
	screenRectangle.extent[1] = 20;
	eastl::shared_ptr<BaseUICheckBox> fullScreen = AddCheckBox(
		gameApp->mOption.mFullScreen, screenRectangle, window, -1, L"Fullscreen");
	fullScreen->SetToolTipText(L"Set Fullscreen or Window Mode");

	screenRectangle.center[0] = screenSize[0] - 250;
	screenRectangle.extent[0] = 90;
	screenRectangle.center[1] = 122;
	screenRectangle.extent[1] = 16;
	eastl::shared_ptr<BaseUIStaticText> videoMultiSampleLine =
		AddStaticText(L"MultiSample:", screenRectangle, false, false, window, -1, false);
	videoMultiSampleLine->SetTextAlignment(UIA_UPPERLEFT, UIA_CENTER);

	screenRectangle.center[0] = screenSize[0] - 130;
	screenRectangle.extent[0] = 120;
	screenRectangle.center[1] = 120;
	screenRectangle.extent[1] = 20;
	eastl::shared_ptr<BaseUIScrollBar> multiSample = AddScrollBar(true, screenRectangle, window, -1);
	multiSample->SetMin(0);
	multiSample->SetMax(8);
	multiSample->SetSmallStep(1);
	multiSample->SetLargeStep(1);
	multiSample->SetPos(gameApp->mOption.mAntiAlias);
	multiSample->SetToolTipText(L"Set the multisample (disable, 1x, 2x, 4x, 8x )");

	screenRectangle.center[0] = screenSize[0] - 35;
	screenRectangle.extent[0] = 50;
	screenRectangle.center[1] = 120;
	screenRectangle.extent[1] = 20;
	eastl::shared_ptr<BaseUIButton> setVideoMode = AddButton(screenRectangle, window, CID_SET_GAME_RADIO, L"Set");
	setVideoMode->SetToolTipText(L"Set video mode with current values");

	screenRectangle.center[0] = 50;
	screenRectangle.extent[0] = 90;
	screenRectangle.center[1] = screenSize[1] - 390;
	screenRectangle.extent[1] = 20;
	eastl::shared_ptr<BaseUIStaticText> mapsLine =
		AddStaticText(L"Maps:", screenRectangle, false, false, window, CID_LEVEL_LABEL, false);
	mapsLine->SetTextAlignment(UIA_UPPERLEFT, UIA_CENTER);

	screenRectangle.center[0] = 190;
	screenRectangle.extent[0] = 380;
	screenRectangle.center[1] = screenSize[1] - 210;
	screenRectangle.extent[1] = 340;
	eastl::shared_ptr<BaseUIListBox> maps = AddListBox(screenRectangle, window, CID_LEVEL_LISTBOX, true);
	maps->SetToolTipText(L"Show the current maps.\n Double-Click the map to start the level");

	eastl::vector<Level*> levels = GameLogic::Get()->GetLevelManager()->GetLevels();
	for (eastl::vector<Level*>::iterator it = levels.begin(); it != levels.end(); ++it)
		maps->AddItem((*it)->GetName().c_str());

	// create a visible Scene Tree
	screenRectangle.center[0] = screenSize[0] - 350;
	screenRectangle.extent[0] = 90;
	screenRectangle.center[1] = screenSize[1] - 390;
	screenRectangle.extent[1] = 20;
	eastl::shared_ptr<BaseUIStaticText> sceneGraphLine =
		AddStaticText(L"Scenegraph:", screenRectangle, false, false, window, -1, false);
	sceneGraphLine->SetTextAlignment(UIA_UPPERLEFT, UIA_CENTER);

	screenRectangle.center[0] = screenSize[0] - 200;
	screenRectangle.extent[0] = 400;
	screenRectangle.center[1] = screenSize[1] - 210;
	screenRectangle.extent[1] = 340;
	eastl::shared_ptr<BaseUITreeView> scenes = AddTreeView(screenRectangle, window, -1, true, true, false);
	scenes->SetToolTipText(L"Show the current scenegraph");

	scenes->GetRoot()->ClearChildren();

	/*

	eastl::shared_ptr<BaseUIEditBox> mGameHostPort;

	eastl::shared_ptr<BaseUIScrollBar> mTesselation;

	eastl::shared_ptr<BaseUIImage> mLogo;

	// load the engine logo
	BaseResource resource(L"Art/irrlichtlogo3.png");
	const eastl::shared_ptr<ResHandle>& resHandle = gameApp->mResCache->GetHandle(&resource);
	if (resHandle)
	{
		const eastl::shared_ptr<ImageResourceExtraData>& extra =
		eastl::static_pointer_cast<ImageResourceExtraData>(resHandle->GetExtra());
		extra->GetImage()->AutogenerateMipmaps();

		mLogo = AddImage(extra->GetImage(), Vector2<int>{5, 16}, true);
		mLogo->SetToolTipText(L"The great Irrlicht Engine");
	}
	*/

	//Set();

	SetUIActive(1);
	return true;
}


void MainMenuUI::Set()
{
	const eastl::shared_ptr<BaseUIElement>& root = GetRootUIElement();
	const eastl::shared_ptr<BaseUIElement>& window = root->GetElementFromId(CID_DEMO_WINDOW);
	const eastl::shared_ptr<BaseUIButton>& createGame = 
		eastl::static_pointer_cast<BaseUIButton>(root->GetElementFromId(CID_CREATE_GAME_RADIO));
	const eastl::shared_ptr<BaseUIButton>& setGame = 
		eastl::static_pointer_cast<BaseUIButton>(root->GetElementFromId(CID_SET_GAME_RADIO));
	const eastl::shared_ptr<BaseUIScrollBar>& numAI = 
		eastl::static_pointer_cast<BaseUIScrollBar>(root->GetElementFromId(CID_NUM_AI_SLIDER));
	const eastl::shared_ptr<BaseUIScrollBar>& numPlayer = 
		eastl::static_pointer_cast<BaseUIScrollBar>(root->GetElementFromId(CID_NUM_PLAYER_SLIDER));
	const eastl::shared_ptr<BaseUIEditBox>& hostPort = 
		eastl::static_pointer_cast<BaseUIEditBox>(root->GetElementFromId(CID_HOST_LISTEN_PORT));
	const eastl::shared_ptr<BaseUIEditBox>& clientPort = 
		eastl::static_pointer_cast<BaseUIEditBox>(root->GetElementFromId(CID_CLIENT_ATTACH_PORT));
	const eastl::shared_ptr<BaseUIButton>& startGame = 
		eastl::static_pointer_cast<BaseUIButton>(root->GetElementFromId(CID_START_BUTTON));
	const eastl::shared_ptr<BaseUIEditBox>& hostName = 
		eastl::static_pointer_cast<BaseUIEditBox>(root->GetElementFromId(CID_HOST_NAME));
	const eastl::shared_ptr<BaseUIListBox>& level = 
		eastl::static_pointer_cast<BaseUIListBox>(root->GetElementFromId(CID_LEVEL_LISTBOX));

	GameApplication* gameApp = (GameApplication*)Application::App;
	gameApp->mOption.mNumAIs = numAI->GetPos();
	gameApp->mOption.mExpectedPlayers = numPlayer->GetPos();
	gameApp->mOption.mGameHost = ToString(hostName->GetText());
	gameApp->mOption.mLevel = level->GetSelected();
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
	GameApplication* gameApp = (GameApplication*)Application::App;
	wchar_t msg[128];
	swprintf(msg, 128,
		L"%03d fps, F1 GUI on/off, F2 respawn, F3-F6 toggle Nodes, F7 Collision on/off"
		L", F8 Gravity on/off, Right Mouse Toggle GUI", gameApp->GetFPS());

	const eastl::shared_ptr<BaseUIElement>& root = GetRootUIElement();
	const eastl::shared_ptr<BaseUIStaticText>& statusLabel =
		eastl::static_pointer_cast<BaseUIStaticText>(root->GetElementFromId(CID_STATUS_LABEL));
	if (statusLabel)
		statusLabel->SetText(msg);

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
	if (evt.mEventType == ET_UI_EVENT)
	{
		switch (evt.mUIEvent.mCaller->GetID())
		{
			case CID_CREATE_GAME_RADIO:
			{
				if (evt.mUIEvent.mEventType == UIEVT_BUTTON_CLICKED)
					mCreatingGame = true;
				break;
			}

			case CID_SET_GAME_RADIO:
			{
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
				if (evt.mUIEvent.mEventType == UIEVT_BUTTON_CLICKED)
				{
					SetVisible(false);

					eastl::shared_ptr<EventDataRequestStartGame> pRequestStartGameEvent(new EventDataRequestStartGame());
					EventManager::Get()->QueueEvent(pRequestStartGameEvent);
				}

				break;
			}

			default:
			{
				LogError("Unknown control.");
			}
		}
	}

	return BaseUI::OnEvent(evt);
}


MainMenuView::MainMenuView() : HumanView()
{
	mMainMenuUI.reset(new MainMenuUI()); 
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

}


StandardHUD::~StandardHUD() 
{ 

}

bool StandardHUD::OnInit()
{
	BaseUI::OnInit();

	return true;
}

bool StandardHUD::OnRestore()
{
	return BaseUI::OnRestore();
}

bool StandardHUD::OnRender(double time, float elapsedTime)
{
	return BaseUI::OnRender(time, elapsedTime);
};


bool StandardHUD::OnMsgProc( const Event& evt )
{
	return BaseUI::OnMsgProc( evt );
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
						/*
						BaseResource resource(L"scripts\\test.lua");
						// this actually loads the Lua file from the zip file
						eastl::shared_ptr<ResHandle> pResourceHandle = 
							ResCache::Get()->GetHandle(&resource);
						*/
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
}


//
// GameDemoHumanView::OnUpdate				- Chapter 19, page 730
//
void GameDemoHumanView::OnUpdate(unsigned long deltaTime)
{
	HumanView::OnUpdate( deltaTime );
	/*
	if (mFreeCameraController)
	{
		mFreeCameraController->OnUpdate(deltaMs);
	}
	*/
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

    mStandardHUD.reset(new StandardHUD());
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