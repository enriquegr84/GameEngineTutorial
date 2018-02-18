// Copyright (C) 2002-2012 Nikolaus Gebhardt
// This file is part of the "Irrlicht Engine".
// For conditions of distribution and use, see copyright notice in irrlicht.h

#ifndef UIBUTTON_H
#define UIBUTTON_H

#include "UIElement.h"
#include "UIFont.h"
#include "UISpriteBank.h"

#include "Graphic/Effect/Texture2Effect.h"
#include "Graphic/Scene/Hierarchy/Visual.h"

enum UIButtonState
{
	//! The button is not pressed
	BS_BUTTON_UP = 0,
	//! The button is currently pressed down
	BS_BUTTON_DOWN,
	//! The mouse cursor is over the button
	BS_BUTTON_MOUSE_OVER,
	//! The mouse cursor is not over the button
	BS_BUTTON_MOUSE_OFF,
	//! The button has the focus
	BS_BUTTON_Focused,
	//! The button doesn't have the focus
	BS_BUTTON_NOT_Focused,
	//! not used, counts the number of enumerated items
	BS_COUNT
};

//! Names for gui button state icons
const char* const UIButtonStateNames[] =
{
	"buttonUp",
	"buttonDown",
	"buttonMouseOver",
	"buttonMouseOff",
	"buttonFocused",
	"buttonNotFocused",
	0,
	0,
};

//! GUI Button interface.
/** \par This element can create the following events of type EGUI_EVENT_TYPE:
\li UIEVT_BUTTON_CLICKED
*/
class BaseUIButton : public BaseUIElement
{
public:

	//! constructor
	BaseUIButton(int id, RectangleShape<2, int> rectangle)
		: BaseUIElement(UIET_BUTTON, id, rectangle) {}

	//! Initialize button
	virtual void OnInit(bool noclip = false) = 0;

	//! Sets another skin independent font.
	/** If this is set to zero, the button uses the font of the skin.
	\param font: New font to set. */
	virtual void SetOverrideFont(const eastl::shared_ptr<BaseUIFont>& font = 0) = 0;

	//! Gets the override font (if any)
	/** \return The override font (may be 0) */
	virtual const eastl::shared_ptr<BaseUIFont>& GetOverrideFont(void) const = 0;

	//! Get the font which is used right now for drawing
	/** Currently this is the override font when one is set and the
	font of the active skin otherwise */
	virtual eastl::shared_ptr<BaseUIFont> GetActiveFont() const = 0;

	//! Sets an image which should be displayed on the button when it is in normal state.
	/** \param image: Image to be displayed */
	virtual void SetImage(const eastl::shared_ptr<Texture2>& image = 0) = 0;

	//! Sets a background image for the button when it is in normal state.
	/** \param image: Texture containing the image to be displayed
	\param pos: Position in the texture, where the image is located */
	virtual void SetImage(const eastl::shared_ptr<Texture2>& image, const RectangleShape<2, int>& pos) = 0;

	//! Sets a background image for the button when it is in pressed state.
	/** If no images is specified for the pressed state via
	setPressedImage(), this image is also drawn in pressed state.
	\param image: Image to be displayed */
	virtual void SetPressedImage(const eastl::shared_ptr<Texture2>& image = 0) = 0;

	//! Sets an image which should be displayed on the button when it is in pressed state.
	/** \param image: Texture containing the image to be displayed
	\param pos: Position in the texture, where the image is located */
	virtual void SetPressedImage(const eastl::shared_ptr<Texture2>& image, const RectangleShape<2, int>& pos) = 0;

	//! Sets the sprite bank used by the button
	virtual void SetSpriteBank(const eastl::shared_ptr<BaseUISpriteBank>& sprites = 0) = 0;

	//! Sets the animated sprite for a specific button state
	/** \param index: Number of the sprite within the sprite bank, use -1 for no sprite
	\param state: State of the button to set the sprite for
	\param index: The sprite number from the current sprite bank
	\param color: The color of the sprite
	\param loop: True if the animation should loop, false if not
	*/
	virtual void SetSprite(UIButtonState state, int index,
		eastl::array<float, 4> color = eastl::array<float, 4>{1.f, 1.f, 1.f, 1.f}, bool loop = false) = 0;

	//! Sets if the button should behave like a push button.
	/** Which means it can be in two states: Normal or Pressed. With a click on the button,
	the user can change the state of the button. */
	virtual void SetPushButton(bool isPushButton = true) = 0;

	//! Sets the pressed state of the button if this is a pushbutton
	virtual void SetPressed(bool pressed = true) = 0;

	//! Returns if the button is currently pressed
	virtual bool IsPressed() const = 0;

	//! Sets if the alpha channel should be used for drawing background images on the button (default is false)
	virtual void SetUseAlphaChannel(bool useAlphaChannel = true) = 0;

	//! Returns if the alpha channel should be used for drawing background images on the button
	virtual bool IsAlphaChannelUsed() const = 0;

	//! Returns whether the button is a push button
	virtual bool IsPushButton() const = 0;

	//! Sets if the button should use the skin to draw its border and button face (default is true)
	virtual void SetDrawBorder(bool border = true) = 0;

	//! Returns if the border and button face are being drawn using the skin
	virtual bool IsDrawingBorder() const = 0;

	//! Sets if the button should scale the button images to fit
	virtual void SetScaleImage(bool scaleImage = true) = 0;

	//! Checks whether the button scales the used images
	virtual bool IsScalingImage() const = 0;
};


class UIButton : public BaseUIButton
{
public:

	//! constructor
	UIButton(BaseUI* ui, int id, RectangleShape<2, int> rectangle);

	//! destructor
	virtual ~UIButton();

	//! initialize button
	virtual void OnInit(bool noclip=false);

	//! called if an event happened.
	virtual bool OnEvent(const Event& event);

	//! draws the element and its children
	virtual void Draw( );

	//! sets another skin independent font. if this is set to zero, the button uses the font of the skin.
	virtual void SetOverrideFont(const eastl::shared_ptr<BaseUIFont>& font=0);

	//! Gets the override font (if any)
	virtual const eastl::shared_ptr<BaseUIFont>& GetOverrideFont() const;

	//! Get the font which is used right now for drawing
	virtual eastl::shared_ptr<BaseUIFont> GetActiveFont() const;

	//! Sets an image which should be displayed on the button when it is in normal state.
	virtual void SetImage(const eastl::shared_ptr<Texture2>& image=0);

	//! Sets an image which should be displayed on the button when it is in normal state.
	virtual void SetImage(const eastl::shared_ptr<Texture2>& image, const RectangleShape<2, int>& pos);

	//! Sets an image which should be displayed on the button when it is in pressed state.
	virtual void SetPressedImage(const eastl::shared_ptr<Texture2>& image=0);

	//! Sets an image which should be displayed on the button when it is in pressed state.
	virtual void SetPressedImage(const eastl::shared_ptr<Texture2>& image, const RectangleShape<2, int>& pos);

	//! Sets the sprite bank used by the button
	virtual void SetSpriteBank(const eastl::shared_ptr<BaseUISpriteBank>& sprites=0);

	//! Sets the animated sprite for a specific button state
	/** \param index: Number of the sprite within the sprite bank, use -1 for no sprite
	\param state: State of the button to set the sprite for
	\param index: The sprite number from the current sprite bank
	\param color: The color of the sprite
	*/
	virtual void SetSprite(UIButtonState state, int index,
		eastl::array<float, 4> color = eastl::array<float, 4>{1.f, 1.f, 1.f, 1.f}, bool loop = false);

	//! Sets if the button should behave like a push button. Which means it
	//! can be in two states: Normal or Pressed. With a click on the button,
	//! the user can change the state of the button.
	virtual void SetPushButton(bool mPushButton=true);

	//! Checks whether the button is a push button
	virtual bool IsPushButton() const;

	//! Sets the pressed state of the button if this is a pushbutton
	virtual void SetPressed(bool pressed=true);

	//! Returns if the button is currently pressed
	virtual bool IsPressed() const;

	//! Sets if the button should use the skin to draw its border
	virtual void SetDrawBorder(bool border=true);

	//! Checks if the button face and border are being drawn
	virtual bool IsDrawingBorder() const;

	//! Sets if the alpha channel should be used for drawing images on the button (default is false)
	virtual void SetUseAlphaChannel(bool useAlphaChannel=true);

	//! Checks if the alpha channel should be used for drawing images on the button
	virtual bool IsAlphaChannelUsed() const;

	//! Sets if the button should scale the button images to fit
	virtual void SetScaleImage(bool scaleImage=true);

	//! Checks whether the button scales the used images
	virtual bool IsScalingImage() const;

private:

	struct ButtonSprite
	{
		int Index;
		eastl::array<float, 4> Color;
		bool Loop;
	};

	ButtonSprite mButtonSprites[BS_COUNT];

	BaseUI* mUI;

	eastl::shared_ptr<Visual> mVisual;
	eastl::shared_ptr<Texture2Effect> mEffect;
	eastl::shared_ptr<BaseUISpriteBank> mSpriteBank;
	eastl::shared_ptr<BaseUIFont> mOverrideFont;

	eastl::shared_ptr<Texture2> mImage;
	eastl::shared_ptr<Texture2> mPressedImage;

	RectangleShape<2, int> mImageRect;
	RectangleShape<2, int> mPressedImageRect;

	unsigned int mClickTime, mHoverTime, mFocusTime;

	bool mPushButton;
	bool mPressed;
	bool mUseAlphaChannel;
	bool mDrawBorder;
	bool mScaleImage;
};

#endif