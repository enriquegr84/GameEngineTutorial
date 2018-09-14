// Copyright (C) 2002-2012 Nikolaus Gebhardt
// This file is part of the "Irrlicht Engine".
// For conditions of distribution and use, see copyright notice in irrlicht.h

#ifndef LIGHTNODE_H
#define LIGHTNODE_H

#include "Graphic/Scene/Hierarchy/Node.h"
#include "Graphic/Scene/Hierarchy/Light.h"

#include "Graphic/Scene/Element/ViewVolumeNode.h"

//! Scene node which is a dynamic light. You can switch the light on and off by
//! making it visible or not, and let it be animated by ordinary scene node animators.
class LightNode : public Node
{
public:	

	//! Constructor
	LightNode(const ActorId actorId, PVWUpdater* updater, 
		WeakBaseRenderComponentPtr renderComponent, const eastl::shared_ptr<Light>& light);

	//! Renders event
	virtual bool PreRender(Scene *pScene);
	virtual bool Render(Scene *pScene);

	//! \return Returns the light data.
	virtual const eastl::shared_ptr<Lighting>& GetLightData() const;

	//! Sets if the node should be visible or not.
	/** All children of this node won't be visible either, when set
	to true.
	\param isVisible If the node shall be visible. */
	void SetVisible(bool isVisible);

private:

	void DoLightRecalc();

	eastl::shared_ptr<ViewVolumeNode> mLightVolume;
	eastl::shared_ptr<Visual> mVisual;
	int mDriverLightIndex;
	bool mLightIsOn;
};

#endif

