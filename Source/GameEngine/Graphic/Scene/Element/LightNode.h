// Copyright (C) 2002-2012 Nikolaus Gebhardt
// This file is part of the "Irrlicht Engine".
// For conditions of distribution and use, see copyright notice in irrlicht.h

#ifndef LIGHTNODE_H
#define LIGHTNODE_H

#include "Graphic/Effect/VisualEffect.h"

#include "Graphic/Scene/Hierarchy/Node.h"
#include "Graphic/Scene/Hierarchy/Light.h"

#include "Graphic/Scene/Element/ViewVolumeNode.h"

//! Scene node which is a dynamic light. You can switch the light on and off by
//! making it visible or not, and let it be animated by ordinary scene node animators.
class LightNode : public Node
{
public:	

	//! Constructor
	LightNode(const ActorId actorId, PVWUpdater* updater, 
		WeakBaseRenderComponentPtr renderComponent, 
		const eastl::shared_ptr<Texture2>& texture, 
		const Vector2<float>& size, const eastl::shared_ptr<Light>& light);

	//! Renders event
	virtual bool PreRender(Scene *pScene);
	virtual bool Render(Scene *pScene);

	//! Gets the light type.
	/** \return The current light type. */
	LightType GetLightType() const;

	//! Gets the light.
	/** \return The current light. */
	eastl::shared_ptr<Light> const& GetLight() const;

	//! sets the size of the lightnode
	void SetSize(const Vector2<float>& size);

	//! gets the size of the lightnode
	const Vector2<float>& GetSize() const;

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
	/** \param textureLayer Layer of texture to be set. Must be a
	value smaller than MATERIAL_MAX_TEXTURES.
	\param texture New texture to be used. */
	virtual void SetMaterialTexture(unsigned int textureLayer, eastl::shared_ptr<Texture2> texture);

	//! Sets the material type of all materials in this scene node to a new material type.
	/** \param newType New type of material to be set. */
	virtual void SetMaterialType(MaterialType newType);

private:

	void DoLightRecalc();
	void DoLightBuffers(Scene* pScene);

	eastl::shared_ptr<Light> mLight;
	//! Size.Width is the bottom edge width
	Vector2<float> mSize;

	eastl::shared_ptr<BlendState> mBlendState;
	eastl::shared_ptr<DepthStencilState> mDepthStencilState;

	eastl::shared_ptr<VisualEffect> mEffect;
	eastl::shared_ptr<MeshBuffer> mMeshBuffer;
	eastl::shared_ptr<Visual> mVisual;
};

#endif

