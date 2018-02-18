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

class Scene;

typedef eastl::list<eastl::shared_ptr<Light> > Lights;

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

	void UpdateLighting(Scene *pScene);
	void UpdateLighting(Lighting* pLighting, Node *pNode);
	int GetLightCount(const Node *node) { return mLights.size(); }

	const Vector4<float>* GetLightAmbient(const Node *node) { return &mLightAmbient; }
	const Vector4<float>* GetLightDirection(const Node *node) { return mLightDir; }
	const eastl::array<float, 4>* GetLightDiffuse(const Node *node) { return mLightDiffuse; }

protected:
	/*
	void CreateScene();
	void UseLightType(int type);
	void UpdateConstants();

	eastl::shared_ptr<RasterizerState> mWireState;
	*/
	eastl::shared_ptr<Node> mDLightRoot;
	eastl::shared_ptr<Light> mDLight;

	enum { LDIR, LPNT, LSPT, LNUM };
	enum { GPLN, GSPH, GNUM };
	enum { SVTX, SPXL, SNUM };
	eastl::shared_ptr<LightingEffect> mEffect[LNUM][GNUM][SNUM];
	eastl::shared_ptr<Visual> mPlane[SNUM], mSphere[SNUM];
	Vector4<float> mLightWorldPosition[2], mLightWorldDirection;
	eastl::string mCaption[LNUM];
	int mType;

	Lights mLights;

	Vector4<float> mLightDir[8];
	eastl::array<float, 4> mLightDiffuse[8];
	Vector4<float> mLightAmbient;

	typedef enum
	{
		NO_MANAGEMENT,
		LIGHTS_NEAREST_NODE,
		LIGHTS_IN_ZONE
	}
	LightManagementMode;

	LightManagementMode mMode;
	LightManagementMode mRequestedMode;

	// These data represent the state information that this light manager
	// is interested in.
	eastl::vector<Node*>* mSceneLightList;
	RenderPass mCurrentRenderPass;
	Node * mCurrentSceneNode;

private:

	void UpdateCameraLightModelPositions(
		eastl::shared_ptr<Node> object, eastl::shared_ptr<Camera> camera);

	// Find the empty scene node that is the parent of the specified node
	Node * FindZone(Node* node)
	{
		if (!node)
			return 0;

		if (node->GetType() == NT_EMPTY)
			return node;

		return FindZone((Node*)node->GetParent());
	}

	// Turn on all lights that are children (directly or indirectly) of the
	// specified scene node.
	void TurnOnZoneLights(Node * node)
	{
		SceneNodeList children = node->GetChildren();
		for (SceneNodeList::const_iterator child = children.begin(); child != children.end(); ++child)
		{
			if ((*child)->GetType() == NT_LIGHT)
				(*child)->SetVisible(true);
			else // Assume that lights don't have any children that are also lights
				TurnOnZoneLights((*child).get());
		}
	}


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
