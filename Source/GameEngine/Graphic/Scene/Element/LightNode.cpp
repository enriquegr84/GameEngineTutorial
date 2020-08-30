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
LightNode::LightNode(const ActorId actorId, PVWUpdater* updater, const eastl::shared_ptr<Light>& light)
	:	Node(actorId, NT_LIGHT), mLight(light)
{
	mPVWUpdater = updater;

	mMaterial = eastl::make_shared<Material>();
	mMaterial->mEmissive = { 0.0f, 0.0f, 0.0f, 1.0f };
	mMaterial->mAmbient = mLight->mLighting->mAmbient;
	mMaterial->mDiffuse = mLight->mLighting->mDiffuse;
	mMaterial->mSpecular = mLight->mLighting->mSpecular;

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
	// do nothing
	return Node::Render(pScene);
}

//! Gets the light type.
/** \return The current light type. */
LightType LightNode::GetLightType() const
{
	return mLight->mLighting->mType;
}

//! Gets the light.
/** \return The current light. */
eastl::shared_ptr<Light> const& LightNode::GetLight() const 
{ 
	return mLight; 
}

void LightNode::DoLightRecalc()
{
	/*
	Vector4<float> lightDirection = Vector4<float>::Unit(1);
	GetAbsoluteTransform().GetRotation().Transformation(Vector4<float>::Unit(1), lightDirection);
	mLight->mLighting->mDirection = HProject(lightDirection);
	Normalize(mLight->mLighting->mDirection);
	*/
	mLight->mLighting->mPosition = GetAbsoluteTransform().GetTranslation();
}


//! returns the material based on the zero based index i. To get the amount
//! of materials used by this scene node, use GetMaterialCount().
//! This function is needed for inserting the node into the scene hirachy on a
//! optimal position for minimizing renderstate changes, but can also be used
//! to directly modify the material of a scene node.
eastl::shared_ptr<Material> const& LightNode::GetMaterial(unsigned int i)
{
	return mMaterial;
}

//! returns amount of materials used by this scene node.
unsigned int LightNode::GetMaterialCount() const
{
	return 1;
}

//! Sets the texture of the specified layer in all materials of this scene node to the new texture.
/** \param textureLayer Layer of texture to be set. Must be a value smaller than MATERIAL_MAX_TEXTURES.
\param texture New texture to be used. */
void LightNode::SetMaterialTexture(unsigned int textureLayer, eastl::shared_ptr<Texture2> texture)
{
	if (textureLayer >= MATERIAL_MAX_TEXTURES)
		return;

	for (unsigned int i = 0; i<GetMaterialCount(); ++i)
		GetMaterial(i)->SetTexture(textureLayer, texture);
}

//! Sets the material type of all materials in this scene node to a new material type.
/** \param newType New type of material to be set. */
void LightNode::SetMaterialType(MaterialType newType)
{
	for (unsigned int i = 0; i<GetMaterialCount(); ++i)
		GetMaterial(i)->mType = newType;
}