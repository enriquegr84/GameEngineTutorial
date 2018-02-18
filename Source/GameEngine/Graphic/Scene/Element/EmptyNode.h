// Copyright (C) 2002-2012 Nikolaus Gebhardt
// This file is part of the "Irrlicht Engine".
// For conditions of distribution and use, see copyright notice in irrlicht.h

#ifndef EMPTYNODE_H
#define EMPTYNODE_H

#include "Graphic/Scene/Hierarchy/Node.h"

class EmptyNode : public Node
{
public:

	//! constructor
	EmptyNode(const ActorId actorId, PVWUpdater& updater, WeakBaseRenderComponentPtr renderComponent);

	//! Renders event
	bool PreRender(Scene *pScene);
	bool Render(Scene *pScene);

	//! Returns type of the scene node
	NodeType GetType() const { return NT_EMPTY; }
};


#endif

