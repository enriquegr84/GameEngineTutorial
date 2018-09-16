// Copyright (C) 2002-2012 Nikolaus Gebhardt
// This file is part of the "Irrlicht Engine".
// For conditions of distribution and use, see copyright notice in irrlicht.h

#include "MeshNode.h"

#include "Graphic/Renderer/Renderer.h"
#include "Graphic/Effect/Material.h"

#include "Graphic/Scene/Scene.h"


//! constructor
MeshNode::MeshNode(const ActorId actorId, PVWUpdater* updater, 
	WeakBaseRenderComponentPtr renderComponent, const eastl::shared_ptr<BaseMesh>& mesh)
:	Node(actorId, renderComponent, RP_NONE, NT_MESH), mMesh(0), mShadow(0), mPassCount(0)
{
	mPVWUpdater = updater;
	SetMesh(mesh);
}


//! destructor
MeshNode::~MeshNode()
{

}


//! Sets a new mesh
void MeshNode::SetMesh(const eastl::shared_ptr<BaseMesh>& mesh)
{
	if (!mesh)
		return; // won't set null mesh

	mMesh = mesh;
	MeshFactory mf;

	mVisuals.clear();
	for (unsigned int i = 0; i<mMesh->GetMeshBufferCount(); ++i)
	{
		const eastl::shared_ptr<MeshBuffer>& meshBuffer = mMesh->GetMeshBuffer(i);
		if (meshBuffer)
		{
			// Create the visual effect.  The world up-direction is (0,0,1).  Choose
			// the light to point down.
			eastl::shared_ptr<Lighting> lighting = eastl::make_shared<Lighting>();
			lighting->mAmbient = Renderer::Get()->GetClearColor();
			lighting->mAttenuation = { 1.0f, 0.0f, 0.0f, 1.0f };

			eastl::shared_ptr<LightCameraGeometry> geometry = eastl::make_shared<LightCameraGeometry>();

			eastl::string path = FileSystem::Get()->GetPath("Effects/PointLightTextureEffect.hlsl");
			eastl::shared_ptr<PointLightTextureEffect> effect = eastl::make_shared<PointLightTextureEffect>(
				ProgramFactory::Get(), mPVWUpdater->GetUpdater(), path, meshBuffer->GetMaterial(), lighting,
				geometry, eastl::make_shared<Texture2>(DF_UNKNOWN, 0, 0, true), 
				SamplerState::MIN_L_MAG_L_MIP_L, SamplerState::WRAP, SamplerState::WRAP);

			eastl::shared_ptr<Visual> visual = eastl::make_shared<Visual>(
				meshBuffer->GetVertice(), meshBuffer->GetIndice(), effect);
			visual->UpdateModelNormals();
			visual->UpdateModelBound();
			mVisuals.push_back(visual);
			mPVWUpdater->Subscribe(mWorldTransform, effect->GetPVWMatrixConstant());
		}
	}
}


//! frame
bool MeshNode::PreRender(Scene *pScene)
{
	if (IsVisible())
	{
		// because this node supports rendering of mixed mode meshes consisting of
		// transparent and solid material at the same time, we need to go through all
		// materials, check of what type they are and register this node for the right
		// render pass according to that.

		mPassCount = 0;
		int transparentCount = 0;
		int solidCount = 0;

		// count transparent and solid materials in this scene node
		for (unsigned int i = 0; i < GetMaterialCount(); ++i)
		{
			if (GetMaterial(i)->IsTransparent())
				++transparentCount;
			else
				++solidCount;

			if (solidCount && transparentCount)
				break;
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
bool MeshNode::Render(Scene *pScene)
{
	if (!mMesh || !Renderer::Get())
		return false;

	Matrix4x4<float> toWorld, fromWorld;
	//Transform(&toWorld, &fromWorld);

	bool isTransparentPass = GetRenderPass() && RP_TRANSPARENT;

	++mPassCount;

	//Renderer::Get()->SetTransform(TS_WORLD, toWorld);
	//mBBox = mMesh->GetBoundingBox();

	if (mShadow && mPassCount==1)
		mShadow->UpdateShadowVolumes(pScene);

	// for debug purposes only:

	bool renderMeshes = true;
	if (DebugDataVisible() && mPassCount==1)
	{
		// overwrite half transparency
		if (DebugDataVisible() & DS_HALF_TRANSPARENCY)
		{
			for (unsigned int i = 0; i<mMesh->GetMeshBufferCount(); ++i)
			{
				eastl::shared_ptr<PointLightTextureEffect> effect =
					eastl::static_pointer_cast<PointLightTextureEffect>(mVisuals[i]->GetEffect());

				const eastl::shared_ptr<MeshBuffer>& mb = mMesh->GetMeshBuffer(i);
				eastl::shared_ptr<Material> material =
					mReadOnlyMaterials ? mb->GetMaterial() : effect->GetMaterial();
				material->mType = MT_TRANSPARENT_ADD_COLOR;

				//effect->SetMaterial(material);
				Renderer::Get()->Draw(mVisuals[i]);
			}
			renderMeshes = false;
		}
	}

	// render original meshes
	if (renderMeshes)
	{
		for (unsigned int i = 0; i<mMesh->GetMeshBufferCount(); ++i)
		{
			eastl::shared_ptr<PointLightTextureEffect> effect =
				eastl::static_pointer_cast<PointLightTextureEffect>(mVisuals[i]->GetEffect());
			bool transparent = (effect->GetMaterial()->IsTransparent());

			// only render transparent buffer if this is the transparent render pass
			// and solid only in solid pass
			if (transparent == isTransparentPass)
			{
				const eastl::shared_ptr<MeshBuffer>& mb = mMesh->GetMeshBuffer(i);
				eastl::shared_ptr<Material> material =
					mReadOnlyMaterials ? mb->GetMaterial() : effect->GetMaterial();

				effect->SetMaterial(material);
				Renderer::Get()->Draw(mVisuals[i]);
			}
		}
	}

	//Renderer::Get()->SetTransform(TS_WORLD, toWorld);
	/*
	// for debug purposes only:
	if (DebugDataVisible() && mPassCount==1)
	{
		Material m;
		m.mLighting = false;
		m.mAntiAliasing=0;
		Renderer::Get()->SetMaterial(m);

		if (DebugDataVisible() & DS_BBOX)
		{
			Renderer::Get()->Draw3DBox(mBBox, Vector4<float>{1.f,1.f,1.f,1.f});
		}
		if (DebugDataVisible() & DS_BBOX_BUFFERS)
		{
			for (unsigned int g=0; g<mMesh->GetMeshBufferCount(); ++g)
			{
				Renderer::Get()->Draw3DBox(
					mMesh->GetMeshBuffer(g)->GetBoundingBox(),
					eastl::array<float, 4>{255.f, 190.f, 128.f, 128.f});
			}
		}

		if (DebugDataVisible() & DS_NORMALS)
		{
			// draw normals
			//const float debugNormalLength = GetParameters()->GetAttributeAsFloat(DEBUG_NORMAL_LENGTH);
			//const eastl::array<float, 4> debugNormalColor = GetParameters()->GetAttributeAColor(DEBUG_NORMAL_COLOR);

			// draw normals
			const float debugNormalLength = 1.f;
			const eastl::array<float, 4> debugNormalColor{ 255.f, 34.f, 221.f, 221.f };
			const unsigned int count = mMesh->GetMeshBufferCount();

			for (unsigned int i=0; i != count; ++i)
				Renderer::Get()->DrawMeshBufferNormals(mMesh->GetMeshBuffer(i), debugNormalLength, debugNormalColor);
		}

		// show mesh
		if (DebugDataVisible() & DS_MESH_WIRE_OVERLAY)
		{
			m.mWireframe = true;
			Renderer::Get()->SetMaterial(m);

			for (unsigned int g=0; g<mMesh->GetMeshBufferCount(); ++g)
			{
				Renderer::Get()->DrawMeshBuffer(mMesh->GetMeshBuffer(g));
			}
		}
	}
	*/
	return true;
}


//! Removes a child from this scene node.
//! Implemented here, to be able to remove the shadow properly, if there is one,
//! or to remove attached childs.
int MeshNode::DetachChild(eastl::shared_ptr<Node> const& child)
{
	if (child && mShadow == child)
		mShadow = 0;

	if (Node::DetachChild(child))
		return true;

	return false;
}


//! Creates shadow volume scene node as child of this node
//! and returns a pointer to it.
eastl::shared_ptr<ShadowVolumeNode> MeshNode::AddShadowVolumeNode(const ActorId actorId,
	Scene* pScene, const eastl::shared_ptr<BaseMesh>& shadowMesh, bool zfailmethod, float infinity)
{
	/*
	if (!Renderer::Get()->QueryFeature(VDF_STENCIL_BUFFER))
		return nullptr;
	*/
	mShadow = eastl::shared_ptr<ShadowVolumeNode>(
		new ShadowVolumeNode(actorId, mPVWUpdater, WeakBaseRenderComponentPtr(),
			shadowMesh ? shadowMesh : mMesh, zfailmethod, infinity));
	shared_from_this()->AttachChild(mShadow);

	return mShadow;
}

//! returns the material based on the zero based index i. To get the amount
//! of materials used by this scene node, use GetMaterialCount().
//! This function is needed for inserting the node into the scene hirachy on a
//! optimal position for minimizing renderstate changes, but can also be used
//! to directly modify the material of a scene node.
eastl::shared_ptr<Material> const& AnimatedMeshNode::GetMaterial(unsigned int i)
{
	if (i >= mMesh->GetMeshBufferCount())
		return nullptr;

	return mMesh->GetMeshBuffer(i)->GetMaterial();
}

//! returns amount of materials used by this scene node.
unsigned int AnimatedMeshNode::GetMaterialCount() const
{
	return mMesh->GetMeshBufferCount();
}

//! Sets all material flags at once to a new value.
/** Useful, for example, if you want the whole mesh to be affected by light.
\param flag Which flag of all materials to be set.
\param newvalue New value of that flag. */
void AnimatedMeshNode::SetMaterialFlag(MaterialFlag flag, bool newvalue)
{
	for (unsigned int i = 0; i<GetMaterialCount(); ++i)
		GetMaterial(i).SetFlag(flag, newvalue);
}

//! Sets the texture of the specified layer in all materials of this scene node to the new texture.
/** \param textureLayer Layer of texture to be set. Must be a value smaller than MATERIAL_MAX_TEXTURES.
\param texture New texture to be used. */
void AnimatedMeshNode::SetMaterialTexture(unsigned int textureLayer, Texture2* texture)
{
	if (textureLayer >= MATERIAL_MAX_TEXTURES)
		return;

	for (unsigned int i = 0; i<GetMaterialCount(); ++i)
		GetMaterial(i).SetTexture(textureLayer, texture);
}

//! Sets the material type of all materials in this scene node to a new material type.
/** \param newType New type of material to be set. */
void AnimatedMeshNode::SetMaterialType(MaterialType newType)
{
	for (unsigned int i = 0; i<GetMaterialCount(); ++i)
		GetMaterial(i)->mType = newType;
}

//! Sets if the scene node should not copy the materials of the mesh but use them in a read only style.
void MeshNode::SetReadOnlyMaterials(bool readonly)
{
	mReadOnlyMaterials = readonly;
}


//! Returns if the scene node should not copy the materials of the mesh but use them in a read only style
bool MeshNode::IsReadOnlyMaterials() const
{
	return mReadOnlyMaterials;
}