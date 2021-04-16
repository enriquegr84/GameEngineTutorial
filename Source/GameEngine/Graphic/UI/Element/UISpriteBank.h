// Copyright (C) 2002-2012 Nikolaus Gebhardt
// This file is part of the "Irrlicht Engine".
// For conditions of distribution and use, see copyright notice in irrlicht.h

#ifndef UISPRITEBANK_H
#define UISPRITEBANK_H

#include "UIElement.h"

#include "Graphic/Effect/Texture2Effect.h"
#include "Graphic/Scene/Hierarchy/Visual.h"

//! A single sprite frame.
struct UISpriteFrame
{
	unsigned int mTextureNumber;
	unsigned int mRectNumber;
};

//! A sprite composed of several frames.
struct UISprite
{
	UISprite() : mFrames(), mFrameTime(0) {}

	eastl::vector<UISpriteFrame> mFrames;
	unsigned int mFrameTime;
};


//! Sprite bank interface.
class BaseUISpriteBank
{
public:

	//! Returns the list of rectangles held by the sprite bank
	virtual eastl::vector<RectangleShape<2, int>>& GetPositions() = 0;

	//! Returns the array of animated sprites within the sprite bank
	virtual eastl::vector<UISprite>& GetSprites() = 0;

	//! Returns the number of textures held by the sprite bank
	virtual unsigned int GetTextureCount() const = 0;

	//! Gets the texture with the specified index
	virtual eastl::shared_ptr<Texture2> GetTexture(unsigned int index) const = 0;

	//! Adds a texture to the sprite bank
	virtual void AddTexture(eastl::shared_ptr<Texture2> texture) = 0;

	//! Changes one of the textures in the sprite bank
	virtual void SetTexture(unsigned int index, eastl::shared_ptr<Texture2> texture) = 0;

	//! Add the texture and use it for a single non-animated sprite.
	//! The texture and the corresponding rectangle and sprite will all be added to the end of each array.
	//! returns the index of the sprite or -1 on failure
	virtual int AddTextureAsSprite(eastl::shared_ptr<Texture2> texture) = 0;

	//! clears sprites, rectangles and textures
	virtual void Clear() = 0;

    //! Draws a sprite in 2d with position and color
    /**
    \param index Index of UISprite to draw
    \param pos Target position - depending on center value either the left-top or the sprite center is used as pivot
    \param clip Clipping rectangle, can be 0 when clipping is not wanted.
    \param color Color with which the image is drawn.
        Note that the alpha component is used. If alpha is other than
        255, the image will be transparent.
    \param starttime Tick when the first frame was drawn (only difference currenttime-starttime matters).
    \param currenttime To calculate the frame of animated sprites
    \param loop When true animations are looped
    \param center When true pivot is set to the sprite-center. So it affects pos.
    */
    virtual void Draw2DSprite(unsigned int index, 
        const eastl::shared_ptr<Visual>& visual, const RectangleShape<2, int>& pos,
        const RectangleShape<2, int>* clip = 0, const SColorF& color = SColorF(1.f, 1.f, 1.f),
        unsigned int starttime = 0, unsigned int currenttime = 0, bool loop = true, bool center = false) = 0;


    //! Draws a sprite in 2d with destination rectangle and colors
    /**
        \param index Index of UISprite to draw
        \param destRect The sprite will be scaled to fit this target rectangle
        \param clip Clipping rectangle, can be 0 when clipping is not wanted.
        \param colors Array of 4 colors denoting the color values of
        the corners of the destRect
        \param timeTicks Current frame for animated sprites
        (same as currenttime-starttime in other draw2DSprite function)
        \param loop When true animations are looped
    */
    virtual void Draw2DSprite(unsigned int index, 
        const eastl::shared_ptr<Visual>& visual, const RectangleShape<2, int>& destRect,
        const RectangleShape<2, int>* clip = 0, const SColorF* const colors = 0, 
        unsigned int timeTicks = 0, bool loop = true) = 0;


	//! Draws a sprite batch in 2d using an array of positions and a color
	virtual void Draw2DSpriteBatch(const eastl::array<unsigned int>& indices, 
        const eastl::shared_ptr<Visual>& visual, const eastl::array<RectangleShape<2, int>>& pos, 
        const SColorF color = SColorF(1.f, 1.f, 1.f, 1.f), const RectangleShape<2, int>* clip = 0, 
        unsigned int starttime = 0, unsigned int currenttime = 0, bool loop = true, bool center = false) = 0;
};

//! Sprite bank interface.
class UISpriteBank : public BaseUISpriteBank
{
public:

	UISpriteBank(BaseUI* ui);
	virtual ~UISpriteBank();

	virtual eastl::vector<RectangleShape<2, int>>& GetPositions();
	virtual eastl::vector<UISprite>& GetSprites();

	virtual unsigned int GetTextureCount() const;
	virtual eastl::shared_ptr<Texture2> GetTexture(unsigned int index) const;
	virtual void AddTexture(eastl::shared_ptr<Texture2> texture);
	virtual void SetTexture(unsigned int index, eastl::shared_ptr<Texture2> texture);

	//! Add the texture and use it for a single non-animated sprite.
	virtual int AddTextureAsSprite(eastl::shared_ptr<Texture2> texture);

	//! clears sprites, rectangles and textures
	virtual void Clear();

    //! Draws a sprite in 2d with position and color
    virtual void Draw2DSprite(unsigned int index, const eastl::shared_ptr<Visual>& visual, const RectangleShape<2, int>& pos,
        const RectangleShape<2, int>* clip = 0, const SColorF& color = SColorF(1.f, 1.f, 1.f),
        unsigned int startTime = 0, unsigned int currentTime = 0, bool loop = true, bool center = false);

	//! Draws a sprite in 2d with destination rectangle and color
	virtual void Draw2DSprite(unsigned int index, const eastl::shared_ptr<Visual>& visual, const RectangleShape<2, int>& destRect, 
		const RectangleShape<2, int>* clip = 0, const SColorF* const colors = 0, unsigned int timeTicks = 0, bool loop = true);

	//! Draws a sprite batch in 2d using an array of positions and a color
	virtual void Draw2DSpriteBatch(const eastl::array<unsigned int>& indices, const eastl::shared_ptr<Visual>& visual, 
		const eastl::array<RectangleShape<2, int>>& pos, const SColorF& color = SColorF(1.f, 1.f, 1.f), 
		const RectangleShape<2, int>* clip = 0, unsigned int startTime = 0, unsigned int currentTime = 0, bool loop = true, bool center = false);

protected:

    unsigned int GetFrameNr(unsigned int index, unsigned int time, bool loop) const;

	struct DrawBatch
	{
		eastl::vector<Vector2<int>> mPositions;
		eastl::vector<RectangleShape<2, int>> mSourceRects;
		unsigned int mTextureNumber;
	};

	BaseUI* mUI;

	eastl::vector<UISprite> mSprites;
	eastl::vector<RectangleShape<2, int>> mRectangles;
	eastl::vector<eastl::shared_ptr<Texture2>> mTextures;
};

#endif