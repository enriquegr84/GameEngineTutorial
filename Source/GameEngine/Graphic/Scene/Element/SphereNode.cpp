// Copyright (C) 2002-2012 Nikolaus Gebhardt
// This file is part of the "Irrlicht Engine".
// For conditions of distribution and use, see copyright notice in irrlicht.h

#include "SphereNode.h"

#include "Graphic/Scene/Hierarchy/Node.h"
#include "Graphic/Renderer/Renderer.h"
#include "Graphic/Scene/Scene.h"

//! constructor
SphereNode::SphereNode(const ActorId actorId, PVWUpdater* updater, WeakBaseRenderComponentPtr renderComponent,
		float radius, unsigned int polyCountX, unsigned int polyCountY)
:	Node(actorId, renderComponent, RP_NONE, NT_CUBE), mShadow(0),
	mRadius(radius), mPolyCountX(polyCountX), mPolyCountY(polyCountY)
{
	mPVWUpdater = updater;

	struct Vertex
	{
		Vector3<float> position;
		Vector4<float> color;
	};
	VertexFormat vformat;
	vformat.Bind(VA_POSITION, DF_R32G32B32_FLOAT, 0);
	vformat.Bind(VA_COLOR, DF_R32G32B32A32_FLOAT, 0);

	MeshFactory mf;
	mf.SetVertexFormat(vformat);
	mVisual = mf.CreateSphere(polyCountX, polyCountY, radius);

	eastl::string path = FileSystem::Get()->GetPath("Effects/AmbientLightEffect.hlsl");
	eastl::shared_ptr<AmbientLightEffect> effect = eastl::make_shared<AmbientLightEffect>(
		ProgramFactory::Get(), mPVWUpdater->GetUpdater(), path, eastl::make_shared<Material>(), 
		eastl::make_shared<Lighting>());
	mVisual->SetEffect(effect);
	mVisual->UpdateModelNormals();
	mPVWUpdater->Subscribe(mVisual->GetAbsoluteTransform(), effect->GetPVWMatrixConstant());
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
		{
			eastl::shared_ptr<AmbientLightEffect> effect =
				eastl::static_pointer_cast<AmbientLightEffect>(mVisual->GetEffect());

			if (effect->GetMaterial()->IsTransparent())
				++transparentCount;
			else
				++solidCount;
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
	//Matrix4x4<float> toWorld, fromWorld;
	//Get()->Transform(&toWorld, &fromWorld);

	if (!Renderer::Get())
		return false;

	//Renderer::Get()->SetMaterial(mMesh->GetMeshBuffer(0)->GetMaterial());
	//Renderer::Get()->SetTransform(TS_WORLD, toWorld);
	if (mShadow)
		mShadow->UpdateShadowVolumes(pScene);

	//Renderer::Get()->DrawMeshBuffer(mMesh->GetMeshBuffer(0));
	Renderer::Get()->Draw(mVisual);
	/*
	if (DebugDataVisible() & DS_BBOX )
	{
		Material m;
		m.mLighting = false;
		Renderer::Get()->SetMaterial(m);
		Renderer::Get()->Draw3DBox(
			mMesh->GetMeshBuffer(0)->GetBoundingBox(), 
			eastl::array<float, 4>{255.f, 255.f, 255.f, 255.f});
	}
	*/
	return Node::Render(pScene);
}


//! Removes a child from this scene node.
//! Implemented here, to be able to remove the shadow properly, if there is one,
//! or to remove attached childs.
int SphereNode::DetachChild(eastl::shared_ptr<Node> const& child)
{
	if (child && mShadow == child)
		mShadow = 0;

	if (Node::DetachChild(child))
		return true;

	return false;
}


//! Creates shadow volume scene node as child of this node
//! and returns a pointer to it.
eastl::shared_ptr<ShadowVolumeNode> SphereNode::AddShadowVolumeNode(const ActorId actorId,
	Scene* pScene, const eastl::shared_ptr<BaseMesh>& shadowMesh, bool zfailmethod, float infinity)
{
	/*
	if (!Renderer::Get()->QueryFeature(VDF_STENCIL_BUFFER))
		return nullptr;
	*/
	mShadow = eastl::shared_ptr<ShadowVolumeNode>(new ShadowVolumeNode(
		actorId, mPVWUpdater, WeakBaseRenderComponentPtr(), shadowMesh, zfailmethod, infinity));
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
	eastl::shared_ptr<AmbientLightEffect> effect =
		eastl::static_pointer_cast<AmbientLightEffect>(mVisual->GetEffect());
	return effect->GetMaterial();
}


//! returns amount of materials used by this scene node.
unsigned int SphereNode::GetMaterialCount() const
{
	return 1;
}