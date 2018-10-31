// Copyright (C) 2002-2012 Nikolaus Gebhardt
// This file is part of the "Irrlicht Engine".
// For conditions of distribution and use, see copyright notice in irrlicht.h

#ifndef CUBENODE_H
#define CUBENODE_H

#include "Graphic/Effect/LightingEffect.h"
#include "Graphic/Scene/Hierarchy/Node.h"

#include "ShadowVolumeNode.h"

class CubeNode : public Node
{
public:

	//! constructor
	CubeNode(const ActorId actorId, PVWUpdater* updater, WeakBaseRenderComponentPtr renderComponent, 
		const eastl::shared_ptr<Texture2>& texture, float texxScale, float texyScale, float size);

	virtual ~CubeNode();

	//! Renders event
	virtual bool PreRender(Scene *pScene);
	virtual bool Render(Scene *pScene);

	//! Returns type of the scene node
	virtual NodeType GetType() const { return NT_CUBE; }

	//! Creates shadow volume scene node as child of this node
	//! and returns a pointer to it.
	eastl::shared_ptr<ShadowVolumeNode> AddShadowVolumeNode(const ActorId actorId, 
		Scene* pScene, const eastl::shared_ptr<BaseMesh>& shadowMesh = 0, bool zfailmethod=true, float infinity=10000.0f);

	//! Removes a child from this scene node.
	//! Implemented here, to be able to remove the shadow properly, if there is one,
	//! or to remove attached childs.
	virtual int DetachChild(eastl::shared_ptr<Node> const& child);

	//! Returns the visual based on the zero based index i. To get the amount 
	//! of visuals used by this scene node, use GetVisualCount(). 
	//! This function is needed for inserting the node into the scene hierarchy 
	//! at an optimal position for minimizing renderstate changes, but can also 
	//! be used to directly modify the visual of a scene node.
	virtual eastl::shared_ptr<Visual> const& GetVisual(unsigned int i);

	//! return amount of visuals of this scene node.
	virtual unsigned int GetVisualCount() const;

	//! returns the material based on the zero based index i. To get the amount
	//! of materials used by this scene node, use GetMaterialCount().
	//! This function is needed for inserting the node into the scene hirachy on a
	//! optimal position for minimizing renderstate changes, but can also be used
	//! to directly modify the material of a scene node.
	virtual eastl::shared_ptr<Material> const& GetMaterial(unsigned int i);

	//! returns amount of materials used by this scene node.
	virtual unsigned int GetMaterialCount() const;

	//! Sets the texture of the specified layer in all materials of this scene node to the new texture.
	/** \param textureLayer Layer of texture to be set. Must be a value smaller than MATERIAL_MAX_TEXTURES.
	\param texture New texture to be used. */
	virtual void SetMaterialTexture(unsigned int textureLayer, eastl::shared_ptr<Texture2> texture);

	//! Sets the material type of all materials in this scene node to a new material type.
	/** \param newType New type of material to be set. */
	virtual void SetMaterialType(MaterialType newType);

private:

	eastl::shared_ptr<BlendState> mBlendState;
	eastl::shared_ptr<DepthStencilState> mDepthStencilState;
	eastl::shared_ptr<RasterizerState> mRasterizerState;

	eastl::shared_ptr<Visual> mVisual;
	eastl::shared_ptr<LightingEffect> mEffect;
	eastl::shared_ptr<Material> mMaterial;
	eastl::shared_ptr<ShadowVolumeNode> mShadow;
	float mSize;
};

#endif