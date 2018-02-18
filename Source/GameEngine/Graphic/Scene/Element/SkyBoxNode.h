// Copyright (C) 2002-2012 Nikolaus Gebhardt
// This file is part of the "Irrlicht Engine".
// For conditions of distribution and use, see copyright notice in irrlicht.h

#ifndef SKYBOXNODE_H
#define SKYBOXNODE_H

#include "Graphic/Scene/Hierarchy/Node.h"

// Skybox, rendered with zbuffer turned off, before all other nodes.
class SkyBoxNode : public Node
{
public:

	//! constructor
	SkyBoxNode(const ActorId actorId, PVWUpdater& updater, WeakBaseRenderComponentPtr renderComponent,
		const eastl::shared_ptr<Texture2>& top, const eastl::shared_ptr<Texture2>& bottom, 
		const eastl::shared_ptr<Texture2>& left, const eastl::shared_ptr<Texture2>& right, 
		const eastl::shared_ptr<Texture2>& front, const eastl::shared_ptr<Texture2>& back);

	//! Render events
	bool PreRender(Scene *pScene);
	bool Render(Scene *pScene);

	//! returns the material based on the zero based index i. To get the amount
	//! of materials used by this scene node, use getMaterialCount().
	//! This function is needed for inserting the node into the scene hirachy on a
	//! optimal position for minimizing renderstate changes, but can also be used
	//! to directly modify the material of a scene node.
	eastl::shared_ptr<Material> const& GetMaterial(unsigned int i);

	//! returns amount of materials used by this scene node.
	unsigned int GetMaterialCount() const;

	//! Returns type of the scene node
	NodeType GetType() const { return NT_SKY_BOX; }

private:

	unsigned int mIndices[4];
	VertexBuffer mVertices[4*6];
	Material mMaterials[6];
};


#endif

