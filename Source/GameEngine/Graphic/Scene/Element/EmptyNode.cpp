// Copyright (C) 2002-2012 Nikolaus Gebhardt
// This file is part of the "Irrlicht Engine".
// For conditions of distribution and use, see copyright notice in irrlicht.h

#include "EmptyNode.h"

#include "Graphic/Scene/Hierarchy/Node.h"
#include "Graphic/Scene/Scene.h"

//! constructor
EmptyNode::EmptyNode(const ActorId actorId, PVWUpdater& updater, 
	WeakBaseRenderComponentPtr renderComponent)
:	Node(actorId, renderComponent, RP_TRANSPARENT, NT_UNKNOWN)
{
	mPVWUpdater = updater;
	SetAutomaticCulling(AC_OFF);
}


//! prerender
bool EmptyNode::PreRender(Scene *pScene)
{
	if (IsVisible())
	{
		// because this node supports rendering of mixed mode meshes consisting of
		// transparent and solid material at the same time, we need to go through all
		// materials, check of what type they are and register this node for the right
		// render pass according to that.
		int transparentCount = 0;
		int solidCount = 0;

		// count transparent and solid materials in this scene node
		if (GetMaterial().IsTransparent())
			++transparentCount;
		else
			++solidCount;

		// register according to material types counted
		if (!pScene->IsCulled(this))
		{
			if (solidCount)
				pScene->AddToRenderQueue(RP_SOLID, shared_from_this());

			if (transparentCount)
				pScene->AddToRenderQueue(RP_TRANSPARENT, shared_from_this());
		}
	}

	return Node::PreRender(pScene);
}


//! render
bool EmptyNode::Render(Scene *pScene)
{
	// do nothing
	return Node::Render(pScene);
}