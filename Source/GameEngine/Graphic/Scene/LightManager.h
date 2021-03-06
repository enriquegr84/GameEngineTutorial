// David Eberly, Geometric Tools, Redmond WA 98052
// Copyright (c) 1998-2017
// Distributed under the Boost Software License, Version 1.0.
// http://www.boost.org/LICENSE_1_0.txt
// http://www.geometrictools.com/License/Boost/LICENSE_1_0.txt
// File Version: 3.0.0 (2016/06/19)

#ifndef LIGHTMANAGER_H
#define LIGHTMANAGER_H

#include "GameEngineStd.h"

#include "Graphic/Scene/Hierarchy/Node.h"
#include "Graphic/Scene/Hierarchy/Light.h"

#include "Graphic/Effect/LightingEffect.h"

class Scene;
class CameraNode;

typedef eastl::list<eastl::shared_ptr<Light> > Lights;

/*
	This is completely optional: if you do not register a light manager, then a default
	distance-based scheme will be used to prioritise hardware lights based on their distance
	from the active camera.

	LIGHTS_NEAREST_NODE shows an implementation that turns on a limited number of lights
	per mesh scene node. If finds the nearest light to the node being rendered,
	and turns it on, turning all other lights off.  This works, but as it operates on every
	light for every node, it does not scale well with many lights.  The flickering you can see
	in this demo is due to the lights swapping their relative positions from the cubes
	(a deliberate demonstration of the limitations of this technique).

	LIGHTS_IN_ZONE shows a technique for turning on lights based on a 'zone'. Each empty scene
	node is considered to be the parent of a zone.  When nodes are rendered, they turn off all
	lights, then find their parent 'zone' and turn on all lights that are inside that zone, i.e.
	are  descendents of it in the scene graph.  This produces true 'local' lighting for each cube
	in this example.  You could use a similar technique to locally light all meshes in (e.g.)
	a room, without the lights spilling out to other rooms.
*/
class LightManager
{
	friend class Scene;

public:

	LightManager();

	//! Called after the scene's light list has been built, but before rendering has begun.
	/** As actual device/hardware lights are not created until the
	NRP_LIGHT render pass, this provides an opportunity for the
	light manager to trim or re-order the light list, before any
	device/hardware lights have actually been created.
	\param lightList: the Scene Manager's light list, which
	the light manager may modify. This reference will remain valid
	until OnPostRender().
	*/
	void OnPreRender(eastl::vector<Node*> & lightList);

	//! Called after the last scene node is rendered.
	/** After this call returns, the lightList passed to OnPreRender() becomes invalid. */
	void OnPostRender(void);

	//! Called before a render pass begins
	/** \param renderPass: the render pass that's about to begin */
	void OnRenderPassPreRender(RenderPass renderPass);

	//! Called after the render pass specified in OnRenderPassPreRender() ends
	/** \param[in] renderPass: the render pass that has finished */
	void OnRenderPassPostRender(RenderPass renderPass);

	//! Called before the given scene node is rendered
	/** \param[in] node: the scene node that's about to be rendered */
	void OnNodePreRender(Node* node);

	//! Called after the the node specified in OnNodePreRender() has been rendered
	/** \param[in] node: the scene node that has just been rendered */
	void OnNodePostRender(Node* node);

	void OnNodeLighting(Scene *pScene, Node* node);

	int GetLightCount(const Node *node) { return mLights.size(); }

protected:

	Lights mLights;
	eastl::shared_ptr<Light> mLight;

	typedef enum
	{
		NO_MANAGEMENT,
		LIGHTS_NEAREST_NODE,
		LIGHTS_IN_ZONE
	}
	LightManagementMode;
	LightManagementMode mMode;

	// These data represent the state information that this light manager
	// is interested in.
	eastl::vector<Node*>* mSceneLightList;
	RenderPass mCurrentRenderPass;
	Node * mCurrentSceneNode;

private:

	void UpdateCameraLightModelPositions(Node* object, CameraNode* camera);

	// Find the empty scene node that is the parent of the specified node
	Node* FindZone(Node* node);

	// Turn on all lights that are children (directly or indirectly) of the
	// specified scene node.
	void TurnOnZoneLights(Node* node);


	// A utility class to aid in sorting scene nodes into a distance order
	class LightDistanceElement
	{
	public:
		LightDistanceElement() {};

		LightDistanceElement(Node* n, float d)
			: node(n), distance(d) { }

		Node* node;
		float distance;

		// Lower distance elements are sorted to the start of the array
		bool operator < (const LightDistanceElement& other) const
		{
			return (distance < other.distance);
		}
	};
};

#endif
