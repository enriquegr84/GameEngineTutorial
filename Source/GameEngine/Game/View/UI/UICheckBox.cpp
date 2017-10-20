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

	// Create a vertex buffer for a single triangle.
	struct Vertex
	{
		Vector3<float> position;
		Vector4<float> color;
	};
	VertexFormat vformat;
	vformat.Bind(VA_POSITION, DF_R32G32B32_FLOAT, 0);
	vformat.Bind(VA_COLOR, DF_R32G32B32A32_FLOAT, 0);

	eastl::shared_ptr<VertexBuffer> vbuffer = eastl::make_shared<VertexBuffer>(vformat, 4);
	eastl::shared_ptr<IndexBuffer> ibuffer = eastl::make_shared<IndexBuffer>(IP_TRISTRIP, 2);

	eastl::string path = FileSystem::Get()->GetPath("Effects/BasicEffect.fx");
	mEffect = eastl::make_shared<BasicEffect>(ProgramFactory::Get(), path);

	// Create the geometric object for drawing.
	mVisual = eastl::make_shared<Visual>(vbuffer, ibuffer, mEffect);
}

//! initialize checkbox
void UICheckBox::OnInit()
{
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
					if ( !mAbsoluteRect.IsPointInside( event.mMouseInput.X, event.mMouseInput.Y) ) )
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

		RectangleBase<2, int> checkRect;
		checkRect.extent[0] = height;
		checkRect.extent[1] = height;
		checkRect.center[0] = mAbsoluteRect.center[0];
		checkRect.center[1] = mAbsoluteRect.center[1];

		UIDefaultColor col = DC_GRAY_EDITABLE;
		if (IsEnabled())
			col = mPressed ? DC_FOCUSED_EDITABLE : DC_EDITABLE;
		skin->Draw3DSunkenPane(shared_from_this(), skin->GetColor(col),
			false, true, mVisual, checkRect, &mAbsoluteClippingRect);

		if (mChecked)
		{
			skin->DrawIcon(shared_from_this(), DI_CHECK_BOX_CHECKED, checkRect.center, mVisual,
				&mAbsoluteClippingRect, mCheckTime, Timer::GetTime(), false);
		}
		if (mText.size())
		{
			checkRect = mAbsoluteRect;
			checkRect.extent[0] -= height + 5;
			checkRect.center[0] += (int)round((height + 5) / 2.f);

			eastl::shared_ptr<BaseUIFont> font = skin->GetFont();
			if (font)
			{
				font->Draw(mText.c_str(), checkRect,
						skin->GetColor(IsEnabled() ? DC_BUTTON_TEXT : DC_GRAY_TEXT), false, true, &mAbsoluteClippingRect);
			}
		}
	}
	BaseUIElement::Draw();
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