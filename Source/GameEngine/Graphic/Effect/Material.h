// David Eberly, Geometric Tools, Redmond WA 98052
// Copyright (c) 1998-2017
// Distributed under the Boost Software License, Version 1.0.
// http://www.boost.org/LICENSE_1_0.txt
// http://www.geometrictools.com/License/Boost/LICENSE_1_0.txt
// File Version: 3.0.0 (2016/06/19)

#ifndef MATERIAL_H
#define MATERIAL_H

#include "Graphic/GraphicStd.h"

#include "MaterialLayer.h"

#include "Graphic/State/DepthStencilState.h"
#include "Graphic/State/RasterizerState.h"
#include "Graphic/State/BlendState.h"

#include "Graphic/Resource/Texture/Texture2.h"

#include "Mathematic/Algebra/Vector4.h"

//! Abstracted and easy to use fixed function/programmable pipeline material modes.
enum GRAPHIC_ITEM MaterialType
{
	//! Standard solid material.
	/** Only first texture is used, which is supposed to be the
	diffuse material. */
	MT_SOLID = 0,

	//! A transparent material.
	/** Only the first texture is used. The new color is calculated
	by simply adding the source color and the dest color. This
	means if for example a billboard using a texture with black
	background and a red circle on it is drawn with this material,
	the result is that only the red circle will be drawn a little
	bit transparent, and everything which was black is 100%
	transparent and not visible. This material type is useful for
	particle effects. */
	MT_TRANSPARENT
};

//! Defines all shading models supported by the library.
enum ShadingModel
{
	//! Flat shading. Shading is done on per-face base, diffuse only. Also known as 'faceted shading'.
	SM_FLAT = 0x1,
	//! Gouraud shading.
	SM_GOURAUD = 0x2,
	//! Phong-Shading
	SM_PHONG = 0x3,
	//! Phong - Blinn - Shading.
	SM_BLINN = 0x4,
	//! No shading at all.
	SM_NONE = 0x9,
	//! Fresnel shading.
	SM_FRESNEL = 0xa
};

class GRAPHIC_ITEM Material
{
public:
    // Construction.
    Material();

	//! Copy constructor
	/** \param other Material to copy from. */
	Material(const Material& other) { *this = other; }

	bool operator!=(const Material& m) const;
	bool operator==(const Material& m) const;

	bool IsTransparent() const;

	//! Gets the i-th texture
	/** \param i The desired level.
	\return Texture for texture level i, if defined, else 0. */
	eastl::shared_ptr<Texture2> GetTexture(unsigned int i) const;

	//! Sets the i-th texture
	/** If i>=MATERIAL_MAX_TEXTURES this setting will be ignored.
	\param i The desired level.
	\param tex Texture for texture level i. */
	void SetTexture(unsigned int i, eastl::shared_ptr<Texture2> tex);

	// Update
	bool Update(eastl::shared_ptr<BlendState>& blendState) const;
	bool Update(eastl::shared_ptr<RasterizerState>& rasterizerState) const;
	bool Update(eastl::shared_ptr<DepthStencilState>& depthStencilState) const;

	//! Texture layer array.
	MaterialLayer mTextureLayer[MATERIAL_MAX_TEXTURES];

	//! Type of the material. Specifies how everything is blended together
	MaterialType mType;

	//! Light emitted by this material. Default is to emit no light.
    Vector4<float> mEmissive;

	//! How much ambient light (a global light) is reflected by this material.
	/** The default is full white, meaning objects are completely
	globally illuminated. Reduce this if you want to see diffuse
	or specular light effects. */
    Vector4<float> mAmbient;

	//! How much diffuse light coming from a light source is reflected by this material.
	/** The default is full white. */
    Vector4<float> mDiffuse;

	//! How much specular light (highlights from a light) is reflected.
	/** The default is to reflect white specular light. See
	SMaterial::Shininess on how to enable specular lights. */
    Vector4<float> mSpecular;

	//! Value affecting the size of specular highlights.
	/** A value of 20 is common. If set to 0, no specular
	highlights are being used. To activate, simply set the
	shininess of a material to a value in the range [0.5;128]:
	\code
	sceneNode->getMaterial(0).Shininess = 20.0f;
	\endcode
	*/
	float mShininess;

	//! Thickness of non-3dimensional elements such as lines and points.
	float mThickness;

	//! Will this material be lighted? Default: true
	bool mLighting : 1;

	//! Is the ZBuffer enabled? Default: ECFN_LESSEQUAL
	/** Values are from E_COMPARISON_FUNC. */
	bool mDepthBuffer : 1;

	//depth stencil mask
	DepthStencilState::WriteMask mDepthMask;

	//! Sets the antialiasing mode
	bool mAntiAliasing : 1;

	bool mMultisampling : 1;

	//blend target state
	BlendState::Target mBlendTarget;

	//!culling state
	RasterizerState::CullMode mCullMode;

	//! Draw as wireframe or filled triangles
	RasterizerState::FillMode mFillMode;

	//! shading model
	ShadingModel mShadingModel;
};

#endif
