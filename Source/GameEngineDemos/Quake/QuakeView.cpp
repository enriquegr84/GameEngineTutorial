//========================================================================
// QuakeView.cpp : Game View Class
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

#include "QuakeStd.h"

#include "Game/GameOption.h"

#include "Audio/Audio.h"
#include "Audio/SoundProcess.h"
#include "Graphic/Renderer/Renderer.h"
#include "Graphic/Image/ImageResource.h"

#include "Graphic/Scene/Scene.h"
/*
#include "Graphic/Scene/Hierarchy/Node.h"
#include "Graphic/Scene/Hierarchy/Camera.h"
*/
#include "Core/Event/Event.h"
#include "Core/Event/EventManager.h"

#include "Physic/PhysicEventListener.h"

#include "Game/View/HumanView.h"

#include "Game/Actor/Actor.h"
#include "Game/Actor/RenderComponent.h"

#include "Quake.h"
#include "QuakeApp.h"
#include "QuakeView.h"
#include "QuakeAIView.h"
#include "QuakeEvents.h"
#include "QuakeLevelManager.h"
#include "QuakeAIManager.h"
#include "QuakeNetwork.h"
#include "QuakePlayerController.h"
#include "QuakeCameraController.h"

//========================================================================
//
// QuakeMainMenuUI & QuakeMainMenuView implementation
//
//
//========================================================================

#define CID_QUAKE_WINDOW				(1)
#define CID_CREATE_GAME_RADIO			(2)
#define CID_NUM_AI_SLIDER				(3)
#define CID_NUM_PLAYER_SLIDER			(4)
#define CID_HOST_LISTEN_PORT			(5)
#define CID_CLIENT_ATTACH_PORT			(6)
#define CID_START_BUTTON				(7)
#define CID_AI_COMBAT_MOD				(8)
#define CID_NUM_AI_LABEL				(9)
#define CID_NUM_PLAYER_LABEL			(10)
#define CID_HOST_LISTEN_PORT_LABEL		(11)
#define CID_CLIENT_ATTACH_PORT_LABEL	(12)
#define CID_AI_COMBAT_LABEL				(13)
#define CID_LEVEL_LABEL					(14)
#define CID_LEVEL_LISTBOX				(15)
#define CID_STATUS_LABEL				(16)
#define CID_DRIVER_LABEL				(17)
#define CID_FULLSCREEN_MODE				(18)
#define CID_SET_GAME_RADIO				(19)


QuakeMainMenuUI::QuakeMainMenuUI()
{

}

QuakeMainMenuUI::~QuakeMainMenuUI()
{ 
	//GUIEngine::DialogQueue::deallocate();

	//D3DRenderer::DialogResourceManager.UnregisterDialog(&mSampleUI);
	//if(quake_manager)
	//	SAFE_DELETE ( quake_manager );

	//ScreenStateManager::Deallocate();
	//GUIEventHandler::Deallocate();
}

bool QuakeMainMenuUI::OnInit()
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
	RectangleShape<2, int> screenRectangle;
	screenRectangle.mCenter[0] = screenSize[0] / 2;
	screenRectangle.mCenter[1] = screenSize[1] / 2;
	screenRectangle.mExtent[0] = (int)screenSize[0];
	screenRectangle.mExtent[1] = (int)screenSize[1];

	eastl::shared_ptr<BaseUIWindow> window = AddWindow(
		screenRectangle, false, L"Quake", 0, CID_QUAKE_WINDOW);
	window->GetCloseButton()->SetToolTipText(L"Quit Quake");

	// add a options line
	RectangleShape<2, int> playerOptionsRectangle;
	playerOptionsRectangle.mCenter[0] = 50;
	playerOptionsRectangle.mExtent[0] = 90;
	playerOptionsRectangle.mCenter[1] = 42;
	playerOptionsRectangle.mExtent[1] = 16;
	eastl::shared_ptr<BaseUIStaticText> playerOptionsLine = AddStaticText(
		L"AI Player:", playerOptionsRectangle, false, false, window, CID_NUM_AI_LABEL, true);
	playerOptionsLine->SetTextAlignment(UIA_UPPERLEFT, UIA_CENTER);

	playerOptionsRectangle.mCenter[0] = 250;
	playerOptionsRectangle.mExtent[0] = 250;
	playerOptionsRectangle.mCenter[1] = 40;
	playerOptionsRectangle.mExtent[1] = 20;
	eastl::shared_ptr<BaseUIScrollBar> gameAI = AddScrollBar(
		true, playerOptionsRectangle, window, CID_NUM_AI_SLIDER);
	gameAI->SetMin(0);
	gameAI->SetMax(gameApp->mOption.mMaxAIs);
	gameAI->SetSmallStep(1);
	gameAI->SetLargeStep(1);
	gameAI->SetPos(gameApp->mOption.mNumAIs);
	gameAI->SetToolTipText(L"Set the AI players");

	playerOptionsRectangle.mCenter[0] = 50;
	playerOptionsRectangle.mExtent[0] = 90;
	playerOptionsRectangle.mCenter[1] = 82;
	playerOptionsRectangle.mExtent[1] = 16;
	playerOptionsLine = AddStaticText(
		L"Human Player:", playerOptionsRectangle, false, false, window, CID_NUM_PLAYER_LABEL, false);
	playerOptionsLine->SetTextAlignment(UIA_UPPERLEFT, UIA_CENTER);

	playerOptionsRectangle.mCenter[0] = 250;
	playerOptionsRectangle.mExtent[0] = 250;
	playerOptionsRectangle.mCenter[1] = 80;
	playerOptionsRectangle.mExtent[1] = 20;
	eastl::shared_ptr<BaseUIScrollBar> gamePlayer = AddScrollBar(
		true, playerOptionsRectangle, window, CID_NUM_PLAYER_SLIDER);
	gamePlayer->SetMin(0);
	gamePlayer->SetMax(gameApp->mOption.mMaxPlayers);
	gamePlayer->SetSmallStep(1);
	gamePlayer->SetLargeStep(1);
	gamePlayer->SetPos(gameApp->mOption.mExpectedPlayers);
	gamePlayer->SetToolTipText(L"Set the Human players");

	playerOptionsRectangle.mCenter[0] = 50;
	playerOptionsRectangle.mExtent[0] = 90;
	playerOptionsRectangle.mCenter[1] = 122;
	playerOptionsRectangle.mExtent[1] = 16;
	playerOptionsLine = AddStaticText(
		L"AI Combat Mod:", playerOptionsRectangle, false, false, window, CID_AI_COMBAT_LABEL, false);
	playerOptionsLine->SetTextAlignment(UIA_UPPERLEFT, UIA_CENTER);

	screenRectangle.mCenter[0] = 180;
	screenRectangle.mExtent[0] = 100;
	screenRectangle.mCenter[1] = 120;
	screenRectangle.mExtent[1] = 20;
	eastl::shared_ptr<BaseUICheckBox> combatMod = AddCheckBox(
		gameApp->mOption.mLevelMod, screenRectangle, window, CID_AI_COMBAT_MOD, L"Set");
	combatMod->SetToolTipText(L"Set AI Combat Mod");

	playerOptionsRectangle.mCenter[0] = 350;
	playerOptionsRectangle.mExtent[0] = 50;
	playerOptionsRectangle.mCenter[1] = 120;
	playerOptionsRectangle.mExtent[1] = 20;
	eastl::shared_ptr<BaseUIButton> gameStart = AddButton(
		playerOptionsRectangle, window, CID_START_BUTTON, L"Start");
	gameStart->SetToolTipText(L"Start Game");

	// add a status line help text
	RectangleShape<2, int> statusRectangle;
	statusRectangle.mCenter[0] = screenSize[0] / 2 + 5;
	statusRectangle.mExtent[0] = screenSize[0] - 10;
	statusRectangle.mCenter[1] = screenSize[1] - 20;
	statusRectangle.mExtent[1] = 20;
	eastl::shared_ptr<BaseUIStaticText> statusLine = AddStaticText(
		L"", statusRectangle, false, false, window, CID_STATUS_LABEL, true);
	statusLine->SetTextAlignment(UIA_UPPERLEFT, UIA_CENTER);

	RectangleShape<2, int> videoRectangle;
	videoRectangle.mCenter[0] = screenSize[0] - 355;
	videoRectangle.mExtent[0] = 90;
	videoRectangle.mCenter[1] = 42;
	videoRectangle.mExtent[1] = 16;
	eastl::shared_ptr<BaseUIStaticText> videoDriverLine = AddStaticText(
		L"VideoDriver:", videoRectangle, false, false, window, -1, true);
	videoDriverLine->SetTextAlignment(UIA_UPPERLEFT, UIA_CENTER);
	
	videoRectangle.mCenter[0] = screenSize[0] - 155;
	videoRectangle.mExtent[0] = 290;
	videoRectangle.mCenter[1] = 42;
	videoRectangle.mExtent[1] = 16;
	eastl::shared_ptr<BaseUIStaticText> videoDriver;
#if defined(_OPENGL_)
	videoDriver = AddStaticText(L"OPENGL", videoRectangle, false, false, window, CID_DRIVER_LABEL, true);
#else
	videoDriver = AddStaticText(L"DIRECTX", videoRectangle, false, false, window, CID_DRIVER_LABEL, true);
#endif
	videoDriver->SetTextAlignment(UIA_UPPERLEFT, UIA_CENTER);

	videoRectangle.mCenter[0] = screenSize[0] - 355;
	videoRectangle.mExtent[0] = 90;
	videoRectangle.mCenter[1] = 82;
	videoRectangle.mExtent[1] = 16;
	eastl::shared_ptr<BaseUIStaticText> videoModeLine =
		AddStaticText(L"VideoMode:", videoRectangle, false, false, window, -1, false);
	videoModeLine->SetTextAlignment(UIA_UPPERLEFT, UIA_CENTER);

	videoRectangle.mCenter[0] = screenSize[0] - 155;
	videoRectangle.mExtent[0] = 290;
	videoRectangle.mCenter[1] = 80;
	videoRectangle.mExtent[1] = 20;
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

	screenRectangle.mCenter[0] = screenSize[0] - 350;
	screenRectangle.mExtent[0] = 100;
	screenRectangle.mCenter[1] = 120;
	screenRectangle.mExtent[1] = 20;
	eastl::shared_ptr<BaseUICheckBox> fullScreen = AddCheckBox(
		gameApp->mOption.mFullScreen, screenRectangle, window, CID_FULLSCREEN_MODE, L"Fullscreen");
	fullScreen->SetToolTipText(L"Set Fullscreen or Window Mode");

	screenRectangle.mCenter[0] = screenSize[0] - 250;
	screenRectangle.mExtent[0] = 90;
	screenRectangle.mCenter[1] = 122;
	screenRectangle.mExtent[1] = 16;
	eastl::shared_ptr<BaseUIStaticText> videoMultiSampleLine =
		AddStaticText(L"Multisample:", screenRectangle, false, false, window, -1, false);
	videoMultiSampleLine->SetTextAlignment(UIA_UPPERLEFT, UIA_CENTER);

	screenRectangle.mCenter[0] = screenSize[0] - 130;
	screenRectangle.mExtent[0] = 120;
	screenRectangle.mCenter[1] = 120;
	screenRectangle.mExtent[1] = 20;
	eastl::shared_ptr<BaseUIScrollBar> multiSample = AddScrollBar(true, screenRectangle, window, -1);
	multiSample->SetMin(0);
	multiSample->SetMax(8);
	multiSample->SetSmallStep(1);
	multiSample->SetLargeStep(1);
	multiSample->SetPos(gameApp->mOption.mAntiAlias);
	multiSample->SetToolTipText(L"Set the multisample (disable, 1x, 2x, 4x, 8x )");

	screenRectangle.mCenter[0] = screenSize[0] - 35;
	screenRectangle.mExtent[0] = 50;
	screenRectangle.mCenter[1] = 120;
	screenRectangle.mExtent[1] = 20;
	eastl::shared_ptr<BaseUIButton> setVideoMode = AddButton(screenRectangle, window, CID_SET_GAME_RADIO, L"Set");
	setVideoMode->SetToolTipText(L"Set video mode with current values");

	screenRectangle.mCenter[0] = 50;
	screenRectangle.mExtent[0] = 90;
	screenRectangle.mCenter[1] = screenSize[1] - 390;
	screenRectangle.mExtent[1] = 20;
	eastl::shared_ptr<BaseUIStaticText> levelLine =
		AddStaticText(L"Levels:", screenRectangle, false, false, window, CID_LEVEL_LABEL, false);
	levelLine->SetTextAlignment(UIA_UPPERLEFT, UIA_CENTER);

	screenRectangle.mCenter[0] = 190;
	screenRectangle.mExtent[0] = 380;
	screenRectangle.mCenter[1] = screenSize[1] - 210;
	screenRectangle.mExtent[1] = 340;
	eastl::shared_ptr<BaseUIListBox> level = AddListBox(screenRectangle, window, CID_LEVEL_LISTBOX, true);
	level->SetToolTipText(L"Select the current level.\n Press button to start the level");

	eastl::vector<Level*> levels = GameLogic::Get()->GetLevelManager()->GetLevels();
	for (eastl::vector<Level*>::iterator it = levels.begin(); it != levels.end(); ++it)
		level->AddItem((*it)->GetName().c_str());

	// create a setting panel
	screenRectangle.mCenter[0] = screenSize[0] - 350;
	screenRectangle.mExtent[0] = 90;
	screenRectangle.mCenter[1] = screenSize[1] - 390;
	screenRectangle.mExtent[1] = 20;
	eastl::shared_ptr<BaseUIStaticText> settingsLine =
		AddStaticText(L"Settings:", screenRectangle, false, false, window, -1, false);
	settingsLine->SetTextAlignment(UIA_UPPERLEFT, UIA_CENTER);

	screenRectangle.mCenter[0] = screenSize[0] - 200;
	screenRectangle.mExtent[0] = 400;
	screenRectangle.mCenter[1] = screenSize[1] - 210;
	screenRectangle.mExtent[1] = 340;
	eastl::shared_ptr<BaseUIListBox> settings = AddListBox(screenRectangle, window, -1, true);
	settings->SetToolTipText(L"Show the current settings");
	settings->AddItem(L"Mouse Right Button - Jump");
	settings->AddItem(L"Mouse Left Button - Attack");
	settings->AddItem(L"Mouse Wheel up - Change Weapon");
	settings->AddItem(L"Mouse Wheel down - Change Weapon");
	settings->AddItem(L"Key W - Move forward");
	settings->AddItem(L"Key S - Move backward");
	settings->AddItem(L"Key A - Move left");
	settings->AddItem(L"Key D - Move right");
	settings->AddItem(L"Key C - Move down");
	settings->AddItem(L"Key Space - Move up");
	settings->AddItem(L"Key 5 - Create map");
	settings->AddItem(L"Key 6 - Graphics wireframe");
	settings->AddItem(L"Key 7 - Physics wireframe");
	settings->AddItem(L"Key 8 - Control/Follow player");
	settings->AddItem(L"Key 9 - Control camera");

	SetUIActive(1);
	return true;
}


void QuakeMainMenuUI::Set()
{
	const eastl::shared_ptr<BaseUIElement>& root = GetRootUIElement();
	const eastl::shared_ptr<BaseUIElement>& window = root->GetElementFromId(CID_QUAKE_WINDOW);
	const eastl::shared_ptr<BaseUIButton>& createGame = 
		eastl::static_pointer_cast<BaseUIButton>(root->GetElementFromId(CID_CREATE_GAME_RADIO, true));
	const eastl::shared_ptr<BaseUIButton>& setGame =
		eastl::static_pointer_cast<BaseUIButton>(root->GetElementFromId(CID_SET_GAME_RADIO, true));
	const eastl::shared_ptr<BaseUICheckBox>& setGameMod =
		eastl::static_pointer_cast<BaseUICheckBox>(root->GetElementFromId(CID_AI_COMBAT_MOD, true));
	const eastl::shared_ptr<BaseUIScrollBar>& numAI = 
		eastl::static_pointer_cast<BaseUIScrollBar>(root->GetElementFromId(CID_NUM_AI_SLIDER, true));
	const eastl::shared_ptr<BaseUIScrollBar>& numPlayer = 
		eastl::static_pointer_cast<BaseUIScrollBar>(root->GetElementFromId(CID_NUM_PLAYER_SLIDER, true));
	const eastl::shared_ptr<BaseUIEditBox>& hostPort = 
		eastl::static_pointer_cast<BaseUIEditBox>(root->GetElementFromId(CID_HOST_LISTEN_PORT, true));
	const eastl::shared_ptr<BaseUIEditBox>& clientPort = 
		eastl::static_pointer_cast<BaseUIEditBox>(root->GetElementFromId(CID_CLIENT_ATTACH_PORT, true));
	const eastl::shared_ptr<BaseUIButton>& startGame = 
		eastl::static_pointer_cast<BaseUIButton>(root->GetElementFromId(CID_START_BUTTON, true));
	const eastl::shared_ptr<BaseUIListBox>& level = 
		eastl::static_pointer_cast<BaseUIListBox>(root->GetElementFromId(CID_LEVEL_LISTBOX, true));

	GameApplication* gameApp = (GameApplication*)Application::App;
	gameApp->mOption.mNumAIs = numAI->GetPos();
	gameApp->mOption.mExpectedPlayers = numPlayer->GetPos();
	gameApp->mOption.mLevelMod = setGameMod->IsChecked();

	if (level->GetSelected() >= 0)
	{
		eastl::vector<Level*> levels = GameLogic::Get()->GetLevelManager()->GetLevels();

		Level* gameLevel = levels[level->GetSelected()];
		gameApp->mOption.mLevel = ToString(gameLevel->GetFileName().c_str());
	}
}

// enable GUI elements
void QuakeMainMenuUI::SetUIActive(int command)
{
	bool inputState = false;

	int guiActive = -1;
	switch (command)
	{
		case 0: guiActive = 0; inputState = !guiActive; break;
		case 1: guiActive = 1; inputState = !guiActive; break;
		case 2: guiActive ^= 1; inputState = !guiActive; break;
		case 3: break;
	}

	const eastl::shared_ptr<BaseUIElement>& root = GetRootUIElement();
	const eastl::shared_ptr<BaseUIElement>& window = root->GetElementFromId(CID_QUAKE_WINDOW);
	if (window)
		window->SetVisible(guiActive != 0);

	SetFocus(guiActive ? window : 0);
}

bool QuakeMainMenuUI::OnRestore()
{
	return true;
}

bool QuakeMainMenuUI::OnRender(double time, float elapsedTime)
{
	GameApplication* gameApp = (GameApplication*)Application::App;

	const eastl::shared_ptr<BaseUIElement>& root = GetRootUIElement();
	const eastl::shared_ptr<BaseUIStaticText>& statusLabel =
		eastl::static_pointer_cast<BaseUIStaticText>(root->GetElementFromId(CID_STATUS_LABEL));
	if (statusLabel)
		statusLabel->SetText(eastl::wstring(L"Press set button to change settings").c_str());

	return BaseUI::OnRender(time, elapsedTime);
};

bool QuakeMainMenuUI::OnMsgProc( const Event& evt )
{
	return BaseUI::OnMsgProc(evt);
}


//--------------------------------------------------------------------------------------
// Handles the GUI events
//--------------------------------------------------------------------------------------
//    Note: pUserContext added to comply with DirectX 9c - June 2005 Update
//
bool QuakeMainMenuUI::OnEvent(const Event& evt)
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

			case CID_QUAKE_WINDOW:
				break;
			case CID_SET_GAME_RADIO:
			{
				/*
				if (evt.mUIEvent.mEventType == UIEVT_BUTTON_CLICKED)
				{
					const eastl::shared_ptr<BaseUIElement>& root = GetRootUIElement();
					const eastl::shared_ptr<BaseUICheckBox>& fullscreen =
						eastl::static_pointer_cast<BaseUICheckBox>(
						root->GetElementFromId(CID_FULLSCREEN_MODE, true));
					if (!System::Get()->IsFullscreen() && fullscreen->IsChecked())
						System::Get()->SetFullscreen(true);
					else if (System::Get()->IsFullscreen() && !fullscreen->IsChecked())
						System::Get()->SetFullscreen(false);
				}
				*/
				break;
			}

			case CID_LEVEL_LISTBOX:
			case CID_NUM_AI_SLIDER:
			case CID_NUM_PLAYER_SLIDER:
			case CID_HOST_LISTEN_PORT:
			case CID_CLIENT_ATTACH_PORT:
			case CID_AI_COMBAT_MOD:
			{
				break;
			}

			case CID_NUM_AI_LABEL:
			case CID_NUM_PLAYER_LABEL:
			case CID_HOST_LISTEN_PORT_LABEL:
			case CID_CLIENT_ATTACH_PORT_LABEL:
			case CID_AI_COMBAT_LABEL:
			case CID_LEVEL_LABEL:
			case CID_STATUS_LABEL:
			{
				break;
			}

			case CID_START_BUTTON:
			{
				if (evt.mUIEvent.mEventType == UIEVT_BUTTON_CLICKED)
				{
					Set();
					SetVisible(false);

					eastl::shared_ptr<EventDataRequestStartGame> 
						pRequestStartGameEvent(new EventDataRequestStartGame());
					EventManager::Get()->QueueEvent(pRequestStartGameEvent);
				}

				break;
			}

			default:
			{
				LogWarning("Unknown control.");
			}
		}
	}

	return BaseUI::OnEvent(evt);
}


QuakeMainMenuView::QuakeMainMenuView() : HumanView()
{
	mQuakeMainMenuUI.reset(new QuakeMainMenuUI()); 
	mQuakeMainMenuUI->OnInit();
	PushElement(mQuakeMainMenuUI);
}


QuakeMainMenuView::~QuakeMainMenuView()
{
}


void QuakeMainMenuView::RenderText()
{
	HumanView::RenderText();
}


void QuakeMainMenuView::OnUpdate(unsigned int timeMs, unsigned long deltaMs)
{
	HumanView::OnUpdate(deltaMs, deltaMs);
}


bool QuakeMainMenuView::OnMsgProc( const Event& evt )
{
	if (mQuakeMainMenuUI->IsVisible() )
	{
		if (HumanView::OnMsgProc(evt))
			return 1;
	}
	return 0;
}

//========================================================================
//
// QuakeStandardHUD implementation
//
//
//========================================================================

//--------------------------------------------------------------------------------------
// UI control IDs
//--------------------------------------------------------------------------------------
#define IDC_TOGGLEFULLSCREEN    1
#define IDC_TOGGLEREF           2
#define IDC_CHANGEDEVICE        3

QuakeStandardHUD::QuakeStandardHUD(const eastl::shared_ptr<QuakeHumanView>& view)
	: mGameView(view)
{
	RectangleShape<2, int> rectangle;
	rectangle.mCenter[0] = (ICON_SIZE / 2);
	rectangle.mCenter[1] = 542 + (ICON_SIZE / 2);
	rectangle.mExtent[0] = CHAR_WIDTH;
	rectangle.mExtent[1] = CHAR_HEIGHT;
	mAmmo.push_back(AddImage(rectangle, 0, true));

	rectangle.mCenter[0] += CHAR_WIDTH;
	mAmmo.push_back(AddImage(rectangle, 0, true));

	rectangle.mCenter[0] += CHAR_WIDTH;
	mAmmo.push_back(AddImage(rectangle, 0, true));

	rectangle.mCenter[0] += CHAR_WIDTH;
	mAmmo.push_back(AddImage(rectangle, 0, true));

	rectangle.mCenter[0] = CHAR_WIDTH * 3 + TEXT_ICON_SPACE * 2 + (ICON_SIZE / 2);
	rectangle.mExtent[0] = ICON_SIZE;
	rectangle.mExtent[1] = ICON_SIZE;
	mAmmoIcon = AddImage(rectangle, 0, false);

	rectangle.mCenter[0] = 185 + (ICON_SIZE / 2);
	rectangle.mExtent[0] = CHAR_WIDTH;
	rectangle.mExtent[1] = CHAR_HEIGHT;
	mHealth.push_back(AddImage(rectangle, 0, true));

	rectangle.mCenter[0] += CHAR_WIDTH;
	mHealth.push_back(AddImage(rectangle, 0, true));

	rectangle.mCenter[0] += CHAR_WIDTH;
	mHealth.push_back(AddImage(rectangle, 0, true));

	rectangle.mCenter[0] += CHAR_WIDTH;
	mHealth.push_back(AddImage(rectangle, 0, true));

	rectangle.mCenter[0] = 185 + CHAR_WIDTH * 3 + TEXT_ICON_SPACE * 2 + (ICON_SIZE / 2);
	rectangle.mExtent[0] = ICON_SIZE;
	rectangle.mExtent[1] = ICON_SIZE;
	mHealthIcon = AddImage(rectangle, 0, false);

	rectangle.mCenter[0] = 370 + (ICON_SIZE / 2);
	rectangle.mExtent[0] = CHAR_WIDTH;
	rectangle.mExtent[1] = CHAR_HEIGHT;
	mArmor.push_back(AddImage(rectangle, 0, true));

	rectangle.mCenter[0] += CHAR_WIDTH;
	mArmor.push_back(AddImage(rectangle, 0, true));

	rectangle.mCenter[0] += CHAR_WIDTH;
	mArmor.push_back(AddImage(rectangle, 0, true));

	rectangle.mCenter[0] += CHAR_WIDTH;
	mArmor.push_back(AddImage(rectangle, 0, true));

	rectangle.mCenter[0] = 370 + CHAR_WIDTH * 3 + TEXT_ICON_SPACE * 2 + (ICON_SIZE / 2);
	rectangle.mExtent[0] = ICON_SIZE;
	rectangle.mExtent[1] = ICON_SIZE;
	mArmorIcon = AddImage(rectangle, 0, false);

	rectangle.mCenter[0] = 720;
	rectangle.mExtent[0] = CHAR_WIDTH;
	rectangle.mExtent[1] = CHAR_WIDTH;
	eastl::shared_ptr<BaseUIStaticText> score =
		AddStaticText(L"0", rectangle, false, false, 0, -1, true);
	score->SetBackgroundColor(eastl::array<float, 4U>{0.f, 0.f, 1.f, 1.f});
	score->SetOverrideColor(eastl::array<float, 4U>{1.f, 1.f, 1.f, 1.f});
	score->SetTextAlignment(UIA_UPPERLEFT, UIA_CENTER);
	mScore.push_back(score);

	rectangle.mCenter[0] += CHAR_WIDTH;
	score = AddStaticText(L"0", rectangle, false, false, 0, -1, true);
	score->SetBackgroundColor(eastl::array<float, 4U>{1.f, 0.f, 0.f, 1.f});
	score->SetOverrideColor(eastl::array<float, 4U>{1.f, 1.f, 1.f, 1.f});
	score->SetTextAlignment(UIA_UPPERLEFT, UIA_CENTER);
	mScore.push_back(score);

	Renderer* renderer = Renderer::Get();
	Vector2<unsigned int> screenSize = renderer->GetScreenSize();

	eastl::shared_ptr<ResHandle> resHandle = 
		ResCache::Get()->GetHandle(&BaseResource(mGameView->mMedia.crosshairShader[1]));
	eastl::shared_ptr<ImageResourceExtraData> resData =
		eastl::static_pointer_cast<ImageResourceExtraData>(resHandle->GetExtra());
	mCrosshair = AddImage(
		resData->GetImage(), Vector2<int>{ (int)screenSize[0] / 2, (int)screenSize[1] / 2});
}

QuakeStandardHUD::~QuakeStandardHUD() 
{ 

}

bool QuakeStandardHUD::OnInit()
{
	BaseUI::OnInit();

	GameApplication* gameApp = (GameApplication*)Application::App;
	System* system = System::Get();
	system->GetCursorControl()->SetVisible(false);

	// set a nicer font
	const eastl::shared_ptr<BaseUIFont>& font = GetFont(L"DefaultFont");
	if (font) GetSkin()->SetFont(font);

	return true;
}

bool QuakeStandardHUD::OnRestore()
{
	return BaseUI::OnRestore();
}

bool QuakeStandardHUD::OnRender(double time, float elapsedTime)
{
	return BaseUI::OnRender(time, elapsedTime);
};


bool QuakeStandardHUD::OnMsgProc( const Event& evt )
{
	return BaseUI::OnMsgProc( evt );
}

//--------------------------------------------------------------------------------------
// Handles the GUI events
//--------------------------------------------------------------------------------------
bool QuakeStandardHUD::OnEvent(const Event& evt)
{
	if (evt.mEventType == ET_UI_EVENT)
	{
		int id = evt.mUIEvent.mCaller->GetID();
	}

	return false;
}

/*
================
UpdateStatusBar
================
*/
void QuakeStandardHUD::UpdateStatusBar(const eastl::shared_ptr<PlayerActor>& player)
{
	// weapon
	unsigned int weapon = player->GetState().weapon - 1;
	if (player->GetState().weapon)
	{
		if (!mGameView->mWeaponMedia[weapon].ammoIcon.empty())
		{
			eastl::shared_ptr<ResHandle> resHandle = ResCache::Get()->GetHandle(
				&BaseResource(mGameView->mWeaponMedia[weapon].ammoIcon));
			eastl::shared_ptr<ImageResourceExtraData> resData =
				eastl::static_pointer_cast<ImageResourceExtraData>(resHandle->GetExtra());
			mAmmoIcon->SetImage(resData->GetImage());

			char num[16];
			sprintf(num, "%i", eastl::clamp(player->GetState().ammo[player->GetState().weapon], -999, 999));
			for (unsigned int n = 0; n < strlen(num); n++)
			{
				if (num[n] == '-')
				{
					eastl::shared_ptr<ResHandle> resHandle = ResCache::Get()->GetHandle(
						&BaseResource(mGameView->mMedia.numberShaders[STAT_MINUS]));
					eastl::shared_ptr<ImageResourceExtraData> resData =
						eastl::static_pointer_cast<ImageResourceExtraData>(resHandle->GetExtra());

					mAmmo[n]->SetImage(resData->GetImage());
					mAmmo[n]->SetVisible(true);
				}
				else
				{
					eastl::shared_ptr<ResHandle> resHandle = ResCache::Get()->GetHandle(
						&BaseResource(mGameView->mMedia.numberShaders[num[n] - '0']));
					eastl::shared_ptr<ImageResourceExtraData> resData =
						eastl::static_pointer_cast<ImageResourceExtraData>(resHandle->GetExtra());

					mAmmo[n]->SetImage(resData->GetImage());
					mAmmo[n]->SetVisible(true);
				}
			}
		}
		else
		{
			eastl::shared_ptr<ResHandle> resHandle = 
				ResCache::Get()->GetHandle(&BaseResource(mGameView->mMedia.deferShader));
			eastl::shared_ptr<ImageResourceExtraData> resData =
				eastl::static_pointer_cast<ImageResourceExtraData>(resHandle->GetExtra());
			mAmmoIcon->SetImage(resData->GetImage());
		}
	}

	//DrawStatusBarHead(185 + CHAR_WIDTH * 3 + TEXT_ICON_SPACE);

	// health
	{
		eastl::shared_ptr<ResHandle> resHandle = 
			ResCache::Get()->GetHandle(&BaseResource(mGameView->mMedia.healthIcon));
		eastl::shared_ptr<ImageResourceExtraData> resData =
			eastl::static_pointer_cast<ImageResourceExtraData>(resHandle->GetExtra());

		mHealthIcon->SetImage(resData->GetImage());

		char num[16];
		sprintf(num, "%i", eastl::clamp(player->GetState().stats[STAT_HEALTH], -999, 999));
		for (unsigned int n = 0; n < strlen(num); n++)
		{
			if (num[n] == '-')
			{
				eastl::shared_ptr<ResHandle> resHandle = ResCache::Get()->GetHandle(
					&BaseResource(mGameView->mMedia.numberShaders[STAT_MINUS]));
				eastl::shared_ptr<ImageResourceExtraData> resData =
					eastl::static_pointer_cast<ImageResourceExtraData>(resHandle->GetExtra());

				mHealth[n]->SetImage(resData->GetImage());
				mHealth[n]->SetVisible(true);
			}
			else
			{
				eastl::shared_ptr<ResHandle> resHandle = ResCache::Get()->GetHandle(
					&BaseResource(mGameView->mMedia.numberShaders[num[n] - '0']));
				eastl::shared_ptr<ImageResourceExtraData> resData =
					eastl::static_pointer_cast<ImageResourceExtraData>(resHandle->GetExtra());

				mHealth[n]->SetImage(resData->GetImage());
				mHealth[n]->SetVisible(true);
			}
		}
	}

	// armor
	{
		eastl::shared_ptr<ResHandle> resHandle = 
			ResCache::Get()->GetHandle(&BaseResource(mGameView->mMedia.armorIcon));
		eastl::shared_ptr<ImageResourceExtraData> resData =
			eastl::static_pointer_cast<ImageResourceExtraData>(resHandle->GetExtra());

		mArmorIcon->SetImage(resData->GetImage());

		if (player->GetState().stats[STAT_ARMOR])
		{
			char num[16];
			sprintf(num, "%i", eastl::clamp(player->GetState().stats[STAT_ARMOR], -999, 999));
			for (unsigned int n = 0; n < strlen(num); n++)
			{
				if (num[n] == '-')
				{
					eastl::shared_ptr<ResHandle> resHandle = ResCache::Get()->GetHandle(
						&BaseResource(mGameView->mMedia.numberShaders[STAT_MINUS]));
					eastl::shared_ptr<ImageResourceExtraData> resData =
						eastl::static_pointer_cast<ImageResourceExtraData>(resHandle->GetExtra());

					mArmor[n]->SetImage(resData->GetImage());
					mArmor[n]->SetVisible(true);
				}
				else
				{
					eastl::shared_ptr<ResHandle> resHandle = ResCache::Get()->GetHandle(
						&BaseResource(mGameView->mMedia.numberShaders[num[n] - '0']));
					eastl::shared_ptr<ImageResourceExtraData> resData =
						eastl::static_pointer_cast<ImageResourceExtraData>(resHandle->GetExtra());

					mArmor[n]->SetImage(resData->GetImage());
					mArmor[n]->SetVisible(true);
				}
			}
		}
	}
}


/*
=================
UpdateScores

Update the small two score display
=================
*/
void QuakeStandardHUD::UpdateScores()
{
	int scoreBlue = 0, scoreRed = 0;
	eastl::map<GameViewType, eastl::vector<eastl::shared_ptr<PlayerActor>>> playerViewTypes;

	GameApplication* gameApp = (GameApplication*)Application::App;
	const GameViewList& gameViews = gameApp->GetGameViews();
	for (auto it = gameViews.begin(); it != gameViews.end(); ++it)
	{
		eastl::shared_ptr<BaseGameView> pView = *it;
		if (pView->GetActorId() != INVALID_ACTOR_ID)
		{
			playerViewTypes[pView->GetType()].push_back(
				eastl::dynamic_shared_pointer_cast<PlayerActor>(
				GameLogic::Get()->GetActor(pView->GetActorId()).lock()));
		}
	}

	eastl::vector<eastl::shared_ptr<PlayerActor>>::iterator itAIPlayer;
	for (itAIPlayer = playerViewTypes[GV_AI].begin(); itAIPlayer != playerViewTypes[GV_AI].end(); )
	{
		bool removeAIPlayer = false;
		eastl::shared_ptr<PlayerActor> pAIPlayer = (*itAIPlayer);
		for (eastl::shared_ptr<PlayerActor> pHumanPlayer : playerViewTypes[GV_HUMAN])
			if (pHumanPlayer->GetId() == pAIPlayer->GetId())
				removeAIPlayer = true;

		if (removeAIPlayer)
			itAIPlayer = playerViewTypes[GV_AI].erase(itAIPlayer);
		else
			itAIPlayer++;
	}

	for (auto playerViewType : playerViewTypes)
	{
		GameViewType viewType = playerViewType.first;
		if (viewType == GV_AI)
		{
			for (auto player : playerViewType.second)
				scoreRed += player->GetState().persistant[PERS_SCORE];
		}
		else
		{
			for (auto player : playerViewType.second)
				scoreBlue += player->GetState().persistant[PERS_SCORE];
		}
	}
	//score
	mScore[0]->SetText(eastl::to_wstring(scoreBlue).c_str());
	mScore[1]->SetText(eastl::to_wstring(scoreRed).c_str());
}


/*
===================
UpdatePickupItem
===================
*/
void QuakeStandardHUD::UpdatePickupItem()
{

}


void QuakeStandardHUD::OnUpdate(unsigned int timeMs, unsigned long deltaMs)
{
	if (mGameView)
	{
		eastl::shared_ptr<Node> target = mGameView->mPlayer;
		if (target)
		{
			mCrosshair->SetVisible(true);
			mAmmoIcon->SetVisible(true);
			mArmorIcon->SetVisible(true);
			mHealthIcon->SetVisible(true);
			for (eastl::shared_ptr<BaseUIImage> ammo : mAmmo)
				ammo->SetVisible(false);
			for (eastl::shared_ptr<BaseUIImage> armor : mArmor)
				armor->SetVisible(false);
			for (eastl::shared_ptr<BaseUIImage> health : mHealth)
				health->SetVisible(false);
			for (eastl::shared_ptr<BaseUIStaticText> score : mScore)
				score->SetVisible(true);

			eastl::shared_ptr<PlayerActor> playerActor(
				eastl::dynamic_shared_pointer_cast<PlayerActor>(
				GameLogic::Get()->GetActor(target->GetId()).lock()));

			// don't draw any status if dead or the scoreboard is being explicitly shown
			if (playerActor->GetState().stats[STAT_HEALTH] > 0)
				UpdateStatusBar(playerActor);

			UpdateScores();
			UpdatePickupItem();
		}
		else
		{
			mCrosshair->SetVisible(false);
			mAmmoIcon->SetVisible(false); 
			mArmorIcon->SetVisible(false);
			mHealthIcon->SetVisible(false);
			for (eastl::shared_ptr<BaseUIImage> ammo : mAmmo)
				ammo->SetVisible(false);
			for (eastl::shared_ptr<BaseUIImage> armor : mArmor)
				armor->SetVisible(false);
			for (eastl::shared_ptr<BaseUIImage> health : mHealth)
				health->SetVisible(false);
			for (eastl::shared_ptr<BaseUIStaticText> score : mScore)
				score->SetVisible(false);
		}
	}
}

//========================================================================
//
// QuakeHumanView Implementation
//
//========================================================================

//
// QuakeHumanView::QuakeHumanView	- Chapter 19, page 724
//
QuakeHumanView::QuakeHumanView() : HumanView()
{ 
	mShowUI = true; 
	mDebugMode = DM_OFF;

	RegisterSound();
	RegisterMedia();
	for (unsigned int i = 0; i < 8; i++)
		RegisterWeapon(i);
    RegisterAllDelegates();
}


QuakeHumanView::~QuakeHumanView()
{
    RemoveAllDelegates();
}

//
// QuakeHumanView::OnMsgProc				- Chapter 19, page 727
//
bool QuakeHumanView::OnMsgProc( const Event& evt )
{
	HumanView::OnMsgProc(evt);

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
					case KEY_KEY_5:
					{
						GameApplication* gameApp = (GameApplication*)Application::App;
						const GameViewList& gameViews = gameApp->GetGameViews();
						for (auto it = gameViews.begin(); it != gameViews.end(); ++it)
						{
							eastl::shared_ptr<BaseGameView> pView = *it;
							if (pView->GetType() == GV_HUMAN)
							{
								eastl::shared_ptr<QuakeHumanView> pHumanView =
									eastl::static_pointer_cast<QuakeHumanView, BaseGameView>(pView);
								GameLogic::Get()->GetAIManager()->CreateMap(pHumanView->GetActorId());
								break;
							}
						}

						return true;
					}

					case KEY_KEY_6:
					{
						mDebugMode = mDebugMode ? DM_OFF : DM_WIREFRAME;
						for (auto child : mScene->GetRootNode()->GetChildren())
							child->SetDebugState(mDebugMode);
						return true;
					}

					case KEY_KEY_7:
					{
						QuakeLogic* game = static_cast<QuakeLogic *>(GameLogic::Get());
						game->ToggleRenderDiagnostics();
						return true;
					}	

					case KEY_KEY_8:
					{
						if (mPlayer)
						{
							GameApplication* gameApp = (GameApplication*)Application::App;
							const GameViewList& gameViews = gameApp->GetGameViews();
							for (auto it = gameViews.begin(); it != gameViews.end(); ++it)
							{
								eastl::shared_ptr<BaseGameView> pView = *it;
								if (pView->GetType() == GV_HUMAN)
								{
									eastl::shared_ptr<QuakeHumanView> pHumanView =
										eastl::static_pointer_cast<QuakeHumanView, BaseGameView>(pView);
									if (pHumanView->GetActorId() != mPlayer->GetId())
									{
										if (gameApp->mOption.mLevelMod)
										{
											mPlayer = mScene->GetSceneNode(pHumanView->GetActorId());

											if (mGamePlayerController)
												mGamePlayerController->SetEnabled(false);
											if (mGameCameraController)
												mGameCameraController->SetEnabled(false);

											mKeyboardHandler = NULL;
											mMouseHandler = NULL;
											mCamera->SetTarget(mPlayer);
										}
										else
										{
											mPlayer = mScene->GetSceneNode(pHumanView->GetActorId());

											if (mGamePlayerController)
												mGamePlayerController->SetEnabled(true);
											if (mGameCameraController)
												mGameCameraController->SetEnabled(false);

											mKeyboardHandler = mGamePlayerController;
											mMouseHandler = mGamePlayerController;
											mCamera->SetTarget(mPlayer);
										}
										break;
									}
								}
								else if (pView->GetType() == GV_REMOTE)
								{
									eastl::shared_ptr<NetworkGameView> pNetworkGameView =
										eastl::static_pointer_cast<NetworkGameView, BaseGameView>(pView);
									if (pNetworkGameView->GetActorId() != mPlayer->GetId())
									{
										mPlayer = mScene->GetSceneNode(pNetworkGameView->GetActorId());
										
										if (mGamePlayerController)
											mGamePlayerController->SetEnabled(false);
										if (mGameCameraController)
											mGameCameraController->SetEnabled(false);

										mKeyboardHandler = NULL;
										mMouseHandler = NULL;
										mCamera->SetTarget(mPlayer);
										break;
									}
								}
								else if (pView->GetType() == GV_AI)
								{
									eastl::shared_ptr<QuakeAIView> pAiView =
										eastl::static_pointer_cast<QuakeAIView, BaseGameView>(pView);
									if (pAiView->GetActorId() != mPlayer->GetId())
									{
										mPlayer = mScene->GetSceneNode(pAiView->GetActorId());

										if (mGamePlayerController)
											mGamePlayerController->SetEnabled(false);
										if (mGameCameraController)
											mGameCameraController->SetEnabled(false);

										mKeyboardHandler = NULL;
										mMouseHandler = NULL;
										mCamera->SetTarget(mPlayer);
										break;
									}
								}
							}
						}
						else
						{
							GameApplication* gameApp = (GameApplication*)Application::App;
							if (gameApp->mOption.mLevelMod)
							{
								mPlayer = mScene->GetSceneNode(mActorId);

								if (mGamePlayerController)
									mGamePlayerController->SetEnabled(false);
								if (mGameCameraController)
									mGameCameraController->SetEnabled(false);

								mKeyboardHandler = NULL;
								mMouseHandler = NULL;
								mCamera->SetTarget(mPlayer);
							}
							else
							{
								SetControlledActor(mActorId);

								if (mGamePlayerController)
									mGamePlayerController->SetEnabled(true);
								if (mGameCameraController)
									mGameCameraController->SetEnabled(false);

								mKeyboardHandler = mGamePlayerController;
								mMouseHandler = mGamePlayerController;
								mCamera->SetTarget(mPlayer);
							}
						}
						return true;
					}

					case KEY_KEY_9:
					{
						if (mGamePlayerController)
							mGamePlayerController->SetEnabled(false);
						if (mGameCameraController)
							mGameCameraController->SetEnabled(true);

						mKeyboardHandler = mGameCameraController;
						mMouseHandler = mGameCameraController;
						mCamera->ClearTarget();
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
// QuakeHumanView::RenderText				- Chapter 19, page 727
//
void QuakeHumanView::RenderText()
{
	HumanView::RenderText();
}

//
// QuakeHumanView::OnUpdate				- Chapter 19, page 730
//
void QuakeHumanView::OnUpdate(unsigned int timeMs, unsigned long deltaMs)
{
	HumanView::OnUpdate( timeMs, deltaMs );

	if (mGameCameraController)
	{
		mGameCameraController->OnUpdate(timeMs, deltaMs);
	}

	if (mGamePlayerController)
	{
		mGamePlayerController->OnUpdate(timeMs, deltaMs);
	}
}

//
// QuakeHumanView::OnAttach				- Chapter 19, page 731
//
void QuakeHumanView::OnAttach(GameViewId vid, ActorId aid)
{
	HumanView::OnAttach(vid, aid);
}

bool QuakeHumanView::LoadGameDelegate(tinyxml2::XMLElement* pLevelData)
{
	if (!HumanView::LoadGameDelegate(pLevelData))
		return false;

    mGameStandardHUD.reset(new QuakeStandardHUD(shared_from_this()));
	mGameStandardHUD->OnInit();
    PushElement(mGameStandardHUD);

    // A movement controller is going to control the camera, 
    // but it could be constructed with any of the objects you see in this function.
    mGameCameraController.reset(new QuakeCameraController(mCamera, 0, 0, false));
	mKeyboardHandler = mGameCameraController;
	mMouseHandler = mGameCameraController;
	mCamera->ClearTarget();

    mScene->OnRestore();
    return true;
}

void QuakeHumanView::SetControlledActor(ActorId actorId)
{ 
	mPlayer = mScene->GetSceneNode(actorId);
    if (!mPlayer)
    {
        LogError("Invalid player");
        return;
    }

	HumanView::SetControlledActor(actorId);

	eastl::shared_ptr<Actor> pGameActor(GameLogic::Get()->GetActor(actorId).lock());
	eastl::shared_ptr<TransformComponent> pTransformComponent(
		pGameActor->GetComponent<TransformComponent>(TransformComponent::Name).lock());
	if (pTransformComponent)
	{
		EulerAngles<float> yawPitchRoll;
		yawPitchRoll.mAxis[1] = 1;
		yawPitchRoll.mAxis[2] = 2;
		pTransformComponent->GetTransform().GetRotation(yawPitchRoll);
		mGamePlayerController.reset(new QuakePlayerController(mPlayer, 
			yawPitchRoll.mAngle[YAW] * (float)GE_C_RAD_TO_DEG, 
			yawPitchRoll.mAngle[ROLL] * (float)GE_C_RAD_TO_DEG));
		mKeyboardHandler = mGamePlayerController;
		mMouseHandler = mGamePlayerController;
	}
}

// RegisterWeapons called during a precache command
void QuakeHumanView::RegisterWeapon(unsigned int weapon)
{
	eastl::shared_ptr<ResHandle> resHandle;
	eastl::shared_ptr<ImageResourceExtraData> resData;

	WeaponType wp = (WeaponType)(weapon + 1);
	switch (wp)
	{
		case WP_GAUNTLET:

			// load cmodel before model so filecache works
			mWeaponMedia[weapon].weaponIcon = eastl::wstring(L"art/quake/icons/iconw_gauntlet.png");
			ResCache::Get()->GetHandle(&BaseResource(mWeaponMedia[weapon].weaponIcon));
			mWeaponMedia[weapon].weaponModel = eastl::wstring(L"art/quake/models/weapons2/gauntlet/gauntlet.md3");
			ResCache::Get()->GetHandle(&BaseResource(mWeaponMedia[weapon].weaponModel));
			mWeaponMedia[weapon].ammoIcon = eastl::wstring();
			mWeaponMedia[weapon].ammoModel = eastl::wstring();

			mWeaponMedia[weapon].flashModel = eastl::make_shared<MD3Mesh>("gauntlet_flash");
			mWeaponMedia[weapon].flashModel->LoadModel(ToWideString(
				FileSystem::Get()->GetPath("art/quake/models/weapons2/gauntlet/gauntlet_flash.md3").c_str()));
			mWeaponMedia[weapon].barrelModel = eastl::make_shared<MD3Mesh>("gauntlet_barrel");
			mWeaponMedia[weapon].barrelModel->LoadModel(ToWideString(
				FileSystem::Get()->GetPath("art/quake/models/weapons2/gauntlet/gauntlet_barrel.md3").c_str()));
			mWeaponMedia[weapon].handsModel = eastl::make_shared<MD3Mesh>("gauntlet_hand");
			mWeaponMedia[weapon].handsModel->LoadModel(ToWideString(
				FileSystem::Get()->GetPath("art/quake/models/weapons2/gauntlet/gauntlet_hand.md3").c_str()));

			mWeaponMedia[weapon].loopFireSound = false;

			mWeaponMedia[weapon].flashDlightColor = eastl::array<float, 4U>{ 0.6f, 0.6f, 1.0f, 1.0f};
			mWeaponMedia[weapon].firingSound = eastl::wstring(L"audio/quake/sound/weapons/melee/fstrun.ogg");
			ResCache::Get()->GetHandle(&BaseResource(mWeaponMedia[weapon].firingSound));
			mWeaponMedia[weapon].flashSound[0] = eastl::wstring(L"audio/quake/sound/weapons/melee/fstatck.ogg");
			ResCache::Get()->GetHandle(&BaseResource(mWeaponMedia[weapon].flashSound[0]));
			break;

		case WP_LIGHTNING:

			// load cmodel before model so filecache works
			mWeaponMedia[weapon].weaponIcon = eastl::wstring(L"art/quake/icons/iconw_lightning.png");
			ResCache::Get()->GetHandle(&BaseResource(mWeaponMedia[weapon].weaponIcon));
			mWeaponMedia[weapon].weaponModel = eastl::wstring(L"art/quake/models/weapons2/lightning/lightning.md3");
			ResCache::Get()->GetHandle(&BaseResource(mWeaponMedia[weapon].weaponModel));
			mWeaponMedia[weapon].ammoIcon = eastl::wstring(L"art/quake/icons/icona_lightning.png");
			ResCache::Get()->GetHandle(&BaseResource(mWeaponMedia[weapon].ammoIcon));
			mWeaponMedia[weapon].ammoModel = eastl::wstring(L"art/quake/models/powerups/ammo/lightningam.md3");
			ResCache::Get()->GetHandle(&BaseResource(mWeaponMedia[weapon].ammoModel));

			mWeaponMedia[weapon].flashModel = eastl::make_shared<MD3Mesh>("lightning_flash");
			mWeaponMedia[weapon].flashModel->LoadModel(ToWideString(
				FileSystem::Get()->GetPath("art/quake/models/weapons2/lightning/lightning_flash.md3").c_str()));
			mWeaponMedia[weapon].barrelModel = eastl::make_shared<MD3Mesh>();
			mWeaponMedia[weapon].handsModel = eastl::make_shared<MD3Mesh>("lightning_hand");
			mWeaponMedia[weapon].handsModel->LoadModel(ToWideString(
				FileSystem::Get()->GetPath("art/quake/models/weapons2/lightning/lightning_hand.md3").c_str()));

			mWeaponMedia[weapon].loopFireSound = false;

			resHandle = ResCache::Get()->GetHandle(&BaseResource(eastl::wstring(L"art/quake/gfx/misc/sun.jpg")));
			resData = eastl::static_pointer_cast<ImageResourceExtraData>(resHandle->GetExtra());
			resData->GetImage()->AutogenerateMipmaps();

			mMedia.lightningShader = eastl::wstring(L"art/quake/gfx/misc/lightning3new.jpg");
			resHandle = ResCache::Get()->GetHandle(&BaseResource(mMedia.lightningShader));
			resData = eastl::static_pointer_cast<ImageResourceExtraData>(resHandle->GetExtra());
			resData->GetImage()->AutogenerateMipmaps();
			mMedia.lightningExplosionModel = eastl::wstring(L"art/quake/models/weaphits/crackle.md3");
			ResCache::Get()->GetHandle(&BaseResource(mMedia.lightningExplosionModel));

			mWeaponMedia[weapon].flashDlightColor = eastl::array<float, 4U>{ 0.6f, 0.6f, 1.0f, 1.0f};
			mWeaponMedia[weapon].readySound = eastl::wstring(L"audio/quake/sound/weapons/melee/fsthum.ogg");
			ResCache::Get()->GetHandle(&BaseResource(mWeaponMedia[weapon].readySound));
			mWeaponMedia[weapon].firingSound = eastl::wstring(L"audio/quake/sound/weapons/lightning/lg_hum.ogg");
			ResCache::Get()->GetHandle(&BaseResource(mWeaponMedia[weapon].firingSound));
			mWeaponMedia[weapon].flashSound[0] = eastl::wstring(L"audio/quake/sound/weapons/lightning/lg_fire.ogg");
			ResCache::Get()->GetHandle(&BaseResource(mWeaponMedia[weapon].flashSound[0]));

			mMedia.sfxLghit1 = eastl::wstring(L"audio/quake/sound/weapons/lightning/lg_hit.ogg");
			ResCache::Get()->GetHandle(&BaseResource(mMedia.sfxLghit1));
			mMedia.sfxLghit2 = eastl::wstring(L"audio/quake/sound/weapons/lightning/lg_hit2.ogg");
			ResCache::Get()->GetHandle(&BaseResource(mMedia.sfxLghit2));
			mMedia.sfxLghit3 = eastl::wstring(L"audio/quake/sound/weapons/lightning/lg_hit3.ogg");
			ResCache::Get()->GetHandle(&BaseResource(mMedia.sfxLghit3));
			break;

		case WP_MACHINEGUN:

			// load cmodel before model so filecache works
			mWeaponMedia[weapon].weaponIcon = eastl::wstring(L"art/quake/icons/iconw_machinegun.png");
			ResCache::Get()->GetHandle(&BaseResource(mWeaponMedia[weapon].weaponIcon));
			mWeaponMedia[weapon].weaponModel = eastl::wstring(L"art/quake/models/weapons2/machinegun/machinegun.md3");
			ResCache::Get()->GetHandle(&BaseResource(mWeaponMedia[weapon].weaponModel));
			mWeaponMedia[weapon].ammoIcon = eastl::wstring(L"art/quake/icons/icona_machinegun.png");
			ResCache::Get()->GetHandle(&BaseResource(mWeaponMedia[weapon].ammoIcon));
			mWeaponMedia[weapon].ammoModel = eastl::wstring(L"art/quake/models/powerups/ammo/machinegunam.md3");
			ResCache::Get()->GetHandle(&BaseResource(mWeaponMedia[weapon].ammoModel));

			mWeaponMedia[weapon].flashModel = eastl::make_shared<MD3Mesh>("machinegun_flash");
			mWeaponMedia[weapon].flashModel->LoadModel(ToWideString(
				FileSystem::Get()->GetPath("art/quake/models/weapons2/machinegun/machinegun_flash.md3").c_str()));
			mWeaponMedia[weapon].barrelModel = eastl::make_shared<MD3Mesh>("machinegun_barrel");
			mWeaponMedia[weapon].barrelModel->LoadModel(ToWideString(
				FileSystem::Get()->GetPath("art/quake/models/weapons2/machinegun/machinegun_barrel.md3").c_str()));
			mWeaponMedia[weapon].handsModel = eastl::make_shared<MD3Mesh>("machinegun_hand");
			mWeaponMedia[weapon].handsModel->LoadModel(ToWideString(
				FileSystem::Get()->GetPath("art/quake/models/weapons2/machinegun/machinegun_hand.md3").c_str()));

			mWeaponMedia[weapon].loopFireSound = false;

			//mWeaponMedia[weapon].ejectBrassFunc = CG_MachineGunEjectBrass;
			mMedia.bulletExplosionShader[0] = eastl::wstring(L"art/quake/models/weaphits/bullet1.jpg");
			resHandle = ResCache::Get()->GetHandle(&BaseResource(mMedia.bulletExplosionShader[0]));
			resData = eastl::static_pointer_cast<ImageResourceExtraData>(resHandle->GetExtra());
			resData->GetImage()->AutogenerateMipmaps();
			mMedia.bulletExplosionShader[1] = eastl::wstring(L"art/quake/models/weaphits/bullet2.jpg");
			resHandle = ResCache::Get()->GetHandle(&BaseResource(mMedia.bulletExplosionShader[1]));
			resData = eastl::static_pointer_cast<ImageResourceExtraData>(resHandle->GetExtra());
			resData->GetImage()->AutogenerateMipmaps();
			mMedia.bulletExplosionShader[2] = eastl::wstring(L"art/quake/models/weaphits/bullet3.jpg");
			resHandle = ResCache::Get()->GetHandle(&BaseResource(mMedia.bulletExplosionShader[2]));
			resData = eastl::static_pointer_cast<ImageResourceExtraData>(resHandle->GetExtra());
			resData->GetImage()->AutogenerateMipmaps();
			mMedia.bulletExplosionShader[3] = eastl::wstring(L"art/quake/gfx/colors/black.jpg");
			resHandle = ResCache::Get()->GetHandle(&BaseResource(mMedia.bulletExplosionShader[3]));
			resData = eastl::static_pointer_cast<ImageResourceExtraData>(resHandle->GetExtra());
			resData->GetImage()->AutogenerateMipmaps();

			mWeaponMedia[weapon].flashDlightColor = eastl::array<float, 4U>{ 1.f, 1.f, 0.f, 1.f};
			mWeaponMedia[weapon].flashSound[0] = eastl::wstring(L"audio/quake/sound/weapons/machinegun/machgf1b.ogg");
			ResCache::Get()->GetHandle(&BaseResource(mWeaponMedia[weapon].flashSound[0]));
			mWeaponMedia[weapon].flashSound[1] = eastl::wstring(L"audio/quake/sound/weapons/machinegun/machgf2b.ogg");
			ResCache::Get()->GetHandle(&BaseResource(mWeaponMedia[weapon].flashSound[1]));
			mWeaponMedia[weapon].flashSound[2] = eastl::wstring(L"audio/quake/sound/weapons/machinegun/machgf3b.ogg");
			ResCache::Get()->GetHandle(&BaseResource(mWeaponMedia[weapon].flashSound[2]));
			mWeaponMedia[weapon].flashSound[3] = eastl::wstring(L"audio/quake/sound/weapons/machinegun/machgf4b.ogg");
			ResCache::Get()->GetHandle(&BaseResource(mWeaponMedia[weapon].flashSound[3]));

			break;

		case WP_SHOTGUN:

			// load cmodel before model so filecache works
			mWeaponMedia[weapon].weaponIcon = eastl::wstring(L"art/quake/icons/iconw_shotgun.png");
			ResCache::Get()->GetHandle(&BaseResource(mWeaponMedia[weapon].weaponIcon));
			mWeaponMedia[weapon].weaponModel = eastl::wstring(L"art/quake/models/weapons2/shotgun/shotgun.md3");
			ResCache::Get()->GetHandle(&BaseResource(mWeaponMedia[weapon].weaponModel));
			mWeaponMedia[weapon].ammoIcon = eastl::wstring(L"art/quake/icons/icona_shotgun.png");
			resHandle = ResCache::Get()->GetHandle(&BaseResource(mWeaponMedia[weapon].ammoIcon));
			mWeaponMedia[weapon].ammoModel = eastl::wstring(L"art/quake/models/powerups/ammo/shotgunam.md3");
			ResCache::Get()->GetHandle(&BaseResource(mWeaponMedia[weapon].ammoModel));

			mWeaponMedia[weapon].flashModel = eastl::make_shared<MD3Mesh>("shotgun_flash");
			mWeaponMedia[weapon].flashModel->LoadModel(ToWideString(
				FileSystem::Get()->GetPath("art/quake/models/weapons2/shotgun/shotgun_flash.md3").c_str()));
			mWeaponMedia[weapon].barrelModel = eastl::make_shared<MD3Mesh>();
			mWeaponMedia[weapon].handsModel = eastl::make_shared<MD3Mesh>("shotgun_hand");
			mWeaponMedia[weapon].handsModel->LoadModel(ToWideString(
				FileSystem::Get()->GetPath("art/quake/models/weapons2/shotgun/shotgun_hand.md3").c_str()));

			mWeaponMedia[weapon].loopFireSound = false;

			//mWeaponMedia[weapon].ejectBrassFunc = CG_ShotgunEjectBrass;

			mWeaponMedia[weapon].flashDlightColor = eastl::array<float, 4U>{ 1.f, 1.f, 0.f, 1.f};
			mWeaponMedia[weapon].flashSound[0] = eastl::wstring(L"audio/quake/sound/weapons/shotgun/sshotf1b.ogg");
			ResCache::Get()->GetHandle(&BaseResource(mWeaponMedia[weapon].flashSound[0]));
			break;

		case WP_ROCKET_LAUNCHER:

			// load cmodel before model so filecache works
			mWeaponMedia[weapon].weaponIcon = eastl::wstring(L"art/quake/icons/iconw_rocket.png");
			ResCache::Get()->GetHandle(&BaseResource(mWeaponMedia[weapon].weaponIcon));
			mWeaponMedia[weapon].weaponModel = eastl::wstring(L"art/quake/models/weapons2/rocketl/rocketl.md3");
			ResCache::Get()->GetHandle(&BaseResource(mWeaponMedia[weapon].weaponModel));
			mWeaponMedia[weapon].ammoIcon = eastl::wstring(L"art/quake/icons/icona_rocket.png");
			ResCache::Get()->GetHandle(&BaseResource(mWeaponMedia[weapon].ammoIcon));
			mWeaponMedia[weapon].ammoModel = eastl::wstring(L"art/quake/models/powerups/ammo/rocketam.md3");
			ResCache::Get()->GetHandle(&BaseResource(mWeaponMedia[weapon].ammoModel));

			mWeaponMedia[weapon].flashModel = eastl::make_shared<MD3Mesh>("rocketl_flash");
			mWeaponMedia[weapon].flashModel->LoadModel(ToWideString(
				FileSystem::Get()->GetPath("art/quake/models/weapons2/rocketl/rocketl_flash.md3").c_str()));
			mWeaponMedia[weapon].barrelModel = eastl::make_shared<MD3Mesh>();
			mWeaponMedia[weapon].handsModel = eastl::make_shared<MD3Mesh>("rocketl_hand");
			mWeaponMedia[weapon].handsModel->LoadModel(ToWideString(
				FileSystem::Get()->GetPath("art/quake/models/weapons2/rocketl/rocketl_hand.md3").c_str()));

			mWeaponMedia[weapon].loopFireSound = false;

			mMedia.rocketExplosionShader[0] = eastl::wstring(L"art/quake/models/weaphits/rlboom/rlboom_1.jpg");
			resHandle = ResCache::Get()->GetHandle(&BaseResource(mMedia.rocketExplosionShader[0]));
			resData = eastl::static_pointer_cast<ImageResourceExtraData>(resHandle->GetExtra());
			resData->GetImage()->AutogenerateMipmaps();
			mMedia.rocketExplosionShader[1] = eastl::wstring(L"art/quake/models/weaphits/rlboom/rlboom_2.jpg");
			resHandle = ResCache::Get()->GetHandle(&BaseResource(mMedia.rocketExplosionShader[1]));
			resData = eastl::static_pointer_cast<ImageResourceExtraData>(resHandle->GetExtra());
			resData->GetImage()->AutogenerateMipmaps();
			mMedia.rocketExplosionShader[2] = eastl::wstring(L"art/quake/models/weaphits/rlboom/rlboom_3.jpg");
			resHandle = ResCache::Get()->GetHandle(&BaseResource(mMedia.rocketExplosionShader[2]));
			resData = eastl::static_pointer_cast<ImageResourceExtraData>(resHandle->GetExtra());
			resData->GetImage()->AutogenerateMipmaps();
			mMedia.rocketExplosionShader[3] = eastl::wstring(L"art/quake/models/weaphits/rlboom/rlboom_4.jpg");
			resHandle = ResCache::Get()->GetHandle(&BaseResource(mMedia.rocketExplosionShader[3]));
			resData = eastl::static_pointer_cast<ImageResourceExtraData>(resHandle->GetExtra());
			resData->GetImage()->AutogenerateMipmaps();
			mMedia.rocketExplosionShader[4] = eastl::wstring(L"art/quake/models/weaphits/rlboom/rlboom_5.jpg");
			resHandle = ResCache::Get()->GetHandle(&BaseResource(mMedia.rocketExplosionShader[4]));
			resData = eastl::static_pointer_cast<ImageResourceExtraData>(resHandle->GetExtra());
			resData->GetImage()->AutogenerateMipmaps();
			mMedia.rocketExplosionShader[5] = eastl::wstring(L"art/quake/models/weaphits/rlboom/rlboom_6.jpg");
			resHandle = ResCache::Get()->GetHandle(&BaseResource(mMedia.rocketExplosionShader[5]));
			resData = eastl::static_pointer_cast<ImageResourceExtraData>(resHandle->GetExtra());
			resData->GetImage()->AutogenerateMipmaps();
			mMedia.rocketExplosionShader[6] = eastl::wstring(L"art/quake/models/weaphits/rlboom/rlboom_7.jpg");
			resHandle = ResCache::Get()->GetHandle(&BaseResource(mMedia.rocketExplosionShader[6]));
			resData = eastl::static_pointer_cast<ImageResourceExtraData>(resHandle->GetExtra());
			resData->GetImage()->AutogenerateMipmaps();
			mMedia.rocketExplosionShader[7] = eastl::wstring(L"art/quake/models/weaphits/rlboom/rlboom_8.jpg");
			resHandle = ResCache::Get()->GetHandle(&BaseResource(mMedia.rocketExplosionShader[7]));
			resData = eastl::static_pointer_cast<ImageResourceExtraData>(resHandle->GetExtra());
			resData->GetImage()->AutogenerateMipmaps();

			mWeaponMedia[weapon].missileModel = eastl::wstring(L"art/quake/models/ammo/rocket/rocket.md3");
			ResCache::Get()->GetHandle(&BaseResource(mWeaponMedia[weapon].missileModel));
			mWeaponMedia[weapon].missileSound = eastl::wstring(L"audio/quake/sound/weapons/rocket/rockfly.ogg");
			ResCache::Get()->GetHandle(&BaseResource(mWeaponMedia[weapon].missileSound));
			//mWeaponMedia[weapon].missileTrailFunc = CG_RocketTrail;
			mWeaponMedia[weapon].missileDlight = 200;
			mWeaponMedia[weapon].wiTrailTime = 2000;
			mWeaponMedia[weapon].trailRadius = 64;

			mWeaponMedia[weapon].missileDlightColor = eastl::array<float, 4U>{ 1.f, 0.75f, 0.f, 1.f};
			mWeaponMedia[weapon].flashDlightColor = eastl::array<float, 4U>{ 1.f, 0.75f, 0.f, 1.f};
			mWeaponMedia[weapon].flashSound[0] = eastl::wstring(L"audio/quake/sound/weapons/rocket/rocklf1a.ogg");
			ResCache::Get()->GetHandle(&BaseResource(mWeaponMedia[weapon].flashSound[0]));
			break;

		case WP_GRENADE_LAUNCHER:

			// load cmodel before model so filecache works
			mWeaponMedia[weapon].weaponIcon = eastl::wstring(L"art/quake/icons/iconw_grenade.png");
			ResCache::Get()->GetHandle(&BaseResource(mWeaponMedia[weapon].weaponIcon));
			mWeaponMedia[weapon].weaponModel = eastl::wstring(L"art/quake/models/weapons2/grenadel/grenadel.md3");
			ResCache::Get()->GetHandle(&BaseResource(mWeaponMedia[weapon].weaponModel));
			mWeaponMedia[weapon].ammoIcon = eastl::wstring(L"art/quake/icons/icona_grenade.png");
			ResCache::Get()->GetHandle(&BaseResource(mWeaponMedia[weapon].ammoIcon));
			mWeaponMedia[weapon].ammoModel = eastl::wstring(L"art/quake/models/powerups/ammo/grenadeam.md3");
			ResCache::Get()->GetHandle(&BaseResource(mWeaponMedia[weapon].ammoModel));

			mWeaponMedia[weapon].flashModel = eastl::make_shared<MD3Mesh>("grenadel_flash");
			mWeaponMedia[weapon].flashModel->LoadModel(ToWideString(
				FileSystem::Get()->GetPath("art/quake/models/weapons2/grenadel/grenadel_flash.md3").c_str()));
			mWeaponMedia[weapon].barrelModel = eastl::make_shared<MD3Mesh>();
			mWeaponMedia[weapon].handsModel = eastl::make_shared<MD3Mesh>("grenadel_hand");
			mWeaponMedia[weapon].handsModel->LoadModel(ToWideString(
				FileSystem::Get()->GetPath("art/quake/models/weapons2/grenadel/grenadel_hand.md3").c_str()));

			mWeaponMedia[weapon].loopFireSound = false;

			mMedia.grenadeExplosionShader[0] = eastl::wstring(L"art/quake/models/weaphits/glboom/glboom_1.jpg");
			resHandle = ResCache::Get()->GetHandle(&BaseResource(mMedia.grenadeExplosionShader[0]));
			resData = eastl::static_pointer_cast<ImageResourceExtraData>(resHandle->GetExtra());
			resData->GetImage()->AutogenerateMipmaps();
			mMedia.grenadeExplosionShader[1] = eastl::wstring(L"art/quake/models/weaphits/glboom/glboom_2.jpg");
			resHandle = ResCache::Get()->GetHandle(&BaseResource(mMedia.grenadeExplosionShader[1]));
			resData = eastl::static_pointer_cast<ImageResourceExtraData>(resHandle->GetExtra());
			resData->GetImage()->AutogenerateMipmaps();
			mMedia.grenadeExplosionShader[2] = eastl::wstring(L"art/quake/models/weaphits/glboom/glboom_3.jpg");
			resHandle = ResCache::Get()->GetHandle(&BaseResource(mMedia.grenadeExplosionShader[2]));
			resData = eastl::static_pointer_cast<ImageResourceExtraData>(resHandle->GetExtra());
			resData->GetImage()->AutogenerateMipmaps();
			mMedia.grenadeExplosionShader[3] = eastl::wstring(L"art/quake/gfx/colors/black.jpg");
			resHandle = ResCache::Get()->GetHandle(&BaseResource(mMedia.grenadeExplosionShader[3]));
			resData = eastl::static_pointer_cast<ImageResourceExtraData>(resHandle->GetExtra());
			resData->GetImage()->AutogenerateMipmaps();

			mWeaponMedia[weapon].missileModel = eastl::wstring(L"art/quake/models/ammo/grenade/grenade1.md3");
			ResCache::Get()->GetHandle(&BaseResource(mWeaponMedia[weapon].missileModel));

			//mWeaponMedia[weapon].missileTrailFunc = CG_GrenadeTrail;
			mWeaponMedia[weapon].wiTrailTime = 700;
			mWeaponMedia[weapon].trailRadius = 32;

			mWeaponMedia[weapon].flashDlightColor = eastl::array<float, 4U>{ 1.f, 0.7f, 0.f, 1.f};
			mWeaponMedia[weapon].flashSound[0] = eastl::wstring(L"audio/quake/sound/weapons/grenade/grenlf1a.ogg");
			ResCache::Get()->GetHandle(&BaseResource(mWeaponMedia[weapon].flashSound[0]));
			break;

		case WP_PLASMAGUN:

			// load cmodel before model so filecache works
			mWeaponMedia[weapon].weaponIcon = eastl::wstring(L"art/quake/icons/iconw_plasma.png");
			ResCache::Get()->GetHandle(&BaseResource(mWeaponMedia[weapon].weaponIcon));
			mWeaponMedia[weapon].weaponModel = eastl::wstring(L"art/quake/models/weapons2/plasma/plasma.md3");
			ResCache::Get()->GetHandle(&BaseResource(mWeaponMedia[weapon].weaponModel));
			mWeaponMedia[weapon].ammoIcon = eastl::wstring(L"art/quake/icons/icona_plasma.png");
			ResCache::Get()->GetHandle(&BaseResource(mWeaponMedia[weapon].ammoIcon));
			mWeaponMedia[weapon].ammoModel = eastl::wstring(L"art/quake/models/powerups/ammo/plasmaam.md3");
			ResCache::Get()->GetHandle(&BaseResource(mWeaponMedia[weapon].ammoModel));

			mWeaponMedia[weapon].flashModel = eastl::make_shared<MD3Mesh>("plasma_flash");
			mWeaponMedia[weapon].flashModel->LoadModel(ToWideString(
				FileSystem::Get()->GetPath("art/quake/models/weapons2/plasma/plasma_flash.md3").c_str()));
			mWeaponMedia[weapon].barrelModel = eastl::make_shared<MD3Mesh>();
			mWeaponMedia[weapon].handsModel = eastl::make_shared<MD3Mesh>("plasma_hand");
			mWeaponMedia[weapon].handsModel->LoadModel(ToWideString(
				FileSystem::Get()->GetPath("art/quake/models/weapons2/plasma/plasma_hand.md3").c_str()));

			mWeaponMedia[weapon].loopFireSound = false;

			mMedia.plasmaExplosionShader = eastl::wstring(L"art/quake/models/weaphits/plasmaboom.jpg");
			resHandle = ResCache::Get()->GetHandle(&BaseResource(mMedia.plasmaExplosionShader));
			resData = eastl::static_pointer_cast<ImageResourceExtraData>(resHandle->GetExtra());
			resData->GetImage()->AutogenerateMipmaps();
			mMedia.railRingsShader = eastl::wstring(L"art/quake/gfx/misc/raildisc_mono2.jpg");
			resHandle = ResCache::Get()->GetHandle(&BaseResource(mMedia.railRingsShader));
			resData = eastl::static_pointer_cast<ImageResourceExtraData>(resHandle->GetExtra());
			resData->GetImage()->AutogenerateMipmaps();

			//weaponInfo->missileModel = cgs.media.invulnerabilityPowerupModel;
			//mWeaponMedia[weapon].missileTrailFunc = CG_PlasmaTrail;
			mWeaponMedia[weapon].flashDlightColor = eastl::array<float, 4U>{ 1.f, 0.6f, 0.6f, 1.f};
			mWeaponMedia[weapon].missileSound = eastl::wstring(L"audio/quake/sound/weapons/plasma/lasfly.ogg");
			ResCache::Get()->GetHandle(&BaseResource(mWeaponMedia[weapon].missileSound));
			mWeaponMedia[weapon].flashSound[0] = eastl::wstring(L"audio/quake/sound/weapons/plasma/hyprbf1a.ogg");
			ResCache::Get()->GetHandle(&BaseResource(mWeaponMedia[weapon].flashSound[0]));
			break;

		case WP_RAILGUN:

			// load cmodel before model so filecache works
			mWeaponMedia[weapon].weaponIcon = eastl::wstring(L"art/quake/icons/iconw_railgun.png");
			ResCache::Get()->GetHandle(&BaseResource(mWeaponMedia[weapon].weaponIcon));
			mWeaponMedia[weapon].weaponModel = eastl::wstring(L"art/quake/models/weapons2/railgun/railgun.md3");
			ResCache::Get()->GetHandle(&BaseResource(mWeaponMedia[weapon].weaponModel));
			mWeaponMedia[weapon].ammoIcon = eastl::wstring(L"art/quake/icons/icona_railgun.png");
			ResCache::Get()->GetHandle(&BaseResource(mWeaponMedia[weapon].ammoIcon));
			mWeaponMedia[weapon].ammoModel = eastl::wstring(L"art/quake/models/powerups/ammo/railgunam.md3");
			ResCache::Get()->GetHandle(&BaseResource(mWeaponMedia[weapon].ammoModel));

			mWeaponMedia[weapon].flashModel = eastl::make_shared<MD3Mesh>("railgun_flash");
			mWeaponMedia[weapon].flashModel->LoadModel(ToWideString(
				FileSystem::Get()->GetPath("art/quake/models/weapons2/railgun/railgun_flash.md3").c_str()));
			mWeaponMedia[weapon].barrelModel = eastl::make_shared<MD3Mesh>();
			mWeaponMedia[weapon].handsModel = eastl::make_shared<MD3Mesh>("railgun_hand");
			mWeaponMedia[weapon].handsModel->LoadModel(ToWideString(
				FileSystem::Get()->GetPath("art/quake/models/weapons2/railgun/railgun_hand.md3").c_str()));

			mWeaponMedia[weapon].loopFireSound = false;

			char name[64];
			for (int i = 1; i < 8; i++)
			{
				sprintf(name, "art/quake/gfx/misc/portal%i.bmp", i);
				resHandle = ResCache::Get()->GetHandle(&BaseResource(ToWideString(name)));
				resData = eastl::static_pointer_cast<ImageResourceExtraData>(resHandle->GetExtra());
				resData->GetImage()->AutogenerateMipmaps();
			}

			mMedia.railExplosionShader[0] = eastl::wstring(L"art/quake/models/weaphits/ring02_1.png");
			resHandle = ResCache::Get()->GetHandle(&BaseResource(mMedia.railExplosionShader[0]));
			resData = eastl::static_pointer_cast<ImageResourceExtraData>(resHandle->GetExtra());
			resData->GetImage()->AutogenerateMipmaps();
			mMedia.railExplosionShader[1] = eastl::wstring(L"art/quake/models/weaphits/ring02_2.png");
			resHandle = ResCache::Get()->GetHandle(&BaseResource(mMedia.railExplosionShader[1]));
			resData = eastl::static_pointer_cast<ImageResourceExtraData>(resHandle->GetExtra());
			resData->GetImage()->AutogenerateMipmaps();
			mMedia.railExplosionShader[2] = eastl::wstring(L"art/quake/models/weaphits/ring02_3.png");
			resHandle = ResCache::Get()->GetHandle(&BaseResource(mMedia.railExplosionShader[2]));
			resData = eastl::static_pointer_cast<ImageResourceExtraData>(resHandle->GetExtra());
			resData->GetImage()->AutogenerateMipmaps();
			mMedia.railExplosionShader[3] = eastl::wstring(L"art/quake/models/weaphits/ring02_4.png");
			resHandle = ResCache::Get()->GetHandle(&BaseResource(mMedia.railExplosionShader[3]));
			resData = eastl::static_pointer_cast<ImageResourceExtraData>(resHandle->GetExtra());
			resData->GetImage()->AutogenerateMipmaps();
			mMedia.railExplosionShader[4] = eastl::wstring(L"art/quake/gfx/colors/black.jpg");
			resHandle = ResCache::Get()->GetHandle(&BaseResource(mMedia.railExplosionShader[4]));
			resData = eastl::static_pointer_cast<ImageResourceExtraData>(resHandle->GetExtra());
			resData->GetImage()->AutogenerateMipmaps();

			mMedia.railRingsShader = eastl::wstring(L"art/quake/gfx/misc/raildisc_mono2.jpg");
			resHandle = ResCache::Get()->GetHandle(&BaseResource(mMedia.railRingsShader));
			resData = eastl::static_pointer_cast<ImageResourceExtraData>(resHandle->GetExtra());
			resData->GetImage()->AutogenerateMipmaps();
			mMedia.railCoreShader = eastl::wstring(L"art/quake/gfx/misc/railcorethin_mono.jpg");
			resHandle = ResCache::Get()->GetHandle(&BaseResource(mMedia.railCoreShader));
			resData = eastl::static_pointer_cast<ImageResourceExtraData>(resHandle->GetExtra());
			resData->GetImage()->AutogenerateMipmaps();

			mWeaponMedia[weapon].flashDlightColor = eastl::array<float, 4U>{ 1.f, 0.5f, 0.f, 1.f};
			mWeaponMedia[weapon].readySound = eastl::wstring(L"audio/quake/sound/weapons/railgun/rg_hum.ogg");
			ResCache::Get()->GetHandle(&BaseResource(mWeaponMedia[weapon].readySound));
			mWeaponMedia[weapon].flashSound[0] = eastl::wstring(L"audio/quake/sound/weapons/railgun/railgf1a.ogg");
			ResCache::Get()->GetHandle(&BaseResource(mWeaponMedia[weapon].flashSound[0]));
			break;

		default:
			mWeaponMedia[weapon].flashDlightColor = eastl::array<float, 4U>{ 1.f, 1.f, 1.f, 1.f};
			mWeaponMedia[weapon].flashSound[0] = eastl::wstring(L"audio/quake/sound/weapons/rocket/rocklf1a.ogg");
			ResCache::Get()->GetHandle(&BaseResource(mWeaponMedia[weapon].flashSound[0]));
			break;
	}
}

// RegisterSounds called during a precache command
void QuakeHumanView::RegisterSound() 
{
	mMedia.oneMinuteSound = eastl::wstring(ToWideString("audio/quake/sound/feedback/1_minute.ogg"));
	ResCache::Get()->GetHandle(&BaseResource(mMedia.oneMinuteSound));
	mMedia.fiveMinuteSound = eastl::wstring(ToWideString("audio/quake/sound/feedback/5_minute.ogg"));
	ResCache::Get()->GetHandle(&BaseResource(mMedia.fiveMinuteSound));
	mMedia.suddenDeathSound = eastl::wstring(ToWideString("audio/quake/sound/feedback/sudden_death.ogg"));
	ResCache::Get()->GetHandle(&BaseResource(mMedia.suddenDeathSound));
	mMedia.oneFragSound = eastl::wstring(ToWideString("audio/quake/sound/feedback/1_frag.ogg"));
	ResCache::Get()->GetHandle(&BaseResource(mMedia.oneFragSound));
	mMedia.twoFragSound = eastl::wstring(ToWideString("audio/quake/sound/feedback/2_frags.ogg"));
	ResCache::Get()->GetHandle(&BaseResource(mMedia.twoFragSound));
	mMedia.threeFragSound = eastl::wstring(ToWideString("audio/quake/sound/feedback/3_frags.ogg"));
	ResCache::Get()->GetHandle(&BaseResource(mMedia.threeFragSound));
	mMedia.count3Sound = eastl::wstring(ToWideString("audio/quake/sound/feedback/three.ogg"));
	ResCache::Get()->GetHandle(&BaseResource(mMedia.count3Sound));
	mMedia.count2Sound = eastl::wstring(ToWideString("audio/quake/sound/feedback/two.ogg"));
	ResCache::Get()->GetHandle(&BaseResource(mMedia.count2Sound));
	mMedia.count1Sound = eastl::wstring(ToWideString("audio/quake/sound/feedback/one.ogg"));
	ResCache::Get()->GetHandle(&BaseResource(mMedia.count1Sound));
	mMedia.countFightSound = eastl::wstring(ToWideString("audio/quake/sound/feedback/fight.ogg"));
	ResCache::Get()->GetHandle(&BaseResource(mMedia.countFightSound));
	mMedia.countPrepareSound = eastl::wstring(ToWideString("audio/quake/sound/feedback/prepare.wav"));
	ResCache::Get()->GetHandle(&BaseResource(mMedia.countPrepareSound));

	mMedia.tracerSound = eastl::wstring(ToWideString("audio/quake/sound/weapons/machinegun/buletby1.ogg"));
	ResCache::Get()->GetHandle(&BaseResource(mMedia.tracerSound));
	mMedia.selectSound = eastl::wstring(ToWideString("audio/quake/sound/weapons/change.ogg"));
	ResCache::Get()->GetHandle(&BaseResource(mMedia.selectSound));
	mMedia.gibSound = eastl::wstring(ToWideString("audio/quake/sound/gibs/gibsplt1.ogg"));
	ResCache::Get()->GetHandle(&BaseResource(mMedia.gibSound));
	mMedia.gibBounce1Sound = eastl::wstring(ToWideString("audio/quake/sound/gibs/gibimp1.ogg"));
	ResCache::Get()->GetHandle(&BaseResource(mMedia.gibBounce1Sound));
	mMedia.gibBounce2Sound = eastl::wstring(ToWideString("audio/quake/sound/gibs/gibimp2.ogg"));
	ResCache::Get()->GetHandle(&BaseResource(mMedia.gibBounce2Sound));
	mMedia.gibBounce3Sound = eastl::wstring(ToWideString("audio/quake/sound/gibs/gibimp3.ogg"));
	ResCache::Get()->GetHandle(&BaseResource(mMedia.gibBounce3Sound));

	mMedia.teleInSound = eastl::wstring(ToWideString("audio/quake/sound/world/telein.ogg"));
	ResCache::Get()->GetHandle(&BaseResource(mMedia.teleInSound));
	mMedia.teleOutSound = eastl::wstring(ToWideString("audio/quake/sound/world/teleout.ogg"));
	ResCache::Get()->GetHandle(&BaseResource(mMedia.teleOutSound));
	mMedia.respawnSound = eastl::wstring(ToWideString("audio/quake/sound/items/respawn1.ogg"));
	ResCache::Get()->GetHandle(&BaseResource(mMedia.respawnSound));

	mMedia.noAmmoSound = eastl::wstring(ToWideString("audio/quake/sound/weapons/noammo.ogg"));
	ResCache::Get()->GetHandle(&BaseResource(mMedia.noAmmoSound));

	mMedia.talkSound = eastl::wstring(ToWideString("audio/quake/sound/player/talk.ogg"));
	ResCache::Get()->GetHandle(&BaseResource(mMedia.talkSound));
	mMedia.landSound = eastl::wstring(ToWideString("audio/quake/sound/player/land1.ogg"));
	ResCache::Get()->GetHandle(&BaseResource(mMedia.landSound));

	mMedia.hitSound = eastl::wstring(ToWideString("audio/quake/sound/feedback/hit.wav"));
	ResCache::Get()->GetHandle(&BaseResource(mMedia.hitSound));

	mMedia.impressiveSound = eastl::wstring(ToWideString("audio/quake/sound/feedback/impressive1.ogg"));
	ResCache::Get()->GetHandle(&BaseResource(mMedia.impressiveSound));
	mMedia.excellentSound = eastl::wstring(ToWideString("audio/quake/sound/feedback/excellent1.ogg"));
	ResCache::Get()->GetHandle(&BaseResource(mMedia.excellentSound));
	mMedia.deniedSound = eastl::wstring(ToWideString("audio/quake/sound/feedback/denied.ogg"));
	ResCache::Get()->GetHandle(&BaseResource(mMedia.deniedSound));
	mMedia.humiliationSound = eastl::wstring(ToWideString("audio/quake/sound/feedback/humiliation1.ogg"));
	ResCache::Get()->GetHandle(&BaseResource(mMedia.humiliationSound));
	mMedia.assistSound = eastl::wstring(ToWideString("audio/quake/sound/feedback/assist.ogg"));
	ResCache::Get()->GetHandle(&BaseResource(mMedia.assistSound));
	mMedia.defendSound = eastl::wstring(ToWideString("audio/quake/sound/feedback/defense.ogg"));
	ResCache::Get()->GetHandle(&BaseResource(mMedia.defendSound));

	mMedia.jumpPadSound = eastl::wstring(ToWideString("audio/quake/sound/world/jumppad.wav"));
	ResCache::Get()->GetHandle(&BaseResource(mMedia.jumpPadSound));

	char name[MAX_QPATH];
	for (int i = 0; i < 4; i++) 
	{
		sprintf(name, "audio/quake/sound/player/footsteps/step%i.ogg", i + 1);
		mMedia.footsteps[FOOTSTEP_NORMAL][i] = eastl::wstring(ToWideString(name));
		ResCache::Get()->GetHandle(&BaseResource(mMedia.footsteps[FOOTSTEP_NORMAL][i]));

		sprintf(name, "audio/quake/sound/player/footsteps/boot%i.ogg", i + 1);
		mMedia.footsteps[FOOTSTEP_BOOT][i] = eastl::wstring(ToWideString(name));
		ResCache::Get()->GetHandle(&BaseResource(mMedia.footsteps[FOOTSTEP_BOOT][i]));

		sprintf(name, "audio/quake/sound/player/footsteps/flesh%i.ogg", i + 1);
		mMedia.footsteps[FOOTSTEP_FLESH][i] = eastl::wstring(ToWideString(name));
		ResCache::Get()->GetHandle(&BaseResource(mMedia.footsteps[FOOTSTEP_FLESH][i]));

		sprintf(name, "audio/quake/sound/player/footsteps/mech%i.ogg", i + 1);
		mMedia.footsteps[FOOTSTEP_MECH][i] = eastl::wstring(ToWideString(name));
		ResCache::Get()->GetHandle(&BaseResource(mMedia.footsteps[FOOTSTEP_MECH][i]));

		sprintf(name, "audio/quake/sound/player/footsteps/energy%i.ogg", i + 1);
		mMedia.footsteps[FOOTSTEP_ENERGY][i] = eastl::wstring(ToWideString(name));
		ResCache::Get()->GetHandle(&BaseResource(mMedia.footsteps[FOOTSTEP_ENERGY][i]));

		sprintf(name, "audio/quake/sound/player/footsteps/splash%i.ogg", i + 1);
		mMedia.footsteps[FOOTSTEP_SPLASH][i] = eastl::wstring(ToWideString(name)); 
		ResCache::Get()->GetHandle(&BaseResource(mMedia.footsteps[FOOTSTEP_SPLASH][i]));

		sprintf(name, "audio/quake/sound/player/footsteps/clank%i.ogg", i + 1);
		mMedia.footsteps[FOOTSTEP_METAL][i] = eastl::wstring(ToWideString(name));
		ResCache::Get()->GetHandle(&BaseResource(mMedia.footsteps[FOOTSTEP_METAL][i]));
	}

	mMedia.playerDeath1Sound = eastl::wstring(ToWideString("audio/quake/sound/player/death1.wav"));
	ResCache::Get()->GetHandle(&BaseResource(mMedia.playerDeath1Sound));
	mMedia.playerDeath2Sound = eastl::wstring(ToWideString("audio/quake/sound/player/death2.wav"));
	ResCache::Get()->GetHandle(&BaseResource(mMedia.playerDeath2Sound));
	mMedia.playerDeath3Sound = eastl::wstring(ToWideString("audio/quake/sound/player/death3.wav"));
	ResCache::Get()->GetHandle(&BaseResource(mMedia.playerDeath3Sound));
	mMedia.playerDrownSound = eastl::wstring(ToWideString("audio/quake/sound/player/drown.wav"));
	ResCache::Get()->GetHandle(&BaseResource(mMedia.playerDrownSound));
	mMedia.playerFallSound = eastl::wstring(ToWideString("audio/quake/sound/player/fall1.wav"));
	ResCache::Get()->GetHandle(&BaseResource(mMedia.playerFallSound));
	mMedia.playerFallingSound = eastl::wstring(ToWideString("audio/quake/sound/player/falling1.wav"));
	ResCache::Get()->GetHandle(&BaseResource(mMedia.playerFallingSound));
	mMedia.playerGaspSound = eastl::wstring(ToWideString("audio/quake/sound/player/gasp.wav"));
	ResCache::Get()->GetHandle(&BaseResource(mMedia.playerGaspSound));
	mMedia.playerJumpSound = eastl::wstring(ToWideString("audio/quake/sound/player/jump1.wav"));
	ResCache::Get()->GetHandle(&BaseResource(mMedia.playerJumpSound));
	mMedia.playerLandSound = eastl::wstring(ToWideString("audio/quake/sound/player/land1.ogg"));
	ResCache::Get()->GetHandle(&BaseResource(mMedia.playerLandSound));
	mMedia.playerPainSound[0] = eastl::wstring(ToWideString("audio/quake/sound/player/pain25_1.wav"));
	ResCache::Get()->GetHandle(&BaseResource(mMedia.playerPainSound[0]));
	mMedia.playerPainSound[1] = eastl::wstring(ToWideString("audio/quake/sound/player/pain50_1.wav"));
	ResCache::Get()->GetHandle(&BaseResource(mMedia.playerPainSound[1]));
	mMedia.playerPainSound[2] = eastl::wstring(ToWideString("audio/quake/sound/player/pain75_1.wav"));
	ResCache::Get()->GetHandle(&BaseResource(mMedia.playerPainSound[2]));
	mMedia.playerPainSound[3] = eastl::wstring(ToWideString("audio/quake/sound/player/pain100_1.wav"));
	ResCache::Get()->GetHandle(&BaseResource(mMedia.playerPainSound[3]));
	mMedia.playerTalkSound = eastl::wstring(ToWideString("audio/quake/sound/player/talk.ogg"));
	ResCache::Get()->GetHandle(&BaseResource(mMedia.playerTalkSound));
	mMedia.playerTauntSound = eastl::wstring(ToWideString("audio/quake/sound/player/taunt.wav"));
	ResCache::Get()->GetHandle(&BaseResource(mMedia.playerTauntSound));

	// FIXME: only needed with item
	mMedia.quadSound = eastl::wstring(ToWideString("audio/quake/sound/items/damage3.ogg"));
	ResCache::Get()->GetHandle(&BaseResource(mMedia.quadSound));
	mMedia.sfxRic1 = eastl::wstring(ToWideString("audio/quake/sound/weapons/machinegun/ric1.ogg"));
	ResCache::Get()->GetHandle(&BaseResource(mMedia.sfxRic1));
	mMedia.sfxRic2 = eastl::wstring(ToWideString("audio/quake/sound/weapons/machinegun/ric2.ogg"));
	ResCache::Get()->GetHandle(&BaseResource(mMedia.sfxRic2));
	mMedia.sfxRic3 = eastl::wstring(ToWideString("audio/quake/sound/weapons/machinegun/ric3.ogg"));
	ResCache::Get()->GetHandle(&BaseResource(mMedia.sfxRic3));
	mMedia.sfxRailg = eastl::wstring(ToWideString("audio/quake/sound/weapons/railgun/railgf1a.ogg"));
	ResCache::Get()->GetHandle(&BaseResource(mMedia.sfxRailg));
	mMedia.sfxRockexp = eastl::wstring(ToWideString("audio/quake/sound/weapons/rocket/rocklx1a.ogg"));
	ResCache::Get()->GetHandle(&BaseResource(mMedia.sfxRockexp));
	mMedia.sfxPlasmaexp = eastl::wstring(ToWideString("audio/quake/sound/weapons/plasma/plasmx1a.ogg"));
	ResCache::Get()->GetHandle(&BaseResource(mMedia.sfxPlasmaexp));

	mMedia.nhealthSound = eastl::wstring(ToWideString("audio/quake/sound/items/n_health.wav"));
	ResCache::Get()->GetHandle(&BaseResource(mMedia.nhealthSound));
	mMedia.hgrenb1aSound = eastl::wstring(ToWideString("audio/quake/sound/weapons/grenade/hgrenb1a.ogg"));
	ResCache::Get()->GetHandle(&BaseResource(mMedia.hgrenb1aSound));
	mMedia.hgrenb2aSound = eastl::wstring(ToWideString("audio/quake/sound/weapons/grenade/hgrenb2a.ogg"));
	ResCache::Get()->GetHandle(&BaseResource(mMedia.hgrenb2aSound));
}

// RegisterSounds called during a precache command
void QuakeHumanView::RegisterMedia()
{
	eastl::shared_ptr<ResHandle> resHandle;
	eastl::shared_ptr<ImageResourceExtraData> resData;

	char* sbNums[11] = 
	{
		"art/quake/gfx/2d/numbers/zero_32b.png",
		"art/quake/gfx/2d/numbers/one_32b.png",
		"art/quake/gfx/2d/numbers/two_32b.png",
		"art/quake/gfx/2d/numbers/three_32b.png",
		"art/quake/gfx/2d/numbers/four_32b.png",
		"art/quake/gfx/2d/numbers/five_32b.png",
		"art/quake/gfx/2d/numbers/six_32b.png",
		"art/quake/gfx/2d/numbers/seven_32b.png",
		"art/quake/gfx/2d/numbers/eight_32b.png",
		"art/quake/gfx/2d/numbers/nine_32b.png",
		"art/quake/gfx/2d/numbers/minus_32b.png",
	};
	for (int i=0 ; i<=10; i++) 
	{
		mMedia.numberShaders[i] = eastl::wstring(ToWideString(sbNums[i]));
		ResCache::Get()->GetHandle(&BaseResource(mMedia.numberShaders[i]));
	}

	mMedia.botSkillShaders[0] = eastl::wstring(ToWideString("art/quake/menu/art/skill1.png"));
	ResCache::Get()->GetHandle(&BaseResource(mMedia.botSkillShaders[0]));
	mMedia.botSkillShaders[1] = eastl::wstring(ToWideString("art/quake/menu/art/skill2.png"));
	ResCache::Get()->GetHandle(&BaseResource(mMedia.botSkillShaders[1]));
	mMedia.botSkillShaders[2] = eastl::wstring(ToWideString("art/quake/menu/art/skill3.png"));
	ResCache::Get()->GetHandle(&BaseResource(mMedia.botSkillShaders[2]));
	mMedia.botSkillShaders[3] = eastl::wstring(ToWideString("art/quake/menu/art/skill4.png"));
	ResCache::Get()->GetHandle(&BaseResource(mMedia.botSkillShaders[3]));
	mMedia.botSkillShaders[4] = eastl::wstring(ToWideString("art/quake/menu/art/skill5.png"));
	ResCache::Get()->GetHandle(&BaseResource(mMedia.botSkillShaders[4]));

	mMedia.viewBloodShader = eastl::wstring(ToWideString("art/quake/gfx/damage/blood_screen.png"));
	ResCache::Get()->GetHandle(&BaseResource(mMedia.viewBloodShader));

	mMedia.deferShader = eastl::wstring(ToWideString("art/quake/gfx/2d/defer.png"));
	ResCache::Get()->GetHandle(&BaseResource(mMedia.deferShader));

	mMedia.scoreboardName = eastl::wstring(ToWideString("art/quake/menu/tab/name.tga"));
	ResCache::Get()->GetHandle(&BaseResource(mMedia.scoreboardName));
	mMedia.scoreboardPing = eastl::wstring(ToWideString("art/quake/menu/tab/ping.tga"));
	ResCache::Get()->GetHandle(&BaseResource(mMedia.scoreboardPing));
	mMedia.scoreboardScore = eastl::wstring(ToWideString("art/quake/menu/tab/score.tga"));
	ResCache::Get()->GetHandle(&BaseResource(mMedia.scoreboardScore));
	mMedia.scoreboardTime = eastl::wstring(ToWideString("art/quake/menu/tab/time.tga"));
	ResCache::Get()->GetHandle(&BaseResource(mMedia.scoreboardTime));

	mMedia.smokePuffShader = eastl::wstring(ToWideString("art/quake/gfx/misc/smokepuff3.png"));
	resHandle = ResCache::Get()->GetHandle(&BaseResource(mMedia.smokePuffShader));
	resData = eastl::static_pointer_cast<ImageResourceExtraData>(resHandle->GetExtra());
	resData->GetImage()->AutogenerateMipmaps();
	mMedia.smokePuffRageProShader = eastl::wstring(ToWideString("art/quake/gfx/misc/smokepuffragepro.png"));
	resHandle = ResCache::Get()->GetHandle(&BaseResource(mMedia.smokePuffRageProShader));
	resData = eastl::static_pointer_cast<ImageResourceExtraData>(resHandle->GetExtra());
	resData->GetImage()->AutogenerateMipmaps();
	mMedia.shotgunSmokePuffShader = eastl::wstring(ToWideString("art/quake/gfx/misc/smokepuff2b.png"));
	resHandle = ResCache::Get()->GetHandle(&BaseResource(mMedia.shotgunSmokePuffShader));
	resData = eastl::static_pointer_cast<ImageResourceExtraData>(resHandle->GetExtra());
	resData->GetImage()->AutogenerateMipmaps();
	mMedia.plasmaBallShader = eastl::wstring(ToWideString("art/quake/sprites/plasmaa.jpg"));
	resHandle = ResCache::Get()->GetHandle(&BaseResource(mMedia.plasmaBallShader));
	resData = eastl::static_pointer_cast<ImageResourceExtraData>(resHandle->GetExtra());
	resData->GetImage()->AutogenerateMipmaps();
	mMedia.bloodTrailShader = eastl::wstring(ToWideString("art/quake/gfx/damage/blood_spurt.png"));
	resHandle = ResCache::Get()->GetHandle(&BaseResource(mMedia.bloodTrailShader));
	resData = eastl::static_pointer_cast<ImageResourceExtraData>(resHandle->GetExtra());
	resData->GetImage()->AutogenerateMipmaps();
	mMedia.lagometerShader = eastl::wstring(ToWideString("art/quake/gfx/2d/lag.jpg"));
	resHandle = ResCache::Get()->GetHandle(&BaseResource(mMedia.lagometerShader));
	mMedia.connectionShader = eastl::wstring(ToWideString("art/quake/gfx/2d/net.jpg"));
	resHandle = ResCache::Get()->GetHandle(&BaseResource(mMedia.connectionShader));

	mMedia.tracerShader = eastl::wstring(ToWideString("art/quake/gfx/misc/tracer2.jpg"));
	ResCache::Get()->GetHandle(&BaseResource(mMedia.tracerShader));
	mMedia.selectShader = eastl::wstring(ToWideString("art/quake/gfx/2d/select.png"));
	ResCache::Get()->GetHandle(&BaseResource(mMedia.selectShader));

	char name[MAX_QPATH];
	for (int i = 0 ; i < NUM_CROSSHAIRS ; i++ ) 
	{
		sprintf(name, "art/quake/gfx/2d/crosshair%i.png", i + 1);
		mMedia.crosshairShader[i] = eastl::wstring(ToWideString(name));
		ResCache::Get()->GetHandle(&BaseResource(mMedia.crosshairShader[i]));
	}

	mMedia.backTileShader = eastl::wstring(ToWideString("art/quake/gfx/2d/backtile.jpg"));
	ResCache::Get()->GetHandle(&BaseResource(mMedia.backTileShader));
	mMedia.noammoShader = eastl::wstring(ToWideString("art/quake/icons/noammo.png"));
	ResCache::Get()->GetHandle(&BaseResource(mMedia.noammoShader));

	mMedia.healthIcon = eastl::wstring(ToWideString("art/quake/icons/iconh_yellow.png"));
	ResCache::Get()->GetHandle(&BaseResource(mMedia.armorIcon));
	mMedia.armorIcon = eastl::wstring(ToWideString("art/quake/icons/iconr_yellow.png"));
	ResCache::Get()->GetHandle(&BaseResource(mMedia.armorIcon));

	mMedia.machinegunBrassModel = eastl::wstring(ToWideString("art/quake/models/projectiles/shells/m_shell.md3"));
	ResCache::Get()->GetHandle(&BaseResource(mMedia.machinegunBrassModel));
	mMedia.shotgunBrassModel = eastl::wstring(ToWideString("art/quake/models/projectiles/shells/s_shell.md3"));
	ResCache::Get()->GetHandle(&BaseResource(mMedia.shotgunBrassModel));

	mMedia.gibAbdomen = eastl::wstring(ToWideString("art/quake/models/gibs/abdomen.md3"));
	ResCache::Get()->GetHandle(&BaseResource(mMedia.gibAbdomen));
	mMedia.gibArm = eastl::wstring(ToWideString("art/quake/models/gibs/arm.md3"));
	ResCache::Get()->GetHandle(&BaseResource(mMedia.gibArm));
	mMedia.gibChest = eastl::wstring(ToWideString("art/quake/models/gibs/chest.md3"));
	ResCache::Get()->GetHandle(&BaseResource(mMedia.gibChest));
	mMedia.gibFist = eastl::wstring(ToWideString("art/quake/models/gibs/fist.md3"));
	ResCache::Get()->GetHandle(&BaseResource(mMedia.gibFist));
	mMedia.gibFoot = eastl::wstring(ToWideString("art/quake/models/gibs/foot.md3"));
	ResCache::Get()->GetHandle(&BaseResource(mMedia.gibFoot));
	mMedia.gibForearm = eastl::wstring(ToWideString("art/quake/models/gibs/forearm.md3"));
	ResCache::Get()->GetHandle(&BaseResource(mMedia.gibForearm));
	mMedia.gibIntestine = eastl::wstring(ToWideString("art/quake/models/gibs/intestine.md3"));
	ResCache::Get()->GetHandle(&BaseResource(mMedia.gibIntestine));
	mMedia.gibLeg = eastl::wstring(ToWideString("art/quake/models/gibs/leg.md3"));
	ResCache::Get()->GetHandle(&BaseResource(mMedia.gibLeg));
	mMedia.gibSkull = eastl::wstring(ToWideString("art/quake/models/gibs/skull.md3"));
	ResCache::Get()->GetHandle(&BaseResource(mMedia.gibSkull));
	mMedia.gibBrain = eastl::wstring(ToWideString("art/quake/models/gibs/brain.md3"));
	ResCache::Get()->GetHandle(&BaseResource(mMedia.gibBrain));

	mMedia.smoke2 = eastl::wstring(ToWideString("art/quake/models/projectiles/shells/s_shell.md3"));
	ResCache::Get()->GetHandle(&BaseResource(mMedia.smoke2));

	mMedia.balloonShader = eastl::wstring(ToWideString("art/quake/sprites/balloon4.png"));
	ResCache::Get()->GetHandle(&BaseResource(mMedia.balloonShader));

	for (int i = 0; i < 5; i++)
	{
		sprintf(name, "art/quake/gfx/damage/blood20%i.jpg", i + 1);
		mMedia.bloodExplosionShader[i] = eastl::wstring(ToWideString(name));
		resHandle = ResCache::Get()->GetHandle(&BaseResource(mMedia.bloodExplosionShader[i]));
		resData = eastl::static_pointer_cast<ImageResourceExtraData>(resHandle->GetExtra());
		resData->GetImage()->AutogenerateMipmaps();
	}

	mMedia.bulletFlashModel = eastl::wstring(ToWideString("art/quake/models/weaphits/bullet.md3"));
	ResCache::Get()->GetHandle(&BaseResource(mMedia.bulletFlashModel));
	mMedia.ringFlashModel = eastl::wstring(ToWideString("art/quake/models/weaphits/ring02.md3"));
	ResCache::Get()->GetHandle(&BaseResource(mMedia.ringFlashModel));
	mMedia.dishFlashModel = eastl::wstring(ToWideString("art/quake/models/weaphits/boom01.md3"));
	ResCache::Get()->GetHandle(&BaseResource(mMedia.dishFlashModel));

	mMedia.teleportEffectModel = eastl::wstring(ToWideString("art/quake/models/misc/telep.md3"));
	ResCache::Get()->GetHandle(&BaseResource(mMedia.teleportEffectModel));
	mMedia.teleportEffectShader = eastl::wstring(ToWideString("art/quake/gfx/misc/teleportEffect2.jpg"));
	resHandle = ResCache::Get()->GetHandle(&BaseResource(mMedia.teleportEffectShader));
	resData = eastl::static_pointer_cast<ImageResourceExtraData>(resHandle->GetExtra());
	resData->GetImage()->AutogenerateMipmaps();

	mMedia.medalImpressive = eastl::wstring(ToWideString("art/quake/menu/medals/medal_impressive.png"));
	ResCache::Get()->GetHandle(&BaseResource(mMedia.medalImpressive));
	mMedia.medalExcellent = eastl::wstring(ToWideString("art/quake/menu/medals/medal_excellent.png"));
	ResCache::Get()->GetHandle(&BaseResource(mMedia.medalExcellent));
	mMedia.medalGauntlet = eastl::wstring(ToWideString("art/quake/menu/medals/medal_gauntlet.png"));
	ResCache::Get()->GetHandle(&BaseResource(mMedia.medalGauntlet));
	mMedia.medalDefend = eastl::wstring(ToWideString("art/quake/menu/medals/medal_defense.png"));
	ResCache::Get()->GetHandle(&BaseResource(mMedia.medalDefend));
	mMedia.medalAssist = eastl::wstring(ToWideString("art/quake/menu/medals/medal_assist.png"));
	ResCache::Get()->GetHandle(&BaseResource(mMedia.medalAssist));
	mMedia.medalCapture = eastl::wstring(ToWideString("art/quake/menu/medals/medal_capture.png"));
	ResCache::Get()->GetHandle(&BaseResource(mMedia.medalCapture));

	// wall marks
	mMedia.bulletMarkShader = eastl::wstring(ToWideString("art/quake/gfx/damage/bullet_mrk.jpg"));
	resHandle = ResCache::Get()->GetHandle(&BaseResource(mMedia.bulletMarkShader));
	resData = eastl::static_pointer_cast<ImageResourceExtraData>(resHandle->GetExtra());
	resData->GetImage()->AutogenerateMipmaps();
	mMedia.burnMarkShader = eastl::wstring(ToWideString("art/quake/gfx/damage/burn_med_mrk.jpg"));
	resHandle = ResCache::Get()->GetHandle(&BaseResource(mMedia.burnMarkShader));
	resData = eastl::static_pointer_cast<ImageResourceExtraData>(resHandle->GetExtra());
	resData->GetImage()->AutogenerateMipmaps();
	mMedia.holeMarkShader = eastl::wstring(ToWideString("art/quake/gfx/damage/hole_lg_mrk.jpg"));
	resHandle = ResCache::Get()->GetHandle(&BaseResource(mMedia.holeMarkShader));
	resData = eastl::static_pointer_cast<ImageResourceExtraData>(resHandle->GetExtra());
	resData->GetImage()->AutogenerateMipmaps();
	mMedia.energyMarkShader = eastl::wstring(ToWideString("art/quake/gfx/damage/plasma_mrk.png"));
	resHandle = ResCache::Get()->GetHandle(&BaseResource(mMedia.energyMarkShader));
	resData = eastl::static_pointer_cast<ImageResourceExtraData>(resHandle->GetExtra());
	resData->GetImage()->AutogenerateMipmaps();
	mMedia.shadowMarkShader = eastl::wstring(ToWideString("art/quake/gfx/damage/shadow.png"));
	resHandle = ResCache::Get()->GetHandle(&BaseResource(mMedia.shadowMarkShader));
	resData = eastl::static_pointer_cast<ImageResourceExtraData>(resHandle->GetExtra());
	resData->GetImage()->AutogenerateMipmaps();
	mMedia.bloodMarkShader = eastl::wstring(ToWideString("art/quake/gfx/damage/blood_stain.png"));
	resHandle = ResCache::Get()->GetHandle(&BaseResource(mMedia.bloodMarkShader));
	resData = eastl::static_pointer_cast<ImageResourceExtraData>(resHandle->GetExtra());
	resData->GetImage()->AutogenerateMipmaps();
}


void QuakeHumanView::GameplayUiUpdateDelegate(BaseEventDataPtr pEventData)
{
    eastl::shared_ptr<QuakeEventDataGameplayUIUpdate> pCastEventData =
		eastl::static_pointer_cast<QuakeEventDataGameplayUIUpdate>(pEventData);
    if (!pCastEventData->GetUiString().empty())
        mGameplayText = pCastEventData->GetUiString();
    else
		mGameplayText.clear();
}

void QuakeHumanView::SetControlledActorDelegate(BaseEventDataPtr pEventData)
{
    eastl::shared_ptr<QuakeEventDataSetControlledActor> pCastEventData =
		eastl::static_pointer_cast<QuakeEventDataSetControlledActor>(pEventData);
    //SetControlledActor(pCastEventData->GetActorId());

	HumanView::SetControlledActor(mGameCameraController->GetCamera()->GetId());
	mKeyboardHandler = mGameCameraController;
	mMouseHandler = mGameCameraController;

}

void QuakeHumanView::FireWeaponDelegate(BaseEventDataPtr pEventData)
{
	eastl::shared_ptr<QuakeEventDataFireWeapon> pCastEventData =
		eastl::static_pointer_cast<QuakeEventDataFireWeapon>(pEventData);

	ActorId actorId = pCastEventData->GetId();
	eastl::shared_ptr<PlayerActor> pPlayerActor(
		eastl::dynamic_shared_pointer_cast<PlayerActor>(
		GameLogic::Get()->GetActor(actorId).lock()));
	eastl::shared_ptr<Node> pNode = mScene->GetSceneNode(actorId);
	if (pNode)
	{
		eastl::shared_ptr<AnimatedMeshNode> animatedNode =
			eastl::dynamic_shared_pointer_cast<AnimatedMeshNode>(pNode);
		eastl::shared_ptr<AnimateMeshMD3> animMeshMD3 =
			eastl::dynamic_shared_pointer_cast<AnimateMeshMD3>(animatedNode->GetMesh());

		eastl::vector<eastl::shared_ptr<MD3Mesh>> meshes;
		animMeshMD3->GetMD3Mesh()->GetMeshes(meshes);
		for (eastl::shared_ptr<MD3Mesh> mesh : meshes)
		{
			if (mesh->GetName() == "lower")
			{
				//run animation
				int legsAnim = pPlayerActor->GetState().legsAnim;
				if (mesh->GetCurrentAnimation() != legsAnim)
				{
					mesh->SetCurrentAnimation(legsAnim);
					mesh->SetCurrentFrame((float)mesh->GetAnimation(legsAnim).mBeginFrame);
				}
			}
			else if (mesh->GetName() == "upper")
			{
				//run animation
				int torsoAnim = pPlayerActor->GetState().torsoAnim;
				if (mesh->GetCurrentAnimation() != torsoAnim ||
					mesh->GetCurrentFrame() == mesh->GetAnimation(torsoAnim).mEndFrame)
				{
					mesh->SetCurrentAnimation(torsoAnim);
					mesh->SetCurrentFrame((float)mesh->GetAnimation(torsoAnim).mBeginFrame);
				}
			}
		}
	}
}

void QuakeHumanView::ChangeWeaponDelegate(BaseEventDataPtr pEventData)
{
	eastl::shared_ptr<QuakeEventDataFireWeapon> pCastEventData =
		eastl::static_pointer_cast<QuakeEventDataFireWeapon>(pEventData);

	ActorId actorId = pCastEventData->GetId();
	eastl::shared_ptr<PlayerActor> pPlayerActor(
		eastl::dynamic_shared_pointer_cast<PlayerActor>(
		GameLogic::Get()->GetActor(actorId).lock()));
	eastl::shared_ptr<Node> pNode = mScene->GetSceneNode(actorId);
	if (pNode)
	{
		eastl::shared_ptr<AnimatedMeshNode> animatedNode =
			eastl::dynamic_shared_pointer_cast<AnimatedMeshNode>(pNode);
		eastl::shared_ptr<AnimateMeshMD3> animMeshMD3 =
			eastl::dynamic_shared_pointer_cast<AnimateMeshMD3>(animatedNode->GetMesh());

		eastl::vector<eastl::shared_ptr<MD3Mesh>> meshes;
		animMeshMD3->GetMD3Mesh()->GetMeshes(meshes);

		int weaponIdx = 0;
		for (eastl::shared_ptr<MD3Mesh> mesh : meshes)
		{
			if (mesh->GetParent() && mesh->GetParent()->GetName() == "tag_weapon")
			{
				weaponIdx++;
				if (pPlayerActor->GetState().weapon == weaponIdx)
					mesh->SetRenderMesh(true);
				else
					mesh->SetRenderMesh(false);
			}

			if (mesh->GetName() == "lower")
			{
				//run animation
				int legsAnim = pPlayerActor->GetState().legsAnim;
				if (mesh->GetCurrentAnimation() != legsAnim)
				{
					mesh->SetCurrentAnimation(legsAnim);
					mesh->SetCurrentFrame((float)mesh->GetAnimation(legsAnim).mBeginFrame);
				}
			}
			else if (mesh->GetName() == "upper")
			{
				//run animation
				int torsoAnim = pPlayerActor->GetState().torsoAnim;
				if (mesh->GetCurrentAnimation() != torsoAnim)
				{
					mesh->SetCurrentAnimation(torsoAnim);
					mesh->SetCurrentFrame((float)mesh->GetAnimation(torsoAnim).mBeginFrame);
				}
			}
		}
	}
}

void QuakeHumanView::DeadActorDelegate(BaseEventDataPtr pEventData)
{
	eastl::shared_ptr<QuakeEventDataDeadActor> pCastEventData =
		eastl::static_pointer_cast<QuakeEventDataDeadActor>(pEventData);

	ActorId actorId = pCastEventData->GetId();
	eastl::shared_ptr<PlayerActor> pPlayerActor(
		eastl::dynamic_shared_pointer_cast<PlayerActor>(
		GameLogic::Get()->GetActor(actorId).lock()));
	eastl::shared_ptr<Node> pNode = mScene->GetSceneNode(actorId);
	if (pNode)
	{
		eastl::shared_ptr<AnimatedMeshNode> animatedNode =
			eastl::dynamic_shared_pointer_cast<AnimatedMeshNode>(pNode);
		eastl::shared_ptr<AnimateMeshMD3> animMeshMD3 =
			eastl::dynamic_shared_pointer_cast<AnimateMeshMD3>(animatedNode->GetMesh());

		eastl::vector<eastl::shared_ptr<MD3Mesh>> meshes;
		animMeshMD3->GetMD3Mesh()->GetMeshes(meshes);

		for (eastl::shared_ptr<MD3Mesh> mesh : meshes)
		{
			if (mesh->GetParent() && mesh->GetParent()->GetName() == "tag_weapon")
				mesh->SetRenderMesh(false);

			if (mesh->GetName() == "lower")
			{
				//run animation
				int legsAnim = pPlayerActor->GetState().legsAnim;
				mesh->SetCurrentAnimation(legsAnim);
				mesh->SetCurrentFrame((float)mesh->GetAnimation(legsAnim).mBeginFrame);
			}
			else if (mesh->GetName() == "upper")
			{
				//run animation
				int torsoAnim = pPlayerActor->GetState().torsoAnim;
				mesh->SetCurrentAnimation(torsoAnim);
				mesh->SetCurrentFrame((float)mesh->GetAnimation(torsoAnim).mBeginFrame);
			}
		}
	}
}

void QuakeHumanView::SpawnActorDelegate(BaseEventDataPtr pEventData)
{
	eastl::shared_ptr<QuakeEventDataSpawnActor> pCastEventData =
		eastl::static_pointer_cast<QuakeEventDataSpawnActor>(pEventData);

	ActorId actorId = pCastEventData->GetId();
	eastl::shared_ptr<PlayerActor> pPlayerActor(
		eastl::dynamic_shared_pointer_cast<PlayerActor>(
		GameLogic::Get()->GetActor(actorId).lock()));
	eastl::shared_ptr<Node> pNode = mScene->GetSceneNode(actorId);
	if (pNode)
	{
		eastl::shared_ptr<AnimatedMeshNode> animatedNode =
			eastl::dynamic_shared_pointer_cast<AnimatedMeshNode>(pNode);
		eastl::shared_ptr<AnimateMeshMD3> animMeshMD3 =
			eastl::dynamic_shared_pointer_cast<AnimateMeshMD3>(animatedNode->GetMesh());

		eastl::vector<eastl::shared_ptr<MD3Mesh>> meshes;
		animMeshMD3->GetMD3Mesh()->GetMeshes(meshes);

		int weaponIdx = 0;
		for (eastl::shared_ptr<MD3Mesh> mesh : meshes)
		{
			if (mesh->GetParent() && mesh->GetParent()->GetName() == "tag_weapon")
			{
				weaponIdx++;
				if (pPlayerActor->GetState().weapon == weaponIdx)
					mesh->SetRenderMesh(true);
				else
					mesh->SetRenderMesh(false);
			}

			if (mesh->GetName() == "lower")
			{
				//run animation
				int legsAnim = pPlayerActor->GetState().legsAnim;
				mesh->SetCurrentAnimation(legsAnim);
				mesh->SetCurrentFrame((float)mesh->GetAnimation(legsAnim).mBeginFrame);
			}
			else if (mesh->GetName() == "upper")
			{
				//run animation
				int torsoAnim = pPlayerActor->GetState().torsoAnim;
				mesh->SetCurrentAnimation(torsoAnim);
				mesh->SetCurrentFrame((float)mesh->GetAnimation(torsoAnim).mBeginFrame);
			}
		}
	}
}

void QuakeHumanView::JumpActorDelegate(BaseEventDataPtr pEventData)
{
	eastl::shared_ptr<QuakeEventDataJumpActor> pCastEventData =
		eastl::static_pointer_cast<QuakeEventDataJumpActor>(pEventData);

	ActorId actorId = pCastEventData->GetId();
	eastl::shared_ptr<PlayerActor> pPlayerActor(
		eastl::dynamic_shared_pointer_cast<PlayerActor>(
		GameLogic::Get()->GetActor(actorId).lock()));
	eastl::shared_ptr<Node> pNode = mScene->GetSceneNode(actorId);
	if (pNode)
	{
		eastl::shared_ptr<AnimatedMeshNode> animatedNode =
			eastl::dynamic_shared_pointer_cast<AnimatedMeshNode>(pNode);
		eastl::shared_ptr<AnimateMeshMD3> animMeshMD3 =
			eastl::dynamic_shared_pointer_cast<AnimateMeshMD3>(animatedNode->GetMesh());

		eastl::vector<eastl::shared_ptr<MD3Mesh>> meshes;
		animMeshMD3->GetMD3Mesh()->GetMeshes(meshes);
		for (eastl::shared_ptr<MD3Mesh> mesh : meshes)
		{
			if (mesh->GetName() == "lower")
			{
				//run animation
				int legsAnim = pPlayerActor->GetState().legsAnim;
				mesh->SetCurrentAnimation(legsAnim);
				mesh->SetCurrentFrame((float)mesh->GetAnimation(legsAnim).mBeginFrame);
			}
			else if (mesh->GetName() == "upper")
			{
				//run animation
				int torsoAnim = pPlayerActor->GetState().torsoAnim;
				mesh->SetCurrentAnimation(torsoAnim);
				mesh->SetCurrentFrame((float)mesh->GetAnimation(torsoAnim).mBeginFrame);
			}
		}
	}
}

void QuakeHumanView::MoveActorDelegate(BaseEventDataPtr pEventData)
{
	eastl::shared_ptr<QuakeEventDataMoveActor> pCastEventData =
		eastl::static_pointer_cast<QuakeEventDataMoveActor>(pEventData);

	ActorId actorId = pCastEventData->GetId();
	eastl::shared_ptr<PlayerActor> pPlayerActor(
		eastl::dynamic_shared_pointer_cast<PlayerActor>(
		GameLogic::Get()->GetActor(actorId).lock()));
	if (pPlayerActor->GetState().weaponState != WEAPON_READY)
		return;

	eastl::shared_ptr<PhysicComponent> pPhysicComponent(
		pPlayerActor->GetComponent<PhysicComponent>(PhysicComponent::Name).lock());
	if (pPhysicComponent)
		if (!pPhysicComponent->OnGround())
			return;

	eastl::shared_ptr<Node> pNode = mScene->GetSceneNode(actorId);
	if (pNode)
	{
		eastl::shared_ptr<AnimatedMeshNode> animatedNode = 
			eastl::dynamic_shared_pointer_cast<AnimatedMeshNode>(pNode);
		eastl::shared_ptr<AnimateMeshMD3> animMeshMD3 =
			eastl::dynamic_shared_pointer_cast<AnimateMeshMD3>(animatedNode->GetMesh());

		eastl::vector<eastl::shared_ptr<MD3Mesh>> meshes;
		animMeshMD3->GetMD3Mesh()->GetMeshes(meshes);
		for (eastl::shared_ptr<MD3Mesh> mesh : meshes)
		{
			if (mesh->GetName() == "lower")
			{
				//run animation
				int legsAnim = pPlayerActor->GetState().legsAnim;
				if (mesh->GetCurrentAnimation() != legsAnim)
				{
					mesh->SetCurrentAnimation(legsAnim);
					mesh->SetCurrentFrame((float)mesh->GetAnimation(legsAnim).mBeginFrame);
				}
			}
			else if (mesh->GetName() == "upper")
			{
				//run animation
				int torsoAnim = pPlayerActor->GetState().torsoAnim;
				if (mesh->GetCurrentAnimation() != torsoAnim)
				{
					mesh->SetCurrentAnimation(torsoAnim);
					mesh->SetCurrentFrame((float)mesh->GetAnimation(torsoAnim).mBeginFrame);
				}
			}
		}
	}
}

void QuakeHumanView::RegisterAllDelegates(void)
{
    BaseEventManager* pGlobalEventManager = BaseEventManager::Get();
    pGlobalEventManager->AddListener(
		MakeDelegate(this, &QuakeHumanView::GameplayUiUpdateDelegate), 
		QuakeEventDataGameplayUIUpdate::skEventType);
    pGlobalEventManager->AddListener(
		MakeDelegate(this, &QuakeHumanView::SetControlledActorDelegate), 
		QuakeEventDataSetControlledActor::skEventType);

	pGlobalEventManager->AddListener(
		MakeDelegate(this, &QuakeHumanView::FireWeaponDelegate),
		QuakeEventDataFireWeapon::skEventType);
	pGlobalEventManager->AddListener(
		MakeDelegate(this, &QuakeHumanView::ChangeWeaponDelegate),
		QuakeEventDataChangeWeapon::skEventType);
	pGlobalEventManager->AddListener(
		MakeDelegate(this, &QuakeHumanView::DeadActorDelegate),
		QuakeEventDataDeadActor::skEventType);
	pGlobalEventManager->AddListener(
		MakeDelegate(this, &QuakeHumanView::SpawnActorDelegate),
		QuakeEventDataSpawnActor::skEventType);
	pGlobalEventManager->AddListener(
		MakeDelegate(this, &QuakeHumanView::JumpActorDelegate),
		QuakeEventDataJumpActor::skEventType);
	pGlobalEventManager->AddListener(
		MakeDelegate(this, &QuakeHumanView::MoveActorDelegate),
		QuakeEventDataMoveActor::skEventType);
}

void QuakeHumanView::RemoveAllDelegates(void)
{
	BaseEventManager* pGlobalEventManager = BaseEventManager::Get();
    pGlobalEventManager->RemoveListener(
		MakeDelegate(this, &QuakeHumanView::GameplayUiUpdateDelegate), 
		QuakeEventDataGameplayUIUpdate::skEventType);
    pGlobalEventManager->RemoveListener(
		MakeDelegate(this, &QuakeHumanView::SetControlledActorDelegate), 
		QuakeEventDataSetControlledActor::skEventType);

	pGlobalEventManager->RemoveListener(
		MakeDelegate(this, &QuakeHumanView::FireWeaponDelegate),
		QuakeEventDataFireWeapon::skEventType);
	pGlobalEventManager->RemoveListener(
		MakeDelegate(this, &QuakeHumanView::ChangeWeaponDelegate),
		QuakeEventDataChangeWeapon::skEventType);
	pGlobalEventManager->RemoveListener(
		MakeDelegate(this, &QuakeHumanView::DeadActorDelegate),
		QuakeEventDataDeadActor::skEventType);
	pGlobalEventManager->RemoveListener(
		MakeDelegate(this, &QuakeHumanView::SpawnActorDelegate),
		QuakeEventDataSpawnActor::skEventType);
	pGlobalEventManager->RemoveListener(
		MakeDelegate(this, &QuakeHumanView::JumpActorDelegate),
		QuakeEventDataJumpActor::skEventType);
	pGlobalEventManager->RemoveListener(
		MakeDelegate(this, &QuakeHumanView::MoveActorDelegate),
		QuakeEventDataMoveActor::skEventType);
}