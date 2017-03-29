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
	GSTT_WINDOWS_CLASSIC=0,

	//! Like EGST_WINDOWS_CLASSIC, but with metallic shaded windows and buttons
	GSTT_WINDOWS_METALLIC,

	//! Burning's skin
	GSTT_BURNING_SKIN,

	//! An unknown skin, not serializable at present
	GSTT_UNKNOWN,

	//! this value is not used, it only specifies the number of skin types
	GSTT_COUNT
};


//! Enumeration for skin colors
enum UIDefaultColor
{
	//! Dark shadow for three-dimensional display elements.
	GDC_3D_DARK_SHADOW = 0,
	//! Shadow color for three-dimensional display elements (for edges facing away from the light source).
	GDC_3D_SHADOW,
	//! Face color for three-dimensional display elements and for dialog box backgrounds.
	GDC_3D_FACE,
	//! Highlight color for three-dimensional display elements (for edges facing the light source.)
	GDC_3D_HIGH_LIGHT,
	//! Light color for three-dimensional display elements (for edges facing the light source.)
	GDC_3D_LIGHT,
	//! Active window border.
	GDC_ACTIVE_BORDER,
	//! Active window title bar text.
	GDC_ACTIVE_CAPTION,
	//! Background color of multiple document interface (MDI) applications.
	GDC_APP_WORKSPACE,
	//! Text on a button
	GDC_BUTTON_TEXT,
	//! Grayed (disabled) text.
	GDC_GRAY_TEXT,
	//! Item(s) selected in a control.
	GDC_HIGH_LIGHT,
	//! Text of item(s) selected in a control.
	GDC_HIGH_LIGHT_TEXT,
	//! Inactive window border.
	GDC_INACTIVE_BORDER,
	//! Inactive window caption.
	GDC_INACTIVE_CAPTION,
	//! Tool tip text color
	GDC_TOOLTIP,
	//! Tool tip background color
	GDC_TOOLTIP_BACKGROUND,
	//! Scrollbar gray area
	GDC_SCROLLBAR,
	//! Window background
	GDC_WINDOW,
	//! Window symbols like on close buttons, scroll bars and check boxes
	GDC_WINDOW_SYMBOL,
	//! Icons in a list or tree
	GDC_ICON,
	//! Selected icons in a list or tree
	GDC_ICON_HIGH_LIGHT,
	//! Grayed (disabled) window symbols like on close buttons, scroll bars and check boxes
	GDC_GRAY_WINDOW_SYMBOL,
	//! Window background for editable field (UIEditBox, checkbox-field)
	GDC_EDITABLE,
	//! Grayed (disabled) window background for editable field (UIEditBox, checkbox-field)
	GDC_GRAY_EDITABLE,
	//! Show focus of window background for editable field (UIEditBox or when checkbox-field is pressed)
	GDC_Focused_EDITABLE,

	//! this value is not used, it only specifies the amount of default colors
	//! available.
	GDC_COUNT
};

//! Enumeration for default sizes.
enum UIDefaultSize
{
	//! default with / height of scrollbar
	GDS_SCROLLBAR_SIZE = 0,
	//! height of menu
	GDS_MENU_HEIGHT,
	//! width of a window button
	GDS_WINDOW_BUTTON_WIDTH,
	//! width of a checkbox check
	GDS_CHECK_BOX_WIDTH,
	//! \deprecated This may be removed by Irrlicht 1.9
	GDS_MESSAGE_BOX_WIDTH,
	//! \deprecated This may be removed by Irrlicht 1.9
	GDS_MESSAGE_BOX_HEIGHT,
	//! width of a default button
	GDS_BUTTON_WIDTH,
	//! height of a default button
	GDS_BUTTON_HEIGHT,
	//! distance for text from background
	GDS_TEXT_DISTANCE_X,
	//! distance for text from background
	GDS_TEXT_DISTANCE_Y,
	//! distance for text in the title bar, from the left of the window rect
	GDS_TITLEBARTEXT_DISTANCE_X,
	//! distance for text in the title bar, from the top of the window rect
	GDS_TITLEBARTEXT_DISTANCE_Y,
	//! free space in a messagebox between borders and contents on all sides
	GDS_MESSAGE_BOX_GAP_SPACE,
	//! minimal space to reserve for messagebox text-width
	GDS_MESSAGE_BOX_MIN_TEXT_WIDTH,
	//! maximal space to reserve for messagebox text-width
	GDS_MESSAGE_BOX_MAX_TEXT_WIDTH,
	//! minimal space to reserve for messagebox text-height
	GDS_MESSAGE_BOX_MIN_TEXT_HEIGHT,
	//! maximal space to reserve for messagebox text-height
	GDS_MESSAGE_BOX_MAX_TEXT_HEIGHT,
	//! pixels to move the button image to the right when a pushbutton is pressed
	GDS_BUTTON_PRESSED_IMAGE_OFFSET_X,
	//! pixels to move the button image down when a pushbutton is pressed
	GDS_BUTTON_PRESSED_IMAGE_OFFSET_Y,
	//! pixels to move the button text to the right when a pushbutton is pressed
	GDS_BUTTON_PRESSED_TEXT_OFFSET_X,
	//! pixels to move the button text down when a pushbutton is pressed
	GDS_BUTTON_PRESSED_TEXT_OFFSET_Y,

	//! this value is not used, it only specifies the amount of default sizes
	//! available.
	GDS_COUNT
};


enum UIDefaultText
{
	//! Text for the OK button on a message box
	GDT_MSG_BOX_OK = 0,
	//! Text for the Cancel button on a message box
	GDT_MSG_BOX_CANCEL,
	//! Text for the Yes button on a message box
	GDT_MSG_BOX_YES,
	//! Text for the No button on a message box
	GDT_MSG_BOX_NO,
	//! Tooltip text for window close button
	GDT_WINDOW_CLOSE,
	//! Tooltip text for window maximize button
	GDT_WINDOW_MAXIMIZE,
	//! Tooltip text for window minimize button
	GDT_WINDOW_MINIMIZE,
	//! Tooltip text for window restore button
	GDT_WINDOW_RESTORE,

	//! this value is not used, it only specifies the number of default texts
	GDT_COUNT
};


//! Customizable symbols for UI
enum UIDefaultIcon
{
	//! maximize window button
	GDI_WINDOW_MAXIMIZE = 0,
	//! restore window button
	GDI_WINDOW_RESTORE,
	//! close window button
	GDI_WINDOW_CLOSE,
	//! minimize window button
	GDI_WINDOW_MINIMIZE,
	//! resize icon for bottom right corner of a window
	GDI_WINDOW_RESIZE,
	//! scroll bar up button
	GDI_CURSOR_UP,
	//! scroll bar down button
	GDI_CURSOR_DOWN,
	//! scroll bar left button
	GDI_CURSOR_LEFT,
	//! scroll bar right button
	GDI_CURSOR_RIGHT,
	//! icon for menu children
	GDI_MENU_MORE,
	//! tick for checkbox
	GDI_CHECK_BOX_CHECKED,
	//! down arrow for dropdown menus
	GDI_DROP_DOWN,
	//! smaller up arrow
	GDI_SMALL_CURSOR_UP,
	//! smaller down arrow
	GDI_SMALL_CURSOR_DOWN,
	//! selection dot in a radio button
	GDI_RADIO_BUTTON_CHECKED,
	//! << icon indicating there is more content to the left
	GDI_MORE_LEFT,
	//! >> icon indicating that there is more content to the right
	GDI_MORE_RIGHT,
	//! icon indicating that there is more content above
	GDI_MORE_UP,
	//! icon indicating that there is more content below
	GDI_MORE_DOWN,
	//! plus icon for trees
	GDI_EXPAND,

	//! minus icon for trees
	GDI_COLLAPSE,
	//! file icon for file selection
	GDI_FILE,
	//! folder icon for file selection
	GDI_DIRECTORY,

	//! value not used, it only specifies the number of icons
	GDI_COUNT
};


// Customizable fonts
enum UIDefaultFont
{
	//! For static text, edit boxes, lists and most other places
	GDF_DEFAULT=0,
	//! Font for buttons
	GDF_BUTTON,
	//! Font for window title bars
	GDF_WINDOW,
	//! Font for menu items
	GDF_MENU,
	//! Font for tooltips
	GDF_TOOLTIP,
	//! this value is not used, it only specifies the amount of default fonts
	//! available.
	GDF_COUNT
};


//! A skin modifies the look of the UI elements.
class UISkin
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
	virtual const eastl::shared_ptr<UIFont>& GetFont(UIDefaultFont which=GDF_DEFAULT) const = 0;

	//! sets a default font
	virtual void SetFont(const eastl::shared_ptr<UIFont>& font, UIDefaultFont which=GDF_DEFAULT) = 0;

	//! Returns a default icon
	/** Returns the sprite index within the sprite bank */
	virtual unsigned int GetIcon(UIDefaultIcon icon) const = 0;

	//! Sets a default icon
	/** Sets the sprite index used for drawing icons like arrows,
	close buttons and ticks in checkboxes
	\param icon: Enum specifying which icon to change
	\param index: The sprite index used to draw this icon */
	virtual void SetIcon(UIDefaultIcon icon, unsigned int index) = 0;

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
		UIAlignment alignment=UIA_UPPERLEFT) = 0;

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
		int tabHeight=-1, UIAlignment alignment=UIA_UPPERLEFT ) = 0;

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
	virtual void DrawIcon(const eastl::shared_ptr<UIElement>& element, UIDefaultIcon icon,
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
	virtual UISkinThemeType GetType() const { return GSTT_UNKNOWN; }
};

#endif

