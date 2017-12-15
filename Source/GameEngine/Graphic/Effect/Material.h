// David Eberly, Geometric Tools, Redmond WA 98052
// Copyright (c) 1998-2017
// Distributed under the Boost Software License, Version 1.0.
// http://www.boost.org/LICENSE_1_0.txt
// http://www.geometrictools.com/License/Boost/LICENSE_1_0.txt
// File Version: 3.0.0 (2016/06/19)

#ifndef MATERIAL_H
#define MATERIAL_H

#include "Graphic/GraphicStd.h"

#include "Mathematic/Algebra/Vector4.h"

//! Material flags
enum GRAPHIC_ITEM MaterialFlag
{
	//! Draw as wireframe or filled triangles? Default: false
	MF_WIREFRAME = 0x1,

	//! Draw as point cloud or filled triangles? Default: false
	MF_POINTCLOUD = 0x2,

	//! Flat or Gouraud shading? Default: true
	MF_GOURAUD_SHADING = 0x4,

	//! Will this material be lighted? Default: true
	MF_LIGHTING = 0x8,

	//! Is the ZBuffer enabled? Default: true
	MF_ZBUFFER = 0x10,

	//! May be written to the zbuffer or is it readonly. Default: true
	/** This flag is ignored, if the material type is a transparent type. */
	MF_ZWRITE_ENABLE = 0x20,

	//! Is backface culling enabled? Default: true
	MF_BACK_FACE_CULLING = 0x40,

	//! Is frontface culling enabled? Default: false
	/** Overrides EMF_BACK_FACE_CULLING if both are enabled. */
	MF_FRONT_FACE_CULLING = 0x80,

	//! Is bilinear filtering enabled? Default: true
	MF_BILINEAR_FILTER = 0x100,

	//! Is trilinear filtering enabled? Default: false
	/** If the trilinear filter flag is enabled,
	the bilinear filtering flag is ignored. */
	MF_TRILINEAR_FILTER = 0x200,

	//! Is anisotropic filtering? Default: false
	/** In Irrlicht you can use anisotropic texture filtering in
	conjunction with bilinear or trilinear texture filtering
	to improve rendering results. Primitives will look less
	blurry with this flag switched on. */
	MF_ANISOTROPIC_FILTER = 0x400,

	//! Is fog enabled? Default: false
	MF_FOG_ENABLE = 0x800,

	//! Normalizes normals. Default: false
	/** You can enable this if you need to scale a dynamic lighted
	model. Usually, its normals will get scaled too then and it
	will get darker. If you enable the EMF_NORMALIZE_NORMALS flag,
	the normals will be normalized again, and the model will look
	as bright as it should. */
	MF_NORMALIZE_NORMALS = 0x1000,

	//! Access to all layers texture wrap settings. Overwrites separate layer settings.
	MF_TEXTURE_WRAP = 0x2000,

	//! AntiAliasing mode
	MF_ANTI_ALIASING = 0x4000,

	//! ColorMask bits, for enabling the color planes
	MF_COLOR_MASK = 0x8000,

	//! ColorMaterial enum for vertex color interpretation
	MF_COLOR_MATERIAL = 0x10000,

	//! Flag for enabling/disabling mipmap usage
	MF_USE_MIP_MAPS = 0x20000,

	//! Flag for blend operation
	MF_BLEND_OPERATION = 0x40000,

	//! Flag for polygon offset
	MF_POLYGON_OFFSET = 0x80000
};

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

class GRAPHIC_ITEM Material
{
public:
    // Construction.
    Material();

	bool operator!=(const Material& m) const;
	bool operator==(const Material& m) const;

	bool IsTransparent() const;

	// material type
	MaterialType mType;

	// material flag
	MaterialFlag mFlag;

    // (r,g,b,*): default (0,0,0,1)
    Vector4<float> mEmissive;

    // (r,g,b,*): default (0,0,0,1)
    Vector4<float> mAmbient;

    // (r,g,b,a): default (0,0,0,1)
    Vector4<float> mDiffuse;

    // (r,g,b,specularPower): default (0,0,0,1)
    Vector4<float> mSpecular;
};

#endif
