// Copyright (C) 2002-2012 Nikolaus Gebhardt
// This file is part of the "Irrlicht Engine".
// For conditions of distribution and use, see copyright notice in irrlicht.h

#include "CubeNode.h"

#include "Graphic/Renderer/Renderer.h"
#include "Graphic/Scene/Scene.h"

//#include "Scenes/Mesh/MeshBuffer.h"

//! constructor
CubeNode::CubeNode(const ActorId actorId, PVWUpdater* updater, WeakBaseRenderComponentPtr renderComponent, 
	const eastl::shared_ptr<Texture2>& texture, float texxScale, float texyScale, float size)
	:	Node(actorId, renderComponent, NT_CUBE), mSize(size), mShadow(0)
{
	mPVWUpdater = updater;

	mBlendState = eastl::make_shared<BlendState>();
	mDepthStencilState = eastl::make_shared<DepthStencilState>();
	mRasterizerState = eastl::make_shared<RasterizerState>();

	struct Vertex
	{
		Vector3<float> position, normal;
		Vector2<float> tcoord;
	};
	VertexFormat vformat;
	vformat.Bind(VA_POSITION, DF_R32G32B32_FLOAT, 0);
	vformat.Bind(VA_NORMAL, DF_R32G32B32_FLOAT, 0);
	vformat.Bind(VA_TEXCOORD, DF_R32G32_FLOAT, 0);

	MeshFactory mf;
	mf.SetVertexFormat(vformat);
	mf.SetVertexBufferUsage(Resource::DYNAMIC_UPDATE);
	mVisual = mf.CreateBox(mSize, mSize, mSize);

	// Multiply the texture coordinates by a factor to enhance the wrap-around.
	eastl::shared_ptr<VertexBuffer> vbuffer = mVisual->GetVertexBuffer();
	unsigned int numVertices = vbuffer->GetNumElements();
	Vertex* vertex = vbuffer->Get<Vertex>();
	for (unsigned int i = 0; i < numVertices; ++i)
	{
		vertex[i].tcoord[0] *= texxScale;
		vertex[i].tcoord[1] *= texyScale;
	}

	// Create the visual effect. The world up-direction is (0,0,1).  Choose
	// the light to point down.
	mMaterial = eastl::make_shared<Material>();
	mMaterial->mEmissive = { 0.0f, 0.0f, 0.0f, 1.0f };
	mMaterial->mAmbient = { 0.5f, 0.5f, 0.5f, 1.0f };
	mMaterial->mDiffuse = { 0.5f, 0.5f, 0.5f, 1.0f };
	mMaterial->mSpecular = { 1.0f, 1.0f, 1.0f, 0.75f };

	eastl::shared_ptr<Lighting> lighting = eastl::make_shared<Lighting>();
	eastl::shared_ptr<LightCameraGeometry> geometry = eastl::make_shared<LightCameraGeometry>();

	eastl::vector<eastl::string> path;
#if defined(_OPENGL_)
	path.push_back("Effects/PointLightTextureEffectVS.glsl");
	path.push_back("Effects/PointLightTextureEffectPS.glsl");
#else
	path.push_back("Effects/PointLightTextureEffectVS.hlsl");
	path.push_back("Effects/PointLightTextureEffectPS.hlsl");
#endif
	mEffect = eastl::make_shared<PointLightTextureEffect>(
		ProgramFactory::Get(), mPVWUpdater->GetUpdater(), path, mMaterial, lighting, geometry,
		texture, SamplerState::MIN_L_MAG_L_MIP_L, SamplerState::WRAP, SamplerState::WRAP);
	mVisual->SetEffect(mEffect);
	mVisual->UpdateModelBound();
	mPVWUpdater->Subscribe(mWorldTransform, mEffect->GetPVWMatrixConstant());
}


CubeNode::~CubeNode()
{

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

//
// CubeSceneNode::Render					- Chapter 16, page 550
//
bool CubeNode::Render(Scene *pScene)
{
	if (!Renderer::Get())
		return false;

	if (mShadow)
		mShadow->UpdateShadowVolumes(pScene);

	for (unsigned int i = 0; i < GetMaterialCount(); ++i)
	{
		if (GetMaterial(i)->Update(mBlendState))
			Renderer::Get()->Unbind(mBlendState);
		if (GetMaterial(i)->Update(mDepthStencilState))
			Renderer::Get()->Unbind(mDepthStencilState);
		if (GetMaterial(i)->Update(mRasterizerState))
			Renderer::Get()->Unbind(mRasterizerState);
	}

	Renderer::Get()->SetBlendState(mBlendState);
	Renderer::Get()->SetDepthStencilState(mDepthStencilState);
	Renderer::Get()->SetRasterizerState(mRasterizerState);

	Renderer* renderer = Renderer::Get();
	renderer->Update(mVisual->GetVertexBuffer());
	renderer->Draw(mVisual);

	Renderer::Get()->SetDefaultBlendState();
	Renderer::Get()->SetDefaultDepthStencilState();
	Renderer::Get()->SetDefaultRasterizerState();

	return true;
}


//! Removes a child from this scene node.
//! Implemented here, to be able to remove the shadow properly, if there is one,
//! or to remove attached childs.
int CubeNode::DetachChild(eastl::shared_ptr<Node> const& child)
{
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

//! Returns the visual based on the zero based index i. To get the amount 
//! of visuals used by this scene node, use GetVisualCount(). 
//! This function is needed for inserting the node into the scene hierarchy 
//! at an optimal position for minimizing renderstate changes, but can also 
//! be used to directly modify the visual of a scene node.
eastl::shared_ptr<Visual> const& CubeNode::GetVisual(unsigned int i)
{
	return mVisual;
}

//! return amount of visuals of this scene node.
unsigned int CubeNode::GetVisualCount() const
{
	return 1;
}

//! returns the material based on the zero based index i. To get the amount
//! of materials used by this scene node, use GetMaterialCount().
//! This function is needed for inserting the node into the scene hirachy on a
//! optimal position for minimizing renderstate changes, but can also be used
//! to directly modify the material of a scene node.
eastl::shared_ptr<Material> const& CubeNode::GetMaterial(unsigned int i)
{
	return mMaterial;
}

//! returns amount of materials used by this scene node.
unsigned int CubeNode::GetMaterialCount() const
{
	return 1;
}

//! Sets the texture of the specified layer in all materials of this scene node to the new texture.
/** \param textureLayer Layer of texture to be set. Must be a value smaller than MATERIAL_MAX_TEXTURES.
\param texture New texture to be used. */
void CubeNode::SetMaterialTexture(unsigned int textureLayer, eastl::shared_ptr<Texture2> texture)
{
	if (textureLayer >= MATERIAL_MAX_TEXTURES)
		return;

	for (unsigned int i = 0; i<GetMaterialCount(); ++i)
		GetMaterial(i)->SetTexture(textureLayer, texture);

	for (unsigned int i = 0; i < GetVisualCount(); ++i)
	{
		eastl::shared_ptr<Visual> visual = GetVisual(i);
		if (visual)
		{
			eastl::shared_ptr<PointLightTextureEffect> textureEffect =
				eastl::dynamic_shared_pointer_cast<PointLightTextureEffect>(visual->GetEffect());
			if (textureEffect)
				textureEffect->SetTexture(texture);
		}
	}
}

//! Sets the material type of all materials in this scene node to a new material type.
/** \param newType New type of material to be set. */
void CubeNode::SetMaterialType(MaterialType newType)
{
	for (unsigned int i = 0; i<GetMaterialCount(); ++i)
		GetMaterial(i)->mType = newType;
}