// Copyright (C) 2002-2012 Nikolaus Gebhardt
// This file is part of the "Irrlicht Engine".
// For conditions of distribution and use, see copyright notice in irrlicht.h

#include "UIButton.h"

#include "Core/OS/OS.h"

#include "Graphic/Renderer/Renderer.h"
#include "Graphic/Image/ImageResource.h"

// Multiply with a color to get the default corresponding hovered color
#define COLOR_HOVERED_MOD 1.25f

// Multiply with a color to get the default corresponding pressed color
#define COLOR_PRESSED_MOD 0.85f

//! constructor
UIButton::UIButton(BaseUI* ui, int id, RectangleShape<2, int> rectangle)
:	BaseUIButton(id, rectangle), mSpriteBank(0), mOverrideFont(0), mUI(ui),
    mClickTime(0), mHoverTime(0), mFocusTime(0), mPushButton(false), 
	mPressed(false), mUseAlphaChannel(false), mDrawBorder(true), mScaleImage(false)
{
	eastl::shared_ptr<ResHandle>& resHandle =
		ResCache::Get()->GetHandle(&BaseResource(L"Art/UserControl/appbar.empty.png"));
	if (resHandle)
	{
		const eastl::shared_ptr<ImageResourceExtraData>& extra =
			eastl::static_pointer_cast<ImageResourceExtraData>(resHandle->GetExtra());
		extra->GetImage()->AutogenerateMipmaps();

		mBlendState = eastl::make_shared<BlendState>();

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


//! destructor
UIButton::~UIButton()
{

}

//! initialize button
void UIButton::OnInit(bool noclip)
{
	SetNotClipped(noclip);

	// Initialize the sprites.
	for (unsigned int i=0; i<BS_COUNT; ++i)
		mButtonSprites[i].Index = -1;
	
	// This element can be tabbed.
	SetTabStop(true);
	SetTabOrder(-1);

    const eastl::shared_ptr<BaseUISkin>& skin = mUI->GetSkin();
    for (size_t i = 0; i < 4; i++)
        mColors[i] = skin->GetColor((UIDefaultColor)i);

    mStaticText = mUI->AddStaticText(mText.c_str(), mAbsoluteRect, false, false, shared_from_this(), mID);
    mStaticText->SetTextAlignment(UIA_CENTER, UIA_CENTER);
}

//! Sets if the images should be scaled to fit the button
void UIButton::SetScaleImage(bool scaleImage)
{
	mScaleImage = scaleImage;
}


//! Returns whether the button scale the used images
bool UIButton::IsScalingImage() const
{
	return mScaleImage;
}

void UIButton::SetColor(SColor color)
{
    eastl::shared_ptr<BaseUISkin> skin = mUI->GetSkin();

    mBGColor = color;

    float d = 0.65f;
    for (size_t i = 0; i < 4; i++) 
    {
        SColor base = skin->GetColor((UIDefaultColor)i);
        mColors[i] = base.GetInterpolated(color, d);
    }
}

//! Sets if the button should use the skin to draw its border
void UIButton::SetDrawBorder(bool border)
{
	mDrawBorder = border;
}


void UIButton::SetSpriteBank(const eastl::shared_ptr<BaseUISpriteBank>& sprites)
{
	mSpriteBank = sprites;
}

void UIButton::SetSprite(UIButtonState state, int index, SColor color, bool loop, bool scale)
{
	mButtonSprites[(unsigned int)state].Index	= index;
	mButtonSprites[(unsigned int)state].Color	= color;
	mButtonSprites[(unsigned int)state].Loop	= loop;
    mButtonSprites[(unsigned int)state].Scale   = scale;
}


//! Get the sprite-index for the given state or -1 when no sprite is set
int UIButton::GetSpriteIndex(UIButtonState state) const
{
    return mButtonSprites[(unsigned int)state].Index;
}


//! Get the sprite color for the given state. Color is only used when a sprite is set.
SColor UIButton::GetSpriteColor(UIButtonState state) const
{
    return mButtonSprites[(unsigned int)state].Color;
}


//! Returns if the sprite in the given state does loop
bool UIButton::GetSpriteLoop(UIButtonState state) const
{
    return mButtonSprites[(unsigned int)state].Loop;
}


//! Returns if the sprite in the given state is scaled
bool UIButton::GetSpriteScale(UIButtonState state) const
{
    return mButtonSprites[(unsigned int)state].Scale;
}


//! called if an event happened.
bool UIButton::OnEvent(const Event& ev)
{
	if (!IsEnabled())
		return BaseUIElement::OnEvent(ev);

	switch(ev.mEventType)
	{
		case ET_KEY_INPUT_EVENT:
			if (ev.mKeyInput.mPressedDown &&
				(ev.mKeyInput.mKey == KEY_RETURN || ev.mKeyInput.mKey == KEY_SPACE))
			{
				if (!mPushButton)
					SetPressed(true);
				else
					SetPressed(!mPressed);

				return true;
			}
			if (mPressed && !mPushButton && 
				ev.mKeyInput.mPressedDown && ev.mKeyInput.mKey == KEY_ESCAPE)
			{
				SetPressed(false);
				return true;
			}
			else
			if (!ev.mKeyInput.mPressedDown && mPressed &&
				(ev.mKeyInput.mKey == KEY_RETURN || ev.mKeyInput.mKey == KEY_SPACE))
			{
				if (!mPushButton)
					SetPressed(false);

				if (mParent)
				{
                    mClickShiftState = ev.mKeyInput.mShift;
                    mClickControlState = ev.mKeyInput.mControl;

					Event newEvent;
					newEvent.mEventType = ET_UI_EVENT;
					newEvent.mUIEvent.mCaller = this;
					newEvent.mUIEvent.mElement = 0;
					newEvent.mUIEvent.mEventType = UIEVT_BUTTON_CLICKED;
					mParent->OnEvent(newEvent);
				}
				return true;
			}
			break;
		case ET_UI_EVENT:
			if (ev.mUIEvent.mCaller == this)
			{
				if (ev.mUIEvent.mEventType == UIEVT_ELEMENT_FOCUS_LOST)
				{
					if (!mPushButton)
						SetPressed(false);
					mFocusTime = Timer::GetTime();
				}
				else if (ev.mUIEvent.mEventType == UIEVT_ELEMENT_FOCUSED)
				{
					mFocusTime = Timer::GetTime();
				}
				else if (ev.mUIEvent.mEventType == UIEVT_ELEMENT_HOVERED || 
						ev.mUIEvent.mEventType == UIEVT_ELEMENT_LEFT)
				{
					mHoverTime = Timer::GetTime();
				}
			}
			break;
		case ET_MOUSE_INPUT_EVENT:
			if (ev.mMouseInput.mEvent == MIE_LMOUSE_PRESSED_DOWN)
			{
				if (mUI->HasFocus(shared_from_this()) &&
					!mAbsoluteRect.IsPointInside(Vector2<int>{ev.mMouseInput.X, ev.mMouseInput.Y}))
				{
					mUI->RemoveFocus(shared_from_this());
					return false;
				}

				if (!mPushButton)
					SetPressed(true);

				mUI->SetFocus(shared_from_this());
				return true;
			}
			else
			if (ev.mMouseInput.mEvent == MIE_LMOUSE_LEFT_UP)
			{
				bool wasPressed = mPressed;

				if (!mAbsoluteRect.IsPointInside(Vector2<int>{ev.mMouseInput.X, ev.mMouseInput.Y }))
				{
					if (!mPushButton)
						SetPressed(false);
					return true;
				}

				if (!mPushButton)
					SetPressed(false);
				else
					SetPressed(!mPressed);

				if ((!mPushButton && wasPressed && mParent) ||
					(mPushButton && wasPressed != mPressed))
				{
                    mClickShiftState = ev.mMouseInput.mShift;
                    mClickControlState = ev.mMouseInput.mControl;

					Event newEvent;
					newEvent.mEventType = ET_UI_EVENT;
					newEvent.mUIEvent.mCaller = this;
					newEvent.mUIEvent.mElement = 0;
					newEvent.mUIEvent.mEventType = UIEVT_BUTTON_CLICKED;
					mParent->OnEvent(newEvent);
				}

				return true;
			}
			break;
		default:
			break;
	}

	return mParent ? mParent->OnEvent(ev) : false;
}


//! draws the element and its children
void UIButton::Draw( )
{
	if (!mVisible)
		return;

	const eastl::shared_ptr<BaseUISkin>& skin = mUI->GetSkin();
	auto effect = eastl::dynamic_pointer_cast<Texture2Effect>(mEffect);

	Renderer::Get()->SetBlendState(mBlendState);

    if (mDrawBorder)
    {
        if (!mPressed)
        {
            skin->Draw3DButtonPaneStandard(shared_from_this(), 
                mVisual, mAbsoluteRect, &mAbsoluteClippingRect, mColors);
        }
        else
        {
            skin->Draw3DButtonPanePressed(shared_from_this(),
                mVisual, mAbsoluteRect, &mAbsoluteClippingRect, mColors);
        }
    }

    // The image changes based on the state, so we use the default every time.
    UIButtonImageState imageState = BIS_IMAGE_UP;
    if (mButtonImages[(unsigned int)imageState].Texture)
    {
        Vector2<int> pos = mAbsoluteRect.mCenter;
        RectangleShape<2, int> sourceRect(mButtonImages[(unsigned int)imageState].SourceRect);
        if (sourceRect.mCenter == Vector2<int>::Zero())
        {
            sourceRect = RectangleShape<2, int>();
            sourceRect.mExtent[0] = mButtonImages[(unsigned int)imageState].Texture->GetWidth();
            sourceRect.mExtent[1] = mButtonImages[(unsigned int)imageState].Texture->GetHeight();
        }
        
        pos[0] -= sourceRect.mExtent[0] / 2;
        pos[1] -= sourceRect.mExtent[1] / 2;

        if (mPressed)
        {
            // Create a pressed-down effect by moving the image when it looks identical to the unpressed state image
            UIButtonImageState unpressedState = GetImageState(false);
            if (unpressedState == imageState || 
                mButtonImages[(unsigned int)imageState] == mButtonImages[(unsigned int)unpressedState])
            {
                pos[0] += skin->GetSize(DS_BUTTON_PRESSED_IMAGE_OFFSET_X);
                pos[1] += skin->GetSize(DS_BUTTON_PRESSED_IMAGE_OFFSET_Y);
            }
        }


        effect->SetTexture(mButtonImages[(unsigned int)imageState].Texture);

        SColor imageColors[] = { mBGColor, mBGColor, mBGColor, mBGColor };
        if (mBGMiddle.GetArea() == 0) 
        {
            skin->Draw2DTexture(shared_from_this(), mVisual,
                mScaleImage ? mAbsoluteRect : RectangleShape<2, int>(pos, sourceRect.mAxis, sourceRect.mExtent),
                sourceRect, mAbsoluteClippingRect.mExtent, imageColors);
        }
        else 
        {
            core::rect<s32> middle = BgMiddle;
            // `-x` is interpreted as `w - x`
            if (middle.LowerRightCorner.X < 0)
                middle.LowerRightCorner.X += texture->getOriginalSize().Width;
            if (middle.LowerRightCorner.Y < 0)
                middle.LowerRightCorner.Y += texture->getOriginalSize().Height;
            draw2DImage9Slice(driver, texture,
                ScaleImage ? AbsoluteRect : core::rect<s32>(pos, sourceRect.getSize()),
                middle, &AbsoluteClippingRect, image_colors);
        }
    }

	// todo: move sprite up and text down if the pressed state has a sprite
	RectangleShape<2, int> spritePos = mAbsoluteRect;

	if (!mPressed)
	{
		if (mDrawBorder)
			skin->Draw2DTexture(shared_from_this(), mVisual, spritePos, mAbsoluteClippingRect.mExtent / 2);

		if (mImage)
		{
			effect->SetTexture(mImage);

			if (mScaleImage)
				skin->Draw2DTexture(shared_from_this(), mVisual, spritePos, mImageRect.mExtent);
			else 
				skin->Draw2DTexture(shared_from_this(), mVisual, mAbsoluteRect, mAbsoluteClippingRect.mExtent / 2);
		}
	}
	else
	{
		if (mPressedImage)
		{
			effect->SetTexture(mPressedImage);

			if (mScaleImage)
			{
				spritePos.mCenter[0] -= mPressedImageRect.mExtent[0] / 2;
				spritePos.mCenter[1] -= mPressedImageRect.mExtent[1] / 2;

				if (mImage == mPressedImage && mPressedImageRect == mImageRect)
				{
					spritePos.mCenter[0] += skin->GetSize(DS_BUTTON_PRESSED_IMAGE_OFFSET_X);
					spritePos.mCenter[1] += skin->GetSize(DS_BUTTON_PRESSED_IMAGE_OFFSET_Y);
				}

				skin->Draw2DTexture(shared_from_this(), mVisual, spritePos, mImageRect.mExtent);
			}
			else
				skin->Draw2DTexture(shared_from_this(), mVisual, mAbsoluteRect, mAbsoluteClippingRect.mExtent / 2);

		}
	}

	if (mSpriteBank)
	{
		// pressed / unpressed animation
		unsigned int state = mPressed ? (unsigned int)BS_BUTTON_DOWN : (unsigned int)BS_BUTTON_UP;
		if (mButtonSprites[state].Index != -1)
		{
			mSpriteBank->Draw2DSprite(mButtonSprites[state].Index, mVisual, spritePos,
			 	&mAbsoluteClippingRect, mButtonSprites[state].Color, mClickTime, Timer::GetTime(),
				mButtonSprites[state].Loop, true);
		}

		// Focused / unFocused animation
		state = mUI->HasFocus(shared_from_this()) ? 
			(unsigned int)BS_BUTTON_FOCUSED : (unsigned int)BS_BUTTON_NOT_FOCUSED;
		if (mButtonSprites[state].Index != -1)
		{
			mSpriteBank->Draw2DSprite(mButtonSprites[state].Index, mVisual, spritePos,
			 	&mAbsoluteClippingRect, mButtonSprites[state].Color, mFocusTime, Timer::GetTime(),
				mButtonSprites[state].Loop, true);
		}

		// mouse over / off animation
		if (IsEnabled())
		{
			state = mUI->GetHovered() == shared_from_this() ? 
				(unsigned int)BS_BUTTON_MOUSE_OVER : (unsigned int)BS_BUTTON_MOUSE_OFF;
			if (mButtonSprites[state].Index != -1)
			{
				mSpriteBank->Draw2DSprite(mButtonSprites[state].Index, mVisual, spritePos,
				 	&mAbsoluteClippingRect, mButtonSprites[state].Color, mHoverTime, Timer::GetTime(),
					mButtonSprites[state].Loop, true);
			}
		}
	}

	Renderer::Get()->SetDefaultBlendState();

	if (mText.size())
	{
		const eastl::shared_ptr<BaseUIFont>& font = GetActiveFont();

		RectangleShape<2, int> rect = mAbsoluteRect;
		rect.mExtent[0] = font->GetDimension(mText.c_str())[0];
		if (mPressed)
		{
			rect.mCenter[0] += skin->GetSize(DS_BUTTON_PRESSED_TEXT_OFFSET_X);
			rect.mExtent[0] -= skin->GetSize(DS_BUTTON_PRESSED_TEXT_OFFSET_X);
			rect.mCenter[1] += skin->GetSize(DS_BUTTON_PRESSED_TEXT_OFFSET_Y);
			rect.mExtent[1] -= skin->GetSize(DS_BUTTON_PRESSED_TEXT_OFFSET_Y);
		}

		if (font)
			font->Draw(mText.c_str(), rect,
				skin->GetColor(IsEnabled() ? DC_GRAY_TEXT : DC_BUTTON_TEXT),
				false, true, &mAbsoluteClippingRect);
	}

	BaseUIElement::Draw();
}

void UIButton::DrawSprite(UIButtonState state, unsigned int startTime, const RectangleShape<2, int>& center)
{
    unsigned int stateIdx = (unsigned int)state;

    if (mButtonSprites[stateIdx].Index != -1)
    {
        if (mButtonSprites[stateIdx].Scale)
        {
            mSpriteBank->Draw2DSprite(mButtonSprites[stateIdx].Index, mVisual, 
                mAbsoluteRect, &mAbsoluteClippingRect, mButtonSprites[stateIdx].Color,
                Timer::GetTime() - startTime, mButtonSprites[stateIdx].Loop, true);
        }
        else
        {
            mSpriteBank->Draw2DSprite(mButtonSprites[stateIdx].Index, mVisual, center,
                &mAbsoluteClippingRect, mButtonSprites[stateIdx].Color, startTime, Timer::GetTime(),
                mButtonSprites[stateIdx].Loop, true);
        }
    }
}

UIButtonImageState UIButton::GetImageState(bool pressed) const
{
    return GetImageState(pressed, mButtonImages);
}

UIButtonImageState UIButton::GetImageState(bool pressed, const ButtonImage* images) const
{
    // figure state we should have
    UIButtonImageState state = BIS_IMAGE_DISABLED;
    bool focused = mUI->HasFocus(shared_from_this());
    bool mouseOver = IsHovered();
    if (IsEnabled())
    {
        if (pressed)
        {
            if (focused && mouseOver)
                state = BIS_IMAGE_DOWN_FOCUSED_MOUSEOVER;
            else if (focused)
                state = BIS_IMAGE_DOWN_FOCUSED;
            else if (mouseOver)
                state = BIS_IMAGE_DOWN_MOUSEOVER;
            else
                state = BIS_IMAGE_DOWN;
        }
        else // !pressed
        {
            if (focused && mouseOver)
                state = BIS_IMAGE_UP_FOCUSED_MOUSEOVER;
            else if (focused)
                state = BIS_IMAGE_UP_FOCUSED;
            else if (mouseOver)
                state = BIS_IMAGE_UP_MOUSEOVER;
            else
                state = BIS_IMAGE_UP;
        }
    }

    // find a compatible state that has images
    while (state != BIS_IMAGE_UP && !images[(unsigned int)state].Texture)
    {
        switch (state)
        {
        case BIS_IMAGE_UP_FOCUSED:
            state = BIS_IMAGE_UP;
            break;
        case BIS_IMAGE_UP_FOCUSED_MOUSEOVER:
            state = BIS_IMAGE_UP_FOCUSED;
            break;
        case BIS_IMAGE_DOWN_MOUSEOVER:
            state = BIS_IMAGE_DOWN;
            break;
        case BIS_IMAGE_DOWN_FOCUSED:
            state = BIS_IMAGE_DOWN;
            break;
        case BIS_IMAGE_DOWN_FOCUSED_MOUSEOVER:
            state = BIS_IMAGE_DOWN_FOCUSED;
            break;
        case BIS_IMAGE_DISABLED:
            if (pressed)
                state = BIS_IMAGE_DOWN;
            else
                state = BIS_IMAGE_UP;
            break;
        default:
            state = BIS_IMAGE_UP;
        }
    }

    return state;
}

//! sets another skin independent font. if this is set to zero, the button uses the font of the skin.
void UIButton::SetOverrideFont(const eastl::shared_ptr<BaseUIFont>& font)
{
	if (mOverrideFont == font)
		return;

	mOverrideFont = font ;
}

//! Gets the override font (if any)
const eastl::shared_ptr<BaseUIFont>& UIButton::GetOverrideFont() const
{
	return mOverrideFont;
}

//! Get the font which is used right now for drawing
eastl::shared_ptr<BaseUIFont> UIButton::GetActiveFont() const
{
	if ( mOverrideFont )
		return mOverrideFont;
	if (mUI->GetSkin())
		return mUI->GetSkin()->GetFont();
	return eastl::shared_ptr<BaseUIFont>();
}


//! Sets another color for the text.
void UIButton::SetOverrideColor(SColor color)
{
    mOverrideColor = color;
    mOverrideColorEnabled = true;

    mStaticText->SetOverrideColor(color);
}

SColor UIButton::GetOverrideColor() const
{
    return mOverrideColor;
}

void UIButton::EnableOverrideColor(bool enable)
{
    mOverrideColorEnabled = enable;
}

bool UIButton::IsOverrideColorEnabled() const
{
    return mOverrideColorEnabled;
}


//! Sets an image which should be displayed on the button when it is in normal state.
void UIButton::SetImage(const eastl::shared_ptr<Texture2>& image)
{
	mImage = image;

	if (image)
	{
		mImageRect = RectangleShape<2, int>();
		mImageRect.mCenter[0] = image->GetDimension(0) / 2;
		mImageRect.mCenter[1] = image->GetDimension(1) / 2;
		mImageRect.mExtent[0] = image->GetDimension(0);
		mImageRect.mExtent[1] = image->GetDimension(1);
	}

	if (!mPressedImage)
		SetPressedImage(mImage);
}


//! Sets the image which should be displayed on the button when it is in its normal state.
void UIButton::SetImage(const eastl::shared_ptr<Texture2>& image, const RectangleShape<2, int>& pos)
{
	SetImage(image);
	mImageRect = pos;
}


//! Sets an image which should be displayed on the button when it is in pressed state.
void UIButton::SetPressedImage(const eastl::shared_ptr<Texture2>& image)
{
	mPressedImage = image;

	if (image)
	{
		mPressedImageRect = RectangleShape<2, int>();
		mImageRect.mCenter[0] = image->GetDimension(0) / 2;
		mImageRect.mCenter[1] = image->GetDimension(1) / 2;
		mImageRect.mExtent[0] = image->GetDimension(0);
		mImageRect.mExtent[1] = image->GetDimension(1);
	}
}


//! Sets the image which should be displayed on the button when it is in its pressed state.
void UIButton::SetPressedImage(const eastl::shared_ptr<Texture2>& image, const RectangleShape<2, int>& pos)
{
	SetPressedImage(image);
	mPressedImageRect = pos;
}


//! Sets if the button should behave like a push button. Which means it
//! can be in two states: Normal or Pressed. With a click on the button,
//! the user can change the state of the button.
void UIButton::SetPushButton(bool pushButton)
{
	mPushButton = pushButton;
}


//! Returns if the button is currently pressed
bool UIButton::IsPressed() const
{
	return mPressed;
}

//! Returns if this element (or one of its direct children) is hovered
bool UIButton::IsHovered() const
{
    eastl::shared_ptr<BaseUIElement> hovered = mUI->GetHovered();
    return  hovered == shared_from_this() || 
        (hovered != nullptr && hovered->GetParent() == shared_from_this());
}

//! Sets the pressed state of the button if this is a pushbutton
void UIButton::SetPressed(bool pressed)
{
	if (mPressed != pressed)
	{
		mClickTime = Timer::GetTime();
		mPressed = pressed;
	}
}


//! Returns whether the button is a push button
bool UIButton::IsPushButton() const
{
	return mPushButton;
}


//! Sets if the alpha channel should be used for drawing images on the button (default is false)
void UIButton::SetUseAlphaChannel(bool useAlphaChannel)
{
	mUseAlphaChannel = useAlphaChannel;

	if (mUseAlphaChannel)
	{
		mBlendState->mTarget[0].enable = true;
		mBlendState->mTarget[0].srcColor = BlendState::BM_ONE;
		mBlendState->mTarget[0].dstColor = BlendState::BM_INV_SRC_COLOR;
		mBlendState->mTarget[0].srcAlpha = BlendState::BM_SRC_ALPHA;
		mBlendState->mTarget[0].dstAlpha = BlendState::BM_INV_SRC_ALPHA;
	}
	else mBlendState->mTarget[0] = BlendState::Target();

	Renderer::Get()->Unbind(mBlendState);
}


//! Returns if the alpha channel should be used for drawing images on the button
bool UIButton::IsAlphaChannelUsed() const
{
	return mUseAlphaChannel;
}


bool UIButton::IsDrawingBorder() const
{
	return mDrawBorder;
}