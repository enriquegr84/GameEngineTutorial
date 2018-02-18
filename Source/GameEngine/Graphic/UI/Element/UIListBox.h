// Copyright (C) 2002-2012 Nikolaus Gebhardt
// This file is part of the "Irrlicht Engine".
// For conditions of distribution and use, see copyright notice in irrlicht.h

#ifndef UILISTBOX_H
#define UILISTBOX_H

#include "UIElement.h"

#include "Graphic/Effect/ColorEffect.h"
#include "Graphic/Scene/Hierarchy/Visual.h"

class BaseUIFont;
class BaseUIScrollBar;
class BaseUISpriteBank;
class BaseUIStaticText;

//! Enumeration for listbox colors
enum UIListboxColor
{
	//! Color of text
	UI_LBC_TEXT = 0,
	//! Color of selected text
	UI_LBC_TEXT_HIGHLIGHT,
	//! Color of icon
	UI_LBC_ICON,
	//! Color of selected icon
	UI_LBC_ICON_HIGHLIGHT,
	//! Not used, just counts the number of available colors
	UI_LBC_COUNT
};


//! Default list box GUI element.
/** \par This element can create the following events of type EGUI_EVENT_TYPE:
\li EGET_LISTBOX_CHANGED
\li EGET_LISTBOX_SELECTED_AGAIN
*/
class BaseUIListBox : public BaseUIElement
{
public:
	//! constructor
	BaseUIListBox(int id, RectangleShape<2, int> rectangle)
		: BaseUIElement(UIET_LIST_BOX, id, rectangle) {}

	//! Initialize listbox
	virtual void OnInit(bool clip = false) = 0;

	//! returns amount of list items
	virtual unsigned int GetItemCount() const = 0;

	//! returns string of a list item. the may id be a value from 0 to itemCount-1
	virtual const wchar_t* GetListItem(unsigned int id) const = 0;

	//! adds an list item, returns id of item
	virtual unsigned int AddItem(const wchar_t* text) = 0;

	//! adds an list item with an icon
	/** \param text Text of list entry
	\param icon Sprite index of the Icon within the current sprite bank. Set it to -1 if you want no icon
	\return The id of the new created item */
	virtual unsigned int AddItem(const wchar_t* text, int icon) = 0;

	//! Removes an item from the list
	virtual void RemoveItem(unsigned int index) = 0;

	//! get the the id of the item at the given absolute coordinates
	/** \return The id of the listitem or -1 when no item is at those coordinates*/
	virtual int GetItemAt(int xpos, int ypos) const = 0;

	//! Returns the icon of an item
	virtual int GetIcon(unsigned int index) const = 0;

	//! Sets the sprite bank which should be used to draw list icons.
	/** This font is set to the sprite bank of the built-in-font by
	default. A sprite can be displayed in front of every list item.
	An icon is an index within the icon sprite bank. Several
	default icons are available in the skin through getIcon. */
	virtual void SetSpriteBank(const eastl::shared_ptr<BaseUISpriteBank>& bank) = 0;

	//! clears the list, deletes all items in the listbox
	virtual void Clear() = 0;

	//! returns id of selected item. returns -1 if no item is selected.
	virtual int GetSelected() const = 0;

	//! sets the selected item. Set this to -1 if no item should be selected
	virtual void SetSelected(int index) = 0;

	//! sets the selected item. Set this to 0 if no item should be selected
	virtual void SetSelected(const wchar_t *item) = 0;

	//! set whether the listbox should scroll to newly selected items
	virtual void SetAutoScrollEnabled(bool scroll) = 0;

	//! returns true if automatic scrolling is enabled, false if not.
	virtual bool IsAutoScrollEnabled() const = 0;

	//! set all item colors at given index to color
	virtual void SetItemOverrideColor(unsigned int index, eastl::array<float, 4> color) = 0;

	//! set all item colors of specified type at given index to color
	virtual void SetItemOverrideColor(unsigned int index, UIListboxColor colorType, eastl::array<float, 4> color) = 0;

	//! clear all item colors at index
	virtual void ClearItemOverrideColor(unsigned int index) = 0;

	//! clear item color at index for given colortype
	virtual void ClearItemOverrideColor(unsigned int index, UIListboxColor colorType) = 0;

	//! has the item at index its color overwritten?
	virtual bool HasItemOverrideColor(unsigned int index, UIListboxColor colorType) const = 0;

	//! return the overwrite color at given item index.
	virtual eastl::array<float, 4> GetItemOverrideColor(unsigned int index, UIListboxColor colorType) const = 0;

	//! return the default color which is used for the given colorType
	virtual eastl::array<float, 4> GetItemDefaultColor(UIListboxColor colorType) const = 0;

	//! set the item at the given index
	virtual void SetItem(unsigned int index, const wchar_t* text, int icon) = 0;

	//! Insert the item at the given index
	/** \return The index on success or -1 on failure. */
	virtual int InsertItem(unsigned int index, const wchar_t* text, int icon) = 0;

	//! Swap the items at the given indices
	virtual void SwapItems(unsigned int index1, unsigned int index2) = 0;

	//! set global itemHeight
	virtual void SetItemHeight(int height) = 0;

	//! Sets whether to draw the background
	virtual void SetDrawBackground(bool draw) = 0;
};

class UIListBox : public BaseUIListBox
{
public:
	//! constructor
	UIListBox(BaseUI* ui, int id, RectangleShape<2, int> rectangle,
		bool clip=true, bool drawBack=false, bool moveOverSelect=false);

	//! destructor
	virtual ~UIListBox();

	//! initialize listbox
	virtual void OnInit(bool clip = false);

	//! returns amount of list items
	virtual unsigned int GetItemCount() const;

	//! returns string of a list item. the id may be a value from 0 to itemCount-1
	virtual const wchar_t* GetListItem(unsigned int id) const;

	//! adds an list item, returns id of item
	virtual unsigned int AddItem(const wchar_t* text);

	//! clears the list
	virtual void Clear();

	//! returns id of selected item. returns -1 if no item is selected.
	virtual int GetSelected() const;

	//! sets the selected item. Set this to -1 if no item should be selected
	virtual void SetSelected(int id);

	//! sets the selected item. Set this to -1 if no item should be selected
	virtual void SetSelected(const wchar_t *item);

	//! called if an event happened.
	virtual bool OnEvent(const Event& event);

	//! draws the element and its children
	virtual void Draw();

	//! adds an list item with an icon
	//! \param text Text of list entry
	//! \param icon Sprite index of the Icon within the current sprite bank. Set it to -1 if you want no icon
	//! \return
	//! returns the id of the new created item
	virtual unsigned int AddItem(const wchar_t* text, int icon);

	//! Returns the icon of an item
	virtual int GetIcon(unsigned int id) const;

	//! removes an item from the list
	virtual void RemoveItem(unsigned int id);

	//! get the the id of the item at the given absolute coordinates
	virtual int GetItemAt(int xpos, int ypos) const;

	//! Sets the sprite bank which should be used to draw list icons. This font is set to the sprite bank of
	//! the built-in-font by default. A sprite can be displayed in front of every list item.
	//! An icon is an index within the icon sprite bank. Several default icons are available in the
	//! skin through getIcon
	virtual void SetSpriteBank(const eastl::shared_ptr<BaseUISpriteBank>& bank);

	//! set whether the listbox should scroll to newly selected items
	virtual void SetAutoScrollEnabled(bool scroll);

	//! returns true if automatic scrolling is enabled, false if not.
	virtual bool IsAutoScrollEnabled() const;

	//! Update the position and size of the listbox, and update the scrollbar
	virtual void UpdateAbsolutePosition();

	//! set all item colors at given index to color
	virtual void SetItemOverrideColor(unsigned int index, eastl::array<float, 4> color);

	//! set all item colors of specified type at given index to color
	virtual void SetItemOverrideColor(unsigned int index, UIListboxColor colorType, eastl::array<float, 4> color);

	//! clear all item colors at index
	virtual void ClearItemOverrideColor(unsigned int index);

	//! clear item color at index for given colortype
	virtual void ClearItemOverrideColor(unsigned int index, UIListboxColor colorType);

	//! has the item at index its color overwritten?
	virtual bool HasItemOverrideColor(unsigned int index, UIListboxColor colorType) const;

	//! return the overwrite color at given item index.
	virtual eastl::array<float, 4> GetItemOverrideColor(unsigned int index, UIListboxColor colorType) const;

	//! return the default color which is used for the given colorType
	virtual eastl::array<float, 4> GetItemDefaultColor(UIListboxColor colorType) const;

	//! set the item at the given index
	virtual void SetItem(unsigned int index, const wchar_t* text, int icon);

	//! Insert the item at the given index
	/** \return The index on success or -1 on failure. */
	virtual int InsertItem(unsigned int index, const wchar_t* text, int icon);

	//! Swap the items at the given indices
	virtual void SwapItems(unsigned int index1, unsigned int index2);

	//! set global itemHeight
	virtual void SetItemHeight(int height);

	//! Sets whether to draw the background
	virtual void SetDrawBackground(bool draw);


private:

	struct ListItem
	{
		ListItem() : mIcon(-1)
		{}

		eastl::wstring mText;
		int mIcon;

		// A multicolor extension
		struct ListItemOverrideColor
		{
			ListItemOverrideColor() : mUse(false) {}
			bool mUse;
			eastl::array<float, 4> mColor;
		};
		ListItemOverrideColor mOverrideColors[UI_LBC_COUNT];
	};

	void RecalculateItemHeight();
	void SelectNew(int ypos, bool onlyHover=false);
	void RecalculateScrollRectangle();
	void RecalculateScrollPos();

	// extracted that function to avoid copy&paste code
	void RecalculateItemWidth(int icon);

	eastl::vector< ListItem > mItems;
	int mSelected;
	int mItemHeight;
	int mItemHeightOverride;
	int mTotalItemHeight;
	int mItemsIconWidth;

	eastl::shared_ptr<BaseUIFont> mFont;
	eastl::shared_ptr<BaseUISpriteBank> mIconBank;
	eastl::shared_ptr<BaseUIScrollBar> mScrollBar;
	eastl::shared_ptr<BaseUIStaticText> mSelectedText;

	BaseUI* mUI;

	eastl::shared_ptr<Visual> mVisual;
	eastl::shared_ptr<ColorEffect> mEffect;

	unsigned int mSelectTime;
	unsigned int mLastKeyTime;
	eastl::wstring mKeyBuffer;
	bool mSelecting;
	bool mDrawBack;
	bool mMoveOverSelect;
	bool mAutoScroll;
	bool mHighlightWhenNotFocused;
};

#endif
