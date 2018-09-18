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

	//! Solid material with 2 texture layers.
	/** The second is blended onto the first using the alpha value
	of the vertex colors. This material is currently not implemented in OpenGL.
	*/
	MT_SOLID_2_LAYER,

	//! Material type with standard lightmap technique
	/** There should be 2 textures: The first texture layer is a
	diffuse map, the second is a light map. Dynamic light is
	ignored. */
	MT_LIGHTMAP,

	//! Material type with lightmap technique like EMT_LIGHTMAP.
	/** But lightmap and diffuse texture are added instead of modulated. */
	MT_LIGHTMAP_ADD,

	//! Material type with standard lightmap technique
	/** There should be 2 textures: The first texture layer is a
	diffuse map, the second is a light map. Dynamic light is
	ignored. The texture colors are effectively multiplied by 2
	for brightening. Like known in DirectX as D3DTOP_MODULATE2X. */
	MT_LIGHTMAP_M2,

	//! Material type with standard lightmap technique
	/** There should be 2 textures: The first texture layer is a
	diffuse map, the second is a light map. Dynamic light is
	ignored. The texture colors are effectively multiplyied by 4
	for brightening. Like known in DirectX as D3DTOP_MODULATE4X. */
	MT_LIGHTMAP_M4,

	//! Like EMT_LIGHTMAP, but also supports dynamic lighting.
	MT_LIGHTMAP_LIGHTING,

	//! Like EMT_LIGHTMAP_M2, but also supports dynamic lighting.
	MT_LIGHTMAP_LIGHTING_M2,

	//! Like EMT_LIGHTMAP_4, but also supports dynamic lighting.
	MT_LIGHTMAP_LIGHTING_M4,

	//! Detail mapped material.
	/** The first texture is diffuse color map, the second is added
	to this and usually displayed with a bigger scale value so that
	it adds more detail. The detail map is added to the diffuse map
	using ADD_SIGNED, so that it is possible to add and substract
	color from the diffuse map. For example a value of
	(127,127,127) will not change the appearance of the diffuse map
	at all. Often used for terrain rendering. */
	MT_DETAIL_MAP,

	//! Look like a reflection of the environment around it.
	/** To make this possible, a texture called 'sphere map' is
	used, which must be set as the first texture. */
	MT_SPHERE_MAP,

	//! A reflecting material with an optional non reflecting texture layer.
	/** The reflection map should be set as first texture. */
	MT_REFLECTION_2_LAYER,

	//! A transparent material.
	/** Only the first texture is used. The new color is calculated
	by simply adding the source color and the dest color. This
	means if for example a billboard using a texture with black
	background and a red circle on it is drawn with this material,
	the result is that only the red circle will be drawn a little
	bit transparent, and everything which was black is 100%
	transparent and not visible. This material type is useful for
	particle effects. */
	MT_TRANSPARENT_ADD_COLOR,

	//! Makes the material transparent based on the texture alpha channel.
	/** The final color is blended together from the destination
	color and the texture color, using the alpha channel value as
	blend factor. Only first texture is used. If you are using
	this material with small textures, it is a good idea to load
	the texture in 32 bit mode
	(IRenderer::VSetTextureCreationFlag()). Also, an alpha
	ref is used, which can be manipulated using
	Material::MaterialTypeParam. This value controls how sharp the
	edges become when going from a transparent to a solid spot on
	the texture. */
	MT_TRANSPARENT_ALPHA_CHANNEL,

	//! Makes the material transparent based on the texture alpha channel.
	/** If the alpha channel value is greater than 127, a
	pixel is written to the target, otherwise not. This
	material does not use alpha blending and is a lot faster
	than EMT_TRANSPARENT_ALPHA_CHANNEL. It is ideal for drawing
	stuff like leafes of plants, because the borders are not
	blurry but sharp. Only first texture is used. If you are
	using this material with small textures and 3d object, it
	is a good idea to load the texture in 32 bit mode
	(IRenderer::VSetTextureCreationFlag()). */
	MT_TRANSPARENT_ALPHA_CHANNEL_REF,

	//! Makes the material transparent based on the vertex alpha value.
	MT_TRANSPARENT_VERTEX_ALPHA,

	//! A transparent reflecting material with an optional additional non reflecting texture layer.
	/** The reflection map should be set as first texture. The
	transparency depends on the alpha value in the vertex colors. A
	texture which will not reflect can be set as second texture.
	Please note that this material type is currently not 100%
	implemented in OpenGL. */
	MT_TRANSPARENT_REFLECTION_2_LAYER,

	//! A solid normal map renderer.
	/** First texture is the color map, the second should be the
	normal map. Note that you should use this material only when
	drawing geometry consisting of vertices of type
	VertexTangents (EVT_TANGENTS). You can convert any mesh into
	this format using IMeshManipulator::CreateMeshWithTangents()
	(See SpecialFX2 Tutorial). This shader runs on vertex shader
	1.1 and pixel shader 1.1 capable hardware and falls back to a
	fixed function lighted material if this hardware is not
	available. Only two lights are supported by this shader, if
	there are more, the nearest two are chosen. */
	MT_NORMAL_MAP_SOLID,

	//! A transparent normal map renderer.
	/** First texture is the color map, the second should be the
	normal map. Note that you should use this material only when
	drawing geometry consisting of vertices of type
	VertexTangents (EVT_TANGENTS). You can convert any mesh into
	this format using IMeshManipulator::CreateMeshWithTangents()
	(See SpecialFX2 Tutorial). This shader runs on vertex shader
	1.1 and pixel shader 1.1 capable hardware and falls back to a
	fixed function lighted material if this hardware is not
	available. Only two lights are supported by this shader, if
	there are more, the nearest two are chosen. */
	MT_NORMAL_MAP_TRANSPARENT_ADD_COLOR,

	//! A transparent (based on the vertex alpha value) normal map renderer.
	/** First texture is the color map, the second should be the
	normal map. Note that you should use this material only when
	drawing geometry consisting of vertices of type
	VertexTangents (EVT_TANGENTS). You can convert any mesh into
	this format using IMeshManipulator::CreateMeshWithTangents()
	(See SpecialFX2 Tutorial). This shader runs on vertex shader
	1.1 and pixel shader 1.1 capable hardware and falls back to a
	fixed function lighted material if this hardware is not
	available.  Only two lights are supported by this shader, if
	there are more, the nearest two are chosen. */
	MT_NORMAL_MAP_TRANSPARENT_VERTEX_ALPHA,

	//! Just like EMT_NORMAL_MAP_SOLID, but uses parallax mapping.
	/** Looks a lot more realistic. This only works when the
	hardware supports at least vertex shader 1.1 and pixel shader
	1.4. First texture is the color map, the second should be the
	normal map. The normal map texture should contain the height
	value in the alpha component. The
	IVideoRenderer::makeNormalMapTexture() method writes this value
	automatically when creating normal maps from a heightmap when
	using a 32 bit texture. The height scale of the material
	(affecting the bumpiness) is being controlled by the
	Material::MaterialTypeParam member. If set to zero, the
	default value (0.02f) will be applied. Otherwise the value set
	in Material::MaterialTypeParam is taken. This value depends on
	with which scale the texture is mapped on the material. Too
	high or low values of MaterialTypeParam can result in strange
	artifacts. */
	MT_PARALLAX_MAP_SOLID,

	//! A material like EMT_PARALLAX_MAP_SOLID, but transparent.
	/** Using EMT_TRANSPARENT_ADD_COLOR as base material. */
	MT_PARALLAX_MAP_TRANSPARENT_ADD_COLOR,

	//! A material like EMT_PARALLAX_MAP_SOLID, but transparent.
	/** Using EMT_TRANSPARENT_VERTEX_ALPHA as base material. */
	MT_PARALLAX_MAP_TRANSPARENT_VERTEX_ALPHA,

	//! BlendFunc = source * sourceFactor + dest * destFactor ( E_BLEND_FUNC )
	/** Using only first texture. Generic blending method. */
	MT_ONETEXTURE_BLEND,

	//! This value is not used. It only forces this enumeration to compile to 32 bit.
	MT_FORCE_32BIT = 0x7fffffff
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
	Texture2* GetTexture(unsigned int i) const;

	//! Sets the i-th texture
	/** If i>=MATERIAL_MAX_TEXTURES this setting will be ignored.
	\param i The desired level.
	\param tex Texture for texture level i. */
	void SetTexture(unsigned int i, Texture2* tex);

	//! Store the blend state of choice
	/** Values to be chosen from BlendState. The actual way to use this value
	is not yet determined, so ignore it for now. */
	eastl::shared_ptr<BlendState> mBlendState;

	eastl::shared_ptr<DepthStencilState> mDepthStencilState;

	eastl::shared_ptr<RasterizerState> mRasterizerState;

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

	//! shading model
	ShadingModel mShadingModel : 1;

	//! Will this material be lighted? Default: true
	bool mLighting : 1;
};

#endif
