// Copyright (C) 2002-2012 Nikolaus Gebhardt
// This file is part of the "Irrlicht Engine".
// For conditions of distribution and use, see copyright notice in irrlicht.h

#include "SkyDomeNode.h"
#include "CameraNode.h"

#include "Core/OS/OS.h"

#include "Graphic/Renderer/Renderer.h"
#include "Graphic/Effect/Material.h"

#include "Graphic/Scene/Scene.h"

//! constructor
SkyDomeNode::SkyDomeNode(const ActorId actorId, PVWUpdater* updater, WeakBaseRenderComponentPtr renderComponent,
	const eastl::shared_ptr<Texture2>& sky, unsigned int horiRes, unsigned int vertRes,
	float texturePercentage, float spherePercentage, float radius)
:	Node(actorId, renderComponent, RP_SKY, NT_SKY), mHorizontalResolution(horiRes), mVerticalResolution(vertRes), 
	mTexturePercentage(texturePercentage), mSpherePercentage(spherePercentage), mRadius(radius)
{
	mPVWUpdater = updater;

	VertexFormat vformat;
	vformat.Bind(VA_POSITION, DF_R32G32B32_FLOAT, 0);
	vformat.Bind(VA_NORMAL, DF_R32G32B32_FLOAT, 0);
	vformat.Bind(VA_TEXCOORD, DF_R32G32_FLOAT, 0);

	//SetAutomaticCulling(AC_OFF);
	mMeshBuffer = eastl::make_shared<MeshBuffer>(vformat, 
		(mHorizontalResolution + 1) * (mVerticalResolution + 1), 
		(2 * mVerticalResolution - 1) * mHorizontalResolution, sizeof(unsigned int));
	// Create the visual effect. The world up-direction is (0,0,1).  Choose
	// the light to point down.
	mMeshBuffer->GetMaterial()->mEmissive = { 0.0f, 0.0f, 0.0f, 1.0f };
	mMeshBuffer->GetMaterial()->mAmbient = { 0.5f, 0.5f, 0.5f, 1.0f };
	mMeshBuffer->GetMaterial()->mDiffuse = { 0.5f, 0.5f, 0.5f, 1.0f };
	mMeshBuffer->GetMaterial()->mSpecular = { 1.0f, 1.0f, 1.0f, 75.0f };

	mMeshBuffer->GetMaterial()->mLighting = false;
	mMeshBuffer->GetMaterial()->mDepthStencilState->mDepthEnable = false;
	mMeshBuffer->GetMaterial()->mRasterizerState->mEnableDepthClip = false;
	mMeshBuffer->GetMaterial()->mRasterizerState->mEnableAntialiasedLine = false;
	mMeshBuffer->GetMaterial()->SetTexture(0, sky);

	eastl::string path = FileSystem::Get()->GetPath("Effects/Texture2Effect.hlsl");
	eastl::shared_ptr<Texture2Effect> effect = eastl::make_shared<Texture2Effect>(
		ProgramFactory::Get(), path, mMeshBuffer->GetMaterial()->GetTexture(0),
		mMeshBuffer->GetMaterial()->mTextureLayer[0].mSamplerState->mFilter,
		mMeshBuffer->GetMaterial()->mTextureLayer[0].mSamplerState->mMode[0],
		mMeshBuffer->GetMaterial()->mTextureLayer[0].mSamplerState->mMode[1]);

	mVisual = eastl::make_shared<Visual>(mMeshBuffer->GetVertice(), mMeshBuffer->GetIndice(), effect);
	mVisual->SetEffect(effect);
	mVisual->UpdateModelNormals();
	mVisual->UpdateModelBound();
	mPVWUpdater->Subscribe(mWorldTransform, effect->GetPVWMatrixConstant());

	// regenerate the mesh
	GenerateMesh(sky);
}

void SkyDomeNode::GenerateMesh(const eastl::shared_ptr<Texture2>& sky)
{
	float azimuth = 0.f;
	const float azimuthStep = ((float)GE_C_PI * 2.f) / mHorizontalResolution;
	if (mSpherePercentage < 0.f)
		mSpherePercentage = mSpherePercentage;
	if (mSpherePercentage > 2.f)
		mSpherePercentage = 2.f;
	const float elevationStep = mSpherePercentage * (float)GE_C_HALF_PI / (float)mVerticalResolution;

	const float onepixel = sky ? (1.0f / (sky->GetDimension(0) * 1.5f)) : 0.0f;
	const float t = 1.0f - onepixel;
	const float o = 0.0f + onepixel;

	struct Vertex
	{
		Vector3<float> position;
		Vector3<float> normal;
		Vector2<float> tcoord;
	};

	unsigned int numVertices = mMeshBuffer->GetVertice()->GetNumElements();
	Vertex* vertex = mMeshBuffer->GetVertice()->Get<Vertex>();

	const float tcV = mTexturePercentage / mVerticalResolution;
	int vtx = 0;
	for (unsigned int k = 0; k <= mHorizontalResolution; ++k)
	{
		float elevation = (float)GE_C_HALF_PI;
		const float tcU = (float)k / (float)mHorizontalResolution;
		const float sinA = Function<float>::Sin(azimuth);
		const float cosA = Function<float>::Cos(azimuth);
		for (unsigned int j = 0; j <= mVerticalResolution; ++j)
		{
			Vertex vertice;

			const float cosEr = mRadius * Function<float>::Cos(elevation);
			vertice.position = Vector3<float>{ cosEr*sinA, cosEr*cosA, mRadius*Function<float>::Sin(elevation) };
			vertice.tcoord = Vector2<float>{ tcU, j*tcV };
			vertice.normal = -vertice.position;
			Normalize(vertice.normal);

			vertex[vtx++] = vertice;
			elevation -= elevationStep;
		}
		azimuth += azimuthStep;
	}

	unsigned int numIndices = mMeshBuffer->GetIndice()->GetNumElements();
	unsigned int* indices = mMeshBuffer->GetIndice()->Get<unsigned int>();

	int idx = 0;
	for (unsigned int k = 0; k < mHorizontalResolution; ++k)
	{
		indices[idx++] = mVerticalResolution + 2 + (mVerticalResolution + 1) * k;
		indices[idx++] = 1 + (mVerticalResolution + 1) * k;
		indices[idx++] = 0 + (mVerticalResolution + 1) * k;

		for (unsigned int j = 1; j < mVerticalResolution; ++j)
		{
			indices[idx++] = mVerticalResolution + 2 + (mVerticalResolution + 1) * k + j;
			indices[idx++] = 1 + (mVerticalResolution + 1) * k + j;
			indices[idx++] = 0 + (mVerticalResolution + 1) * k + j;

			indices[idx++] = mVerticalResolution + 1 + (mVerticalResolution + 1) * k + j;
			indices[idx++] = mVerticalResolution + 2 + (mVerticalResolution + 1) * k + j;
			indices[idx++] = 0 + (mVerticalResolution + 1) * k + j;
		}
	}
	//mVisual->UpdateModelNormals();
	//Buffer->SetHardwareMappingHint(scene::EHM_STATIC);
}

//! pre render method
bool SkyDomeNode::PreRender(Scene* pScene)
{
	if (IsVisible())
		pScene->AddToRenderQueue(RP_SKY, shared_from_this());

	return Node::PreRender(pScene);
}

//! renders the node.
bool SkyDomeNode::Render(Scene* pScene)
{
	const eastl::shared_ptr<CameraNode>& camera = pScene->GetActiveCamera();

	if (!camera || !Renderer::Get())
		return false;

	if ( camera->Get()->IsPerspective() )
	{
		// draw perspective skydome
		//Matrix4x4<float> translate(toWorld);
		GetRelativeTransform().SetTranslation(camera->GetAbsoluteTransform().GetTranslation());
		/*
		Renderer::Get()->SetBlendState(mMeshBuffer->GetMaterial()->mBlendState);
		Renderer::Get()->SetRasterizerState(mMeshBuffer->GetMaterial()->mRasterizerState);
		Renderer::Get()->SetDepthStencilState(mMeshBuffer->GetMaterial()->mDepthStencilState);

		effect->SetMaterial(mMeshBuffer->GetMaterial());
		*/

		Renderer::Get()->Draw(mVisual);
		/*
		Renderer::Get()->SetDefaultDepthStencilState();
		Renderer::Get()->SetDefaultRasterizerState();
		Renderer::Get()->SetDefaultBlendState();
		*/
	}
	/*
	// for debug purposes only:
	if (DebugDataVisible())
	{
		Material m;
		m.mLighting = false;
		Renderer::Get()->SetMaterial(m);

		if (DebugDataVisible() & DS_NORMALS)
		{
			// draw normals
			const float debugNormalLength = 1.f;
			const eastl::array<float, 4> debugNormalColor{ 
				255 / 255.f, 34 / 255.f, 221 / 255.f, 221 / 255.f };
			Renderer::Get()->DrawMeshBufferNormals(mBuffer, debugNormalLength, debugNormalColor);
		}

		// show mesh
		if (DebugDataVisible() & DS_MESH_WIRE_OVERLAY)
		{
			m.mWireframe = true;
			Renderer::Get()->SetMaterial(m);

			Renderer::Get()->DrawMeshBuffer(mBuffer);
		}
	}
	*/
	return Node::Render(pScene);
}

//! returns the material based on the zero based index i. To get the amount
//! of materials used by this scene node, use GetMaterialCount().
//! This function is needed for inserting the node into the scene hirachy on a
//! optimal position for minimizing renderstate changes, but can also be used
//! to directly modify the material of a scene node.
eastl::shared_ptr<Material> const& SkyDomeNode::GetMaterial(unsigned int i)
{
	return mMeshBuffer->GetMaterial();
}

//! returns amount of materials used by this scene node.
unsigned int SkyDomeNode::GetMaterialCount() const
{
	return 1;
}

//! Sets the texture of the specified layer in all materials of this scene node to the new texture.
/** \param textureLayer Layer of texture to be set. Must be a value smaller than MATERIAL_MAX_TEXTURES.
\param texture New texture to be used. */
void SkyDomeNode::SetMaterialTexture(unsigned int textureLayer, eastl::shared_ptr<Texture2> texture)
{
	if (textureLayer >= MATERIAL_MAX_TEXTURES)
		return;

	for (unsigned int i = 0; i<GetMaterialCount(); ++i)
		GetMaterial(i)->SetTexture(textureLayer, texture);
}

//! Sets the material type of all materials in this scene node to a new material type.
/** \param newType New type of material to be set. */
void SkyDomeNode::SetMaterialType(MaterialType newType)
{
	for (unsigned int i = 0; i<GetMaterialCount(); ++i)
		GetMaterial(i)->mType = newType;
}