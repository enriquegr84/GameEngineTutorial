// Copyright (C) 2002-2012 Nikolaus Gebhardt
// This file is part of the "Irrlicht Engine".
// For conditions of distribution and use, see copyright notice in irrlicht.h

#include "CubeNode.h"

#include "Graphic/Scene/Hierarchy/Node.h"
#include "Graphic/Renderer/Renderer.h"
#include "Graphic/Scene/Scene.h"

//#include "Scenes/Mesh/MeshBuffer.h"

	/*
        011         111
          /6,8------/5        y
         /  |      / |        ^  z
        /   |     /  |        | /
    010 3,9-------2  |        |/
        |   7- - -10,4 101     *---->x
        |  /      |  /
        |/        | /
        0------11,1/
       000       100
	*/

//! constructor
CubeNode::CubeNode(const ActorId actorId, PVWUpdater& updater, 
	WeakBaseRenderComponentPtr renderComponent, float size)
	:	Node(actorId, renderComponent, RP_NONE, NT_CUBE), mSize(size), mShadow(0)
{
	#ifdef _DEBUG
	//setDebugName("CubeSceneNode");
	#endif
	mPVWUpdater = updater;
	SetSize();
}


CubeNode::~CubeNode()
{

}


void CubeNode::SetSize()
{
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
	mVisual = mf.CreateBox(mSize, mSize, mSize);

	eastl::string path = FileSystem::Get()->GetPath("Effects/AmbientLightEffect.hlsl");
	eastl::shared_ptr<AmbientLightEffect> effect = eastl::make_shared<AmbientLightEffect>(
		ProgramFactory::Get(), mPVWUpdater.GetUpdater(), path, eastl::make_shared<Material>(),
		eastl::make_shared<Lighting>());
	mVisual->SetEffect(effect);
	mPVWUpdater.Subscribe(mVisual->GetAbsoluteTransform(), effect->GetPVWMatrixConstant());
}


//! prerender
bool CubeNode::PreRender(Scene *pScene)
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

//
// CubeSceneNode::Render					- Chapter 16, page 550
//
bool CubeNode::Render(Scene *pScene)
{
	if (!Renderer::Get())
		return false;

	if (mShadow)
		mShadow->UpdateShadowVolumes(pScene);

	// for debug purposes only:
	eastl::shared_ptr<AmbientLightEffect> effect =
		eastl::static_pointer_cast<AmbientLightEffect>(mVisual->GetEffect());
	eastl::shared_ptr<Material> material = effect->GetMaterial();

	// overwrite half transparency
	if (DebugDataVisible() & DS_HALF_TRANSPARENCY)
		material->mType = MT_TRANSPARENT_ADD_COLOR;
	//effect->SetMaterial(material);
	Renderer::Get()->Draw(mVisual);

	/*
	// for debug purposes only:
	if (DebugDataVisible())
	{
		Material m;
		m.mLighting = false;
		m.mAntiAliasing=0;
		Renderer::Get()->SetMaterial(m);

		if (DebugDataVisible() & DS_BBOX)
		{
			Renderer::Get()->Draw3DBox(
				mMesh->GetMeshBuffer(0)->GetBoundingBox(), eastl::array<float, 4>{255.f, 255.f, 255.f, 255.f});
		}
		if (DebugDataVisible() & DS_BBOX_BUFFERS)
		{
			Renderer::Get()->Draw3DBox(
				mMesh->GetMeshBuffer(0)->GetBoundingBox(), eastl::array<float, 4>{255.f, 190.f, 128.f, 128.f});
		}
		if (DebugDataVisible() & DS_NORMALS)
		{
			// draw normals
			//const f32 debugNormalLength = pScene->GetParameters()->GetAttributeAsFloat(DEBUG_NORMAL_LENGTH);
			//const Color debugNormalColor = pScene->GetParameters()->GetAttributeAColor(DEBUG_NORMAL_COLOR);

			// draw normals
			const float debugNormalLength = 1.f;
			const eastl::array<float, 4> debugNormalColor{ 255.f, 34.f, 221.f, 221.f };
			const unsigned int count = mMesh->GetMeshBufferCount();

			for (unsigned int i=0; i != count; ++i)
			{
				Renderer::Get()->DrawMeshBufferNormals(
					mMesh->GetMeshBuffer(i), debugNormalLength, debugNormalColor);
			}
		}

		// show mesh
		if (DebugDataVisible() & DS_MESH_WIRE_OVERLAY)
		{
			m.mWireframe = true;
			Renderer::Get()->SetMaterial(m);
			Renderer::Get()->DrawMeshBuffer(mMesh->GetMeshBuffer(0));
		}
	}
	*/
	return true;
}


//! Removes a child from this scene node.
//! Implemented here, to be able to remove the shadow properly, if there is one,
//! or to remove attached childs.
bool CubeNode::RemoveChild(ActorId id)
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
eastl::shared_ptr<ShadowVolumeNode> CubeNode::AddShadowVolumeNode(const ActorId actorId,
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


//! returns the material based on the zero based index i.
eastl::shared_ptr<Material> const& CubeNode::GetMaterial(unsigned int i)
{
	eastl::shared_ptr<AmbientLightEffect> effect =
		eastl::static_pointer_cast<AmbientLightEffect>(mVisual->GetEffect());
	return effect->GetMaterial();
}


//! returns amount of materials used by this scene node.
unsigned int CubeNode::GetMaterialCount() const
{
	return 1;
}