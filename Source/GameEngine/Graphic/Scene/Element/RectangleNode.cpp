// Copyright (C) 2002-2012 Nikolaus Gebhardt
// This file is part of the "Irrlicht Engine".
// For conditions of distribution and use, see copyright notice in irrlicht.h

#include "RectangleNode.h"

#include "Graphic/Scene/Hierarchy/Node.h"
#include "Graphic/Renderer/Renderer.h"
#include "Graphic/Scene/Scene.h"

//#include "Scenes/Mesh/MeshBuffer.h"


//! constructor
RectangleNode::RectangleNode(const ActorId actorId, PVWUpdater* updater, 
	WeakBaseRenderComponentPtr renderComponent, const eastl::shared_ptr<Texture2>& texture,
	float texxScale, float texyScale, float xSize, float ySize, int xPolyCount, int yPolyCount)
:	Node(actorId, renderComponent, RP_NONE, NT_CUBE), mShadow(0),
	mSizeX(xSize), mSizeY(ySize), mPolyCountX(xPolyCount), mPolyCountY(yPolyCount)
{
	mPVWUpdater = updater;

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
	mVisual = mf.CreateRectangle(mPolyCountX, mPolyCountY, mSizeX, mSizeY);

	// Multiply the texture coordinates by a factor to enhance the wrap-around.
	eastl::shared_ptr<VertexBuffer> vbuffer = mVisual->GetVertexBuffer();
	unsigned int numVertices = vbuffer->GetNumElements();
	Vertex* vertex = vbuffer->Get<Vertex>();
	for (unsigned int i = 0; i < numVertices; ++i)
	{
		vertex[i].tcoord[0] *= texxScale;
		vertex[i].tcoord[1] *= texyScale;
	}

	// Create the visual effect.  The world up-direction is (0,0,1).  Choose
	// the light to point down.
	mMaterial = eastl::make_shared<Material>();
	mMaterial->mEmissive = { 0.0f, 0.0f, 0.0f, 1.0f };
	mMaterial->mAmbient = { 0.5f, 0.5f, 0.5f, 1.0f };
	mMaterial->mDiffuse = { 0.5f, 0.5f, 0.5f, 1.0f };
	mMaterial->mSpecular = { 1.0f, 1.0f, 1.0f, 75.0f };

	eastl::shared_ptr<Lighting> lighting = eastl::make_shared<Lighting>();
	lighting->mAmbient = Renderer::Get()->GetClearColor();
	lighting->mAttenuation = { 1.0f, 0.0f, 0.0f, 1.0f };

	eastl::shared_ptr<LightCameraGeometry> geometry = eastl::make_shared<LightCameraGeometry>();

	eastl::string path = FileSystem::Get()->GetPath("Effects/PointLightTextureEffect.hlsl");
	eastl::shared_ptr<PointLightTextureEffect> effect = eastl::make_shared<PointLightTextureEffect>(
		ProgramFactory::Get(), mPVWUpdater->GetUpdater(), path, mMaterial, lighting, geometry,
		texture, SamplerState::MIN_L_MAG_L_MIP_L, SamplerState::WRAP, SamplerState::WRAP);
	mVisual->SetEffect(effect);
	mVisual->UpdateModelNormals();
	mPVWUpdater->Subscribe(mWorldTransform, effect->GetPVWMatrixConstant());
}


RectangleNode::~RectangleNode()
{

}

//! prerender
bool RectangleNode::PreRender(Scene *pScene)
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
// RectangleNode::Render					- Chapter 16, page 550
//
bool RectangleNode::Render(Scene *pScene)
{
	if (!Renderer::Get())
		return false;

	if (mShadow)
		mShadow->UpdateShadowVolumes(pScene);

	// overwrite half transparency
	if (DebugDataVisible() & DS_HALF_TRANSPARENCY)
		mMaterial->mType = MT_TRANSPARENT_ADD_COLOR;

	Renderer::Get()->SetBlendState(mMaterial->mBlendState);
	Renderer::Get()->SetRasterizerState(mMaterial->mRasterizerState);
	Renderer::Get()->SetDepthStencilState(mMaterial->mDepthStencilState);

	eastl::shared_ptr<AmbientLightEffect> effect =
		eastl::static_pointer_cast<AmbientLightEffect>(mVisual->GetEffect());
	effect->SetMaterial(mMaterial);
	Renderer::Get()->Draw(mVisual);

	Renderer::Get()->SetDefaultDepthStencilState();
	Renderer::Get()->SetDefaultRasterizerState();
	Renderer::Get()->SetDefaultBlendState();

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
int RectangleNode::DetachChild(eastl::shared_ptr<Node> const& child)
{
	if (child && mShadow == child)
		mShadow = 0;

	if (Node::DetachChild(child))
		return true;

	return false;
}


//! Creates shadow volume scene node as child of this node
//! and returns a pointer to it.
eastl::shared_ptr<ShadowVolumeNode> RectangleNode::AddShadowVolumeNode(const ActorId actorId,
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
//! of materials used by this scene node, use GetMaterialCount().
//! This function is needed for inserting the node into the scene hirachy on a
//! optimal position for minimizing renderstate changes, but can also be used
//! to directly modify the material of a scene node.
eastl::shared_ptr<Material> const& RectangleNode::GetMaterial(unsigned int i)
{
	return mMaterial;
}

//! returns amount of materials used by this scene node.
unsigned int RectangleNode::GetMaterialCount() const
{
	return 1;
}

//! Sets the texture of the specified layer in all materials of this scene node to the new texture.
/** \param textureLayer Layer of texture to be set. Must be a value smaller than MATERIAL_MAX_TEXTURES.
\param texture New texture to be used. */
void RectangleNode::SetMaterialTexture(unsigned int textureLayer, Texture2* texture)
{
	if (textureLayer >= MATERIAL_MAX_TEXTURES)
		return;

	for (unsigned int i = 0; i<GetMaterialCount(); ++i)
		GetMaterial(i)->SetTexture(textureLayer, texture);
}

//! Sets the material type of all materials in this scene node to a new material type.
/** \param newType New type of material to be set. */
void RectangleNode::SetMaterialType(MaterialType newType)
{
	for (unsigned int i = 0; i<GetMaterialCount(); ++i)
		GetMaterial(i)->mType = newType;
}