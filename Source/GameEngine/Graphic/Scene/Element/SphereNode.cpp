// Copyright (C) 2002-2012 Nikolaus Gebhardt
// This file is part of the "Irrlicht Engine".
// For conditions of distribution and use, see copyright notice in irrlicht.h

#include "SphereNode.h"

#include "Graphic/Scene/Scene.h"

//! constructor
SphereNode::SphereNode(const ActorId actorId, PVWUpdater& updater, WeakBaseRenderComponentPtr renderComponent,
		float radius, unsigned int polyCountX, unsigned int polyCountY)
:	Node(actorId, renderComponent, RP_NONE, NT_CUBE), mMesh(0), mShadow(0),
	mRadius(radius), mPolyCountX(polyCountX), mPolyCountY(polyCountY)
{
	#ifdef _DEBUG
	//setDebugName("CSphereSceneNode");
	#endif
	mPVWUpdater = updater;
	mMesh = eastl::make_shared<BaseMesh>(CreateSphereMesh(radius, polyCountX, polyCountY));
}


//! destructor
SphereNode::~SphereNode()
{

}

//! prerender
bool SphereNode::PreRender(Scene *pScene)
{
	if (IsVisible())
	{
		// because this node supports rendering of mixed mode meshes consisting of
		// transparent and solid material at the same time, we need to go through all
		// materials, check of what type they are and register this node for the right
		// render pass according to that.

		int transparentCount = 0;
		int solidCount = 0;

		// count transparent and solid materials in this scene node
		if (mMesh)
		{
			// count mesh materials
			for (unsigned int i=0; i<mMesh->GetMeshBufferCount(); ++i)
			{
				const eastl::shared_ptr<MeshBuffer<float>>& mb = mMesh->GetMeshBuffer(i);

				if (mb->GetMaterial().IsTransparent())
					++transparentCount;
				else
					++solidCount;

				if (solidCount && transparentCount)
					break;
			}
		}

		// register according to material types counted
		if (!pScene->IsCulled(this))
		{
			if (solidCount)
				pScene->AddToRenderQueue(RP_SOLID, shared_from_this());

			if (transparentCount)
				pScene->AddToRenderQueue(RP_TRANSPARENT, shared_from_this());
		}
	}

	return Node::PreRender(pScene);
}

//! renders the node.
bool SphereNode::Render(Scene *pScene)
{
	Matrix4x4<float> toWorld, fromWorld;
	//Get()->Transform(&toWorld, &fromWorld);

	if (mMesh && renderer)
	{
		Renderer::Get()->SetMaterial(mMesh->GetMeshBuffer(0)->GetMaterial());
		Renderer::Get()->SetTransform(TS_WORLD, toWorld);
		if (mShadow)
			mShadow->UpdateShadowVolumes(pScene);

		renderer->DrawMeshBuffer(mMesh->GetMeshBuffer(0));
		if (DebugDataVisible() & DS_BBOX )
		{
			Material m;
			m.mLighting = false;
			renderer->SetMaterial(m);
			renderer->Draw3DBox(
				mMesh->GetMeshBuffer(0)->GetBoundingBox(), 
				eastl::array<float, 4>{255.f, 255.f, 255.f, 255.f});
		}
	}

	return Node::Render(pScene);
}


//! Removes a child from this scene node.
//! Implemented here, to be able to remove the shadow properly, if there is one,
//! or to remove attached childs.
bool SphereNode::RemoveChild(ActorId id)
{
	const eastl::shared_ptr<Node>& child = GetChild(id);
	if (child && mShadow == child)
		mShadow = 0;

	if (Node::DetachChild(child))
		return true;

	return false;
}


//! Creates shadow volume scene node as child of this node
//! and returns a pointer to it.
const eastl::shared_ptr<ShadowVolumeNode>& SphereNode::AddShadowVolumeNode(const ActorId actorId,
	Scene* pScene, const eastl::shared_ptr<BaseMesh>& shadowMesh, bool zfailmethod, float infinity)
{
	/*
	if (!Renderer::Get()->QueryFeature(VDF_STENCIL_BUFFER))
	return 0;
	*/
	mShadow = eastl::shared_ptr<ShadowVolumeNode>(
		new ShadowVolumeNode(actorId, WeakBaseRenderComponentPtr(),
			shadowMesh ? shadowMesh : mMesh, zfailmethod, infinity));
	shared_from_this()->AttachChild(mShadow);

	return mShadow;
}

//! returns the material based on the zero based index i. To get the amount
//! of materials used by this scene node, use getMaterialCount().
//! This function is needed for inserting the node into the scene hirachy on a
//! optimal position for minimizing renderstate changes, but can also be used
//! to directly modify the material of a scene node.
eastl::shared_ptr<Material> const& SphereNode::GetMaterial(unsigned int i)
{
	return mMesh->GetMeshBuffer(0)->GetMaterial();
}


//! returns amount of materials used by this scene node.
unsigned int SphereNode::GetMaterialCount() const
{
	return 1;
}