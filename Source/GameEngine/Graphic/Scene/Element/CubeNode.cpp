// Copyright (C) 2002-2012 Nikolaus Gebhardt
// This file is part of the "Irrlicht Engine".
// For conditions of distribution and use, see copyright notice in irrlicht.h

#include "CubeNode.h"

#include "Graphic/Scene/Hierarchy/Node.h"
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
CubeNode::CubeNode(const ActorId actorId, WeakBaseRenderComponentPtr renderComponent, float size)
	:	Node(actorId, renderComponent, ERP_NONE, ESNT_CUBE, t), mMesh(0), mSize(size), mShadow(0)
{
	#ifdef _DEBUG
	//setDebugName("CubeSceneNode");
	#endif

	SetSize();
}


CubeNode::~CubeNode()
{

}


void CubeNode::SetSize()
{
	const eastl::shared_ptr<Scene>& pScene = gameApp->GetHumanView()->mScene;
	mMesh = eastl::shared_ptr<Mesh>(
		pScene->GetGeometryCreator()->CreateCubeMesh(Vector3<float>(mSize)));
}


//! prerender
bool CubeNode::PreRender(Scene *pScene)
{
	if (Get()->IsVisible())
	{
		// because this node supports rendering of mixed mode meshes consisting of
		// transparent and solid material at the same time, we need to go through all
		// materials, check of what type they are and register this node for the right
		// render pass according to that.
		const eastl::shared_ptr<Renderer>& renderer = pScene->GetRenderer();

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
				pScene->AddToRenderQueue(ERP_SOLID, shared_from_this());

			if (transparentCount)
				pScene->AddToRenderQueue(ERP_TRANSPARENT, shared_from_this());
		}
	}

	return Node::PreRender(pScene);
}

//
// CubeSceneNode::Render					- Chapter 16, page 550
//
bool CubeNode::Render(Scene *pScene)
{
	Matrix4x4<float> toWorld, fromWorld;
	Get()->Transform(&toWorld, &fromWorld);

	const eastl::shared_ptr<Renderer>& renderer = pScene->GetRenderer();
	renderer->SetTransform(ETS_WORLD, toWorld);

	if (mShadow)
		mShadow->UpdateShadowVolumes(pScene);

	// for debug purposes only:
	Material mat = mMesh->GetMeshBuffer(0)->GetMaterial();

	// overwrite half transparency
	if (mProps.DebugDataVisible() & EDS_HALF_TRANSPARENCY)
		mat.MaterialType = EMT_TRANSPARENT_ADD_COLOR;
	renderer->SetMaterial(mat);
	
	renderer->DrawMeshBuffer(mMesh->GetMeshBuffer(0));

	// for debug purposes only:
	if (mProps.DebugDataVisible())
	{
		Material m;
		m.Lighting = false;
		m.AntiAliasing=0;
		renderer->SetMaterial(m);

		if (mProps.DebugDataVisible() & EDS_BBOX)
		{
			renderer->Draw3DBox(
				mMesh->GetMeshBuffer(0)->GetBoundingBox(), eastl::array<float, 4>{255.f, 255.f, 255.f, 255.f});
		}
		if (mProps.DebugDataVisible() & EDS_BBOX_BUFFERS)
		{
			renderer->Draw3DBox(
				mMesh->GetMeshBuffer(0)->GetBoundingBox(), eastl::array<float, 4>{255.f, 190.f, 128.f, 128.f});
		}
		if (mProps.DebugDataVisible() & EDS_NORMALS)
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
				renderer->DrawMeshBufferNormals(
					mMesh->GetMeshBuffer(i), debugNormalLength, debugNormalColor);
			}
		}

		// show mesh
		if (mProps.DebugDataVisible() & EDS_MESH_WIRE_OVERLAY)
		{
			m.Wireframe = true;
			renderer->SetMaterial(m);
			renderer->DrawMeshBuffer(mMesh->GetMeshBuffer(0));
		}
	}

	return true;
}

//! returns the axis aligned bounding box of this node
const AlignedBox3<float>& CubeNode::GetBoundingBox() const
{
	return mMesh->GetMeshBuffer(0)->GetBoundingBox();
}


//! Removes a child from this scene node.
//! Implemented here, to be able to remove the shadow properly, if there is one,
//! or to remove attached childs.
bool CubeNode::RemoveChild(ActorId id)
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
eastl::shared_ptr<ShadowVolumeNode> CubeNode::AddShadowVolumeNode(const ActorId actorId,
	Scene* pScene, const eastl::shared_ptr<Mesh>& shadowMesh, bool zfailmethod, float infinity)
{
	const eastl::shared_ptr<Renderer>& renderer = pScene->GetRenderer();

	if (renderer->QueryFeature(EVDF_STENCIL_BUFFER))
		return 0;

	mShadow = eastl::shared_ptr<ShadowVolumeNode>(
		new ShadowVolumeNode(actorId, WeakBaseRenderComponentPtr(), 
		&Matrix4x4<float>::Identity, shadowMesh ? shadowMesh : mMesh, zfailmethod, infinity));
	AddChild(mShadow);

	return mShadow;
}


//! returns the material based on the zero based index i.
Material& CubeNode::GetMaterial(unsigned int i)
{
	return mMesh->GetMeshBuffer(0)->GetMaterial();
}


//! returns amount of materials used by this scene node.
unsigned int CubeNode::GetMaterialCount() const
{
	return 1;
}