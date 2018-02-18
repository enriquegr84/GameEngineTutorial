// Copyright (C) 2002-2012 Nikolaus Gebhardt
// This file is part of the "Irrlicht Engine".
// For conditions of distribution and use, see copyright notice in irrlicht.h

#ifndef UIWINDOW_H
#define UIWINDOW_H

#include "UIElement.h"
//#include "EMessageBoxFlags.h"

#include "Graphic/Effect/ColorEffect.h"
#include "Graphic/Scene/Hierarchy/Visual.h"

class BaseUIButton;

//! Default moveable window GUI element with border, caption and close icons.
/** \par This element can create the following events of type EGUI_EVENT_TYPE:
\li UIEVT_ELEMENT_CLOSED
*/
class BaseUIWindow : public BaseUIElement
{
public:

	//! constructor
	BaseUIWindow(int id, RectangleShape<2, int> rectangle)
		: BaseUIElement(UIET_WINDOW, id, rectangle) {}

	//! Initialize Window
	virtual void OnInit() = 0;

	//! Returns pointer to the close button
	/** You can hide the button by calling setVisible(false) on the result. */
	virtual const eastl::shared_ptr<BaseUIButton>& GetCloseButton() const = 0;

	//! Returns pointer to the minimize button
	/** You can hide the button by calling setVisible(false) on the result. */
	virtual const eastl::shared_ptr<BaseUIButton>& GetMinimizeButton() const = 0;

	//! Returns pointer to the maximize button
	/** You can hide the button by calling setVisible(false) on the result. */
	virtual const eastl::shared_ptr<BaseUIButton>& GetMaximizeButton() const = 0;

	//! Returns true if the window can be dragged with the mouse, false if not
	virtual bool IsDraggable() const = 0;

	//! Sets whether the window can be dragged by the mouse
	virtual void SetDraggable(bool draggable) = 0;

	//! Set if the window background will be drawn
	virtual void SetDrawBackground(bool draw) = 0;

	//! Get if the window background will be drawn
	virtual bool GetDrawBackground() const = 0;

	//! Set if the window titlebar will be drawn
	//! Note: If the background is not drawn, then the titlebar is automatically also not drawn
	virtual void SetDrawTitlebar(bool draw) = 0;

	//! Get if the window titlebar will be drawn
	virtual bool GetDrawTitlebar() const = 0;

	//! Returns the rectangle of the drawable area (without border and without titlebar)
	/** The coordinates are given relative to the top-left position of the gui element.<br>
	So to get absolute positions you have to add the resulting rectangle to getAbsolutePosition().UpperLeftCorner.<br>
	To get it relative to the parent element you have to add the resulting rectangle to getRelativePosition().UpperLeftCorner.
	Beware that adding a menu will not change the clientRect as menus are own gui elements, so in that case you might want to subtract
	the menu area additionally. */
	virtual RectangleShape<2, int> GetClientRect() const = 0;
};

class UIWindow : public BaseUIWindow
{
public:

	//! constructor
	UIWindow(BaseUI* ui, int id, RectangleShape<2, int> rectangle);

	//! destructor
	virtual ~UIWindow();

	//! Initialize Window
	virtual void OnInit();

	//! called if an event happened.
	virtual bool OnEvent(const Event& event);

	//! update absolute position
	virtual void UpdateAbsoluteTransformation();

	//! draws the element and its children
	virtual void Draw();

	//! Returns pointer to the close button
	virtual const eastl::shared_ptr<BaseUIButton>& GetCloseButton() const;

	//! Returns pointer to the minimize button
	virtual const eastl::shared_ptr<BaseUIButton>& GetMinimizeButton() const;

	//! Returns pointer to the maximize button
	virtual const eastl::shared_ptr<BaseUIButton>& GetMaximizeButton() const;

	//! Returns true if the window is draggable, false if not
	virtual bool IsDraggable() const;

	//! Sets whether the window is draggable
	virtual void SetDraggable(bool draggable);

	//! Set if the window background will be drawn
	virtual void SetDrawBackground(bool draw);

	//! Get if the window background will be drawn
	virtual bool GetDrawBackground() const;

	//! Set if the window titlebar will be drawn
	//! Note: If the background is not drawn, then the titlebar is automatically also not drawn
	virtual void SetDrawTitlebar(bool draw);

	//! Get if the window titlebar will be drawn
	virtual bool GetDrawTitlebar() const;

	//! Returns the rectangle of the drawable area (without border and without titlebar)
	virtual RectangleShape<2, int> GetClientRect() const;

protected:

	void UpdateClientRect();
	void RefreshSprites();

	BaseUI* mUI;

	eastl::shared_ptr<ColorEffect> mEffect;
	eastl::shared_ptr<Visual> mVisualTitle;
	eastl::shared_ptr<Visual> mVisualBackground;
	eastl::shared_ptr<BaseUIButton> mCloseButton;
	eastl::shared_ptr<BaseUIButton> mMinButton;
	eastl::shared_ptr<BaseUIButton> mRestoreButton;
	RectangleShape<2, int> mClientRect;
	eastl::array<float, 4> mCurrentIconColor;

	Vector2<int> mDragStart;
	bool mDragging, mIsDraggableWindow;
	bool mDrawBackground;
	bool mDrawTitlebar;
	bool mIsActive;
};


#endif

