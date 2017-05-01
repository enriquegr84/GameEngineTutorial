// Copyright (C) 2002-2012 Nikolaus Gebhardt
// This file is part of the "Irrlicht Engine".
// For conditions of distribution and use, see copyright notice in irrlicht.h

#include "UIWindow.h"

#include "UISkin.h"
#include "UIButton.h"
#include "UIFont.h"

#include "Graphic/Renderer/Renderer.h"
#include "UserInterface.h"

//! constructor
UIWindow::UIWindow(BaseUI* ui, int id, RectangleBase<2, int> rectangle)
	: BaseUIWindow(id, rectangle), mDragging(false), mIsDraggableWindow(true),
	mDrawBackground(true), mDrawTitlebar(true), mIsActive(false), mUI(ui)
{
#ifdef _DEBUG
	//SetDebugName("UIWindow");
#endif

}

//! destructor
UIWindow::~UIWindow()
{

}

void UIWindow::RefreshSprites()
{
	if (!mUI)
		return;

	const eastl::shared_ptr<BaseUISkin>& skin = mUI->GetSkin();
	if (!skin)
		return;
	/*
	const eastl::shared_ptr<BaseUISpriteBank>& sprites = skin->GetSpriteBank();
	if (!sprites)
		return;
	*/
	mCurrentIconColor = skin->GetColor(IsEnabled() ? DC_WINDOW_SYMBOL : DC_GRAY_WINDOW_SYMBOL);
	/*
	if (sprites)
	{
		mCloseButton->SetSpriteBank(sprites);
		mCloseButton->SetSprite(BS_BUTTON_UP, skin->GetIcon(DI_WINDOW_CLOSE), mCurrentIconColor);
		mCloseButton->SetSprite(BS_BUTTON_DOWN, skin->GetIcon(DI_WINDOW_CLOSE), mCurrentIconColor);

		mRestoreButton->SetSpriteBank(sprites);
		mRestoreButton->SetSprite(BS_BUTTON_UP, skin->GetIcon(DI_WINDOW_RESTORE), mCurrentIconColor);
		mRestoreButton->SetSprite(BS_BUTTON_DOWN, skin->GetIcon(DI_WINDOW_RESTORE), mCurrentIconColor);

		mMinButton->SetSpriteBank(sprites);
		mMinButton->SetSprite(BS_BUTTON_UP, skin->GetIcon(DI_WINDOW_MINIMIZE), mCurrentIconColor);
		mMinButton->SetSprite(BS_BUTTON_DOWN, skin->GetIcon(DI_WINDOW_MINIMIZE), mCurrentIconColor);
	}
	*/
}


//! Initialize Window
void UIWindow::OnInit()
{
	eastl::shared_ptr<BaseUISkin> skin = 0;
	if (mUI)
		skin = mUI->GetSkin();

	mCurrentIconColor = eastl::array<float, 4>{1.f, 1.f, 1.f, 1.f};

	int buttonw = 15;
	if (skin)
	{
		buttonw = skin->GetSize(DS_WINDOW_BUTTON_WIDTH);
	}
	int posx = mRelativeRect.extent[0] - buttonw - 4;
	
	RectangleBase<2, int> rect;
	rect.center[0] = posx;
	rect.center[1] = 3;
	rect.extent[0] = posx + buttonw;
	rect.extent[1] = 3 + buttonw;
	mCloseButton = mUI->AddButton(rect, shared_from_this(), -1,
		L"", skin ? skin->GetDefaultText(DT_WINDOW_CLOSE) : L"Close");
	mCloseButton->SetSubElement(true);
	mCloseButton->SetTabStop(false);
	mCloseButton->SetAlignment(UIA_LOWERRIGHT, UIA_LOWERRIGHT, UIA_UPPERLEFT, UIA_UPPERLEFT);
	posx -= buttonw + 2;

	mRestoreButton = mUI->AddButton(rect, shared_from_this(), -1,
		L"", skin ? skin->GetDefaultText(DT_WINDOW_RESTORE) : L"Restore");
	mRestoreButton->SetVisible(false);
	mRestoreButton->SetSubElement(true);
	mRestoreButton->SetTabStop(false);
	mRestoreButton->SetAlignment(UIA_LOWERRIGHT, UIA_LOWERRIGHT, UIA_UPPERLEFT, UIA_UPPERLEFT);
	posx -= buttonw + 2;

	mMinButton = mUI->AddButton(rect, shared_from_this(), -1,
		L"", skin ? skin->GetDefaultText(DT_WINDOW_MINIMIZE) : L"Minimize");
	mMinButton->SetVisible(false);
	mMinButton->SetSubElement(true);
	mMinButton->SetTabStop(false);
	mMinButton->SetAlignment(UIA_LOWERRIGHT, UIA_LOWERRIGHT, UIA_UPPERLEFT, UIA_UPPERLEFT);

	// this element is a tab group
	SetTabGroup(true);
	SetTabStop(true);
	SetTabOrder(-1);

	RefreshSprites();
	UpdateClientRect();
}

//! called if an event happened.
bool UIWindow::OnEvent(const Event& ev)
{
	if (IsEnabled())
	{

		switch (ev.mEventType)
		{
		case ET_UI_EVENT:
			if (ev.mUIEvent.mEventType == UIEVT_ELEMENT_FOCUS_LOST)
			{
				mDragging = false;
				mIsActive = false;
			}
			else
				if (ev.mUIEvent.mEventType == UIEVT_ELEMENT_FOCUSED)
				{
					if (mParent && ((ev.mUIEvent.mCaller == this) || IsMyChild(ev.mUIEvent.mCaller)))
					{
						mParent->BringToFront(shared_from_this());
						mIsActive = true;
					}
					else
					{
						mIsActive = false;
					}
				}
				else
					if (ev.mUIEvent.mEventType == UIEVT_BUTTON_CLICKED)
					{
						if (ev.mUIEvent.mCaller == mCloseButton.get())
						{
							if (mParent)
							{
								// send close event to parent
								Event e;
								e.mEventType = ET_UI_EVENT;
								e.mUIEvent.mCaller = this;
								e.mUIEvent.mElement = 0;
								e.mUIEvent.mEventType = UIEVT_ELEMENT_CLOSED;

								// if the event was not absorbed
								if (!mParent->OnEvent(e))
									Remove();

								return true;

							}
							else
							{
								Remove();
								return true;
							}
						}
					}
			break;
		case ET_MOUSE_INPUT_EVENT:
			switch (ev.mMouseInput.mEvent)
			{
				case MIE_LMOUSE_PRESSED_DOWN:
					mDragStart[0] = ev.mMouseInput.X;
					mDragStart[1] = ev.mMouseInput.Y;
					mDragging = mIsDraggableWindow;
					if (mParent)
						mParent->BringToFront(shared_from_this());
					return true;
				case MIE_LMOUSE_LEFT_UP:
					mDragging = false;
					return true;
				case MIE_MOUSE_MOVED:
					if (!ev.mMouseInput.IsLeftPressed())
						mDragging = false;

					if (mDragging)
					{
						// gui window should not be dragged outside its parent
						if (mParent)
						{
							eastl::array<Vector2<int>, 4U> vertex;
							mParent->GetAbsolutePosition().GetVertices(vertex);
							if (ev.mMouseInput.X < vertex[1][0] + 1 ||
								ev.mMouseInput.Y < vertex[1][1] + 1 ||
								ev.mMouseInput.X > vertex[3][0] - 1 ||
								ev.mMouseInput.Y > vertex[3][0] - 1)
								return true;
						}

						Move(Vector2<int>{ev.mMouseInput.X - mDragStart[0], ev.mMouseInput.Y - mDragStart[1]});
						mDragStart[0] = ev.mMouseInput.X;
						mDragStart[1] = ev.mMouseInput.Y;
						return true;
					}
					break;
				default:
					break;
				}
			default:
				break;
		}
	}

	return BaseUIElement::OnEvent(ev);
}


//! Updates the absolute position.
void UIWindow::UpdateAbsoluteTransformation()
{
	BaseUIElement::UpdateAbsoluteTransformation();
}


//! draws the element and its children
void UIWindow::Draw()
{
	if (mVisible)
	{
		const eastl::shared_ptr<BaseUISkin>& skin = mUI->GetSkin();

		// update each time because the skin is allowed to change this always.
		UpdateClientRect();
		/*
		if (mCurrentIconColor != skin->GetColor(IsEnabled() ? DC_WINDOW_SYMBOL : DC_GRAY_WINDOW_SYMBOL))
			RefreshSprites();
		*/
		RectangleBase<2, int> rect = mAbsoluteRect;

		// draw body fast
		if (mDrawBackground)
		{
			rect = skin->Draw3DWindowBackground(shared_from_this(), mDrawTitlebar,
				skin->GetColor(mIsActive ? DC_ACTIVE_BORDER : DC_INACTIVE_BORDER),
				mAbsoluteRect, &mAbsoluteClippingRect);

			if (mDrawTitlebar && mText.size())
			{
				rect.center[0] += skin->GetSize(DS_TITLEBARTEXT_DISTANCE_X);
				rect.center[1] += skin->GetSize(DS_TITLEBARTEXT_DISTANCE_Y);
				rect.center[0] -= skin->GetSize(DS_WINDOW_BUTTON_WIDTH) + 5;

				rect.extent[0] += skin->GetSize(DS_TITLEBARTEXT_DISTANCE_X);
				rect.extent[1] += skin->GetSize(DS_TITLEBARTEXT_DISTANCE_Y);
				rect.extent[0] -= skin->GetSize(DS_WINDOW_BUTTON_WIDTH) + 5;

				eastl::shared_ptr<BaseUIFont> font = skin->GetFont(DF_WINDOW);
				if (font)
				{
					font->Draw(mText.c_str(), rect,
						skin->GetColor(mIsActive ? DC_ACTIVE_CAPTION : DC_INACTIVE_CAPTION),
						false, true, &mAbsoluteClippingRect);
				}
			}
		}
	}

	BaseUIElement::Draw();
}


//! Returns pointer to the close button
const eastl::shared_ptr<BaseUIButton>& UIWindow::GetCloseButton() const
{
	return mCloseButton;
}


//! Returns pointer to the minimize button
const eastl::shared_ptr<BaseUIButton>& UIWindow::GetMinimizeButton() const
{
	return mMinButton;
}


//! Returns pointer to the maximize button
const eastl::shared_ptr<BaseUIButton>& UIWindow::GetMaximizeButton() const
{
	return mRestoreButton;
}


//! Returns true if the window is draggable, false if not
bool UIWindow::IsDraggable() const
{
	return mIsDraggableWindow;
}


//! Sets whether the window is draggable
void UIWindow::SetDraggable(bool draggable)
{
	mIsDraggableWindow = draggable;

	if (mDragging && !mIsDraggableWindow)
		mDragging = false;
}


//! Set if the window background will be drawn
void UIWindow::SetDrawBackground(bool draw)
{
	mDrawBackground = draw;
}


//! Get if the window background will be drawn
bool UIWindow::GetDrawBackground() const
{
	return mDrawBackground;
}


//! Set if the window titlebar will be drawn
void UIWindow::SetDrawTitlebar(bool draw)
{
	mDrawTitlebar = draw;
}


//! Get if the window titlebar will be drawn
bool UIWindow::GetDrawTitlebar() const
{
	return mDrawTitlebar;
}


void UIWindow::UpdateClientRect()
{
	if (!mDrawBackground)
	{
		Vector2<int> center;
		mClientRect = RectangleBase<2, int>(center, mAbsoluteRect.axis, mAbsoluteRect.extent);
		return;
	}
	const eastl::shared_ptr<BaseUISkin>& skin = mUI->GetSkin();
	/*
	skin->Draw3DWindowBackground(shared_from_this(), mDrawTitlebar,
		skin->GetColor(mIsActive ? DC_ACTIVE_BORDER : DC_INACTIVE_BORDER),
		mAbsoluteRect, &mAbsoluteClippingRect, &mClientRect);
	*/
	mClientRect.center -= mAbsoluteRect.center;
}


//! Returns the rectangle of the drawable area (without border, without titlebar and without scrollbars)
RectangleBase<2, int> UIWindow::GetClientRect() const
{
	return mClientRect;
}