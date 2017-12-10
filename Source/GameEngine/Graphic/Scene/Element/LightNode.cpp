// Copyright (C) 2002-2012 Nikolaus Gebhardt
// This file is part of the "Irrlicht Engine".
// For conditions of distribution and use, see copyright notice in irrlicht.h

#include "LightNode.h"

#include "Graphic/Renderer/Renderer.h"
#include "Graphic/Effect/Material.h"

#include "Core/OS/OS.h"

#include "Graphic/Scene/Scene.h"

//! constructor
LightNode::LightNode(const ActorId actorId,
	WeakBaseRenderComponentPtr renderComponent, eastl::array<float, 4> color, float radius)
:	Node(actorId, renderComponent, ERP_TRANSPARENT, ESNT_LIGHT), mDriverLightIndex(-1), mLightIsOn(true)
{
	#ifdef _DEBUG
	//setDebugName("LightSceneNode");
	#endif

	mLightData.mDiffuseColor = color;
	// set some useful specular color
	mLightData.mSpecularColor = 
		color.GetInterpolated(eastl::array<float, 4>{255.f, 255.f, 255.f, 255.f}, 0.7f);

	SetRadius(radius);
}

//! prerender
bool LightNode::PreRender(Scene *pScene)
{
	DoLightRecalc();

	if (mProps.IsVisible())
		pScene->AddToRenderQueue(ERP_LIGHT, eastl::shared_from_this());

	return Node::PreRender(pScene);
}

//
// LightSceneNode::Render
//
bool LightNode::Render(Scene *pScene)
{
	Matrix4x4 toWorld, fromWorld;
	Get()->Transform(&toWorld, &fromWorld);

	const eastl::shared_ptr<Renderer>& renderer = pScene->GetRenderer();
	if (!renderer)
		return false;

	if ( mProps.DebugDataVisible() & EDS_BBOX )
	{
		renderer->SetTransform(ETS_WORLD, toWorld);
		Material m;
		m.Lighting = false;
		renderer->SetMaterial(m);

		switch ( mLightData.mType )
		{
			case ELT_POINT:
			case ELT_SPOT:
				renderer->Draw3DBox(mBBox, mLightData.mDiffuseColor.ToColor());
				break;

			case ELT_DIRECTIONAL:
				renderer->Draw3DLine(Vector3<float>{0.f, 0.f, 0.f},
						mLightData.mDirection * mLightData.mRadius,
						mLightData.mDiffuseColor.ToColor());
				break;
			default:
				break;
		}
	}

	mDriverLightIndex = renderer->AddDynamicLight(mLightData);
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
	mProps.SetVisible(isVisible);

	if(mDriverLightIndex < 0)
		return;

	const eastl::shared_ptr<ScreenElementScene>& pScene = gameApp->GetHumanView()->mScene;
	const eastl::shared_ptr<Renderer>& renderer = pScene->GetRenderer();
	if (!renderer)
		return;

	mLightIsOn = isVisible;
	renderer->TurnLightOn((unsigned int)mDriverLightIndex, mLightIsOn);
}

//! returns the axis aligned bounding box of this node
const AABBox3<float>& LightNode::GetBoundingBox() const
{
	return mBBox;
}

//! Sets the light's radius of influence.
/** Outside this radius the light won't lighten geometry and cast no
shadows. Setting the radius will also influence the attenuation, setting
it to (0,1/radius,0). If you want to override this behavior, set the
attenuation after the radius.
\param radius The new radius. */
void LightNode::SetRadius(float radius)
{
	mLightData.m_Radius=radius;
	mLightData.m_Attenuation.set(0.f, 1.f/radius, 0.f);
	DoLightRecalc();
}


//! Gets the light's radius of influence.
/** \return The current radius. */
float LightNode::GetRadius() const
{
	return mLightData.m_Radius;
}


//! Sets the light type.
/** \param type The new type. */
void LightNode::SetLightType(E_LIGHT_TYPE type)
{
	mLightData.mType=type;
}


//! Gets the light type.
/** \return The current light type. */
E_LIGHT_TYPE LightNode::GetLightType() const
{
	return mLightData.m_Type;
}


//! Sets whether this light casts shadows.
/** Enabling this flag won't automatically cast shadows, the meshes
will still need shadow scene nodes attached. But one can enable or
disable distinct lights for shadow casting for performance reasons.
\param shadow True if this light shall cast shadows. */
void LightNode::EnableCastShadow(bool shadow)
{
	mLightData.mCastShadows=shadow;
}


//! Check whether this light casts shadows.
/** \return True if light would cast shadows, else false. */
bool LightNode::GetCastShadow() const
{
	return mLightData.mCastShadows;
}


void LightNode::DoLightRecalc()
{
	Matrix4x4 toWorld, fromWorld;
	Get()->Transform(&toWorld, &fromWorld);

	if ((mLightData.mType == ELT_SPOT) || (mLightData.mType == ELT_DIRECTIONAL))
	{
		mLightData.mDirection = Vector3<float>{ 0.f, 0.f, 1.f };
		toWorld.RotateVect(mLightData.mDirection);
		mLightData.mDirection.Normalize();
	}
	if ((mLightData.mType == ELT_SPOT) || (mLightData.mType == ELT_POINT))
	{
		const float r = mLightData.mRadius * mLightData.mRadius * 0.5f;
		mBBox.MaxEdge.set( r, r, r );
		mBBox.MinEdge.set( -r, -r, -r );
		//Get()->SetAutomaticCulling( EAC_BOX );
		Get()->SetAutomaticCulling( EAC_OFF );
		mLightData.mPosition = toWorld.GetTranslation();
	}
	if (mLightData.mType == ELT_DIRECTIONAL)
	{
		mBBox.Reset( 0, 0, 0 );
		Get()->SetAutomaticCulling( EAC_OFF );
	}

	SetTransform(&toWorld);
}
