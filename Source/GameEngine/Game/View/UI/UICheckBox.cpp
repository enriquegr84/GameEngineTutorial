// Copyright (C) 2002-2012 Nikolaus Gebhardt
// This file is part of the "Irrlicht Engine".
// For conditions of distribution and use, see copyright notice in irrlicht.h

#include "UICheckBox.h"

#include "UISkin.h"
#include "UIFont.h"

#include "UserInterface.h"

#include "Graphic/Renderer/Renderer.h"
#include "Core/OS/OS.h"

//! constructor
UICheckBox::UICheckBox(BaseUI* ui, int id, RectangleBase<2, int> rectangle, bool checked)
: BaseUICheckBox(ui, id, rectangle), mUI(ui), mCheckTime(0), mPressed(false), mChecked(checked)
{
	#ifdef _DEBUG
	//setDebugName("CGUICheckBox");
	#endif

	// this element can be tabbed into
	SetTabStop(true);
	SetTabOrder(-1);
}


//! called if an event happened.
bool UICheckBox::OnEvent(const Event& event)
{
	if (IsEnabled())
	{
		switch(event.mEventType)
		{
		case ET_KEY_INPUT_EVENT:
			if (event.mKeyInput.mPressedDown &&
				(event.mKeyInput.mKey == KEY_RETURN || event.mKeyInput.mKey == KEY_SPACE))
			{
				mPressed = true;
				return true;
			}
			else
			if (mPressed && event.mKeyInput.mPressedDown && event.mKeyInput.mKey == KEY_ESCAPE)
			{
				mPressed = false;
				return true;
			}
			else
			if (!event.mKeyInput.mPressedDown && mPressed &&
				(event.mKeyInput.mKey == KEY_RETURN || event.mKeyInput.mKey == KEY_SPACE))
			{
				mPressed = false;
				if (mParent)
				{
					Event newEvent;
					newEvent.mEventType = ET_UI_EVENT;
					newEvent.mUIEvent.mCaller = this;
					newEvent.mUIEvent.mElement = 0;
					mChecked = !mChecked;
					newEvent.mUIEvent.mEventType = UIEVT_CHECKBOX_CHANGED;
					mParent->OnEvent(newEvent);
				}
				return true;
			}
			break;
		case ET_UI_EVENT:
			if (event.mUIEvent.mEventType == UIEVT_ELEMENT_FOCUS_LOST)
			{
				if (event.mUIEvent.mCaller == this)
					mPressed = false;
			}
			break;
		case ET_MOUSE_INPUT_EVENT:
			if (event.mMouseInput.mEvent == MIE_LMOUSE_PRESSED_DOWN)
			{
				mPressed = true;
				mCheckTime = Timer::GetTime();
				mUI->SetFocus(shared_from_this());
				return true;
			}
			else
			if (event.mMouseInput.mEvent == MIE_LMOUSE_LEFT_UP)
			{
				bool wasPressed = mPressed;
				mUI->RemoveFocus(shared_from_this());
				mPressed = false;

				if (wasPressed && mParent)
				{
					/*
					if ( !mAbsoluteClippingRect.IsPointInside( event.mMouseInput.X, event.mMouseInput.Y) ) )
					{
						Pressed = false;
						return true;
					}
					*/
					Event newEvent;
					newEvent.mEventType = ET_UI_EVENT;
					newEvent.mUIEvent.mCaller = this;
					newEvent.mUIEvent.mElement = 0;
					mChecked = !mChecked;
					newEvent.mUIEvent.mEventType = UIEVT_CHECKBOX_CHANGED;
					mParent->OnEvent(newEvent);
				}

				return true;
			}
			break;
		default:
			break;
		}
	}

	return BaseUIElement::OnEvent(event);
}


//! draws the element and its children
void UICheckBox::Draw()
{
	if (!IsVisible())
		return;

	eastl::shared_ptr<BaseUISkin> skin = mUI->GetSkin();
	if (skin)
	{
		const int height = skin->GetSize(DS_CHECK_BOX_WIDTH);

		core::rect<s32> checkRect(AbsoluteRect.UpperLeftCorner.X,
					((AbsoluteRect.getHeight() - height) / 2) + AbsoluteRect.UpperLeftCorner.Y,
					0, 0);

		checkRect.LowerRightCorner.X = checkRect.UpperLeftCorner.X + height;
		checkRect.LowerRightCorner.Y = checkRect.UpperLeftCorner.Y + height;

		EGUI_DEFAULT_COLOR col = EGDC_GRAY_EDITABLE;
		if ( isEnabled() )
			col = Pressed ? EGDC_FOCUSED_EDITABLE : EGDC_EDITABLE;
		skin->draw3DSunkenPane(this, skin->getColor(col),
			false, true, checkRect, &AbsoluteClippingRect);

		if (Checked)
		{
			skin->drawIcon(this, EGDI_CHECK_BOX_CHECKED, checkRect.getCenter(),
				checkTime, os::Timer::getTime(), false, &AbsoluteClippingRect);
		}
		if (Text.size())
		{
			checkRect = AbsoluteRect;
			checkRect.UpperLeftCorner.X += height + 5;

			IGUIFont* font = skin->getFont();
			if (font)
			{
				font->draw(Text.c_str(), checkRect,
						skin->getColor(isEnabled() ? EGDC_BUTTON_TEXT : EGDC_GRAY_TEXT), false, true, &AbsoluteClippingRect);
			}
		}
	}
	IGUIElement::draw();
}


//! set if box is checked
void UICheckBox::SetChecked(bool checked)
{
	mChecked = checked;
}


//! returns if box is checked
bool UICheckBox::IsChecked() const
{
	return mChecked;
}