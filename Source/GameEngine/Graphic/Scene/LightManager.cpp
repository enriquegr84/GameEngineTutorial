// David Eberly, Geometric Tools, Redmond WA 98052
// Copyright (c) 1998-2017
// Distributed under the Boost Software License, Version 1.0.
// http://www.boost.org/LICENSE_1_0.txt
// http://www.geometrictools.com/License/Boost/LICENSE_1_0.txt
// File Version: 3.0.0 (2016/06/19)

#include "LightManager.h"

#include "Graphic/Scene/Element/CameraNode.h"

#include "Application/GameApplication.h"

LightManager::LightManager()
	:	mMode(NO_MANAGEMENT), mSceneLightList(0), mCurrentRenderPass(RP_NONE), mCurrentSceneNode(0)
{ 
#if defined(_OPENGL_)
	mLight = eastl::make_shared<Light>(true, false);
#else
	mLight = eastl::make_shared<Light>(true, true);
#endif
	mLight->mLighting = eastl::make_shared<Lighting>();
	mLight->mLighting->mAmbient = Renderer::Get()->GetClearColor();
	mLight->mLighting->mAttenuation = { 1.0f, 0.0f, 0.0f, 1.0f };
}

void LightManager::UpdateCameraLightModelPositions(Node* node, CameraNode* cameraNode)
{
	if (node->IsVisible())
	{
		for (unsigned int i = 0; i < node->GetVisualCount(); ++i)
		{
			eastl::shared_ptr<Visual> visual = node->GetVisual(i);

			if (visual)
			{
				eastl::shared_ptr<LightingEffect> ltEffect = 
					eastl::dynamic_shared_pointer_cast<LightingEffect>(visual->GetEffect());
				if (ltEffect)
				{
					ltEffect->SetLighting(mLight->mLighting);
					ltEffect->UpdateLightingConstant();

					ltEffect->SetGeometry(eastl::make_shared<LightCameraGeometry>());
					ltEffect->UpdateGeometryConstant();

					for (unsigned int i = 0; i < mSceneLightList->size(); i++)
					{
						LightNode* lightNode = dynamic_cast<LightNode*>((*mSceneLightList)[i]);
						if (!lightNode->IsVisible()) continue;

						Vector4<float> cameraPosition = cameraNode->GetAbsoluteTransform().GetTranslationW1();
						Matrix4x4<float> cameraRotation = cameraNode->GetAbsoluteTransform().GetRotation();

						// The pvw-matrices are updated automatically whenever the camera moves
						// or is rotated.  Here we need to update the camera model position, 
						// light model position, and light model direction.
						Matrix4x4<float> invWMatrix = node->GetAbsoluteTransform().GetHInverse();

						Vector4<float> lightPosition = lightNode->GetAbsoluteTransform().GetTranslationW1();
						Vector4<float> lightDirection = HLift(lightNode->GetLight()->mLighting->mDirection, 0.f);
						Normalize(lightDirection);

						auto geometry = ltEffect->GetGeometry();
#if defined(GE_USE_MAT_VEC)
						geometry->lightModelPosition = invWMatrix * lightPosition;
						geometry->lightModelDirection = invWMatrix * lightDirection;
						geometry->cameraModelPosition = invWMatrix * cameraPosition;
#else
						geometry->lightModelPosition = lightPosition * invWMatrix;
						geometry->lightModelDirection = lightDirection * invWMatrix;
						geometry->cameraModelPosition = cameraPosition * invWMatrix;
#endif
						Normalize(geometry->lightModelDirection);
						ltEffect->UpdateGeometryConstant();

						eastl::shared_ptr<Lighting> lighting = ltEffect->GetLighting();
						lighting->mAttenuation = lightNode->GetLight()->mLighting->mAttenuation;
						lighting->mSpotCutoff = lightNode->GetLight()->mLighting->mSpotCutoff;
						ltEffect->SetLighting(lighting);
						ltEffect->UpdateLightingConstant();
						break;
					}
				}
			}
		}
	}
}

//
// LightManager::OnNodeLighting
//
void LightManager::OnNodeLighting(Scene *pScene, Node* node)
{
	UpdateCameraLightModelPositions(node, pScene->GetActiveCamera().get());
}

// This is called before the first scene node is rendered.
void LightManager::OnPreRender(eastl::vector<Node*> & lightList)
{
	// Store the light list. I am free to alter this list until the end of OnPostRender().
	mSceneLightList = &lightList;
}

// Called after the last scene node is rendered.
void LightManager::OnPostRender()
{
	// Since light management might be switched off in the event handler, we'll turn all
	// lights on to ensure that they are in a consistent state. You wouldn't normally have
	// to do this when using a light manager, since you'd continue to do light management
	// yourself.
	for (unsigned int i = 0; i < mSceneLightList->size(); i++)
	{
		Node* lightNode = (*mSceneLightList)[i];
		lightNode->SetVisible(true);
	}
}

void LightManager::OnRenderPassPreRender(RenderPass renderPass)
{
	// I don't have to do anything here except remember which render pass I am in.
	mCurrentRenderPass = renderPass;
}

void LightManager::OnRenderPassPostRender(RenderPass renderPass)
{
	// I only want solid nodes to be lit, so after the light pass, turn all lights off.
	if (RP_LIGHT == renderPass)
	{
		for (unsigned int i = 0; i < mSceneLightList->size(); ++i)
		{
			Node* lightNode = (*mSceneLightList)[i];
			lightNode->SetVisible(false);
		}
	}
}

// This is called before the specified scene node is rendered
void LightManager::OnNodePreRender(Node* node)
{
	mCurrentSceneNode = node;

	// This light manager only considers solid objects, but you are free to manipulate
	// lights during any phase, depending on your requirements.
	if (RP_SOLID != mCurrentRenderPass)
		return;

	// And in fact for this example, I only want to consider lighting for cube scene
	// nodes. You will probably want to deal with lighting for (at least) mesh /
	// animated mesh scene nodes as well.
	if (node->GetType() != NT_CUBE)
		return;

	if (LIGHTS_NEAREST_NODE == mMode)
	{
		// This is a naive implementation that prioritises every light in the scene
		// by its proximity to the node being rendered.  This produces some flickering
		// when lights orbit closer to a cube than its 'zone' lights.

		const Vector3<float> nodePosition = node->GetAbsoluteTransform().GetTranslation();

		// Sort the light list by prioritising them based on their distance from the node
		// that's about to be rendered.
		eastl::vector<LightDistanceElement> sortingArray(mSceneLightList->size());

		unsigned int i;
		for (i = 0; i < mSceneLightList->size(); ++i)
		{
			Node* lightNode = (*mSceneLightList)[i];

			const float distance = Length(
				lightNode->GetAbsoluteTransform().GetTranslation() - nodePosition);
			sortingArray.push_back(LightDistanceElement(lightNode, distance));
		}

		eastl::sort(sortingArray.begin(), sortingArray.end());

		// The list is now sorted by proximity to the node.
		// Turn on the nearest light, and turn the others off.
		for (i = 0; i < sortingArray.size(); ++i)
			sortingArray[i].node->SetVisible(i < 1);
	}
	else if (LIGHTS_IN_ZONE == mMode)
	{
		// Empty scene nodes are used to represent 'zones'. For each solid mesh that
		// is being rendered, turn off all lights, then find its 'zone' parent, and turn
		// on all lights that are found under that node in the scene graph.
		// This is a general purpose algorithm that doesn't use any special
		// knowledge of how this particular scene graph is organised.
		for (unsigned int i = 0; i < mSceneLightList->size(); ++i)
		{
			if ((*mSceneLightList)[i]->GetType() != NT_LIGHT)
				continue;
			LightNode* lightNode = static_cast<LightNode*>((*mSceneLightList)[i]);

			if (LT_DIRECTIONAL != lightNode->GetLightType())
				lightNode->SetVisible(false);
		}

		Node * parentZone = FindZone(node);
		if (parentZone)
			TurnOnZoneLights(parentZone);
	}
}

// Called after the specified scene node is rendered
void LightManager::OnNodePostRender(Node* node)
{
	// any light management after individual node rendering.
}

// Find the empty scene node that is the parent of the specified node
Node* LightManager::FindZone(Node* node)
{
	if (!node)
		return nullptr;

	if (node->GetType() == NT_EMPTY)
		return node;

	return FindZone((Node*)node->GetParent());
}

// Turn on all lights that are children (directly or indirectly) of the
// specified scene node.
void LightManager::TurnOnZoneLights(Node * node)
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
//----------------------------------------------------------------------------
