// Copyright (C) 2002-2012 Nikolaus Gebhardt
// This file is part of the "Irrlicht Engine".
// For conditions of distribution and use, see copyright notice in irrlicht.h

#ifndef UIEDITBOX_H
#define UIEDITBOX_H

#include "UIElement.h"
#include "UIFont.h"

#include "Graphic/Effect/Texture2Effect.h"
#include "Graphic/Effect/BasicEffect.h"

#include "Graphic/Scene/Hierarchy/Visual.h"

//! Single line edit box for editing simple text.
/** \par This element can create the following events of type EGUI_EVENT_TYPE:
\li UIEDITBOX_ENTER
\li UIEDITBOX_CHANGED
\li UIEDITBOX_MARKING_CHANGED
*/
class BaseUIEditBox : public BaseUIElement
{
public:

	//! constructor
	BaseUIEditBox(BaseUI* ui, int id, RectangleBase<2, int> rectangle)
		: BaseUIElement(UIET_EDIT_BOX, id, rectangle) {}

	//! Initialize editbox
	virtual void OnInit() = 0;

	//! Sets another skin independent font.
	/** If this is set to zero, the button uses the font of the skin.
	\param font: New font to set. */
	virtual void SetOverrideFont(const eastl::shared_ptr<BaseUIFont>& font = 0) = 0;

	//! Gets the override font (if any)
	/** \return The override font (may be 0) */
	virtual const eastl::shared_ptr<BaseUIFont>& GetOverrideFont() const = 0;

	//! Get the font which is used right now for drawing
	/** Currently this is the override font when one is set and the
	font of the active skin otherwise */
	virtual const eastl::shared_ptr<BaseUIFont>& GetActiveFont() const = 0;

	//! Sets another color for the text.
	/** If set, the edit box does not use the EGDC_BUTTON_TEXT color defined
	in the skin, but the set color instead. You don't need to call
	IGUIEditBox::enableOverrrideColor(true) after this, this is done
	by this function.
	If you set a color, and you want the text displayed with the color
	of the skin again, call IGUIEditBox::enableOverrideColor(false);
	\param color: New color of the text. */
	virtual void SetOverrideColor(eastl::array<float, 4> color) = 0;

	//! Gets the override color
	virtual eastl::array<float, 4> GetOverrideColor() const = 0;

	//! Sets if the text should use the override color or the color in the gui skin.
	/** \param enable: If set to true, the override color, which can be set
	with IGUIEditBox::setOverrideColor is used, otherwise the
	EGDC_BUTTON_TEXT color of the skin. */
	virtual void EnableOverrideColor(bool enable) = 0;

	//! Checks if an override color is enabled
	/** \return true if the override color is enabled, false otherwise */
	virtual bool IsOverrideColorEnabled(void) const = 0;

	//! Sets whether to draw the background
	virtual void SetDrawBackground(bool draw) = 0;

	//! Turns the border on or off
	/** \param border: true if you want the border to be drawn, false if not */
	virtual void SetDrawBorder(bool border) = 0;

	//! Sets text justification mode
	/** \param horizontal: EGUIA_UPPERLEFT for left justified (default),
	EGUIA_LOWERRIGHT for right justified, or EGUIA_CENTER for centered text.
	\param vertical: EGUIA_UPPERLEFT to align with top edge,
	EGUIA_LOWERRIGHT for bottom edge, or EGUIA_CENTER for centered text (default). */
	virtual void SetTextAlignment(UIAlignment horizontal, UIAlignment vertical) = 0;

	//! Enables or disables word wrap.
	/** \param enable: If set to true, words going over one line are
	broken to the next line. */
	virtual void SetWordWrap(bool enable) = 0;

	//! Checks if word wrap is enabled
	/** \return true if word wrap is enabled, false otherwise */
	virtual bool IsWordWrapEnabled() const = 0;

	//! Enables or disables newlines.
	/** \param enable: If set to true, the EGET_EDITBOX_ENTER event will not be fired,
	instead a newline character will be inserted. */
	virtual void SetMultiLine(bool enable) = 0;

	//! Checks if multi line editing is enabled
	/** \return true if multi-line is enabled, false otherwise */
	virtual bool IsMultiLineEnabled() const = 0;

	//! Enables or disables automatic scrolling with cursor position
	/** \param enable: If set to true, the text will move around with the cursor position */
	virtual void SetAutoScroll(bool enable) = 0;

	//! Checks to see if automatic scrolling is enabled
	/** \return true if automatic scrolling is enabled, false if not */
	virtual bool IsAutoScrollEnabled() const = 0;

	//! Sets whether the edit box is a password box. Setting this to true will
	/** disable MultiLine, WordWrap and the ability to copy with ctrl+c or ctrl+x
	\param passwordBox: true to enable password, false to disable
	\param passwordChar: the character that is displayed instead of letters */
	virtual void SetPasswordBox(bool passwordBox, wchar_t passwordChar = L'*') = 0;

	//! Returns true if the edit box is currently a password box.
	virtual bool IsPasswordBox() const = 0;

	//! Gets the size area of the text in the edit box
	/** \return The size in pixels of the text */
	virtual Vector2<int> GetTextDimension() = 0;

	//! Sets the maximum amount of characters which may be entered in the box.
	/** \param max: Maximum amount of characters. If 0, the character amount is
	infinity. */
	virtual void SetMax(unsigned int max) = 0;

	//! Returns maximum amount of characters, previously set by setMax();
	virtual unsigned int GetMax() const = 0;
};

class UIEditBox : public BaseUIEditBox
{
public:

	//! constructor
	UIEditBox(const wchar_t* text, bool border, BaseUI* ui, int id, RectangleBase<2, int> rectangle);

	//! destructor
	virtual ~UIEditBox();

	//! initialize editbox
	virtual void OnInit();

	//! Sets another skin independent font.
	virtual void SetOverrideFont(const eastl::shared_ptr<BaseUIFont>& font = 0);

	//! Gets the override font (if any)
	virtual const eastl::shared_ptr<BaseUIFont>& GetOverrideFont() const;

	//! Get the font which is used right now for drawing
	virtual const eastl::shared_ptr<BaseUIFont>& GetActiveFont() const;

	//! Sets another color for the text.
	virtual void SetOverrideColor(eastl::array<float, 4> color);

	//! Gets the override color
	virtual eastl::array<float, 4> GetOverrideColor() const;

	//! Sets if the text should use the override color or the color in the gui skin.
	virtual void EnableOverrideColor(bool enable);

	//! Checks if an override color is enabled
	virtual bool IsOverrideColorEnabled(void) const;

	//! Sets whether to draw the background
	virtual void SetDrawBackground(bool draw);

	//! Turns the border on or off
	virtual void SetDrawBorder(bool border);

	//! Sets text justification mode
	virtual void SetTextAlignment(UIAlignment horizontal, UIAlignment vertical);

	//! Enables or disables word wrap.
	virtual void SetWordWrap(bool enable);

	//! Checks if word wrap is enabled
	virtual bool IsWordWrapEnabled() const;

	//! Enables or disables newlines.
	virtual void SetMultiLine(bool enable);

	//! Checks if multi line editing is enabled
	virtual bool IsMultiLineEnabled() const;

	//! Enables or disables automatic scrolling with cursor position
	virtual void SetAutoScroll(bool enable);

	//! Checks to see if automatic scrolling is enabled
	virtual bool IsAutoScrollEnabled() const;

	//! Sets whether the edit box is a password box. Setting this to true will
	virtual void SetPasswordBox(bool passwordBox, wchar_t passwordChar = L'*');

	//! Returns true if the edit box is currently a password box.
	virtual bool IsPasswordBox() const;

	//! Gets the size area of the text in the edit box
	virtual Vector2<int> GetTextDimension();

	//! Sets the maximum amount of characters which may be entered in the box.
	virtual void SetMax(unsigned int max);

	//! Returns maximum amount of characters, previously set by setMax();
	virtual unsigned int GetMax() const;

	//! called if an event happened.
	virtual bool OnEvent(const Event& event);

	//! draws the element and its children
	virtual void Draw();

	//! Sets the new caption of this element.
	virtual void SetText(const wchar_t* text);

	//! Updates the absolute position, splits text if required
	virtual void UpdateAbsolutePosition();

protected:
	//! Breaks the single text line.
	void BreakText();
	//! sets the area of the given line
	void SetTextRect(int line);
	//! returns the line number that the cursor is on
	int GetLineFromPos(int pos);
	//! adds a letter to the edit box
	void InputChar(wchar_t c);
	//! calculates the current scroll position
	void CalculateScrollPos();
	//! calculated the FrameRect
	void CalculateFrameRect();
	//! send some ui event to parent
	void SendUIEvent(UIEventType type);
	//! set text markers
	void SetTextMarkers(int begin, int end);

	bool ProcessKey(const Event& event);
	bool ProcessMouse(const Event& event);
	int GetCursorPos(int x, int y);

	bool mMouseMarking;
	bool mBorder;
	bool mBackground;
	bool mOverrideColorEnabled;
	int mMarkBegin;
	int mMarkEnd;

	eastl::array<float, 4> mOverrideColor;
	eastl::shared_ptr<BaseUIFont> mOverrideFont, mLastBreakFont;

	unsigned int mBlinkStartTime;
	int mCursorPos;
	int mHScrollPos, mVScrollPos; // scroll position in characters
	unsigned int mMax;

	bool mWordWrap, mMultiLine, mAutoScroll, mPasswordBox;
	wchar_t mPasswordChar;
	UIAlignment mHAlign, mVAlign;

	eastl::vector<eastl::wstring> mBrokenText;
	eastl::vector<int> mBrokenTextPositions;

	RectangleBase<2, int> mCurrentTextRect, mFrameRect;

private:

	BaseUI* mUI;

	eastl::shared_ptr<Visual> mVisual;
	eastl::shared_ptr<Texture2Effect> mEffect;

	eastl::shared_ptr<Visual> mVisualHighlight;
	eastl::shared_ptr<BasicEffect> mEffectHighlight;
};

#endif

