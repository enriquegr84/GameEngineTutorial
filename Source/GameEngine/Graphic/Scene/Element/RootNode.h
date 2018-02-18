// Copyright (C) 2002-2012 Nikolaus Gebhardt
// This file is part of the "Irrlicht Engine".
// For conditions of distribution and use, see copyright notice in irrlicht.h

#ifndef ROOTNODE_H
#define ROOTNODE_H

#include "Graphic/Scene/Hierarchy/Node.h"

class RootNode : public Node
{
public:

	//! constructor
	RootNode();
	RootNode(const ActorId actorId, PVWUpdater& updater, WeakBaseRenderComponentPtr renderComponent);

	//! Renders event
	bool PreRender(Scene *pScene);
	bool Render(Scene *pScene);
	bool RenderChildren(Scene *pScene);
	bool PostRender(Scene *pScene);

	//! Returns type of the scene node
	NodeType GetType() const { return NT_ROOT; }
};


#endif

