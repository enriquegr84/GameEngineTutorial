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

//! constructor
BaseUI::BaseUI()
	: mHovered(0), mHoveredNoSubelement(0), mFocus(0), 
	mLastHoveredMousePos{ 0,0 }, mCurrentSkin(0)
{
	Renderer* renderer = Renderer::Get();
	Vector2<unsigned int> screenSize(renderer->GetScreenSize());
	RectangleBase<2, int> screenRectangle;
	screenRectangle.extent[0] = (int)screenSize[0];
	screenRectangle.extent[1] = (int)screenSize[1];

	mRoot = eastl::shared_ptr<BaseUIElement>(
		new UIRoot(this, UIET_ROOT, 0, screenRectangle));

	// environment is root tab group
	mRoot->SetTabGroup(true);
}


//! destructor
BaseUI::~BaseUI()
{

}

bool BaseUI::OnInit()
{
	//LoadBuiltInFont();

	const eastl::shared_ptr<BaseUISkin>& skin = CreateSkin( STT_WINDOWS_CLASSIC );
	SetSkin(skin);

	return true;
}

//! draws all gui elements
bool BaseUI::OnRender(double fTime, float fElapsedTime)
{
	Renderer* renderer = Renderer::Get();
	Vector2<unsigned int> screenSize(renderer->GetScreenSize());
	if (mRoot->mAbsoluteRect.extent[0] != screenSize[0] ||
		mRoot->mAbsoluteRect.extent[1] != screenSize[1])
	{
		// resize gui environment
		mRoot->mDesiredRect.extent[0] = (int)screenSize[0];
		mRoot->mDesiredRect.extent[1] = (int)screenSize[1];
		mRoot->mAbsoluteClippingRect = mRoot->mDesiredRect;
		mRoot->mAbsoluteRect = mRoot->mDesiredRect;
		mRoot->UpdateAbsoluteTransformation();
	}

	mRoot->Draw();

	return OnPostRender ( Timer::GetTime () );
}


//! clear all UI elements
void BaseUI::Clear()
{
	// Remove the focus
	if (mFocus)
		mFocus = 0;

	if (mHovered && mHovered != mRoot)
		mHovered = 0;
	
	if ( mHoveredNoSubelement && mHoveredNoSubelement != mRoot)
		mHoveredNoSubelement = 0;

	// get the root's children in case the root changes in future
	const eastl::list<eastl::shared_ptr<BaseUIElement>>& children =
		GetRootUIElement()->GetChildren();

	while (!children.empty())
		children.back()->Remove();
}

//
bool BaseUI::OnPostRender( unsigned int time )
{
	mRoot->OnPostRender ( time );
	return true;
}


//
void BaseUI::UpdateHoveredElement(Vector2<int> mousePos)
{
	eastl::shared_ptr<BaseUIElement> lastHovered = mHovered;
	eastl::shared_ptr<BaseUIElement> lastHoveredNoSubelement = mHoveredNoSubelement;
	mLastHoveredMousePos = mousePos;

	mHovered = mRoot->GetElementFromPoint(mousePos);

	// for tooltips we want the element itself and not some of it's subelements
	mHoveredNoSubelement = mHovered;
	while ( mHoveredNoSubelement && mHoveredNoSubelement->IsSubElement() )
	{
		mHoveredNoSubelement = mHoveredNoSubelement->GetParent();
	}

	if (mHovered != lastHovered)
	{
		Event ev;
		ev.mEventType = ET_UI_EVENT;

		if (lastHovered)
		{
			ev.mUIEvent.mCaller = lastHovered.get();
			ev.mUIEvent.mElement = 0;
			ev.mUIEvent.mEventType = UIEVT_ELEMENT_LEFT;
			lastHovered->OnEvent(ev);
		}

		if ( mHovered )
		{
			ev.mUIEvent.mCaller  = mHovered.get();
			ev.mUIEvent.mElement = mHovered.get();
			ev.mUIEvent.mEventType = UIEVT_ELEMENT_HOVERED;
			mHovered->OnEvent(ev);
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
			if ( (mHovered && mHovered != mFocus) || !mFocus )
		{
			SetFocus(mHovered);
		}

		// sending input to focus
		if (mFocus && mFocus->OnEvent(ev))
			return true;

		// focus could have died in last call
		if (!mFocus && mHovered)
			return mHovered->OnEvent(ev);

		break;
	case ET_KEY_INPUT_EVENT:
		{
			if (mFocus && mFocus->OnEvent(ev))
				return true;

			// For keys we handle the event before changing focus to give elements 
			// the chance for catching the TAB. Send focus changing event
			if (ev.mEventType == ET_KEY_INPUT_EVENT &&
				ev.mKeyInput.mPressedDown &&
				ev.mKeyInput.mKey == KEY_TAB)
			{
				const eastl::shared_ptr<BaseUIElement>& next =
					GetNextElement(ev.mKeyInput.mShift, ev.mKeyInput.mControl);
				if (next && next != mFocus)
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
bool BaseUI::SetFocus(eastl::shared_ptr<BaseUIElement> element)
{
	if (mFocus == element)
		return false;

	// UI Environment should not get the focus
	if (element == mRoot)
		element = 0;

	// focus may change or be removed in this call
	eastl::shared_ptr<BaseUIElement> currentFocus = 0;
	if (mFocus)
	{
		currentFocus = mFocus;
		Event ev;
		ev.mEventType = ET_UI_EVENT;
		ev.mUIEvent.mCaller = mFocus.get();
		ev.mUIEvent.mElement = element.get();
		ev.mUIEvent.mEventType = UIEVT_ELEMENT_FOCUS_LOST;
		if (mFocus->OnEvent(ev))
			return false;

		currentFocus = 0;
	}

	if (element)
	{
		currentFocus = mFocus;

		// send Focused event
		Event ev;
		ev.mEventType = ET_UI_EVENT;
		ev.mUIEvent.mCaller = element.get();
		ev.mUIEvent.mElement = mFocus.get();
		ev.mUIEvent.mEventType = UIEVT_ELEMENT_FOCUSED;
		if (element->OnEvent(ev))
			return false;
	}

	// element is the new focus so it doesn't have to be dropped
	mFocus = element;

	return true;
}


//! returns the element with the focus
const eastl::shared_ptr<BaseUIElement>& BaseUI::GetFocus() const
{
	return mFocus;
}

//! returns the element last known to be under the mouse cursor
const eastl::shared_ptr<BaseUIElement>& BaseUI::GetHovered() const
{
	return mHovered;
}


//! removes the focus from an element
bool BaseUI::RemoveFocus(const eastl::shared_ptr<BaseUIElement>& element)
{
	if (mFocus && mFocus==element)
	{
		Event ev;
		ev.mEventType = ET_UI_EVENT;
		ev.mUIEvent.mCaller = mFocus.get();
		ev.mUIEvent.mElement = 0;
		ev.mUIEvent.mEventType = UIEVT_ELEMENT_FOCUS_LOST;
		if (mFocus->OnEvent(ev))
			return false;
	}

	if (mFocus)
		mFocus = 0;

	return true;
}


//! Returns whether the element has focus
bool BaseUI::HasFocus(const eastl::shared_ptr<BaseUIElement>& element, bool checkSubElements) const
{
	if (element == mFocus)
		return true;

	if ( !checkSubElements || !element )
		return false;

	eastl::shared_ptr<BaseUIElement> f = mFocus;
	while ( f && f->IsSubElement() )
	{
		f = f->GetParent();
		if ( f == element )
			return true;
	}
	return false;
}


//! returns the current gui skin
const eastl::shared_ptr<BaseUISkin>& BaseUI::GetSkin() const
{
	return mCurrentSkin;
}


//! Sets a new UI Skin
void BaseUI::SetSkin(const eastl::shared_ptr<BaseUISkin>& skin)
{
	mCurrentSkin = skin;
}


//! Creates a new UI Skin based on a template.
// \return Returns a pointer to the created skin.
//	If you no longer need the skin, you should call BaseUISkin::drop().
//	See IReferenceCounted::drop() for more information.
eastl::shared_ptr<BaseUISkin> BaseUI::CreateSkin(UISkinThemeType type)
{

	eastl::shared_ptr<UISkin> skin(new UISkin(this, type));
	/*
	To make the font a little bit nicer, we load an external font
	and set it as the new default font in the skin.
	*/
	eastl::shared_ptr<BaseUIFont> font(GetFont("art/fonts/builtinfont.bmp"));
	if (font)
		skin->SetFont(font);

	UIFontBitmap* bitfont = 0;
	if (font && font->GetType() == FT_BITMAP)
		bitfont = (UIFontBitmap*)font.get();

	eastl::shared_ptr<BaseUISpriteBank> bank = 0;
	skin->SetFont(font);

	if (bitfont)
		bank = bitfont->GetSpriteBank();

	skin->SetSpriteBank(bank);
	return skin;
}

//! returns the font
eastl::shared_ptr<BaseUIFont> BaseUI::GetFont(const eastl::string& filename)
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
		if (eastl::string("font").compare(eastl::string(pNode->Value())) == 0)
		{
			eastl::string type(pNode->Attribute("type"));
			if (eastl::string("vector").compare(type) == 0)
			{
				t = EGFT_VECTOR;
				found=true;
			}
			else if (eastl::string("bitmap").compare(type) == 0)
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
		font = eastl::shared_ptr<UIFont>(new UIFont(shared_from_this(), filename));
		// change working directory, for loading textures
		path workingDir = FileSystem::Get()->GetWorkingDirectory();
		FileSystem::Get()->ChangeWorkingDirectoryTo(
			FileSystem::Get()->GetFileDir(f.FontNamedPath.GetPath()));

		// load the font
		if (!((UIFont*)font.get())->Load(pRoot))
			font  = 0;

		// change working dir back again
		FileSystem::Get()->ChangeWorkingDirectoryTo( workingDir );
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
	return eastl::shared_ptr<BaseUIFont>();
}


//! add an externally loaded font
const eastl::shared_ptr<BaseUIFont>& BaseUI::AddFont(
	const eastl::string& name, const eastl::shared_ptr<BaseUIFont>& font)
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
void BaseUI::RemoveFont(const eastl::shared_ptr<BaseUIFont>& font)
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
eastl::shared_ptr<BaseUIFont> BaseUI::GetBuiltInFont() const
{
	/*
	if (Fonts.empty())
		return 0;

	return Fonts.front().GuiFont;
	*/
	return eastl::shared_ptr<BaseUIFont>();
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
const eastl::shared_ptr<BaseUIElement>& BaseUI::GetRootUIElement()
{
	return mRoot;
}


//! Returns the next element in the tab group starting at the Focused element
eastl::shared_ptr<BaseUIElement> BaseUI::GetNextElement(bool reverse, bool group)
{
	// start the search at the root of the current tab group
	eastl::shared_ptr<BaseUIElement> startPos = mFocus ? mFocus->GetTabGroup() : 0;
	int startOrder = -1;

	// if we're searching for a group
	if (group && startPos)
	{
		startOrder = startPos->GetTabOrder();
	}
	else
	if (!group && mFocus && !mFocus->IsTabGroup())
	{
		startOrder = mFocus->GetTabOrder();
		if (startOrder == -1)
		{
			// this element is not part of the tab cycle,
			// but its parent might be...
			eastl::shared_ptr<BaseUIElement> el = mFocus;
			while (el && el->GetParent() && startOrder == -1)
			{
				el = el->GetParent();
				startOrder = el->GetTabOrder();
			}

		}
	}

	if (group || !startPos)
		startPos = mRoot; // start at the root

	// find the element
	eastl::shared_ptr<BaseUIElement> closest = 0;
	eastl::shared_ptr<BaseUIElement> first = 0;
	startPos->GetNextElement(startOrder, reverse, group, first, closest);

	if (closest)
		return closest; // we found an element
	else if (first)
		return first; // go to the end or the start
	else if (group)
		return mRoot; // no group found? root group
	else
		return 0;
}

//! adds a UI Element using its name
eastl::shared_ptr<BaseUIElement> BaseUI::AddUIElement(UIElementType elementType,
	const eastl::shared_ptr<BaseUIElement>& parent)
{
	eastl::shared_ptr<BaseUIElement> node=0;

	for (int i=UIElementFactoryList.size()-1; i>=0 && !node; --i)
		node = UIElementFactoryList[i]->AddUIElement(elementType, parent ? parent : mRoot);

	return node;
}

//! adds a button. The returned pointer must not be dropped.
eastl::shared_ptr<BaseUIButton> BaseUI::AddButton(const RectangleBase<2, int>& rectangle,
	const eastl::shared_ptr<BaseUIElement>& parent, int id, const wchar_t* text, const wchar_t *tooltiptext)
{
	eastl::shared_ptr<BaseUIButton> button(new UIButton(this, id, rectangle));
	button->SetParent(parent ? parent : mRoot);
	button->OnInit();
	if (text)
		button->SetText(text);

	if (tooltiptext)
		button->SetToolTipText(tooltiptext);

	return button;
}


//! adds a static text. The returned pointer must not be dropped.
eastl::shared_ptr<BaseUIStaticText> BaseUI::AddStaticText(const wchar_t* text, const RectangleBase<2, int>& rectangle,
	bool border, bool wordWrap, const eastl::shared_ptr<BaseUIElement>& parent, int id, bool background)
{
	eastl::shared_ptr<BaseUIStaticText> staticText(new UIStaticText(this, id, text, border, rectangle, background));
	staticText->SetParent(parent ? parent : mRoot);
	staticText->SetWordWrap(wordWrap);
	return staticText;
}


//! adds a window. The returned pointer must not be dropped.
eastl::shared_ptr<BaseUIWindow> BaseUI::AddWindow(const RectangleBase<2, int>& rectangle, bool modal,
	const wchar_t* text, const eastl::shared_ptr<BaseUIElement>& parent, int id)
{
	eastl::shared_ptr<BaseUIWindow> win(new UIWindow(this, id, rectangle));
	win->SetParent(parent ? parent : mRoot);
	win->OnInit();
	if (text)
		win->SetText(text);

	if (modal)
	{
		// Careful, don't just set the modal as parent above. That will mess up the focus 
		// (and is hard to change because we have to be very careful not to get virtual function 
		//	call, like OnEvent, in the window.
		/*
		eastl::shared_ptr<BaseUIModalScreen> modalScreen(new UIModalScreen(this, parent ? parent : Root, -1));
		modalScreen->SetParent(parent ? parent : mRoot);
		modalScreen->AddChild(win);
		*/
	}

	return win;
}