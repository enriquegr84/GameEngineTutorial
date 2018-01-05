// Copyright (C) 2002-2012 Nikolaus Gebhardt
// This file is part of the "Irrlicht Engine".
// For conditions of distribution and use, see copyright notice in irrlicht.h

#include "SphereNode.h"

#include "Graphic/Scene/Scene.h"

//! constructor
SphereNode::SphereNode(const ActorId actorId, WeakBaseRenderComponentPtr renderComponent,
		float radius, unsigned int polyCountX, unsigned int polyCountY)
:	Node(actorId, renderComponent, ERP_NONE, ESNT_CUBE), mMesh(0), mShadow(0),
	mRadius(radius), mPolyCountX(polyCountX), mPolyCountY(polyCountY)
{
	#ifdef _DEBUG
	//setDebugName("CSphereSceneNode");
	#endif

	const eastl::shared_ptr<Scene>& pScene = gameApp->GetHumanView()->mScene;
	mMesh = eastl::shared_ptr<Mesh>(
		pScene->GetGeometryCreator()->CreateSphereMesh(radius, polyCountX, polyCountY));
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
				const eastl::shared_ptr<MeshBuffer>& mb = mMesh->GetMeshBuffer(i);
				const eastl::shared_ptr<MaterialRenderer>& rnd =
					mb ? renderer->GetMaterialRenderer(mb->GetMaterial().MaterialType) : 0;

				if (rnd && rnd->IsTransparent())
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
	Get()->Transform(&toWorld, &fromWorld);

	const eastl::shared_ptr<Renderer>& renderer = pScene->GetRenderer();

	if (mMesh && renderer)
	{
		renderer->SetMaterial(m_Mesh->GetMeshBuffer(0)->GetMaterial());
		renderer->SetTransform(ETS_WORLD, toWorld);
		if (mShadow)
			mShadow->UpdateShadowVolumes(pScene);

		renderer->DrawMeshBuffer(mMesh->GetMeshBuffer(0));
		if ( mProps.DebugDataVisible() & EDS_BBOX )
		{
			Material m;
			m.Lighting = false;
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
	for(SceneNodeList::iterator i=mChildren.begin(); i!=mChildren.end(); ++i)
	{
		if((mProps.ActorId() != INVALID_ACTOR_ID) && (id == mProps.ActorId()))
			if ((*i) && mShadow == (*i))
				mShadow = 0;
	}

	return Node::RemoveChild(id);
}


//! Creates shadow volume scene node as child of this node
//! and returns a pointer to it.
const shared_ptr<ShadowVolumeNode>& SphereNode::AddShadowVolumeNode(const ActorId actorId,
	Scene* pScene, const shared_ptr<IMesh>& shadowMesh, bool zfailmethod, f32 infinity)
{
	const eastl::shared_ptr<Renderer>& renderer = pScene->GetRenderer();

	if (renderer->QueryFeature(EVDF_STENCIL_BUFFER))
		return mShadow;

	mShadow = eastl::shared_ptr<ShadowVolumeNode>(
		new ShadowVolumeNode(actorId, WeakBaseRenderComponentPtr(), 
		&Matrix4x4<float>::Identity, shadowMesh ? shadowMesh : mMesh, zfailmethod, infinity));
	AddChild(mShadow);

	return mShadow;
}

//! returns the material based on the zero based index i. To get the amount
//! of materials used by this scene node, use getMaterialCount().
//! This function is needed for inserting the node into the scene hirachy on a
//! optimal position for minimizing renderstate changes, but can also be used
//! to directly modify the material of a scene node.
Material& SphereNode::GetMaterial(unsigned int i)
{
	if (i>0 || !mMesh)
		return Node::GetMaterial(i);
	else
		return mMesh->GetMeshBuffer(i)->GetMaterial();
}


//! returns amount of materials used by this scene node.
u32 SphereNode::GetMaterialCount() const
{
	return 1;
}