// Copyright (C) 2002-2012 Nikolaus Gebhardt
// This file is part of the "Irrlicht Engine".
// For conditions of distribution and use, see copyright notice in irrlicht.h

#ifndef LIGHTNODE_H
#define LIGHTNODE_H

#include "Graphic/Scene/Hierarchy/Node.h"

//! Scene node which is a dynamic light. You can switch the light on and off by
//! making it visible or not, and let it be animated by ordinary scene node animators.
class LightNode : public Node
{
public:	

	//! Constructor
	LightNode(const ActorId actorId, WeakBaseRenderComponentPtr renderComponent,
		eastl::array<float, 4> color, float radius);

	//! Renders event
	bool PreRender(Scene *pScene);
	bool Render(Scene *pScene);

	//! set node light data from light info
	void SetLightData(const Light& light);

	//! \return Returns the light data.
	const Light& GetLightData() const;

	//! \return Returns the light data.
	Light& GetLightData();

	//! Sets if the node should be visible or not.
	/** All children of this node won't be visible either, when set
	to true.
	\param isVisible If the node shall be visible. */
	void SetVisible(bool isVisible);

	//! returns the axis aligned bounding box of this node
	const AlignedBox3<float>& GetBoundingBox() const;

	//! Returns type of the scene node
	NodeType GetType() const { return ESNT_LIGHT; }

	//! Sets the light's radius of influence.
	/** Outside this radius the light won't lighten geometry and cast no
	shadows. Setting the radius will also influence the attenuation, setting
	it to (0,1/radius,0). If you want to override this behavior, set the
	attenuation after the radius.
	\param radius The new radius. */
	void SetRadius(float radius);

	//! Gets the light's radius of influence.
	/** \return The current radius. */
	float GetRadius() const;

	//! Sets the light type.
	/** \param type The new type. */
	void SetLightType(E_LIGHT_TYPE type);

	//! Gets the light type.
	/** \return The current light type. */
	E_LIGHT_TYPE GetLightType() const;

	//! Sets whether this light casts shadows.
	/** Enabling this flag won't automatically cast shadows, the meshes
	will still need shadow scene nodes attached. But one can enable or
	disable distinct lights for shadow casting for performance reasons.
	\param shadow True if this light shall cast shadows. */
	void EnableCastShadow(bool shadow=true);

	//! Check whether this light casts shadows.
	/** \return True if light would cast shadows, else false. */
	bool GetCastShadow() const;

private:

	void DoLightRecalc();

	Light mLightData;
	AlignedBox3<float> mBBox;
	int mDriverLightIndex;
	bool mLightIsOn;
};

#endif

