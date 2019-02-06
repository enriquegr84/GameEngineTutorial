// Copyright (C) 2007-2012 Dean Wadsworth
// This file is part of the "Irrlicht Engine".
// For conditions of distribution and use, see copyright notice in irrlicht.h

#include "VolumeLightNode.h"

#include "Graphic/Renderer/Renderer.h"
#include "Graphic/Effect/Material.h"

#include "Core/OS/OS.h"

#include "Graphic/Scene/Scene.h"

//! constructor
VolumeLightNode::VolumeLightNode(const ActorId actorId, PVWUpdater* updater, 
	WeakBaseRenderComponentPtr renderComponent, const Vector2<float>& textureSize, 
	const eastl::shared_ptr<Texture2>& texture, const Vector2<int>& subdivision,
	const eastl::array<float, 4>& foot, const eastl::array<float, 4>& tail)
:	Node(actorId, renderComponent, NT_VOLUME_LIGHT),
	mLPDistance(8.0f), mSubdivideU(subdivision[0]), mSubdivideV(subdivision[1]), 
	mFootColor(foot), mTailColor(tail), mLightDimensions(Vector3<float>{1.0f, 1.2f, 1.0f})
{
	mTexture = texture;
	mPVWUpdater = updater;

	mBlendState = eastl::make_shared<BlendState>();
	mDepthStencilState = eastl::make_shared<DepthStencilState>();
	mRasterizerState = eastl::make_shared<RasterizerState>();

	ConstructLight();
}


VolumeLightNode::~VolumeLightNode()
{

}

void VolumeLightNode::AddToBuffer(Vector3<float>& position,
	eastl::array<float, 4U>& color, Vector2<float>& tcoord, 
	Vertex* vertex, unsigned int& vtx, unsigned int* index, unsigned int& idx)
{

	Vertex vertice;
	vertice.position = position;
	vertice.tcoord = tcoord;
	vertice.color = color;
	index[idx++] = vtx;
	vertex[vtx++] = vertice;
}


void VolumeLightNode::ConstructLight()
{
	VertexFormat vformat;
	vformat.Bind(VA_POSITION, DF_R32G32B32_FLOAT, 0);
	vformat.Bind(VA_TEXCOORD, DF_R32G32_FLOAT, 0);
	vformat.Bind(VA_COLOR, DF_R32G32B32A32_FLOAT, 0);

	mMeshBuffer = eastl::make_shared<MeshBuffer>(vformat,
		6 + 12 * (mSubdivideU + mSubdivideV),
		2 + 4 * (mSubdivideU + mSubdivideV), sizeof(unsigned int));

	eastl::shared_ptr<Material> material = eastl::make_shared<Material>();
	for (unsigned int i = 0; i < GetMaterialCount(); ++i)
		mMeshBuffer->GetMaterial() = material;
	SetMaterialTexture(0, mTexture);

	const Vector3<float> lightPoint{ 0.f, -mLPDistance * mLightDimensions[1], 0.f };
	const float ax = mLightDimensions[0] * 0.5f; // X Axis
	const float az = mLightDimensions[2] * 0.5f; // Z Axis

	unsigned int vtx = 0, idx = 0;
	unsigned int numVertices = mMeshBuffer->GetVertice()->GetNumElements();
	Vertex* vertex = mMeshBuffer->GetVertice()->Get<Vertex>();
	unsigned int* index = mMeshBuffer->GetIndice()->Get<unsigned int>();

	//draw the bottom foot.. the glowing region
	AddToBuffer(Vector3<float>{-ax, 0, az}, mFootColor, Vector2<float>{ 0, 1 }, vertex, vtx, index, idx);
	AddToBuffer(Vector3<float>{ax, 0, az}, mFootColor, Vector2<float>{ 1, 1 }, vertex, vtx, index, idx);
	AddToBuffer(Vector3<float>{ax, 0, -az}, mFootColor, Vector2<float>{ 1, 0 }, vertex, vtx, index, idx);

	AddToBuffer(Vector3<float>{ax, 0, -az}, mFootColor, Vector2<float>{ 1, 0 }, vertex, vtx, index, idx);
	AddToBuffer(Vector3<float>{-ax, 0, -az}, mFootColor, Vector2<float>{ 0, 0 }, vertex, vtx, index, idx);
	AddToBuffer(Vector3<float>{-ax, 0, az}, mFootColor, Vector2<float>{ 0, 1 }, vertex, vtx, index, idx);

	float tu = 0.f;
	const float tuStep = 1.f / mSubdivideU;
	float bx = -ax;
	const float bxStep = mLightDimensions[0] * tuStep;
	// Slices in X/U space
	for (unsigned int i = 0; i < mSubdivideU; ++i)
	{
		// These are the two endpoints for a slice at the foot
		Vector3<float> end1{ bx, 0, -az };
		Vector3<float> end2{ bx, 0, az };

		end1 -= lightPoint;				// get a vector from point to lightsource
		Normalize(end1);				// normalize vector
		end1 *= mLightDimensions[1];	// multiply it out by shootlength

		end1[0] += bx;			// Add the original point location to the vector
		end1[2] -= az;

		// Do it again for the other point.
		end2 -= lightPoint;
		Normalize(end2);
		end2 *= mLightDimensions[1];

		end2[0] += bx;
		end2[2] += az;

		AddToBuffer(Vector3<float>{bx, 0, az}, mFootColor, Vector2<float>{ tu, 1 }, vertex, vtx, index, idx);
		AddToBuffer(Vector3<float>{bx, 0, -az}, mFootColor, Vector2<float>{ tu, 0 }, vertex, vtx, index, idx);
		AddToBuffer(Vector3<float>{end2[0], end2[1], end2[2]}, mTailColor, Vector2<float>{ tu, 1 }, vertex, vtx, index, idx);
		
		AddToBuffer(Vector3<float>{bx, 0, -az}, mFootColor, Vector2<float>{ tu, 0 }, vertex, vtx, index, idx);
		AddToBuffer(Vector3<float>{end1[0], end1[1], end1[2]}, mTailColor, Vector2<float>{ tu, 0 }, vertex, vtx, index, idx);
		AddToBuffer(Vector3<float>{end2[0], end2[1], end2[2]}, mTailColor, Vector2<float>{ tu, 1 }, vertex, vtx, index, idx);

		//back side
		AddToBuffer(Vector3<float>{-end2[0], end2[1], -end2[2]}, mTailColor, Vector2<float>{ tu, 1 }, vertex, vtx, index, idx);
		AddToBuffer(Vector3<float>{-bx, 0, -az}, mFootColor, Vector2<float>{ tu, 1 }, vertex, vtx, index, idx);
		AddToBuffer(Vector3<float>{-bx, 0, az}, mFootColor, Vector2<float>{ tu, 0 }, vertex, vtx, index, idx);

		AddToBuffer(Vector3<float>{-bx, 0, az}, mFootColor, Vector2<float>{ tu, 0 }, vertex, vtx, index, idx);
		AddToBuffer(Vector3<float>{-end1[0], end1[1], -end1[2]}, mTailColor, Vector2<float>{ tu, 0 }, vertex, vtx, index, idx);
		AddToBuffer(Vector3<float>{-end2[0], end2[1], -end2[2]}, mTailColor, Vector2<float>{ tu, 1 }, vertex, vtx, index, idx);

		tu += tuStep;
		bx += bxStep;
	}

	float tv = 0.f;
	const float tvStep = 1.f / mSubdivideV;
	float bz = -az;
	const float bzStep = mLightDimensions[2] * tvStep;
	// Slices in Z/V space
	for (unsigned int i = 0; i < mSubdivideV; ++i)
	{
		// These are the two endpoints for a slice at the foot
		Vector3<float> end1{ -ax, 0, bz };
		Vector3<float> end2{ ax, 0, bz };

		end1 -= lightPoint;				// get a vector from point to lightsource
		Normalize(end1);				// normalize vector
		end1 *= mLightDimensions[1];	// multiply it out by shootlength

		end1[0] -= ax;			// Add the original point location to the vector
		end1[2] += bz;

		// Do it again for the other point.
		end2 -= lightPoint;
		Normalize(end2);
		end2 *= mLightDimensions[1];

		end2[0] += ax;
		end2[2] += bz;

		AddToBuffer(Vector3<float>{-ax, 0, bz}, mFootColor, Vector2<float>{ 0, tv }, vertex, vtx, index, idx);
		AddToBuffer(Vector3<float>{ax, 0, bz}, mFootColor, Vector2<float>{ 1, tv }, vertex, vtx, index, idx);
		AddToBuffer(Vector3<float>{end2[0], end2[1], end2[2]}, mTailColor, Vector2<float>{ 1, tv }, vertex, vtx, index, idx);

		AddToBuffer(Vector3<float>{end2[0], end2[1], end2[2]}, mTailColor, Vector2<float>{ 1, tv }, vertex, vtx, index, idx);
		AddToBuffer(Vector3<float>{end1[0], end1[1], end1[2]}, mTailColor, Vector2<float>{ 0, tv }, vertex, vtx, index, idx);
		AddToBuffer(Vector3<float>{-ax, 0, bz}, mFootColor, Vector2<float>{ 0, tv }, vertex, vtx, index, idx);

		//back side
		AddToBuffer(Vector3<float>{ax, 0, -bz}, mFootColor, Vector2<float>{ 0, tv }, vertex, vtx, index, idx);
		AddToBuffer(Vector3<float>{-ax, 0, -bz}, mFootColor, Vector2<float>{ 1, tv }, vertex, vtx, index, idx);
		AddToBuffer(Vector3<float>{-end2[0], end2[1], -end2[2]}, mTailColor, Vector2<float>{ 1, tv }, vertex, vtx, index, idx);

		AddToBuffer(Vector3<float>{-end2[0], end2[1], -end2[2]}, mTailColor, Vector2<float>{ 1, tv }, vertex, vtx, index, idx);
		AddToBuffer(Vector3<float>{-end1[0], end1[1], -end1[2]}, mTailColor, Vector2<float>{ 0, tv }, vertex, vtx, index, idx);
		AddToBuffer(Vector3<float>{ax, 0, -bz}, mFootColor, Vector2<float>{ 0, tv }, vertex, vtx, index, idx);

		tv += tvStep;
		bz += bzStep;
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

	mVisual->UpdateModelBound();
}


//! prerender
bool VolumeLightNode::PreRender(Scene *pScene)
{
	if (IsVisible())
	{
		// register according to material types counted
		//if (!pScene->IsCulled(this))
			pScene->AddToRenderQueue(RP_TRANSPARENT, shared_from_this());
	}

	return Node::PreRender(pScene);
}

//
// VolumeLightNode::Render
//
bool VolumeLightNode::Render(Scene *pScene)
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


void VolumeLightNode::SetSubDivideU (const unsigned int inU)
{
	if (inU != mSubdivideU)
	{
		mSubdivideU = inU;
		ConstructLight();
	}
}


void VolumeLightNode::SetSubDivideV (const unsigned int inV)
{
	if (inV != mSubdivideV)
	{
		mSubdivideV = inV;
		ConstructLight();
	}
}


void VolumeLightNode::SetFootColor(const eastl::array<float, 4> inColor)
{
	if (inColor != mFootColor)
	{
		mFootColor = inColor;
		ConstructLight();
	}
}


void VolumeLightNode::SetTailColor(const eastl::array<float, 4> inColor)
{
	if (inColor != mTailColor)
	{
		mTailColor = inColor;
		ConstructLight();
	}
}

//! returns the material based on the zero based index i. To get the amount
//! of materials used by this scene node, use GetMaterialCount().
//! This function is needed for inserting the node into the scene hirachy on a
//! optimal position for minimizing renderstate changes, but can also be used
//! to directly modify the material of a scene node.
eastl::shared_ptr<Material> const& VolumeLightNode::GetMaterial(unsigned int i)
{
	return mMeshBuffer->GetMaterial();
}

//! returns amount of materials used by this scene node.
unsigned int VolumeLightNode::GetMaterialCount() const
{
	return 1;
}

//! Sets the texture of the specified layer in all materials of this scene node to the new texture.
/** \param textureLayer Layer of texture to be set. Must be a value smaller than MATERIAL_MAX_TEXTURES.
\param texture New texture to be used. */
void VolumeLightNode::SetMaterialTexture(unsigned int textureLayer, eastl::shared_ptr<Texture2> texture)
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
void VolumeLightNode::SetMaterialType(MaterialType newType)
{
	for (unsigned int i = 0; i<GetMaterialCount(); ++i)
		GetMaterial(i)->mType = newType;
}
