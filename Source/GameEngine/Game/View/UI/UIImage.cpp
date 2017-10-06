// Copyright (C) 2002-2012 Nikolaus Gebhardt
// This file is part of the "Irrlicht Engine".
// For conditions of distribution and use, see copyright notice in irrlicht.h

#include "UIImage.h"

#include "UISkin.h"
#include "UIFont.h"

#include "UserInterface.h"

#include "Graphic/Renderer/Renderer.h"

//! constructor
UIImage::UIImage(BaseUI* ui, int id, RectangleBase<2, int> rectangle)
	: BaseUIImage(id, rectangle), mUI(ui), mTexture(0), mColor{ 1.f, 1.f, 1.f, 1.f },
	mUseAlphaChannel(false), mScaleImage(false)
{
	#ifdef _DEBUG
	//setDebugName("CGUIImage");
	#endif

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

	// Create an effect for the vertex and pixel shaders.  The texture is
	// bilinearly filtered and the texture coordinates are clamped to [0,1]^2.
	eastl::string path = FileSystem::Get()->GetPath("Effects/Texture2Effect.hlsl");
	mEffect = eastl::make_shared<Texture2Effect>(ProgramFactory::Get(), path, eastl::shared_ptr<Texture2>(),
		SamplerState::MIN_L_MAG_L_MIP_P, SamplerState::CLAMP, SamplerState::CLAMP);

	// Create the geometric object for drawing.
	mVisual = eastl::make_shared<Visual>(vbuffer, ibuffer, mEffect);
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
		if (mScaleImage)
		{
			const eastl::array<float, 4> colors[]{ mColor,mColor,mColor,mColor };

			const RectangleBase<2, int>& sourceRect = mAbsoluteRect;
			Vector2<int> sourceSize(sourceRect.extent);
			Vector2<int> sourcePos;
			sourcePos[0] = sourceRect.center[0];
			sourcePos[0] = sourceRect.center[1];

			mEffect->SetTexture(mTexture);

			struct Vertex
			{
				Vector3<float> position;
				Vector2<float> tcoord;
			};
			Vertex* vertex = mVisual->GetVertexBuffer()->Get<Vertex>();
			vertex[0].position = {
				(float)sourcePos[0] / mTexture->GetDimension(0),
				(float)sourcePos[1] / mTexture->GetDimension(1), 0.0f };
			vertex[0].tcoord = { 0.0f, 1.0f };
			vertex[1].position = {
				(float)(sourcePos[0] + (sourceSize[0] / 2)) / mTexture->GetDimension(0),
				(float)sourcePos[1] / mTexture->GetDimension(1), 0.0f };
			vertex[1].tcoord = { 1.0f, 1.0f };
			vertex[2].position = {
				(float)sourcePos[0] / mTexture->GetDimension(0),
				(float)(sourcePos[1] + (sourceSize[1] / 2)) / mTexture->GetDimension(1), 0.0f };
			vertex[2].tcoord = { 0.0f, 0.0f };
			vertex[3].position = {
				(float)(sourcePos[0] + (sourceSize[0] / 2)) / mTexture->GetDimension(0),
				(float)(sourcePos[1] + (sourceSize[1] / 2)) / mTexture->GetDimension(1), 0.0f };
			vertex[3].tcoord = { 1.0f, 0.0f };
		}
		else
		{
			const eastl::array<float, 4> colors[]{ mColor,mColor,mColor,mColor };

			const RectangleBase<2, int>& sourceRect = mAbsoluteRect;
			Vector2<int> sourceSize(sourceRect.extent);
			Vector2<int> sourcePos;
			sourcePos[0] = sourceRect.center[0] - (sourceSize[0] / 2);
			sourcePos[0] = sourceRect.center[1] - (sourceSize[1] / 2);

			mEffect->SetTexture(mTexture);

			struct Vertex
			{
				Vector3<float> position;
				Vector2<float> tcoord;
			};
			Vertex* vertex = mVisual->GetVertexBuffer()->Get<Vertex>();
			vertex[0].position = {
				(float)sourcePos[0] / mTexture->GetDimension(0),
				(float)sourcePos[1] / mTexture->GetDimension(1), 0.0f };
			vertex[0].tcoord = { 0.0f, 1.0f };
			vertex[1].position = {
				(float)(sourcePos[0] + (sourceSize[0] / 2)) / mTexture->GetDimension(0),
				(float)sourcePos[1] / mTexture->GetDimension(1), 0.0f };
			vertex[1].tcoord = { 1.0f, 1.0f };
			vertex[2].position = {
				(float)sourcePos[0] / mTexture->GetDimension(0),
				(float)(sourcePos[1] + (sourceSize[1] / 2)) / mTexture->GetDimension(1), 0.0f };
			vertex[2].tcoord = { 0.0f, 0.0f };
			vertex[3].position = {
				(float)(sourcePos[0] + (sourceSize[0] / 2)) / mTexture->GetDimension(0),
				(float)(sourcePos[1] + (sourceSize[1] / 2)) / mTexture->GetDimension(1), 0.0f };
			vertex[3].tcoord = { 1.0f, 0.0f };
		}

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