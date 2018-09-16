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
	WeakBaseRenderComponentPtr renderComponent, const unsigned int subdivU, 
	const unsigned int subdivV, const eastl::array<float, 4> foot, const eastl::array<float, 4> tail)
:	Node(actorId, renderComponent, RP_TRANSPARENT, NT_VOLUME_LIGHT), mMesh(0), 
	mLPDistance(8.0f), mSubdivideU(subdivU), mSubdivideV(subdivV), mFootColor(foot), 
	mTailColor(tail), mLightDimensions(Vector3<float>{1.0f, 1.2f, 1.0f})
{
	mPVWUpdater = updater;
	ConstructLight();
}


VolumeLightNode::~VolumeLightNode()
{

}


void VolumeLightNode::ConstructLight()
{
	/*
	mMesh = eastl::shared_ptr<BaseMesh>(GetGeometryCreator()->CreateVolumeLightMesh(
		mSubdivideU, mSubdivideV, mFootColor, mTailColor, mLPDistance, mLightDimensions));
	*/
}


//! prerender
bool VolumeLightNode::PreRender(Scene *pScene)
{
	if (IsVisible())
	{
		// register according to material types counted
		if (!pScene->IsCulled(this))
			pScene->AddToRenderQueue(RP_TRANSPARENT, shared_from_this());
	}

	return Node::PreRender(pScene);
}

//
// VolumeLightNode::Render
//
bool VolumeLightNode::Render(Scene *pScene)
{
	/*
	Matrix4x4<float> toWorld, fromWorld;
	//Get()->Transform(&toWorld, &fromWorld);

	Renderer::Get()->SetTransform(ETS_WORLD, toWorld);

	Renderer::Get()->SetMaterial(mMesh->GetMeshBuffer(0)->GetMaterial());
	Renderer::Get()->DrawMeshBuffer(mMesh->GetMeshBuffer(0));
	*/
	return Node::Render(pScene);
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
	if (i >= mMesh->GetMeshBufferCount())
		return nullptr;

	return mMesh->GetMeshBuffer(i)->GetMaterial();
}

//! returns amount of materials used by this scene node.
unsigned int VolumeLightNode::GetMaterialCount() const
{
	return mMesh->GetMeshBufferCount();
}

//! Sets all material flags at once to a new value.
/** Useful, for example, if you want the whole mesh to be affected by light.
\param flag Which flag of all materials to be set.
\param newvalue New value of that flag. */
void VolumeLightNode::SetMaterialFlag(MaterialFlag flag, bool newvalue)
{
	for (unsigned int i = 0; i<GetMaterialCount(); ++i)
		GetMaterial(i).SetFlag(flag, newvalue);
}

//! Sets the texture of the specified layer in all materials of this scene node to the new texture.
/** \param textureLayer Layer of texture to be set. Must be a value smaller than MATERIAL_MAX_TEXTURES.
\param texture New texture to be used. */
void VolumeLightNode::SetMaterialTexture(unsigned int textureLayer, Texture2* texture)
{
	if (textureLayer >= MATERIAL_MAX_TEXTURES)
		return;

	for (unsigned int i = 0; i<GetMaterialCount(); ++i)
		GetMaterial(i).SetTexture(textureLayer, texture);
}

//! Sets the material type of all materials in this scene node to a new material type.
/** \param newType New type of material to be set. */
void VolumeLightNode::SetMaterialType(MaterialType newType)
{
	for (unsigned int i = 0; i<GetMaterialCount(); ++i)
		GetMaterial(i)->mType = newType;
}
