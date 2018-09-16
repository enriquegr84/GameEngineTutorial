// Copyright (C) 2002-2012 Nikolaus Gebhardt
// This file is part of the "Irrlicht Engine".
// For conditions of distribution and use, see copyright notice in irrlicht.h

#ifndef SHPERENODE_H
#define SHPERENODE_H

#include "Graphic/Scene/Hierarchy/Node.h"

#include "ShadowVolumeNode.h"

class SphereNode : public Node
{
public:

	//! constructor
	SphereNode(const ActorId actorId, PVWUpdater* updater, WeakBaseRenderComponentPtr renderComponent,
		float radius, unsigned int polyCountX, unsigned int polyCountY);

	virtual ~SphereNode();

	//! Returns type of the scene node
	virtual NodeType GetType() const { return NT_SPHERE; }

	//! Renders event
	virtual bool PreRender(Scene *pScene);
	virtual bool Render(Scene *pScene);

		//! Removes a child from this scene node.
	//! Implemented here, to be able to remove the shadow properly, if there is one,
	//! or to remove attached childs.
	virtual int DetachChild(eastl::shared_ptr<Node> const& child);

	//! Creates shadow volume scene node as child of this node
	//! and returns a pointer to it.
	eastl::shared_ptr<ShadowVolumeNode> AddShadowVolumeNode(const ActorId actorId,
		Scene* pScene, const eastl::shared_ptr<BaseMesh>& shadowMesh = 0, bool zfailmethod = true, float infinity = 10000.0f);

	//! returns the material based on the zero based index i. To get the amount
	//! of materials used by this scene node, use GetMaterialCount().
	//! This function is needed for inserting the node into the scene hirachy on a
	//! optimal position for minimizing renderstate changes, but can also be used
	//! to directly modify the material of a scene node.
	virtual eastl::shared_ptr<Material> const& GetMaterial(unsigned int i);

	//! returns amount of materials used by this scene node.
	virtual unsigned int GetMaterialCount() const;

	//! Sets all material flags at once to a new value.
	/** Useful, for example, if you want the whole mesh to be
	affected by light.
	\param flag Which flag of all materials to be set.
	\param newvalue New value of that flag. */
	virtual void SetMaterialFlag(MaterialFlag flag, bool newvalue);

	//! Sets the texture of the specified layer in all materials of this scene node to the new texture.
	/** \param textureLayer Layer of texture to be set. Must be a
	value smaller than MATERIAL_MAX_TEXTURES.
	\param texture New texture to be used. */
	virtual void SetMaterialTexture(unsigned int textureLayer, Texture2* texture);

	//! Sets the material type of all materials in this scene node to a new material type.
	/** \param newType New type of material to be set. */
	virtual void SetMaterialType(MaterialType newType);

	//! Sets if the scene node should not copy the materials of the mesh but use them in a read only style.
	/** In this way it is possible to change the materials a mesh
	causing all mesh scene nodes referencing this mesh to change too. */
	void SetReadOnlyMaterials(bool readonly);

	//! Returns if the scene node should not copy the materials of the mesh but use them in a read only style
	bool IsReadOnlyMaterials() const;

private:

	eastl::shared_ptr<Visual> mVisual;
	eastl::shared_ptr<Material> mMaterial;
	eastl::shared_ptr<ShadowVolumeNode> mShadow;

	float mRadius;
	unsigned int mPolyCountX;
	unsigned int mPolyCountY;
};

#endif

