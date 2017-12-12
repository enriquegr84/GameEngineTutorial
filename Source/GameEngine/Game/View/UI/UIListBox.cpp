// Copyright (C) 2002-2012 Nikolaus Gebhardt
// This file is part of the "Irrlicht Engine".
// For conditions of distribution and use, see copyright notice in irrlicht.h

#include "UIListBox.h"
#include "UISkin.h"
#include "UIFont.h"
#include "UIScrollBar.h"

#include "UserInterface.h"

#include "Core/OS/OS.h"

#include "Graphic/Renderer/Renderer.h"
#include "Graphic/Image/ImageResource.h"

//! constructor
UIListBox::UIListBox(BaseUI* ui, int id, RectangleShape<2, int> rectangle, bool clip, bool drawBack, bool moveOverSelect)
: BaseUIListBox(id, rectangle), mUI(ui), mSelected(-1), mItemHeight(0), mItemHeightOverride(0),
	mTotalItemHeight(0), mItemsIconWidth(0), mFont(0), mIconBank(0), mScrollBar(0), mSelectedText(0), mSelectTime(0),
	mLastKeyTime(0), mSelecting(false), mDrawBack(drawBack), mMoveOverSelect(moveOverSelect), mAutoScroll(true), 
	mHighlightWhenNotFocused(true)
{
	#ifdef _DEBUG
	//setDebugName("UIListBox");
	#endif

	// Create a vertex buffer for a single triangle.
	VertexFormat vformat;
	vformat.Bind(VA_POSITION, DF_R32G32B32_FLOAT, 0);
	vformat.Bind(VA_COLOR, DF_R32G32B32A32_FLOAT, 0);

	eastl::string path = FileSystem::Get()->GetPath("Effects/ColorEffect.hlsl");
	mEffect = eastl::make_shared<ColorEffect>(ProgramFactory::Get(), path);

	eastl::shared_ptr<VertexBuffer> vbuffer = eastl::make_shared<VertexBuffer>(vformat, 4);
	eastl::shared_ptr<IndexBuffer> ibuffer = eastl::make_shared<IndexBuffer>(IP_TRISTRIP, 2);
	vbuffer->SetUsage(Resource::DYNAMIC_UPDATE);

	// Create the geometric object for drawing.
	mVisual = eastl::make_shared<Visual>(vbuffer, ibuffer, mEffect);
}


//! destructor
UIListBox::~UIListBox()
{

}

//! initialize listbox
void UIListBox::OnInit(bool clip)
{
	if (mUI && mUI->GetSkin())
	{
		mSelectedText.reset(new UIStaticText(mUI, -1, L"", false, mRelativeRect, false));
		mSelectedText->SetParent(shared_from_this());
		mSelectedText->SetSubElement(true);
		mSelectedText->SetAlignment(UIA_UPPERLEFT, UIA_LOWERRIGHT, UIA_UPPERLEFT, UIA_LOWERRIGHT);
		mSelectedText->SetTextAlignment(UIA_UPPERLEFT, UIA_CENTER);
		mSelectedText->EnableOverrideColor(true);

		mScrollBar.reset(new UIScrollBar(mUI, -1, mRelativeRect, false));
		mScrollBar->SetParent(shared_from_this());
		mScrollBar->OnInit(!clip);
		mScrollBar->SetSubElement(true);
		mScrollBar->SetTabStop(false);
		mScrollBar->SetAlignment(UIA_LOWERRIGHT, UIA_LOWERRIGHT, UIA_UPPERLEFT, UIA_LOWERRIGHT);
		mScrollBar->SetVisible(false);
		mScrollBar->SetPos(0);

		SetNotClipped(!clip);

		// this element can be tabbed to
		SetTabStop(true);
		SetTabOrder(-1);

		UpdateAbsolutePosition();
	}
}


//! returns amount of list items
unsigned int UIListBox::GetItemCount() const
{
	return mItems.size();
}


//! returns string of a list item. the may be a value from 0 to itemCount-1
const wchar_t* UIListBox::GetListItem(unsigned int id) const
{
	if (id>=mItems.size())
		return 0;

	return mItems[id].mText.c_str();
}


//! Returns the icon of an item
int UIListBox::GetIcon(unsigned int id) const
{
	if (id>=mItems.size())
		return -1;

	return mItems[id].mIcon;
}


//! adds a list item, returns id of item
unsigned int UIListBox::AddItem(const wchar_t* text)
{
	return AddItem(text, -1);
}


//! adds a list item, returns id of item
void UIListBox::RemoveItem(unsigned int idx)
{
	if (idx >= mItems.size())
		return;

	if ((unsigned int)mSelected==idx)
	{
		mSelected = -1;
	}
	else if ((unsigned int)mSelected > idx)
	{
		mSelected -= 1;
		mSelectTime = Timer::GetTime();
	}

	mItems.erase(mItems.begin() + idx);

	RecalculateItemHeight();
}


int UIListBox::GetItemAt(int xPos, int yPos) const
{
	if (xPos < mAbsoluteRect.center[0] - (mAbsoluteRect.extent[0] / 2)  ||
		yPos < mAbsoluteRect.center[1] - (mAbsoluteRect.extent[1] / 2) ||
		xPos > mAbsoluteRect.center[0] + (int)round(mAbsoluteRect.extent[0] / 2.f) ||
		yPos > mAbsoluteRect.center[1] + (int)round(mAbsoluteRect.extent[1] / 2.f))
		return -1;

	if ( mItemHeight == 0 )
		return -1;

	int item = ((yPos - (mAbsoluteRect.center[1] - (mAbsoluteRect.extent[1] / 2)) - 1) + mScrollBar->GetPos()) / mItemHeight;
	if ( item < 0 || item >= (int)mItems.size())
		return -1;

	return item;
}

//! clears the list
void UIListBox::Clear()
{
	mItems.clear();
	mItemsIconWidth = 0;
	mSelected = -1;

	if (mScrollBar)
		mScrollBar->SetPos(0);

	RecalculateItemHeight();
}


void UIListBox::RecalculateItemHeight()
{
	if (mFont != mUI->GetSkin()->GetFont())
	{
		mFont = mUI->GetSkin()->GetFont();
		if ( 0 == mItemHeightOverride )
			mItemHeight = 0;

		if (mFont)
		{
			if ( 0 == mItemHeightOverride )
				mItemHeight = mFont->GetDimension(L"A")[1] + 4;
		}
	}

	mTotalItemHeight = mItemHeight * mItems.size();
	mScrollBar->SetMax(eastl::max(0, mTotalItemHeight - mAbsoluteRect.extent[1]));
	int minItemHeight = mItemHeight > 0 ? mItemHeight : 1;
	mScrollBar->SetSmallStep ( minItemHeight );
	mScrollBar->SetLargeStep ( 2*minItemHeight );

	if ( mTotalItemHeight <= mAbsoluteRect.extent[1] )
		mScrollBar->SetVisible(false);
	else
		mScrollBar->SetVisible(true);
}


//! returns id of selected item. returns -1 if no item is selected.
int UIListBox::GetSelected() const
{
	return mSelected;
}


//! sets the selected item. Set this to -1 if no item should be selected
void UIListBox::SetSelected(int id)
{
	if ((unsigned int)id>=mItems.size())
		mSelected = -1;
	else
		mSelected = id;

	mSelectTime = Timer::GetTime();

	RecalculateScrollPos();
}

//! sets the selected item. Set this to -1 if no item should be selected
void UIListBox::SetSelected(const wchar_t *item)
{
	int index = -1;

	if ( item )
	{
		for ( index = 0; index < (int)mItems.size(); ++index )
		{
			if ( mItems[index].mText == item )
				break;
		}
	}
	SetSelected ( index );
}

//! called if an event happened.
bool UIListBox::OnEvent(const Event& event)
{
	if (IsEnabled())
	{
		switch(event.mEventType)
		{
			case ET_KEY_INPUT_EVENT:
				if (event.mKeyInput.mPressedDown &&
					(event.mKeyInput.mKey == KEY_DOWN ||
					event.mKeyInput.mKey == KEY_UP   ||
					event.mKeyInput.mKey == KEY_HOME ||
					event.mKeyInput.mKey == KEY_END  ||
					event.mKeyInput.mKey == KEY_NEXT ||
					event.mKeyInput.mKey == KEY_PRIOR ) )
				{
					int oldSelected = mSelected;
					switch (event.mKeyInput.mKey)
					{
						case KEY_DOWN:
							mSelected += 1;
							break;
						case KEY_UP:
							mSelected -= 1;
							break;
						case KEY_HOME:
							mSelected = 0;
							break;
						case KEY_END:
							mSelected = (int)mItems.size()-1;
							break;
						case KEY_NEXT:
							mSelected += mAbsoluteRect.extent[1] / mItemHeight;
							break;
						case KEY_PRIOR:
							mSelected -= mAbsoluteRect.extent[1] / mItemHeight;
							break;
						default:
							break;
					}
					if (mSelected<0)
						mSelected = 0;
					if (mSelected >= (int)mItems.size())
						mSelected = mItems.size() - 1;	// will set Selected to -1 for empty listboxes which is correct
				

					RecalculateScrollPos();

					// post the news

					if (oldSelected != mSelected && mParent && !mSelecting && !mMoveOverSelect)
					{
						Event e;
						e.mEventType = ET_UI_EVENT;
						e.mUIEvent.mCaller = this;
						e.mUIEvent.mElement = 0;
						e.mUIEvent.mEventType = UIEVT_LISTBOX_CHANGED;
						mParent->OnEvent(e);
					}

					return true;
				}
				else
				if (!event.mKeyInput.mPressedDown && 
					( event.mKeyInput.mKey == KEY_RETURN || event.mKeyInput.mKey == KEY_SPACE ) )
				{
					if (mParent)
					{
						Event e;
						e.mEventType = ET_UI_EVENT;
						e.mUIEvent.mCaller = this;
						e.mUIEvent.mElement = 0;
						e.mUIEvent.mEventType = UIEVT_LISTBOX_SELECTED_AGAIN;
						mParent->OnEvent(e);
					}
					return true;
				}
				else if (event.mKeyInput.mPressedDown && event.mKeyInput.mChar)
				{
					// change selection based on text as it is typed.
					unsigned int now = Timer::GetTime();

					if (now - mLastKeyTime < 500)
					{
						// add to key buffer if it isn't a key repeat
						if (!(mKeyBuffer.size() == 1 && mKeyBuffer[0] == event.mKeyInput.mChar))
						{
							mKeyBuffer += L" ";
							mKeyBuffer[mKeyBuffer.size()-1] = event.mKeyInput.mChar;
						}
					}
					else
					{
						mKeyBuffer = L" ";
						mKeyBuffer[0] = event.mKeyInput.mChar;
					}
					mLastKeyTime = now;

					// find the selected item, starting at the current selection
					int start = mSelected;
					// dont change selection if the key buffer matches the current item
					if (mSelected > -1 && mKeyBuffer.size() > 1)
					{
						if (mItems[mSelected].mText.size() >= mKeyBuffer.size() &&
							mKeyBuffer.compare(mItems[mSelected].mText.substr(0,mKeyBuffer.size())))
							return true;
					}

					int current;
					for (current = start+1; current < (int)mItems.size(); ++current)
					{
						if (mItems[current].mText.size() >= mKeyBuffer.size())
						{
							if (mKeyBuffer.compare(mItems[current].mText.substr(0,mKeyBuffer.size())))
							{
								if (mParent && mSelected != current && !mSelecting && !mMoveOverSelect)
								{
									Event e;
									e.mEventType = ET_UI_EVENT;
									e.mUIEvent.mCaller = this;
									e.mUIEvent.mElement = 0;
									e.mUIEvent.mEventType = UIEVT_LISTBOX_CHANGED;
									mParent->OnEvent(e);
								}
								SetSelected(current);
								return true;
							}
						}
					}
					for (current = 0; current <= start; ++current)
					{
						if (mItems[current].mText.size() >= mKeyBuffer.size())
						{
							if (mKeyBuffer.compare(mItems[current].mText.substr(0,mKeyBuffer.size())))
							{
								if (mParent && mSelected != current && !mSelecting && !mMoveOverSelect)
								{
									mSelected = current;
									Event e;
									e.mEventType = ET_UI_EVENT;
									e.mUIEvent.mCaller = this;
									e.mUIEvent.mElement = 0;
									e.mUIEvent.mEventType = UIEVT_LISTBOX_CHANGED;
									mParent->OnEvent(e);
								}
								SetSelected(current);
								return true;
							}
						}
					}

					return true;
				}
				break;

			case ET_UI_EVENT:
				switch(event.mUIEvent.mEventType)
				{
					case UIEVT_SCROLL_BAR_CHANGED:
						if (event.mUIEvent.mCaller == mScrollBar.get())
							return true;
						break;
					case UIEVT_ELEMENT_FOCUS_LOST:
					{
						if (event.mUIEvent.mCaller == this)
							mSelecting = false;
					}
					default:
						break;
				}
				break;

			case ET_MOUSE_INPUT_EVENT:
				{
					Vector2<int> p{ event.mMouseInput.X, event.mMouseInput.Y };

					switch(event.mMouseInput.mEvent)
					{
					case MIE_MOUSE_WHEEL:
						mScrollBar->SetPos(
							mScrollBar->GetPos() + (event.mMouseInput.mWheel < 0 ? -1 : 1)* - mItemHeight/2);
						return true;

					case MIE_LMOUSE_PRESSED_DOWN:
					{
						mSelecting = true;
						return true;
					}

					case MIE_LMOUSE_LEFT_UP:
					{
						mSelecting = false;

						if (IsPointInside(p))
							SelectNew(event.mMouseInput.Y);

						return true;
					}

					case MIE_MOUSE_MOVED:
						if (mSelecting || mMoveOverSelect)
						{
							if (IsPointInside(p))
							{
								SelectNew(event.mMouseInput.Y, true);
								return true;
							}
						}
					default:
						break;
					}
				}
				break;
			case ET_LOG_TEXT_EVENT:
			case ET_USER_EVENT:
			case ET_JOYSTICK_INPUT_EVENT:
			case UIET_FORCE_32_BIT:
				break;
		}
	}

	return BaseUIElement::OnEvent(event);
}


void UIListBox::SelectNew(int ypos, bool onlyHover)
{
	unsigned int now = Timer::GetTime();
	int oldSelected = mSelected;

	mSelected = GetItemAt(mAbsoluteRect.center[0] - mAbsoluteRect.extent[0]/2, ypos);
	if (mSelected<0 && !mItems.empty())
		mSelected = 0;

	RecalculateScrollPos();

	UIEventType eventType = 
		(mSelected == oldSelected && now < mSelectTime + 500) ? 
		UIEVT_LISTBOX_SELECTED_AGAIN : UIEVT_LISTBOX_CHANGED;
	mSelectTime = now;
	// post the news
	if (mParent && !onlyHover)
	{
		Event evt;
		evt.mEventType = ET_UI_EVENT;
		evt.mUIEvent.mCaller = this;
		evt.mUIEvent.mElement = 0;
		evt.mUIEvent.mEventType = eventType;
		mParent->OnEvent(evt);
	}
}


//! Update the position and size of the listbox, and update the scrollbar
void UIListBox::UpdateAbsolutePosition()
{
	BaseUIElement::UpdateAbsolutePosition();

	RecalculateScrollRectangle();
	RecalculateItemHeight();
}


//! draws the element and its children
void UIListBox::Draw()
{
	if (!mVisible)
		return;

	const eastl::shared_ptr<BaseUISkin>& skin = mUI->GetSkin();

	RecalculateItemHeight(); // if the font changed

	RectangleShape<2, int>* clipRect = 0;
	RectangleShape<2, int> frameRect(mAbsoluteRect);
	RectangleShape<2, int> clientClip(mAbsoluteClippingRect);
	/*
	clientClip.extent[0] -= 1;
	clientClip.extent[1] -= 1;

	if (mScrollBar->IsVisible())
		clientClip.extent[0] = mAbsoluteRect.extent[0] - skin->GetSize(DS_SCROLLBAR_SIZE);
	clientClip.extent[1] -= 1;

	//clientClip.clipAgainst(mAbsoluteClippingRect);
	*/
	skin->Draw2DRectangle(
		shared_from_this(), skin->GetColor(DC_3D_HIGH_LIGHT), mVisual, frameRect, &mAbsoluteClippingRect);

	frameRect.center[0] += 1;
	if (mScrollBar->IsVisible())
		frameRect.extent[0] = mAbsoluteRect.extent[0] - skin->GetSize(DS_SCROLLBAR_SIZE);

	frameRect.center[1] -= (mAbsoluteRect.extent[1] / 2);
	frameRect.center[1] += (mItemHeight / 2);
	frameRect.center[1] -= mScrollBar->GetPos();
	frameRect.extent[1] = mItemHeight;

	bool hl = (mHighlightWhenNotFocused || mUI->HasFocus(shared_from_this()) || mUI->HasFocus(mScrollBar));
	
	for (int i=0; i<(int)mItems.size(); ++i)
	{
		if (frameRect.center[1] + (int)round(frameRect.extent[1] / 2.f) <= 
			mAbsoluteRect.center[1] + (int)round(mAbsoluteRect.extent[1] / 2) &&
			frameRect.center[1] + (int)round(frameRect.extent[1] / 2.f) >= 
			mAbsoluteRect.center[1] - (mAbsoluteRect.extent[1] / 2))
		{
			RectangleShape<2, int> textRect = frameRect;
			textRect.extent[0] -= 3;

			if (mFont)
			{
				if (mIconBank && (mItems[i].mIcon > -1))
				{
					RectangleShape<2, int> iconPos = textRect;
					iconPos.center = textRect.center - (textRect.center / 2);
					iconPos.center[1] += textRect.extent[1] / 2;
					iconPos.center[0] += mItemsIconWidth / 2;

					if ( i==mSelected && hl )
					{
						mIconBank->Draw2DSprite( (unsigned int)mItems[i].mIcon, 
							mVisual, iconPos, &clientClip,
							HasItemOverrideColor(i, UI_LBC_ICON_HIGHLIGHT) ?
							GetItemOverrideColor(i, UI_LBC_ICON_HIGHLIGHT) : 
							GetItemDefaultColor(UI_LBC_ICON_HIGHLIGHT),
							mSelectTime, Timer::GetTime(), false, true);
					}
					else
					{
						mIconBank->Draw2DSprite( (unsigned int)mItems[i].mIcon, 
							mVisual, iconPos, &clientClip,
							HasItemOverrideColor(i, UI_LBC_ICON) ? 
							GetItemOverrideColor(i, UI_LBC_ICON) : 
							GetItemDefaultColor(UI_LBC_ICON),
							0 , (i==mSelected) ? Timer::GetTime() : 0, false, true);
					}
				}

				textRect.center[0] += mItemsIconWidth+3;

				if ( i==mSelected )
				{
					mSelectedText->SetBackgroundColor(skin->GetColor(DC_HIGH_LIGHT));
					if (hl)
					{
						mSelectedText->SetDrawBackground(true);
						mSelectedText->SetOverrideColor(
							HasItemOverrideColor(i, UI_LBC_TEXT_HIGHLIGHT) ?
							GetItemOverrideColor(i, UI_LBC_TEXT_HIGHLIGHT) :
							GetItemDefaultColor(UI_LBC_TEXT_HIGHLIGHT));
					}
					else
					{
						mSelectedText->SetDrawBackground(false);
						mSelectedText->SetOverrideColor(
							HasItemOverrideColor(i, UI_LBC_TEXT) ?
							GetItemOverrideColor(i, UI_LBC_TEXT) :
							GetItemDefaultColor(UI_LBC_TEXT));
					}
					mSelectedText->SetText(mItems[i].mText.c_str());

					RectangleShape<2, int> selectedRect(frameRect);
					selectedRect.center -= mAbsoluteRect.center - (mAbsoluteRect.extent / 2);
					mSelectedText->SetRelativePosition(selectedRect);
				}
				else
				{
					mFont->Draw(mItems[i].mText.c_str(), textRect,
						HasItemOverrideColor(i, UI_LBC_TEXT) ? 
						GetItemOverrideColor(i, UI_LBC_TEXT) : 
						GetItemDefaultColor(UI_LBC_TEXT),
						false, true, &clientClip);
				}

				textRect.center[0] -= mItemsIconWidth+3;
			}
		}

		frameRect.center[1] += mItemHeight;
	}

	BaseUIElement::Draw();
}


//! adds an list item with an icon
unsigned int UIListBox::AddItem(const wchar_t* text, int icon)
{
	ListItem i;
	i.mText = text;
	i.mIcon = icon;

	mItems.push_back(i);
	RecalculateItemHeight();
	RecalculateItemWidth(icon);

	return mItems.size() - 1;
}


void UIListBox::SetSpriteBank(const eastl::shared_ptr<BaseUISpriteBank>& bank)
{
	if ( bank == mIconBank )
		return;

	mIconBank = bank;
}


void UIListBox::RecalculateScrollPos()
{
	if (!mAutoScroll)
		return;

	const int selPos = (mSelected == -1 ? mTotalItemHeight : mSelected * mItemHeight) - mScrollBar->GetPos();

	if (selPos < 0)
	{
		mScrollBar->SetPos(mScrollBar->GetPos() + selPos);
	}
	else if (selPos > mAbsoluteRect.extent[1] - mItemHeight)
	{
		mScrollBar->SetPos(mScrollBar->GetPos() + selPos - mAbsoluteRect.extent[1] + mItemHeight);
	}
}


void UIListBox::RecalculateScrollRectangle()
{
	const eastl::shared_ptr<BaseUISkin>& skin = mUI->GetSkin();
	const int s = skin->GetSize(DS_SCROLLBAR_SIZE);

	RectangleShape<2, int> rectangle;
	rectangle.center[0] = mRelativeRect.extent[0] - ( s / 2 );
	rectangle.center[1] = mRelativeRect.extent[1] / 2;
	rectangle.extent[0] = s;
	rectangle.extent[1] = mRelativeRect.extent[1];
	mScrollBar->SetRelativePosition(rectangle);
}


void UIListBox::SetAutoScrollEnabled(bool scroll)
{
	mAutoScroll = scroll;
}


bool UIListBox::IsAutoScrollEnabled() const
{
	return mAutoScroll;
}

void UIListBox::RecalculateItemWidth(int icon)
{
	if (mIconBank && icon > -1 &&
		mIconBank->GetSprites().size() > (unsigned int)icon &&
		mIconBank->GetSprites()[(unsigned int)icon].mFrames.size())
	{
		unsigned int rno = mIconBank->GetSprites()[(unsigned int)icon].mFrames[0].mRectNumber;
		if (mIconBank->GetPositions().size() > rno)
		{
			const int w = mIconBank->GetPositions()[rno].extent[0];
			if (w > mItemsIconWidth)
				mItemsIconWidth = w;
		}
	}
}


void UIListBox::SetItem(unsigned int index, const wchar_t* text, int icon)
{
	if ( index >= mItems.size() )
		return;

	mItems[index].mText = text;
	mItems[index].mIcon = icon;

	RecalculateItemHeight();
	RecalculateItemWidth(icon);
}


//! Insert the item at the given index
//! Return the index on success or -1 on failure.
int UIListBox::InsertItem(unsigned int index, const wchar_t* text, int icon)
{
	ListItem i;
	i.mText = text;
	i.mIcon = icon;

	mItems[index] = i;
	RecalculateItemHeight();
	RecalculateItemWidth(icon);

	return index;
}


void UIListBox::SwapItems(unsigned int index1, unsigned int index2)
{
	if ( index1 >= mItems.size() || index2 >= mItems.size() )
		return;

	ListItem dummmy = mItems[index1];
	mItems[index1] = mItems[index2];
	mItems[index2] = dummmy;
}


void UIListBox::SetItemOverrideColor(unsigned int index, eastl::array<float, 4> color)
{
	for ( unsigned int c=0; c < UI_LBC_COUNT; ++c )
	{
		mItems[index].mOverrideColors[c].mUse = true;
		mItems[index].mOverrideColors[c].mColor = color;
	}
}


void UIListBox::SetItemOverrideColor(unsigned int index, UIListboxColor colorType, eastl::array<float, 4> color)
{
	if ( index >= mItems.size() || colorType < 0 || colorType >= UI_LBC_COUNT )
		return;

	mItems[index].mOverrideColors[colorType].mUse = true;
	mItems[index].mOverrideColors[colorType].mColor = color;
}


void UIListBox::ClearItemOverrideColor(unsigned int index)
{
	for (unsigned int c=0; c < (unsigned int)UI_LBC_COUNT; ++c )
	{
		mItems[index].mOverrideColors[c].mUse = false;
	}
}


void UIListBox::ClearItemOverrideColor(unsigned int index, UIListboxColor colorType)
{
	if ( index >= mItems.size() || colorType < 0 || colorType >= UI_LBC_COUNT )
		return;

	mItems[index].mOverrideColors[colorType].mUse = false;
}


bool UIListBox::HasItemOverrideColor(unsigned int index, UIListboxColor colorType) const
{
	if ( index >= mItems.size() || colorType < 0 || colorType >= UI_LBC_COUNT )
		return false;

	return mItems[index].mOverrideColors[colorType].mUse;
}


eastl::array<float, 4> UIListBox::GetItemOverrideColor(unsigned int index, UIListboxColor colorType) const
{
	if ( (unsigned int)index >= mItems.size() || colorType < 0 || colorType >= UI_LBC_COUNT )
		return eastl::array<float, 4>();

	return mItems[index].mOverrideColors[colorType].mColor;
}


eastl::array<float, 4> UIListBox::GetItemDefaultColor(UIListboxColor colorType) const
{
	const eastl::shared_ptr<BaseUISkin>& skin = mUI->GetSkin();
	if ( !skin )
		return eastl::array<float, 4>();

	switch ( colorType )
	{
		case UI_LBC_TEXT:
			return skin->GetColor(DC_BUTTON_TEXT);
		case UI_LBC_TEXT_HIGHLIGHT:
			return skin->GetColor(DC_HIGH_LIGHT_TEXT);
		case UI_LBC_ICON:
			return skin->GetColor(DC_ICON_NORMAL);
		case UI_LBC_ICON_HIGHLIGHT:
			return skin->GetColor(DC_ICON_HIGH_LIGHT);
		default:
			return eastl::array<float, 4>();
	}
}

//! set global itemHeight
void UIListBox::SetItemHeight(int height)
{
	mItemHeight = height;
	mItemHeightOverride = 1;
}


//! Sets whether to draw the background
void UIListBox::SetDrawBackground(bool draw)
{
    mDrawBack = draw;
}