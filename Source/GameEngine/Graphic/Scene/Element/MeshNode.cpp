// Copyright (C) 2002-2012 Nikolaus Gebhardt
// This file is part of the "Irrlicht Engine".
// For conditions of distribution and use, see copyright notice in irrlicht.h

#include "MeshNode.h"
#include "CameraNode.h"

#include "Graphic/Renderer/Renderer.h"
#include "Graphic/Effect/Material.h"

#include "Graphic/Scene/Scene.h"

/*
#include "Graphic/Scene/Mesh/Mesh.h"
#include "Graphic/Scene/Mesh/MeshCache.h"
#include "Graphic/Scene/Mesh/AnimatedMesh.h"
*/


//! constructor
MeshNode::MeshNode(const ActorId actorId, WeakBaseRenderComponentPtr renderComponent,
	const eastl::shared_ptr<Mesh>& mesh)
:	Node(actorId, renderComponent, ERP_NONE, ESNT_MESH), 
	mMesh(0), mShadow(0), mPassCount(0), mReadOnlyMaterials(false)
{
	#ifdef _DEBUG
	//setDebugName("CMeshSceneNode");
	#endif

	SetMesh(mesh);
}


//! destructor
MeshNode::~MeshNode()
{

}


//! frame
bool MeshNode::PreRender(Scene *pScene)
{
	if (mProps.IsVisible())
	{
		// because this node supports rendering of mixed mode meshes consisting of
		// transparent and solid material at the same time, we need to go through all
		// materials, check of what type they are and register this node for the right
		// render pass according to that.
		const eastl::shared_ptr<Renderer>& renderer = pScene->GetRenderer();

		mPassCount = 0;
		int transparentCount = 0;
		int solidCount = 0;

		// count transparent and solid materials in this scene node
		if (mReadOnlyMaterials && mMesh)
		{
			// count mesh materials
			for (unsigned int i=0; i<mMesh->GetMeshBufferCount(); ++i)
			{
				const eastl::shared_ptr<IMeshBuffer>& mb = mMesh->GetMeshBuffer(i);
				const eastl::shared_ptr<IMaterialRenderer>& rnd =
					mb ? renderer->GetMaterialRenderer(mb->GetMaterial().MaterialType) : 0;
				if (rnd && rnd->IsTransparent())
					++transparentCount;
				else
					++solidCount;

				if (solidCount && transparentCount)
					break;
			}
		}
		else
		{
			// count copied materials
			for (unsigned int i=0; i<mMaterials.size(); ++i)
			{
				const eastl::shared_ptr<MaterialRenderer>& rnd =
					renderer->GetMaterialRenderer(mMaterials[i].MaterialType);
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
				pScene->AddToRenderQueue(ERP_SOLID, eastl::shared_from_this());

			if (transparentCount)
				pScene->AddToRenderQueue(ERP_TRANSPARENT, eastl::shared_from_this());
		}

	}

	return Node::PreRender(pScene);
}

//! renders the node.
bool MeshNode::Render(Scene *pScene)
{
	const eastl::shared_ptr<Renderer>& renderer = pScene->GetRenderer();

	if (!mMesh || !renderer)
		return false;

	Matrix4x4 toWorld, fromWorld;
	Get()->Transform(&toWorld, &fromWorld);

	bool isTransparentPass = Get()->GetRenderPass() && ERP_TRANSPARENT;

	++mPassCount;

	renderer->SetTransform(ETS_WORLD, toWorld);
	mBBox = mMesh->GetBoundingBox();

	if (mShadow && mPassCount==1)
		mShadow->UpdateShadowVolumes(pScene);

	// for debug purposes only:

	bool renderMeshes = true;
	Material mat;
	if (mProps.DebugDataVisible() && mPassCount==1)
	{
		// overwrite half transparency
		if (mProps.DebugDataVisible() & EDS_HALF_TRANSPARENCY)
		{
			for (unsigned int g=0; g< mMesh->GetMeshBufferCount(); ++g)
			{
				mat = mMaterials[g];
				mat.MaterialType = EMT_TRANSPARENT_ADD_COLOR;
				renderer->SetMaterial(mat);
				renderer->DrawMeshBuffer(mMesh->GetMeshBuffer(g));
			}
			renderMeshes = false;
		}
	}

	// render original meshes
	if (renderMeshes)
	{
		for (unsigned int i=0; i<mMesh->GetMeshBufferCount(); ++i)
		{
			const eastl::shared_ptr<MeshBuffer>& mb = mMesh->GetMeshBuffer(i);
			if (mb)
			{
				const Material& material = 
					mReadOnlyMaterials ? mb->GetMaterial() : mMaterials[i];

				const eastl::shared_ptr<MaterialRenderer>& rnd =
					renderer->GetMaterialRenderer(material.MaterialType);
				bool transparent = (rnd && rnd->IsTransparent());

				// only render transparent buffer if this is the transparent render pass
				// and solid only in solid pass
				if (transparent == isTransparentPass)
				{
					renderer->SetMaterial(material);
					renderer->DrawMeshBuffer(mb);
				}
			}
		}
	}

	renderer->SetTransform(ETS_WORLD, toWorld);

	// for debug purposes only:
	if (mProps.DebugDataVisible() && mPassCount==1)
	{
		Material m;
		m.Lighting = false;
		m.AntiAliasing=0;
		renderer->SetMaterial(m);

		if (mProps.DebugDataVisible() & EDS_BBOX)
		{
			renderer->Draw3DBox(mBBox, Color(255,255,255,255));
		}
		if (mProps.DebugDataVisible() & EDS_BBOX_BUFFERS)
		{
			for (unsigned int g=0; g<mMesh->GetMeshBufferCount(); ++g)
			{
				renderer->Draw3DBox(
					mMesh->GetMeshBuffer(g)->GetBoundingBox(),
					eastl::array<float, 4>{255.f, 190.f, 128.f, 128.f});
			}
		}

		if (mProps.DebugDataVisible() & EDS_NORMALS)
		{
			// draw normals
			//const float debugNormalLength = pScene->GetParameters()->GetAttributeAsFloat(DEBUG_NORMAL_LENGTH);
			//const eastl::array<float, 4> debugNormalColor = pScene->GetParameters()->GetAttributeAColor(DEBUG_NORMAL_COLOR);

			// draw normals
			const float debugNormalLength = 1.f;
			const eastl::array<float, 4> debugNormalColor{ 255.f, 34.f, 221.f, 221.f };
			const unsigned int count = mMesh->GetMeshBufferCount();

			for (unsigned int i=0; i != count; ++i)
				renderer->DrawMeshBufferNormals(mMesh->GetMeshBuffer(i), debugNormalLength, debugNormalColor);
		}

		// show mesh
		if (mProps.DebugDataVisible() & EDS_MESH_WIRE_OVERLAY)
		{
			m.Wireframe = true;
			renderer->SetMaterial(m);

			for (unsigned int g=0; g<mMesh->GetMeshBufferCount(); ++g)
			{
				renderer->DrawMeshBuffer(mMesh->GetMeshBuffer(g));
			}
		}
	}
	return true;
}


//! Removes a child from this scene node.
//! Implemented here, to be able to remove the shadow properly, if there is one,
//! or to remove attached childs.
bool MeshNode::RemoveChild(ActorId id)
{
	for(SceneNodeList::iterator i=mChildren.begin(); i!=mChildren.end(); ++i)
	{
		if((mProps.ActorId() != INVALID_ACTOR_ID) && (id == mProps.ActorId()))
			if ((*i) && mShadow == (*i))
				mShadow = 0;
	}

	return Node::RemoveChild(id);
}


//! returns the axis aligned bounding box of this node
const AABBox3<float>& MeshNode::GetBoundingBox() const
{
	return mMesh ? mMesh->GetBoundingBox() : mBBox;
}


//! returns the material based on the zero based index i. To get the amount
//! of materials used by this scene node, use GetMaterialCount().
//! This function is needed for inserting the node into the scene hierarchy on a
//! optimal position for minimizing renderstate changes, but can also be used
//! to directly modify the material of a scene node.
Material& MeshNode::GetMaterial(unsigned int i)
{
	if (mMesh && mReadOnlyMaterials && i<mMesh->GetMeshBufferCount())
	{
		mReadOnlyMaterial = mMesh->GetMeshBuffer(i)->GetMaterial();
		return mReadOnlyMaterial;
	}

	if (i >= mMaterials.size())
		return Matrix4x4::Identity;

	return mMaterials[i];
}


//! returns amount of materials used by this scene node.
unsigned int MeshNode::GetMaterialCount() const
{
	if (mMesh && mReadOnlyMaterials)
		return mMesh->GetMeshBufferCount();

	return mMaterials.size();
}


//! Sets a new mesh
void MeshNode::SetMesh(const eastl::shared_ptr<Mesh>& mesh)
{
	if (mesh)
	{
		mMesh = mesh;
		CopyMaterials();
	}
}


//! Creates shadow volume scene node as child of this node
//! and returns a pointer to it.
eastl::shared_ptr<ShadowVolumeNode> MeshNode::AddShadowVolumeNode(const ActorId actorId,
	Scene* pScene, const eastl::shared_ptr<Mesh>& shadowMesh, bool zfailmethod, float infinity)
{
	const eastl::shared_ptr<Renderer>& renderer = pScene->GetRenderer();

	if (renderer->QueryFeature(EVDF_STENCIL_BUFFER))
		return 0;

	mShadow = eastl::shared_ptr<ShadowVolumeNode>(
		new ShadowVolumeNode(actorId, WeakBaseRenderComponentPtr(), 
		&Matrix4x4::Identity, shadowMesh ? shadowMesh : mMesh, zfailmethod, infinity));
	AddChild(mShadow);

	return mShadow;
}


void MeshNode::CopyMaterials()
{
	mMaterials.clear();

	if (mMesh)
	{
		Material mat;

		for (unsigned int i=0; i<mMesh->GetMeshBufferCount(); ++i)
		{
			const eastl::shared_ptr<MeshBuffer>& mb = mMesh->GetMeshBuffer(i);
			if (mb)
				mat = mb->GetMaterial();

			mMaterials.push_back(mat);
		}
	}
}

//! Sets if the scene node should not copy the materials of the mesh but use them in a read only style.
/* In this way it is possible to change the materials a mesh causing all mesh scene nodes
referencing this mesh to change too. */
void MeshNode::SetReadOnlyMaterials(bool readonly)
{
	mReadOnlyMaterials = readonly;
}


//! Returns if the scene node should not copy the materials of the mesh but use them in a read only style
bool MeshNode::IsReadOnlyMaterials() const
{
	return mReadOnlyMaterials;
}


