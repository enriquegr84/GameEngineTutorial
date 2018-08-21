// Copyright (C) 2002-2012 Nikolaus Gebhardt
// This file is part of the "Irrlicht Engine".
// For conditions of distribution and use, see copyright notice in irrlicht.h

#include "SkyBoxNode.h"
#include "CameraNode.h"

#include "Core/OS/OS.h"

#include "Graphic/Renderer/Renderer.h"
#include "Graphic/Effect/Material.h"

#include "Graphic/Scene/Scene.h"

//! constructor
SkyBoxNode::SkyBoxNode(const ActorId actorId, PVWUpdater* updater, WeakBaseRenderComponentPtr renderComponent,
	const eastl::shared_ptr<Texture2>& top, const eastl::shared_ptr<Texture2>& bottom, const eastl::shared_ptr<Texture2>& left,
	const eastl::shared_ptr<Texture2>& right, const eastl::shared_ptr<Texture2>& front, const eastl::shared_ptr<Texture2>& back)
:	Node(actorId, renderComponent, RP_SKY_BOX, NT_SKY_BOX)
{
	mPVWUpdater = updater;

	//SetAutomaticCulling(AC_OFF);
	//mBBox.MaxEdge.set(0,0,0);
	//mBBox.MinEdge.set(0,0,0);

	// create material
	eastl::shared_ptr<Material> mat = eastl::make_shared<Material>();
	/*
	Material mat[6];
	mat.mLighting = false;
	mat.mZBuffer = CFN_DISABLED;
	mat.mZWriteEnable = false;
	mat.mAntiAliasing=0;
	mat.mTextureLayer[0].TextureWrapU = TC_CLAMP_TO_EDGE;
	mat.mTextureLayer[0].TextureWrapV = TC_CLAMP_TO_EDGE;
	*/
	/*

       -111         111
          /6--------/5        y
         /  |      / |        ^  z
        /   |   11-1 |        | /
  -11-1 3---------2  |        |/
        |   7- - -| -4 1-11    *---->x
        | -1-11   |  /       3-------|2
        |/        | /         |    //|
        0---------1/          |  //  |
     -1-1-1     1-1-1         |//    |
	                     0--------1
	*/

	eastl::shared_ptr<Texture2> tex = front;
	if (!tex) tex = left;
	if (!tex) tex = back;
	if (!tex) tex = right;
	if (!tex) tex = top;
	if (!tex) tex = bottom;

	VertexFormat vformat;
	vformat.Bind(VA_POSITION, DF_R32G32B32A32_FLOAT, 0);
	vformat.Bind(VA_NORMAL, DF_R32G32B32A32_FLOAT, 0);
	vformat.Bind(VA_COLOR, DF_R32G32B32A32_FLOAT, 0);
	vformat.Bind(VA_TEXCOORD, DF_R32G32_FLOAT, 0);

	eastl::shared_ptr<VertexBuffer> vertices = eastl::make_shared<VertexBuffer>(vformat, 24);
	eastl::shared_ptr<IndexBuffer> indices = eastl::make_shared<IndexBuffer>(IP_TRISTRIP, 4);

	eastl::string path = FileSystem::Get()->GetPath("Effects/PointLightTextureEffect.hlsl");
	eastl::shared_ptr<PointLightTextureEffect> effect = eastl::make_shared<PointLightTextureEffect>(
		ProgramFactory::Get(), mPVWUpdater->GetUpdater(), path, mat, eastl::make_shared<Lighting>(),
		eastl::make_shared<LightCameraGeometry>(), tex, SamplerState::MIN_L_MAG_L_MIP_L,
		SamplerState::WRAP, SamplerState::WRAP);

	const float onepixel = tex?(1.0f / (tex->GetDimension(0) * 1.5f)) : 0.0f;
	const float t = 1.0f - onepixel;
	const float o = 0.0f + onepixel;

	struct Vertex
	{
		Vector3<float> position;
		Vector3<float> normal;
		Vector4<float> color;
		Vector2<float> tcoord;
	};
	Vertex* vertex = vertices->Get<Vertex>();

	// create front side
	vertex[0].position = { -1.0f, -1.0f, -1.0f };
	vertex[0].normal = { 0.0f, 0.0f, 1.0f };
	vertex[0].tcoord = { t, t };
	vertex[0].color = eastl::array<float, 4>{255.f, 255.f, 255.f, 255.f};

	vertex[1].position = { -1.0f, -1.0f, -1.0f };
	vertex[1].normal = { 0.0f, 0.0f, 1.0f };
	vertex[1].tcoord = { 0.0f, t };
	vertex[1].color = eastl::array<float, 4>{255.f, 255.f, 255.f, 255.f};

	vertex[2].position = { -1.0f, -1.0f, -1.0f };
	vertex[2].normal = { 0.0f, 0.0f, 1.0f };
	vertex[2].tcoord = { 0.0f, 0.0f };
	vertex[2].color = eastl::array<float, 4>{255.f, 255.f, 255.f, 255.f};

	vertex[3].position = { -1.0f, -1.0f, -1.0f };
	vertex[3].normal = { 0.0f, 0.0f, 1.0f };
	vertex[3].tcoord = { t, 0.0f };
	vertex[3].color = eastl::array<float, 4>{255.f, 255.f, 255.f, 255.f};

	// create left side
	vertex[4].position = { 1.0f, -1.0f, -1.0f };
	vertex[4].normal = { -1.0f, 0.0f, 1.0f };
	vertex[4].tcoord = { t, t };
	vertex[4].color = eastl::array<float, 4>{255.f, 255.f, 255.f, 255.f};

	vertex[5].position = { 1.0f, -1.0f, 1.0f };
	vertex[5].normal = { -1.0f, 0.0f, 1.0f };
	vertex[5].tcoord = { 0.0f, t };
	vertex[5].color = eastl::array<float, 4>{255.f, 255.f, 255.f, 255.f};

	vertex[6].position = { 1.0f, 1.0f, 1.0f };
	vertex[6].normal = { -1.0f, 0.0f, 1.0f };
	vertex[6].tcoord = { 0.0f, 0.0f };
	vertex[6].color = eastl::array<float, 4>{255.f, 255.f, 255.f, 255.f};

	vertex[7].position = { 1.0f, 1.0f, -1.0f };
	vertex[7].normal = { -1.0f, 0.0f, 1.0f };
	vertex[7].tcoord = { t, 0.0f };
	vertex[7].color = eastl::array<float, 4>{255.f, 255.f, 255.f, 255.f};

	// create back side
	vertex[8].position = { 1.0f, -1.0f, 1.0f };
	vertex[8].normal = { 0.0f, 0.0f, -1.0f };
	vertex[8].tcoord = { t, t };
	vertex[8].color = eastl::array<float, 4>{255.f, 255.f, 255.f, 255.f};

	vertex[9].position = { -1.0f, -1.0f, 1.0f };
	vertex[9].normal = { 0.0f, 0.0f, -1.0f };
	vertex[9].tcoord = { 0.0f, t };
	vertex[9].color = eastl::array<float, 4>{255.f, 255.f, 255.f, 255.f};

	vertex[10].position = { -1.0f, 1.0f, 1.0f };
	vertex[10].normal = { 0.0f, 0.0f, -1.0f };
	vertex[10].tcoord = { 0.0f, 0.0f };
	vertex[10].color = eastl::array<float, 4>{255.f, 255.f, 255.f, 255.f};

	vertex[11].position = { 1.0f, 1.0f, 1.0f };
	vertex[11].normal = { 0.0f, 0.0f, -1.0f };
	vertex[11].tcoord = { t, 0.0f };
	vertex[11].color = eastl::array<float, 4>{255.f, 255.f, 255.f, 255.f};

	// create right side
	vertex[12].position = { -1.0f, -1.0f, 1.0f };
	vertex[12].normal = { 1.0f, 0.0f, 0.0f };
	vertex[12].tcoord = { t, t };
	vertex[12].color = eastl::array<float, 4>{255.f, 255.f, 255.f, 255.f};

	vertex[13].position = { -1.0f, -1.0f, -1.0f };
	vertex[13].normal = { 1.0f, 0.0f, 0.0f };
	vertex[13].tcoord = { 0.0f, t };
	vertex[13].color = eastl::array<float, 4>{255.f, 255.f, 255.f, 255.f};

	vertex[14].position = { -1.0f, 1.0f, -1.0f };
	vertex[14].normal = { 1.0f, 0.0f, 0.0f };
	vertex[14].tcoord = { 0.0f, 0.0f };
	vertex[14].color = eastl::array<float, 4>{255.f, 255.f, 255.f, 255.f};

	vertex[15].position = { -1.0f, 1.0f, 1.0f };
	vertex[15].normal = { 1.0f, 0.0f, 0.0f };
	vertex[15].tcoord = { t, 0.0f };
	vertex[15].color = eastl::array<float, 4>{255.f, 255.f, 255.f, 255.f};

	// create top side
	vertex[16].position = { 1.0f, 1.0f, -1.0f };
	vertex[16].normal = { 0.0f, -1.0f, 0.0f };
	vertex[16].tcoord = { t, t };
	vertex[16].color = eastl::array<float, 4>{255.f, 255.f, 255.f, 255.f};

	vertex[17].position = { 1.0f, 1.0f, 1.0f };
	vertex[17].normal = { 0.0f, -1.0f, 0.0f };
	vertex[17].tcoord = { 0.0f, t };
	vertex[17].color = eastl::array<float, 4>{255.f, 255.f, 255.f, 255.f};

	vertex[18].position = { -1.0f, 1.0f, 1.0f };
	vertex[18].normal = { 0.0f, -1.0f, 0.0f };
	vertex[18].tcoord = { 0.0f, 0.0f };
	vertex[18].color = eastl::array<float, 4>{255.f, 255.f, 255.f, 255.f};

	vertex[19].position = { -1.0f, 1.0f, -1.0f };
	vertex[19].normal = { 0.0f, -1.0f, 0.0f };
	vertex[19].tcoord = { t, 0.0f };
	vertex[19].color = eastl::array<float, 4>{255.f, 255.f, 255.f, 255.f};

	// create bottom side
	vertex[20].position = { 1.0f, -1.0f, 1.0f };
	vertex[20].normal = { 0.0f, 1.0f, 0.0f };
	vertex[20].tcoord = { 0.0f, 0.0f };
	vertex[20].color = eastl::array<float, 4>{255.f, 255.f, 255.f, 255.f};

	vertex[21].position = { 1.0f, -1.0f, -1.0f };
	vertex[21].normal = { 0.0f, 1.0f, 0.0f };
	vertex[21].tcoord = { t, 0.0f };
	vertex[21].color = eastl::array<float, 4>{255.f, 255.f, 255.f, 255.f};

	vertex[22].position = { -1.0f, -1.0f, -1.0f };
	vertex[22].normal = { 0.0f, 1.0f, 0.0f };
	vertex[22].tcoord = { t, t };
	vertex[22].color = eastl::array<float, 4>{255.f, 255.f, 255.f, 255.f};

	vertex[23].position = { -1.0f, -1.0f, 1.0f };
	vertex[23].normal = { 0.0f, 1.0f, 0.0f };
	vertex[23].tcoord = { 0.0f, t };
	vertex[23].color = eastl::array<float, 4>{255.f, 255.f, 255.f, 255.f};

	mVisual = eastl::make_shared<Visual>(vertices, indices, effect);
	mVisual->SetEffect(effect);
	mPVWUpdater->Subscribe(mVisual->GetAbsoluteTransform(), effect->GetPVWMatrixConstant());
}

//! pre render method
bool SkyBoxNode::PreRender(Scene* pScene)
{
	if (IsVisible())
		pScene->AddToRenderQueue(RP_SKY_BOX, shared_from_this());

	return Node::PreRender(pScene);
}

//! renders the node.
bool SkyBoxNode::Render(Scene* pScene)
{
	const eastl::shared_ptr<CameraNode>& camera = pScene->GetActiveCamera();

	if (!camera || !Renderer::Get())
		return false;

	//Matrix4x4<float> toWorld, fromWorld;
	//mParent->Transform(&toWorld, &fromWorld);

	//if ( !camera->IsOrthogonal() )
	{
		// draw perspective skybox
		//Matrix4x4<float> translate(toWorld);
		GetAbsoluteTransform().SetTranslation(camera->GetAbsoluteTransform().GetTranslation());

		// Draw the sky box between the near and far clip plane
		const float viewDistance = (camera->Get()->GetDMin() + camera->Get()->GetDMax()) * 0.5f;
		
		GetAbsoluteTransform().SetScale(Vector3<float>{ viewDistance, viewDistance, viewDistance });

		/*
		Renderer::Get()->SetTransform(TS_WORLD, translate * scale);
		for (int i=0; i<6; ++i)
		{
			Renderer::Get()->SetMaterial(mMaterials[i]);
			Renderer::Get()->DrawIndexedTriangleFan(&mVertices[i*4], 4, mIndices, 2);
		}
		*/
	}
	//else
	{
		// draw orthogonal skybox,
		// simply choose one texture and draw it as 2d picture.
		// there could be better ways to do this, but currently I think this is ok.

		Vector3<float> target =
			camera->GetAbsoluteTransform().GetTranslation() + HProject(camera->Get()->GetDVector());
			/* +
			camera->GetAbsoluteTransform().GetRotation().RotationToDirection();*/
		if (camera->GetTarget())
			target = camera->GetTarget()->GetAbsoluteTransform().GetTranslation();

		Vector3<float> lookVect(target - camera->GetAbsoluteTransform().GetTranslation());
		Normalize(lookVect);
		Vector3<float> absVect{ fabs(lookVect[0]), fabs(lookVect[1]), fabs(lookVect[2]) };

		int idx = 0;
		if ( absVect[0] >= absVect[1] && absVect[0] >= absVect[2] )
		{
			// x direction
			idx = lookVect[0] > 0 ? 0 : 2;
		}
		else
		if ( absVect[1] >= absVect[0] && absVect[1] >= absVect[2] )
		{
			// y direction
			idx = lookVect[1] > 0 ? 4 : 5;
		}
		else
		if ( absVect[2] >= absVect[0] && absVect[2] >= absVect[1] )
		{
			// z direction
			idx = lookVect[2] > 0 ? 1 : 3;
		}
		/*
		eastl::shared_ptr<Texture2> tex(mMaterials[idx].GetTexture(0));

		if ( tex )
		{
			RectangleShape<2, int> rctDest(Vector2<int>{-1, 0},
									Dimension2i(renderer->GetCurrentRenderTargetSize()));
			RectangleShape<2, int> rctSrc(Vector2<int>(0,0),
									Dimension2i(tex->GetSize()));

			Renderer::Get()->Draw2DImage(tex, rctDest, rctSrc);
		}
		*/
		Renderer::Get()->Draw(mVisual);
	}

	return Node::Render(pScene);
}

//! returns the material based on the zero based index i. To get the amount
//! of materials used by this scene node, use getMaterialCount().
//! This function is needed for inserting the node into the scene hirachy on a
//! optimal position for minimizing renderstate changes, but can also be used
//! to directly modify the material of a scene node.
eastl::shared_ptr<Material> const& SkyBoxNode::GetMaterial(unsigned int i)
{
	eastl::shared_ptr<PointLightTextureEffect> effect =
		eastl::static_pointer_cast<PointLightTextureEffect>(mVisual->GetEffect());
	return effect->GetMaterial();
	//return mMaterials[i];
}

//! returns amount of materials used by this scene node.
unsigned int SkyBoxNode::GetMaterialCount() const
{
	return 6;
}