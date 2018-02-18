//========================================================================
// UserInterface.h : Defines UI elements of the GameEngine application
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

#ifndef USERINTERFACE_H
#define USERINTERFACE_H

#include "GameEngineStd.h"

#include "Element/UIRoot.h"
#include "Element/UIFont.h"
#include "Element/UISkin.h"
#include "Element/UIButton.h"
#include "Element/UIWindow.h"
#include "Element/UIElement.h"
#include "Element/UICheckBox.h"
#include "Element/UIComboBox.h"
#include "Element/UIEditBox.h"
#include "Element/UIImage.h"
#include "Element/UIListBox.h"
#include "Element/UIScrollBar.h"
#include "Element/UITreeView.h"
#include "Element/UIStaticText.h"
#include "Element/UISpriteBank.h"

#include "UIElementFactory.h"

#include "Graphic/ScreenElement.h"

const unsigned long QuitNoPrompt = MAKELPARAM(-1,-1);
const UINT MsgEndModal = (WM_USER+100);

enum
{
	// Lower numbers get drawn first
	ZORDER_BACKGROUND, 
	ZORDER_LAYER1, 
	ZORDER_LAYER2, 
	ZORDER_LAYER3, 
	ZORDER_TOOLTIP
};

// This class is a group of user interface controls.
// It can be modal or modeless.


//
// class BaseUI									- Chapter 10, page 286  
//
// This was factored to create a common class that
// implements some of the BaseScreenElement class common
// to modal/modeless dialogs
//
class BaseUI : public BaseScreenElement
{

public:

	//! constructor
	BaseUI();

	//! destructor
	virtual ~BaseUI();

	virtual bool OnInit();
	virtual bool OnRestore() { return true; }
	virtual bool OnLostDevice() { return true; }

	virtual bool IsVisible() { return mVisible; }
	virtual void SetVisible(bool visible) { mVisible = visible; }

	virtual void OnUpdate(int) { };

	//! draws all gui elements
	virtual bool OnRender(double time, float elapsedTime);

	virtual bool OnPostRender( unsigned int time );

	
	//! Posts an input event to the environment.
	/** Usually you do not have to
	use this method, it is used by the engine internally.
	\param event The event to post.
	\return True if succeeded, else false. */
	virtual bool OnMsgProc(const Event& ev);

	// Don't handle any messages
	virtual bool OnEvent( const Event& ev ) { return false; }

	//! removes all elements from the environment
	virtual void Clear();

	//! returns the font
	virtual eastl::shared_ptr<BaseUIFont> GetFont(const eastl::wstring& fileName);

	//! add an externally loaded font
	virtual const eastl::shared_ptr<BaseUIFont>& AddFont(
		const eastl::wstring& name, const eastl::shared_ptr<BaseUIFont>& font);

	//! Returns the element with the focus
	virtual const eastl::shared_ptr<BaseUIElement>& GetFocus() const;

	//! Returns the element last known to be under the mouse
	virtual const eastl::shared_ptr<BaseUIElement>& GetHovered() const;

	//! Returns the root gui element.
	virtual const eastl::shared_ptr<BaseUIElement>& GetRootUIElement();

	//! Returns the default element factory which can create all built in elements
	virtual eastl::shared_ptr<UIElementFactory> GetDefaultUIElementFactory();

	//! Returns a scene node factory by index
	virtual eastl::shared_ptr<UIElementFactory> GetUIElementFactory(unsigned int index) const;

	//! returns the current gui skin
	virtual const eastl::shared_ptr<BaseUISkin>& GetSkin() const;

	//! Sets a new UI Skin
	virtual void SetSkin(const eastl::shared_ptr<BaseUISkin>& skin);

	//! Creates a new UI Skin based on a template.
	/** \return Returns a pointer to the created skin.
	If you no longer need the skin, you should call UISkin::drop().
	See IReferenceCounted::drop() for more information. */
	virtual eastl::shared_ptr<BaseUISkin> CreateSkin(UISkinThemeType type);

	//! returns default font
	virtual eastl::shared_ptr<BaseUIFont> GetBuiltInFont();

	//! returns the sprite bank
	virtual eastl::shared_ptr<BaseUISpriteBank> GetSpriteBank(const eastl::wstring& filename);

	//! returns the sprite bank
	virtual eastl::shared_ptr<BaseUISpriteBank> AddEmptySpriteBank(const eastl::wstring& filename);

	//! remove loaded font
	virtual void RemoveFont(const eastl::shared_ptr<BaseUIFont>& font);

	//! sets the focus to an element
	virtual bool SetFocus(eastl::shared_ptr<BaseUIElement> element);

	//! removes the focus from an element
	virtual bool RemoveFocus(const eastl::shared_ptr<BaseUIElement>& element);

	//! Returns if the element has focus
	virtual bool HasFocus(
		const eastl::shared_ptr<BaseUIElement>& element, bool checkSubElements = false) const;

	//! Adds an element factory to the gui environment.
	/** Use this to extend the gui environment with new element types which it should be
	able to create automaticly, for example when loading data from xml files. */
	virtual void RegisterUIElementFactory(const eastl::shared_ptr<UIElementFactory>& factoryToAdd);

	//! Returns amount of registered scene node factories.
	virtual unsigned int GetRegisteredUIElementFactoryCount() const;

	//! Adds a UI Element by its name
	virtual eastl::shared_ptr<BaseUIElement> AddUIElement(
		UIElementType elementType, const eastl::shared_ptr<BaseUIElement>& parent = 0);

	//! adds an button. The returned pointer must not be dropped.
	virtual eastl::shared_ptr<BaseUIButton> AddButton(const RectangleShape<2, int>& rectangle,
		const eastl::shared_ptr<BaseUIElement>& parent = 0, int id = -1, const wchar_t* text = 0, 
		const wchar_t* tooltiptext = 0);

	//! adds a window. The returned pointer must not be dropped.
	virtual eastl::shared_ptr<BaseUIWindow> AddWindow(const RectangleShape<2, int>& rectangle, bool modal = false,
		const wchar_t* text = 0, const eastl::shared_ptr<BaseUIElement>& parent = 0, int id = -1);

	//! adds a static text. The returned pointer must not be dropped.
	virtual eastl::shared_ptr<BaseUIStaticText> AddStaticText(const wchar_t* text, 
		const RectangleShape<2, int>& rectangle, bool border = false, bool wordWrap = true, 
		const eastl::shared_ptr<BaseUIElement>& parent = 0, int id = -1, bool drawBackground = false);

	//! Adds an edit box. The returned pointer must not be dropped.
	virtual eastl::shared_ptr<BaseUIEditBox> AddEditBox(const wchar_t* text, 
		const RectangleShape<2, int>& rectangle, bool border = false, 
		const eastl::shared_ptr<BaseUIElement>& parent = 0, int id = -1);

	//! Adds an image element.
	virtual eastl::shared_ptr<BaseUIImage> AddImage(eastl::shared_ptr<Texture2> image, Vector2<int> pos, 
		bool useAlphaChannel = true, const eastl::shared_ptr<BaseUIElement>& parent = 0, int id = -1, const wchar_t* text = 0);

	//! adds an image. The returned pointer must not be dropped.
	virtual eastl::shared_ptr<BaseUIImage> AddImage(const RectangleShape<2, int>& rectangle,
		const eastl::shared_ptr<BaseUIElement>& parent = 0, int id = -1, const wchar_t* text = 0, bool useAlphaChannel = true);

	//! adds a scrollbar. The returned pointer must not be dropped.
	virtual eastl::shared_ptr<BaseUIScrollBar> AddScrollBar(bool horizontal, const RectangleShape<2, int>& rectangle,
		const eastl::shared_ptr<BaseUIElement>& parent = 0, int id = -1);

	//! adds a checkbox
	virtual eastl::shared_ptr<BaseUICheckBox> AddCheckBox(bool checked, const RectangleShape<2, int>& rectangle,
		const eastl::shared_ptr<BaseUIElement>& parent = 0, int id = -1, const wchar_t* text = 0);

	//! adds a list box
	virtual eastl::shared_ptr<BaseUIListBox> AddListBox(const RectangleShape<2, int>& rectangle,
		const eastl::shared_ptr<BaseUIElement>& parent = 0, int id = -1, bool drawBackground = false);

	//! adds a tree view
	virtual eastl::shared_ptr<BaseUITreeView> AddTreeView(const RectangleShape<2, int>& rectangle,
		const eastl::shared_ptr<BaseUIElement>& parent = 0, int id = -1, bool drawBackground = false,
		bool scrollBarVertical = true, bool scrollBarHorizontal = false);

	//! Adds a combo box to the environment.
	virtual eastl::shared_ptr<BaseUIComboBox> AddComboBox(const RectangleShape<2, int>& rectangle,
		const eastl::shared_ptr<BaseUIElement>& parent = 0, int id = -1);

protected:
	eastl::shared_ptr<BaseUIElement> mRoot;
	bool mVisible;

private:

	eastl::shared_ptr<BaseUIElement> GetNextElement(bool reverse = false, bool group = false);
	void UpdateHoveredElement(Vector2<int> mousePos);

	struct ToolTip
	{
		eastl::shared_ptr<BaseUIStaticText> mElement;
		unsigned int mLastTime;
		unsigned int mEnterTime;
		unsigned int mLaunchTime;
		unsigned int mRelaunchTime;
	};

	ToolTip mToolTip;

	eastl::vector<eastl::shared_ptr<UIElementFactory>> UIElementFactoryList;

	eastl::map<eastl::wstring, eastl::shared_ptr<BaseUISpriteBank>> mBanks;
	eastl::map<eastl::wstring, eastl::shared_ptr<BaseUIFont>> mFonts;
	eastl::shared_ptr<BaseUIElement> mHovered;
	eastl::shared_ptr<BaseUIElement> mHoveredNoSubelement;	// subelements replaced by their parent, so you only have 'real' elements here
	eastl::shared_ptr<BaseUIElement> mFocus;
	Vector2<int> mLastHoveredMousePos;
	eastl::shared_ptr<BaseUISkin> mCurrentSkin;
};


#endif