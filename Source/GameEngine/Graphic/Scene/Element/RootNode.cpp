// Copyright (C) 2002-2012 Nikolaus Gebhardt
// This file is part of the "Irrlicht Engine".
// For conditions of distribution and use, see copyright notice in irrlicht.h

#include "RootNode.h"

#include "Graphic/Scene/Hierarchy/Node.h"
#include "Graphic/Scene/Scene.h"

#include "Graphic/Renderer/Renderer.h"
#include "Graphic/Effect/Material.h"

////////////////////////////////////////////////////
// RootNode Implementation
////////////////////////////////////////////////////

//
// RootNode::RootNode					- Chapter 16, page 545
//
RootNode::RootNode()
	: Node(INVALID_ACTOR_ID, WeakBaseRenderComponentPtr(), NT_ROOT)
{

}

//
// RootNode::RootNode					- Chapter 16, page 545
//
RootNode::RootNode(const ActorId actorId, PVWUpdater* updater, 
	WeakBaseRenderComponentPtr renderComponent)
	: Node(actorId, renderComponent, NT_ROOT)
{
	mPVWUpdater = updater;
}

//
// RootNode::PreRender
//
bool RootNode::PreRender(Scene *pScene)
{
	// reset all transforms
	/*
	Renderer::Get()->SetMaterial(Material());
	Renderer::Get()->SetTransform(TS_PROJECTION, Matrix4x4<float>::Identity());
	Renderer::Get()->SetTransform(TS_VIEW, Matrix4x4<float>::Identity());
	Renderer::Get()->SetTransform(TS_WORLD, Matrix4x4<float>::Identity());
	for (unsigned int i = TS_COUNT - 1; i >= TS_TEXTURE_0; --i)
		renderer->SetTransform((TRANSFORMATION_STATE)i, Matrix4x4<float>::Identity());
	Renderer::Get()->SetAllowZWriteOnTransparent(true);

	// first scene node for prerendering should be the active camera
	// consistent Camera is needed for culling
	if (pScene->GetActiveCamera())
		pScene->GetActiveCamera()->Render(pScene);
	*/
	bool success = Node::PreRender(pScene);

	if (success)
		Node::SortRenderList(pScene);

	return success;
}

//
// RootNode::RenderChildren					- Chapter 16, page 547
//
bool RootNode::RenderChildren(Scene *pScene)
{
	// Iterate through the render children....
	for (int pass = 0; pass < RP_LAST; pass++)
	{
		pScene->SetCurrentRenderPass((RenderPass)pass);

		SceneNodeRenderList::iterator itNode = pScene->GetRenderList(pass).begin();
		SceneNodeRenderList::iterator end = pScene->GetRenderList(pass).end();
		/*
		Renderer::Get()->GetOverrideMaterial().mEnabled =
			((Renderer::Get()->GetOverrideMaterial().mEnablePasses & pass) != 0);
		*/
		if (pScene->GetLightManager())
			pScene->GetLightManager()->OnRenderPassPreRender((RenderPass)pass);
		/*
		if (pass == RP_LIGHT)
		{
			Renderer::Get()->DeleteAllDynamicLights();
			Renderer::Get()->SetAmbientLight(pScene->GetAmbientLight());

			unsigned int maxLights = pScene->GetRenderList(pass).size();
			if (!pScene->GetLightManager())
				end = itNode +
				eastl::min(Renderer::Get()->GetMaximalDynamicLightAmount(), maxLights);
		}
		*/
		// This code creates fine control of the render passes.
		for (; itNode != end; ++itNode)
		{
			if (pScene->GetLightManager())
			{
				pScene->GetLightManager()->OnNodePreRender((*itNode));
				pScene->GetLightManager()->OnNodeLighting(pScene, (*itNode));
			}

			(*itNode)->Render(pScene);

			if (pScene->GetLightManager())
				pScene->GetLightManager()->OnNodePostRender((*itNode));

		}
		/*
		if (pass == RP_SHADOW)
		{
			if (!pScene->GetRenderList(pass).empty())
				Renderer::Get()->DrawStencilShadow(true,
					pScene->GetShadowColor(), pScene->GetShadowColor(),
					pScene->GetShadowColor(), pScene->GetShadowColor());
		}
		*/
		if (pScene->GetLightManager())
			pScene->GetLightManager()->OnRenderPassPostRender((RenderPass)pass);
	}
	return true;
}

//
// RootNode::Render
//
bool RootNode::Render(Scene* pScene)
{
	return RenderChildren(pScene);
}

//
// RootNode::Render
//
bool RootNode::PostRender(Scene* pScene)
{
	pScene->ClearRenderList();
	pScene->ClearDeletionList();

	pScene->SetCurrentRenderPass(RP_NONE);

	return Node::PostRender(pScene);
}
