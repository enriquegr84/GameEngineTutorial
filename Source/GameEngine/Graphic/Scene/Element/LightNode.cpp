// Copyright (C) 2002-2012 Nikolaus Gebhardt
// This file is part of the "Irrlicht Engine".
// For conditions of distribution and use, see copyright notice in irrlicht.h

#include "LightNode.h"

#include "Graphic/Renderer/Renderer.h"
#include "Graphic/Effect/Material.h"

#include "Core/OS/OS.h"

#include "Graphic/Scene/Scene.h"

#include "Application/GameApplication.h"

//! constructor
LightNode::LightNode(const ActorId actorId, PVWUpdater* updater,
	WeakBaseRenderComponentPtr renderComponent, const eastl::shared_ptr<Light>& light)
	:	Node(actorId, renderComponent, RP_TRANSPARENT, NT_LIGHT), mDriverLightIndex(-1), mLightIsOn(true)
{
	mPVWUpdater = updater;

	// A point light illuminates the target.  Create a semitransparent
	// material for the patch.
	eastl::shared_ptr<Material> material = eastl::make_shared<Material>();
	eastl::shared_ptr<LightCameraGeometry> geometry = eastl::make_shared<LightCameraGeometry>();

	material->mEmissive = { 0.0f, 0.0f, 0.0f, 1.0f };
	material->mAmbient = { 0.5f, 0.5f, 0.5f, 1.0f };
	material->mDiffuse = { 1.0f, 0.85f, 0.75f, 0.5f };
	material->mSpecular = { 0.8f, 0.8f, 0.8f, 1.0f };

	// Create a parabolic rectangle patch that is illuminated by the light.
	// To hide the artifacts of vertex normal lighting on a grid, the patch
	// is slightly bent so that the intersection with a plane is nearly
	// circular.  The patch is translated slightly below the plane of the
	// ground to hide the corners and the jaggies.
	struct Vertex
	{
		Vector3<float> position, normal;
	};

	VertexFormat vformat;
	vformat.Bind(VA_POSITION, DF_R32G32B32_FLOAT, 0);
	vformat.Bind(VA_NORMAL, DF_R32G32B32_FLOAT, 0);

	// Create a flat surface.
	MeshFactory mf;
	mf.SetVertexFormat(vformat);
	mVisual = mf.CreateRectangle(64, 64, light->mLighting->mRadius, light->mLighting->mRadius);

	// Adjust the heights to form a paraboloid.
	eastl::shared_ptr<VertexBuffer> vbuffer = mVisual->GetVertexBuffer();
	unsigned int const numVertices = vbuffer->GetNumActiveElements();
	Vertex* vertex = vbuffer->Get<Vertex>();
	for (unsigned int i = 0; i < numVertices; ++i)
	{
		Vector3<float>& pos = vertex[i].position;
		pos[2] = 1.0f - (pos[0] * pos[0] + pos[1] * pos[1]) / 128.0f;
	}

	eastl::string path = FileSystem::Get()->GetPath("Effects/PointLightEffectPerVertex.hlsl");
	eastl::shared_ptr<PointLightEffect> effect = eastl::make_shared<PointLightEffect>(
		ProgramFactory::Get(), mPVWUpdater->GetUpdater(), path, 0, material, light->mLighting, geometry);
	mVisual->SetEffect(effect);
	mVisual->UpdateModelNormals();
	mPVWUpdater->Subscribe(mWorldTransform, effect->GetPVWMatrixConstant());

	// Encapsulate the light in a light node.  Rotate the light node so the
	// light points downward.
	mLightVolume = eastl::make_shared<ViewVolumeNode>(INVALID_ACTOR_ID, light);
	mLightVolume->SetOnUpdate([this](ViewVolumeNode*)
	{
		// The camera model position must be updated for the light targets to
		// move.  The light model position is not updated because the point
		// lights must move with their corresponding light targets.
		eastl::shared_ptr<PointLightEffect> effect =
			eastl::static_pointer_cast<PointLightEffect>(mVisual->GetEffect());
		auto geometry = effect->GetGeometry();

		GameApplication* gameApp = (GameApplication*)Application::App;
		const eastl::shared_ptr<ScreenElementScene>& pScene = gameApp->GetHumanView()->mScene;
		geometry->cameraModelPosition = 
			pScene->GetActiveCamera()->GetAbsoluteTransform().GetTranslationW1();
		effect->UpdateGeometryConstant();
	});

	AttachChild(mLightVolume);

	DoLightRecalc();
}

//! prerender
bool LightNode::PreRender(Scene *pScene)
{
	DoLightRecalc();

	if (IsVisible())
		pScene->AddToRenderQueue(RP_LIGHT, shared_from_this());

	return Node::PreRender(pScene);
}

//
// LightSceneNode::Render
//
bool LightNode::Render(Scene *pScene)
{
	if (!Renderer::Get())
		return false;

	/*
	if (DebugDataVisible() & DS_BBOX )
	{
		Renderer::Get()->SetTransform(TS_WORLD, toWorld);
		Material m;
		m.mLighting = false;
		Renderer::Get()->SetMaterial(m);

		switch ( mLightData.mType )
		{
			case LT_POINT:
			case LT_SPOT:
				Renderer::Get()->Draw3DBox(mBBox, mLightData.mLighting->mDiffuse.ToColor());
				break;

			case LT_DIRECTIONAL:
				Renderer::Get()->Draw3DLine(Vector3<float>{0.f, 0.f, 0.f},
						mLightData.mLighting->mDirection * mLightData.mRadius,
						mLightData.mLighting.>mDiffuse.ToColor());
				break;
			default:
				break;
		}
	}

	mDriverLightIndex = Renderer::Get()->AddDynamicLight(mLightData.mLighting);
	*/
	SetVisible(mLightIsOn);
	return true;
}

const eastl::shared_ptr<Lighting>& LightNode::GetLightData() const
{
	eastl::shared_ptr<PointLightEffect> effect =
		eastl::static_pointer_cast<PointLightEffect>(mVisual->GetEffect());
	return effect->GetLighting();
}

void LightNode::SetVisible(bool isVisible)
{
	mIsVisible = isVisible;

	if(mDriverLightIndex < 0)
		return;

	if (!Renderer::Get())
		return;

	mLightIsOn = isVisible;
	//Renderer::Get()->TurnLightOn((unsigned int)mDriverLightIndex, mLightIsOn);
}

void LightNode::DoLightRecalc()
{
	eastl::shared_ptr<PointLightEffect> effect =
		eastl::static_pointer_cast<PointLightEffect>(mVisual->GetEffect());

	if ((effect->GetLighting()->mType == LT_SPOT) ||
		(effect->GetLighting()->mType == LT_DIRECTIONAL))
	{
		effect->GetLighting()->mDirection = Vector3<float>{ 0.f, 0.f, 1.f };
		//toWorld.RotateVect(mLightData.mLighting->mDirection);
		//mLightData.mLighting->mDirection.Normalize();
	}
	if ((effect->GetLighting()->mType == LT_SPOT) ||
		(effect->GetLighting()->mType == LT_POINT))
	{
		const float r = effect->GetLighting()->mRadius * effect->GetLighting()->mRadius * 0.5f;
		//mBBox.MaxEdge.set( r, r, r );
		//mBBox.MinEdge.set( -r, -r, -r );
		//Get()->SetAutomaticCulling( AC_BOX );
		//Get()->SetAutomaticCulling( AC_OFF );
		//mLightData.mLighting->mPosition = toWorld.GetTranslation();
	}
	if (effect->GetLighting()->mType == LT_DIRECTIONAL)
	{
		//mBBox.Reset( 0, 0, 0 );
		//Get()->SetAutomaticCulling( AC_OFF );
	}

	//SetTransform(&toWorld);
}