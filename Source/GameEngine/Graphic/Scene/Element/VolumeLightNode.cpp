// Copyright (C) 2007-2012 Dean Wadsworth
// This file is part of the "Irrlicht Engine".
// For conditions of distribution and use, see copyright notice in irrlicht.h

#include "VolumeLightNode.h"

#include "Graphic/Renderer/Renderer.h"
#include "Graphic/Effect/Material.h"

#include "Core/OS/OS.h"

#include "Graphic/Scene/Scene.h"

//! constructor
VolumeLightNode::VolumeLightNode(const ActorId actorId, WeakBaseRenderComponentPtr renderComponent,
	const unsigned int subdivU, const unsigned int subdivV, 
	const eastl::array<float, 4> foot, const eastl::array<float, 4> tail)
:	Node(actorId, renderComponent, ERP_TRANSPARENT, ESNT_VOLUME_LIGHT, t), mMesh(0), mLPDistance(8.0f),
mSubdivideU(subdivU), mSubdivideV(subdivV), mFootColor(foot), mTailColor(tail), mLightDimensions(Vector3<float>{1.0f, 1.2f, 1.0f})
{

	#ifdef _DEBUG
	//setDebugName("VolumeLightSceneNode");
	#endif

	ConstructLight();
}


VolumeLightNode::~VolumeLightNode()
{

}


void VolumeLightNode::ConstructLight()
{
	const shared_ptr<ScreenElementScene>& pScene = g_pGameApp->GetHumanView()->m_pScene;
	m_Mesh = shared_ptr<IMesh>(
		pScene->GetGeometryCreator()->CreateVolumeLightMesh(
					m_SubdivideU, m_SubdivideV, m_FootColor, 
					m_TailColor, m_LPDistance, m_LightDimensions)
		);
}


//! prerender
bool VolumeLightNode::PreRender(Scene *pScene)
{
	if (Get()->IsVisible())
	{
		// register according to material types counted
		if (!pScene->IsCulled(this))
			pScene->AddToRenderQueue(ERP_TRANSPARENT, shared_from_this());
	}

	return Node::PreRender(pScene);
}

//
// VolumeLightNode::Render
//
bool VolumeLightNode::Render(Scene *pScene)
{
	Matrix4x4<float> toWorld, fromWorld;
	Get()->Transform(&toWorld, &fromWorld);

	const eastl::shared_ptr<Renderer>& renderer = pScene->GetRenderer();
	renderer->SetTransform(ETS_WORLD, toWorld);

	renderer->SetMaterial(mMesh->GetMeshBuffer(0)->GetMaterial());
	renderer->DrawMeshBuffer(mMesh->GetMeshBuffer(0));

	return Node::Render(pScene);
}


//! returns the axis aligned bounding box of this node
const AlignedBox3<float>& VolumeLightNode::GetBoundingBox() const
{
	return mMesh->GetBoundingBox();
}


//! returns the material based on the zero based index i. To get the amount
//! of materials used by this scene node, use getMaterialCount().
//! This function is needed for inserting the node into the scene hirachy on a
//! optimal position for minimizing renderstate changes, but can also be used
//! to directly modify the material of a scene node.
Material& VolumeLightNode::GetMaterial(unsigned int i)
{
	return mMesh->GetMeshBuffer(i)->GetMaterial();
}


//! returns amount of materials used by this scene node.
unsigned int VolumeLightNode::GetMaterialCount() const
{
	return 1;
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

