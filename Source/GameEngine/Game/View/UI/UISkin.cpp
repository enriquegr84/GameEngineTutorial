// Copyright (C) 2002-2012 Nikolaus Gebhardt
// This file is part of the "Irrlicht Engine".
// For conditions of distribution and use, see copyright notice in irrlicht.h

#include "UISkin.h"

#include "UIFont.h"
//#include "UISpriteBank.h"
#include "UIElement.h"

#include "Graphic/Renderer/Renderer.h"


UISkin::UISkin(BaseUI* ui, UISkinThemeType type)
	: mSpriteBank(0), mType(type), mUI(ui)
{
	#ifdef _DEBUG
	//setDebugName("UISkin");
	#endif

	if ((mType == STT_WINDOWS_CLASSIC) || (mType == STT_WINDOWS_METALLIC))
	{
		mColors[DC_3D_DARK_SHADOW] = eastl::array<float, 4>{101 / 255.f, 50 / 255.f, 50 / 255.f, 50 / 255.f};
		mColors[DC_3D_SHADOW] = eastl::array<float, 4>{101 / 255.f, 130 / 255.f, 130 / 255.f, 130 / 255.f};
		mColors[DC_3D_FACE] = eastl::array<float, 4>{101 / 255.f, 210 / 255.f, 210 / 255.f, 210 / 255.f};
		mColors[DC_3D_HIGH_LIGHT] = eastl::array<float, 4>{101 / 255.f, 255 / 255.f, 255 / 255.f, 255 / 255.f};
		mColors[DC_3D_LIGHT] = eastl::array<float, 4>{101 / 255.f, 210 / 255.f, 210 / 255.f, 210 / 255.f};
		mColors[DC_ACTIVE_BORDER] = eastl::array<float, 4>{101 / 255.f, 16 / 255.f, 14 / 255.f, 115 / 255.f};
		mColors[DC_ACTIVE_CAPTION] = eastl::array<float, 4>{255 / 255.f, 255 / 255.f, 255 / 255.f, 255 / 255.f};
		mColors[DC_APP_WORKSPACE] = eastl::array<float, 4>{101 / 255.f, 100 / 255.f, 100 / 255.f, 100 / 255.f};
		mColors[DC_BUTTON_TEXT] = eastl::array<float, 4>{240 / 255.f, 10 / 255.f, 10 / 255.f, 10 / 255.f};
		mColors[DC_GRAY_TEXT] = eastl::array<float, 4>{240 / 255.f, 130 / 255.f, 130 / 255.f, 130 / 255.f};
		mColors[DC_HIGH_LIGHT] = eastl::array<float, 4>{101 / 255.f, 8 / 255.f, 36 / 255.f, 107 / 255.f};
		mColors[DC_HIGH_LIGHT_TEXT] = eastl::array<float, 4>{240 / 255.f, 255 / 255.f, 255 / 255.f, 255 / 255.f};
		mColors[DC_INACTIVE_BORDER] = eastl::array<float, 4>{101 / 255.f, 165 / 255.f, 165 / 255.f, 165 / 255.f};
		mColors[DC_INACTIVE_CAPTION] = eastl::array<float, 4>{255 / 255.f, 30 / 255.f, 30 / 255.f, 30 / 255.f};
		mColors[DC_TOOLTIP] = eastl::array<float, 4>{200 / 255.f, 0 / 255.f, 0 / 255.f, 0 / 255.f};
		mColors[DC_TOOLTIP_BACKGROUND] = eastl::array<float, 4>{200 / 255.f, 255 / 255.f, 255 / 255.f, 225 / 255.f};
		mColors[DC_SCROLLBAR] = eastl::array<float, 4>{101 / 255.f, 230 / 255.f, 230 / 255.f, 230 / 255.f};
		mColors[DC_WINDOW] = eastl::array<float, 4>{101 / 255.f, 255 / 255.f, 255 / 255.f, 255 / 255.f};
		mColors[DC_WINDOW_SYMBOL] = eastl::array<float, 4>{200 / 255.f, 10 / 255.f, 10 / 255.f, 10 / 255.f};
		mColors[DC_ICON_NORMAL] = eastl::array<float, 4>{200 / 255.f, 255 / 255.f, 255 / 255.f, 255 / 255.f};
		mColors[DC_ICON_HIGH_LIGHT] = eastl::array<float, 4>{200 / 255.f, 8 / 255.f, 36 / 255.f, 107 / 255.f};
		mColors[DC_GRAY_WINDOW_SYMBOL] = eastl::array<float, 4>{240 / 255.f, 100 / 255.f, 100 / 255.f, 100 / 255.f};
		mColors[DC_EDITABLE] = eastl::array<float, 4>{255 / 255.f, 255 / 255.f, 255 / 255.f, 255 / 255.f};
		mColors[DC_GRAY_EDITABLE] = eastl::array<float, 4>{255 / 255.f, 120 / 255.f, 120 / 255.f, 120 / 255.f};
		mColors[DC_FOCUSED_EDITABLE] = eastl::array<float, 4>{255 / 255.f, 240 / 255.f, 240 / 255.f, 255 / 255.f};


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
		/*
		//0x80a6a8af
		mColors[DC_3D_DARK_SHADOW] 	=	0x60767982;
		//Colors[DC_3D_FACE]			=	0xc0c9ccd4;		// tab background
		mColors[DC_3D_FACE]			=	0xc0cbd2d9;		// tab background
		mColors[DC_3D_SHADOW]			=	0x50e4e8f1;		// tab background, and left-top highlight
		mColors[DC_3D_HIGH_LIGHT]		=	0x40c7ccdc;
		mColors[DC_3D_LIGHT]			=	0x802e313a;
		mColors[DC_ACTIVE_BORDER]		=	0x80404040;		// window title
		mColors[DC_ACTIVE_CAPTION] 	=	0xffd0d0d0;
		mColors[DC_APP_WORKSPACE]		=	0xc0646464;		// unused
		mColors[DC_BUTTON_TEXT]		=	0xd0161616;
		mColors[DC_GRAY_TEXT]			=	0x3c141414;
		mColors[DC_HIGH_LIGHT]			=	0x6c606060;
		mColors[DC_HIGH_LIGHT_TEXT]	=	0xd0e0e0e0;
		mColors[DC_INACTIVE_BORDER]	=	0xf0a5a5a5;
		mColors[DC_INACTIVE_CAPTION]	=	0xffd2d2d2;
		mColors[DC_TOOLTIP]			=	0xf00f2033;
		mColors[DC_TOOLTIP_BACKGROUND]	= 	0xc0cbd2d9;
		mColors[DC_SCROLLBAR]			= 	0xf0e0e0e0;
		mColors[DC_WINDOW]				= 	0xf0f0f0f0;
		mColors[DC_WINDOW_SYMBOL]		= 	0xd0161616;
		mColors[DC_ICON_NORMAL]			= 	0xd0161616;
		mColors[DC_ICON_HIGH_LIGHT]	= 	0xd0606060;
		mColors[DC_GRAY_WINDOW_SYMBOL] = 	0x3c101010;
		mColors[DC_EDITABLE] 			= 	0xf0ffffff;
		mColors[DC_GRAY_EDITABLE]		= 	0xf0cccccc;
		mColors[DC_FOCUSED_EDITABLE]	= 	0xf0fffff0;
		*/
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

	mIcons[DI_WINDOW_MAXIMIZE] = L"Art/UserControl/appbar.window.maximize.png";
	mIcons[DI_WINDOW_RESTORE] = L"Art/UserControl/appbar.window.restore.png";
	mIcons[DI_WINDOW_CLOSE] = L"Art/UserControl/appbar.close.png";
	mIcons[DI_WINDOW_MINIMIZE] = L"Art/UserControl/appbar.window.minimize.png";
	mIcons[DI_WINDOW_RESIZE] = L"Art/UserControl/appbar.window.restore.png";
	mIcons[DI_CURSOR_UP] = L"Art/UserControl/appbar.chevron.up.png";
	mIcons[DI_CURSOR_DOWN] = L"Art/UserControl/appbar.chevron.down.png";
	mIcons[DI_CURSOR_LEFT] = L"Art/UserControl/appbar.chevron.left.png";
	mIcons[DI_CURSOR_RIGHT] = L"Art/UserControl/appbar.chevron.right.png";
	mIcons[DI_MENU_MORE] = L"Art/UserControl/appbar.add.png";
	mIcons[DI_CHECK_BOX_CHECKED] = L"Art/UserControl/appbar.check.png";
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

	mUseGradient = (mType == STT_WINDOWS_METALLIC) || (mType == STT_BURNING_SKIN) ;
}


//! destructor
UISkin::~UISkin()
{

}


//! returns default color
eastl::array<float, 4> const UISkin::GetColor(UIDefaultColor color) const
{
	if ((unsigned int)color < DC_COUNT)
		return mColors[color];
	else
		return eastl::array<float, 4>();
}


//! sets a default color
void UISkin::SetColor(UIDefaultColor which, eastl::array<float, 4> newColor)
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
		return 0;
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
/**	Used for drawing for example buttons in normal state.
It uses the colors EGDC_3D_DARK_SHADOW, EGDC_3D_HIGH_LIGHT, EGDC_3D_SHADOW and
EGDC_3D_FACE for this. See EGUI_DEFAULT_COLOR for details.
\param rect: Defining area where to draw.
\param clip: Clip area.
\param element: Pointer to the element which wishes to draw this. This parameter
is usually not used by ISkin, but can be used for example by more complex
implementations to find out how to draw the part exactly. */
void UISkin::Draw3DButtonPaneStandard(const eastl::shared_ptr<BaseUIElement>& element,
	const eastl::shared_ptr<Visual>& visual, const RectangleBase<2, int>& r, const RectangleBase<2, int>* clip)
{
	if (!Renderer::Get())
		return;

	RectangleBase<2, int> rect = r;
	Vector2<int> dimension(clip->extent / 2);

	struct Vertex
	{
		Vector3<float> position;
		Vector4<float> color;
	};
	Vertex* vertex = visual->GetVertexBuffer()->Get<Vertex>();

	if (mUseGradient)
	{
		const eastl::array<float, 4> c1 = GetColor(DC_3D_FACE);
		const eastl::array<float, 4> c2 = GetColor(DC_3D_DARK_SHADOW);

		vertex[0].position = {
			(float)(rect.center[0] - dimension[0] - (rect.extent[0] / 2)) / dimension[0],
			(float)(dimension[1] - rect.center[1] - (rect.extent[1] / 2)) / dimension[1], 0.0f };
		vertex[0].color = { c1[0], c1[1], c1[2], 1.0f };
		vertex[1].position = {
			(float)(rect.center[0] - dimension[0] + (int)round(rect.extent[0] / 2.f)) / dimension[0],
			(float)(dimension[1] - rect.center[1] - (rect.extent[1] / 2)) / dimension[1], 0.0f };
		vertex[1].color = { c2[0], c2[1], c2[2], 1.0f };
		vertex[2].position = {
			(float)(rect.center[0] - dimension[0] - (rect.extent[0] / 2)) / dimension[0],
			(float)(dimension[1] - rect.center[1] + (int)round(rect.extent[1] / 2.f)) / dimension[1], 0.0f };
		vertex[2].color = { c1[0], c1[1], c1[2], 1.0f };
		vertex[3].position = {
			(float)(rect.center[0] - dimension[0] + (int)round(rect.extent[0] / 2.f)) / dimension[0],
			(float)(dimension[1] - rect.center[1] + (int)round(rect.extent[1] / 2.f)) / dimension[1], 0.0f };
		vertex[3].color = { c1[0], c1[1], c1[2], 1.0f };
	}
	else
	{
		const eastl::array<float, 4> color = GetColor(DC_3D_FACE);

		vertex[0].position = {
			(float)(rect.center[0] - dimension[0] - (rect.extent[0] / 2)) / dimension[0],
			(float)(dimension[1] - rect.center[1] - (rect.extent[1] / 2)) / dimension[1], 0.0f };
		vertex[0].color = { color[0], color[1], color[2], 1.0f };
		vertex[1].position = {
			(float)(rect.center[0] - dimension[0] + (int)round(rect.extent[0] / 2.f)) / dimension[0],
			(float)(dimension[1] - rect.center[1] - (rect.extent[1] / 2)) / dimension[1], 0.0f };
		vertex[1].color = { color[0], color[1], color[2], 1.0f };
		vertex[2].position = {
			(float)(rect.center[0] - dimension[0] - (rect.extent[0] / 2)) / dimension[0],
			(float)(dimension[1] - rect.center[1] + (int)round(rect.extent[1] / 2.f)) / dimension[1], 0.0f };
		vertex[2].color = { color[0], color[1], color[2], 1.0f };
		vertex[3].position = {
			(float)(rect.center[0] - dimension[0] + (int)round(rect.extent[0] / 2.f)) / dimension[0],
			(float)(dimension[1] - rect.center[1] + (int)round(rect.extent[1] / 2.f)) / dimension[1], 0.0f };
		vertex[3].color = { color[0], color[1], color[2], 1.0f };
	}

	// Create the geometric object for drawing.
	Renderer::Get()->Draw(visual);
}


//! draws a pressed 3d button pane
/**	Used for drawing for example buttons in pressed state.
It uses the colors EGDC_3D_DARK_SHADOW, EGDC_3D_HIGH_LIGHT, EGDC_3D_SHADOW and
EGDC_3D_FACE for this. See EGUI_DEFAULT_COLOR for details.
\param rect: Defining area where to draw.
\param clip: Clip area.
\param element: Pointer to the element which wishes to draw this. This parameter
is usually not used by ISkin, but can be used for example by more complex
implementations to find out how to draw the part exactly. */
void UISkin::Draw3DButtonPanePressed(const eastl::shared_ptr<BaseUIElement>& element,
	const eastl::shared_ptr<Visual>& visual, const RectangleBase<2, int>& r, const RectangleBase<2, int>* clip)
{
	if (!Renderer::Get())
		return;

	RectangleBase<2, int> rect = r;
	Vector2<int> dimension(clip->extent / 2);

	struct Vertex
	{
		Vector3<float> position;
		Vector4<float> color;
	};
	Vertex* vertex = visual->GetVertexBuffer()->Get<Vertex>();

	if (mUseGradient)
	{
		const eastl::array<float, 4> c1 = GetColor(DC_3D_FACE);
		const eastl::array<float, 4> c2 = GetColor(DC_3D_DARK_SHADOW);

		vertex[0].position = {
			(float)(rect.center[0] - dimension[0] - (rect.extent[0] / 2)) / dimension[0],
			(float)(dimension[1] - rect.center[1] - (rect.extent[1] / 2)) / dimension[1], 0.0f };
		vertex[0].color = { c1[0], c1[1], c1[2], 1.0f };
		vertex[1].position = {
			(float)(rect.center[0] - dimension[0] + (int)round(rect.extent[0] / 2.f)) / dimension[0],
			(float)(dimension[1] - rect.center[1] - (rect.extent[1] / 2)) / dimension[1], 0.0f };
		vertex[1].color = { c2[0], c2[1], c2[2], 1.0f };
		vertex[2].position = {
			(float)(rect.center[0] - dimension[0] - (rect.extent[0] / 2)) / dimension[0],
			(float)(dimension[1] - rect.center[1] + (int)round(rect.extent[1] / 2.f)) / dimension[1], 0.0f };
		vertex[2].color = { c1[0], c1[1], c1[2], 1.0f };
		vertex[3].position = {
			(float)(rect.center[0] - dimension[0] + (int)round(rect.extent[0] / 2.f)) / dimension[0],
			(float)(dimension[1] - rect.center[1] + (int)round(rect.extent[1] / 2.f)) / dimension[1], 0.0f };
		vertex[3].color = { c1[0], c1[1], c1[2], 1.0f };
	}
	else
	{
		const eastl::array<float, 4> color = GetColor(DC_3D_FACE);

		vertex[0].position = {
			(float)(rect.center[0] - dimension[0] - (rect.extent[0] / 2)) / dimension[0],
			(float)(dimension[1] - rect.center[1] - (rect.extent[1] / 2)) / dimension[1], 0.0f };
		vertex[0].color = { color[0], color[1], color[2], 1.0f };
		vertex[1].position = {
			(float)(rect.center[0] - dimension[0] + (int)round(rect.extent[0] / 2.f)) / dimension[0],
			(float)(dimension[1] - rect.center[1] - (rect.extent[1] / 2)) / dimension[1], 0.0f };
		vertex[1].color = { color[0], color[1], color[2], 1.0f };
		vertex[2].position = {
			(float)(rect.center[0] - dimension[0] - (rect.extent[0] / 2)) / dimension[0],
			(float)(dimension[1] - rect.center[1] + (int)round(rect.extent[1] / 2.f)) / dimension[1], 0.0f };
		vertex[2].color = { color[0], color[1], color[2], 1.0f };
		vertex[3].position = {
			(float)(rect.center[0] - dimension[0] + (int)round(rect.extent[0] / 2.f)) / dimension[0],
			(float)(dimension[1] - rect.center[1] + (int)round(rect.extent[1] / 2.f)) / dimension[1], 0.0f };
		vertex[3].color = { color[0], color[1], color[2], 1.0f };
	}

	// Create the geometric object for drawing.
	Renderer::Get()->Draw(visual);
}


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
void UISkin::Draw3DSunkenPane(const eastl::shared_ptr<BaseUIElement>& element, 
	eastl::array<float, 4> bgcolor, bool flat, bool fillBackGround,
	const eastl::shared_ptr<Visual>& visual, const RectangleBase<2, int>& r, 
	const RectangleBase<2, int>* clip)
{
	if (!Renderer::Get())
		return;

	RectangleBase<2, int> rect = r;
	Vector2<int> dimension(clip->extent / 2);

	struct Vertex
	{
		Vector3<float> position;
		Vector4<float> color;
	};
	Vertex* vertex = visual->GetVertexBuffer()->Get<Vertex>();

	if (mUseGradient)
	{
		const eastl::array<float, 4> c1 = GetColor(DC_3D_FACE);
		const eastl::array<float, 4> c2 = GetColor(DC_3D_DARK_SHADOW);

		vertex[0].position = {
			(float)(rect.center[0] - dimension[0] - (rect.extent[0] / 2)) / dimension[0],
			(float)(dimension[1] - rect.center[1] - (rect.extent[1] / 2)) / dimension[1], 0.0f };
		vertex[0].color = { c1[0], c1[1], c1[2], 1.0f };
		vertex[1].position = {
			(float)(rect.center[0] - dimension[0] + (int)round(rect.extent[0] / 2.f)) / dimension[0],
			(float)(dimension[1] - rect.center[1] - (rect.extent[1] / 2)) / dimension[1], 0.0f };
		vertex[1].color = { c2[0], c2[1], c2[2], 1.0f };
		vertex[2].position = {
			(float)(rect.center[0] - dimension[0] - (rect.extent[0] / 2)) / dimension[0],
			(float)(dimension[1] - rect.center[1] + (int)round(rect.extent[1] / 2.f)) / dimension[1], 0.0f };
		vertex[2].color = { c1[0], c1[1], c1[2], 1.0f };
		vertex[3].position = {
			(float)(rect.center[0] - dimension[0] + (int)round(rect.extent[0] / 2.f)) / dimension[0],
			(float)(dimension[1] - rect.center[1] + (int)round(rect.extent[1] / 2.f)) / dimension[1], 0.0f };
		vertex[3].color = { c1[0], c1[1], c1[2], 1.0f };
	}
	else
	{
		const eastl::array<float, 4> color = GetColor(DC_3D_FACE);

		vertex[0].position = {
			(float)(rect.center[0] - dimension[0] - (rect.extent[0] / 2)) / dimension[0],
			(float)(dimension[1] - rect.center[1] - (rect.extent[1] / 2)) / dimension[1], 0.0f };
		vertex[0].color = { color[0], color[1], color[2], 1.0f };
		vertex[1].position = {
			(float)(rect.center[0] - dimension[0] + (int)round(rect.extent[0] / 2.f)) / dimension[0],
			(float)(dimension[1] - rect.center[1] - (rect.extent[1] / 2)) / dimension[1], 0.0f };
		vertex[1].color = { color[0], color[1], color[2], 1.0f };
		vertex[2].position = {
			(float)(rect.center[0] - dimension[0] - (rect.extent[0] / 2)) / dimension[0],
			(float)(dimension[1] - rect.center[1] + (int)round(rect.extent[1] / 2.f)) / dimension[1], 0.0f };
		vertex[2].color = { color[0], color[1], color[2], 1.0f };
		vertex[3].position = {
			(float)(rect.center[0] - dimension[0] + (int)round(rect.extent[0] / 2.f)) / dimension[0],
			(float)(dimension[1] - rect.center[1] + (int)round(rect.extent[1] / 2.f)) / dimension[1], 0.0f };
		vertex[3].color = { color[0], color[1], color[2], 1.0f };
	}

	// Create the geometric object for drawing.
	Renderer::Get()->Draw(visual);
}


//! draws a window background
// return where to draw title bar text.
RectangleBase<2, int> UISkin::Draw3DWindowBackground(const eastl::shared_ptr<BaseUIElement>& element, 
	const eastl::shared_ptr<Visual>& visualBackground, const eastl::shared_ptr<Visual>& visualTitle, 
	bool drawTitleBar, eastl::array<float, 4> titleBarColor, const RectangleBase<2, int>& r, 
	const RectangleBase<2, int>* clip, RectangleBase<2, int>* checkClientArea)
{
	if (!Renderer::Get())
	{
		if ( checkClientArea )
		{
			*checkClientArea = r;
		}
		return r;
	}
	
	RectangleBase<2, int> rect = r;
	Vector2<int> dimension(clip->extent / 2);

	struct Vertex
	{
		Vector3<float> position;
		Vector4<float> color;
	};
	Vertex* vertex = visualBackground->GetVertexBuffer()->Get<Vertex>();

	const eastl::array<float, 4> c2 = GetColor(DC_3D_SHADOW);
	const eastl::array<float, 4> c1 = GetColor(DC_3D_FACE);

	vertex[0].position = {
		(float)(rect.center[0] - dimension[0] - (rect.extent[0] / 2)) / dimension[0],
		(float)(dimension[1] - rect.center[1] - (rect.extent[1] / 2)) / dimension[1], 0.0f };
	vertex[0].color = { c1[0], c1[1], c1[2], 1.0f };
	vertex[1].position = {
		(float)(rect.center[0] - dimension[0] + (int)round(rect.extent[0] / 2.f)) / dimension[0],
		(float)(dimension[1] - rect.center[1] - (rect.extent[1] / 2)) / dimension[1], 0.0f };
	vertex[1].color = { c2[0], c2[1], c2[2], 1.0f };
	vertex[2].position = {
		(float)(rect.center[0] - dimension[0] - (rect.extent[0] / 2)) / dimension[0],
		(float)(dimension[1] - rect.center[1] + (int)round(rect.extent[1] / 2.f)) / dimension[1], 0.0f };
	vertex[2].color = { c1[0], c1[1], c1[2], 1.0f };
	vertex[3].position = {
		(float)(rect.center[0] - dimension[0] + (int)round(rect.extent[0] / 2.f)) / dimension[0],
		(float)(dimension[1] - rect.center[1] + (int)round(rect.extent[1] / 2.f)) / dimension[1], 0.0f };
	vertex[3].color = { c1[0], c1[1], c1[2], 1.0f };

	// Create the geometric object for drawing.
	Renderer::Get()->Draw(visualBackground);

	// title bar
	rect = r;
	rect.extent[0] -= 2;
	rect.center[1] += 1;
	rect.extent[1] -= 2;
	rect.center[1] = (rect.center[1] - rect.extent[1] / 2) + (GetSize(DS_WINDOW_BUTTON_WIDTH) + 2) / 2;
	rect.extent[1] = GetSize(DS_WINDOW_BUTTON_WIDTH) + 2;

	if (drawTitleBar)
	{
		if (!checkClientArea)
		{
			Vertex* vertex = visualTitle->GetVertexBuffer()->Get<Vertex>();

			// draw title bar
			vertex[0].position = {
				(float)(rect.center[0] - dimension[0] - (rect.extent[0] / 2)) / dimension[0],
				(float)(dimension[1] - rect.center[1] - (rect.extent[1] / 2)) / dimension[1], 0.0f };
			vertex[0].color = { titleBarColor[0], titleBarColor[1], titleBarColor[2], 1.0f };
			vertex[1].position = {
				(float)(rect.center[0] - dimension[0] + (int)round(rect.extent[0] / 2.f)) / dimension[0],
				(float)(dimension[1] - rect.center[1] - (rect.extent[1] / 2)) / dimension[1], 0.0f };
			vertex[1].color = { titleBarColor[0], titleBarColor[1], titleBarColor[2], 1.0f };
			vertex[2].position = {
				(float)(rect.center[0] - dimension[0] - (rect.extent[0] / 2)) / dimension[0],
				(float)(dimension[1] - rect.center[1] + (int)round(rect.extent[1] / 2.f)) / dimension[1], 0.0f };
			vertex[2].color = { titleBarColor[0], titleBarColor[1], titleBarColor[2], 1.0f };
			vertex[3].position = {
				(float)(rect.center[0] - dimension[0] + (int)round(rect.extent[0] / 2.f)) / dimension[0],
				(float)(dimension[1] - rect.center[1] + (int)round(rect.extent[1] / 2.f)) / dimension[1], 0.0f };
			vertex[3].color = { titleBarColor[0], titleBarColor[1], titleBarColor[2], 1.0f };

			// Create the geometric object for drawing.
			Renderer::Get()->Draw(visualTitle);
		}
		else
		{
			(*checkClientArea).center[1] = (*checkClientArea).center[1] + (int)round((*checkClientArea).extent[1] / 2.f);
			(*checkClientArea).extent[1] = 0;
		}
	}
	return rect;
}


//! draws a standard 3d menu pane
/**	Used for drawing for menus and context menus.
It uses the colors EGDC_3D_DARK_SHADOW, EGDC_3D_HIGH_LIGHT, EGDC_3D_SHADOW and
EGDC_3D_FACE for this. See EGUI_DEFAULT_COLOR for details.
\param element: Pointer to the element which wishes to draw this. This parameter
is usually not used by ISkin, but can be used for example by more complex
implementations to find out how to draw the part exactly.
\param rect: Defining area where to draw.
\param clip: Clip area.	*/
void UISkin::Draw3DMenuPane(const eastl::shared_ptr<BaseUIElement>& element,
	const eastl::shared_ptr<Visual>& visual, const RectangleBase<2, int>& r, const RectangleBase<2, int>* clip)
{
	if (!Renderer::Get())
		return;

	RectangleBase<2, int> rect = r;
	Vector2<int> dimension(clip->extent / 2);

	struct Vertex
	{
		Vector3<float> position;
		Vector4<float> color;
	};
	Vertex* vertex = visual->GetVertexBuffer()->Get<Vertex>();

	if (mUseGradient)
	{
		const eastl::array<float, 4> c1 = GetColor(DC_3D_FACE);
		const eastl::array<float, 4> c2 = GetColor(DC_3D_DARK_SHADOW);

		vertex[0].position = {
			(float)(rect.center[0] - dimension[0] - (rect.extent[0] / 2)) / dimension[0],
			(float)(dimension[1] - rect.center[1] - (rect.extent[1] / 2)) / dimension[1], 0.0f };
		vertex[0].color = { c1[0], c1[1], c1[2], 1.0f };
		vertex[1].position = {
			(float)(rect.center[0] - dimension[0] + (int)round(rect.extent[0] / 2.f)) / dimension[0],
			(float)(dimension[1] - rect.center[1] - (rect.extent[1] / 2)) / dimension[1], 0.0f };
		vertex[1].color = { c2[0], c2[1], c2[2], 1.0f };
		vertex[2].position = {
			(float)(rect.center[0] - dimension[0] - (rect.extent[0] / 2)) / dimension[0],
			(float)(dimension[1] - rect.center[1] + (int)round(rect.extent[1] / 2.f)) / dimension[1], 0.0f };
		vertex[2].color = { c1[0], c1[1], c1[2], 1.0f };
		vertex[3].position = {
			(float)(rect.center[0] - dimension[0] + (int)round(rect.extent[0] / 2.f)) / dimension[0],
			(float)(dimension[1] - rect.center[1] + (int)round(rect.extent[1] / 2.f)) / dimension[1], 0.0f };
		vertex[3].color = { c1[0], c1[1], c1[2], 1.0f };
	}
	else
	{
		const eastl::array<float, 4> color = GetColor(DC_3D_FACE);

		vertex[0].position = {
			(float)(rect.center[0] - dimension[0] - (rect.extent[0] / 2)) / dimension[0],
			(float)(dimension[1] - rect.center[1] - (rect.extent[1] / 2)) / dimension[1], 0.0f };
		vertex[0].color = { color[0], color[1], color[2], 1.0f };
		vertex[1].position = {
			(float)(rect.center[0] - dimension[0] + (int)round(rect.extent[0] / 2.f)) / dimension[0],
			(float)(dimension[1] - rect.center[1] - (rect.extent[1] / 2)) / dimension[1], 0.0f };
		vertex[1].color = { color[0], color[1], color[2], 1.0f };
		vertex[2].position = {
			(float)(rect.center[0] - dimension[0] - (rect.extent[0] / 2)) / dimension[0],
			(float)(dimension[1] - rect.center[1] + (int)round(rect.extent[1] / 2.f)) / dimension[1], 0.0f };
		vertex[2].color = { color[0], color[1], color[2], 1.0f };
		vertex[3].position = {
			(float)(rect.center[0] - dimension[0] + (int)round(rect.extent[0] / 2.f)) / dimension[0],
			(float)(dimension[1] - rect.center[1] + (int)round(rect.extent[1] / 2.f)) / dimension[1], 0.0f };
		vertex[3].color = { color[0], color[1], color[2], 1.0f };
	}

	// Create the geometric object for drawing.
	Renderer::Get()->Draw(visual);
}


//! draws a standard 3d tool bar
/**	Used for drawing for toolbars and menus.
\param element: Pointer to the element which wishes to draw this. This parameter
is usually not used by ISkin, but can be used for example by more complex
implementations to find out how to draw the part exactly.
\param rect: Defining area where to draw.
\param clip: Clip area.	*/
void UISkin::Draw3DToolBar(const eastl::shared_ptr<BaseUIElement>& element,
	const eastl::shared_ptr<Visual>& visual, const RectangleBase<2, int>& r, const RectangleBase<2, int>* clip)
{
	if (!Renderer::Get())
		return;

	RectangleBase<2, int> rect = r;
	Vector2<int> dimension(clip->extent / 2);

	struct Vertex
	{
		Vector3<float> position;
		Vector4<float> color;
	};
	Vertex* vertex = visual->GetVertexBuffer()->Get<Vertex>();

	if (mUseGradient)
	{
		const eastl::array<float, 4> c1 = GetColor(DC_3D_FACE);
		const eastl::array<float, 4> c2 = GetColor(DC_3D_DARK_SHADOW);

		vertex[0].position = {
			(float)(rect.center[0] - dimension[0] - (rect.extent[0] / 2)) / dimension[0],
			(float)(dimension[1] - rect.center[1] - (rect.extent[1] / 2)) / dimension[1], 0.0f };
		vertex[0].color = { c1[0], c1[1], c1[2], 1.0f };
		vertex[1].position = {
			(float)(rect.center[0] - dimension[0] + (int)round(rect.extent[0] / 2.f)) / dimension[0],
			(float)(dimension[1] - rect.center[1] - (rect.extent[1] / 2)) / dimension[1], 0.0f };
		vertex[1].color = { c2[0], c2[1], c2[2], 1.0f };
		vertex[2].position = {
			(float)(rect.center[0] - dimension[0] - (rect.extent[0] / 2)) / dimension[0],
			(float)(dimension[1] - rect.center[1] + (int)round(rect.extent[1] / 2.f)) / dimension[1], 0.0f };
		vertex[2].color = { c1[0], c1[1], c1[2], 1.0f };
		vertex[3].position = {
			(float)(rect.center[0] - dimension[0] + (int)round(rect.extent[0] / 2.f)) / dimension[0],
			(float)(dimension[1] - rect.center[1] + (int)round(rect.extent[1] / 2.f)) / dimension[1], 0.0f };
		vertex[3].color = { c1[0], c1[1], c1[2], 1.0f };
	}
	else
	{
		const eastl::array<float, 4> color = GetColor(DC_3D_FACE);

		vertex[0].position = {
			(float)(rect.center[0] - dimension[0] - (rect.extent[0] / 2)) / dimension[0],
			(float)(dimension[1] - rect.center[1] - (rect.extent[1] / 2)) / dimension[1], 0.0f };
		vertex[0].color = { color[0], color[1], color[2], 1.0f };
		vertex[1].position = {
			(float)(rect.center[0] - dimension[0] + (int)round(rect.extent[0] / 2.f)) / dimension[0],
			(float)(dimension[1] - rect.center[1] - (rect.extent[1] / 2)) / dimension[1], 0.0f };
		vertex[1].color = { color[0], color[1], color[2], 1.0f };
		vertex[2].position = {
			(float)(rect.center[0] - dimension[0] - (rect.extent[0] / 2)) / dimension[0],
			(float)(dimension[1] - rect.center[1] + (int)round(rect.extent[1] / 2.f)) / dimension[1], 0.0f };
		vertex[2].color = { color[0], color[1], color[2], 1.0f };
		vertex[3].position = {
			(float)(rect.center[0] - dimension[0] + (int)round(rect.extent[0] / 2.f)) / dimension[0],
			(float)(dimension[1] - rect.center[1] + (int)round(rect.extent[1] / 2.f)) / dimension[1], 0.0f };
		vertex[3].color = { color[0], color[1], color[2], 1.0f };
	}

	// Create the geometric object for drawing.
	Renderer::Get()->Draw(visual);
}


//! draws a tab button
/**	Used for drawing for tab buttons on top of tabs.
\param element: Pointer to the element which wishes to draw this. This parameter
is usually not used by ISkin, but can be used for example by more complex
implementations to find out how to draw the part exactly.
\param active: Specifies if the tab is currently active.
\param rect: Defining area where to draw.
\param clip: Clip area.	*/
void UISkin::Draw3DTabButton(const eastl::shared_ptr<BaseUIElement>& element, bool active,
	const eastl::shared_ptr<Visual>& visual, const RectangleBase<2, int>& r, const RectangleBase<2, int>* clip, 
	UIAlignment alignment)
{
	if (!Renderer::Get())
		return;

	RectangleBase<2, int> rect = r;
	Vector2<int> dimension(clip->extent / 2);

	struct Vertex
	{
		Vector3<float> position;
		Vector4<float> color;
	};
	Vertex* vertex = visual->GetVertexBuffer()->Get<Vertex>();

	if (mUseGradient)
	{
		const eastl::array<float, 4> c1 = GetColor(DC_3D_FACE);
		const eastl::array<float, 4> c2 = GetColor(DC_3D_DARK_SHADOW);

		vertex[0].position = {
			(float)(rect.center[0] - dimension[0] - (rect.extent[0] / 2)) / dimension[0],
			(float)(dimension[1] - rect.center[1] - (rect.extent[1] / 2)) / dimension[1], 0.0f };
		vertex[0].color = { c1[0], c1[1], c1[2], 1.0f };
		vertex[1].position = {
			(float)(rect.center[0] - dimension[0] + (int)round(rect.extent[0] / 2.f)) / dimension[0],
			(float)(dimension[1] - rect.center[1] - (rect.extent[1] / 2)) / dimension[1], 0.0f };
		vertex[1].color = { c2[0], c2[1], c2[2], 1.0f };
		vertex[2].position = {
			(float)(rect.center[0] - dimension[0] - (rect.extent[0] / 2)) / dimension[0],
			(float)(dimension[1] - rect.center[1] + (int)round(rect.extent[1] / 2.f)) / dimension[1], 0.0f };
		vertex[2].color = { c1[0], c1[1], c1[2], 1.0f };
		vertex[3].position = {
			(float)(rect.center[0] - dimension[0] + (int)round(rect.extent[0] / 2.f)) / dimension[0],
			(float)(dimension[1] - rect.center[1] + (int)round(rect.extent[1] / 2.f)) / dimension[1], 0.0f };
		vertex[3].color = { c1[0], c1[1], c1[2], 1.0f };
	}
	else
	{
		const eastl::array<float, 4> color = GetColor(DC_3D_FACE);

		vertex[0].position = {
			(float)(rect.center[0] - dimension[0] - (rect.extent[0] / 2)) / dimension[0],
			(float)(dimension[1] - rect.center[1] - (rect.extent[1] / 2)) / dimension[1], 0.0f };
		vertex[0].color = { color[0], color[1], color[2], 1.0f };
		vertex[1].position = {
			(float)(rect.center[0] - dimension[0] + (int)round(rect.extent[0] / 2.f)) / dimension[0],
			(float)(dimension[1] - rect.center[1] - (rect.extent[1] / 2)) / dimension[1], 0.0f };
		vertex[1].color = { color[0], color[1], color[2], 1.0f };
		vertex[2].position = {
			(float)(rect.center[0] - dimension[0] - (rect.extent[0] / 2)) / dimension[0],
			(float)(dimension[1] - rect.center[1] + (int)round(rect.extent[1] / 2.f)) / dimension[1], 0.0f };
		vertex[2].color = { color[0], color[1], color[2], 1.0f };
		vertex[3].position = {
			(float)(rect.center[0] - dimension[0] + (int)round(rect.extent[0] / 2.f)) / dimension[0],
			(float)(dimension[1] - rect.center[1] + (int)round(rect.extent[1] / 2.f)) / dimension[1], 0.0f };
		vertex[3].color = { color[0], color[1], color[2], 1.0f };
	}

	// Create the geometric object for drawing.
	Renderer::Get()->Draw(visual);
}


//! draws a tab control body
/**	\param element: Pointer to the element which wishes to draw this. This parameter
is usually not used by ISkin, but can be used for example by more complex
implementations to find out how to draw the part exactly.
\param border: Specifies if the border should be drawn.
\param background: Specifies if the background should be drawn.
\param rect: Defining area where to draw.
\param clip: Clip area.	*/
void UISkin::Draw3DTabBody(const eastl::shared_ptr<BaseUIElement>& element, bool border, bool background,
	const eastl::shared_ptr<Visual>& visual, const RectangleBase<2, int>& r, const RectangleBase<2, int>* clip, 
	int tabHeight, UIAlignment alignment)
{
	if (!Renderer::Get())
		return;

	RectangleBase<2, int> rect = r;
	Vector2<int> dimension(clip->extent / 2);

	struct Vertex
	{
		Vector3<float> position;
		Vector4<float> color;
	};
	Vertex* vertex = visual->GetVertexBuffer()->Get<Vertex>();

	if (mUseGradient)
	{
		const eastl::array<float, 4> c1 = GetColor(DC_3D_FACE);
		const eastl::array<float, 4> c2 = GetColor(DC_3D_DARK_SHADOW);

		vertex[0].position = {
			(float)(rect.center[0] - dimension[0] - (rect.extent[0] / 2)) / dimension[0],
			(float)(dimension[1] - rect.center[1] - (rect.extent[1] / 2)) / dimension[1], 0.0f };
		vertex[0].color = { c1[0], c1[1], c1[2], 1.0f };
		vertex[1].position = {
			(float)(rect.center[0] - dimension[0] + (int)round(rect.extent[0] / 2.f)) / dimension[0],
			(float)(dimension[1] - rect.center[1] - (rect.extent[1] / 2)) / dimension[1], 0.0f };
		vertex[1].color = { c2[0], c2[1], c2[2], 1.0f };
		vertex[2].position = {
			(float)(rect.center[0] - dimension[0] - (rect.extent[0] / 2)) / dimension[0],
			(float)(dimension[1] - rect.center[1] + (int)round(rect.extent[1] / 2.f)) / dimension[1], 0.0f };
		vertex[2].color = { c1[0], c1[1], c1[2], 1.0f };
		vertex[3].position = {
			(float)(rect.center[0] - dimension[0] + (int)round(rect.extent[0] / 2.f)) / dimension[0],
			(float)(dimension[1] - rect.center[1] + (int)round(rect.extent[1] / 2.f)) / dimension[1], 0.0f };
		vertex[3].color = { c1[0], c1[1], c1[2], 1.0f };
	}
	else
	{
		const eastl::array<float, 4> color = GetColor(DC_3D_FACE);

		vertex[0].position = {
			(float)(rect.center[0] - dimension[0] - (rect.extent[0] / 2)) / dimension[0],
			(float)(dimension[1] - rect.center[1] - (rect.extent[1] / 2)) / dimension[1], 0.0f };
		vertex[0].color = { color[0], color[1], color[2], 1.0f };
		vertex[1].position = {
			(float)(rect.center[0] - dimension[0] + (int)round(rect.extent[0] / 2.f)) / dimension[0],
			(float)(dimension[1] - rect.center[1] - (rect.extent[1] / 2)) / dimension[1], 0.0f };
		vertex[1].color = { color[0], color[1], color[2], 1.0f };
		vertex[2].position = {
			(float)(rect.center[0] - dimension[0] - (rect.extent[0] / 2)) / dimension[0],
			(float)(dimension[1] - rect.center[1] + (int)round(rect.extent[1] / 2.f)) / dimension[1], 0.0f };
		vertex[2].color = { color[0], color[1], color[2], 1.0f };
		vertex[3].position = {
			(float)(rect.center[0] - dimension[0] + (int)round(rect.extent[0] / 2.f)) / dimension[0],
			(float)(dimension[1] - rect.center[1] + (int)round(rect.extent[1] / 2.f)) / dimension[1], 0.0f };
		vertex[3].color = { color[0], color[1], color[2], 1.0f };
	}

	// Create the geometric object for drawing.
	Renderer::Get()->Draw(visual);
}


//! draws an icon, usually from the skin's sprite bank
/**	\param parent: Pointer to the element which wishes to draw this icon.
This parameter is usually not used by IGUISkin, but can be used for example
by more complex implementations to find out how to draw the part exactly.
\param icon: Specifies the icon to be drawn.
\param position: The position to draw the icon
\param starttime: The time at the start of the animation
\param currenttime: The present time, used to calculate the frame number
\param loop: Whether the animation should loop or not
\param clip: Clip area.	*/
void UISkin::DrawIcon(const eastl::shared_ptr<BaseUIElement>& element, UIDefaultIcon icon,
	const Vector2<int> position, const eastl::shared_ptr<Visual>& visual, const RectangleBase<2, int>* clip, 
	unsigned int starttime, unsigned int currenttime, bool loop)
{
	if (!mSpriteBank)
		return;

	bool gray = element && !element->IsEnabled();
	/*
	mSpriteBank->Draw2DSprite(mIcons[icon], mVisual,position, clip,
		mColors[gray? DC_GRAY_WINDOW_SYMBOL : DC_WINDOW_SYMBOL], starttime, currenttime, loop, true);
	*/
}


UISkinThemeType UISkin::GetType() const
{
	return mType;
}


//! draws a 2d rectangle.
void UISkin::Draw2DRectangle(const eastl::shared_ptr<BaseUIElement>& element, 
	const eastl::array<float, 4> &color, const eastl::shared_ptr<Visual>& visual, 
	const RectangleBase<2, int>& r, const RectangleBase<2, int>* clip)
{
	if (!Renderer::Get())
		return;

	RectangleBase<2, int> rect = r;
	Vector2<int> dimension(clip->extent / 2);

	struct Vertex
	{
		Vector3<float> position;
		Vector4<float> color;
	};
	Vertex* vertex = visual->GetVertexBuffer()->Get<Vertex>();

	if (mUseGradient)
	{
		const eastl::array<float, 4> c1 = GetColor(DC_3D_FACE);
		const eastl::array<float, 4> c2 = GetColor(DC_3D_DARK_SHADOW);

		vertex[0].position = {
			(float)(rect.center[0] - dimension[0] - (rect.extent[0] / 2)) / dimension[0],
			(float)(dimension[1] - rect.center[1] - (rect.extent[1] / 2)) / dimension[1], 0.0f };
		vertex[0].color = { c1[0], c1[1], c1[2], 1.0f };
		vertex[1].position = {
			(float)(rect.center[0] - dimension[0] + (int)round(rect.extent[0] / 2.f)) / dimension[0],
			(float)(dimension[1] - rect.center[1] - (rect.extent[1] / 2)) / dimension[1], 0.0f };
		vertex[1].color = { c2[0], c2[1], c2[2], 1.0f };
		vertex[2].position = {
			(float)(rect.center[0] - dimension[0] - (rect.extent[0] / 2)) / dimension[0],
			(float)(dimension[1] - rect.center[1] + (int)round(rect.extent[1] / 2.f)) / dimension[1], 0.0f };
		vertex[2].color = { c1[0], c1[1], c1[2], 1.0f };
		vertex[3].position = {
			(float)(rect.center[0] - dimension[0] + (int)round(rect.extent[0] / 2.f)) / dimension[0],
			(float)(dimension[1] - rect.center[1] + (int)round(rect.extent[1] / 2.f)) / dimension[1], 0.0f };
		vertex[3].color = { c1[0], c1[1], c1[2], 1.0f };
	}
	else
	{
		const eastl::array<float, 4> color = GetColor(DC_3D_FACE);

		vertex[0].position = {
			(float)(rect.center[0] - dimension[0] - (rect.extent[0] / 2)) / dimension[0],
			(float)(dimension[1] - rect.center[1] - (rect.extent[1] / 2)) / dimension[1], 0.0f };
		vertex[0].color = { color[0], color[1], color[2], 1.0f };
		vertex[1].position = {
			(float)(rect.center[0] - dimension[0] + (int)round(rect.extent[0] / 2.f)) / dimension[0],
			(float)(dimension[1] - rect.center[1] - (rect.extent[1] / 2)) / dimension[1], 0.0f };
		vertex[1].color = { color[0], color[1], color[2], 1.0f };
		vertex[2].position = {
			(float)(rect.center[0] - dimension[0] - (rect.extent[0] / 2)) / dimension[0],
			(float)(dimension[1] - rect.center[1] + (int)round(rect.extent[1] / 2.f)) / dimension[1], 0.0f };
		vertex[2].color = { color[0], color[1], color[2], 1.0f };
		vertex[3].position = {
			(float)(rect.center[0] - dimension[0] + (int)round(rect.extent[0] / 2.f)) / dimension[0],
			(float)(dimension[1] - rect.center[1] + (int)round(rect.extent[1] / 2.f)) / dimension[1], 0.0f };
		vertex[3].color = { color[0], color[1], color[2], 1.0f };
	}

	// Create the geometric object for drawing.
	Renderer::Get()->Draw(visual);
}