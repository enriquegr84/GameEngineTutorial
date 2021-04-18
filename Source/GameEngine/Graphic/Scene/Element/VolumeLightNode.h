// Copyright (C) 2002-2012 Nikolaus Gebhardt
// This file is part of the "Irrlicht Engine".
// For conditions of distribution and use, see copyright notice in irrlicht.h
//
// created by Dean Wadsworth aka Varmint Dec 31 2007

#ifndef VOLUMELIGHTNODE_H
#define VOLUMELIGHTNODE_H

#include "Graphic/Scene/Hierarchy/Node.h"

class GRAPHIC_ITEM VolumeLightNode : public Node
{
public:

	//! constructor
	VolumeLightNode(const ActorId actorId, PVWUpdater* updater, const Vector2<float>& textureSize,
		const eastl::shared_ptr<Texture2>& texture, const Vector2<int>& subdivision = Vector2<int>{32, 32},
		const eastl::array<float, 4>& foot = eastl::array<float, 4>{51.f, 0.f, 230.f, 180.f},
		const eastl::array<float, 4>& tail = eastl::array<float, 4>{0.f, 0.f, 0.f, 0.f});

	virtual ~VolumeLightNode();

	//! Renders event
	virtual bool PreRender(Scene *pScene);
	virtual bool Render(Scene *pScene);

	//! Returns type of the scene node
	virtual NodeType GetType() const { return NT_VOLUME_LIGHT; }

	//! Sets the number of segments across the U axis
	void SetSubDivideU(const unsigned int inU);

	//! Sets the number of segments across the V axis
	void SetSubDivideV(const unsigned int inV);

	//! Returns the number of segments across the U axis
	unsigned int GetSubDivideU() const { return mSubdivideU; }

	//! Returns the number of segments across the V axis
	unsigned int GetSubDivideV() const { return mSubdivideV; }

	//! Sets the color of the base of the light
	void SetFootColor(const eastl::array<float, 4> inColor);

	//! Sets the color of the tip of the light
	void SetTailColor(const eastl::array<float, 4> inColor);

	//! Returns the color of the base of the light
	eastl::array<float, 4> GetFootColor() const { return mFootColor; }

	//! Returns the color of the tip of the light
	eastl::array<float, 4> GetTailColor() const { return mTailColor; }

	//! returns the material based on the zero based index i. To get the amount
	//! of materials used by this scene node, use GetMaterialCount().
	//! This function is needed for inserting the node into the scene hirachy on a
	//! optimal position for minimizing renderstate changes, but can also be used
	//! to directly modify the material of a scene node.
	virtual eastl::shared_ptr<Material> const& GetMaterial(unsigned int i);

	//! returns amount of materials used by this scene node.
	virtual unsigned int GetMaterialCount() const;

	//! Sets the texture of the specified layer in all materials of this scene node to the new texture.
	/** \param textureLayer Layer of texture to be set. Must be a
	value smaller than MATERIAL_MAX_TEXTURES.
	\param texture New texture to be used. */
	virtual void SetMaterialTexture(unsigned int textureLayer, eastl::shared_ptr<Texture2> texture);

	//! Sets the material type of all materials in this scene node to a new material type.
	/** \param newType New type of material to be set. */
	virtual void SetMaterialType(MaterialType newType);

private:

	void ConstructLight();

	struct Vertex
	{
		Vector3<float> position;
		Vector2<float> tcoord;
		Vector4<float> color;
	};
	void AddToBuffer(Vector3<float>& position,
		eastl::array<float, 4U>& color, Vector2<float>& tcoord,
		Vertex* vertex, unsigned int& vtx, unsigned int* index, unsigned int& idx);

	eastl::shared_ptr<BlendState> mBlendState;
	eastl::shared_ptr<DepthStencilState> mDepthStencilState;
	eastl::shared_ptr<RasterizerState> mRasterizerState;

	eastl::shared_ptr<Texture2> mTexture;
	eastl::shared_ptr<VisualEffect> mEffect;
	eastl::shared_ptr<MeshBuffer> mMeshBuffer;
	eastl::shared_ptr<Visual> mVisual;

	float  mLPDistance;		// Distance to hypothetical lightsource point -- affects fov angle

	unsigned int  mSubdivideU;		// Number of subdivisions in U and V space.
	unsigned int  mSubdivideV;		// Controls the number of "slices" in the volume.
	// NOTE : Total number of polygons = 2 + ((SubdivideU + 1) + (SubdivideV + 1)) * 2
	// Each slice being a quad plus the rectangular plane at the bottom.

	eastl::array<float, 4> mFootColor;	// Color at the source
	eastl::array<float, 4> mTailColor;	// Color at the end.

	// LightDimensions.Y Distance of shooting -- Length of beams
	// LightDimensions.X and LightDimensions.Z determine the
	// size/dimension of the plane
	Vector3<float> mLightDimensions;
};

#endif

