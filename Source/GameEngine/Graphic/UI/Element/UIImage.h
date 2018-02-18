// Copyright (C) 2002-2012 Nikolaus Gebhardt
// This file is part of the "Irrlicht Engine".
// For conditions of distribution and use, see copyright notice in irrlicht.h

#ifndef UIIMAGE_H
#define UIIMAGE_H

#include "UIElement.h"

#include "Graphic/Effect/Texture2Effect.h"
#include "Graphic/Scene/Hierarchy/Visual.h"

//! BaseUI element displaying an image.
class BaseUIImage : public BaseUIElement
{
public:

	//! constructor
	BaseUIImage(int id, RectangleShape<2, int> rectangle)
		: BaseUIElement(UIET_IMAGE, id, rectangle) {}

	//! Sets an image texture
	virtual void SetImage(const eastl::shared_ptr<Texture2>& image = 0) = 0;

	//! Gets the image texture
	virtual const eastl::shared_ptr<Texture2>& GetImage() const = 0;

	//! Sets the color of the image
	virtual void SetColor(eastl::array<float, 4> color) = 0;

	//! Sets if the image should scale to fit the element
	virtual void SetScaleImage(bool scale) = 0;

	//! Sets if the image should use its alpha channel to draw itself
	virtual void SetUseAlphaChannel(bool use) = 0;

	//! Gets the color of the image
	virtual eastl::array<float, 4> GetColor() const = 0;

	//! Returns true if the image is scaled to fit, false if not
	virtual bool IsImageScaled() const = 0;

	//! Returns true if the image is using the alpha channel, false if not
	virtual bool IsAlphaChannelUsed() const = 0;
};

class UIImage : public BaseUIImage
{
public:

	//! constructor
	UIImage(BaseUI* ui, int id, RectangleShape<2, int> rectangle);

	//! destructor
	virtual ~UIImage();

	//! Sets an image texture
	virtual void SetImage(const eastl::shared_ptr<Texture2>& image = 0);

	//! Gets the image texture
	virtual const eastl::shared_ptr<Texture2>& GetImage() const;

	//! Sets the color of the image
	virtual void SetColor(eastl::array<float, 4> color);

	//! Sets if the image should scale to fit the element
	virtual void SetScaleImage(bool scale);

	//! Sets if the image should use its alpha channel to draw itself
	virtual void SetUseAlphaChannel(bool use);

	//! Gets the color of the image
	virtual eastl::array<float, 4> GetColor() const;

	//! Returns true if the image is scaled to fit, false if not
	virtual bool IsImageScaled() const;

	//! Returns true if the image is using the alpha channel, false if not
	virtual bool IsAlphaChannelUsed() const;

	//! draws the element and its children
	virtual void Draw();

private:

	BaseUI* mUI;

	eastl::shared_ptr<Visual> mVisual;
	eastl::shared_ptr<Texture2Effect> mEffect;

	eastl::shared_ptr<Texture2> mTexture;
	eastl::array<float, 4> mColor;
	bool mUseAlphaChannel;
	bool mScaleImage;

};

#endif
