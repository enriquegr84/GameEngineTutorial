// Copyright (C) 2002-2012 Nikolaus Gebhardt
// This file is part of the "Irrlicht Engine".
// For conditions of distribution and use, see copyright notice in irrlicht.h

#include "UISpriteBank.h"

#include "UserInterface.h"

#include "Graphic/Renderer/Renderer.h"
#include "Core/OS/OS.h"

UISpriteBank::UISpriteBank(BaseUI* ui) : mUI(ui)
{
	#ifdef _DEBUG
	//setDebugName("UISpriteBank");
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


UISpriteBank::~UISpriteBank()
{
	// drop textures
	/*
	for (unsigned int i = 0; i < mTextures.size(); ++i)
		mTextures[i].SubscribeForDestruction();
	*/
}


eastl::vector<RectangleBase<2, int>>& UISpriteBank::GetPositions()
{
	return mRectangles;
}


eastl::vector< UISprite >& UISpriteBank::GetSprites()
{
	return mSprites;
}


unsigned int UISpriteBank::GetTextureCount() const
{
	return mTextures.size();
}


eastl::shared_ptr<Texture2> UISpriteBank::GetTexture(unsigned int index) const
{
	if (index < mTextures.size())
		return mTextures[index];
	else
		return 0;
}


void UISpriteBank::AddTexture(eastl::shared_ptr<Texture2> texture)
{
	mTextures.push_back(texture);
}


void UISpriteBank::SetTexture(unsigned int index, eastl::shared_ptr<Texture2> texture)
{
	while (index >= mTextures.size())
		mTextures.push_back(nullptr);

	mTextures[index] = texture;
}


//! clear everything
void UISpriteBank::Clear()
{
	// drop textures
	/*
	for (unsigned int i = 0; i < mTextures.size(); ++i)
		mTextures[i].SubscribeForDestruction();
	*/
	mTextures.clear();
	mSprites.clear();
	mRectangles.clear();
}

//! Add the texture and use it for a single non-animated sprite.
int UISpriteBank::AddTextureAsSprite(eastl::shared_ptr<Texture2> texture)
{
	if ( !texture )
		return -1;

	AddTexture(texture);
	unsigned int textureIndex = GetTextureCount() - 1;

	unsigned int rectangleIndex = mRectangles.size();

	RectangleBase<2, int> rectangle;
	rectangle.center[0] = texture->GetDimension(0) / 2;
	rectangle.center[1] = texture->GetDimension(1) / 2;
	rectangle.extent[0] = texture->GetDimension(0);
	rectangle.extent[1] = texture->GetDimension(1);
	mRectangles.push_back(rectangle);

	UISprite sprite;
	sprite.mFrameTime = 0;

	UISpriteFrame frame;
	frame.mTextureNumber = textureIndex;
	frame.mRectNumber = rectangleIndex;
	sprite.mFrames.push_back( frame );

	mSprites.push_back( sprite );

	return mSprites.size() - 1;
}

//! draws a sprite in 2d with scale and color
void UISpriteBank::Draw2DSprite(unsigned int index, const Vector2<int>& pos, const RectangleBase<2, int>* clip,
	const eastl::array<float, 4> color, unsigned int starttime, unsigned int currenttime, bool loop, bool center)
{
	if (index >= mSprites.size() || mSprites[index].mFrames.empty() )
		return;

	// work out frame number
	unsigned int frame = 0;
	if (mSprites[index].mFrameTime)
	{
		unsigned int f = ((currenttime - starttime) / mSprites[index].mFrameTime);
		if (loop)
			frame = f % mSprites[index].mFrames.size();
		else
			frame = (f >= mSprites[index].mFrames.size()) ? mSprites[index].mFrames.size()-1 : f;
	}

	eastl::shared_ptr<Texture2> tex = mTextures[mSprites[index].mFrames[frame].mTextureNumber];
	if (!tex) return;

	const unsigned int rn = mSprites[index].mFrames[frame].mRectNumber;
	if (rn >= mRectangles.size()) return;

	const RectangleBase<2, int>& sourceRect = mRectangles[rn];
	Vector2<int> sourceSize(sourceRect.extent);
	Vector2<int> sourcePos;
	sourcePos[0] = sourceRect.center[0] - (int)round(sourceSize[0] / 2.f);
	sourcePos[0] = sourceRect.center[1] - (int)round(sourceSize[1] / 2.f);

	Vector2<int> targetPos = pos;
	if (center) targetPos -= sourceSize / 2;

	mEffect->SetTexture(tex);

	struct Vertex
	{
		Vector3<float> position;
		Vector2<float> tcoord;
	};
	Vertex* vertex = mVisual->GetVertexBuffer()->Get<Vertex>();
	vertex[0].position = { 
		(float)sourcePos[0] / tex->GetDimension(0), 
		(float)sourcePos[1] / tex->GetDimension(1), 0.0f };
	vertex[0].tcoord = { 0.0f, 1.0f };
	vertex[1].position = { 
		(float)(sourcePos[0] + (sourceSize[0] / 2)) / tex->GetDimension(0), 
		(float)sourcePos[1] / tex->GetDimension(1), 0.0f };
	vertex[1].tcoord = { 1.0f, 1.0f };
	vertex[2].position = { 
		(float)sourcePos[0] / tex->GetDimension(0), 
		(float)(sourcePos[1] + (sourceSize[1] / 2)) / tex->GetDimension(1), 0.0f };
	vertex[2].tcoord = { 0.0f, 0.0f };
	vertex[3].position = { 
		(float)(sourcePos[0] + (sourceSize[0] / 2)) / tex->GetDimension(0), 
		(float)(sourcePos[1] + (sourceSize[1] / 2)) / tex->GetDimension(1), 0.0f };
	vertex[3].tcoord = { 1.0f, 0.0f };

	// Create the geometric object for drawing.
	Renderer::Get()->Draw(mVisual);
}


void UISpriteBank::Draw2DSpriteBatch(const eastl::array<unsigned int>& indices, 
	const eastl::array<Vector2<int>>& pos, const eastl::array<float, 4> color, 
	const RectangleBase<2, int>* clip, unsigned int starttime, 
	unsigned int currenttime, bool loop, bool center)
{
	const unsigned int drawCount = eastl::min<unsigned int>(indices.size(), pos.size());

	if( mTextures.empty() )
		return;

	eastl::vector<DrawBatch> drawBatches(mTextures.size());
	for(unsigned int i = 0; i < mTextures.size(); i++)
	{
		drawBatches.push_back(DrawBatch());
		drawBatches[i].mPositions.set_capacity(drawCount);
		drawBatches[i].mSourceRects.set_capacity(drawCount);
	}

	for(unsigned int i = 0;i < drawCount;i++)
	{
		const unsigned int index = indices[i];

		if (index >= mSprites.size() || mSprites[index].mFrames.empty() )
			continue;

		// work out frame number
		unsigned int frame = 0;
		if (mSprites[index].mFrameTime)
		{
			unsigned int f = ((currenttime - starttime) / mSprites[index].mFrameTime);
			if (loop)
				frame = f % mSprites[index].mFrames.size();
			else
				frame = (f >= mSprites[index].mFrames.size()) ? mSprites[index].mFrames.size()-1 : f;
		}

		const unsigned int texNum = mSprites[index].mFrames[frame].mTextureNumber;

		DrawBatch& currentBatch = drawBatches[texNum];

		const unsigned int rn = mSprites[index].mFrames[frame].mRectNumber;
		if (rn >= mRectangles.size())
			return;

		const RectangleBase<2, int>& r = mRectangles[rn];

		if (center)
		{
			Vector2<int> p = pos[i];
			p -= r.extent / 2;

			currentBatch.mPositions.push_back(p);
			currentBatch.mSourceRects.push_back(r);
		}
		else
		{
			currentBatch.mPositions.push_back(pos[i]);
			currentBatch.mSourceRects.push_back(r);
		}
	}

	for(unsigned int i = 0;i < drawBatches.size();i++)
	{
		if (!drawBatches[i].mPositions.empty() && !drawBatches[i].mSourceRects.empty())
		{
			/*
			Driver->Draw2DImageBatch(mTextures[i], drawBatches[i].mPositions,
				drawBatches[i].mSourceRects, clip, color, true);
				*/
		}
	}
}