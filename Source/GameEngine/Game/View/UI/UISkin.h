// Copyright (C) 2002-2012 Nikolaus Gebhardt
// This file is part of the "Irrlicht Engine".
// For conditions of distribution and use, see copyright notice in irrlicht.h

#ifndef UISKIN_H
#define UISKIN_H

#include "UIFont.h"
#include "UIElement.h"

//! Enumeration of available default skins.
/** To set one of the skins, use the following code, for example to set
the Windows classic skin:
\code
shared_ptr<UISkin> newskin = environment->CreateSkin(EGSTT_WINDOWS_CLASSIC);
environment->SetSkin(newskin);
newskin->drop();
\endcode
*/
enum UISkinThemeType
{
	//! Default windows look and feel
	STT_WINDOWS_CLASSIC=0,

	//! Like EGST_WINDOWS_CLASSIC, but with metallic shaded windows and buttons
	STT_WINDOWS_METALLIC,

	//! Burning's skin
	STT_BURNING_SKIN,

	//! An unknown skin, not serializable at present
	STT_UNKNOWN,

	//! this value is not used, it only specifies the number of skin types
	STT_COUNT
};


//! Enumeration for skin colors
enum UIDefaultColor
{
	//! Dark shadow for three-dimensional display elements.
	DC_3D_DARK_SHADOW = 0,
	//! Shadow color for three-dimensional display elements (for edges facing away from the light source).
	DC_3D_SHADOW,
	//! Face color for three-dimensional display elements and for dialog box backgrounds.
	DC_3D_FACE,
	//! Highlight color for three-dimensional display elements (for edges facing the light source.)
	DC_3D_HIGH_LIGHT,
	//! Light color for three-dimensional display elements (for edges facing the light source.)
	DC_3D_LIGHT,
	//! Active window border.
	DC_ACTIVE_BORDER,
	//! Active window title bar text.
	DC_ACTIVE_CAPTION,
	//! Background color of multiple document interface (MDI) applications.
	DC_APP_WORKSPACE,
	//! Text on a button
	DC_BUTTON_TEXT,
	//! Grayed (disabled) text.
	DC_GRAY_TEXT,
	//! Item(s) selected in a control.
	DC_HIGH_LIGHT,
	//! Text of item(s) selected in a control.
	DC_HIGH_LIGHT_TEXT,
	//! Inactive window border.
	DC_INACTIVE_BORDER,
	//! Inactive window caption.
	DC_INACTIVE_CAPTION,
	//! Tool tip text color
	DC_TOOLTIP,
	//! Tool tip background color
	DC_TOOLTIP_BACKGROUND,
	//! Scrollbar gray area
	DC_SCROLLBAR,
	//! Window background
	DC_WINDOW,
	//! Window symbols like on close buttons, scroll bars and check boxes
	DC_WINDOW_SYMBOL,
	//! Icons in a list or tree
	DC_ICON_NORMAL,
	//! Selected icons in a list or tree
	DC_ICON_HIGH_LIGHT,
	//! Grayed (disabled) window symbols like on close buttons, scroll bars and check boxes
	DC_GRAY_WINDOW_SYMBOL,
	//! Window background for editable field (UIEditBox, checkbox-field)
	DC_EDITABLE,
	//! Grayed (disabled) window background for editable field (UIEditBox, checkbox-field)
	DC_GRAY_EDITABLE,
	//! Show focus of window background for editable field (UIEditBox or when checkbox-field is pressed)
	DC_FOCUSED_EDITABLE,

	//! this value is not used, it only specifies the amount of default colors
	//! available.
	DC_COUNT
};

//! Enumeration for default sizes.
enum UIDefaultSize
{
	//! default with / height of scrollbar
	DS_SCROLLBAR_SIZE = 0,
	//! height of menu
	DS_MENU_HEIGHT,
	//! width of a window button
	DS_WINDOW_BUTTON_WIDTH,
	//! width of a checkbox check
	DS_CHECK_BOX_WIDTH,
	//! \deprecated This may be removed by Irrlicht 1.9
	DS_MESSAGE_BOX_WIDTH,
	//! \deprecated This may be removed by Irrlicht 1.9
	DS_MESSAGE_BOX_HEIGHT,
	//! width of a default button
	DS_BUTTON_WIDTH,
	//! height of a default button
	DS_BUTTON_HEIGHT,
	//! distance for text from background
	DS_TEXT_DISTANCE_X,
	//! distance for text from background
	DS_TEXT_DISTANCE_Y,
	//! distance for text in the title bar, from the left of the window rect
	DS_TITLEBARTEXT_DISTANCE_X,
	//! distance for text in the title bar, from the top of the window rect
	DS_TITLEBARTEXT_DISTANCE_Y,
	//! free space in a messagebox between borders and contents on all sides
	DS_MESSAGE_BOX_GAP_SPACE,
	//! minimal space to reserve for messagebox text-width
	DS_MESSAGE_BOX_MIN_TEXT_WIDTH,
	//! maximal space to reserve for messagebox text-width
	DS_MESSAGE_BOX_MAX_TEXT_WIDTH,
	//! minimal space to reserve for messagebox text-height
	DS_MESSAGE_BOX_MIN_TEXT_HEIGHT,
	//! maximal space to reserve for messagebox text-height
	DS_MESSAGE_BOX_MAX_TEXT_HEIGHT,
	//! pixels to move the button image to the right when a pushbutton is pressed
	DS_BUTTON_PRESSED_IMAGE_OFFSET_X,
	//! pixels to move the button image down when a pushbutton is pressed
	DS_BUTTON_PRESSED_IMAGE_OFFSET_Y,
	//! pixels to move the button text to the right when a pushbutton is pressed
	DS_BUTTON_PRESSED_TEXT_OFFSET_X,
	//! pixels to move the button text down when a pushbutton is pressed
	DS_BUTTON_PRESSED_TEXT_OFFSET_Y,

	//! this value is not used, it only specifies the amount of default sizes
	//! available.
	DS_COUNT
};


enum UIDefaultText
{
	//! Text for the OK button on a message box
	DT_MSG_BOX_OK = 0,
	//! Text for the Cancel button on a message box
	DT_MSG_BOX_CANCEL,
	//! Text for the Yes button on a message box
	DT_MSG_BOX_YES,
	//! Text for the No button on a message box
	DT_MSG_BOX_NO,
	//! Tooltip text for window close button
	DT_WINDOW_CLOSE,
	//! Tooltip text for window maximize button
	DT_WINDOW_MAXIMIZE,
	//! Tooltip text for window minimize button
	DT_WINDOW_MINIMIZE,
	//! Tooltip text for window restore button
	DT_WINDOW_RESTORE,

	//! this value is not used, it only specifies the number of default texts
	DT_COUNT
};


//! Customizable symbols for UI
enum UIDefaultIcon
{
	//! maximize window button
	DI_WINDOW_MAXIMIZE = 0,
	//! restore window button
	DI_WINDOW_RESTORE,
	//! close window button
	DI_WINDOW_CLOSE,
	//! minimize window button
	DI_WINDOW_MINIMIZE,
	//! resize icon for bottom right corner of a window
	DI_WINDOW_RESIZE,
	//! scroll bar up button
	DI_CURSOR_UP,
	//! scroll bar down button
	DI_CURSOR_DOWN,
	//! scroll bar left button
	DI_CURSOR_LEFT,
	//! scroll bar right button
	DI_CURSOR_RIGHT,
	//! icon for menu children
	DI_MENU_MORE,
	//! tick for checkbox
	DI_CHECK_BOX_CHECKED,
	//! down arrow for dropdown menus
	DI_DROP_DOWN,
	//! smaller up arrow
	DI_SMALL_CURSOR_UP,
	//! smaller down arrow
	DI_SMALL_CURSOR_DOWN,
	//! selection dot in a radio button
	DI_RADIO_BUTTON_CHECKED,
	//! << icon indicating there is more content to the left
	DI_MORE_LEFT,
	//! >> icon indicating that there is more content to the right
	DI_MORE_RIGHT,
	//! icon indicating that there is more content above
	DI_MORE_UP,
	//! icon indicating that there is more content below
	DI_MORE_DOWN,
	//! plus icon for trees
	DI_EXPAND,

	//! minus icon for trees
	DI_COLLAPSE,
	//! file icon for file selection
	DI_FILE,
	//! folder icon for file selection
	DI_DIRECTORY,

	//! value not used, it only specifies the number of icons
	DI_COUNT
};


// Customizable fonts
enum UIDefaultFont
{
	//! For static text, edit boxes, lists and most other places
	DF_DEFAULT=0,
	//! Font for buttons
	DF_BUTTON,
	//! Font for window title bars
	DF_WINDOW,
	//! Font for menu items
	DF_MENU,
	//! Font for tooltips
	DF_TOOLTIP,
	//! this value is not used, it only specifies the amount of default fonts
	//! available.
	DF_COUNT
};


//! A skin modifies the look of the UI elements.
class BaseUISkin
{
public:

	//! returns default color
	virtual eastl::array<float, 4> const GetColor(UIDefaultColor color) const = 0;

	//! sets a default color
	virtual void SetColor(UIDefaultColor which, eastl::array<float, 4> const newColor) = 0;

	//! returns size for the given size type
	virtual int GetSize(UIDefaultSize size) const = 0;

	//! Returns a default text.
	/** For example for Message box button captions:
	"OK", "Cancel", "Yes", "No" and so on. */
	virtual const wchar_t* GetDefaultText(UIDefaultText text) const = 0;

	//! Sets a default text.
	/** For example for Message box button captions:
	"OK", "Cancel", "Yes", "No" and so on. */
	virtual void SetDefaultText(UIDefaultText which, const wchar_t* newText) = 0;

	//! sets a default size
	virtual void SetSize(UIDefaultSize which, int size) = 0;

	//! returns the default font
	virtual const eastl::shared_ptr<BaseUIFont>& GetFont(UIDefaultFont which=DF_DEFAULT) const = 0;

	//! sets a default font
	virtual void SetFont(const eastl::shared_ptr<BaseUIFont>& font, UIDefaultFont which=DF_DEFAULT) = 0;

	//! sets the sprite bank used for drawing icons
	virtual void SetSpriteBank(const eastl::shared_ptr<BaseUISpriteBank>& bank) = 0;

	//! gets the sprite bank used for drawing icons
	virtual const eastl::shared_ptr<BaseUISpriteBank>& GetSpriteBank() const = 0;

	//! Returns a default icon
	/** Returns the sprite index within the sprite bank */
	virtual const wchar_t* GetIcon(UIDefaultIcon icon) const = 0;

	//! Sets a default icon
	/** Sets the sprite index used for drawing icons like arrows,
	close buttons and ticks in checkboxes
	\param icon: Enum specifying which icon to change
	\param index: The sprite index used to draw this icon */
	virtual void SetIcon(UIDefaultIcon icon, wchar_t* path) = 0;

	//! draws a standard 3d button pane
	/** Used for drawing for example buttons in normal state.
	It uses the colors EGDC_3D_DARK_SHADOW, EGDC_3D_HIGH_LIGHT, EGDC_3D_SHADOW and
	EGDC_3D_FACE for this. See EGUI_DEFAULT_COLOR for details.
	\param rect: Defining area where to draw.
	\param clip: Clip area.
	\param element: Pointer to the element which wishes to draw this. This parameter
	is usually not used by ISkin, but can be used for example by more complex
	implementations to find out how to draw the part exactly. */
	virtual void Draw3DButtonPaneStandard(const eastl::shared_ptr<BaseUIElement>&,
		const eastl::shared_ptr<Visual>& visual, const RectangleBase<2, int>& rect, const RectangleBase<2, int>* clip = 0) = 0;

	//! draws a pressed 3d button pane
	/** Used for drawing for example buttons in pressed state.
	It uses the colors EGDC_3D_DARK_SHADOW, EGDC_3D_HIGH_LIGHT, EGDC_3D_SHADOW and
	EGDC_3D_FACE for this. See EGUI_DEFAULT_COLOR for details.
	\param rect: Defining area where to draw.
	\param clip: Clip area.
	\param element: Pointer to the element which wishes to draw this. This parameter
	is usually not used by ISkin, but can be used for example by more complex
	implementations to find out how to draw the part exactly. */
	virtual void Draw3DButtonPanePressed(const eastl::shared_ptr<BaseUIElement>&,
		const eastl::shared_ptr<Visual>& visual, const RectangleBase<2, int>& rect, const RectangleBase<2, int>* clip = 0) = 0;

	//! draws a sunken 3d pane
	/** Used for drawing the background of edit, combo or check boxes.
	\param element: Pointer to the element which wishes to draw this. This parameter
	is usually not used by ISkin, but can be used for example by more complex
	implementations to find out how to draw the part exactly.
	\param bgcolor: Background color.
	\param flat: Specifies if the sunken pane should be flat or displayed as sunken
	deep into the ground.
	\param rect: Defining area where to draw.
	\param clip: Clip area.	*/
	virtual void Draw3DSunkenPane(const eastl::shared_ptr<BaseUIElement>& element,
		eastl::array<float, 4> const bgcolor, bool flat, bool fillBackGround,
		const eastl::shared_ptr<Visual>& visual, const RectangleBase<2, int>& rect, const RectangleBase<2, int>* clip = 0) = 0;

	//! draws a window background
	/** Used for drawing the background of dialogs and windows.
	\param element: Pointer to the element which wishes to draw this. This parameter
	is usually not used by ISkin, but can be used for example by more complex
	implementations to find out how to draw the part exactly.
	\param titleBarColor: Title color.
	\param drawTitleBar: True to enable title drawing.
	\param rect: Defining area where to draw.
	\param clip: Clip area.
	\param checkClientArea: When set to non-null the function will not draw anything,
	but will instead return the clientArea which can be used for drawing by the calling window.
	That is the area without borders and without titlebar.
	\return Returns rect where it would be good to draw title bar text. This will
	work even when checkClientArea is set to a non-null value.*/
	virtual RectangleBase<2, int> Draw3DWindowBackground(const eastl::shared_ptr<BaseUIElement>& element,
		const eastl::shared_ptr<Visual>& visualBackground, const eastl::shared_ptr<Visual>& visualTitle,
		bool drawTitleBar, eastl::array<float, 4> const titleBarColor, const RectangleBase<2, int>& rect,
		const RectangleBase<2, int>* clip = 0, RectangleBase<2, int>* checkClientArea = 0) = 0;

	//! draws a standard 3d menu pane
	/** Used for drawing for menus and context menus.
	It uses the colors EGDC_3D_DARK_SHADOW, EGDC_3D_HIGH_LIGHT, EGDC_3D_SHADOW and
	EGDC_3D_FACE for this. See EGUI_DEFAULT_COLOR for details.
	\param element: Pointer to the element which wishes to draw this. This parameter
	is usually not used by ISkin, but can be used for example by more complex
	implementations to find out how to draw the part exactly.
	\param rect: Defining area where to draw.
	\param clip: Clip area.	*/
	virtual void Draw3DMenuPane(const eastl::shared_ptr<BaseUIElement>& element,
		const eastl::shared_ptr<Visual>& visual, const RectangleBase<2, int>& rect, const RectangleBase<2, int>* clip = 0) = 0;

	//! draws a standard 3d tool bar
	/** Used for drawing for toolbars and menus.
	\param element: Pointer to the element which wishes to draw this. This parameter
	is usually not used by ISkin, but can be used for example by more complex
	implementations to find out how to draw the part exactly.
	\param rect: Defining area where to draw.
	\param clip: Clip area.	*/
	virtual void Draw3DToolBar(const eastl::shared_ptr<BaseUIElement>& element,
		const eastl::shared_ptr<Visual>& visual, const RectangleBase<2, int>& rect, const RectangleBase<2, int>* clip = 0) = 0;

	//! draws a tab button
	/** Used for drawing for tab buttons on top of tabs.
	\param element: Pointer to the element which wishes to draw this. This parameter
	is usually not used by ISkin, but can be used for example by more complex
	implementations to find out how to draw the part exactly.
	\param active: Specifies if the tab is currently active.
	\param rect: Defining area where to draw.
	\param clip: Clip area.	*/
	virtual void Draw3DTabButton(const eastl::shared_ptr<BaseUIElement>& element, bool active,
		const eastl::shared_ptr<Visual>& visual, const RectangleBase<2, int>& rect, const RectangleBase<2, int>* clip = 0,
		UIAlignment alignment = UIA_UPPERLEFT) = 0;

	//! draws a tab control body
	/** \param element: Pointer to the element which wishes to draw this. This parameter
	is usually not used by ISkin, but can be used for example by more complex
	implementations to find out how to draw the part exactly.
	\param border: Specifies if the border should be drawn.
	\param background: Specifies if the background should be drawn.
	\param rect: Defining area where to draw.
	\param clip: Clip area.	*/
	virtual void Draw3DTabBody(const eastl::shared_ptr<BaseUIElement>& element, bool border, bool background,
		const eastl::shared_ptr<Visual>& visual, const RectangleBase<2, int>& rect, const RectangleBase<2, int>* clip = 0,
		int tabHeight = -1, UIAlignment alignment = UIA_UPPERLEFT) = 0;

	//! draws an icon, usually from the skin's sprite bank
	/** \param element: Pointer to the element which wishes to draw this icon.
	This parameter is usually not used by IGUISkin, but can be used for example
	by more complex implementations to find out how to draw the part exactly.
	\param icon: Specifies the icon to be drawn.
	\param position: The position to draw the icon
	\param starttime: The time at the start of the animation
	\param currenttime: The present time, used to calculate the frame number
	\param loop: Whether the animation should loop or not
	\param clip: Clip area.	*/
	virtual void DrawIcon(const eastl::shared_ptr<BaseUIElement>& element, UIDefaultIcon icon,
		const Vector2<int> position, const eastl::shared_ptr<Visual>& visual, const RectangleBase<2, int>* clip = 0,
		unsigned int starttime = 0, unsigned int currenttime = 0, bool loop = false) = 0;

	//! draws a 2d rectangle.
	/** \param element: Pointer to the element which wishes to draw this icon.
	This parameter is usually not used by IGUISkin, but can be used for example
	by more complex implementations to find out how to draw the part exactly.
	\param color: Color of the rectangle to draw. The alpha component specifies how
	transparent the rectangle will be.
	\param pos: Position of the rectangle.
	\param clip: Pointer to rectangle against which the rectangle will be clipped.
	If the pointer is null, no clipping will be performed. */
	virtual void Draw2DRectangle(const eastl::shared_ptr<BaseUIElement>& element,
		const eastl::array<float, 4> &color, const eastl::shared_ptr<Visual>& visual,
		const RectangleBase<2, int>& r, const RectangleBase<2, int>* clip = 0) = 0;

	//! get the type of this skin
	virtual UISkinThemeType GetType() const { return STT_UNKNOWN; }
};

class Renderer;

class UISkin : public BaseUISkin
{
public:

	UISkin(BaseUI* ui, UISkinThemeType type);

	//! destructor
	virtual ~UISkin();

	//! returns default color
	virtual eastl::array<float, 4> const GetColor(UIDefaultColor color) const;

	//! sets a default color
	virtual void SetColor(UIDefaultColor which, eastl::array<float, 4> const newColor);

	//! returns size for the given size type
	virtual int GetSize(UIDefaultSize size) const;

	//! sets a default size
	virtual void SetSize(UIDefaultSize which, int size);

	//! returns the default font
	virtual const eastl::shared_ptr<BaseUIFont>& GetFont(UIDefaultFont which = DF_DEFAULT) const;

	//! sets a default font
	virtual void SetFont(const eastl::shared_ptr<BaseUIFont>& font, UIDefaultFont which = DF_DEFAULT);

	//! sets the sprite bank used for drawing icons
	virtual void SetSpriteBank(const eastl::shared_ptr<BaseUISpriteBank>& bank);

	//! gets the sprite bank used for drawing icons
	virtual const eastl::shared_ptr<BaseUISpriteBank>& GetSpriteBank() const;

	//! Returns a default icon
	/** Returns the sprite index within the sprite bank */
	virtual const wchar_t* GetIcon(UIDefaultIcon icon) const;

	//! Sets a default icon
	/** Sets the sprite index used for drawing icons like arrows,
	close buttons and ticks in checkboxes
	\param icon: Enum specifying which icon to change
	\param index: The sprite index used to draw this icon */
	virtual void SetIcon(UIDefaultIcon icon, wchar_t* path);

	//! Returns a default text.
	/** For example for Message box button captions:
	"OK", "Cancel", "Yes", "No" and so on. */
	virtual const wchar_t* GetDefaultText(UIDefaultText text) const;

	//! Sets a default text.
	/** For example for Message box button captions:
	"OK", "Cancel", "Yes", "No" and so on. */
	virtual void SetDefaultText(UIDefaultText which, const wchar_t* newText);

	//! draws a standard 3d button pane
	/** Used for drawing for example buttons in normal state.
	It uses the colors EGDC_3D_DARK_SHADOW, EGDC_3D_HIGH_LIGHT, EGDC_3D_SHADOW and
	EGDC_3D_FACE for this. See EGUI_DEFAULT_COLOR for details.
	\param rect: Defining area where to draw.
	\param clip: Clip area.
	\param element: Pointer to the element which wishes to draw this. This parameter
	is usually not used by ISkin, but can be used for example by more complex
	implementations to find out how to draw the part exactly. */
	virtual void Draw3DButtonPaneStandard(const eastl::shared_ptr<BaseUIElement>&,
		const eastl::shared_ptr<Visual>& visual, const RectangleBase<2, int>& rect, const RectangleBase<2, int>* clip = 0);

	//! draws a pressed 3d button pane
	/** Used for drawing for example buttons in pressed state.
	It uses the colors EGDC_3D_DARK_SHADOW, EGDC_3D_HIGH_LIGHT, EGDC_3D_SHADOW and
	EGDC_3D_FACE for this. See EGUI_DEFAULT_COLOR for details.
	\param rect: Defining area where to draw.
	\param clip: Clip area.
	\param element: Pointer to the element which wishes to draw this. This parameter
	is usually not used by ISkin, but can be used for example by more complex
	implementations to find out how to draw the part exactly. */
	virtual void Draw3DButtonPanePressed(const eastl::shared_ptr<BaseUIElement>&,
		const eastl::shared_ptr<Visual>& visual, const RectangleBase<2, int>& rect, const RectangleBase<2, int>* clip = 0);

	//! draws a sunken 3d pane
	/** Used for drawing the background of edit, combo or check boxes.
	\param element: Pointer to the element which wishes to draw this. This parameter
	is usually not used by ISkin, but can be used for example by more complex
	implementations to find out how to draw the part exactly.
	\param bgcolor: Background color.
	\param flat: Specifies if the sunken pane should be flat or displayed as sunken
	deep into the ground.
	\param rect: Defining area where to draw.
	\param clip: Clip area.	*/
	virtual void Draw3DSunkenPane(const eastl::shared_ptr<BaseUIElement>& element,
		eastl::array<float, 4> const bgcolor, bool flat, bool fillBackGround,
		const eastl::shared_ptr<Visual>& visual, const RectangleBase<2, int>& rect, const RectangleBase<2, int>* clip = 0);

	//! draws a window background
	/** Used for drawing the background of dialogs and windows.
	\param element: Pointer to the element which wishes to draw this. This parameter
	is usually not used by ISkin, but can be used for example by more complex
	implementations to find out how to draw the part exactly.
	\param titleBarColor: Title color.
	\param drawTitleBar: True to enable title drawing.
	\param rect: Defining area where to draw.
	\param clip: Clip area.
	\param checkClientArea: When set to non-null the function will not draw anything,
	but will instead return the clientArea which can be used for drawing by the calling window.
	That is the area without borders and without titlebar.
	\return Returns rect where it would be good to draw title bar text. This will
	work even when checkClientArea is set to a non-null value.*/
	virtual RectangleBase<2, int> Draw3DWindowBackground(const eastl::shared_ptr<BaseUIElement>& element,
		const eastl::shared_ptr<Visual>& visualBackground, const eastl::shared_ptr<Visual>& visualTitle, 
		bool drawTitleBar, eastl::array<float, 4> const titleBarColor, const RectangleBase<2, int>& rect, 
		const RectangleBase<2, int>* clip = 0, RectangleBase<2, int>* checkClientArea = 0);

	//! draws a standard 3d menu pane
	/** Used for drawing for menus and context menus.
	It uses the colors EGDC_3D_DARK_SHADOW, EGDC_3D_HIGH_LIGHT, EGDC_3D_SHADOW and
	EGDC_3D_FACE for this. See EGUI_DEFAULT_COLOR for details.
	\param element: Pointer to the element which wishes to draw this. This parameter
	is usually not used by ISkin, but can be used for example by more complex
	implementations to find out how to draw the part exactly.
	\param rect: Defining area where to draw.
	\param clip: Clip area.	*/
	virtual void Draw3DMenuPane(const eastl::shared_ptr<BaseUIElement>& element,
		const eastl::shared_ptr<Visual>& visual, const RectangleBase<2, int>& rect, const RectangleBase<2, int>* clip = 0);

	//! draws a standard 3d tool bar
	/** Used for drawing for toolbars and menus.
	\param element: Pointer to the element which wishes to draw this. This parameter
	is usually not used by ISkin, but can be used for example by more complex
	implementations to find out how to draw the part exactly.
	\param rect: Defining area where to draw.
	\param clip: Clip area.	*/
	virtual void Draw3DToolBar(const eastl::shared_ptr<BaseUIElement>& element,
		const eastl::shared_ptr<Visual>& visual, const RectangleBase<2, int>& rect, const RectangleBase<2, int>* clip = 0);

	//! draws a tab button
	/** Used for drawing for tab buttons on top of tabs.
	\param element: Pointer to the element which wishes to draw this. This parameter
	is usually not used by ISkin, but can be used for example by more complex
	implementations to find out how to draw the part exactly.
	\param active: Specifies if the tab is currently active.
	\param rect: Defining area where to draw.
	\param clip: Clip area.	*/
	virtual void Draw3DTabButton(const eastl::shared_ptr<BaseUIElement>& element, bool active,
		const eastl::shared_ptr<Visual>& visual, const RectangleBase<2, int>& rect, const RectangleBase<2, int>* clip = 0, 
		UIAlignment alignment = UIA_UPPERLEFT);

	//! draws a tab control body
	/** \param element: Pointer to the element which wishes to draw this. This parameter
	is usually not used by ISkin, but can be used for example by more complex
	implementations to find out how to draw the part exactly.
	\param border: Specifies if the border should be drawn.
	\param background: Specifies if the background should be drawn.
	\param rect: Defining area where to draw.
	\param clip: Clip area.	*/
	virtual void Draw3DTabBody(const eastl::shared_ptr<BaseUIElement>& element, bool border, bool background,
		const eastl::shared_ptr<Visual>& visual, const RectangleBase<2, int>& rect, const RectangleBase<2, int>* clip = 0, 
		int tabHeight = -1, UIAlignment alignment = UIA_UPPERLEFT);

	//! draws an icon, usually from the skin's sprite bank
	/** \param element: Pointer to the element which wishes to draw this icon.
	This parameter is usually not used by IGUISkin, but can be used for example
	by more complex implementations to find out how to draw the part exactly.
	\param icon: Specifies the icon to be drawn.
	\param position: The position to draw the icon
	\param starttime: The time at the start of the animation
	\param currenttime: The present time, used to calculate the frame number
	\param loop: Whether the animation should loop or not
	\param clip: Clip area.	*/
	virtual void DrawIcon(const eastl::shared_ptr<BaseUIElement>& element, UIDefaultIcon icon, 
		const Vector2<int> position, const eastl::shared_ptr<Visual>& visual, const RectangleBase<2, int>* clip = 0, 
		unsigned int starttime = 0, unsigned int currenttime = 0, bool loop = false);

	//! draws a 2d rectangle.
	/** \param element: Pointer to the element which wishes to draw this icon.
	This parameter is usually not used by IGUISkin, but can be used for example
	by more complex implementations to find out how to draw the part exactly.
	\param color: Color of the rectangle to draw. The alpha component specifies how
	transparent the rectangle will be.
	\param pos: Position of the rectangle.
	\param clip: Pointer to rectangle against which the rectangle will be clipped.
	If the pointer is null, no clipping will be performed. */
	virtual void Draw2DRectangle(const eastl::shared_ptr<BaseUIElement>& element, 
		const eastl::array<float, 4> &color, const eastl::shared_ptr<Visual>& visual, 
		const RectangleBase<2, int>& r, const RectangleBase<2, int>* clip = 0 );

	//! get the type of this skin
	virtual UISkinThemeType GetType() const;

private:

	BaseUI* mUI;

	eastl::array<float, 4> mColors[DC_COUNT];
	eastl::shared_ptr<BaseUIFont> mFonts[DF_COUNT];
	eastl::shared_ptr<BaseUISpriteBank> mSpriteBank;
	eastl::wstring mTexts[DT_COUNT];
	eastl::wstring mIcons[DI_COUNT];
	int mSizes[DS_COUNT];
	bool mUseGradient;

	UISkinThemeType mType;
};

#endif