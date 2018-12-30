// Copyright (C) 2002-2012 Nikolaus Gebhardt
// This file is part of the "Irrlicht Engine".
// For conditions of distribution and use, see copyright notice in irrlicht.h

#ifndef MESHSNODE_H
#define MESHSNODE_H

#include "Graphic/Scene/Hierarchy/Node.h"

#include "ShadowVolumeNode.h"

class MeshNode : public Node
{
public:

	//! constructor
	MeshNode(const ActorId actorId, PVWUpdater* updater, 
		WeakBaseRenderComponentPtr renderComponent, const eastl::shared_ptr<BaseMesh>& mesh);

	//! destructor
	virtual ~MeshNode();

	//! Render events
	virtual bool PreRender(Scene *pScene);
	virtual bool Render(Scene *pScene);
	
	//! Removes a child from this scene node.
	//! Implemented here, to be able to remove the shadow properly, if there is one,
	//! or to remove attached childs.
	virtual int DetachChild(eastl::shared_ptr<Node> const& child);

	//! Sets a new mesh
	void SetMesh(const eastl::shared_ptr<BaseMesh>& mesh);

	//! Returns the current mesh
	const eastl::shared_ptr<BaseMesh>& GetMesh(void) { return mMesh; }

	//! Creates shadow volume scene node as child of this node
	//! and returns a pointer to it.
	eastl::shared_ptr<ShadowVolumeNode> AddShadowVolumeNode(const ActorId actorId,
		Scene* pScene, const eastl::shared_ptr<BaseMesh>& shadowMesh = 0, bool zfailmethod=true, float infinity=10000.0f);

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

	//! Sets if the scene node should not copy the materials of the mesh but use them in a read only style.
	/** In this way it is possible to change the materials a mesh
	causing all mesh scene nodes referencing this mesh to change too. */
	void SetReadOnlyMaterials(bool readonly);

	//! Returns if the scene node should not copy the materials of the mesh but use them in a read only style
	bool IsReadOnlyMaterials() const;

protected:

	eastl::vector<eastl::shared_ptr<Material>> mMaterials;
	eastl::vector<eastl::shared_ptr<BlendState>> mBlendStates;
	eastl::vector<eastl::shared_ptr<DepthStencilState>> mDepthStencilStates;
	eastl::shared_ptr<RasterizerState> mRasterizerState;

	eastl::vector<eastl::shared_ptr<Visual>> mVisuals;
	eastl::shared_ptr<BaseMesh> mMesh;
	eastl::shared_ptr<ShadowVolumeNode> mShadow;

	int mPassCount;
	bool mReadOnlyMaterials;
};


#endif

