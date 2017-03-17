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

#include "UserInterface.h"

#include "Core/OS/Os.h"

#include "Application/GameApplication.h"

//! constructor
BaseUI::BaseUI()
	: Hovered(0), HoveredNoSubelement(0), Focus(0), 
	LastHoveredMousePos{ 0,0 }, CurrentSkin(0)
{
	GameApplication* gameApp = (GameApplication*)Application::App;
	Vector2<unsigned int> screenSize(gameApp->mRenderer->GetScreenSize());
	RectangleBase<2, int> screenRectangle;
	screenRectangle.extent[0] = (int)screenSize[0];
	screenRectangle.extent[1] = (int)screenSize[1];

	Root = eastl::shared_ptr<UIElement>( 
		new UIRoot(this, EUIET_ROOT, 0, screenRectangle));

	// environment is root tab group
	Root->SetTabGroup(true);
}


//! destructor
BaseUI::~BaseUI()
{

}

bool BaseUI::OnInit()
{
	//LoadBuiltInFont();

	const eastl::shared_ptr<UISkin>& skin = CreateSkin( EGSTT_WINDOWS_CLASSIC );
	SetSkin(skin);

	return true;
}

//! draws all gui elements
bool BaseUI::OnRender(double fTime, float fElapsedTime)
{
	GameApplication* gameApp = (GameApplication*)Application::App;
	Vector2<unsigned int> screenSize(gameApp->mRenderer->GetScreenSize());
	if (Root->AbsoluteRect.extent[0] != screenSize[0] ||
		Root->AbsoluteRect.extent[1] != screenSize[1])
	{
		// resize gui environment
		Root->DesiredRect.extent[0] = (int)screenSize[0];
		Root->DesiredRect.extent[1] = (int)screenSize[1];
		Root->AbsoluteClippingRect = Root->DesiredRect;
		Root->AbsoluteRect = Root->DesiredRect;
		Root->UpdateAbsoluteTransformation();
	}

	Root->Draw();

	return OnPostRender ( Timer::GetTime () );
}


//! clear all UI elements
void BaseUI::Clear()
{
	// Remove the focus
	if (Focus)
		Focus = 0;

	if (Hovered && Hovered != Root)
		Hovered = 0;
	
	if ( HoveredNoSubelement && HoveredNoSubelement != Root)
		HoveredNoSubelement = 0;

	// get the root's children in case the root changes in future
	const eastl::list<eastl::shared_ptr<UIElement>>& children =
		GetRootUIElement()->GetChildren();

	while (!children.empty())
		children.back()->Remove();
}

//
bool BaseUI::OnPostRender( unsigned int time )
{
	Root->OnPostRender ( time );
	return true;
}


//
void BaseUI::UpdateHoveredElement(Vector2<int> mousePos)
{
	eastl::shared_ptr<UIElement> lastHovered = Hovered;
	eastl::shared_ptr<UIElement> lastHoveredNoSubelement = HoveredNoSubelement;
	LastHoveredMousePos = mousePos;

	Hovered = Root->GetElementFromPoint(mousePos);

	// for tooltips we want the element itself and not some of it's subelements
	HoveredNoSubelement = Hovered;
	while ( HoveredNoSubelement && HoveredNoSubelement->IsSubElement() )
	{
		HoveredNoSubelement = HoveredNoSubelement->GetParent();
	}

	if (Hovered != lastHovered)
	{
		Event ev;
		ev.mEventType = ET_UI_EVENT;

		if (lastHovered)
		{
			ev.mUIEvent.mCaller = lastHovered.get();
			ev.mUIEvent.mElement = 0;
			ev.mUIEvent.mEventType = UIET_ELEMENT_LEFT;
			lastHovered->OnEvent(ev);
		}

		if ( Hovered )
		{
			ev.mUIEvent.mCaller  = Hovered.get();
			ev.mUIEvent.mElement = Hovered.get();
			ev.mUIEvent.mEventType = UIET_ELEMENT_HOVERED;
			Hovered->OnEvent(ev);
		}
	}
}

//! posts an input event to the environment
bool BaseUI::OnMsgProc(const Event& ev)
{
	switch(ev.mEventType)
	{
	case ET_UI_EVENT:
		// hey, why is the user sending gui events..?
		break;
	case ET_MOUSE_INPUT_EVENT:

		UpdateHoveredElement(Vector2<int>{ev.mMouseInput.X, ev.mMouseInput.Y});

		if (ev.mMouseInput.mEvent == MIE_LMOUSE_PRESSED_DOWN)
			if ( (Hovered && Hovered != Focus) || !Focus )
		{
			SetFocus(Hovered);
		}

		// sending input to focus
		if (Focus && Focus->OnEvent(ev))
			return true;

		// focus could have died in last call
		if (!Focus && Hovered)
			return Hovered->OnEvent(ev);

		break;
	case ET_KEY_INPUT_EVENT:
		{
			if (Focus && Focus->OnEvent(ev))
				return true;

			// For keys we handle the event before changing focus to give elements 
			// the chance for catching the TAB. Send focus changing event
			if (ev.mEventType == ET_KEY_INPUT_EVENT &&
				ev.mKeyInput.mPressedDown &&
				ev.mKeyInput.mKey == KEY_TAB)
			{
				const eastl::shared_ptr<UIElement>& next = 
					GetNextElement(ev.mKeyInput.mShift, ev.mKeyInput.mControl);
				if (next && next != Focus)
				{
					if (SetFocus(next))
						return true;
				}
			}

		}
		break;
	default:
		break;
	} // end switch

	return false;
}


//! sets the focus to an element
bool BaseUI::SetFocus(eastl::shared_ptr<UIElement> element)
{
	if (Focus == element)
		return false;

	// UI Environment should not get the focus
	if (element == Root)
		element = 0;

	// focus may change or be removed in this call
	eastl::shared_ptr<UIElement> currentFocus = 0;
	if (Focus)
	{
		currentFocus = Focus;
		Event ev;
		ev.mEventType = ET_UI_EVENT;
		ev.mUIEvent.mCaller = Focus.get();
		ev.mUIEvent.mElement = element.get();
		ev.mUIEvent.mEventType = UIET_ELEMENT_FOCUS_LOST;
		if (Focus->OnEvent(ev))
			return false;

		currentFocus = 0;
	}

	if (element)
	{
		currentFocus = Focus;

		// send Focused event
		Event ev;
		ev.mEventType = ET_UI_EVENT;
		ev.mUIEvent.mCaller = element.get();
		ev.mUIEvent.mElement = Focus.get();
		ev.mUIEvent.mEventType = UIET_ELEMENT_Focused;
		if (element->OnEvent(ev))
			return false;
	}

	// element is the new focus so it doesn't have to be dropped
	Focus = element;

	return true;
}


//! returns the element with the focus
const eastl::shared_ptr<UIElement>& BaseUI::GetFocus() const
{
	return Focus;
}

//! returns the element last known to be under the mouse cursor
const eastl::shared_ptr<UIElement>& BaseUI::GetHovered() const
{
	return Hovered;
}


//! removes the focus from an element
bool BaseUI::RemoveFocus(const eastl::shared_ptr<UIElement>& element)
{
	if (Focus && Focus==element)
	{
		Event ev;
		ev.mEventType = ET_UI_EVENT;
		ev.mUIEvent.mCaller = Focus.get();
		ev.mUIEvent.mElement = 0;
		ev.mUIEvent.mEventType = UIET_ELEMENT_FOCUS_LOST;
		if (Focus->OnEvent(ev))
			return false;
	}

	if (Focus)
		Focus = 0;

	return true;
}


//! Returns whether the element has focus
bool BaseUI::HasFocus(const eastl::shared_ptr<UIElement>& element, bool checkSubElements) const
{
	if (element == Focus)
		return true;

	if ( !checkSubElements || !element )
		return false;

	eastl::shared_ptr<UIElement> f = Focus;
	while ( f && f->IsSubElement() )
	{
		f = f->GetParent();
		if ( f == element )
			return true;
	}
	return false;
}


//! returns the current gui skin
const eastl::shared_ptr<UISkin>& BaseUI::GetSkin() const
{
	return CurrentSkin;
}


//! Sets a new UI Skin
void BaseUI::SetSkin(const eastl::shared_ptr<UISkin>& skin)
{
	CurrentSkin = skin;
}


//! Creates a new UI Skin based on a template.
// \return Returns a pointer to the created skin.
//	If you no longer need the skin, you should call BaseUISkin::drop().
//	See IReferenceCounted::drop() for more information.
eastl::shared_ptr<UISkin> BaseUI::CreateSkin(EUI_SKIN_THEME_TYPE type)
{
	/*
	eastl::shared_ptr<UISkin> skin(new UISkin(this, type));

	To make the font a little bit nicer, we load an external font
	and set it as the new default font in the skin.

	eastl::shared_ptr<UIFont> font(GetFont("art/fonts/builtinfont.bmp"));
	if (font)
		skin->SetFont(font);

	UIFontBitmap* bitfont = 0;
	if (font && font->GetType() == EGFT_BITMAP)
		bitfont = (UIFontBitmap*)font.get();

	eastl::shared_ptr<UISpriteBank> bank = 0;
	skin->SetFont(font);

	if (bitfont)
		bank = bitfont->GetSpriteBank();

	skin->SetSpriteBank(bank);
	return skin;
	*/
	return eastl::shared_ptr<UISkin>();
}

//! returns the font
eastl::shared_ptr<UIFont> BaseUI::GetFont(const eastl::string& filename)
{
	/*
	// search existing font
	eastl::vector<Font>::iterator itFont = Fonts.begin();
	for (; itFont != Fonts.end(); itFont++)
	{
		if ((*itFont).FontNamedPath == filename)
			return (*itFont).GuiFont;
	}

	eastl::shared_ptr<UIFont> font=0;
	if (!font)
	{
		font = eastl::shared_ptr<UIFont>(new UIFont(this, filename));
		if (!((UIFont*)font.get())->Load(filename))
		{
			font = 0;
			return 0;
		}
	}

	// add to fonts.
	Font f;
	f.GuiFont = font;
	f.FontNamedPath.SetPath(filename);
	Fonts.push_back(f);
	return font;

	/*
	XMLElement* pRoot = XmlResourceLoader::LoadAndReturnRootXMLElement(filename.c_str());
    // font doesn't exist, attempt to load it
	if (!pRoot)
    {
        LogError("Failed to find level resource file: " + filename);
        return 0;
    }
	
	bool found=false;
	// this is an XML font, but we need to know what type
	UI_FONT_TYPE t = GFT_CUSTOM;
	// Loop through each child element and load the component
    for (XMLElement* pNode = pRoot->FirstChildElement(); pNode; pNode = pNode->NextSiblingElement())
    {
		if (eastl::string("font") == eastl::string(pNode->Value()))
		{
			eastl::string type(pNode->Attribute("type"));
			if (eastl::string("vector") == type)
			{
				t = EGFT_VECTOR;
				found=true;
			}
			else if (eastl::string("bitmap") == type)
			{
				t = EGFT_BITMAP;
				found=true;
			}
			else found=true;
		}
        unsigned char flags = 0;
      
    }

	eastl::shared_ptr<UIFont> font=0;
	if (t==EGFT_BITMAP)
	{
		const eastl::shared_ptr<FileSystem>& fileSystem = g_pGameApp->m_pFileSystem;
		font = eastl::shared_ptr<UIFont>(new UIFont(shared_from_this(), filename));
		// change working directory, for loading textures
		path workingDir = fileSystem->GetWorkingDirectory();
		fileSystem->ChangeWorkingDirectoryTo(
			fileSystem->GetFileDir(f.FontNamedPath.GetPath()));

		// load the font
		if (!((UIFont*)font.get())->Load(pRoot))
			font  = 0;

		// change working dir back again
		fileSystem->ChangeWorkingDirectoryTo( workingDir );
	}
	else if (t==EGFT_VECTOR)
	{
		// todo: vector fonts
		LogError("Unable to load font, XML vector fonts are not supported yet " + f.FontNamedPath);

		//UIFontVector* fontVector = new UIFontVector(Driver);
		//font = eastl::shared_ptr<BaseUIFont>(font);
		//if (!font->Load(pRoot))
	}

	if (!font)
	{

		font = eastl::shared_ptr<UIFont>(
			new UIFont(shared_from_this(), f.FontNamedPath.GetPath()));

		if (!((UIFont*)font.get())->Load(f.FontNamedPath.GetPath()))
		{
			font = 0;
			return 0;
		}
	}

	// add to fonts.
	f.GuiFont = font;
	Fonts.push_back(f);
	return font;
	*/
	return eastl::shared_ptr<UIFont>();
}


//! add an externally loaded font
const eastl::shared_ptr<UIFont>& BaseUI::AddFont(
	const eastl::string& name, const eastl::shared_ptr<UIFont>& font)
{
	/*
	if (font)
	{
		eastl::vector<Font>::iterator itFont = Fonts.begin();
		for (; itFont != Fonts.end(); itFont++)
		{
			if ((*itFont).FontNamedPath == name)
				return (*itFont).GuiFont;
		}

		Font f;
		f.GuiFont = font;
		f.FontNamedPath.SetPath(name);
		Fonts.push_back(f);
	}
	*/
	return font;
}

//! remove loaded font
void BaseUI::RemoveFont(const eastl::shared_ptr<UIFont>& font)
{
	if ( !font )
		return;
	/*
	eastl::vector<Font>::iterator itFont = Fonts.begin();
	for (; itFont != Fonts.end(); itFont++)
	{
		if ((*itFont).GuiFont== font)
		{
			(*itFont).GuiFont=0;
			Fonts.erase(itFont);
			return;
		}
	}
	*/
}

//! returns default font
eastl::shared_ptr<UIFont> BaseUI::GetBuiltInFont() const
{
	/*
	if (Fonts.empty())
		return 0;

	return Fonts.front().GuiFont;
	*/
	return eastl::shared_ptr<UIFont>();
}

//! Returns the default element factory which can create all built in elements
eastl::shared_ptr<UIElementFactory> BaseUI::GetDefaultUIElementFactory()
{
	// gui factory
	if (!GetUIElementFactory(0))
	{
		eastl::shared_ptr<UIElementFactory> factory(new DefaultUIElementFactory(this));
		RegisterUIElementFactory(factory);
	}
	return GetUIElementFactory(0);
}


//! Adds an element factory to the gui environment.
// Use this to extend the gui environment with new element types which it should be
// able to create automaticly, for example when loading data from xml files.
void BaseUI::RegisterUIElementFactory(const eastl::shared_ptr<UIElementFactory>& factoryToAdd)
{
	if (factoryToAdd)
		UIElementFactoryList.push_back(factoryToAdd);
}


//! Returns amount of registered scene node factories.
unsigned int BaseUI::GetRegisteredUIElementFactoryCount() const
{
	return UIElementFactoryList.size();
}


//! Returns a scene node factory by index
eastl::shared_ptr<UIElementFactory> BaseUI::GetUIElementFactory(unsigned int index) const
{
	if (index < UIElementFactoryList.size())
		return UIElementFactoryList[index];
	else
		return 0;
}


//! Returns the root gui element.
const eastl::shared_ptr<UIElement>& BaseUI::GetRootUIElement()
{
	return Root;
}


//! Returns the next element in the tab group starting at the Focused element
eastl::shared_ptr<UIElement> BaseUI::GetNextElement(bool reverse, bool group)
{
	// start the search at the root of the current tab group
	eastl::shared_ptr<UIElement> startPos = Focus ? Focus->GetTabGroup() : 0;
	int startOrder = -1;

	// if we're searching for a group
	if (group && startPos)
	{
		startOrder = startPos->GetTabOrder();
	}
	else
	if (!group && Focus && !Focus->IsTabGroup())
	{
		startOrder = Focus->GetTabOrder();
		if (startOrder == -1)
		{
			// this element is not part of the tab cycle,
			// but its parent might be...
			eastl::shared_ptr<UIElement> el = Focus;
			while (el && el->GetParent() && startOrder == -1)
			{
				el = el->GetParent();
				startOrder = el->GetTabOrder();
			}

		}
	}

	if (group || !startPos)
		startPos = Root; // start at the root

	// find the element
	eastl::shared_ptr<UIElement> closest = 0;
	eastl::shared_ptr<UIElement> first = 0;
	startPos->GetNextElement(startOrder, reverse, group, first, closest);

	if (closest)
		return closest; // we found an element
	else if (first)
		return first; // go to the end or the start
	else if (group)
		return Root; // no group found? root group
	else
		return 0;
}

//! adds a UI Element using its name
eastl::shared_ptr<UIElement> BaseUI::AddUIElement(EUI_ELEMENT_TYPE elementType,
	const eastl::shared_ptr<UIElement>& parent)
{
	eastl::shared_ptr<UIElement> node=0;

	for (int i=UIElementFactoryList.size()-1; i>=0 && !node; --i)
		node = UIElementFactoryList[i]->AddUIElement(elementType, parent ? parent : Root);

	return node;
}