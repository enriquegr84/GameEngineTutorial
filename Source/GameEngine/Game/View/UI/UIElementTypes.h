// Copyright (C) 2002-2012 Nikolaus Gebhardt
// This file is part of the "Irrlicht Engine".
// For conditions of distribution and use, see copyright notice in irrlicht.h

#ifndef UIELEMENTTYPES_H
#define UIELEMENTTYPES_H

//! List of all basic Irrlicht UI elements.
/** An IUIElement returns this when calling IUIElement::GetType(); */
enum EUI_ELEMENT_TYPE
{
	//! A button (IUIButton)
	EUIET_BUTTON = 0,

	//! A check box (IUICheckBox)
	EUIET_CHECK_BOX,

	//! A combo box (IUIComboBox)
	EUIET_COMBO_BOX,

	//! A context menu (IUIContextMenu)
	EUIET_CONTEXT_MENU,

	//! A menu (IUIMenu)
	EUIET_MENU,

	//! An edit box (IUIEditBox)
	EUIET_EDIT_BOX,

	//! A file open dialog (IUIFileOpenDialog)
	EUIET_FILE_OPEN_DIALOG,

	//! A color select open dialog (IUIColorSelectDialog)
	EUIET_COLOR_SELECT_DIALOG,

	//! A in/out fader (IUIInOutFader)
	EUIET_IN_OUT_FADER,

	//! An image (IUIImage)
	EUIET_IMAGE,

	//! A list box (IUIListBox)
	EUIET_LIST_BOX,

	//! A mesh viewer (IUIMeshViewer)
	EUIET_MESH_VIEWER,

	//! A message box (IUIWindow)
	EUIET_MESSAGE_BOX,

	//! A modal screen
	EUIET_MODAL_SCREEN,

	//! A scroll bar (IUIScrollBar)
	EUIET_SCROLL_BAR,

	//! A spin box (IUISpinBox)
	EUIET_SPIN_BOX,

	//! A static text (IUIStaticText)
	EUIET_STATIC_TEXT,

	//! A tab (IUITab)
	EUIET_TAB,

	//! A tab control
	EUIET_TAB_CONTROL,

	//! A Table
	EUIET_TABLE,

	//! A tool bar (IUIToolBar)
	EUIET_TOOL_BAR,

	//! A Tree View
	EUIET_TREE_VIEW,

	//! A window
	EUIET_WINDOW,

	//! Unknown type.
	EUIET_ELEMENT,

	//! The root of the UI
	EUIET_ROOT,

	//! Not an element, amount of elements in there
	EUIET_COUNT,

	//! This enum is never used, it only forces the compiler to compile this enumeration to 32 bit.
	EUIET_FORCE_32_BIT = 0x7fffffff

};

#endif




