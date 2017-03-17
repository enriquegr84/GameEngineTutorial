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
enum EUI_SKIN_THEME_TYPE
{
	//! Default windows look and feel
	EGSTT_WINDOWS_CLASSIC=0,

	//! Like EGST_WINDOWS_CLASSIC, but with metallic shaded windows and buttons
	EGSTT_WINDOWS_METALLIC,

	//! Burning's skin
	EGSTT_BURNING_SKIN,

	//! An unknown skin, not serializable at present
	EGSTT_UNKNOWN,

	//! this value is not used, it only specifies the number of skin types
	EGSTT_COUNT
};


//! Enumeration for skin colors
enum EUI_DEFAULT_COLOR
{
	//! Dark shadow for three-dimensional display elements.
	EGDC_3D_DARK_SHADOW = 0,
	//! Shadow color for three-dimensional display elements (for edges facing away from the light source).
	EGDC_3D_SHADOW,
	//! Face color for three-dimensional display elements and for dialog box backgrounds.
	EGDC_3D_FACE,
	//! Highlight color for three-dimensional display elements (for edges facing the light source.)
	EGDC_3D_HIGH_LIGHT,
	//! Light color for three-dimensional display elements (for edges facing the light source.)
	EGDC_3D_LIGHT,
	//! Active window border.
	EGDC_ACTIVE_BORDER,
	//! Active window title bar text.
	EGDC_ACTIVE_CAPTION,
	//! Background color of multiple document interface (MDI) applications.
	EGDC_APP_WORKSPACE,
	//! Text on a button
	EGDC_BUTTON_TEXT,
	//! Grayed (disabled) text.
	EGDC_GRAY_TEXT,
	//! Item(s) selected in a control.
	EGDC_HIGH_LIGHT,
	//! Text of item(s) selected in a control.
	EGDC_HIGH_LIGHT_TEXT,
	//! Inactive window border.
	EGDC_INACTIVE_BORDER,
	//! Inactive window caption.
	EGDC_INACTIVE_CAPTION,
	//! Tool tip text color
	EGDC_TOOLTIP,
	//! Tool tip background color
	EGDC_TOOLTIP_BACKGROUND,
	//! Scrollbar gray area
	EGDC_SCROLLBAR,
	//! Window background
	EGDC_WINDOW,
	//! Window symbols like on close buttons, scroll bars and check boxes
	EGDC_WINDOW_SYMBOL,
	//! Icons in a list or tree
	EGDC_ICON,
	//! Selected icons in a list or tree
	EGDC_ICON_HIGH_LIGHT,
	//! Grayed (disabled) window symbols like on close buttons, scroll bars and check boxes
	EGDC_GRAY_WINDOW_SYMBOL,
	//! Window background for editable field (UIEditBox, checkbox-field)
	EGDC_EDITABLE,
	//! Grayed (disabled) window background for editable field (UIEditBox, checkbox-field)
	EGDC_GRAY_EDITABLE,
	//! Show focus of window background for editable field (UIEditBox or when checkbox-field is pressed)
	EGDC_Focused_EDITABLE,

	//! this value is not used, it only specifies the amount of default colors
	//! available.
	EGDC_COUNT
};

//! Enumeration for default sizes.
enum EUI_DEFAULT_SIZE
{
	//! default with / height of scrollbar
	EGDS_SCROLLBAR_SIZE = 0,
	//! height of menu
	EGDS_MENU_HEIGHT,
	//! width of a window button
	EGDS_WINDOW_BUTTON_WIDTH,
	//! width of a checkbox check
	EGDS_CHECK_BOX_WIDTH,
	//! \deprecated This may be removed by Irrlicht 1.9
	EGDS_MESSAGE_BOX_WIDTH,
	//! \deprecated This may be removed by Irrlicht 1.9
	EGDS_MESSAGE_BOX_HEIGHT,
	//! width of a default button
	EGDS_BUTTON_WIDTH,
	//! height of a default button
	EGDS_BUTTON_HEIGHT,
	//! distance for text from background
	EGDS_TEXT_DISTANCE_X,
	//! distance for text from background
	EGDS_TEXT_DISTANCE_Y,
	//! distance for text in the title bar, from the left of the window rect
	EGDS_TITLEBARTEXT_DISTANCE_X,
	//! distance for text in the title bar, from the top of the window rect
	EGDS_TITLEBARTEXT_DISTANCE_Y,
	//! free space in a messagebox between borders and contents on all sides
	EGDS_MESSAGE_BOX_GAP_SPACE,
	//! minimal space to reserve for messagebox text-width
	EGDS_MESSAGE_BOX_MIN_TEXT_WIDTH,
	//! maximal space to reserve for messagebox text-width
	EGDS_MESSAGE_BOX_MAX_TEXT_WIDTH,
	//! minimal space to reserve for messagebox text-height
	EGDS_MESSAGE_BOX_MIN_TEXT_HEIGHT,
	//! maximal space to reserve for messagebox text-height
	EGDS_MESSAGE_BOX_MAX_TEXT_HEIGHT,
	//! pixels to move the button image to the right when a pushbutton is pressed
	EGDS_BUTTON_PRESSED_IMAGE_OFFSET_X,
	//! pixels to move the button image down when a pushbutton is pressed
	EGDS_BUTTON_PRESSED_IMAGE_OFFSET_Y,
	//! pixels to move the button text to the right when a pushbutton is pressed
	EGDS_BUTTON_PRESSED_TEXT_OFFSET_X,
	//! pixels to move the button text down when a pushbutton is pressed
	EGDS_BUTTON_PRESSED_TEXT_OFFSET_Y,

	//! this value is not used, it only specifies the amount of default sizes
	//! available.
	EGDS_COUNT
};


enum EUI_DEFAULT_TEXT
{
	//! Text for the OK button on a message box
	EGDT_MSG_BOX_OK = 0,
	//! Text for the Cancel button on a message box
	EGDT_MSG_BOX_CANCEL,
	//! Text for the Yes button on a message box
	EGDT_MSG_BOX_YES,
	//! Text for the No button on a message box
	EGDT_MSG_BOX_NO,
	//! Tooltip text for window close button
	EGDT_WINDOW_CLOSE,
	//! Tooltip text for window maximize button
	EGDT_WINDOW_MAXIMIZE,
	//! Tooltip text for window minimize button
	EGDT_WINDOW_MINIMIZE,
	//! Tooltip text for window restore button
	EGDT_WINDOW_RESTORE,

	//! this value is not used, it only specifies the number of default texts
	EGDT_COUNT
};


//! Customizable symbols for UI
enum EUI_DEFAULT_ICON
{
	//! maximize window button
	EGDI_WINDOW_MAXIMIZE = 0,
	//! restore window button
	EGDI_WINDOW_RESTORE,
	//! close window button
	EGDI_WINDOW_CLOSE,
	//! minimize window button
	EGDI_WINDOW_MINIMIZE,
	//! resize icon for bottom right corner of a window
	EGDI_WINDOW_RESIZE,
	//! scroll bar up button
	EGDI_CURSOR_UP,
	//! scroll bar down button
	EGDI_CURSOR_DOWN,
	//! scroll bar left button
	EGDI_CURSOR_LEFT,
	//! scroll bar right button
	EGDI_CURSOR_RIGHT,
	//! icon for menu children
	EGDI_MENU_MORE,
	//! tick for checkbox
	EGDI_CHECK_BOX_CHECKED,
	//! down arrow for dropdown menus
	EGDI_DROP_DOWN,
	//! smaller up arrow
	EGDI_SMALL_CURSOR_UP,
	//! smaller down arrow
	EGDI_SMALL_CURSOR_DOWN,
	//! selection dot in a radio button
	EGDI_RADIO_BUTTON_CHECKED,
	//! << icon indicating there is more content to the left
	EGDI_MORE_LEFT,
	//! >> icon indicating that there is more content to the right
	EGDI_MORE_RIGHT,
	//! icon indicating that there is more content above
	EGDI_MORE_UP,
	//! icon indicating that there is more content below
	EGDI_MORE_DOWN,
	//! plus icon for trees
	EGDI_EXPAND,

	//! minus icon for trees
	EGDI_COLLAPSE,
	//! file icon for file selection
	EGDI_FILE,
	//! folder icon for file selection
	EGDI_DIRECTORY,

	//! value not used, it only specifies the number of icons
	EGDI_COUNT
};


// Customizable fonts
enum EUI_DEFAULT_FONT
{
	//! For static text, edit boxes, lists and most other places
	EGDF_DEFAULT=0,
	//! Font for buttons
	EGDF_BUTTON,
	//! Font for window title bars
	EGDF_WINDOW,
	//! Font for menu items
	EGDF_MENU,
	//! Font for tooltips
	EGDF_TOOLTIP,
	//! this value is not used, it only specifies the amount of default fonts
	//! available.
	EGDF_COUNT
};


//! A skin modifies the look of the UI elements.
class UISkin
{
public:

	//! returns default color
	virtual eastl::array<float, 4> const GetColor(EUI_DEFAULT_COLOR color) const = 0;

	//! sets a default color
	virtual void SetColor(EUI_DEFAULT_COLOR which, eastl::array<float, 4> const newColor) = 0;

	//! returns size for the given size type
	virtual int GetSize(EUI_DEFAULT_SIZE size) const = 0;

	//! Returns a default text.
	/** For example for Message box button captions:
	"OK", "Cancel", "Yes", "No" and so on. */
	virtual const wchar_t* GetDefaultText(EUI_DEFAULT_TEXT text) const = 0;

	//! Sets a default text.
	/** For example for Message box button captions:
	"OK", "Cancel", "Yes", "No" and so on. */
	virtual void SetDefaultText(EUI_DEFAULT_TEXT which, const wchar_t* newText) = 0;

	//! sets a default size
	virtual void SetSize(EUI_DEFAULT_SIZE which, int size) = 0;

	//! returns the default font
	virtual const eastl::shared_ptr<UIFont>& GetFont(EUI_DEFAULT_FONT which=EGDF_DEFAULT) const = 0;

	//! sets a default font
	virtual void SetFont(const eastl::shared_ptr<UIFont>& font, EUI_DEFAULT_FONT which=EGDF_DEFAULT) = 0;

	//! Returns a default icon
	/** Returns the sprite index within the sprite bank */
	virtual unsigned int GetIcon(EUI_DEFAULT_ICON icon) const = 0;

	//! Sets a default icon
	/** Sets the sprite index used for drawing icons like arrows,
	close buttons and ticks in checkboxes
	\param icon: Enum specifying which icon to change
	\param index: The sprite index used to draw this icon */
	virtual void SetIcon(EUI_DEFAULT_ICON icon, unsigned int index) = 0;

	//! draws a standard 3d button pane
	/** Used for drawing for example buttons in normal state.
	It uses the colors EGDC_3D_DARK_SHADOW, EGDC_3D_HIGH_LIGHT, EGDC_3D_SHADOW and
	EGDC_3D_FACE for this. See EUI_DEFAULT_COLOR for details.
	\param element: Pointer to the element which wishes to draw this. This parameter
	is usually not used by UISkin, but can be used for example by more complex
	implementations to find out how to draw the part exactly.
	\param rect: Defining area where to draw.
	\param clip: Clip area. */
	virtual void Draw3DButtonPaneStandard(const eastl::shared_ptr<UIElement>& element,
		const RectangleBase<2, int>& rect, const RectangleBase<2, int>* clip=0) = 0;

	//! draws a pressed 3d button pane
	/** Used for drawing for example buttons in pressed state.
	It uses the colors EGDC_3D_DARK_SHADOW, EGDC_3D_HIGH_LIGHT, EGDC_3D_SHADOW and
	EGDC_3D_FACE for this. See EUI_DEFAULT_COLOR for details.
	\param element: Pointer to the element which wishes to draw this. This parameter
	is usually not used by UISkin, but can be used for example by more complex
	implementations to find out how to draw the part exactly.
	\param rect: Defining area where to draw.
	\param clip: Clip area. */
	virtual void Draw3DButtonPanePressed(const eastl::shared_ptr<UIElement>& element,
		const RectangleBase<2, int>& rect, const RectangleBase<2, int>* clip=0) = 0;

	//! draws a sunken 3d pane
	/** Used for drawing the background of edit, combo or check boxes.
	\param element: Pointer to the element which wishes to draw this. This parameter
	is usually not used by UISkin, but can be used for example by more complex
	implementations to find out how to draw the part exactly.
	\param bgcolor: Background color.
	\param flat: Specifies if the sunken pane should be flat or displayed as sunken
	deep into the ground.
	\param fillBackGround: Specifies if the background should be filled with the background
	color or not be drawn at all.
	\param rect: Defining area where to draw.
	\param clip: Clip area. */
	virtual void Draw3DSunkenPane(const eastl::shared_ptr<UIElement>& element, 
		eastl::array<float, 4> const bgcolor, bool flat, bool fillBackGround, 
		const RectangleBase<2, int>& rect, const RectangleBase<2, int>* clip=0) = 0;

	//! draws a window background
	/** Used for drawing the background of dialogs and windows.
	\param element: Pointer to the element which wishes to draw this. This parameter
	is usually not used by UISkin, but can be used for example by more complex
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
	virtual RectangleBase<2, int> Draw3DWindowBackground(const eastl::shared_ptr<UIElement>& element, 
		bool drawTitleBar, eastl::array<float, 4> const titleBarColor, const RectangleBase<2, int>& rect,
		const RectangleBase<2, int>* clip=0, RectangleBase<2, int>* checkClientArea=0) = 0;

	//! draws a standard 3d menu pane
	/** Used for drawing for menus and context menus.
	It uses the colors EGDC_3D_DARK_SHADOW, EGDC_3D_HIGH_LIGHT, EGDC_3D_SHADOW and
	EGDC_3D_FACE for this. See EUI_DEFAULT_COLOR for details.
	\param element: Pointer to the element which wishes to draw this. This parameter
	is usually not used by UISkin, but can be used for example by more complex
	implementations to find out how to draw the part exactly.
	\param rect: Defining area where to draw.
	\param clip: Clip area. */
	virtual void Draw3DMenuPane(const eastl::shared_ptr<UIElement>& element, const RectangleBase<2, int>& rect, 
		const RectangleBase<2, int>* clip=0) = 0;

	//! draws a standard 3d tool bar
	/** Used for drawing for toolbars and menus.
	\param element: Pointer to the element which wishes to draw this. This parameter
	is usually not used by UISkin, but can be used for example by more complex
	implementations to find out how to draw the part exactly.
	\param rect: Defining area where to draw.
	\param clip: Clip area. */
	virtual void Draw3DToolBar(const eastl::shared_ptr<UIElement>& element, const RectangleBase<2, int>& rect, 
		const RectangleBase<2, int>* clip=0) = 0;

	//! draws a tab button
	/** Used for drawing for tab buttons on top of tabs.
	\param element: Pointer to the element which wishes to draw this. This parameter
	is usually not used by UISkin, but can be used for example by more complex
	implementations to find out how to draw the part exactly.
	\param active: Specifies if the tab is currently active.
	\param rect: Defining area where to draw.
	\param clip: Clip area.
	\param alignment Alignment of UI element. */
	virtual void Draw3DTabButton(const eastl::shared_ptr<UIElement>& element, bool active, 
		const RectangleBase<2, int>& rect, const RectangleBase<2, int>* clip=0, 
		EUI_ALIGNMENT alignment=EUIA_UPPERLEFT) = 0;

	//! draws a tab control body
	/** \param element: Pointer to the element which wishes to draw this. This parameter
	is usually not used by UISkin, but can be used for example by more complex
	implementations to find out how to draw the part exactly.
	\param border: Specifies if the border should be drawn.
	\param background: Specifies if the background should be drawn.
	\param rect: Defining area where to draw.
	\param clip: Clip area.
	\param tabHeight Height of tab.
	\param alignment Alignment of UI element. */
	virtual void Draw3DTabBody(const eastl::shared_ptr<UIElement>& element, bool border, 
		bool background, const RectangleBase<2, int>& rect, const RectangleBase<2, int>* clip=0, 
		int tabHeight=-1, EUI_ALIGNMENT alignment=EUIA_UPPERLEFT ) = 0;

	//! draws an icon, usually from the skin's sprite bank
	/** \param element: Pointer to the element which wishes to draw this icon.
	This parameter is usually not used by UISkin, but can be used for example
	by more complex implementations to find out how to draw the part exactly.
	\param icon: Specifies the icon to be drawn.
	\param position: The position to draw the icon
	\param starttime: The time at the start of the animation
	\param currenttime: The present time, used to calculate the frame number
	\param loop: Whether the animation should loop or not
	\param clip: Clip area. */
	virtual void DrawIcon(const eastl::shared_ptr<UIElement>& element, EUI_DEFAULT_ICON icon,
		const Vector2<int> position, unsigned int starttime=0, unsigned int currenttime=0,
		bool loop=false, const RectangleBase<2, int>* clip=0) = 0;

	//! draws a 2d rectangle.
	/** \param element: Pointer to the element which wishes to draw this icon.
	This parameter is usually not used by UISkin, but can be used for example
	by more complex implementations to find out how to draw the part exactly.
	\param color: Color of the rectangle to draw. The alpha component specifies how
	transparent the rectangle will be.
	\param pos: Position of the rectangle.
	\param clip: Pointer to rectangle against which the rectangle will be clipped.
	If the pointer is null, no clipping will be performed. 
	virtual void Draw2DRectangle(const eastl::shared_ptr<UIElement>& element, 
		const eastl::array<float, 4> const& color, const RectangleBase<2, int>& pos, 
		const RectangleBase<2, int>* clip = 0) = 0;*/

	//! get the type of this skin
	virtual EUI_SKIN_THEME_TYPE GetType() const { return EGSTT_UNKNOWN; }
};

#endif

