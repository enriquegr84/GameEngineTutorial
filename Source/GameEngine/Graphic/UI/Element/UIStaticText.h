// Copyright (C) 2002-2012 Nikolaus Gebhardt
// This file is part of the "Irrlicht Engine".
// For conditions of distribution and use, see copyright notice in irrlicht.h

#ifndef UISTATICTEXT_H
#define UISTATICTEXT_H

#include "UIElement.h"

#include "Core/Utility/EnrichedString.h"

#include "Graphic/Scene/Hierarchy/Visual.h"

class BaseUIFont;

//! Multi or single line text label.
class BaseUIStaticText : public BaseUIElement
{
public:

	//! constructor
	BaseUIStaticText(int id, RectangleShape<2, int> rectangle)
		: BaseUIElement(UIET_STATIC_TEXT, id, rectangle) {}

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

	//! Sets another color for the text.
	/** If set, the static text does not use the EGDC_BUTTON_TEXT color defined
	in the skin, but the set color instead. You don't need to call
	IGUIStaticText::enableOverrrideColor(true) after this, this is done
	by this function.
	If you set a color, and you want the text displayed with the color
	of the skin again, call IGUIStaticText::enableOverrideColor(false);
	\param color: New color of the text. */
	virtual void SetOverrideColor(SColor color) = 0;

	//! Gets the override color
	/** \return: The override color */
	virtual SColor GetOverrideColor(void) const = 0;

	//! Sets if the static text should use the overide color or the color in the gui skin.
	/** \param enable: If set to true, the override color, which can be set
	with IGUIStaticText::setOverrideColor is used, otherwise the
	EGDC_BUTTON_TEXT color of the skin. */
	virtual void EnableOverrideColor(bool enable) = 0;

	//! Checks if an override color is enabled
	/** \return true if the override color is enabled, false otherwise */
	virtual bool IsOverrideColorEnabled(void) const = 0;

	//! Sets another color for the background.
	virtual void SetBackgroundColor(SColor color) = 0;

	//! Sets whether to draw the background
	virtual void SetDrawBackground(bool draw) = 0;

	//! Checks if background drawing is enabled
	/** \return true if background drawing is enabled, false otherwise */
	virtual bool IsDrawBackgroundEnabled() const = 0;

	//! Gets the background color
	/** \return: The background color */
	virtual SColor GetBackgroundColor() const = 0;

	//! Sets whether to draw the border
	virtual void SetDrawBorder(bool draw) = 0;

	//! Checks if border drawing is enabled
	/** \return true if border drawing is enabled, false otherwise */
	virtual bool IsDrawBorderEnabled() const = 0;

	//! Sets text justification mode
	/** \param horizontal: EGUIA_UPPERLEFT for left justified (default),
	EGUIA_LOWEERRIGHT for right justified, or EGUIA_CENTER for centered text.
	\param vertical: EGUIA_UPPERLEFT to align with top edge,
	EGUIA_LOWEERRIGHT for bottom edge, or EGUIA_CENTER for centered text (default). */
	virtual void SetTextAlignment(UIAlignment horizontal, UIAlignment vertical) = 0;

	//! Enables or disables word wrap for using the static text as multiline text control.
	/** \param enable: If set to true, words going over one line are
	broken on to the next line. */
	virtual void SetWordWrap(bool enable) = 0;

	//! Checks if word wrap is enabled
	/** \return true if word wrap is enabled, false otherwise */
	virtual bool IsWordWrapEnabled(void) const = 0;

	//! Returns the height of the text in pixels when it is drawn.
	/** This is useful for adjusting the layout of gui elements based on the height
	of the multiline text in this element.
	\return Height of text in pixels. */
	virtual int GetTextHeight() const = 0;

	//! Returns the width of the current text, in the current font
	/** If the text is broken, this returns the width of the widest line
	\return The width of the text, or the widest broken line. */
	virtual int GetTextWidth(void) const = 0;

	//! Set whether the text in this label should be clipped if it goes outside bounds
	virtual void SetTextRestrainedInside(bool restrainedInside) = 0;

	//! Checks if the text in this label should be clipped if it goes outside bounds
	virtual bool IsTextRestrainedInside() const = 0;

	//! Set whether the string should be interpreted as right-to-left (RTL) text
	/** \note This component does not implement the Unicode bidi standard, the
	text of the component should be already RTL if you call this. The
	main difference when RTL is enabled is that the linebreaks for multiline
	elements are performed starting from the end.
	*/
	virtual void SetRightToLeft(bool rtl) = 0;

	//! Checks whether the text in this element should be interpreted as right-to-left
	virtual bool IsRightToLeft() const = 0;
};

class UIStaticText : public BaseUIStaticText
{
public:

	//! constructor
	UIStaticText(BaseUI* ui, int id, const wchar_t* text,
		bool border, const RectangleShape<2, int>& rectangle, bool background = false);

	//! destructor
	virtual ~UIStaticText();

	//! draws the element and its children
	virtual void Draw( );

	//! Sets another skin independent font.
	virtual void SetOverrideFont(const eastl::shared_ptr<BaseUIFont>& font=0);

	//! Gets the override font (if any)
	virtual const eastl::shared_ptr<BaseUIFont>& GetOverrideFont() const;

	//! Get the font which is used right now for drawing
	virtual eastl::shared_ptr<BaseUIFont> GetActiveFont() const;

	//! Sets another color for the text.
	virtual void SetOverrideColor(SColor color);

	//! Sets another color for the background.
	virtual void SetBackgroundColor(SColor color);

	//! Sets whether to draw the background
	virtual void SetDrawBackground(bool draw);

	//! Gets the background color
	virtual SColor GetBackgroundColor() const;

	//! Checks if background drawing is enabled
	virtual bool IsDrawBackgroundEnabled() const;

	//! Sets whether to draw the border
	virtual void SetDrawBorder(bool draw);

	//! Checks if border drawing is enabled
	virtual bool IsDrawBorderEnabled() const;

	//! Sets alignment mode for text
	virtual void SetTextAlignment(UIAlignment horizontal, UIAlignment vertical);

	//! Gets the override color
	virtual SColor GetOverrideColor() const;

	//! Sets if the static text should use the overide color or the
	//! color in the gui skin.
	virtual void EnableOverrideColor(bool enable);

	//! Checks if an override color is enabled
	virtual bool IsOverrideColorEnabled() const;

	//! Set whether the text in this label should be clipped if it goes outside bounds
	virtual void SetTextRestrainedInside(bool restrainedInside);

	//! Checks if the text in this label should be clipped if it goes outside bounds
	virtual bool IsTextRestrainedInside() const;

	//! Enables or disables word wrap for using the static text as
	//! multiline text control.
	virtual void SetWordWrap(bool enable);

	//! Checks if word wrap is enabled
	virtual bool IsWordWrapEnabled() const;

    //! Sets the new caption of this element.
    virtual void SetText(const wchar_t* text);

	//! Returns the height of the text in pixels when it is drawn.
	virtual int GetTextHeight() const;

	//! Returns the width of the current text, in the current font
	virtual int GetTextWidth() const;

	//! Updates the absolute position, splits text if word wrap is enabled
	virtual void UpdateAbsoluteTransformation();

	//! Set whether the string should be interpreted as right-to-left (RTL) text
	/** \note This component does not implement the Unicode bidi standard, the
	text of the component should be already RTL if you call this. The
	main difference when RTL is enabled is that the linebreaks for multiline
	elements are performed starting from the end.
	*/
	virtual void SetRightToLeft(bool rtl);

	//! Checks if the text should be interpreted as right-to-left text
	virtual bool IsRightToLeft() const;

private:

	//! Update the single text line.
	void UpdateText();

	UIAlignment mHAlign, mVAlign;
	bool mBorder;
	bool mOverrideColorEnabled;
	bool mOverrideBGColorEnabled;
	bool mWordWrap;
	bool mBackground;
	bool mRestrainTextInside;
	bool mRightToLeft;

	SColor mOverrideColor, mBGColor;
	eastl::shared_ptr<Visual> mVisual;
	eastl::shared_ptr<VisualEffect> mEffect;
	eastl::shared_ptr<BaseUIFont> mOverrideFont;
	eastl::shared_ptr<BaseUIFont> mLastBreakFont; // stored because: if skin changes, line break must be recalculated.

	BaseUI* mUI;

	eastl::vector<EnrichedString> mBrokenText;
};

#endif

