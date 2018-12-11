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


// ---------------------------------------------------------------------------
/** @brief Defines the purpose of a texture
*
*  This is a very difficult topic. Different 3D packages support different
*  kinds of textures. For very common texture types, such as bumpmaps, the
*  rendering results depend on implementation details in the rendering
*  pipelines of these applications. Assimp loads all texture references from
*  the model file and tries to determine which of the predefined texture
*  types below is the best choice to match the original use of the texture
*  as closely as possible.<br>
*
*  In content pipelines you'll usually define how textures have to be handled,
*  and the artists working on models have to conform to this specification,
*  regardless which 3D tool they're using.
*/
enum TextureType
{
	/** The texture is combined with the result of the diffuse
	*  lighting equation.
	*/
	TT_DIFFUSE = 0x0,

	/** The texture is combined with the result of the specular
	*  lighting equation.
	*/
	TT_SPECULAR = 0x1,

	/** The texture is combined with the result of the ambient
	*  lighting equation.
	*/
	TT_AMBIENT = 0x2,

	/** The texture is added to the result of the lighting
	*  calculation. It isn't influenced by incoming light.
	*/
	TT_EMISSIVE = 0x3,

	/** The texture is a height map.
	*
	*  By convention, higher gray-scale values stand for
	*  higher elevations from the base height.
	*/
	TT_HEIGHT = 0x4,

	/** The texture is a (tangent space) normal-map.
	*
	*  Again, there are several conventions for tangent-space
	*  normal maps. Assimp does (intentionally) not
	*  distinguish here.
	*/
	TT_NORMALS = 0x5,

	/** The texture defines the glossiness of the material.
	*
	*  The glossiness is in fact the exponent of the specular
	*  (phong) lighting equation. Usually there is a conversion
	*  function defined to map the linear color values in the
	*  texture to a suitable exponent. Have fun.
	*/
	TT_SHININESS = 0x6,

	/** The texture defines per-pixel opacity.
	*
	*  Usually 'white' means opaque and 'black' means
	*  'transparency'. Or quite the opposite. Have fun.
	*/
	TT_OPACITY = 0x7,

	/** Displacement texture
	*
	*  The exact purpose and format is application-dependent.
	*  Higher color values stand for higher vertex displacements.
	*/
	TT_DISPLACEMENT = 0x8,

	/** Lightmap texture (aka Ambient Occlusion)
	*
	*  Both 'Lightmaps' and dedicated 'ambient occlusion maps' are
	*  covered by this material property. The texture contains a
	*  scaling value for the final color value of a pixel. Its
	*  intensity is not affected by incoming light.
	*/
	TT_LIGHTMAP = 0x9,

	/** Reflection texture
	*
	* Contains the color of a perfect mirror reflection.
	* Rarely used, almost never for real-time applications.
	*/
	TT_REFLECTION = 0xA,

	/** Unknown texture
	*
	*  A texture reference that does not match any of the definitions
	*  above is considered to be 'enum count'.
	*/
	TT_COUNT = 0xB
};

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
