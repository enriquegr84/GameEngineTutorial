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
	MeshNode(const ActorId actorId, WeakBaseRenderComponentPtr renderComponent,
		const eastl::shared_ptr<Mesh>& mesh);
/*
			const Vector3<float>& position = Vector3<float>(0,0,0),
			const Vector3<float>& rotation = Vector3<float>(0,0,0),
			const Vector3<float>& scale = Vector3<float>(1.0f, 1.0f, 1.0f));
*/

	//! destructor
	~MeshNode();

	//! Render events
	bool PreRender(Scene *pScene);
	bool Render(Scene *pScene);
	
	//! Removes a child from this scene node.
	//! Implemented here, to be able to remove the shadow properly, if there is one,
	//! or to remove attached childs.
	bool RemoveChild(ActorId id);


	//! returns the axis aligned bounding box of this node
	const AABBox3<float>& GetBoundingBox() const;

	//! returns the material based on the zero based index i. To get the amount
	//! of materials used by this scene node, use GetMaterialCount().
	//! This function is needed for inserting the node into the scene hirachy on a
	//! optimal position for minimizing renderstate changes, but can also be used
	//! to directly modify the material of a scene node.
	Material& GetMaterial(unsigned int i);

	//! returns amount of materials used by this scene node.
	unsigned int GetMaterialCount() const;

	//! Sets a new mesh
	void SetMesh(const eastl::shared_ptr<Mesh>& mesh);

	//! Returns the current mesh
	const eastl::shared_ptr<Mesh>& GetMesh(void) { return mMesh; }

	//! Creates shadow volume scene node as child of this node
	//! and returns a pointer to it.
	eastl::shared_ptr<ShadowVolumeNode> AddShadowVolumeNode(const ActorId actorId,
		Scene* pScene, const eastl::shared_ptr<Mesh>& shadowMesh = 0, bool zfailmethod=true, float infinity=10000.0f);

	//! Sets if the scene node should not copy the materials of the mesh but use them in a read only style.
	/* In this way it is possible to change the materials a mesh causing all mesh scene nodes
	referencing this mesh to change too. */
	void SetReadOnlyMaterials(bool readonly);

	//! Returns if the scene node should not copy the materials of the mesh but use them in a read only style
	bool IsReadOnlyMaterials() const;

protected:

	void CopyMaterials();

	eastl::vector<Material> mMaterials;
	AABBox3<float> mBBox;
	Material mReadOnlyMaterial;

	eastl::shared_ptr<Mesh> mMesh;
	eastl::shared_ptr<ShadowVolumeNode> mShadow;

	int mPassCount;
	bool mReadOnlyMaterials;
};


#endif

