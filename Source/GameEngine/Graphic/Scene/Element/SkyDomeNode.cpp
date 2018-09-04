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
	vformat.Bind(VA_POSITION, DF_R32G32B32A32_FLOAT, 0);
	vformat.Bind(VA_NORMAL, DF_R32G32B32A32_FLOAT, 0);
	vformat.Bind(VA_COLOR, DF_R32G32B32A32_FLOAT, 0);
	vformat.Bind(VA_TEXCOORD, DF_R32G32_FLOAT, 0);

	eastl::shared_ptr<VertexBuffer> vertices = eastl::make_shared<VertexBuffer>(vformat, 24);
	eastl::shared_ptr<IndexBuffer> indices = eastl::make_shared<IndexBuffer>(IP_TRISTRIP, 4, sizeof(unsigned int));

	// Create the visual effect. The world up-direction is (0,0,1).  Choose
	// the light to point down.
	eastl::shared_ptr<Material> material = eastl::make_shared<Material>();
	material->mEmissive = { 0.0f, 0.0f, 0.0f, 1.0f };
	material->mAmbient = { 0.5f, 0.5f, 0.5f, 1.0f };
	material->mDiffuse = { 0.5f, 0.5f, 0.5f, 1.0f };
	material->mSpecular = { 1.0f, 1.0f, 1.0f, 75.0f };

	//SetAutomaticCulling(AC_OFF);
	/*
	Buffer = new SMeshBuffer();
	Buffer->Material.Lighting = false;
	Buffer->Material.ZBuffer = video::ECFN_NEVER;
	Buffer->Material.ZWriteEnable = false;
	Buffer->Material.AntiAliasing = video::EAAM_OFF;
	Buffer->Material.setTexture(0, sky);
	Buffer->BoundingBox.MaxEdge.set(0, 0, 0);
	Buffer->BoundingBox.MinEdge.set(0, 0, 0);
	*/

	eastl::shared_ptr<Lighting> lighting = eastl::make_shared<Lighting>();
	lighting->mAmbient = Renderer::Get()->GetClearColor();
	lighting->mAttenuation = { 1.0f, 0.0f, 0.0f, 1.0f };

	eastl::shared_ptr<LightCameraGeometry> geometry = eastl::make_shared<LightCameraGeometry>();

	eastl::string path = FileSystem::Get()->GetPath("Effects/PointLightTextureEffect.hlsl");
	eastl::shared_ptr<PointLightTextureEffect> effect = eastl::make_shared<PointLightTextureEffect>(
		ProgramFactory::Get(), mPVWUpdater->GetUpdater(), path, material, lighting, geometry, sky, 
		SamplerState::MIN_L_MAG_L_MIP_L, SamplerState::WRAP, SamplerState::WRAP);

	mVisual = eastl::make_shared<Visual>(vertices, indices, effect);
	mVisual->SetEffect(effect);
	mVisual->UpdateModelNormals();
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
		Vector4<float> color;
		Vector2<float> tcoord;
	};
	eastl::shared_ptr<VertexBuffer> vbuffer = mVisual->GetVertexBuffer();
	vbuffer->Reallocate((mHorizontalResolution + 1) * (mVerticalResolution + 1));
	unsigned int numVertices = vbuffer->GetNumElements();
	Vertex* vertex = vbuffer->Get<Vertex>();

	Vertex vtx;
	vtx.color = Vector4<float>{ 1.f, 1.f, 1.f, 1.f };
	vtx.normal = Vector3<float>{ 0.0f, -1.f, 0.0f };

	const float tcV = mTexturePercentage / mVerticalResolution;
	for (unsigned int k = 0; k <= mHorizontalResolution; ++k)
	{
		float elevation = (float)GE_C_HALF_PI;
		const float tcU = (float)k / (float)mHorizontalResolution;
		const float sinA = Function<float>::Sin(azimuth);
		const float cosA = Function<float>::Cos(azimuth);
		for (unsigned int j = 0; j <= mVerticalResolution; ++j)
		{
			const float cosEr = mRadius * Function<float>::Cos(elevation);
			vtx.position = Vector3<float>{ cosEr*sinA, mRadius*Function<float>::Sin(elevation), cosEr*cosA };
			vtx.tcoord = Vector2<float>{ tcU, j*tcV };

			vtx.normal = -vtx.position;
			Normalize(vtx.normal);

			vertex[k].position = vtx.position;
			vertex[k].normal = vtx.normal;
			vertex[k].color = vtx.color;
			vertex[k].tcoord = vtx.tcoord;
			elevation -= elevationStep;
		}
		azimuth += azimuthStep;
	}

	eastl::shared_ptr<IndexBuffer> ibuffer = mVisual->GetIndexBuffer();
	ibuffer->Reallocate(3 * (2 * mVerticalResolution - 1) * mHorizontalResolution);
	unsigned int numIndices = ibuffer->GetNumElements();
	unsigned int* indices = ibuffer->Get<unsigned int>();

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

	//Buffer->setHardwareMappingHint(scene::EHM_STATIC);
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
		GetAbsoluteTransform().SetTranslation(camera->GetAbsoluteTransform().GetTranslation());

		Renderer::Get()->Draw(mVisual);
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
//! of materials used by this scene node, use getMaterialCount().
//! This function is needed for inserting the node into the scene hirachy on a
//! optimal position for minimizing renderstate changes, but can also be used
//! to directly modify the material of a scene node.
eastl::shared_ptr<Material> const& SkyDomeNode::GetMaterial(unsigned int i)
{
	eastl::shared_ptr<PointLightTextureEffect> effect =
		eastl::static_pointer_cast<PointLightTextureEffect>(mVisual->GetEffect());
	return effect->GetMaterial();
	//return mMaterials[i];
}

//! returns amount of materials used by this scene node.
unsigned int SkyDomeNode::GetMaterialCount() const
{
	return 6;
}