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

		if (pScene->GetLightManager())
			pScene->GetLightManager()->OnRenderPassPreRender((RenderPass)pass);

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
