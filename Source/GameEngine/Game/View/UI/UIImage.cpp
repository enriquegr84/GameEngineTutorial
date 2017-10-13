// Copyright (C) 2002-2012 Nikolaus Gebhardt
// This file is part of the "Irrlicht Engine".
// For conditions of distribution and use, see copyright notice in irrlicht.h

#include "UIImage.h"

#include "UISkin.h"
#include "UIFont.h"

#include "UserInterface.h"

#include "Core/OS/OS.h"

#include "Graphic/Image/ImageResource.h"
#include "Graphic/Renderer/Renderer.h"

//! constructor
UIImage::UIImage(BaseUI* ui, int id, RectangleBase<2, int> rectangle)
	: BaseUIImage(id, rectangle), mUI(ui), mTexture(0), mColor{ 1.f, 1.f, 1.f, 1.f },
	mUseAlphaChannel(false), mScaleImage(false)
{
	#ifdef _DEBUG
	//setDebugName("CGUIImage");
	#endif

	eastl::shared_ptr<ResHandle>& resHandle =
		ResCache::Get()->GetHandle(&BaseResource(L"Art/UserControl/appbar.empty.png"));
	if (resHandle)
	{
		const eastl::shared_ptr<ImageResourceExtraData>& extra =
			eastl::static_pointer_cast<ImageResourceExtraData>(resHandle->GetExtra());
		extra->GetImage()->AutogenerateMipmaps();

		// Create a vertex buffer for a two-triangles square. The PNG is stored
		// in left-handed coordinates. The texture coordinates are chosen to
		// reflect the texture in the y-direction.
		struct Vertex
		{
			Vector3<float> position;
			Vector2<float> tcoord;
		};
		VertexFormat vformat;
		vformat.Bind(VA_POSITION, DF_R32G32B32_FLOAT, 0);
		vformat.Bind(VA_TEXCOORD, DF_R32G32_FLOAT, 0);

		eastl::shared_ptr<VertexBuffer> vbuffer = eastl::make_shared<VertexBuffer>(vformat, 4);
		eastl::shared_ptr<IndexBuffer> ibuffer = eastl::make_shared<IndexBuffer>(IP_TRISTRIP, 2);

		// Create an effect for the vertex and pixel shaders. The texture is
		// bilinearly filtered and the texture coordinates are clamped to [0,1]^2.
		eastl::string path = FileSystem::Get()->GetPath("Effects/Texture2Effect.hlsl");
		mEffect = eastl::make_shared<Texture2Effect>(ProgramFactory::Get(), path, extra->GetImage(),
			SamplerState::MIN_L_MAG_L_MIP_P, SamplerState::CLAMP, SamplerState::CLAMP);

		// Create the geometric object for drawing.
		mVisual = eastl::make_shared<Visual>(vbuffer, ibuffer, mEffect);
	}
}


//! destructor
UIImage::~UIImage()
{
}


//! sets an image
void UIImage::SetImage(const eastl::shared_ptr<Texture2>& image)
{
	if (image == mTexture)
		return;
	mTexture = image;
}

//! Gets the image texture
const eastl::shared_ptr<Texture2>& UIImage::GetImage() const
{
	return mTexture;
}

//! sets the color of the image
void UIImage::SetColor(eastl::array<float, 4> color)
{
	mColor = color;
}

//! Gets the color of the image
eastl::array<float, 4> UIImage::GetColor() const
{
	return mColor;
}

//! draws the element and its children
void UIImage::Draw()
{
	if (!IsVisible())
		return;

	const eastl::shared_ptr<BaseUISkin>& skin = mUI->GetSkin();
	if (mTexture)
	{
		const eastl::array<float, 4> colors[]{ mColor,mColor,mColor,mColor };

		Vector2<int> targetPos{ mAbsoluteRect.center[0] , mAbsoluteRect.center[1] };
		Vector2<int> dimension(mParent != nullptr ? mParent->GetAbsolutePosition().extent / 2 : mAbsoluteRect.extent / 2);

		if (!mScaleImage)
		{
			targetPos[0] = mAbsoluteRect.center[0] - (dimension[0] / 2);
			targetPos[1] = mAbsoluteRect.center[1] - (dimension[1] / 2);
		}

		Vector2<unsigned int> sourceCenter{ mTexture->GetDimension(0) / 2, mTexture->GetDimension(1) / 2 };
		Vector2<unsigned int> sourceSize{ mTexture->GetDimension(0), mTexture->GetDimension(1) };

		mEffect->SetTexture(mTexture);

		struct Vertex
		{
			Vector3<float> position;
			Vector2<float> tcoord;
		};
		Vertex* vertex = mVisual->GetVertexBuffer()->Get<Vertex>();
		vertex[0].position = {
			(float)(targetPos[0] - dimension[0] - (mAbsoluteClippingRect.extent[0] / 2)) / dimension[0],
			(float)(dimension[1] - targetPos[1] - (mAbsoluteClippingRect.extent[1] / 2)) / dimension[1], 0.0f };
		vertex[0].tcoord = {
			(float)(sourceCenter[0] - (sourceSize[0] / 2)) / sourceSize[0],
			(float)(sourceCenter[1] + (int)round(sourceSize[1] / 2.f)) / sourceSize[1] };
		vertex[1].position = {
			(float)(targetPos[0] - dimension[0] + (int)round(mAbsoluteClippingRect.extent[0] / 2.f)) / dimension[0],
			(float)(dimension[1] - targetPos[1] - (mAbsoluteClippingRect.extent[1] / 2)) / dimension[1], 0.0f };
		vertex[1].tcoord = {
			(float)(sourceCenter[0] + (int)round(sourceSize[0] / 2.f)) / sourceSize[0],
			(float)(sourceCenter[1] + (int)round(sourceSize[1] / 2.f)) / sourceSize[1] };
		vertex[2].position = {
			(float)(targetPos[0] - dimension[0] - (mAbsoluteClippingRect.extent[0] / 2)) / dimension[0],
			(float)(dimension[1] - targetPos[1] + (int)round(mAbsoluteClippingRect.extent[1] / 2.f)) / dimension[1], 0.0f };
		vertex[2].tcoord = {
			(float)(sourceCenter[0] - (sourceSize[0] / 2)) / sourceSize[0],
			(float)(sourceCenter[1] - (sourceSize[1] / 2)) / sourceSize[1] };
		vertex[3].position = {
			(float)(targetPos[0] - dimension[0] + (int)round(mAbsoluteClippingRect.extent[0] / 2.f)) / dimension[0],
			(float)(dimension[1] - targetPos[1] + (int)round(mAbsoluteClippingRect.extent[1] / 2.f)) / dimension[1], 0.0f };
		vertex[3].tcoord = {
			(float)(sourceCenter[0] + (int)round(sourceSize[0] / 2.f)) / sourceSize[0],
			(float)(sourceCenter[1] - (sourceSize[1] / 2)) / sourceSize[1] };

		// Create the geometric object for drawing.
		Renderer::Get()->Draw(mVisual);
	}
	else
	{
		skin->Draw2DRectangle(shared_from_this(), skin->GetColor(DC_3D_DARK_SHADOW), 
			mVisual, mAbsoluteRect, &mAbsoluteClippingRect);
	}

	BaseUIElement::Draw();
}


//! sets if the image should use its alpha channel to draw itself
void UIImage::SetUseAlphaChannel(bool use)
{
	mUseAlphaChannel = use;
}


//! sets if the image should use its alpha channel to draw itself
void UIImage::SetScaleImage(bool scale)
{
	mScaleImage = scale;
}


//! Returns true if the image is scaled to fit, false if not
bool UIImage::IsImageScaled() const
{
	return mScaleImage;
}

//! Returns true if the image is using the alpha channel, false if not
bool UIImage::IsAlphaChannelUsed() const
{
	return mUseAlphaChannel;
}