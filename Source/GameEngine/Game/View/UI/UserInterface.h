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

#include "UI/UIRoot.h"
#include "UI/UIFont.h"
#include "UI/UISkin.h"
#include "UI/UIElement.h"
#include "UI/UIElementFactory.h"

const unsigned long g_QuitNoPrompt = MAKELPARAM(-1,-1);
const UINT g_MsgEndModal = (WM_USER+100);

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

	virtual bool IsVisible() { return Visible; }
	virtual void SetVisible(bool visible) { Visible = visible; }

	virtual void OnUpdate(int) { };

	//! draws all gui elements
	virtual bool OnRender(double fTime, float fElapsedTime);

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
	virtual eastl::shared_ptr<UIFont> GetFont(const path& filename);

	//! add an externally loaded font
	virtual const eastl::shared_ptr<UIFont>& AddFont(
		const path& name, const eastl::shared_ptr<UIFont>& font);

	//! Returns the element with the focus
	virtual const eastl::shared_ptr<UIElement>& GetFocus() const;

	//! Returns the element last known to be under the mouse
	virtual const eastl::shared_ptr<UIElement>& GetHovered() const;

	//! Returns the root gui element.
	virtual const eastl::shared_ptr<UIElement>& GetRootUIElement();

	//! Returns the default element factory which can create all built in elements
	virtual eastl::shared_ptr<UIElementFactory> GetDefaultUIElementFactory();

	//! Returns a scene node factory by index
	virtual eastl::shared_ptr<UIElementFactory> GetUIElementFactory(u32 index) const;

	//! returns the current gui skin
	virtual const eastl::shared_ptr<UISkin>& GetSkin() const;

	//! Sets a new UI Skin
	virtual void SetSkin(const eastl::shared_ptr<UISkin>& skin);

	//! Creates a new UI Skin based on a template.
	/** \return Returns a pointer to the created skin.
	If you no longer need the skin, you should call UISkin::drop().
	See IReferenceCounted::drop() for more information. */
	virtual eastl::shared_ptr<UISkin> CreateSkin(EUI_SKIN_THEME_TYPE type);

	//! returns default font
	virtual eastl::shared_ptr<UIFont> GetBuiltInFont() const;

	//! remove loaded font
	virtual void RemoveFont(const eastl::shared_ptr<UIFont>& font);

	//! sets the focus to an element
	virtual bool SetFocus(eastl::shared_ptr<UIElement> element);

	//! removes the focus from an element
	virtual bool RemoveFocus(const eastl::shared_ptr<UIElement>& element);

	//! Returns if the element has focus
	virtual bool HasFocus(
		const eastl::shared_ptr<UIElement>& element, bool checkSubElements = false) const;

	//! Adds an element factory to the gui environment.
	/** Use this to extend the gui environment with new element types which it should be
	able to create automaticly, for example when loading data from xml files. */
	virtual void RegisterUIElementFactory(const eastl::shared_ptr<UIElementFactory>& factoryToAdd);

	//! Returns amount of registered scene node factories.
	virtual u32 GetRegisteredUIElementFactoryCount() const;

	//! Adds a UI Element by its name
	virtual eastl::shared_ptr<UIElement> AddUIElement(const c8* elementName,
		const eastl::shared_ptr<UIElement>& parent = 0);


protected:
	eastl::shared_ptr<BaseUIElement> Root;
	bool Visible;
};


#endif