// David Eberly, Geometric Tools, Redmond WA 98052
// Copyright (c) 1998-2017
// Distributed under the Boost Software License, Version 1.0.
// http://www.boost.org/LICENSE_1_0.txt
// http://www.geometrictools.com/License/Boost/LICENSE_1_0.txt
// File Version: 3.0.0 (2016/06/19)

#include "BillboardNode.h"

#include "Mathematic/Algebra/Rotation.h"
#include "Mathematic/Function/Functions.h"

#include "Graphic/Renderer/Renderer.h"
#include "Graphic/Effect/Material.h"

#include "Graphic/Scene/Scene.h"

//! constructor
BillboardNode::BillboardNode(const ActorId actorId, PVWUpdater* updater, 
	const eastl::shared_ptr<Texture2>& texture, const Vector2<float>& size)
	: Node(actorId, NT_BILLBOARD)
{
	SetSize(size);

	mPVWUpdater = updater;

	mBlendState = eastl::make_shared<BlendState>();
	mDepthStencilState = eastl::make_shared<DepthStencilState>();
	mRasterizerState = eastl::make_shared<RasterizerState>();

	struct Vertex
	{
		Vector3<float> position;
		Vector2<float> tcoord;
		Vector4<float> color;
	};
	VertexFormat vformat;
	vformat.Bind(VA_POSITION, DF_R32G32B32_FLOAT, 0);
	vformat.Bind(VA_TEXCOORD, DF_R32G32_FLOAT, 0);
	vformat.Bind(VA_COLOR, DF_R32G32B32A32_FLOAT, 0);

	MeshBuffer* meshBuffer = new MeshBuffer(vformat, 4, 2, sizeof(unsigned int));

	// A point light illuminates the target.  Create a semitransparent
	// material for the patch.
	eastl::shared_ptr<Material> material = eastl::make_shared<Material>();
	for (unsigned int i = 0; i < GetMaterialCount(); ++i)
		meshBuffer->GetMaterial() = material;
	mMeshBuffer.reset(meshBuffer);
	SetMaterialTexture(0, texture);

	// fill vertices
	for (unsigned int i = 0; i<mMeshBuffer->GetVertice()->GetNumElements(); i += 4)
	{
		mMeshBuffer->TCoord(0, 0 + i) = Vector2<float>{ 0.0f, 0.0f };
		mMeshBuffer->TCoord(0, 1 + i) = Vector2<float>{ 0.0f, 1.0f };
		mMeshBuffer->TCoord(0, 2 + i) = Vector2<float>{ 1.0f, 1.0f };
		mMeshBuffer->TCoord(0, 3 + i) = Vector2<float>{ 1.0f, 0.0f };
	}

	// fill indices
	int vertices = 0;
	for (unsigned int i = 0; i<mMeshBuffer->GetIndice()->GetNumPrimitives(); i += 2, vertices += 4)
	{
		mMeshBuffer->GetIndice()->SetTriangle(i,
			(unsigned int)0 + vertices, (unsigned int)2 + vertices, (unsigned int)1 + vertices);
		mMeshBuffer->GetIndice()->SetTriangle(i + 1,
			(unsigned int)0 + vertices, (unsigned int)3 + vertices, (unsigned int)2 + vertices);
	}

	eastl::vector<eastl::string> path;
#if defined(_OPENGL_)
	path.push_back("Effects/Texture2ColorEffectVS.glsl");
	path.push_back("Effects/Texture2ColorEffectPS.glsl");
#else
	path.push_back("Effects/Texture2ColorEffectVS.hlsl");
	path.push_back("Effects/Texture2ColorEffectPS.hlsl");
#endif
	eastl::shared_ptr<Texture2Effect> effect = eastl::make_shared<Texture2Effect>(
		ProgramFactory::Get(), path, mMeshBuffer->GetMaterial()->GetTexture(TT_DIFFUSE),
		SamplerState::MIN_L_MAG_L_MIP_P, SamplerState::WRAP, SamplerState::WRAP);
	mEffect = effect;
	mVisual = eastl::make_shared<Visual>(
		mMeshBuffer->GetVertice(), mMeshBuffer->GetIndice(), mEffect);
	mPVWUpdater->Subscribe(mWorldTransform, effect->GetPVWMatrixConstant());
}

//! prerender
bool BillboardNode::PreRender(Scene *pScene)
{
	DoBillboardBuffers(pScene);

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
// BillboardNode::Render
//
bool BillboardNode::Render(Scene *pScene)
{
	const eastl::shared_ptr<CameraNode>& cameraNode = pScene->GetActiveCamera();

	if (!cameraNode || !Renderer::Get())
		return false;

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
	renderer->Update(mMeshBuffer->GetVertice());
	renderer->Draw(mVisual);

	Renderer::Get()->SetDefaultBlendState();
	Renderer::Get()->SetDefaultDepthStencilState();
	Renderer::Get()->SetDefaultRasterizerState();

	return true;
}

void BillboardNode::DoBillboardBuffers(Scene *pScene)
{
	const eastl::shared_ptr<CameraNode>& cameraNode = pScene->GetActiveCamera();

	if (!cameraNode)
		return;

	GetAbsoluteTransform().SetRotation(Matrix4x4<float>::Identity());
	Vector3<float> scale = GetAbsoluteTransform().GetScale();

	float f = 0.5f * mSize[0];
	const Vector4<float> horizontal = cameraNode->Get()->GetRVector() * f;

	f = 0.5f * mSize[1];
	const Vector4<float> vertical = cameraNode->Get()->GetUVector() * f;

	mMeshBuffer->Position(0) = HProject(horizontal + vertical) * scale;
	mMeshBuffer->Color(0, 0) = mMeshBuffer->GetMaterial()->mDiffuse;
	mMeshBuffer->Position(1) = HProject(horizontal - vertical) * scale;
	mMeshBuffer->Color(0, 1) = mMeshBuffer->GetMaterial()->mDiffuse;
	mMeshBuffer->Position(2) = HProject(-horizontal - vertical) * scale;
	mMeshBuffer->Color(0, 2) = mMeshBuffer->GetMaterial()->mDiffuse;
	mMeshBuffer->Position(3) = HProject(-horizontal + vertical) * scale;
	mMeshBuffer->Color(0, 3) = mMeshBuffer->GetMaterial()->mDiffuse;

	mVisual->UpdateModelBound();
}

//! sets the size of the billboard
void BillboardNode::SetSize(const Vector2<float>& size)
{
	mSize = size;
}

//! gets the size of the light node
const Vector2<float>& BillboardNode::GetSize() const
{
	return mSize;
}

//! Returns the visual based on the zero based index i. To get the amount 
//! of visuals used by this scene node, use GetVisualCount(). 
//! This function is needed for inserting the node into the scene hierarchy 
//! at an optimal position for minimizing renderstate changes, but can also 
//! be used to directly modify the visual of a scene node.
eastl::shared_ptr<Visual> const& BillboardNode::GetVisual(unsigned int i)
{
	return mVisual;
}

//! return amount of visuals of this scene node.
unsigned int BillboardNode::GetVisualCount() const
{
	return 1;
}

//! returns the material based on the zero based index i. To get the amount
//! of materials used by this scene node, use GetMaterialCount().
//! This function is needed for inserting the node into the scene hirachy on a
//! optimal position for minimizing renderstate changes, but can also be used
//! to directly modify the material of a scene node.
eastl::shared_ptr<Material> const& BillboardNode::GetMaterial(unsigned int i)
{
	return mMeshBuffer->GetMaterial();
}

//! returns amount of materials used by this scene node.
unsigned int BillboardNode::GetMaterialCount() const
{
	return 1;
}

//! Sets the texture of the specified layer in all materials of this scene node to the new texture.
/** \param textureLayer Layer of texture to be set. Must be a value smaller than MATERIAL_MAX_TEXTURES.
\param texture New texture to be used. */
void BillboardNode::SetMaterialTexture(unsigned int textureLayer, eastl::shared_ptr<Texture2> texture)
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
			eastl::shared_ptr<Texture2Effect> textureEffect =
				eastl::dynamic_shared_pointer_cast<Texture2Effect>(visual->GetEffect());
			if (textureEffect)
				textureEffect->SetTexture(texture);
		}
	}
}

//! Sets the material type of all materials in this scene node to a new material type.
/** \param newType New type of material to be set. */
void BillboardNode::SetMaterialType(MaterialType newType)
{
	for (unsigned int i = 0; i<GetMaterialCount(); ++i)
		GetMaterial(i)->mType = newType;
}