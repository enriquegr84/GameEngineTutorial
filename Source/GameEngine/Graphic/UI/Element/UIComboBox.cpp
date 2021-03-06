// Copyright (C) 2002-2012 Nikolaus Gebhardt
// This file is part of the "Irrlicht Engine".
// For conditions of distribution and use, see copyright notice in irrlicht.h

#include "UIComboBox.h"

#include "Core/OS/OS.h"

#include "Graphic/Image/ImageResource.h"
#include "Graphic/Renderer/Renderer.h"

//! constructor
UIComboBox::UIComboBox(BaseUI* ui, int id, RectangleShape<2, int> rectangle)
: BaseUIComboBox(ui, id, rectangle),
	mUI(ui), mListButton(0), mSelectedText(0), mListBox(0), mLastFocus(0),
	mSelected(-1), mHAlign(UIA_UPPERLEFT), mVAlign(UIA_CENTER), mMaxSelectionRows(5), mHasFocus(false)
{
	eastl::shared_ptr<ResHandle>& resHandle =
		ResCache::Get()->GetHandle(&BaseResource(L"Art/UserControl/appbar.empty.png"));
	if (resHandle)
	{
		const eastl::shared_ptr<ImageResourceExtraData>& extra =
			eastl::static_pointer_cast<ImageResourceExtraData>(resHandle->GetExtra());
		extra->GetImage()->AutogenerateMipmaps();

		// Create a vertex buffer for a two-triangles square. The PNG is stored
		// in left-handed coordinates. The texture coordinates are chosen to
		// reflect the texture in the y-direction.
		VertexFormat vformat;
		vformat.Bind(VA_POSITION, DF_R32G32B32_FLOAT, 0);
		vformat.Bind(VA_TEXCOORD, DF_R32G32_FLOAT, 0);

		eastl::shared_ptr<VertexBuffer> vbuffer = eastl::make_shared<VertexBuffer>(vformat, 4);
		eastl::shared_ptr<IndexBuffer> ibuffer = eastl::make_shared<IndexBuffer>(IP_TRISTRIP, 2);
		vbuffer->SetUsage(Resource::DYNAMIC_UPDATE);

		// Create an effect for the vertex and pixel shaders. The texture is
		// bilinearly filtered and the texture coordinates are clamped to [0,1]^2.
		eastl::vector<eastl::string> path;
#if defined(_OPENGL_)
		path.push_back("Effects/Texture2EffectVS.glsl");
		path.push_back("Effects/Texture2EffectPS.glsl");
#else
		path.push_back("Effects/Texture2EffectVS.hlsl");
		path.push_back("Effects/Texture2EffectPS.hlsl");
#endif
		mEffect = eastl::make_shared<Texture2Effect>(ProgramFactory::Get(), path, extra->GetImage(),
			SamplerState::MIN_L_MAG_L_MIP_P, SamplerState::CLAMP, SamplerState::CLAMP);

		// Create the geometric object for drawing.
		mVisual = eastl::make_shared<Visual>(vbuffer, ibuffer, mEffect);
	}
}

//! initialize combobox
void UIComboBox::OnInit()
{
	eastl::shared_ptr<BaseUISkin> skin = mUI->GetSkin();

	int width = 15;
	if (skin)
		width = skin->GetSize(DS_WINDOW_BUTTON_WIDTH);

	mListBox.reset(new UIListBox(mUI, -1, mAbsoluteRect, false, true, true));
	mListBox->SetParent(shared_from_this());
	mListBox->OnInit();
	mListBox->SetSubElement(true);
	mListBox->SetNotClipped(true);
	mListBox->SetEnabled(false);
	mListBox->SetVisible(false);

	RectangleShape<2, int> r;
	r.mExtent[0] = width;
	r.mExtent[1] = mRelativeRect.mExtent[1] - 4;
	r.mCenter[0] = mRelativeRect.mExtent[0] - (width / 2) - 1;
	r.mCenter[1] = mRelativeRect.mExtent[1] / 2;

	mListButton = mUI->AddButton(r, shared_from_this(), -1, L"");
	if (skin && skin->GetSpriteBank())
	{
		mListButton->SetSpriteBank(skin->GetSpriteBank());
		mListButton->SetSprite(BS_BUTTON_UP, DI_CURSOR_DOWN, skin->GetColor(DC_WINDOW_SYMBOL));
		mListButton->SetSprite(BS_BUTTON_DOWN, DI_CURSOR_DOWN, skin->GetColor(DC_WINDOW_SYMBOL));
	}
	mListButton->SetAlignment(UIA_LOWERRIGHT, UIA_LOWERRIGHT, UIA_UPPERLEFT, UIA_LOWERRIGHT);
	mListButton->SetSubElement(true);
	mListButton->SetTabStop(false);

	r.mCenter[0] = (mRelativeRect.mExtent[0] - mListButton->GetAbsolutePosition().mExtent[0]) / 2;
	r.mExtent[0] = mRelativeRect.mExtent[0] - mListButton->GetAbsolutePosition().mExtent[0] - 4;
	r.mCenter[1] = (mRelativeRect.mExtent[1] / 2);
	r.mExtent[1] = mRelativeRect.mExtent[1] - 4;

	mSelectedText = mUI->AddStaticText(L"", r, false, false, shared_from_this(), -1, false);
	mSelectedText->SetSubElement(true);
	mSelectedText->SetAlignment(UIA_UPPERLEFT, UIA_LOWERRIGHT, UIA_UPPERLEFT, UIA_LOWERRIGHT);
	mSelectedText->SetTextAlignment(UIA_UPPERLEFT, UIA_CENTER);
	if (skin)
		mSelectedText->SetOverrideColor(skin->GetColor(DC_BUTTON_TEXT));
	mSelectedText->EnableOverrideColor(true);

	// this element can be tabbed to
	SetTabStop(true);
	SetTabOrder(-1);
}


void UIComboBox::SetTextAlignment(UIAlignment horizontal, UIAlignment vertical)
{
	mHAlign = horizontal;
	mVAlign = vertical;
	mSelectedText->SetTextAlignment(horizontal, vertical);
}


//! Set the maximal number of rows for the selection listbox
void UIComboBox::SetMaxSelectionRows(unsigned int max)
{
	mMaxSelectionRows = max;

	// force recalculation of open listbox
	if (mListBox && mListBox->IsEnabled())
	{
		OpenCloseMenu();
		OpenCloseMenu();
	}
}

//! Get the maximimal number of rows for the selection listbox
unsigned int UIComboBox::GetMaxSelectionRows() const
{
	return mMaxSelectionRows;
}


//! Returns amount of items in box
unsigned int UIComboBox::GetItemCount() const
{
	return mItems.size();
}


//! returns string of an item. the idx may be a value from 0 to itemCount-1
const wchar_t* UIComboBox::GetItem(unsigned int idx) const
{
	if (idx >= mItems.size())
		return nullptr;

	return mItems[idx].mName.c_str();
}

//! returns string of an item. the idx may be a value from 0 to itemCount-1
unsigned int UIComboBox::GetItemData(unsigned int idx) const
{
	if (idx >= mItems.size())
		return 0;

	return mItems[idx].mData;
}

//! Returns index based on item data
int UIComboBox::GetIndexForItemData(unsigned int data ) const
{
	for (unsigned int i = 0; i < mItems.size (); ++i )
	{
		if ( mItems[i].mData == data )
			return i;
	}
	return -1;
}


//! Removes an item from the combo box.
void UIComboBox::RemoveItem(unsigned int idx)
{
	if (idx >= mItems.size())
		return;

	if (mSelected == (int)idx)
		SetSelected(-1);

	mItems.erase(mItems.begin() + idx);
}


//! Returns caption of this element.
const wchar_t* UIComboBox::GetText() const
{
	return GetItem(mSelected);
}


//! adds an item and returns the index of it
unsigned int UIComboBox::AddItem(const wchar_t* text, unsigned int data)
{
	mItems.push_back( ComboData ( text, data ) );

	if (mSelected == -1)
		SetSelected(0);

	return mItems.size() - 1;
}


//! deletes all items in the combo box
void UIComboBox::Clear()
{
	mItems.clear();
	SetSelected(-1);
}


//! returns id of selected item. returns -1 if no item is selected.
int UIComboBox::GetSelected() const
{
	return mSelected;
}


//! sets the selected item. Set this to -1 if no item should be selected
void UIComboBox::SetSelected(int idx)
{
	if (idx < -1 || idx >= (int)mItems.size())
		return;

	mSelected = idx;
	if (mSelected == -1)
		mSelectedText->SetText(L"");
	else
		mSelectedText->SetText(mItems[mSelected].mName.c_str());
}


//! called if an event happened.
bool UIComboBox::OnEvent(const Event& event)
{
	if (IsEnabled())
	{
		switch(event.mEventType)
		{
			case ET_KEY_INPUT_EVENT:
				if (mListBox && mListBox->IsEnabled() && 
					event.mKeyInput.mPressedDown && event.mKeyInput.mKey == KEY_ESCAPE)
				{
					// hide list box
					OpenCloseMenu();
					return true;
				}
				else if (event.mKeyInput.mKey == KEY_RETURN || event.mKeyInput.mKey == KEY_SPACE)
				{
					if (!event.mKeyInput.mPressedDown)
					{
						OpenCloseMenu();
					}

					mListButton->SetPressed(mListBox->IsEnabled() == false);

					return true;
				}
				else if (event.mKeyInput.mPressedDown)
				{
					int oldSelected = mSelected;
					bool absorb = true;
					switch (event.mKeyInput.mKey)
					{
						case KEY_DOWN:
							SetSelected(mSelected+1);
							break;
						case KEY_UP:
							SetSelected(mSelected-1);
							break;
						case KEY_HOME:
						case KEY_PRIOR:
							SetSelected(0);
							break;
						case KEY_END:
						case KEY_NEXT:
							SetSelected(mItems.size()-1);
							break;
						default:
							absorb = false;
					}

					if (mSelected < 0)
						SetSelected(0);

					if (mSelected >= (int)mItems.size())
						SetSelected(mItems.size() -1);

					if (mSelected != oldSelected)
					{
						SendSelectionChangedEvent();
						return true;
					}

					if (absorb)
						return true;
				}
				break;

			case ET_UI_EVENT:

				switch(event.mUIEvent.mEventType)
				{
					case UIEVT_ELEMENT_FOCUS_LOST:
						if (mListBox  && mListBox->IsEnabled() &&
							(mUI->HasFocus(mListBox) || mListBox->IsMyChild(event.mUIEvent.mCaller) ) &&
							event.mUIEvent.mElement != this &&
							!IsMyChild(event.mUIEvent.mElement) &&
							!mListBox->IsMyChild(event.mUIEvent.mElement))
						{
							OpenCloseMenu();
						}
						break;
					case UIEVT_BUTTON_CLICKED:
						if (event.mUIEvent.mCaller == mListButton.get())
						{
							OpenCloseMenu();
							return true;
						}
						break;
					case UIEVT_LISTBOX_SELECTED_AGAIN:
					case UIEVT_LISTBOX_CHANGED:
						if (event.mUIEvent.mCaller == mListBox.get())
						{
							SetSelected(mListBox->GetSelected());
							if (mSelected <0 || mSelected >= (int)mItems.size())
								SetSelected(-1);
							OpenCloseMenu();

							SendSelectionChangedEvent();
						}
						return true;
					default:
						break;
				}
				break;
			case ET_MOUSE_INPUT_EVENT:

				switch(event.mMouseInput.mEvent)
				{
					case MIE_LMOUSE_PRESSED_DOWN:
						{
							Vector2<int> p{ event.mMouseInput.X, event.mMouseInput.Y };

							// send to list box
							if (mListBox && mListBox->IsEnabled() && 
								mListBox->IsPointInside(p) && mListBox->OnEvent(event))
								return true;

							return true;
						}
					case MIE_LMOUSE_LEFT_UP:
						{
							Vector2<int> p{ event.mMouseInput.X, event.mMouseInput.Y };

							// send to list box
							if (!(mListBox && mListBox->IsEnabled() &&
								mListBox->GetAbsolutePosition().IsPointInside(p) &&
								mListBox->OnEvent(event)))
							{
								OpenCloseMenu();
							}
							return true;
						}
					case MIE_MOUSE_WHEEL:
						{
							int oldSelected = mSelected;
							SetSelected( mSelected + ((event.mMouseInput.mWheel < 0) ? 1 : -1));

							if (mSelected < 0 ) 
								SetSelected(0);

							if (mSelected >= (int)mItems.size())
								SetSelected(mItems.size() -1);

							if (mSelected != oldSelected)
							{
								SendSelectionChangedEvent();
								return true;
							}
						}
					default:
						break;
				}
				break;
			default:
				break;
		}
	}

	return BaseUIElement::OnEvent(event);
}


void UIComboBox::SendSelectionChangedEvent()
{
	if (mParent)
	{
		Event event;

		event.mEventType = ET_UI_EVENT;
		event.mUIEvent.mCaller = this;
		event.mUIEvent.mElement = 0;
		event.mUIEvent.mEventType = UIEVT_COMBO_BOX_CHANGED;
		mParent->OnEvent(event);
	}
}


//! draws the element and its children
void UIComboBox::Draw()
{
	if (!IsVisible())
		return;

	eastl::shared_ptr<BaseUISkin> skin = mUI->GetSkin();
	eastl::shared_ptr<BaseUIElement> currentFocus = mUI->GetFocus();
	if (currentFocus != mLastFocus)
	{
		mHasFocus = currentFocus == shared_from_this() || IsMyChild(currentFocus.get());
		mLastFocus = currentFocus;
	}

	// set colors each time as skin-colors can be changed
	mSelectedText->SetBackgroundColor(skin->GetColor(DC_HIGH_LIGHT));
	if(IsEnabled())
	{
		mSelectedText->SetDrawBackground(mHasFocus);
		mSelectedText->SetOverrideColor(skin->GetColor(mHasFocus ? DC_HIGH_LIGHT_TEXT : DC_BUTTON_TEXT));
	}
	else
	{
		mSelectedText->SetDrawBackground(false);
		mSelectedText->SetOverrideColor(skin->GetColor(DC_GRAY_TEXT));
	}
	mListButton->SetSprite(BS_BUTTON_UP, DI_CURSOR_DOWN, skin->GetColor(IsEnabled() ? DC_WINDOW_SYMBOL : DC_GRAY_WINDOW_SYMBOL));
	mListButton->SetSprite(BS_BUTTON_DOWN, DI_CURSOR_DOWN, skin->GetColor(IsEnabled() ? DC_WINDOW_SYMBOL : DC_GRAY_WINDOW_SYMBOL));

	skin->Draw2DTexture(shared_from_this(), mVisual, mAbsoluteRect, mAbsoluteClippingRect.mExtent / 2);

	// draw children
	BaseUIElement::Draw();
}


void UIComboBox::OpenCloseMenu()
{
	if (mListBox && mListBox->IsEnabled())
	{
		// close list box
		mUI->SetFocus(shared_from_this());
		mListBox->SetEnabled(false);
		mListBox->SetVisible(false);
	}
	else
	{
		if (mParent)
			mParent->BringToFront(shared_from_this());

		eastl::shared_ptr<BaseUISkin> skin = mUI->GetSkin();
		unsigned int height = mItems.size();

		if (height > GetMaxSelectionRows())
			height = GetMaxSelectionRows();
		if (height == 0)
			height = 1;

		eastl::shared_ptr<BaseUIFont> font = skin->GetFont();
		if (font)
			height *= (font->GetDimension(L"A")[1] + 4);

		// open list box
		RectangleShape<2, int> r;
		r.mExtent[0] = mAbsoluteRect.mExtent[0];
		r.mExtent[1] = height;
		r.mCenter[0] = mAbsoluteRect.mExtent[0] / 2;
		r.mCenter[1] = mAbsoluteRect.mExtent[1] + (int)round(height / 2.f);
		mListBox->SetRelativePosition(r);
		mListBox->SetEnabled(true);
		mListBox->SetVisible(true);
		mListBox->Clear();

		// ensure that list box is always completely visible
		if (mListBox->GetAbsolutePosition().mCenter[1] + 
			(int)round(mListBox->GetAbsolutePosition().mExtent[1] / 2.f) >
			mUI->GetRootUIElement()->GetAbsolutePosition().mExtent[1])
		{
			RectangleShape<2, int> rect;
			rect.mExtent[0] = mAbsoluteRect.mExtent[0];
			rect.mExtent[1] = mAbsoluteRect.mExtent[1];
			rect.mCenter[0] = mAbsoluteRect.mExtent[0] / 2;
			rect.mCenter[1] = -mAbsoluteRect.mExtent[1] / 2;

			mListBox->SetRelativePosition(rect);
		}

		for (int idx = 0; idx < (int)mItems.size(); ++idx)
			mListBox->AddItem(mItems[idx].mName.c_str());
		mListBox->SetSelected(mSelected);

		// set focus
		mUI->SetFocus(mListBox);
	}
}