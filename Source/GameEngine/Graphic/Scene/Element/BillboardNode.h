// David Eberly, Geometric Tools, Redmond WA 98052
// Copyright (c) 1998-2017
// Distributed under the Boost Software License, Version 1.0.
// http://www.boost.org/LICENSE_1_0.txt
// http://www.geometrictools.com/License/Boost/LICENSE_1_0.txt
// File Version: 3.0.0 (2016/06/19)

#ifndef BILLBOARDNODE_H
#define BILLBOARDNODE_H

#include "Graphic/Effect/VisualEffect.h"

#include "Graphic/Scene/Hierarchy/Node.h"

//! Scene node which is a billboard. A billboard is like a 3d sprite: A 2d element,
//! which always looks to the camera.
class GRAPHIC_ITEM BillboardNode : public Node
{
public:
    // The model space of the billboard has an up vector of (0,1,0) that is
    // chosen to be the billboard's axis of rotation.
	//! Constructor
	BillboardNode(const ActorId actorId, PVWUpdater* updater,
		const eastl::shared_ptr<Texture2>& texture, const Vector2<float>& size);

	//! Returns type of the scene node
	virtual NodeType GetType() const { return NT_BILLBOARD; }

	//! Renders event
	virtual bool PreRender(Scene *pScene);
	virtual bool Render(Scene *pScene);

	//! sets the size of the billboard
	void SetSize(const Vector2<float>& size);

	//! gets the size of the billboard
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
	/** \param textureLayer Layer of texture to be set. Must be a value smaller than MATERIAL_MAX_TEXTURES.
	\param texture New texture to be used. */
	virtual void SetMaterialTexture(unsigned int textureLayer, eastl::shared_ptr<Texture2> texture);

	//! Sets the material type of all materials in this scene node to a new material type.
	/** \param newType New type of material to be set. */
	virtual void SetMaterialType(MaterialType newType);

private:

	void DoBillboardBuffers(Scene *pScene);

	//! Size.Width is the bottom edge width
	Vector2<float> mSize;

	eastl::shared_ptr<BlendState> mBlendState;
	eastl::shared_ptr<DepthStencilState> mDepthStencilState;
	eastl::shared_ptr<RasterizerState> mRasterizerState;

	eastl::shared_ptr<VisualEffect> mEffect;
	eastl::shared_ptr<MeshBuffer> mMeshBuffer;
	eastl::shared_ptr<Visual> mVisual;
};

#endif