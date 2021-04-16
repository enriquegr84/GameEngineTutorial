// Copyright (C) 2002-2012 Nikolaus Gebhardt
// This file is part of the "Irrlicht Engine".
// For conditions of distribution and use, see copyright notice in irrlicht.h

#include "UISkin.h"

#include "Graphic/Renderer/Renderer.h"


UISkin::UISkin(BaseUI* ui, UISkinThemeType type)
	: mSpriteBank(0), mType(type), mUI(ui)
{
	if ((mType == STT_WINDOWS_CLASSIC) || (mType == STT_WINDOWS_METALLIC))
	{
        mColors[DC_3D_DARK_SHADOW] = SColor(101, 50, 50, 50);
        mColors[DC_3D_SHADOW] = SColor(101, 130, 130, 130);
        mColors[DC_3D_FACE] = SColor(220, 100, 100, 100);
        mColors[DC_3D_HIGH_LIGHT] = SColor(101, 255, 255, 255);
        mColors[DC_3D_LIGHT] = SColor(101, 210, 210, 210);
        mColors[DC_ACTIVE_BORDER] = SColor(101, 16, 14, 115);
        mColors[DC_ACTIVE_CAPTION] = SColor(255, 255, 255, 255);
        mColors[DC_APP_WORKSPACE] = SColor(101, 100, 100, 100);
        mColors[DC_BUTTON_TEXT] = SColor(240, 10, 10, 10);
        mColors[DC_GRAY_TEXT] = SColor(240, 130, 130, 130);
        mColors[DC_HIGH_LIGHT] = SColor(101, 8, 36, 107);
        mColors[DC_HIGH_LIGHT_TEXT] = SColor(240, 255, 255, 255);
        mColors[DC_INACTIVE_BORDER] = SColor(101, 165, 165, 165);
        mColors[DC_INACTIVE_CAPTION] = SColor(255, 30, 30, 30);
        mColors[DC_TOOLTIP] = SColor(200, 0, 0, 0);
        mColors[DC_TOOLTIP_BACKGROUND] = SColor(200, 255, 255, 225);
        mColors[DC_SCROLLBAR] = SColor(101, 230, 230, 230);
        mColors[DC_WINDOW] = SColor(101, 255, 255, 255);
        mColors[DC_WINDOW_SYMBOL] = SColor(200, 10, 10, 10);
        mColors[DC_ICON] = SColor(200, 255, 255, 255);
        mColors[DC_ICON_HIGH_LIGHT] = SColor(200, 8, 36, 107);
        mColors[DC_GRAY_WINDOW_SYMBOL] = SColor(240, 100, 100, 100);
        mColors[DC_EDITABLE] = SColor(255, 255, 255, 255);
        mColors[DC_GRAY_EDITABLE] = SColor(255, 120, 120, 120);
        mColors[DC_FOCUSED_EDITABLE] = SColor(255, 240, 240, 255);

		mSizes[DS_SCROLLBAR_SIZE] = 14;
		mSizes[DS_MENU_HEIGHT] = 30;
		mSizes[DS_WINDOW_BUTTON_WIDTH] = 15;
		mSizes[DS_CHECK_BOX_WIDTH] = 18;
		mSizes[DS_MESSAGE_BOX_WIDTH] = 500;
		mSizes[DS_MESSAGE_BOX_HEIGHT] = 200;
		mSizes[DS_BUTTON_WIDTH] = 80;
		mSizes[DS_BUTTON_HEIGHT] = 30;

		mSizes[DS_TEXT_DISTANCE_X] = 2;
		mSizes[DS_TEXT_DISTANCE_Y] = 0;

		mSizes[DS_TITLEBARTEXT_DISTANCE_X] = 2;
		mSizes[DS_TITLEBARTEXT_DISTANCE_Y] = 0;
	}
	else
	{
        //0x80a6a8af
        mColors[DC_3D_DARK_SHADOW] = 0x60767982;
        //Colors[DC_3D_FACE]			=	0xc0c9ccd4;		// tab background
        mColors[DC_3D_FACE] = 0xc0cbd2d9;		// tab background
        mColors[DC_3D_SHADOW] = 0x50e4e8f1;		// tab background, and left-top highlight
        mColors[DC_3D_HIGH_LIGHT] = 0x40c7ccdc;
        mColors[DC_3D_LIGHT] = 0x802e313a;
        mColors[DC_ACTIVE_BORDER] = 0x80404040;		// window title
        mColors[DC_ACTIVE_CAPTION] = 0xffd0d0d0;
        mColors[DC_APP_WORKSPACE] = 0xc0646464;		// unused
        mColors[DC_BUTTON_TEXT] = 0xd0161616;
        mColors[DC_GRAY_TEXT] = 0x3c141414;
        mColors[DC_HIGH_LIGHT] = 0x6c606060;
        mColors[DC_HIGH_LIGHT_TEXT] = 0xd0e0e0e0;
        mColors[DC_INACTIVE_BORDER] = 0xf0a5a5a5;
        mColors[DC_INACTIVE_CAPTION] = 0xffd2d2d2;
        mColors[DC_TOOLTIP] = 0xf00f2033;
        mColors[DC_TOOLTIP_BACKGROUND] = 0xc0cbd2d9;
        mColors[DC_SCROLLBAR] = 0xf0e0e0e0;
        mColors[DC_WINDOW] = 0xf0f0f0f0;
        mColors[DC_WINDOW_SYMBOL] = 0xd0161616;
        mColors[DC_ICON] = 0xd0161616;
        mColors[DC_ICON_HIGH_LIGHT] = 0xd0606060;
        mColors[DC_GRAY_WINDOW_SYMBOL] = 0x3c101010;
        mColors[DC_EDITABLE] = 0xf0ffffff;
        mColors[DC_GRAY_EDITABLE] = 0xf0cccccc;
        mColors[DC_FOCUSED_EDITABLE] = 0xf0fffff0;

		mSizes[DS_SCROLLBAR_SIZE] = 14;
		mSizes[DS_MENU_HEIGHT] = 48;
		mSizes[DS_WINDOW_BUTTON_WIDTH] = 15;
		mSizes[DS_CHECK_BOX_WIDTH] = 18;
		mSizes[DS_MESSAGE_BOX_WIDTH] = 500;
		mSizes[DS_MESSAGE_BOX_HEIGHT] = 200;
		mSizes[DS_BUTTON_WIDTH] = 80;
		mSizes[DS_BUTTON_HEIGHT] = 30;

		mSizes[DS_TEXT_DISTANCE_X] = 3;
		mSizes[DS_TEXT_DISTANCE_Y] = 2;

		mSizes[DS_TITLEBARTEXT_DISTANCE_X] = 3;
		mSizes[DS_TITLEBARTEXT_DISTANCE_Y] = 2;
	}

	mSizes[DS_MESSAGE_BOX_GAP_SPACE] = 15;
	mSizes[DS_MESSAGE_BOX_MIN_TEXT_WIDTH] = 0;
	mSizes[DS_MESSAGE_BOX_MAX_TEXT_WIDTH] = 500;
	mSizes[DS_MESSAGE_BOX_MIN_TEXT_HEIGHT] = 0;
	mSizes[DS_MESSAGE_BOX_MAX_TEXT_HEIGHT] = 99999;

	mSizes[DS_BUTTON_PRESSED_IMAGE_OFFSET_X] = 1;
	mSizes[DS_BUTTON_PRESSED_IMAGE_OFFSET_Y] = 1;
	mSizes[DS_BUTTON_PRESSED_TEXT_OFFSET_X] = 0;
	mSizes[DS_BUTTON_PRESSED_TEXT_OFFSET_Y] = 2;

	mTexts[DT_MSG_BOX_OK] = L"OK";
	mTexts[DT_MSG_BOX_CANCEL] = L"Cancel";
	mTexts[DT_MSG_BOX_YES] = L"Yes";
	mTexts[DT_MSG_BOX_NO] = L"No";
	mTexts[DT_WINDOW_CLOSE] = L"Close";
	mTexts[DT_WINDOW_RESTORE] = L"Restore";
	mTexts[DT_WINDOW_MINIMIZE] = L"Minimize";
	mTexts[DT_WINDOW_MAXIMIZE] = L"Maximize";
	mTexts[DT_WINDOW_COLLAPSE] = L"Collapse";
	mTexts[DT_WINDOW_EXPAND] = L"Expand";

	mIcons[DI_WINDOW_MAXIMIZE] = L"Art/UserControl/appbar.window.maximize.png";
	mIcons[DI_WINDOW_RESTORE] = L"Art/UserControl/appbar.window.restore.png";
	mIcons[DI_WINDOW_CLOSE] = L"Art/UserControl/appbar.close.png";
	mIcons[DI_WINDOW_MINIMIZE] = L"Art/UserControl/appbar.window.minimize.png";
	mIcons[DI_WINDOW_RESIZE] = L"Art/UserControl/appbar.window.restore.png";
	mIcons[DI_WINDOW_COLLAPSE] = L"Art/UserControl/appbar.arrow.collapsed.png";
	mIcons[DI_WINDOW_EXPAND] = L"Art/UserControl/appbar.arrow.expand.png";
	mIcons[DI_CURSOR_UP] = L"Art/UserControl/appbar.chevron.up.png";
	mIcons[DI_CURSOR_DOWN] = L"Art/UserControl/appbar.chevron.down.png";
	mIcons[DI_CURSOR_LEFT] = L"Art/UserControl/appbar.chevron.left.png";
	mIcons[DI_CURSOR_RIGHT] = L"Art/UserControl/appbar.chevron.right.png";
	mIcons[DI_MENU_MORE] = L"Art/UserControl/appbar.add.png";
	mIcons[DI_CHECKBOX_CHECKED] = L"Art/UserControl/appbar.checkbox.check.png";
	mIcons[DI_CHECKBOX_UNCHECKED] = L"Art/UserControl/appbar.checkbox.uncheck.png";
	mIcons[DI_DROP_DOWN] = L"Art/UserControl/appbar.chevron.down.png";
	mIcons[DI_SMALL_CURSOR_UP] = L"Art/UserControl/appbar.cursor.pointer.png";
	mIcons[DI_SMALL_CURSOR_DOWN] = L"Art/UserControl/appbar.cursor.pointer.png";
	mIcons[DI_RADIO_BUTTON_CHECKED] = L"Art/UserControl/appbar.checkmark.cross.png";
	mIcons[DI_MORE_LEFT] = L"Art/UserControl/appbar.chevron.left.png";
	mIcons[DI_MORE_RIGHT] = L"Art/UserControl/appbar.chevron.right.png";
	mIcons[DI_MORE_UP] = L"Art/UserControl/appbar.chevron.up.png";
	mIcons[DI_MORE_DOWN] = L"Art/UserControl/appbar.chevron.down.png";
	mIcons[DI_EXPAND] = L"Art/UserControl/appbar.arrow.expand.png";
	mIcons[DI_COLLAPSE] = L"Art/UserControl/appbar.arrow.collapsed.png";

	mIcons[DI_FILE] = L"Art/UserControl/appbar.file.png";
	mIcons[DI_DIRECTORY] = L"Art/UserControl/appbar.folder.png";

	for (unsigned int i=0; i<DF_COUNT; ++i)
		mFonts[i] = 0;

	mUseGradient = mType == STT_WINDOWS_METALLIC;
}


//! destructor
UISkin::~UISkin()
{

}

//! returns default color
SColor UISkin::GetColor(UIDefaultColor color) const
{
	if ((unsigned int)color < DC_COUNT)
		return mColors[color];
	else
		return SColor();
}


//! sets a default color
void UISkin::SetColor(UIDefaultColor which, SColor newColor)
{
	if ((unsigned int)which < DC_COUNT)
		mColors[which] = newColor;
}


//! returns size for the given size type
int UISkin::GetSize(UIDefaultSize size) const
{
	if ((unsigned int)size < DS_COUNT)
		return mSizes[size];
	else
		return 0;
}


//! sets a default size
void UISkin::SetSize(UIDefaultSize which, int size)
{
	if ((unsigned int)which < DS_COUNT)
		mSizes[which] = size;
}


//! returns the default font
const eastl::shared_ptr<BaseUIFont>& UISkin::GetFont(UIDefaultFont which) const
{
	if (((unsigned int)which < DF_COUNT) && mFonts[which])
		return mFonts[which];
	else
		return mFonts[DF_DEFAULT];
}


//! sets a default font
void UISkin::SetFont(const eastl::shared_ptr<BaseUIFont>& font, UIDefaultFont which)
{
	if ((unsigned int)which >= DF_COUNT)
		return;

	if (font)
		mFonts[which] = font;
}


//! gets the sprite bank stored
const eastl::shared_ptr<BaseUISpriteBank>& UISkin::GetSpriteBank() const
{
	return mSpriteBank;
}


//! set a new sprite bank or Remove one by passing 0
void UISkin::SetSpriteBank(const eastl::shared_ptr<BaseUISpriteBank>& bank)
{
	mSpriteBank = bank;
}


//! Returns a default icon
const wchar_t* UISkin::GetIcon(UIDefaultIcon icon) const
{
	if ((unsigned int)icon < DI_COUNT)
		return mIcons[icon].c_str();
	else
		return nullptr;
}


//! Sets a default icon
void UISkin::SetIcon(UIDefaultIcon icon, wchar_t* path)
{
	if ((unsigned int)icon < DI_COUNT)
		mIcons[icon] = path;
}


//! Returns a default text. For example for Message box button captions:
//! "OK", "Cancel", "Yes", "No" and so on.
const wchar_t* UISkin::GetDefaultText(UIDefaultText text) const
{
	if ((unsigned int)text < DT_COUNT)
		return mTexts[text].c_str();
	else
		return mTexts[0].c_str();
}


//! Sets a default text. For example for Message box button captions:
//! "OK", "Cancel", "Yes", "No" and so on.
void UISkin::SetDefaultText(UIDefaultText which, const wchar_t* newText)
{
	if ((unsigned int)which < DT_COUNT)
		mTexts[which] = newText;
}


//! draws a standard 3d button pane
void UISkin::Draw3DButtonPaneStandard(const eastl::shared_ptr<BaseUIElement>& element,
	const eastl::shared_ptr<Visual>& visual, const RectangleShape<2, int>& r, 
    const RectangleShape<2, int>* clip, const SColor* colors)
{
	if (!Renderer::Get())
		return;

    if (!colors)
        colors = mColors;

    RectangleShape<2, int> rect = r;
    Vector2<int> dimension(clip->mExtent / 2);

	struct Vertex
	{
		Vector3<float> position;
		Vector4<float> color;
	};
	Vertex* vertex = visual->GetVertexBuffer()->Get<Vertex>();

	if (mUseGradient)
	{
		const eastl::array<float, 4U> c1 = SColorF(colors[DC_3D_FACE]).ToArray();
		const eastl::array<float, 4U> c2 = 
            SColorF(colors[DC_3D_FACE].GetInterpolated(colors[DC_3D_DARK_SHADOW], 0.4f)).ToArray();

		vertex[0].position = {
			(float)(rect.mCenter[0] - dimension[0] - (rect.mExtent[0] / 2)) / dimension[0],
			(float)(dimension[1] - rect.mCenter[1] - (rect.mExtent[1] / 2)) / dimension[1], 0.0f };
		vertex[0].color = { c1[0], c1[1], c1[2], 1.0f };
		vertex[1].position = {
			(float)(rect.mCenter[0] - dimension[0] + (int)round(rect.mExtent[0] / 2.f)) / dimension[0],
			(float)(dimension[1] - rect.mCenter[1] - (rect.mExtent[1] / 2)) / dimension[1], 0.0f };
		vertex[1].color = { c2[0], c2[1], c2[2], 1.0f };
		vertex[2].position = {
			(float)(rect.mCenter[0] - dimension[0] - (rect.mExtent[0] / 2)) / dimension[0],
			(float)(dimension[1] - rect.mCenter[1] + (int)round(rect.mExtent[1] / 2.f)) / dimension[1], 0.0f };
		vertex[2].color = { c1[0], c1[1], c1[2], 1.0f };
		vertex[3].position = {
			(float)(rect.mCenter[0] - dimension[0] + (int)round(rect.mExtent[0] / 2.f)) / dimension[0],
			(float)(dimension[1] - rect.mCenter[1] + (int)round(rect.mExtent[1] / 2.f)) / dimension[1], 0.0f };
		vertex[3].color = { c1[0], c1[1], c1[2], 1.0f };
	}
	else
	{
		const eastl::array<float, 4U> color = SColorF(colors[DC_3D_FACE]).ToArray();

		vertex[0].position = {
			(float)(rect.mCenter[0] - dimension[0] - (rect.mExtent[0] / 2)) / dimension[0],
			(float)(dimension[1] - rect.mCenter[1] - (rect.mExtent[1] / 2)) / dimension[1], 0.0f };
		vertex[0].color = color;
		vertex[1].position = {
			(float)(rect.mCenter[0] - dimension[0] + (int)round(rect.mExtent[0] / 2.f)) / dimension[0],
			(float)(dimension[1] - rect.mCenter[1] - (rect.mExtent[1] / 2)) / dimension[1], 0.0f };
		vertex[1].color = color;
		vertex[2].position = {
			(float)(rect.mCenter[0] - dimension[0] - (rect.mExtent[0] / 2)) / dimension[0],
			(float)(dimension[1] - rect.mCenter[1] + (int)round(rect.mExtent[1] / 2.f)) / dimension[1], 0.0f };
		vertex[2].color = color;
		vertex[3].position = {
			(float)(rect.mCenter[0] - dimension[0] + (int)round(rect.mExtent[0] / 2.f)) / dimension[0],
			(float)(dimension[1] - rect.mCenter[1] + (int)round(rect.mExtent[1] / 2.f)) / dimension[1], 0.0f };
		vertex[3].color = color;
	}

	// Create the geometric object for drawing.
	Renderer::Get()->Update(visual->GetVertexBuffer());
	Renderer::Get()->Draw(visual);
}


//! draws a pressed 3d button pane
void UISkin::Draw3DButtonPanePressed(const eastl::shared_ptr<BaseUIElement>& element,
	const eastl::shared_ptr<Visual>& visual, const RectangleShape<2, int>& r, 
    const RectangleShape<2, int>* clip, const SColor* colors)
{
	if (!Renderer::Get())
		return;

    if (!colors)
        colors = mColors;

	RectangleShape<2, int> rect = r;
	Vector2<int> dimension(clip->mExtent / 2);

	struct Vertex
	{
		Vector3<float> position;
		Vector4<float> color;
	};
	Vertex* vertex = visual->GetVertexBuffer()->Get<Vertex>();

	if (mUseGradient)
	{
        const eastl::array<float, 4U> c1 = SColorF(colors[DC_3D_FACE]).ToArray();
        const eastl::array<float, 4U> c2 = 
            SColorF(colors[DC_3D_FACE].GetInterpolated(colors[DC_3D_DARK_SHADOW], 0.4f)).ToArray();

		vertex[0].position = {
			(float)(rect.mCenter[0] - dimension[0] - (rect.mExtent[0] / 2)) / dimension[0],
			(float)(dimension[1] - rect.mCenter[1] - (rect.mExtent[1] / 2)) / dimension[1], 0.0f };
		vertex[0].color = { c1[0], c1[1], c1[2], 1.0f };
		vertex[1].position = {
			(float)(rect.mCenter[0] - dimension[0] + (int)round(rect.mExtent[0] / 2.f)) / dimension[0],
			(float)(dimension[1] - rect.mCenter[1] - (rect.mExtent[1] / 2)) / dimension[1], 0.0f };
		vertex[1].color = { c2[0], c2[1], c2[2], 1.0f };
		vertex[2].position = {
			(float)(rect.mCenter[0] - dimension[0] - (rect.mExtent[0] / 2)) / dimension[0],
			(float)(dimension[1] - rect.mCenter[1] + (int)round(rect.mExtent[1] / 2.f)) / dimension[1], 0.0f };
		vertex[2].color = { c1[0], c1[1], c1[2], 1.0f };
		vertex[3].position = {
			(float)(rect.mCenter[0] - dimension[0] + (int)round(rect.mExtent[0] / 2.f)) / dimension[0],
			(float)(dimension[1] - rect.mCenter[1] + (int)round(rect.mExtent[1] / 2.f)) / dimension[1], 0.0f };
		vertex[3].color = { c1[0], c1[1], c1[2], 1.0f };
	}
	else
	{
        const eastl::array<float, 4U> color = SColorF(colors[DC_3D_FACE]).ToArray();

		vertex[0].position = {
			(float)(rect.mCenter[0] - dimension[0] - (rect.mExtent[0] / 2)) / dimension[0],
			(float)(dimension[1] - rect.mCenter[1] - (rect.mExtent[1] / 2)) / dimension[1], 0.0f };
		vertex[0].color = color;
		vertex[1].position = {
			(float)(rect.mCenter[0] - dimension[0] + (int)round(rect.mExtent[0] / 2.f)) / dimension[0],
			(float)(dimension[1] - rect.mCenter[1] - (rect.mExtent[1] / 2)) / dimension[1], 0.0f };
		vertex[1].color = color;
		vertex[2].position = {
			(float)(rect.mCenter[0] - dimension[0] - (rect.mExtent[0] / 2)) / dimension[0],
			(float)(dimension[1] - rect.mCenter[1] + (int)round(rect.mExtent[1] / 2.f)) / dimension[1], 0.0f };
		vertex[2].color = color;
		vertex[3].position = {
			(float)(rect.mCenter[0] - dimension[0] + (int)round(rect.mExtent[0] / 2.f)) / dimension[0],
			(float)(dimension[1] - rect.mCenter[1] + (int)round(rect.mExtent[1] / 2.f)) / dimension[1], 0.0f };
		vertex[3].color = color;
	}

	// Create the geometric object for drawing.
	Renderer::Get()->Update(visual->GetVertexBuffer());
	Renderer::Get()->Draw(visual);
}


//! draws a sunken 3d pane
void UISkin::Draw3DSunkenPane(const eastl::shared_ptr<BaseUIElement>& element, 
	SColorF bgcolor, bool flat, bool fillBackGround, const eastl::shared_ptr<Visual>& visual, 
    const RectangleShape<2, int>& r, const RectangleShape<2, int>* clip, const SColor* colors)
{
	if (!Renderer::Get())
		return;

    if (!colors)
        colors = mColors;

    if (fillBackGround)
    {
        RectangleShape<2, int> rect = r;
        Vector2<int> dimension(clip->mExtent / 2);

        struct Vertex
        {
            Vector3<float> position;
            Vector4<float> color;
        };
        Vertex* vertex = visual->GetVertexBuffer()->Get<Vertex>();

        const eastl::array<float, 4U> color = bgcolor.ToArray();

        if (mUseGradient)
        {
            vertex[0].position = {
                (float)(rect.mCenter[0] - dimension[0] - (rect.mExtent[0] / 2)) / dimension[0],
                (float)(dimension[1] - rect.mCenter[1] - (rect.mExtent[1] / 2)) / dimension[1], 0.0f };
            vertex[0].color = color;
            vertex[1].position = {
                (float)(rect.mCenter[0] - dimension[0] + (int)round(rect.mExtent[0] / 2.f)) / dimension[0],
                (float)(dimension[1] - rect.mCenter[1] - (rect.mExtent[1] / 2)) / dimension[1], 0.0f };
            vertex[1].color = color;
            vertex[2].position = {
                (float)(rect.mCenter[0] - dimension[0] - (rect.mExtent[0] / 2)) / dimension[0],
                (float)(dimension[1] - rect.mCenter[1] + (int)round(rect.mExtent[1] / 2.f)) / dimension[1], 0.0f };
            vertex[2].color = color;
            vertex[3].position = {
                (float)(rect.mCenter[0] - dimension[0] + (int)round(rect.mExtent[0] / 2.f)) / dimension[0],
                (float)(dimension[1] - rect.mCenter[1] + (int)round(rect.mExtent[1] / 2.f)) / dimension[1], 0.0f };
            vertex[3].color = color;
        }
        else
        {
            vertex[0].position = {
                (float)(rect.mCenter[0] - dimension[0] - (rect.mExtent[0] / 2)) / dimension[0],
                (float)(dimension[1] - rect.mCenter[1] - (rect.mExtent[1] / 2)) / dimension[1], 0.0f };
            vertex[0].color = color;
            vertex[1].position = {
                (float)(rect.mCenter[0] - dimension[0] + (int)round(rect.mExtent[0] / 2.f)) / dimension[0],
                (float)(dimension[1] - rect.mCenter[1] - (rect.mExtent[1] / 2)) / dimension[1], 0.0f };
            vertex[1].color = color;
            vertex[2].position = {
                (float)(rect.mCenter[0] - dimension[0] - (rect.mExtent[0] / 2)) / dimension[0],
                (float)(dimension[1] - rect.mCenter[1] + (int)round(rect.mExtent[1] / 2.f)) / dimension[1], 0.0f };
            vertex[2].color = color;
            vertex[3].position = {
                (float)(rect.mCenter[0] - dimension[0] + (int)round(rect.mExtent[0] / 2.f)) / dimension[0],
                (float)(dimension[1] - rect.mCenter[1] + (int)round(rect.mExtent[1] / 2.f)) / dimension[1], 0.0f };
            vertex[3].color = color;
        }

        // Create the geometric object for drawing.
        Renderer::Get()->Update(visual->GetVertexBuffer());
        Renderer::Get()->Draw(visual);
    }
}


//! draws a window background
// return where to draw title bar text.
RectangleShape<2, int> UISkin::Draw3DWindowBackground(
    const eastl::shared_ptr<BaseUIElement>& element, const eastl::shared_ptr<Visual>& visualBackground, 
    const eastl::shared_ptr<Visual>& visualTitle, bool drawTitleBar, SColor titleBarColor, 
    const RectangleShape<2, int>& r, const RectangleShape<2, int>* clip, 
    RectangleShape<2, int>* checkClientArea, const SColor* colors)
{
	if (!Renderer::Get())
	{
		if ( checkClientArea )
			*checkClientArea = r;

		return r;
	}

    if (!colors)
        colors = mColors;
	
	RectangleShape<2, int> rect = r;
	Vector2<int> dimension(clip->mExtent / 2);

	if (visualBackground)
	{
		struct Vertex
		{
			Vector3<float> position;
			Vector4<float> color;
		};
		Vertex* vertex = visualBackground->GetVertexBuffer()->Get<Vertex>();

		const eastl::array<float, 4U> c2 = SColorF(colors[DC_3D_SHADOW]).ToArray();
		const eastl::array<float, 4U> c1 = SColorF(colors[DC_3D_FACE]).ToArray();

		vertex[0].position = {
			(float)(rect.mCenter[0] - dimension[0] - (rect.mExtent[0] / 2)) / dimension[0],
			(float)(dimension[1] - rect.mCenter[1] - (rect.mExtent[1] / 2)) / dimension[1], 0.0f };
		vertex[0].color = { c1[0], c1[1], c1[2], 1.0f };
		vertex[1].position = {
			(float)(rect.mCenter[0] - dimension[0] + (int)round(rect.mExtent[0] / 2.f)) / dimension[0],
			(float)(dimension[1] - rect.mCenter[1] - (rect.mExtent[1] / 2)) / dimension[1], 0.0f };
		vertex[1].color = { c2[0], c2[1], c2[2], 1.0f };
		vertex[2].position = {
			(float)(rect.mCenter[0] - dimension[0] - (rect.mExtent[0] / 2)) / dimension[0],
			(float)(dimension[1] - rect.mCenter[1] + (int)round(rect.mExtent[1] / 2.f)) / dimension[1], 0.0f };
		vertex[2].color = { c1[0], c1[1], c1[2], 1.0f };
		vertex[3].position = {
			(float)(rect.mCenter[0] - dimension[0] + (int)round(rect.mExtent[0] / 2.f)) / dimension[0],
			(float)(dimension[1] - rect.mCenter[1] + (int)round(rect.mExtent[1] / 2.f)) / dimension[1], 0.0f };
		vertex[3].color = { c1[0], c1[1], c1[2], 1.0f };

		// Create the geometric object for drawing.
		Renderer::Get()->Update(visualBackground->GetVertexBuffer());
		Renderer::Get()->Draw(visualBackground);
	}

	// title bar
	rect = r;
	rect.mExtent[0] -= 2;
	rect.mExtent[1] -= 2;
	rect.mCenter[1] = (rect.mCenter[1] - rect.mExtent[1] / 2) + (GetSize(DS_WINDOW_BUTTON_WIDTH) + 2) / 2;
	rect.mExtent[1] = GetSize(DS_WINDOW_BUTTON_WIDTH) + 2;

	if (drawTitleBar)
	{
		if (!checkClientArea)
		{
			struct Vertex
			{
				Vector3<float> position;
				Vector4<float> color;
			};
			Vertex* vertex = visualTitle->GetVertexBuffer()->Get<Vertex>();

            eastl::array<float, 4U> color = SColorF(
                titleBarColor.GetInterpolated(SColor(titleBarColor.GetAlpha(), 0, 0, 0), 0.2f)).ToArray();

			// draw title bar
			vertex[0].position = {
				(float)(rect.mCenter[0] - dimension[0] - (rect.mExtent[0] / 2)) / dimension[0],
				(float)(dimension[1] - rect.mCenter[1] - (rect.mExtent[1] / 2)) / dimension[1], 0.0f };
			vertex[0].color = color;
			vertex[1].position = {
				(float)(rect.mCenter[0] - dimension[0] + (int)round(rect.mExtent[0] / 2.f)) / dimension[0],
				(float)(dimension[1] - rect.mCenter[1] - (rect.mExtent[1] / 2)) / dimension[1], 0.0f };
			vertex[1].color = color;
			vertex[2].position = {
				(float)(rect.mCenter[0] - dimension[0] - (rect.mExtent[0] / 2)) / dimension[0],
				(float)(dimension[1] - rect.mCenter[1] + (int)round(rect.mExtent[1] / 2.f)) / dimension[1], 0.0f };
			vertex[2].color = color;
			vertex[3].position = {
				(float)(rect.mCenter[0] - dimension[0] + (int)round(rect.mExtent[0] / 2.f)) / dimension[0],
				(float)(dimension[1] - rect.mCenter[1] + (int)round(rect.mExtent[1] / 2.f)) / dimension[1], 0.0f };
			vertex[3].color = color;

			// Create the geometric object for drawing.
			Renderer::Get()->Update(visualTitle->GetVertexBuffer());
			Renderer::Get()->Draw(visualTitle);
		}
		else
		{
			(*checkClientArea).mCenter[1] += (int)round((*checkClientArea).mExtent[1] / 2.f);
			(*checkClientArea).mExtent[1] = 0;
		}
	}
	return rect;
}


//! draws a standard 3d menu pane
void UISkin::Draw3DMenuPane(const eastl::shared_ptr<BaseUIElement>& element,
	const eastl::shared_ptr<Visual>& visual, const RectangleShape<2, int>& r, 
    const RectangleShape<2, int>* clip, const SColor* colors)
{
	if (!Renderer::Get())
		return;

    if (!colors)
        colors = mColors;

	RectangleShape<2, int> rect = r;
	Vector2<int> dimension(clip->mExtent / 2);

	struct Vertex
	{
		Vector3<float> position;
		Vector4<float> color;
	};
	Vertex* vertex = visual->GetVertexBuffer()->Get<Vertex>();

	if (mUseGradient)
	{
		const eastl::array<float, 4U> c1 = SColorF(colors[DC_3D_FACE]).ToArray();
		const eastl::array<float, 4U> c2 = SColorF(colors[DC_3D_DARK_SHADOW]).ToArray();

		vertex[0].position = {
			(float)(rect.mCenter[0] - dimension[0] - (rect.mExtent[0] / 2)) / dimension[0],
			(float)(dimension[1] - rect.mCenter[1] - (rect.mExtent[1] / 2)) / dimension[1], 0.0f };
		vertex[0].color = { c1[0], c1[1], c1[2], 1.0f };
		vertex[1].position = {
			(float)(rect.mCenter[0] - dimension[0] + (int)round(rect.mExtent[0] / 2.f)) / dimension[0],
			(float)(dimension[1] - rect.mCenter[1] - (rect.mExtent[1] / 2)) / dimension[1], 0.0f };
		vertex[1].color = { c2[0], c2[1], c2[2], 1.0f };
		vertex[2].position = {
			(float)(rect.mCenter[0] - dimension[0] - (rect.mExtent[0] / 2)) / dimension[0],
			(float)(dimension[1] - rect.mCenter[1] + (int)round(rect.mExtent[1] / 2.f)) / dimension[1], 0.0f };
		vertex[2].color = { c1[0], c1[1], c1[2], 1.0f };
		vertex[3].position = {
			(float)(rect.mCenter[0] - dimension[0] + (int)round(rect.mExtent[0] / 2.f)) / dimension[0],
			(float)(dimension[1] - rect.mCenter[1] + (int)round(rect.mExtent[1] / 2.f)) / dimension[1], 0.0f };
		vertex[3].color = { c1[0], c1[1], c1[2], 1.0f };
	}
	else
	{
		const eastl::array<float, 4U> color = SColorF(colors[DC_3D_FACE]).ToArray();

		vertex[0].position = {
			(float)(rect.mCenter[0] - dimension[0] - (rect.mExtent[0] / 2)) / dimension[0],
			(float)(dimension[1] - rect.mCenter[1] - (rect.mExtent[1] / 2)) / dimension[1], 0.0f };
		vertex[0].color = color;
		vertex[1].position = {
			(float)(rect.mCenter[0] - dimension[0] + (int)round(rect.mExtent[0] / 2.f)) / dimension[0],
			(float)(dimension[1] - rect.mCenter[1] - (rect.mExtent[1] / 2)) / dimension[1], 0.0f };
		vertex[1].color = color;
		vertex[2].position = {
			(float)(rect.mCenter[0] - dimension[0] - (rect.mExtent[0] / 2)) / dimension[0],
			(float)(dimension[1] - rect.mCenter[1] + (int)round(rect.mExtent[1] / 2.f)) / dimension[1], 0.0f };
		vertex[2].color = color;
		vertex[3].position = {
			(float)(rect.mCenter[0] - dimension[0] + (int)round(rect.mExtent[0] / 2.f)) / dimension[0],
			(float)(dimension[1] - rect.mCenter[1] + (int)round(rect.mExtent[1] / 2.f)) / dimension[1], 0.0f };
		vertex[3].color = color;
	}

	// Create the geometric object for drawing.
	Renderer::Get()->Update(visual->GetVertexBuffer());
	Renderer::Get()->Draw(visual);
}


//! draws a standard 3d tool bar
void UISkin::Draw3DToolBar(const eastl::shared_ptr<BaseUIElement>& element,
	const eastl::shared_ptr<Visual>& visual, const RectangleShape<2, int>& r, 
    const RectangleShape<2, int>* clip, const SColor* colors)
{
	if (!Renderer::Get())
		return;

    if (!colors)
        colors = mColors;

	RectangleShape<2, int> rect = r;
	Vector2<int> dimension(clip->mExtent / 2);

	struct Vertex
	{
		Vector3<float> position;
		Vector4<float> color;
	};
	Vertex* vertex = visual->GetVertexBuffer()->Get<Vertex>();

	if (mUseGradient)
	{
		const eastl::array<float, 4U> c1 = SColorF(colors[DC_3D_FACE]).ToArray();
		const eastl::array<float, 4U> c2 = SColorF(colors[DC_3D_DARK_SHADOW]).ToArray();

		vertex[0].position = {
			(float)(rect.mCenter[0] - dimension[0] - (rect.mExtent[0] / 2)) / dimension[0],
			(float)(dimension[1] - rect.mCenter[1] - (rect.mExtent[1] / 2)) / dimension[1], 0.0f };
		vertex[0].color = { c1[0], c1[1], c1[2], 1.0f };
		vertex[1].position = {
			(float)(rect.mCenter[0] - dimension[0] + (int)round(rect.mExtent[0] / 2.f)) / dimension[0],
			(float)(dimension[1] - rect.mCenter[1] - (rect.mExtent[1] / 2)) / dimension[1], 0.0f };
		vertex[1].color = { c2[0], c2[1], c2[2], 1.0f };
		vertex[2].position = {
			(float)(rect.mCenter[0] - dimension[0] - (rect.mExtent[0] / 2)) / dimension[0],
			(float)(dimension[1] - rect.mCenter[1] + (int)round(rect.mExtent[1] / 2.f)) / dimension[1], 0.0f };
		vertex[2].color = { c1[0], c1[1], c1[2], 1.0f };
		vertex[3].position = {
			(float)(rect.mCenter[0] - dimension[0] + (int)round(rect.mExtent[0] / 2.f)) / dimension[0],
			(float)(dimension[1] - rect.mCenter[1] + (int)round(rect.mExtent[1] / 2.f)) / dimension[1], 0.0f };
		vertex[3].color = { c1[0], c1[1], c1[2], 1.0f };
	}
	else
	{
		const eastl::array<float, 4U> color = SColorF(colors[DC_3D_FACE]).ToArray();

		vertex[0].position = {
			(float)(rect.mCenter[0] - dimension[0] - (rect.mExtent[0] / 2)) / dimension[0],
			(float)(dimension[1] - rect.mCenter[1] - (rect.mExtent[1] / 2)) / dimension[1], 0.0f };
		vertex[0].color = color;
		vertex[1].position = {
			(float)(rect.mCenter[0] - dimension[0] + (int)round(rect.mExtent[0] / 2.f)) / dimension[0],
			(float)(dimension[1] - rect.mCenter[1] - (rect.mExtent[1] / 2)) / dimension[1], 0.0f };
		vertex[1].color = color;
		vertex[2].position = {
			(float)(rect.mCenter[0] - dimension[0] - (rect.mExtent[0] / 2)) / dimension[0],
			(float)(dimension[1] - rect.mCenter[1] + (int)round(rect.mExtent[1] / 2.f)) / dimension[1], 0.0f };
		vertex[2].color = color;
		vertex[3].position = {
			(float)(rect.mCenter[0] - dimension[0] + (int)round(rect.mExtent[0] / 2.f)) / dimension[0],
			(float)(dimension[1] - rect.mCenter[1] + (int)round(rect.mExtent[1] / 2.f)) / dimension[1], 0.0f };
		vertex[3].color = color;
	}

	// Create the geometric object for drawing.
	Renderer::Get()->Update(visual->GetVertexBuffer());
	Renderer::Get()->Draw(visual);
}


//! draws a tab button
void UISkin::Draw3DTabButton(const eastl::shared_ptr<BaseUIElement>& element, 
    bool active, const eastl::shared_ptr<Visual>& visual, const RectangleShape<2, int>& r, 
    const RectangleShape<2, int>* clip, UIAlignment alignment, const SColor* colors)
{
	if (!Renderer::Get())
		return;

    if (!colors)
        colors = mColors;

	RectangleShape<2, int> rect = r;
	Vector2<int> dimension(clip->mExtent / 2);

	struct Vertex
	{
		Vector3<float> position;
		Vector4<float> color;
	};
	Vertex* vertex = visual->GetVertexBuffer()->Get<Vertex>();

	if (mUseGradient)
	{
		const eastl::array<float, 4U> c1 = SColorF(colors[DC_3D_FACE]).ToArray();
		const eastl::array<float, 4U> c2 = SColorF(colors[DC_3D_DARK_SHADOW]).ToArray();

		vertex[0].position = {
			(float)(rect.mCenter[0] - dimension[0] - (rect.mExtent[0] / 2)) / dimension[0],
			(float)(dimension[1] - rect.mCenter[1] - (rect.mExtent[1] / 2)) / dimension[1], 0.0f };
		vertex[0].color = { c1[0], c1[1], c1[2], 1.0f };
		vertex[1].position = {
			(float)(rect.mCenter[0] - dimension[0] + (int)round(rect.mExtent[0] / 2.f)) / dimension[0],
			(float)(dimension[1] - rect.mCenter[1] - (rect.mExtent[1] / 2)) / dimension[1], 0.0f };
		vertex[1].color = { c2[0], c2[1], c2[2], 1.0f };
		vertex[2].position = {
			(float)(rect.mCenter[0] - dimension[0] - (rect.mExtent[0] / 2)) / dimension[0],
			(float)(dimension[1] - rect.mCenter[1] + (int)round(rect.mExtent[1] / 2.f)) / dimension[1], 0.0f };
		vertex[2].color = { c1[0], c1[1], c1[2], 1.0f };
		vertex[3].position = {
			(float)(rect.mCenter[0] - dimension[0] + (int)round(rect.mExtent[0] / 2.f)) / dimension[0],
			(float)(dimension[1] - rect.mCenter[1] + (int)round(rect.mExtent[1] / 2.f)) / dimension[1], 0.0f };
		vertex[3].color = { c1[0], c1[1], c1[2], 1.0f };
	}
	else
	{
		const eastl::array<float, 4U> color = SColorF(colors[DC_3D_FACE]).ToArray();

		vertex[0].position = {
			(float)(rect.mCenter[0] - dimension[0] - (rect.mExtent[0] / 2)) / dimension[0],
			(float)(dimension[1] - rect.mCenter[1] - (rect.mExtent[1] / 2)) / dimension[1], 0.0f };
		vertex[0].color = color;
		vertex[1].position = {
			(float)(rect.mCenter[0] - dimension[0] + (int)round(rect.mExtent[0] / 2.f)) / dimension[0],
			(float)(dimension[1] - rect.mCenter[1] - (rect.mExtent[1] / 2)) / dimension[1], 0.0f };
		vertex[1].color = color;
		vertex[2].position = {
			(float)(rect.mCenter[0] - dimension[0] - (rect.mExtent[0] / 2)) / dimension[0],
			(float)(dimension[1] - rect.mCenter[1] + (int)round(rect.mExtent[1] / 2.f)) / dimension[1], 0.0f };
		vertex[2].color = color;
		vertex[3].position = {
			(float)(rect.mCenter[0] - dimension[0] + (int)round(rect.mExtent[0] / 2.f)) / dimension[0],
			(float)(dimension[1] - rect.mCenter[1] + (int)round(rect.mExtent[1] / 2.f)) / dimension[1], 0.0f };
		vertex[3].color = color;
	}

	// Create the geometric object for drawing.
	Renderer::Get()->Update(visual->GetVertexBuffer());
	Renderer::Get()->Draw(visual);
}


//! draws a tab control body
void UISkin::Draw3DTabBody(const eastl::shared_ptr<BaseUIElement>& element, bool border, bool background,
	const eastl::shared_ptr<Visual>& visual, const RectangleShape<2, int>& r, const RectangleShape<2, int>* clip, 
	int tabHeight, UIAlignment alignment, const SColor* colors)
{
	if (!Renderer::Get())
		return;

    if (!colors)
        colors = mColors;

	RectangleShape<2, int> rect = r;
	Vector2<int> dimension(clip->mExtent / 2);

	struct Vertex
	{
		Vector3<float> position;
		Vector4<float> color;
	};
	Vertex* vertex = visual->GetVertexBuffer()->Get<Vertex>();

	if (mUseGradient)
	{
		const eastl::array<float, 4U> c1 = SColorF(colors[DC_3D_FACE]).ToArray();
		const eastl::array<float, 4U> c2 = SColorF(colors[DC_3D_DARK_SHADOW]).ToArray();

		vertex[0].position = {
			(float)(rect.mCenter[0] - dimension[0] - (rect.mExtent[0] / 2)) / dimension[0],
			(float)(dimension[1] - rect.mCenter[1] - (rect.mExtent[1] / 2)) / dimension[1], 0.0f };
		vertex[0].color = { c1[0], c1[1], c1[2], 1.0f };
		vertex[1].position = {
			(float)(rect.mCenter[0] - dimension[0] + (int)round(rect.mExtent[0] / 2.f)) / dimension[0],
			(float)(dimension[1] - rect.mCenter[1] - (rect.mExtent[1] / 2)) / dimension[1], 0.0f };
		vertex[1].color = { c2[0], c2[1], c2[2], 1.0f };
		vertex[2].position = {
			(float)(rect.mCenter[0] - dimension[0] - (rect.mExtent[0] / 2)) / dimension[0],
			(float)(dimension[1] - rect.mCenter[1] + (int)round(rect.mExtent[1] / 2.f)) / dimension[1], 0.0f };
		vertex[2].color = { c1[0], c1[1], c1[2], 1.0f };
		vertex[3].position = {
			(float)(rect.mCenter[0] - dimension[0] + (int)round(rect.mExtent[0] / 2.f)) / dimension[0],
			(float)(dimension[1] - rect.mCenter[1] + (int)round(rect.mExtent[1] / 2.f)) / dimension[1], 0.0f };
		vertex[3].color = { c1[0], c1[1], c1[2], 1.0f };
	}
	else
	{
		const eastl::array<float, 4U> color = SColorF(colors[DC_3D_FACE]).ToArray();

		vertex[0].position = {
			(float)(rect.mCenter[0] - dimension[0] - (rect.mExtent[0] / 2)) / dimension[0],
			(float)(dimension[1] - rect.mCenter[1] - (rect.mExtent[1] / 2)) / dimension[1], 0.0f };
		vertex[0].color = color;
		vertex[1].position = {
			(float)(rect.mCenter[0] - dimension[0] + (int)round(rect.mExtent[0] / 2.f)) / dimension[0],
			(float)(dimension[1] - rect.mCenter[1] - (rect.mExtent[1] / 2)) / dimension[1], 0.0f };
		vertex[1].color = color;
		vertex[2].position = {
			(float)(rect.mCenter[0] - dimension[0] - (rect.mExtent[0] / 2)) / dimension[0],
			(float)(dimension[1] - rect.mCenter[1] + (int)round(rect.mExtent[1] / 2.f)) / dimension[1], 0.0f };
		vertex[2].color = color;
		vertex[3].position = {
			(float)(rect.mCenter[0] - dimension[0] + (int)round(rect.mExtent[0] / 2.f)) / dimension[0],
			(float)(dimension[1] - rect.mCenter[1] + (int)round(rect.mExtent[1] / 2.f)) / dimension[1], 0.0f };
		vertex[3].color = color;
	}

	// Create the geometric object for drawing.
	Renderer::Get()->Update(visual->GetVertexBuffer());
	Renderer::Get()->Draw(visual);
}


//! draws an icon, usually from the skin's sprite bank
void UISkin::DrawIcon(const eastl::shared_ptr<BaseUIElement>& element, UIDefaultIcon icon,
    const eastl::shared_ptr<Visual>& visual, const RectangleShape<2, int> position,
    const RectangleShape<2, int>* clip, unsigned int starttime,
    unsigned int currenttime, bool loop, const SColor* colors)
{
    if (!mSpriteBank)
        return;

    if (!colors)
        colors = mColors;

    bool gray = element && !element->IsEnabled();

    mSpriteBank->Draw2DSprite(icon, visual, position, clip,
        colors[gray ? DC_GRAY_WINDOW_SYMBOL : DC_WINDOW_SYMBOL], starttime, currenttime, loop, true);
}

//! draws a 2d texture.
void UISkin::Draw2DTexture(
    const eastl::shared_ptr<BaseUIElement>& element, const eastl::shared_ptr<Visual>& visual,
    const RectangleShape<2, int>& targetRect, const RectangleShape<2, int>& sourceRect, const Vector2<int>& dimension)
{
	if (!Renderer::Get())
		return;

	struct Vertex
	{
		Vector3<float> position;
		Vector2<float> tcoord;
	};
	Vertex* vertex = visual->GetVertexBuffer()->Get<Vertex>();
	vertex[0].position = {
		(float)(targetRect.mCenter[0] - dimension[0] - (targetRect.mExtent[0] / 2)) / dimension[0],
		(float)(dimension[1] - targetRect.mCenter[1] - (targetRect.mExtent[1] / 2)) / dimension[1], 0.0f };
	vertex[0].tcoord = {
		(float)(sourceRect.mCenter[0] - (sourceRect.mExtent[0] / 2)) / sourceRect.mExtent[0],
		(float)(sourceRect.mCenter[1] + (int)round(sourceRect.mExtent[1] / 2.f)) / sourceRect.mExtent[1] };
	vertex[1].position = {
		(float)(targetRect.mCenter[0] - dimension[0] + (int)round(targetRect.mExtent[0] / 2.f)) / dimension[0],
		(float)(dimension[1] - targetRect.mCenter[1] - (targetRect.mExtent[1] / 2)) / dimension[1], 0.0f };
	vertex[1].tcoord = {
		(float)(sourceRect.mCenter[0] + (int)round(sourceRect.mExtent[0] / 2.f)) / sourceRect.mExtent[0],
		(float)(sourceRect.mCenter[1] + (int)round(sourceRect.mExtent[1] / 2.f)) / sourceRect.mExtent[1] };
	vertex[2].position = {
		(float)(targetRect.mCenter[0] - dimension[0] - (targetRect.mExtent[0] / 2)) / dimension[0],
		(float)(dimension[1] - targetRect.mCenter[1] + (int)round(targetRect.mExtent[1] / 2.f)) / dimension[1], 0.0f };
	vertex[2].tcoord = {
		(float)(sourceRect.mCenter[0] - (sourceRect.mExtent[0] / 2)) / sourceRect.mExtent[0],
		(float)(sourceRect.mCenter[1] - (sourceRect.mExtent[1] / 2)) / sourceRect.mExtent[1] };
	vertex[3].position = {
		(float)(targetRect.mCenter[0] - dimension[0] + (int)round(targetRect.mExtent[0] / 2.f)) / dimension[0],
		(float)(dimension[1] - targetRect.mCenter[1] + (int)round(targetRect.mExtent[1] / 2.f)) / dimension[1], 0.0f };
	vertex[3].tcoord = {
		(float)(sourceRect.mCenter[0] + (int)round(sourceRect.mExtent[0] / 2.f)) / sourceRect.mExtent[0],
		(float)(sourceRect.mCenter[1] - (sourceRect.mExtent[1] / 2)) / sourceRect.mExtent[1] };

	// Create the geometric object for drawing.
	Renderer::Get()->Update(visual->GetVertexBuffer());
	Renderer::Get()->Draw(visual);
}

//! draws a 2d texture.
void UISkin::Draw2DTexture(
    const eastl::shared_ptr<BaseUIElement>& element, const eastl::shared_ptr<Visual>& visual, 
    const RectangleShape<2, int>& targetRect, const RectangleShape<2, int>& sourceRect,
    const Vector2<int>& dimension, const SColor* colors)
{
    if (!Renderer::Get())
        return;

    const SColor temp[4] =
    {
        0xFFFFFFFF,
        0xFFFFFFFF,
        0xFFFFFFFF,
        0xFFFFFFFF
    };
    const SColor* const useColor = colors ? colors : temp;

    struct Vertex
    {
        Vector3<float> position;
        Vector2<float> tcoord;
        Vector4<float> color;
    };
    Vertex* vertex = visual->GetVertexBuffer()->Get<Vertex>();
    vertex[0].position = {
        (float)(targetRect.mCenter[0] - dimension[0] - (targetRect.mExtent[0] / 2)) / dimension[0],
        (float)(dimension[1] - targetRect.mCenter[1] - (targetRect.mExtent[1] / 2)) / dimension[1], 0.0f };
    vertex[0].tcoord = {
        (float)(sourceRect.mCenter[0] - (sourceRect.mExtent[0] / 2)) / sourceRect.mExtent[0],
        (float)(sourceRect.mCenter[1] + (int)round(sourceRect.mExtent[1] / 2.f)) / sourceRect.mExtent[1] };
    vertex[0].color = SColorF(useColor[0]).ToArray();

    vertex[1].position = {
        (float)(targetRect.mCenter[0] - dimension[0] + (int)round(targetRect.mExtent[0] / 2.f)) / dimension[0],
        (float)(dimension[1] - targetRect.mCenter[1] - (targetRect.mExtent[1] / 2)) / dimension[1], 0.0f };
    vertex[1].tcoord = {
        (float)(sourceRect.mCenter[0] + (int)round(sourceRect.mExtent[0] / 2.f)) / sourceRect.mExtent[0],
        (float)(sourceRect.mCenter[1] + (int)round(sourceRect.mExtent[1] / 2.f)) / sourceRect.mExtent[1] };
    vertex[1].color = SColorF(useColor[3]).ToArray();

    vertex[2].position = {
        (float)(targetRect.mCenter[0] - dimension[0] - (targetRect.mExtent[0] / 2)) / dimension[0],
        (float)(dimension[1] - targetRect.mCenter[1] + (int)round(targetRect.mExtent[1] / 2.f)) / dimension[1], 0.0f };
    vertex[2].tcoord = {
        (float)(sourceRect.mCenter[0] - (sourceRect.mExtent[0] / 2)) / sourceRect.mExtent[0],
        (float)(sourceRect.mCenter[1] - (sourceRect.mExtent[1] / 2)) / sourceRect.mExtent[1] };
    vertex[2].color = SColorF(useColor[1]).ToArray();

    vertex[3].position = {
        (float)(targetRect.mCenter[0] - dimension[0] + (int)round(targetRect.mExtent[0] / 2.f)) / dimension[0],
        (float)(dimension[1] - targetRect.mCenter[1] + (int)round(targetRect.mExtent[1] / 2.f)) / dimension[1], 0.0f };
    vertex[3].tcoord = {
        (float)(sourceRect.mCenter[0] + (int)round(sourceRect.mExtent[0] / 2.f)) / sourceRect.mExtent[0],
        (float)(sourceRect.mCenter[1] - (sourceRect.mExtent[1] / 2)) / sourceRect.mExtent[1] };
    vertex[3].color = SColorF(useColor[2]).ToArray();

    // Create the geometric object for drawing.
    Renderer::Get()->Update(visual->GetVertexBuffer());
    Renderer::Get()->Draw(visual);
}

//! draws a 2d rectangle.
void UISkin::Draw2DRectangle(const eastl::shared_ptr<BaseUIElement>& element,
	const SColorF &color, const eastl::shared_ptr<Visual>& visual, const RectangleShape<2, int>& r, 
    const RectangleShape<2, int>* clip)
{
	if (!Renderer::Get())
		return;

	RectangleShape<2, int> rect = r;
	Vector2<int> dimension(clip->mExtent / 2);

	struct Vertex
	{
		Vector3<float> position;
		Vector4<float> color;
	};
	Vertex* vertex = visual->GetVertexBuffer()->Get<Vertex>();

    eastl::array<float, 4U> cl = color.ToArray();

	if (mUseGradient)
	{
		vertex[0].position = {
			(float)(rect.mCenter[0] - dimension[0] - (rect.mExtent[0] / 2)) / dimension[0],
			(float)(dimension[1] - rect.mCenter[1] - (rect.mExtent[1] / 2)) / dimension[1], 0.0f };
		vertex[0].color = cl;
		vertex[1].position = {
			(float)(rect.mCenter[0] - dimension[0] + (int)round(rect.mExtent[0] / 2.f)) / dimension[0],
			(float)(dimension[1] - rect.mCenter[1] - (rect.mExtent[1] / 2)) / dimension[1], 0.0f };
		vertex[1].color = cl;
		vertex[2].position = {
			(float)(rect.mCenter[0] - dimension[0] - (rect.mExtent[0] / 2)) / dimension[0],
			(float)(dimension[1] - rect.mCenter[1] + (int)round(rect.mExtent[1] / 2.f)) / dimension[1], 0.0f };
		vertex[2].color = cl;
		vertex[3].position = {
			(float)(rect.mCenter[0] - dimension[0] + (int)round(rect.mExtent[0] / 2.f)) / dimension[0],
			(float)(dimension[1] - rect.mCenter[1] + (int)round(rect.mExtent[1] / 2.f)) / dimension[1], 0.0f };
		vertex[3].color = cl;
	}
	else
	{
		vertex[0].position = {
			(float)(rect.mCenter[0] - dimension[0] - (rect.mExtent[0] / 2)) / dimension[0],
			(float)(dimension[1] - rect.mCenter[1] - (rect.mExtent[1] / 2)) / dimension[1], 0.0f };
		vertex[0].color = cl;
		vertex[1].position = {
			(float)(rect.mCenter[0] - dimension[0] + (int)round(rect.mExtent[0] / 2.f)) / dimension[0],
			(float)(dimension[1] - rect.mCenter[1] - (rect.mExtent[1] / 2)) / dimension[1], 0.0f };
		vertex[1].color = cl;
		vertex[2].position = {
			(float)(rect.mCenter[0] - dimension[0] - (rect.mExtent[0] / 2)) / dimension[0],
			(float)(dimension[1] - rect.mCenter[1] + (int)round(rect.mExtent[1] / 2.f)) / dimension[1], 0.0f };
		vertex[2].color = cl;
		vertex[3].position = {
			(float)(rect.mCenter[0] - dimension[0] + (int)round(rect.mExtent[0] / 2.f)) / dimension[0],
			(float)(dimension[1] - rect.mCenter[1] + (int)round(rect.mExtent[1] / 2.f)) / dimension[1], 0.0f };
		vertex[3].color = cl;
	}

	// Create the geometric object for drawing.
	Renderer::Get()->Update(visual->GetVertexBuffer());
	Renderer::Get()->Draw(visual);
}


UISkinThemeType UISkin::GetType() const
{
	return mType;
}