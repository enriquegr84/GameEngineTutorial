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

	mIcons[DI_WINDOW_MAXIMIZE] = 0;
	mIcons[DI_WINDOW_RESTORE] = 0;
	mIcons[DI_WINDOW_CLOSE] = 0;
	mIcons[DI_WINDOW_MINIMIZE] = 0;
	mIcons[DI_CURSOR_UP] = 0;
	mIcons[DI_CURSOR_DOWN] = 0;
	mIcons[DI_CURSOR_LEFT] = 0;
	mIcons[DI_CURSOR_RIGHT] = 0;
	mIcons[DI_MENU_MORE] = 0;
	mIcons[DI_CHECK_BOX_CHECKED] = 0;
	mIcons[DI_DROP_DOWN] = 0;
	mIcons[DI_SMALL_CURSOR_UP] = 0;
	mIcons[DI_SMALL_CURSOR_DOWN] = 0;
	mIcons[DI_RADIO_BUTTON_CHECKED] = 0;
	mIcons[DI_MORE_LEFT] = 0;
	mIcons[DI_MORE_RIGHT] = 0;
	mIcons[DI_MORE_UP] = 0;
	mIcons[DI_MORE_DOWN] = 0;
	mIcons[DI_WINDOW_RESIZE] = 0;
	mIcons[DI_EXPAND] = 0;
	mIcons[DI_COLLAPSE] = 244;

	mIcons[DI_FILE] = 245;
	mIcons[DI_DIRECTORY] = 246;

	for (unsigned int i=0; i<DF_COUNT; ++i)
		mFonts[i] = 0;

	mUseGradient = (mType == STT_WINDOWS_METALLIC) || (mType == STT_BURNING_SKIN) ;

	eastl::string path = FileSystem::Get()->GetPath("Effects/BasicEffect.fx");
	mEffect = eastl::make_shared<BasicEffect>(ProgramFactory::Get(), path);
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
unsigned int UISkin::GetIcon(UIDefaultIcon icon) const
{
	if ((unsigned int)icon < DI_COUNT)
		return mIcons[icon];
	else
		return 0;
}


//! Sets a default icon
void UISkin::SetIcon(UIDefaultIcon icon, unsigned int index)
{
	if ((unsigned int)icon < DI_COUNT)
		mIcons[icon] = index;
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
					const RectangleBase<2, int>& r, const RectangleBase<2, int>* clip)
{
	if (!Renderer::Get())
		return;
	/*
	RectangleBase<2, int> rect = r;

	if ( mType == STT_BURNING_SKIN )
	{
		rect.extent[0] += 2;
		rect.extent[1] += 2;
		Draw3DSunkenPane(element,
			GetColor( DC_WINDOW ).GetInterpolated( 0xFFFFFFFF, 0.9f ),false, true, rect, clip);
		return;
	}

	Renderer::Get()->Draw2DRectangle(GetColor(DC_3D_DARK_SHADOW), rect, clip);

	rect.extent[0] += 1;
	rect.extent[1] += 1;
	rect.center[0] -= 1;
	rect.center[1] -= 1;
	Renderer::Get()->Draw2DRectangle(GetColor(DC_3D_HIGH_LIGHT), rect, clip);

	rect.extent[0] += 1;
	rect.extent[1] += 1;
	rect.center[0] += 1;
	rect.center[1] += 1;
	Renderer::Get()->Draw2DRectangle(GetColor(DC_3D_SHADOW), rect, clip);

	rect.extent[0] += 1;
	rect.extent[1] += 1;
	rect.center[0] -= 1;
	rect.center[1] -= 1;
	if (mUseGradient)
	{
		const eastl::array<float, 4> c1 = GetColor(DC_3D_FACE);
		const eastl::array<float, 4> c2 = c1.GetInterpolated(GetColor(DC_3D_DARK_SHADOW), 0.4f);
		Renderer::Get()->Draw2DRectangle(rect, c1, c1, c2, c2, clip);

	}
	else Renderer::Get()->Draw2DRectangle(GetColor(DC_3D_FACE), rect, clip);*/
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
				const RectangleBase<2, int>& r, const RectangleBase<2, int>* clip)
{
	if (!Renderer::Get())
		return;
	/*
	RectangleBase<2, int> rect = r;
	Renderer::Get()->Draw2DRectangle(GetColor(DC_3D_HIGH_LIGHT), rect, clip);

	rect.extent[0] += 1;
	rect.extent[1] += 1;
	rect.center[0] -= 1;
	rect.center[1] -= 1;
	Renderer::Get()->Draw2DRectangle(GetColor(DC_3D_DARK_SHADOW), rect, clip);

	rect.extent[0] += 1;
	rect.extent[1] += 1;
	rect.center[0] += 1;
	rect.center[1] += 1;
	Renderer::Get()->Draw2DRectangle(GetColor(DC_3D_SHADOW), rect, clip);

	rect.extent[0] += 1;
	rect.extent[1] += 1;
	rect.center[0] += 1;
	rect.center[1] += 1;
	if (!mUseGradient)
	{
		Renderer::Get()->Draw2DRectangle(GetColor(DC_3D_FACE), rect, clip);
	}
	else
	{
		const eastl::array<float, 4> c1 = GetColor(DC_3D_FACE);
		const eastl::array<float, 4> c2 = c1.GetInterpolated(GetColor(DC_3D_DARK_SHADOW), 0.4f);
		Renderer::Get()->Draw2DRectangle(rect, c1, c1, c2, c2, clip);
	}*/
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
void UISkin::Draw3DSunkenPane(const eastl::shared_ptr<BaseUIElement>& element, eastl::array<float, 4> bgcolor,
			bool flat, bool fillBackGround, const RectangleBase<2, int>& r, const RectangleBase<2, int>* clip)
{
	if (!Renderer::Get())
		return;
	/*
	RectangleBase<2, int> rect = r;

	if (fillBackGround)
		Renderer::Get()->Draw2DRectangle(bgcolor, rect, clip);

	if (flat)
	{
		// draw flat sunken pane

		rect.LowerRightCorner.Y = rect.UpperLeftCorner.Y + 1;
		Renderer::Get()->Draw2DRectangle(GetColor(DC_3D_SHADOW), rect, clip);	// top

		++rect.UpperLeftCorner.Y;
		rect.LowerRightCorner.Y = r.LowerRightCorner.Y;
		rect.LowerRightCorner.X = rect.UpperLeftCorner.X + 1;
		Renderer::Get()->Draw2DRectangle(GetColor(DC_3D_SHADOW), rect, clip);	// left

		rect = r;
		++rect.UpperLeftCorner.Y;
		rect.UpperLeftCorner.X = rect.LowerRightCorner.X - 1;
		Renderer::Get()->Draw2DRectangle(GetColor(DC_3D_HIGH_LIGHT), rect, clip);	// right

		rect = r;
		++rect.UpperLeftCorner.X;
		rect.UpperLeftCorner.Y = r.LowerRightCorner.Y - 1;
		--rect.LowerRightCorner.X;
		Renderer::Get()->Draw2DRectangle(GetColor(DC_3D_HIGH_LIGHT), rect, clip);	// bottom
	}
	else
	{
		// draw deep sunken pane
		rect.LowerRightCorner.Y = rect.UpperLeftCorner.Y + 1;
		Renderer::Get()->Draw2DRectangle(GetColor(DC_3D_SHADOW), rect, clip);	// top
		++rect.UpperLeftCorner.X;
		++rect.UpperLeftCorner.Y;
		--rect.LowerRightCorner.X;
		++rect.LowerRightCorner.Y;
		Renderer::Get()->Draw2DRectangle(GetColor(DC_3D_DARK_SHADOW), rect, clip);

		rect.UpperLeftCorner.X = r.UpperLeftCorner.X;
		rect.UpperLeftCorner.Y = r.UpperLeftCorner.Y+1;
		rect.LowerRightCorner.X = rect.UpperLeftCorner.X + 1;
		rect.LowerRightCorner.Y = r.LowerRightCorner.Y;
		Renderer::Get()->Draw2DRectangle(GetColor(DC_3D_SHADOW), rect, clip);	// left
		++rect.UpperLeftCorner.X;
		++rect.UpperLeftCorner.Y;
		++rect.LowerRightCorner.X;
		--rect.LowerRightCorner.Y;
		Renderer::Get()->Draw2DRectangle(GetColor(DC_3D_DARK_SHADOW), rect, clip);

		rect = r;
		rect.UpperLeftCorner.X = rect.LowerRightCorner.X - 1;
		++rect.UpperLeftCorner.Y;
		Renderer::Get()->Draw2DRectangle(GetColor(DC_3D_HIGH_LIGHT), rect, clip);	// right
		--rect.UpperLeftCorner.X;
		++rect.UpperLeftCorner.Y;
		--rect.LowerRightCorner.X;
		--rect.LowerRightCorner.Y;
		Renderer::Get()->Draw2DRectangle(GetColor(DC_3D_LIGHT), rect, clip);

		rect = r;
		++rect.UpperLeftCorner.X;
		rect.UpperLeftCorner.Y = r.LowerRightCorner.Y - 1;
		--rect.LowerRightCorner.X;
		Renderer::Get()->Draw2DRectangle(GetColor(DC_3D_HIGH_LIGHT), rect, clip);	// bottom
		++rect.UpperLeftCorner.X;
		--rect.UpperLeftCorner.Y;
		--rect.LowerRightCorner.X;
		--rect.LowerRightCorner.Y;
		Renderer::Get()->Draw2DRectangle(GetColor(DC_3D_LIGHT), rect, clip);
	}*/
}


//! draws a window background
// return where to draw title bar text.
RectangleBase<2, int> UISkin::Draw3DWindowBackground(const eastl::shared_ptr<BaseUIElement>& element, 
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

	// Create a vertex buffer for a single triangle.
	struct Vertex
	{
		Vector3<float> position;
		Vector4<float> color;
	};
	VertexFormat vformat;
	vformat.Bind(VA_POSITION, DF_R32G32B32_FLOAT, 0);
	vformat.Bind(VA_COLOR, DF_R32G32B32A32_FLOAT, 0);
	eastl::shared_ptr<VertexBuffer> vbuffer = eastl::make_shared<VertexBuffer>(vformat, 4);

	eastl::array<Vector2<int>, 4> vertices;
	rect.GetVertices(vertices);

	const eastl::array<float, 4> c2 = GetColor(DC_3D_SHADOW);
	const eastl::array<float, 4> c1 = GetColor(DC_3D_FACE);

	Vertex* vertex = vbuffer->Get<Vertex>();
	vertex[0].position = { (float)vertices[0][0], (float)vertices[0][1], 0.0f };
	vertex[0].color = { c1[0], c1[1], c1[2], 1.0f };
	vertex[1].position = { (float)vertices[1][0], (float)vertices[1][1], 0.0f };
	vertex[1].color = { c1[0], c1[1], c1[2], 1.0f };
	vertex[2].position = { (float)vertices[2][0], (float)vertices[2][1], 0.0f };
	vertex[2].color = { c1[0], c1[1], c1[2], 1.0f };
	vertex[3].position = { (float)vertices[3][0], (float)vertices[3][1], 0.0f };
	vertex[3].color = { c2[0], c2[1], c2[2], 1.0f };

	// Create an indexless buffer for a triangle mesh with one triangle.
	eastl::shared_ptr<IndexBuffer> ibuffer = eastl::make_shared<IndexBuffer>(IP_TRISTRIP, 2);

	// Create the geometric object for drawing.
	Renderer::Get()->Draw(eastl::make_shared<Visual>(vbuffer, ibuffer, mEffect));
	/*
	// top border
	rect.LowerRightCorner.Y = rect.UpperLeftCorner.Y + 1;
	if ( !checkClientArea )
	{
		//Renderer::Get()->Draw(mSurface);

		Renderer::Get()->Draw2DRectangle(GetColor(DC_3D_HIGH_LIGHT), rect, clip);
	}

	// left border
	rect.LowerRightCorner.Y = r.LowerRightCorner.Y;
	rect.LowerRightCorner.X = rect.UpperLeftCorner.X + 1;
	if ( !checkClientArea )
	{
		Renderer::Get()->Draw2DRectangle(GetColor(DC_3D_HIGH_LIGHT), rect, clip);
	}

	// right border dark outer line
	rect.UpperLeftCorner.X = r.LowerRightCorner.X - 1;
	rect.LowerRightCorner.X = r.LowerRightCorner.X;
	rect.UpperLeftCorner.Y = r.UpperLeftCorner.Y;
	rect.LowerRightCorner.Y = r.LowerRightCorner.Y;
	if ( !checkClientArea )
	{
		Renderer::Get()->Draw2DRectangle(GetColor(DC_3D_DARK_SHADOW), rect, clip);
	}

	// right border bright innner line
	rect.UpperLeftCorner.X -= 1;
	rect.LowerRightCorner.X -= 1;
	rect.UpperLeftCorner.Y += 1;
	rect.LowerRightCorner.Y -= 1;
	if ( !checkClientArea )
	{
		Renderer::Get()->Draw2DRectangle(GetColor(DC_3D_SHADOW), rect, clip);
	}

	// bottom border dark outer line
	rect.UpperLeftCorner.X = r.UpperLeftCorner.X;
	rect.UpperLeftCorner.Y = r.LowerRightCorner.Y - 1;
	rect.LowerRightCorner.Y = r.LowerRightCorner.Y;
	rect.LowerRightCorner.X = r.LowerRightCorner.X;
	if ( !checkClientArea )
	{
		Renderer::Get()->Draw2DRectangle(GetColor(DC_3D_DARK_SHADOW), rect, clip);
	}

	// bottom border bright inner line
	rect.UpperLeftCorner.X += 1;
	rect.LowerRightCorner.X -= 1;
	rect.UpperLeftCorner.Y -= 1;
	rect.LowerRightCorner.Y -= 1;
	if ( !checkClientArea )
	{
		Renderer::Get()->Draw2DRectangle(GetColor(DC_3D_SHADOW), rect, clip);
	}

	// client area for background
	rect = r;
	rect.UpperLeftCorner.X +=1;
	rect.UpperLeftCorner.Y +=1;
	rect.LowerRightCorner.X -= 2;
	rect.LowerRightCorner.Y -= 2;
	if (checkClientArea)
	{
		*checkClientArea = rect;
	}

	if ( !checkClientArea )
	{
		if (!mUseGradient)
		{
			Renderer::Get()->Draw2DRectangle(GetColor(DC_3D_FACE), rect, clip);
		}
		else if ( mType == STT_BURNING_SKIN )
		{
			const eastl::array<float, 4> c1 = 
				GetColor(DC_WINDOW).GetInterpolated ( 0xFFFFFFFF, 0.9f );
			const eastl::array<float, 4> c2 = 
				GetColor(DC_WINDOW).GetInterpolated ( 0xFFFFFFFF, 0.8f );

			Renderer::Get()->Draw2DRectangle(rect, c1, c1, c2, c2, clip);
		}
		else
		{
			const eastl::array<float, 4> c2 = GetColor(DC_3D_SHADOW);
			const eastl::array<float, 4> c1 = GetColor(DC_3D_FACE);
			Renderer::Get()->Draw2DRectangle(rect, c1, c1, c1, c2, clip);
		}
	}

	// title bar
	rect = r;
	rect.UpperLeftCorner.X += 2;
	rect.UpperLeftCorner.Y += 2;
	rect.LowerRightCorner.X -= 2;
	rect.LowerRightCorner.Y = rect.UpperLeftCorner.Y + GetSize(DS_WINDOW_BUTTON_WIDTH) + 2;

	if (drawTitleBar )
	{
		if (checkClientArea)
		{
			(*checkClientArea).UpperLeftCorner.Y = rect.LowerRightCorner.Y;
		}
		else
		{
			// draw title bar
			//if (!UseGradient)
			//	Driver->Draw2DRectangle(titleBarColor, rect, clip);
			//else
			if ( mType == STT_BURNING_SKIN )
			{
				const eastl::array<float, 4> c = titleBarColor.GetInterpolated(
					eastl::array<float, 4>(titleBarColor.GetAlpha(),255,255,255), 0.8f);
				Renderer::Get()->Draw2DRectangle(rect, titleBarColor, titleBarColor, c, c, clip);
			}
			else
			{
				const eastl::array<float, 4> c = titleBarColor.GetInterpolated(
					eastl::array<float, 4>(titleBarColor.GetAlpha(),0,0,0), 0.2f);
				Renderer::Get()->Draw2DRectangle(rect, titleBarColor, c, titleBarColor, c, clip);
			}
		}
	}
	*/
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
			const RectangleBase<2, int>& r, const RectangleBase<2, int>* clip)
{
	if (!Renderer::Get())
		return;
	/*
	RectangleBase<2, int> rect = r;

	if ( mType == STT_BURNING_SKIN )
	{
		rect.UpperLeftCorner.Y -= 3;
		Draw3DButtonPaneStandard(element, rect, clip);
		return;
	}

	// in this skin, this is exactly what non pressed buttons look like,
	// so we could simply call
	// Draw3DButtonPaneStandard(element, rect, clip);
	// here.
	// but if the skin is transparent, this doesn't look that nice. So
	// We draw it a little bit better, with some more Draw2DRectangle calls,
	// but there aren't that much menus visible anyway.

	rect.LowerRightCorner.Y = rect.UpperLeftCorner.Y + 1;
	Renderer::Get()->Draw2DRectangle(GetColor(DC_3D_HIGH_LIGHT), rect, clip);

	rect.LowerRightCorner.Y = r.LowerRightCorner.Y;
	rect.LowerRightCorner.X = rect.UpperLeftCorner.X + 1;
	Renderer::Get()->Draw2DRectangle(GetColor(DC_3D_HIGH_LIGHT), rect, clip);

	rect.UpperLeftCorner.X = r.LowerRightCorner.X - 1;
	rect.LowerRightCorner.X = r.LowerRightCorner.X;
	rect.UpperLeftCorner.Y = r.UpperLeftCorner.Y;
	rect.LowerRightCorner.Y = r.LowerRightCorner.Y;
	Renderer::Get()->Draw2DRectangle(GetColor(DC_3D_DARK_SHADOW), rect, clip);

	rect.UpperLeftCorner.X -= 1;
	rect.LowerRightCorner.X -= 1;
	rect.UpperLeftCorner.Y += 1;
	rect.LowerRightCorner.Y -= 1;
	Renderer::Get()->Draw2DRectangle(GetColor(DC_3D_SHADOW), rect, clip);

	rect.UpperLeftCorner.X = r.UpperLeftCorner.X;
	rect.UpperLeftCorner.Y = r.LowerRightCorner.Y - 1;
	rect.LowerRightCorner.Y = r.LowerRightCorner.Y;
	rect.LowerRightCorner.X = r.LowerRightCorner.X;
	Renderer::Get()->Draw2DRectangle(GetColor(DC_3D_DARK_SHADOW), rect, clip);

	rect.UpperLeftCorner.X += 1;
	rect.LowerRightCorner.X -= 1;
	rect.UpperLeftCorner.Y -= 1;
	rect.LowerRightCorner.Y -= 1;
	Renderer::Get()->Draw2DRectangle(GetColor(DC_3D_SHADOW), rect, clip);

	rect = r;
	rect.UpperLeftCorner.X +=1;
	rect.UpperLeftCorner.Y +=1;
	rect.LowerRightCorner.X -= 2;
	rect.LowerRightCorner.Y -= 2;

	if (!mUseGradient)
		Renderer::Get()->Draw2DRectangle(GetColor(DC_3D_FACE), rect, clip);
	else
	{
		const eastl::array<float, 4> c1 = GetColor(DC_3D_FACE);
		const eastl::array<float, 4> c2 = GetColor(DC_3D_SHADOW);
		Renderer::Get()->Draw2DRectangle(rect, c1, c1, c2, c2, clip);
	}*/
}


//! draws a standard 3d tool bar
/**	Used for drawing for toolbars and menus.
\param element: Pointer to the element which wishes to draw this. This parameter
is usually not used by ISkin, but can be used for example by more complex
implementations to find out how to draw the part exactly.
\param rect: Defining area where to draw.
\param clip: Clip area.	*/
void UISkin::Draw3DToolBar(const eastl::shared_ptr<BaseUIElement>& element,
				const RectangleBase<2, int>& r, const RectangleBase<2, int>* clip)
{
	if (!Renderer::Get())
		return;
	/*
	RectangleBase<2, int> rect = r;

	rect.UpperLeftCorner.X = r.UpperLeftCorner.X;
	rect.UpperLeftCorner.Y = r.LowerRightCorner.Y - 1;
	rect.LowerRightCorner.Y = r.LowerRightCorner.Y;
	rect.LowerRightCorner.X = r.LowerRightCorner.X;
	Renderer::Get()->Draw2DRectangle(GetColor(DC_3D_SHADOW), rect, clip);

	rect = r;
	rect.LowerRightCorner.Y -= 1;

	if (!mUseGradient)
	{
		Renderer::Get()->Draw2DRectangle(GetColor(DC_3D_FACE), rect, clip);
	}
	else
	if ( mType == STT_BURNING_SKIN )
	{
		const eastl::array<float, 4> c1 = 0xF0000000 | GetColor(DC_3D_FACE).color;
		const eastl::array<float, 4> c2 = 0xF0000000 | GetColor(DC_3D_SHADOW).color;

		rect.LowerRightCorner.Y += 1;
		Renderer::Get()->Draw2DRectangle(rect, c1, c2, c1, c2, clip);
	}
	else
	{
		const eastl::array<float, 4> c1 = GetColor(DC_3D_FACE);
		const eastl::array<float, 4> c2 = GetColor(DC_3D_SHADOW);
		Renderer::Get()->Draw2DRectangle(rect, c1, c1, c2, c2, clip);
	}*/
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
	const RectangleBase<2, int>& frameRect, const RectangleBase<2, int>* clip, UIAlignment alignment)
{
	if (!Renderer::Get())
		return;
	/*
	RectangleBase<2, int> tr = frameRect;

	if ( alignment == UIA_UPPERLEFT )
	{
		tr.LowerRightCorner.X -= 2;
		tr.LowerRightCorner.Y = tr.UpperLeftCorner.Y + 1;
		tr.UpperLeftCorner.X += 1;
		Renderer::Get()->Draw2DRectangle(GetColor(DC_3D_HIGH_LIGHT), tr, clip);

		// draw left highlight
		tr = frameRect;
		tr.LowerRightCorner.X = tr.UpperLeftCorner.X + 1;
		tr.UpperLeftCorner.Y += 1;
		Renderer::Get()->Draw2DRectangle(GetColor(DC_3D_HIGH_LIGHT), tr, clip);

		// draw grey background
		tr = frameRect;
		tr.UpperLeftCorner.X += 1;
		tr.UpperLeftCorner.Y += 1;
		tr.LowerRightCorner.X -= 2;
		Renderer::Get()->Draw2DRectangle(GetColor(DC_3D_FACE), tr, clip);

		// draw right middle gray shadow
		tr.LowerRightCorner.X += 1;
		tr.UpperLeftCorner.X = tr.LowerRightCorner.X - 1;
		Renderer::Get()->Draw2DRectangle(GetColor(DC_3D_SHADOW), tr, clip);

		tr.LowerRightCorner.X += 1;
		tr.UpperLeftCorner.X += 1;
		tr.UpperLeftCorner.Y += 1;
		Renderer::Get()->Draw2DRectangle(GetColor(DC_3D_DARK_SHADOW), tr, clip);
	}
	else
	{
		tr.LowerRightCorner.X -= 2;
		tr.UpperLeftCorner.Y = tr.LowerRightCorner.Y - 1;
		tr.UpperLeftCorner.X += 1;
		Renderer::Get()->Draw2DRectangle(GetColor(DC_3D_HIGH_LIGHT), tr, clip);

		// draw left highlight
		tr = frameRect;
		tr.LowerRightCorner.X = tr.UpperLeftCorner.X + 1;
		tr.LowerRightCorner.Y -= 1;
		Renderer::Get()->Draw2DRectangle(GetColor(DC_3D_HIGH_LIGHT), tr, clip);

		// draw grey background
		tr = frameRect;
		tr.UpperLeftCorner.X += 1;
		tr.UpperLeftCorner.Y -= 1;
		tr.LowerRightCorner.X -= 2;
		tr.LowerRightCorner.Y -= 1;
		Renderer::Get()->Draw2DRectangle(GetColor(DC_3D_FACE), tr, clip);

		// draw right middle gray shadow
		tr.LowerRightCorner.X += 1;
		tr.UpperLeftCorner.X = tr.LowerRightCorner.X - 1;
		//tr.LowerRightCorner.Y -= 1;
		Renderer::Get()->Draw2DRectangle(GetColor(DC_3D_SHADOW), tr, clip);

		tr.LowerRightCorner.X += 1;
		tr.UpperLeftCorner.X += 1;
		tr.LowerRightCorner.Y -= 1;
		Renderer::Get()->Draw2DRectangle(GetColor(DC_3D_DARK_SHADOW), tr, clip);
	}*/
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
	const RectangleBase<2, int>& rect, const RectangleBase<2, int>* clip, int tabHeight, UIAlignment alignment)
{
	if (!Renderer::Get())
		return;

	RectangleBase<2, int> tr = rect;
	/*
	if ( tabHeight == -1 )
		tabHeight = GetSize(DS_BUTTON_HEIGHT);

	// draw border.
	if (border)
	{
		if ( alignment == UIA_UPPERLEFT )
		{
			// draw left hightlight
			tr.UpperLeftCorner.Y += tabHeight + 2;
			tr.LowerRightCorner.X = tr.UpperLeftCorner.X + 1;
			Renderer::Get()->Draw2DRectangle(GetColor(DC_3D_HIGH_LIGHT), tr, clip);

			// draw right shadow
			tr.UpperLeftCorner.X = rect.LowerRightCorner.X - 1;
			tr.LowerRightCorner.X = tr.UpperLeftCorner.X + 1;
			Renderer::Get()->Draw2DRectangle(GetColor(DC_3D_SHADOW), tr, clip);

			// draw lower shadow
			tr = rect;
			tr.UpperLeftCorner.Y = tr.LowerRightCorner.Y - 1;
			Renderer::Get()->Draw2DRectangle(GetColor(DC_3D_SHADOW), tr, clip);
		}
		else
		{
			// draw left hightlight
			tr.LowerRightCorner.Y -= tabHeight + 2;
			tr.LowerRightCorner.X = tr.UpperLeftCorner.X + 1;
			Renderer::Get()->Draw2DRectangle(GetColor(DC_3D_HIGH_LIGHT), tr, clip);

			// draw right shadow
			tr.UpperLeftCorner.X = rect.LowerRightCorner.X - 1;
			tr.LowerRightCorner.X = tr.UpperLeftCorner.X + 1;
			Renderer::Get()->Draw2DRectangle(GetColor(DC_3D_SHADOW), tr, clip);

			// draw lower shadow
			tr = rect;
			tr.LowerRightCorner.Y = tr.UpperLeftCorner.Y + 1;
			Renderer::Get()->Draw2DRectangle(GetColor(DC_3D_HIGH_LIGHT), tr, clip);
		}
	}

	if (background)
	{
		if ( alignment == UIA_UPPERLEFT )
		{
			tr = rect;
			tr.UpperLeftCorner.Y += tabHeight + 2;
			tr.LowerRightCorner.X -= 1;
			tr.UpperLeftCorner.X += 1;
			tr.LowerRightCorner.Y -= 1;
		}
		else
		{
			tr = rect;
			tr.UpperLeftCorner.X += 1;
			tr.UpperLeftCorner.Y -= 1;
			tr.LowerRightCorner.X -= 1;
			tr.LowerRightCorner.Y -= tabHeight + 2;
			//tr.UpperLeftCorner.X += 1;
		}

		if (!mUseGradient)
			Renderer::Get()->Draw2DRectangle(GetColor(DC_3D_FACE), tr, clip);
		else
		{
			eastl::array<float, 4> c1 = GetColor(DC_3D_FACE);
			eastl::array<float, 4> c2 = GetColor(DC_3D_SHADOW);
			Renderer::Get()->Draw2DRectangle(tr, c1, c1, c2, c2, clip);
		}
	}*/
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
	const Vector2<int> position, unsigned int starttime, unsigned int currenttime, bool loop, 
	const RectangleBase<2, int>* clip)
{
	if (!mSpriteBank)
		return;

	bool gray = element && !element->IsEnabled();
	/*
	mSpriteBank->Draw2DSprite(mIcons[icon], position, clip,
		mColors[gray? DC_GRAY_WINDOW_SYMBOL : DC_WINDOW_SYMBOL], starttime, currenttime, loop, true);
	*/
}


UISkinThemeType UISkin::GetType() const
{
	return mType;
}


//! draws a 2d rectangle.
void UISkin::Draw2DRectangle(const eastl::shared_ptr<BaseUIElement>& element, 
	const eastl::array<float, 4> &color, const RectangleBase<2, int>& pos, const RectangleBase<2, int>* clip)
{
	if (!Renderer::Get())
		return;

	//Renderer::Get()->Draw2DRectangle(color, pos, clip);
}