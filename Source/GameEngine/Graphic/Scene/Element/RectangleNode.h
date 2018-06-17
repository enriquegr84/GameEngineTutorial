// Copyright (C) 2002-2012 Nikolaus Gebhardt
// This file is part of the "Irrlicht Engine".
// For conditions of distribution and use, see copyright notice in irrlicht.h

#ifndef RECTANGLENODE_H
#define RECTANGLENODE_H

#include "Graphic/Scene/Hierarchy/Node.h"

#include "ShadowVolumeNode.h"

class RectangleNode : public Node
{
public:

	//! constructor
	RectangleNode(const ActorId actorId, PVWUpdater& updater, 
		WeakBaseRenderComponentPtr renderComponent, const eastl::shared_ptr<Texture2>& texture,
		float xSize, float ySize, int xPolyCount, int yPolyCount);

	~RectangleNode();

	//! Renders event
	virtual bool PreRender(Scene *pScene);
	virtual bool Render(Scene *pScene);

	//! returns the material based on the zero based index i. To get the amount
	//! of materials used by this scene node, use getMaterialCount().
	//! This function is needed for inserting the node into the scene hirachy on a
	//! optimal position for minimizing renderstate changes, but can also be used
	//! to directly modify the material of a scene node.
	virtual eastl::shared_ptr<Material> const& GetMaterial(unsigned int i);

	//! returns amount of materials used by this scene node.
	virtual unsigned int GetMaterialCount() const;

	//! Returns type of the scene node
	virtual NodeType GetType() const { return NT_CUBE; }

	//! Creates shadow volume scene node as child of this node
	//! and returns a pointer to it.
	eastl::shared_ptr<ShadowVolumeNode> AddShadowVolumeNode(const ActorId actorId, 
		Scene* pScene, const eastl::shared_ptr<BaseMesh>& shadowMesh = 0, bool zfailmethod=true, float infinity=10000.0f);

	//! Sets if the scene node should not copy the materials of the mesh but use them in a read only style.
	/* In this way it is possible to change the materials a mesh causing all mesh scene nodes 
	referencing this mesh to change too. */
	void SetReadOnlyMaterials(bool readonly) {}

	//! Returns if the scene node should not copy the materials of the mesh but use them in a read only style
	bool IsReadOnlyMaterials() const { return false; }

	//! Removes a child from this scene node.
	//! Implemented here, to be able to remove the shadow properly, if there is one,
	//! or to remove attached childs.
	virtual int DetachChild(eastl::shared_ptr<Node> const& child);

private:

	eastl::shared_ptr<Visual> mVisual;
	eastl::shared_ptr<ShadowVolumeNode> mShadow;

	float mSizeX;
	float mSizeY;
	unsigned int mPolyCountX;
	unsigned int mPolyCountY;
};

#endif