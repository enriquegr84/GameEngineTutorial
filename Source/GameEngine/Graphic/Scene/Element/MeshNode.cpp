// Copyright (C) 2002-2012 Nikolaus Gebhardt
// This file is part of the "Irrlicht Engine".
// For conditions of distribution and use, see copyright notice in irrlicht.h

#include "MeshNode.h"

#include "Graphic/Scene/Hierarchy/Node.h"
#include "Graphic/Scene/Scene.h"

#include "Graphic/Renderer/Renderer.h"
#include "Graphic/Effect/Material.h"

//! constructor
MeshNode::MeshNode(const ActorId actorId, PVWUpdater* updater,
	WeakBaseRenderComponentPtr renderComponent, const eastl::shared_ptr<BaseMesh>& mesh)
:	Node(actorId, renderComponent, NT_MESH)
{
	mPVWUpdater = updater;

	mRasterizerState = eastl::make_shared<RasterizerState>();

	SetMesh(mesh);
}

//! Sets a new mesh
void MeshNode::SetMesh(const eastl::shared_ptr<BaseMesh>& mesh)
{
	if (!mesh)
		return; // won't set null mesh

	mMesh = mesh;
	eastl::vector<eastl::shared_ptr<BaseMeshBuffer>> meshBuffers;
	for (unsigned int i = 0; i < mMesh->GetMeshBufferCount(); ++i)
		meshBuffers.push_back(mMesh->GetMeshBuffer(i));

	mVisuals.clear();
	for (unsigned int i = 0; i < meshBuffers.size(); ++i)
	{
		const eastl::shared_ptr<BaseMeshBuffer>& meshBuffer = meshBuffers[i];
		if (meshBuffer)
		{
			mBlendStates.push_back(eastl::make_shared<BlendState>());
			mDepthStencilStates.push_back(eastl::make_shared<DepthStencilState>());

			eastl::shared_ptr<Texture2> textureDiffuse = meshBuffer->GetMaterial()->GetTexture(TT_DIFFUSE);
			if (textureDiffuse)
			{
				eastl::vector<eastl::string> path;
#if defined(_OPENGL_)
				path.push_back("Effects/Texture2EffectVS.glsl");
				path.push_back("Effects/Texture2EffectPS.glsl");
#else
				path.push_back("Effects/Texture2EffectVS.hlsl");
				path.push_back("Effects/Texture2EffectPS.hlsl");
#endif

				eastl::shared_ptr<Texture2Effect> effect = eastl::make_shared<Texture2Effect>(
					ProgramFactory::Get(), path, textureDiffuse,
					meshBuffer->GetMaterial()->mTextureLayer[TT_DIFFUSE].mFilter,
					meshBuffer->GetMaterial()->mTextureLayer[TT_DIFFUSE].mModeU,
					meshBuffer->GetMaterial()->mTextureLayer[TT_DIFFUSE].mModeV);

				eastl::shared_ptr<Visual> visual = eastl::make_shared<Visual>(
					meshBuffer->GetVertice(), meshBuffer->GetIndice(), effect);
				visual->UpdateModelBound();
				mVisuals.push_back(visual);
				mPVWUpdater->Subscribe(mWorldTransform, effect->GetPVWMatrixConstant());
			}
			else
			{
				eastl::vector<eastl::string> path;
#if defined(_OPENGL_)
				path.push_back("Effects/ConstantColorEffectVS.glsl");
				path.push_back("Effects/ConstantColorEffectPS.glsl");
#else
				path.push_back("Effects/ConstantColorEffectVS.hlsl");
				path.push_back("Effects/ConstantColorEffectPS.hlsl");
#endif
				eastl::shared_ptr<ConstantColorEffect> effect =
					eastl::make_shared<ConstantColorEffect>(ProgramFactory::Get(), path, Vector4<float>::Zero());

				eastl::shared_ptr<Visual> visual = eastl::make_shared<Visual>(
					meshBuffer->GetVertice(), meshBuffer->GetIndice(), effect);
				visual->UpdateModelBound();
				mVisuals.push_back(visual);
				mPVWUpdater->Subscribe(mWorldTransform, effect->GetPVWMatrixConstant());
			}
		}
	}
}

//! prerender
bool MeshNode::PreRender(Scene *pScene)
{
	if (IsVisible())
	{
		// because this node supports rendering of mixed mode meshes consisting of
		// transparent and solid material at the same time, we need to go through all
		// materials, check of what type they are and register this node for the right
		// render pass according to that.
		int transparentCount = 0;
		int solidCount = 0;

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

//! render
bool MeshNode::Render(Scene *pScene)
{
	if (!mMesh || !Renderer::Get())
		return false;

	bool isTransparentPass =
		pScene->GetCurrentRenderPass() == RP_TRANSPARENT;
	++mPassCount;

	if (mShadow && mPassCount == 1)
		mShadow->UpdateShadowVolumes(pScene);

	for (unsigned int i = 0; i < GetVisualCount(); ++i)
	{
		// only render transparent buffer if this is the transparent render pass
		// and solid only in solid pass
		bool transparent = (GetMaterial(i)->IsTransparent());
		if (transparent == isTransparentPass)
		{
			if (GetMaterial(i)->Update(mBlendStates[i]))
				Renderer::Get()->Unbind(mBlendStates[i]);
			if (GetMaterial(i)->Update(mDepthStencilStates[i]))
				Renderer::Get()->Unbind(mDepthStencilStates[i]);
			if (GetMaterial(i)->Update(mRasterizerState))
				Renderer::Get()->Unbind(mRasterizerState);

			Renderer::Get()->SetBlendState(mBlendStates[i]);
			Renderer::Get()->SetDepthStencilState(mDepthStencilStates[i]);
			Renderer::Get()->SetRasterizerState(mRasterizerState);

			Renderer::Get()->Draw(mVisuals[i]);

			Renderer::Get()->SetDefaultBlendState();
			Renderer::Get()->SetDefaultDepthStencilState();
			Renderer::Get()->SetDefaultRasterizerState();
		}
	}

	return true;
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

//! Returns the visual based on the zero based index i. To get the amount 
//! of visuals used by this scene node, use GetVisualCount(). 
//! This function is needed for inserting the node into the scene hierarchy 
//! at an optimal position for minimizing renderstate changes, but can also 
//! be used to directly modify the visual of a scene node.
eastl::shared_ptr<Visual> const& MeshNode::GetVisual(unsigned int i)
{
	if (i >= mVisuals.size())
		return nullptr;

	return mVisuals[i];
}

//! return amount of visuals of this scene node.
unsigned int MeshNode::GetVisualCount() const
{
	return mVisuals.size();
}

//! returns the material based on the zero based index i. To get the amount
//! of materials used by this scene node, use GetMaterialCount().
//! This function is needed for inserting the node into the scene hirachy on a
//! optimal position for minimizing renderstate changes, but can also be used
//! to directly modify the material of a scene node.
eastl::shared_ptr<Material> const& MeshNode::GetMaterial(unsigned int i)
{
	if (i >= mMesh->GetMeshBufferCount())
		return nullptr;

	return mMesh->GetMeshBuffer(i)->GetMaterial();
}

//! returns amount of materials used by this scene node.
unsigned int MeshNode::GetMaterialCount() const
{
	return mMesh->GetMeshBufferCount();
}

//! Sets the texture of the specified layer in all materials of this scene node to the new texture.
/** \param textureLayer Layer of texture to be set. Must be a value smaller than MATERIAL_MAX_TEXTURES.
\param texture New texture to be used. */
void MeshNode::SetMaterialTexture(unsigned int textureLayer, eastl::shared_ptr<Texture2> texture)
{
	if (textureLayer >= MATERIAL_MAX_TEXTURES)
		return;

	for (unsigned int i = 0; i < GetMaterialCount(); ++i)
		GetMaterial(i)->SetTexture(textureLayer, texture);
}

//! Sets the material type of all materials in this scene node to a new material type.
/** \param newType New type of material to be set. */
void MeshNode::SetMaterialType(MaterialType newType)
{
	for (unsigned int i = 0; i < GetMaterialCount(); ++i)
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