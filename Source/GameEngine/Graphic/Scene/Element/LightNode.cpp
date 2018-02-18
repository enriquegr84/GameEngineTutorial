// Copyright (C) 2002-2012 Nikolaus Gebhardt
// This file is part of the "Irrlicht Engine".
// For conditions of distribution and use, see copyright notice in irrlicht.h

#include "LightNode.h"

#include "Graphic/Renderer/Renderer.h"
#include "Graphic/Effect/Material.h"

#include "Core/OS/OS.h"

#include "Graphic/Scene/Scene.h"

//! constructor
LightNode::LightNode(const ActorId actorId, PVWUpdater& updater,
	WeakBaseRenderComponentPtr renderComponent, eastl::array<float, 4> color, float radius)
	:	Node(actorId, renderComponent, RP_TRANSPARENT, NT_LIGHT), mDriverLightIndex(-1), mLightIsOn(true), mLightData(true, true)
{
	#ifdef _DEBUG
	//setDebugName("LightNode");
	#endif
	mPVWUpdater = updater;
	mLightData.mLighting->mDiffuse = color;
	// set some useful specular color
	//mLightData.mLighting->mSpecular = 
	//	color.GetInterpolated(eastl::array<float, 4>{1.0f, 1.0f, 1.0f, 1.0f}, 0.7f);
	
	SetRadius(radius);
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

	Matrix4x4<float> toWorld, fromWorld;
	//Transform(&toWorld, &fromWorld);
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


//! sets the light data
void LightNode::SetLightData(const Light& light)
{
	mLightData = light;
}


//! \return Returns the light data.
const Light& LightNode::GetLightData() const
{
	return mLightData;
}


//! \return Returns the light data.
Light& LightNode::GetLightData()
{
	return mLightData;
}

void LightNode::SetVisible(bool isVisible)
{
	SetVisible(isVisible);

	if(mDriverLightIndex < 0)
		return;

	if (!Renderer::Get())
		return;

	mLightIsOn = isVisible;
	//Renderer::Get()->TurnLightOn((unsigned int)mDriverLightIndex, mLightIsOn);
}

//! Sets the light's radius of influence.
/** Outside this radius the light won't lighten geometry and cast no
shadows. Setting the radius will also influence the attenuation, setting
it to (0,1/radius,0). If you want to override this behavior, set the
attenuation after the radius.
\param radius The new radius. */
void LightNode::SetRadius(float radius)
{
	mLightData.mLighting->mRadius=radius;
	mLightData.mLighting->mAttenuation = Vector4<float>{ 0.f, 1.f / radius, 0.f, 0.f };
	DoLightRecalc();
}


//! Gets the light's radius of influence.
/** \return The current radius. */
float LightNode::GetRadius() const
{
	return mLightData.mLighting->mRadius;
}


//! Sets the light type.
/** \param type The new type. */
void LightNode::SetLightType(LightType type)
{
	mLightData.mLighting->mType=type;
}


//! Gets the light type.
/** \return The current light type. */
LightType LightNode::GetLightType() const
{
	return mLightData.mLighting->mType;
}


//! Sets whether this light casts shadows.
/** Enabling this flag won't automatically cast shadows, the meshes
will still need shadow scene nodes attached. But one can enable or
disable distinct lights for shadow casting for performance reasons.
\param shadow True if this light shall cast shadows. */
void LightNode::EnableCastShadow(bool shadow)
{
	mLightData.mLighting->mCastShadows=shadow;
}


//! Check whether this light casts shadows.
/** \return True if light would cast shadows, else false. */
bool LightNode::GetCastShadow() const
{
	return mLightData.mLighting->mCastShadows;
}


void LightNode::DoLightRecalc()
{
	Matrix4x4<float> toWorld, fromWorld;
	//Transform(&toWorld, &fromWorld);

	if ((mLightData.mLighting->mType == LT_SPOT) || 
		(mLightData.mLighting->mType == LT_DIRECTIONAL))
	{
		mLightData.mLighting->mDirection = Vector3<float>{ 0.f, 0.f, 1.f };
		//toWorld.RotateVect(mLightData.mLighting->mDirection);
		//mLightData.mLighting->mDirection.Normalize();
	}
	if ((mLightData.mLighting->mType == LT_SPOT) || 
		(mLightData.mLighting->mType == LT_POINT))
	{
		const float r = mLightData.mLighting->mRadius * mLightData.mLighting->mRadius * 0.5f;
		//mBBox.MaxEdge.set( r, r, r );
		//mBBox.MinEdge.set( -r, -r, -r );
		//Get()->SetAutomaticCulling( AC_BOX );
		//Get()->SetAutomaticCulling( AC_OFF );
		//mLightData.mLighting->mPosition = toWorld.GetTranslation();
	}
	if (mLightData.mLighting->mType == LT_DIRECTIONAL)
	{
		//mBBox.Reset( 0, 0, 0 );
		//Get()->SetAutomaticCulling( AC_OFF );
	}

	//SetTransform(&toWorld);
}
