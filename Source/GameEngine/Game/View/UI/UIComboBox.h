// Copyright (C) 2002-2012 Nikolaus Gebhardt
// This file is part of the "Irrlicht Engine".
// For conditions of distribution and use, see copyright notice in irrlicht.h

#ifndef UICOMBOBOX_H
#define UICOMBOBOX_H

#include "UIElement.h"

#include "Graphic/Effect/BasicEffect.h"
#include "Graphic/Scene/Element/Visual.h"

class BaseUIButton;
class BaseUIListBox;
class BaseUIStaticText;

//! Combobox widget
/** \par This element can create the following events of type EGUI_EVENT_TYPE:
\li EGET_COMBO_BOX_CHANGED
*/
class BaseUIComboBox : public BaseUIElement
{
public:

	//! constructor
	BaseUIComboBox(BaseUI* ui, int id, RectangleBase<2, int> rectangle)
		: BaseUIElement(UIET_COMBO_BOX, id, rectangle) {}

	//! Returns amount of items in box
	virtual unsigned int GetItemCount() const = 0;

	//! Returns string of an item. the idx may be a value from 0 to itemCount-1
	virtual const wchar_t* GetItem(unsigned int idx) const = 0;

	//! Returns item data of an item. the idx may be a value from 0 to itemCount-1
	virtual unsigned int GetItemData(unsigned int idx) const = 0;

	//! Returns index based on item data
	virtual int getIndexForItemData(unsigned int data) const = 0;

	//! Adds an item and returns the index of it
	virtual unsigned int AddItem(const wchar_t* text, unsigned int data = 0) = 0;

	//! Removes an item from the combo box.
	/** Warning. This will change the index of all following items */
	virtual void RemoveItem(unsigned int idx) = 0;

	//! Deletes all items in the combo box
	virtual void Clear() = 0;

	//! Returns id of selected item. returns -1 if no item is selected.
	virtual int GetSelected() const = 0;

	//! Sets the selected item. Set this to -1 if no item should be selected
	virtual void SetSelected(int idx) = 0;

	//! Sets text justification of the text area
	/** \param horizontal: EGUIA_UPPERLEFT for left justified (default),
	EGUIA_LOWEERRIGHT for right justified, or EGUIA_CENTER for centered text.
	\param vertical: EGUIA_UPPERLEFT to align with top edge,
	EGUIA_LOWEERRIGHT for bottom edge, or EGUIA_CENTER for centered text (default). */
	virtual void setTextAlignment(UIAlignment horizontal, UIAlignment vertical) = 0;

	//! Set the maximal number of rows for the selection listbox
	virtual void setMaxSelectionRows(unsigned int max) = 0;

	//! Get the maximimal number of rows for the selection listbox
	virtual unsigned int GetMaxSelectionRows() const = 0;
};

//! Single line edit box for editing simple text.
class UIComboBox : public BaseUIComboBox
{
public:

	//! constructor
	UIComboBox(BaseUI* ui, int id, RectangleBase<2, int> rectangle);

	//! Returns amount of items in box
	virtual unsigned int GetItemCount() const;

	//! returns string of an item. the idx may be a value from 0 to itemCount-1
	virtual const wchar_t* GetItem(unsigned int idx) const;

	//! Returns item data of an item. the idx may be a value from 0 to itemCount-1
	virtual unsigned int GetItemData(unsigned int idx) const;

	//! Returns index based on item data
	virtual int GetIndexForItemData(unsigned int data ) const;

	//! adds an item and returns the index of it
	virtual unsigned int AddItem(const wchar_t* text, unsigned int data);

	//! Removes an item from the combo box.
	virtual void RemoveItem(unsigned int id);

	//! deletes all items in the combo box
	virtual void Clear();

	//! returns the text of the currently selected item
	virtual const wchar_t* GetText() const;

	//! returns id of selected item. returns -1 if no item is selected.
	virtual int GetSelected() const;

	//! sets the selected item. Set this to -1 if no item should be selected
	virtual void SetSelected(int idx);

	//! sets the text alignment of the text part
	virtual void SetTextAlignment(UIAlignment horizontal, UIAlignment vertical);

	//! Set the maximal number of rows for the selection listbox
	virtual void SetMaxSelectionRows(unsigned int max);

	//! Get the maximimal number of rows for the selection listbox
	virtual unsigned int GetMaxSelectionRows() const;

	//! called if an event happened.
	virtual bool OnEvent(const Event& event);

	//! draws the element and its children
	virtual void Draw();

private:

	void OpenCloseMenu();
	void SendSelectionChangedEvent();

	eastl::shared_ptr<BaseUIButton> mListButton;
	eastl::shared_ptr<BaseUIStaticText> mSelectedText;
	eastl::shared_ptr<BaseUIListBox> mListBox;
	eastl::shared_ptr<BaseUIElement> mLastFocus;

	eastl::map<eastl::wstring, eastl::shared_ptr<unsigned int>> Items;

	BaseUI* mUI;

	int mSelected;
	UIAlignment mHAlign, mVAlign;
	int mMaxSelectionRows;
	bool mHasFocus;
};

#endif

