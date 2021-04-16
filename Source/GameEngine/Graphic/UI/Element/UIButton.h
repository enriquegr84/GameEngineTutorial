// Copyright (C) 2002-2012 Nikolaus Gebhardt
// This file is part of the "Irrlicht Engine".
// For conditions of distribution and use, see copyright notice in irrlicht.h

#ifndef UIBUTTON_H
#define UIBUTTON_H

#include "UIElement.h"

#include "Graphic/State/BlendState.h"
#include "Graphic/Scene/Hierarchy/Visual.h"

//! Current state of buttons used for drawing sprites.
//! Note that up to 3 states can be active at the same time:
//! BS_BUTTON_UP or BS_BUTTON_DOWN
//! BS_BUTTON_MOUSE_OVER or BS_BUTTON_MOUSE_OFF
//! BS_BUTTON_FOCUSED or BS_BUTTON_NOT_FOCUSED
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
	BS_BUTTON_FOCUSED,
	//! The button doesn't have the focus
	BS_BUTTON_NOT_FOCUSED,
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

//! State of buttons used for drawing texture images.
//! Note that only a single state is active at a time
//! Also when no image is defined for a state it will use images from another state
//! and if that state is not set from the replacement for that,etc.
//! So in many cases setting BIS_IMAGE_UP and BIS_IMAGE_DOWN is sufficient.
enum UIButtonImageState
{
    //! When no other states have images they will all use this one.
    BIS_IMAGE_UP,
    //! When not set BIS_IMAGE_UP is used.
    BIS_IMAGE_UP_MOUSEOVER,
    //! When not set BIS_IMAGE_UP_MOUSEOVER is used.
    BIS_IMAGE_UP_FOCUSED,
    //! When not set BIS_IMAGE_UP_FOCUSED is used.
    BIS_IMAGE_UP_FOCUSED_MOUSEOVER,
    //! When not set BIS_IMAGE_UP is used.
    BIS_IMAGE_DOWN,
    //! When not set BIS_IMAGE_DOWN is used.
    BIS_IMAGE_DOWN_MOUSEOVER,
    //! When not set BIS_IMAGE_DOWN_MOUSEOVER is used.
    BIS_IMAGE_DOWN_FOCUSED,
    //! When not set BIS_IMAGE_DOWN_FOCUSED is used.
    BIS_IMAGE_DOWN_FOCUSED_MOUSEOVER,
    //! When not set BIS_IMAGE_UP or BIS_IMAGE_DOWN are used (depending on button state).
    BIS_IMAGE_DISABLED,
    //! not used, counts the number of enumerated items
    BIS_COUNT
};

//! Names for gui button image states
const char* const UIButtonImageStateNames[] =
{
    "Image",	// not "ImageUp" as it otherwise breaks serialization of old files
    "ImageUpOver",
    "ImageUpFocused",
    "ImageUpFocusedOver",
    "PressedImage",	// not "ImageDown" as it otherwise breaks serialization of old files
    "ImageDownOver",
    "ImageDownFocused",
    "ImageDownFocusedOver",
    "ImageDisabled",
    0	// count
};

class BaseUIStaticText;
class BaseUISpriteBank;
class BaseUIFont;

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

    //! Sets another color for the button text.
    /** When set, this color is used instead of EGDC_BUTTON_TEXT/EGDC_GRAY_TEXT.
    You don't need to call enableOverrideColor(true), that's done by this function.
    If you want the the color of the skin back, call enableOverrideColor(false);
    \param color: New color of the text. */
    virtual void SetOverrideColor(SColor color) = 0;

    //! Gets the override color
    virtual SColor GetOverrideColor() const = 0;

    //! Sets if the text should use the override color or the color in the gui skin.
    /** \param enable: If set to true, the override color, which can be set
    with IGUIEditBox::setOverrideColor is used, otherwise the
    EGDC_BUTTON_TEXT color of the skin. */
    virtual void EnableOverrideColor(bool enable) = 0;

    //! Checks if an override color is enabled
    /** \return true if the override color is enabled, false otherwise */
    virtual bool IsOverrideColorEnabled(void) const = 0;

    //! Set button color
    virtual void SetColor(SColor color) = 0;

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
    \param scale: True if the sprite should scale to button size, false if not	*/
	virtual void SetSprite(UIButtonState state, int index,
        SColor color = SColor(255, 255, 255, 255), bool loop = false, bool scale = false) = 0;

    //! Get the sprite-index for the given state or -1 when no sprite is set
    virtual int GetSpriteIndex(UIButtonState state) const = 0;

    //! Get the sprite color for the given state. Color is only used when a sprite is set.
    virtual SColor GetSpriteColor(UIButtonState state) const = 0;

    //! Returns if the sprite in the given state does loop
    virtual bool GetSpriteLoop(UIButtonState state) const = 0;

    //! Returns if the sprite in the given state is scaled
    virtual bool GetSpriteScale(UIButtonState state) const = 0;

	//! Sets if the button should behave like a push button.
	/** Which means it can be in two states: Normal or Pressed. With a click on the button,
	the user can change the state of the button. */
	virtual void SetPushButton(bool isPushButton = true) = 0;

	//! Sets the pressed state of the button if this is a pushbutton
	virtual void SetPressed(bool pressed = true) = 0;

	//! Returns if the button is currently pressed
	virtual bool IsPressed() const = 0;

    //! Returns if this element (or one of its direct children) is hovered
    virtual bool IsHovered() const = 0;

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

    //! Get if the shift key was pressed in last EGET_BUTTON_CLICKED event
    /** Generated together with event, so info is available in the event-receiver.	*/
    virtual bool GetClickShiftState() const = 0;

    //! Get if the control key was pressed in last EGET_BUTTON_CLICKED event
    /** Generated together with event, so info is available in the event-receiver.	*/
    virtual bool GetClickControlState() const = 0;
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

    //! Sets another color for the text.
    virtual void SetOverrideColor(SColor color);

    //! Gets the override color
    virtual SColor GetOverrideColor() const;

    //! Sets if the text should use the override color or the color in the gui skin.
    virtual void EnableOverrideColor(bool enable);

    //! Checks if an override color is enabled
    virtual bool IsOverrideColorEnabled(void) const;

    //! Set button color
    virtual void SetColor(SColor color);

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
    \param loop: True if the animation should loop, false if not
    \param scale: True if the sprite should scale to button size, false if not	*/
    virtual void SetSprite(UIButtonState state, int index,
        SColor color = SColor(255, 255, 255, 255), bool loop = false, bool scale = false);

    //! Get the sprite-index for the given state or -1 when no sprite is set
    virtual int GetSpriteIndex(UIButtonState state) const;

    //! Get the sprite color for the given state. Color is only used when a sprite is set.
    virtual SColor GetSpriteColor(UIButtonState state) const;

    //! Returns if the sprite in the given state does loop
    virtual bool GetSpriteLoop(UIButtonState state) const;

    //! Returns if the sprite in the given state is scaled
    virtual bool GetSpriteScale(UIButtonState state) const;

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

    //! Returns if this element (or one of its direct children) is hovered
    virtual bool IsHovered() const ;

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

    //! Get if the shift key was pressed in last EGET_BUTTON_CLICKED event
    /** Generated together with event, so info is available in the event-receiver.	*/
    virtual bool GetClickShiftState() const
    {
        return mClickShiftState;
    }

    //! Get if the control key was pressed in last EGET_BUTTON_CLICKED event
    /** Generated together with event, so info is available in the event-receiver.	*/
    virtual bool GetClickControlState() const
    {
        return mClickControlState;
    }

protected:

    void DrawSprite(UIButtonState state, uint32_t startTime, const RectangleShape<2, int>& center);

    struct ButtonImage
    {
        ButtonImage() : Texture(0)
        {
        }

        ButtonImage(const ButtonImage& other)
        {
            *this = other;
        }

        ~ButtonImage()
        {
        }

        ButtonImage& operator=(const ButtonImage& other)
        {
            if (this == &other)
                return *this;

            Texture = other.Texture;
            SourceRect = other.SourceRect;
            return *this;
        }

        bool operator==(const ButtonImage& other) const
        {
            return Texture == other.Texture && SourceRect == other.SourceRect;
        }


        eastl::shared_ptr<Texture2> Texture;
        RectangleShape<2, int> SourceRect;
    };

    UIButtonImageState GetImageState(bool pressed) const;
    UIButtonImageState GetImageState(bool pressed, const ButtonImage* images) const;

private:

	struct ButtonSprite
	{
        ButtonSprite() : Index(-1), Loop(false), Scale(false)
        {
        }

        bool operator==(const ButtonSprite& other) const
        {
            return Index == other.Index && Color == other.Color && Loop == other.Loop && Scale == other.Scale;
        }

        int Index;
        SColor Color;
        bool Loop;
        bool Scale;
	};

	ButtonSprite mButtonSprites[BS_COUNT];
    ButtonImage mButtonImages[BIS_COUNT];

	BaseUI* mUI;

	eastl::shared_ptr<Visual> mVisual;
	eastl::shared_ptr<VisualEffect> mEffect;
	eastl::shared_ptr<BlendState> mBlendState;

	eastl::shared_ptr<BaseUISpriteBank> mSpriteBank;
	eastl::shared_ptr<BaseUIFont> mOverrideFont;

    eastl::shared_ptr<BaseUIStaticText> mStaticText;

    bool mOverrideColorEnabled;
    SColor mOverrideColor;

	unsigned int mClickTime, mHoverTime, mFocusTime;

    bool mClickShiftState;
    bool mClickControlState;

	bool mPushButton;
	bool mPressed;
	bool mUseAlphaChannel;
	bool mDrawBorder;
	bool mScaleImage;

    SColor mColors[4];
    bool mWasHovered = false;

    SColor mBGColor;
    RectangleShape<2, int> mBGMiddle;
    RectangleShape<2, int> mPadding;
    Vector2<int> mContentOffset;
};

#endif