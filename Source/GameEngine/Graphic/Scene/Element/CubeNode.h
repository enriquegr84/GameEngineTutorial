// Copyright (C) 2002-2012 Nikolaus Gebhardt
// This file is part of the "Irrlicht Engine".
// For conditions of distribution and use, see copyright notice in irrlicht.h

#ifndef CUBENODE_H
#define CUBENODE_H

#include "Graphic/Scene/Hierarchy/Node.h"

#include "ShadowVolumeNode.h"

class CubeNode : public Node
{
public:

	//! constructor
	CubeNode(const ActorId actorId, WeakBaseRenderComponentPtr renderComponent, float size);
	/*
		const Vector3<float>& position = Vector3<float>{ 0.f, 0.f, 0.f },
		const Vector3<float>& rotation = Vector3<float>{ 0.f, 0.f, 0.f },
		const Vector3<float>& scale = Vector3<float>{ 255.f, 255.f, 255.f });
	*/
	~CubeNode();

	//! Renders event
	bool PreRender(Scene *pScene);
	bool Render(Scene *pScene);

	//! returns the material based on the zero based index i. To get the amount
	//! of materials used by this scene node, use getMaterialCount().
	//! This function is needed for inserting the node into the scene hirachy on a
	//! optimal position for minimizing renderstate changes, but can also be used
	//! to directly modify the material of a scene node.
	Material& GetMaterial(unsigned int i);

	//! returns amount of materials used by this scene node.
	unsigned int GetMaterialCount() const;

	//! Returns type of the scene node
	NodeType GetType() const { return NT_CUBE; }

	//! Creates shadow volume scene node as child of this node
	//! and returns a pointer to it.
	eastl::shared_ptr<ShadowVolumeNode> AddShadowVolumeNode(const ActorId actorId, 
		Scene* pScene, const eastl::shared_ptr<BaseMesh>& shadowMesh = 0, bool zfailmethod=true, float infinity=10000.0f);

	//! Returns the current mesh
	eastl::shared_ptr<StandardMesh> GetMesh(void) { return mMesh; }

	//! Sets if the scene node should not copy the materials of the mesh but use them in a read only style.
	/* In this way it is possible to change the materials a mesh causing all mesh scene nodes 
	referencing this mesh to change too. */
	void SetReadOnlyMaterials(bool readonly) {}

	//! Returns if the scene node should not copy the materials of the mesh but use them in a read only style
	bool IsReadOnlyMaterials() const { return false; }

	//! Removes a child from this scene node.
	//! Implemented here, to be able to remove the shadow properly, if there is one,
	//! or to remove attached childs.
	bool RemoveChild(ActorId id);

private:
	void SetSize();

	eastl::shared_ptr<StandardMesh> mMesh;
	eastl::shared_ptr<ShadowVolumeNode> mShadow;
	float mSize;
};

#endif