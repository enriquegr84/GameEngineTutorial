// Copyright (C) 2002-2012 Nikolaus Gebhardt
// This file is part of the "Irrlicht Engine".
// For conditions of distribution and use, see copyright notice in irrlicht.h

#ifndef UIFONT_H
#define UIFONT_H

#include "UIElement.h"

#include "Graphic/Renderer/Renderer.h"

//! An enum for the different types of UI font.
enum UIFontType
{
	//! Bitmap fonts loaded from an XML file or a texture.
	FT_BITMAP = 0,

	//! Scalable vector fonts loaded from an XML file.
	/** These fonts reside in system memory and use no video memory
	until they are displayed. These are slower than bitmap fonts
	but can be easily scaled and rotated. */
	FT_VECTOR,

	//! A font which uses a the native API provided by the operating system.
	/** Currently not used. */
	FT_OS,

	//! An external font type provided by the user.
	FT_CUSTOM
};

class BaseUISpriteBank;
class BaseUI;

//! Font interface.
class BaseUIFont
{
public:

	//! Draws some text and clips it to the specified rectangle if wanted.
	/** \param text: Text to draw
	\param position: Rectangle specifying position where to draw the text.
	\param color: Color of the text
	\param hcenter: Specifies if the text should be centered horizontally into the rectangle.
	\param vcenter: Specifies if the text should be centered vertically into the rectangle.
	\param clip: Optional pointer to a rectangle against which the text will be clipped.
	If the pointer is null, no clipping will be done. */
	virtual void Draw(const eastl::wstring& text, const RectangleShape<2, int>& position,
		eastl::array<float, 4> const& color, bool hcenter=false, bool vcenter=false, 
		const RectangleShape<2, int>* clip=0) = 0;

	// Font widht and height info
	virtual Vector2<int> GetDimension(eastl::wstring const& message) const = 0;

	//! Returns the type of this font
	virtual UIFontType GetType() const { return FT_CUSTOM; }
};

//! Font interface.
class BaseUIFontBitmap : public BaseUIFont
{
public:

	//! Returns the type of this font
	virtual UIFontType GetType() const { return FT_BITMAP; }

	//! returns the parsed Symbol Information
	virtual eastl::shared_ptr<BaseUISpriteBank> GetSpriteBank() const = 0;
};


class UIFont : public BaseUIFontBitmap
{
public:

	//! constructor
	UIFont(BaseUI* ui, eastl::wstring fileName);
	UIFont(BaseUI* ui, eastl::wstring fileName, const eastl::shared_ptr<Font> font);

	//! destructor
	virtual ~UIFont();

	//! loads a font from a texture file
	bool Load(const eastl::wstring& filename);

	//! draws an text and clips it to the specified rectangle if wanted
	virtual void Draw(const eastl::wstring& text, const RectangleShape<2, int>& position,
		eastl::array<float, 4> const& color, bool hcenter = false, bool vcenter = false, 
		const RectangleShape<2, int>* clip = 0);

	//! returns the parsed Symbol Information
	virtual eastl::shared_ptr<BaseUISpriteBank> GetSpriteBank() const { return mSpriteBank; }

	// Font widht and height info
	virtual Vector2<int> GetDimension(eastl::wstring const& message) const;

	//! Returns the type of this font
	virtual UIFontType GetType() const { return FT_BITMAP; }

private:

	BaseUI* mUI;

	eastl::shared_ptr<Font> mFont;
	eastl::shared_ptr<BaseUISpriteBank> mSpriteBank;
};

#endif