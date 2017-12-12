// Copyright (C) 2002-2012 Nikolaus Gebhardt
// This file is part of the "Irrlicht Engine".
// For conditions of distribution and use, see copyright notice in irrlicht.h

#include "UIScrollBar.h"

#include "UISkin.h"
#include "UIFont.h"

#include "UserInterface.h"

#include "Core/OS/OS.h"

#include "Graphic/Image/ImageResource.h"
#include "Graphic/Renderer/Renderer.h"


//! constructor
UIScrollBar::UIScrollBar(BaseUI* ui, int id, RectangleShape<2, int> rectangle, bool horizontal)
	: BaseUIScrollBar(id, rectangle), mUI(ui), mUpButton(0), mDownButton(0), mDragging(false), mHorizontal(horizontal),
	mDraggedBySlider(false), mTrayClick(false), mPos(0), mDrawPos(0), mDrawHeight(0), mMin(0), mMax(100), 
	mSmallStep(10), mLargeStep(50), mDesiredPos(0), mLastChange(0)
{
	#ifdef _DEBUG
	//setDebugName("UIScrollBar");
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
	vbuffer->SetUsage(Resource::DYNAMIC_UPDATE);

	eastl::string path = FileSystem::Get()->GetPath("Effects/ColorEffect.hlsl");
	mEffect = eastl::make_shared<ColorEffect>(ProgramFactory::Get(), path);

	// Create the geometric object for drawing.
	mVisual = eastl::make_shared<Visual>(vbuffer, ibuffer, mEffect);

	vbuffer = eastl::make_shared<VertexBuffer>(vformat, 4);
	ibuffer = eastl::make_shared<IndexBuffer>(IP_TRISTRIP, 2);
	vbuffer->SetUsage(Resource::DYNAMIC_UPDATE);

	mEffectSliderRect = eastl::make_shared<ColorEffect>(ProgramFactory::Get(), path);
	mVisualSliderRect = eastl::make_shared<Visual>(vbuffer, ibuffer, mEffectSliderRect);
}


//! destructor
UIScrollBar::~UIScrollBar()
{

}


//! initialize checkbox
void UIScrollBar::OnInit(bool noclip)
{
	RefreshControls();

	SetNotClipped(noclip);

	// this element can be tabbed to
	SetTabStop(true);
	SetTabOrder(-1);

	SetPos(0);
}


//! called if an event happened.
bool UIScrollBar::OnEvent(const Event& event)
{
	if (IsEnabled())
	{

		switch(event.mEventType)
		{
			case ET_KEY_INPUT_EVENT:
				if (event.mKeyInput.mPressedDown)
				{
					const int oldPos = mPos;
					bool absorb = true;
					switch (event.mKeyInput.mKey)
					{
						case KEY_LEFT:
						case KEY_UP:
							SetPos(mPos-mSmallStep);
							break;
						case KEY_RIGHT:
						case KEY_DOWN:
							SetPos(mPos+mSmallStep);
							break;
						case KEY_HOME:
							SetPos(mMin);
							break;
						case KEY_PRIOR:
							SetPos(mPos-mLargeStep);
							break;
						case KEY_END:
							SetPos(mMax);
							break;
						case KEY_NEXT:
							SetPos(mPos+mLargeStep);
							break;
						default:
							absorb = false;
					}

					if (mPos != oldPos)
					{
						Event newEvent;
						newEvent.mEventType = ET_UI_EVENT;
						newEvent.mUIEvent.mCaller = this;
						newEvent.mUIEvent.mElement = 0;
						newEvent.mUIEvent.mEventType = UIEVT_SCROLL_BAR_CHANGED;
						mParent->OnEvent(newEvent);
					}
					if (absorb)
						return true;
				}
				break;
			case ET_UI_EVENT:
				if (event.mUIEvent.mEventType == UIEVT_BUTTON_CLICKED)
				{
					if (event.mUIEvent.mCaller == mUpButton.get())
						SetPos(mPos-mSmallStep);
					else
					if (event.mUIEvent.mCaller == mDownButton.get())
						SetPos(mPos+mSmallStep);

					Event newEvent;
					newEvent.mEventType = ET_UI_EVENT;
					newEvent.mUIEvent.mCaller = this;
					newEvent.mUIEvent.mElement = 0;
					newEvent.mUIEvent.mEventType = UIEVT_SCROLL_BAR_CHANGED;
					mParent->OnEvent(newEvent);

					return true;
				}
				else
				if (event.mUIEvent.mEventType == UIEVT_ELEMENT_FOCUS_LOST)
				{
					if (event.mUIEvent.mCaller == this)
						mDragging = false;
				}
				break;
			case ET_MOUSE_INPUT_EVENT:
				{
					Vector2<int> p{ event.mMouseInput.X, event.mMouseInput.Y };

					bool isInside = IsPointInside ( p );
					switch(event.mMouseInput.mEvent)
					{
						case MIE_MOUSE_WHEEL:
							if (mUI->HasFocus(shared_from_this()))
							{
								// thanks to a bug report by REAPER
								// thanks to tommi by tommi for another bugfix
								// everybody needs a little thanking. hallo niko!;-)
								SetPos(	GetPos() + ( (event.mMouseInput.mWheel < 0 ? -1 : 1) * mSmallStep * (mHorizontal ? 1 : -1 ) ));

								Event newEvent;
								newEvent.mEventType = ET_UI_EVENT;
								newEvent.mUIEvent.mCaller = this;
								newEvent.mUIEvent.mElement = 0;
								newEvent.mUIEvent.mEventType = UIEVT_SCROLL_BAR_CHANGED;
								mParent->OnEvent(newEvent);
								return true;
							}
							break;
						case MIE_LMOUSE_PRESSED_DOWN:
						{
							if (isInside)
							{
								mDragging = true;
								mDraggedBySlider = IsPointInside(mSliderRect, p);
								mTrayClick = !mDraggedBySlider;
								mDesiredPos = GetPosFromMousePos(p);
								mUI->SetFocus ( shared_from_this() );
								return true;
							}
							break;
						}
						case MIE_LMOUSE_LEFT_UP:
						case MIE_MOUSE_MOVED:
						{
							if ( !event.mMouseInput.IsLeftPressed () )
								mDragging = false;

							if ( !mDragging )
							{
								if ( event.mMouseInput.mEvent == MIE_MOUSE_MOVED )
									break;
								return isInside;
							}

							if ( event.mMouseInput.mEvent == MIE_LMOUSE_LEFT_UP )
								mDragging = false;

							const int newPos = GetPosFromMousePos(p);
							const int oldPos = mPos;

							if (!mDraggedBySlider)
							{
								if ( isInside )
								{
									mDraggedBySlider = IsPointInside(mSliderRect, p);
									mTrayClick = !mDraggedBySlider;
								}

								if (mDraggedBySlider)
								{
									SetPos(newPos);
								}
								else
								{
									mTrayClick = false;
									if (event.mMouseInput.mEvent == MIE_MOUSE_MOVED)
										return isInside;
								}
							}

							if (mDraggedBySlider)
							{
								SetPos(newPos);
							}
							else
							{
								mDesiredPos = newPos;
							}

							if (mPos != oldPos && mParent)
							{
								Event newEvent;
								newEvent.mEventType = ET_UI_EVENT;
								newEvent.mUIEvent.mCaller = this;
								newEvent.mUIEvent.mElement = 0;
								newEvent.mUIEvent.mEventType = UIEVT_SCROLL_BAR_CHANGED;
								mParent->OnEvent(newEvent);
							}
							return isInside;
						} break;

						default:
							break;
					}
				} 
				break;

			default:
				break;
		}
	}

	return BaseUIElement::OnEvent(event);
}

void UIScrollBar::OnPostRender(unsigned int timeMs)
{
	if (mDragging && !mDraggedBySlider && mTrayClick && timeMs > mLastChange + 200)
	{
		mLastChange = timeMs;

		const int oldPos = mPos;

		if (mDesiredPos >= mPos + mLargeStep)
			SetPos(mPos + mLargeStep);
		else
		if (mDesiredPos <= mPos - mLargeStep)
			SetPos(mPos - mLargeStep);
		else
		if (mDesiredPos >= mPos - mLargeStep && mDesiredPos <= mPos + mLargeStep)
			SetPos(mDesiredPos);

		if (mPos != oldPos && mParent)
		{
			Event newEvent;
			newEvent.mEventType = ET_UI_EVENT;
			newEvent.mUIEvent.mCaller = this;
			newEvent.mUIEvent.mElement = 0;
			newEvent.mUIEvent.mEventType = UIEVT_SCROLL_BAR_CHANGED;
			mParent->OnEvent(newEvent);
		}
	}

}

//! draws the element and its children
void UIScrollBar::Draw()
{
	if (!IsVisible())
		return;

	const eastl::shared_ptr<BaseUISkin>& skin = mUI->GetSkin();
	if (!skin)
		return;

	eastl::array<float, 4> iconColor = skin->GetColor(IsEnabled() ? DC_WINDOW_SYMBOL : DC_GRAY_WINDOW_SYMBOL);
	if ( iconColor != mCurrentIconColor )
	{
		RefreshControls();
	}

	mSliderRect = mAbsoluteRect;

	// draws the background
	skin->Draw2DRectangle(shared_from_this(), skin->GetColor(DC_SCROLLBAR), mVisual, mSliderRect, &mAbsoluteClippingRect);

	if (Function<float>::IsNotZero(range()))
	{
		// recalculate slider rectangle
		if (mHorizontal)
		{
			mSliderRect.center[0] = (mAbsoluteRect.center[0] - (mAbsoluteRect.extent[0] / 2)) + mDrawPos + mRelativeRect.extent[1];
			mSliderRect.extent[0] = mDrawHeight;
		}
		else
		{
			mSliderRect.center[1] = (mAbsoluteRect.center[1] - (mAbsoluteRect.extent[1] / 2)) + mDrawPos + mRelativeRect.extent[0];
			mSliderRect.extent[1] = mDrawHeight;
		}

		skin->Draw3DButtonPaneStandard(shared_from_this(), mVisualSliderRect, mSliderRect, &mAbsoluteClippingRect);
	}

	// draw buttons
	BaseUIElement::Draw();
}


void UIScrollBar::UpdateAbsolutePosition()
{
	BaseUIElement::UpdateAbsolutePosition();

	// todo: properly resize
	RefreshControls();
	SetPos ( mPos );
}

//!
int UIScrollBar::GetPosFromMousePos(const Vector2<int> &pos) const
{
	float w, p;
	if (mHorizontal)
	{
		w = mRelativeRect.extent[0] - (mRelativeRect.extent[1] * 3.0f);
		p = pos[0] - (mAbsoluteRect.center[0] - (mAbsoluteRect.extent[0] / 2)) - (mRelativeRect.extent[1] * 1.5f);
	}
	else
	{
		w = mRelativeRect.extent[1] - (mRelativeRect.extent[0] * 3.0f);
		p = pos[1] - (mAbsoluteRect.center[1] - (mAbsoluteRect.extent[1] / 2)) - (mRelativeRect.extent[0] * 1.5f);
	}
	return (int) ( p/w * range() ) + mMin;
}


//! sets the position of the scrollbar
void UIScrollBar::SetPos(int pos)
{
	mPos = eastl::clamp ( pos, mMin, mMax );

	if (mHorizontal)
	{
		float f = (mRelativeRect.extent[0] - (mRelativeRect.extent[1] * 3.0f)) / range();
		mDrawPos = (int)(( ( mPos - mMin ) * f) + (mRelativeRect.extent[1] * 0.5f));
		mDrawHeight = mRelativeRect.extent[1];
	}
	else
	{
		float f = (mRelativeRect.extent[1] - (mRelativeRect.extent[0] * 3.0f)) / range();
		mDrawPos = (int)(( ( mPos - mMin ) * f) + (mRelativeRect.extent[0] * 0.5f));
		mDrawHeight = mRelativeRect.extent[0];
	}
}


//! gets the small step value
int UIScrollBar::GetSmallStep() const
{
	return mSmallStep;
}


//! sets the small step value
void UIScrollBar::SetSmallStep(int step)
{
	if (step > 0)
		mSmallStep = step;
	else
		mSmallStep = 10;
}


//! gets the small step value
int UIScrollBar::GetLargeStep() const
{
	return mLargeStep;
}


//! sets the small step value
void UIScrollBar::SetLargeStep(int step)
{
	if (step > 0)
		mLargeStep = step;
	else
		mLargeStep = 50;
}


//! gets the maximum value of the scrollbar.
int UIScrollBar::GetMax() const
{
	return mMax;
}


//! sets the maximum value of the scrollbar.
void UIScrollBar::SetMax(int max)
{
	mMax = max;
	if ( mMin > mMax )
		mMin = mMax;

	bool enable = Function<float>::IsNotZero(range());
	mUpButton->SetEnabled(enable);
	mDownButton->SetEnabled(enable);
	SetPos(mPos);
}

//! gets the minimum value of the scrollbar.
int UIScrollBar::GetMin() const
{
	return mMin;
}


//! sets the minimum value of the scrollbar.
void UIScrollBar::SetMin(int min)
{
	mMin = min;
	if ( mMax < mMin )
		mMax = mMin;

	bool enable = Function<float>::IsNotZero(range());
	mUpButton->SetEnabled(enable);
	mDownButton->SetEnabled(enable);
	SetPos(mPos);
}


//! gets the current position of the scrollbar
int UIScrollBar::GetPos() const
{
	return mPos;
}


//! refreshes the position and text on child buttons
void UIScrollBar::RefreshControls()
{
	mCurrentIconColor = eastl::array<float, 4>{1.f, 1.f, 1.f, 1.f};

	const eastl::shared_ptr<BaseUISkin>& skin = mUI->GetSkin();
	eastl::shared_ptr<BaseUISpriteBank> sprites = 0;

	if (skin)
	{
		sprites = skin->GetSpriteBank();
		mCurrentIconColor = skin->GetColor(IsEnabled() ? DC_WINDOW_SYMBOL : DC_GRAY_WINDOW_SYMBOL);
	}

	if (mHorizontal)
	{
		int h = mRelativeRect.extent[1];
		RectangleShape<2, int> rectangle;
		rectangle.center[0] = h / 2;
		rectangle.center[1] = h / 2;
		rectangle.extent[0] = h;
		rectangle.extent[1] = h;

		if (!mUpButton)
		{
			mUpButton.reset(new UIButton(mUI, -1, rectangle));
			mUpButton->SetParent(shared_from_this());
			mUpButton->OnInit();
			mUpButton->SetSubElement(true);
			mUpButton->SetTabStop(false);
		}
		if (sprites)
		{
			mUpButton->SetSpriteBank(sprites);
			mUpButton->SetSprite(BS_BUTTON_UP, DI_CURSOR_LEFT, mCurrentIconColor);
			mUpButton->SetSprite(BS_BUTTON_DOWN, DI_CURSOR_LEFT, mCurrentIconColor);
		}
		mUpButton->SetRelativePosition(rectangle);
		mUpButton->SetAlignment(UIA_UPPERLEFT, UIA_UPPERLEFT, UIA_UPPERLEFT, UIA_LOWERRIGHT);

		rectangle.center[0] = mRelativeRect.extent[0] - ( h / 2 );
		rectangle.center[1] = h / 2;
		rectangle.extent[0] = h;
		rectangle.extent[1] = h;
		if (!mDownButton)
		{
			mDownButton.reset(new UIButton(mUI, -1, rectangle));
			mDownButton->SetParent(shared_from_this());
			mDownButton->OnInit();
			mDownButton->SetSubElement(true);
			mDownButton->SetTabStop(false);
		}
		if (sprites)
		{
			mDownButton->SetSpriteBank(sprites);
			mDownButton->SetSprite(BS_BUTTON_UP, DI_CURSOR_RIGHT, mCurrentIconColor);
			mDownButton->SetSprite(BS_BUTTON_DOWN, DI_CURSOR_RIGHT, mCurrentIconColor);
		}
		mDownButton->SetRelativePosition(rectangle);
		mDownButton->SetAlignment(UIA_LOWERRIGHT, UIA_LOWERRIGHT, UIA_UPPERLEFT, UIA_LOWERRIGHT);
	}
	else
	{
		int w = mRelativeRect.extent[0];
		RectangleShape<2, int> rectangle;
		rectangle.center[0] = w / 2;
		rectangle.center[1] = w / 2;
		rectangle.extent[0] = w;
		rectangle.extent[1] = w;

		if (!mUpButton)
		{
			mUpButton.reset(new UIButton(mUI, -1, rectangle));
			mUpButton->SetParent(shared_from_this());
			mUpButton->OnInit();
			mUpButton->SetSubElement(true);
			mUpButton->SetTabStop(false);
		}
		if (sprites)
		{
			mUpButton->SetSpriteBank(sprites);
			mUpButton->SetSprite(BS_BUTTON_UP, DI_CURSOR_UP, mCurrentIconColor);
			mUpButton->SetSprite(BS_BUTTON_DOWN, DI_CURSOR_UP, mCurrentIconColor);
		}
		mUpButton->SetRelativePosition(rectangle);
		mUpButton->SetAlignment(UIA_UPPERLEFT, UIA_LOWERRIGHT, UIA_UPPERLEFT, UIA_UPPERLEFT);

		rectangle.center[0] = w / 2;
		rectangle.center[1] = mRelativeRect.extent[1] - ( w / 2 );
		rectangle.extent[0] = w;
		rectangle.extent[1] = w;
		if (!mDownButton)
		{
			mDownButton.reset(new UIButton(mUI, -1, rectangle));
			mDownButton->SetParent(shared_from_this());
			mDownButton->OnInit();
			mDownButton->SetSubElement(true);
			mDownButton->SetTabStop(false);
		}
		if (sprites)
		{
			mDownButton->SetSpriteBank(sprites);
			mDownButton->SetSprite(BS_BUTTON_UP, DI_CURSOR_DOWN, mCurrentIconColor);
			mDownButton->SetSprite(BS_BUTTON_DOWN, DI_CURSOR_DOWN, mCurrentIconColor);
		}
		mDownButton->SetRelativePosition(rectangle);
		mDownButton->SetAlignment(UIA_LOWERRIGHT, UIA_LOWERRIGHT, UIA_UPPERLEFT, UIA_LOWERRIGHT);
	}
}