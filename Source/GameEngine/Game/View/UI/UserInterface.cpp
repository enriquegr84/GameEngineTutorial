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

const eastl::string BaseUI::DefaultFontName = "#DefaultFont";

//! constructor
BaseUI::BaseUI()
:	Hovered(0), HoveredNoSubelement(0), Focus(0), LastHoveredMousePos(0,0), CurrentSkin(0)
{
	#ifdef _DEBUG
	//BaseUIEnvironment::setDebugName("CUIEnvironment");
	#endif

	Root = eastl::shared_ptr<BaseUIElement>( 
		new UIRoot( this, EUIET_ROOT, 0,  Rectangle<2, int>(Position2<int>(0,0), 
		renderer ? Dimension2<int>(renderer->GetScreenSize()) : Dimension2<int>(0,0))));

	//set tooltip default
	UIToolTip.LastTime = 0;
	UIToolTip.EnterTime = 0;
	UIToolTip.LaunchTime = 1000;
	UIToolTip.RelaunchTime = 500;
	UIToolTip.Element = 0;

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

	const eastl::shared_ptr<BaseUISkin>& skin = CreateSkin( EGSTT_WINDOWS_CLASSIC );
	SetSkin(skin);

	return true;
}

void BaseUI::LoadBuiltInFont()
{	
	/*
	const eastl::shared_ptr<BaseFileSystem>& fileSystem = g_pGameApp->m_pFileSystem;
	BaseReadFile* file = fileSystem->CreateMemoryReadFile(
		BuiltInFontData, BuiltInFontDataSize, DefaultFontName, false);

	eastl::shared_ptr<UIFont> font(new UIFont(this, DefaultFontName ));
	if (!font->Load(file->GetFileName()))
	{
		GE_ERROR("Error: Could not load built-in Font. Did you compile without the BMP loader?");
		SAFE_DELETE( file );
		return;
	}

	Font f;
	f.FontNamedPath.SetPath(DefaultFontName);
	f.GuiFont = font;
	Fonts.push_back(f);

	SAFE_DELETE( file );
	*/
}


//! draws all gui elements
bool BaseUI::OnRender(double fTime, float fElapsedTime)
{
	Dimension2<int> dim(renderer->GetScreenSize());
	if (Root->AbsoluteRect.LowerRightCorner.X != dim.Width ||
		Root->AbsoluteRect.LowerRightCorner.Y != dim.Height)
	{
		// resize gui environment
		Root->DesiredRect.LowerRightCorner = dim;
		Root->AbsoluteClippingRect = Root->DesiredRect;
		Root->AbsoluteRect = Root->DesiredRect;
		Root->UpdateAbsoluteTransformation();
	}

	// make sure tooltip is always on top
	if (UIToolTip.Element)
		Root->BringToFront(UIToolTip.Element);

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
	const eastl::list<eastl::shared_ptr<BaseUIElement>>& children =
		GetRootUIElement()->GetChildren();

	while (!children.empty())
		children.back()->Remove();
}

//
bool BaseUI::OnPostRender( unsigned int time )
{
	// launch tooltip
	if (UIToolTip.Element == 0 &&
		HoveredNoSubelement && HoveredNoSubelement != Root &&
		(time - UIToolTip.EnterTime >= UIToolTip.LaunchTime || 
		(time - UIToolTip.LastTime >= UIToolTip.RelaunchTime && 
		time - UIToolTip.LastTime < UIToolTip.LaunchTime)) &&
		HoveredNoSubelement->GetToolTipText().size() &&
		GetSkin() && GetSkin()->GetFont(EGDF_TOOLTIP) )
	{
		Rectangle<2, int> pos;

		pos.UpperLeftCorner = LastHoveredMousePos;
		Vector2<unsigned int> dim = GetSkin()->GetFont(EGDF_TOOLTIP)->
			GetDimension(HoveredNoSubelement->GetToolTipText().c_str());
		dim.Width += GetSkin()->GetSize(EGDS_TEXT_DISTANCE_X)*2;
		dim.Height += GetSkin()->GetSize(EGDS_TEXT_DISTANCE_Y)*2;

		pos.UpperLeftCorner.Y -= dim.Height+1;
		pos.LowerRightCorner.Y = pos.UpperLeftCorner.Y + dim.Height-1;
		pos.LowerRightCorner.X = pos.UpperLeftCorner.X + dim.Width;

		pos.ConstrainTo(Root->GetAbsolutePosition());

		UIToolTip.Element = AddStaticText(
			HoveredNoSubelement->GetToolTipText().c_str(), pos, true, true, Root, -1, true);
		UIToolTip.Element->SetOverrideColor(GetSkin()->GetColor(EGDC_TOOLTIP));
		UIToolTip.Element->SetBackgroundColor(GetSkin()->GetColor(EGDC_TOOLTIP_BACKGROUND));
		UIToolTip.Element->SetOverrideFont(GetSkin()->GetFont(EGDF_TOOLTIP));
		UIToolTip.Element->SetSubElement(true);

		int textHeight = UIToolTip.Element->GetTextHeight();
		pos = UIToolTip.Element->GetRelativePosition();
		pos.LowerRightCorner.Y = pos.UpperLeftCorner.Y + textHeight;
		UIToolTip.Element->SetRelativePosition(pos);
	}

	// (isVisible() check only because we might use visibility for ToolTip one day)
	if (UIToolTip.Element && UIToolTip.Element->IsVisible() )
	{
		UIToolTip.LastTime = time;

		// got invisible or removed in the meantime?
		if ( !HoveredNoSubelement ||
			!HoveredNoSubelement->IsVisible() ||
			// got invisible or removed in the meantime?
			!HoveredNoSubelement->GetParent())
		{
			UIToolTip.Element->Remove();
			UIToolTip.Element = 0;
		}
	}

	Root->OnPostRender ( time );
	return true;
}


//
void BaseUI::UpdateHoveredElement(Position2<int> mousePos)
{
	shared_ptr<BaseUIElement> lastHovered = Hovered;
	shared_ptr<BaseUIElement> lastHoveredNoSubelement = HoveredNoSubelement;
	LastHoveredMousePos = mousePos;

	Hovered = Root->GetElementFromPoint(mousePos);

	if ( UIToolTip.Element && Hovered == UIToolTip.Element )
	{
		// When the mouse is over the ToolTip we remove that so it will be re-created at a new position.
		// Note that UIToolTip.EnterTime does not get changed here, so it will be re-created at once.
		UIToolTip.Element->Remove();
		UIToolTip.Element = 0;

		// Get the real Hovered
		Hovered = Root->GetElementFromPoint(mousePos);
	}

	// for tooltips we want the element itself and not some of it's subelements
	HoveredNoSubelement = Hovered;
	while ( HoveredNoSubelement && HoveredNoSubelement->IsSubElement() )
	{
		HoveredNoSubelement = HoveredNoSubelement->GetParent();
	}

	if (Hovered != lastHovered)
	{
		Event ev;
		ev.m_EventType = EET_UI_EVENT;

		if (lastHovered)
		{
			ev.m_UIEvent.m_Caller = lastHovered.get();
			ev.m_UIEvent.m_Element = 0;
			ev.m_UIEvent.m_EventType = EGET_ELEMENT_LEFT;
			lastHovered->OnEvent(ev);
		}

		if ( Hovered )
		{
			ev.m_UIEvent.m_Caller  = Hovered.get();
			ev.m_UIEvent.m_Element = Hovered.get();
			ev.m_UIEvent.m_EventType = EGET_ELEMENT_HOVERED;
			Hovered->OnEvent(ev);
		}
	}

	if ( lastHoveredNoSubelement != HoveredNoSubelement )
	{
		if (UIToolTip.Element)
		{
			UIToolTip.Element->Remove();
			UIToolTip.Element = 0;
		}

		if ( HoveredNoSubelement )
		{
			unsigned int now = Timer::GetTime();
			UIToolTip.EnterTime = now;
		}
	}
}

//! posts an input event to the environment
bool BaseUI::OnMsgProc(const Event& ev)
{
	switch(ev.m_EventType)
	{
	case EET_UI_EVENT:
		// hey, why is the user sending gui events..?
		break;
	case EET_MOUSE_INPUT_EVENT:

		UpdateHoveredElement(Position2<int>(ev.m_MouseInput.X, ev.m_MouseInput.Y));

		if (ev.m_MouseInput.m_Event == EMIE_LMOUSE_PRESSED_DOWN)
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
	case EET_KEY_INPUT_EVENT:
		{
			if (Focus && Focus->OnEvent(ev))
				return true;

			// For keys we handle the event before changing focus to give elements 
			// the chance for catching the TAB. Send focus changing event
			if (ev.m_EventType == EET_KEY_INPUT_EVENT &&
				ev.m_KeyInput.m_bPressedDown &&
				ev.m_KeyInput.m_Key == KEY_TAB)
			{
				const shared_ptr<BaseUIElement>& next = 
					GetNextElement(ev.m_KeyInput.m_bShift, ev.m_KeyInput.m_bControl);
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
bool BaseUI::SetFocus(eastl::shared_ptr<BaseUIElement> element)
{
	if (Focus == element)
		return false;

	// UI Environment should not get the focus
	if (element == Root)
		element = 0;

	// focus may change or be removed in this call
	eastl::shared_ptr<BaseUIElement> currentFocus = 0;
	if (Focus)
	{
		currentFocus = Focus;
		Event ev;
		ev.m_EventType = EET_UI_EVENT;
		ev.m_UIEvent.m_Caller = Focus.get();
		ev.m_UIEvent.m_Element = element.get();
		ev.m_UIEvent.m_EventType = EGET_ELEMENT_FOCUS_LOST;
		if (Focus->OnEvent(ev))
			return false;

		currentFocus = 0;
	}

	if (element)
	{
		currentFocus = Focus;

		// send Focused event
		Event ev;
		ev.m_EventType = EET_UI_EVENT;
		ev.m_UIEvent.m_Caller = element.get();
		ev.m_UIEvent.m_Element = Focus.get();
		ev.m_UIEvent.m_EventType = EGET_ELEMENT_Focused;
		if (element->OnEvent(ev))
			return false;
	}

	// element is the new focus so it doesn't have to be dropped
	Focus = element;

	return true;
}


//! returns the element with the focus
const eastl::shared_ptr<BaseUIElement>& BaseUI::GetFocus() const
{
	return Focus;
}

//! returns the element last known to be under the mouse cursor
const eastl::shared_ptr<BaseUIElement>& BaseUI::GetHovered() const
{
	return Hovered;
}


//! removes the focus from an element
bool BaseUI::RemoveFocus(const eastl::shared_ptr<BaseUIElement>& element)
{
	if (Focus && Focus==element)
	{
		Event ev;
		ev.m_EventType = EET_UI_EVENT;
		ev.m_UIEvent.m_Caller = Focus.get();
		ev.m_UIEvent.m_Element = 0;
		ev.m_UIEvent.m_EventType = EGET_ELEMENT_FOCUS_LOST;
		if (Focus->OnEvent(ev))
			return false;
	}

	if (Focus)
		Focus = 0;

	return true;
}


//! Returns whether the element has focus
bool BaseUI::HasFocus(const eastl::shared_ptr<BaseUIElement>& element, bool checkSubElements) const
{
	if (element == Focus)
		return true;

	if ( !checkSubElements || !element )
		return false;

	eastl::shared_ptr<BaseUIElement> f = Focus;
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
	return CurrentSkin;
}


//! Sets a new UI Skin
void BaseUI::SetSkin(const eastl::shared_ptr<BaseUISkin>& skin)
{
	CurrentSkin = skin;
}


//! Creates a new UI Skin based on a template.
// \return Returns a pointer to the created skin.
//	If you no longer need the skin, you should call BaseUISkin::drop().
//	See IReferenceCounted::drop() for more information.
eastl::shared_ptr<BaseUISkin> BaseUI::CreateSkin(EUI_SKIN_THEME_TYPE type)
{
	eastl::shared_ptr<BaseUISkin> skin(new UISkin(this, type));

	/*
	To make the font a little bit nicer, we load an external font
	and set it as the new default font in the skin.
	*/
	eastl::shared_ptr<BaseUIFont> font(GetFont("art/fonts/builtinfont.bmp"));
	if (font)
		skin->SetFont(font);

	BaseUIFontBitmap* bitfont = 0;
	if (font && font->GetType() == EGFT_BITMAP)
		bitfont = (BaseUIFontBitmap*)font.get();

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
	// search existing font
	eastl::vector<Font>::iterator itFont = Fonts.begin();
	for (; itFont != Fonts.end(); itFont++)
	{
		if ((*itFont).FontNamedPath == filename)
			return (*itFont).GuiFont;
	}

	eastl::shared_ptr<BaseUIFont> font=0;
	if (!font)
	{
		font = eastl::shared_ptr<BaseUIFont>(new UIFont(this, filename));
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
        GE_ERROR(eastl::string("Failed to find level resource file: ") + filename);
        return 0;
    }
	
	bool found=false;
	// this is an XML font, but we need to know what type
	EUI_FONT_TYPE t = EGFT_CUSTOM;
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

	eastl::shared_ptr<BaseUIFont> font=0;
	if (t==EGFT_BITMAP)
	{
		const eastl::shared_ptr<BaseFileSystem>& fileSystem = g_pGameApp->m_pFileSystem;
		font = eastl::shared_ptr<BaseUIFont>(new UIFont(shared_from_this(), filename));
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
		GE_ERROR(eastl::string("Unable to load font, XML vector fonts are not supported yet ") + f.FontNamedPath);

		//UIFontVector* fontVector = new UIFontVector(Driver);
		//font = eastl::shared_ptr<BaseUIFont>(font);
		//if (!font->Load(pRoot))
	}

	if (!font)
	{

		font = eastl::shared_ptr<BaseUIFont>(
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
}


//! add an externally loaded font
const shared_ptr<BaseUIFont>& BaseUI::AddFont(const eastl::string& name, const shared_ptr<BaseUIFont>& font)
{
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
	return font;
}

//! remove loaded font
void BaseUI::RemoveFont(const eastl::shared_ptr<BaseUIFont>& font)
{
	if ( !font )
		return;

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
}

//! returns default font
eastl::shared_ptr<BaseUIFont> BaseUI::GetBuiltInFont() const
{
	if (Fonts.empty())
		return 0;

	return Fonts.front().GuiFont;
}


eastl::shared_ptr<BaseUISpriteBank> BaseUI::GetSpriteBank(const eastl::string& filename)
{
	// search for the file name
	eastl::vector<SpriteBank>::iterator itBank = Banks.begin();
	for (; itBank != Banks.end(); itBank++)
	{
		if ((*itBank).SpriteNamedPath == filename)
			return (*itBank).Bank;
	}

	const eastl::shared_ptr<BaseFileSystem>& fileSystem = g_pGameApp->m_pFileSystem;
	// we don't have this sprite bank, we should load it
	if (!fileSystem->ExistFile(filename))
	{
		if ( filename != DefaultFontName )
		{
			GE_WARNING(eastl::string("Could not load sprite bank because the file does not exist ") + filename);
		}
		return 0;
	}

	// todo: load it!
	SpriteBank b;
	b.SpriteNamedPath.SetPath(filename);
	return 0;
}


//! Returns the default element factory which can create all built in elements
eastl::shared_ptr<BaseUIElementFactory> BaseUI::GetDefaultUIElementFactory()
{
	// gui factory
	if (!GetUIElementFactory(0))
	{
		eastl::shared_ptr<BaseUIElementFactory> factory(new DefaultUIElementFactory(this));
		RegisterUIElementFactory(factory);
	}
	return GetUIElementFactory(0);
}


//! Adds an element factory to the gui environment.
// Use this to extend the gui environment with new element types which it should be
// able to create automaticly, for example when loading data from xml files.
void BaseUI::RegisterUIElementFactory(const eastl::shared_ptr<BaseUIElementFactory>& factoryToAdd)
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
eastl::shared_ptr<BaseUIElementFactory> BaseUI::GetUIElementFactory(unsigned int index) const
{
	if (index < UIElementFactoryList.size())
		return UIElementFactoryList[index];
	else
		return 0;
}


//! Returns the root gui element.
const eastl::shared_ptr<BaseUIElement>& BaseUI::GetRootUIElement()
{
	return Root;
}


//! Returns the next element in the tab group starting at the Focused element
eastl::shared_ptr<BaseUIElement> BaseUI::GetNextElement(bool reverse, bool group)
{
	// start the search at the root of the current tab group
	eastl::shared_ptr<BaseUIElement> startPos = Focus ? Focus->GetTabGroup() : 0;
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
			eastl::shared_ptr<BaseUIElement> el = Focus;
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
	eastl::shared_ptr<BaseUIElement> closest = 0;
	eastl::shared_ptr<BaseUIElement> first = 0;
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
eastl::shared_ptr<BaseUIElement> BaseUI::AddUIElement(const c8* elementName, 
	const eastl::shared_ptr<BaseUIElement>& parent)
{
	eastl::shared_ptr<BaseUIElement> node=0;

	for (int i=UIElementFactoryList.size()-1; i>=0 && !node; --i)
		node = UIElementFactoryList[i]->AddUIElement(elementName, parent ? parent : Root);

	return node;
}