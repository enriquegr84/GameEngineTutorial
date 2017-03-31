//========================================================================
// File: Scene.cpp - implements the container class for 3D Graphics scenes
//
// Part of the GameEngine Application
//
// GameEngine is the sample application that encapsulates much of the source code
// discussed in "Game Coding Complete - 4th Edition" by Mike McShaffry and David
// "Rez" Graham, published by Charles River Media. 
// ISBN-10: 1133776574 | ISBN-13: 978-1133776574
//
// If this source code has found it's way to you, and you think it has helped you
// in any way, do the authors a favor and buy a new copy of the book - there are 
// detailed explanations in it that compliment this code well. Buy a copy at Amazon.com
// by clicking here: 
//    http://www.amazon.com/gp/product/1133776574/ref=olp_product_details?ie=UTF8&me=&seller=
//
// There's a companion web site at http://www.mcshaffry.com/GameCode/
// 
// The source code is managed and maintained through Google Code: 
//    http://code.google.com/p/GameEngine/
//
// (c) Copyright 2012 Michael L. McShaffry and David Graham
//
// This program is free software; you can redistribute it and/or
// modify it under the terms of the GNU Lesser GPL v3
// as published by the Free Software Foundation.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See 
// http://www.gnu.org/licenses/lgpl-3.0.txt for more details.
//
// You should have received a copy of the GNU Lesser GPL v3
// along with this program; if not, write to the Free Software
// Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
//
//========================================================================


#include "Scene.h"

#include "Core/OS/Os.h"

//#include "EventManager/EventManager.h"
//#include "EventManager/Events.h"

//#include "Lights.h"


////////////////////////////////////////////////////
// Scene Implementation
////////////////////////////////////////////////////


//
// Scene::Scene						- Chapter 16, page 539
//
//    Note: The shared_ptr<IRenderer> was added to allow for both D3D9 and D3D11 renderer implementations.
//          The book only describes D3D11, so to find all the differences, just search for m_Renderer!
//
Scene::Scene()
	: m_ShadowColor{ 150,0,0,0 }, m_AmbientLight{ 0, 0, 0, 0 }
{
	//m_pRoot.reset(new RootNode());
	m_CurrentRenderPass = ERP_NONE;

	// [mrmike] - event delegates were added post-press
	/*
    BaseEventManager* pEventMgr = BaseEventManager::Get();
    pEventMgr->AddListener(MakeDelegate(this, &NewRenderComponentDelegate), EvtData_New_Render_Component::sk_EventType);
    pEventMgr->AddListener(MakeDelegate(this, &DestroyActorDelegate), EvtData_Destroy_Actor::sk_EventType);
    pEventMgr->AddListener(MakeDelegate(this, &MoveActorDelegate), EvtData_Move_Actor::sk_EventType);
    pEventMgr->AddListener(MakeDelegate(this, &ModifiedRenderComponentDelegate), EvtData_Modified_Render_Component::sk_EventType);
	*/
}

//
// ~Scene::Scene					- Chapter 16, page 539
//
Scene::~Scene()
{
	ClearRenderList();
	ClearDeletionList();

	//! force to remove hardwareTextures from the driver
	//! because Scenes may hold internally data bounded to sceneNodes
	//! which may be destroyed twice
	/*
	if (m_pRenderer)
		m_pRenderer->RemoveAllHardwareBuffers();
	*/

	// remove all nodes and animators before dropping the driver
	// as render targets may be destroyed twice

	RemoveAll();
	//RemoveAnimators();

	// [mrmike] - event delegates were added post-press!
	/*
    IEventManager* pEventMgr = IEventManager::Get();
    pEventMgr->RemoveListener(MakeDelegate(this, &NewRenderComponentDelegate), EvtData_New_Render_Component::sk_EventType);
    pEventMgr->RemoveListener(MakeDelegate(this, &DestroyActorDelegate), EvtData_Destroy_Actor::sk_EventType);
    pEventMgr->RemoveListener(MakeDelegate(this, &MoveActorDelegate), EvtData_Move_Actor::sk_EventType);

    pEventMgr->RemoveListener(MakeDelegate(this, &ModifiedRenderComponentDelegate), EvtData_Modified_Render_Component::sk_EventType);

	//SAFE_RELEASE(m_MatrixStack);
	//SAFE_DELETE(m_LightManager);
	*/
}

//
// Scene::OnUpdate					- Chapter 16, page 540
//
bool Scene::OnAnimate(unsigned int uTime)
{
	if (!m_pRoot)
		return true;

	return true;
	//m_pRoot->OnAnimate(this, uTime);
}

//
// OnUpdate					- Chapter 16, page 540
//
bool Scene::OnUpdate(unsigned long elapsedTime)
{
	if (!m_pRoot)
		return true;

	return true;
	//m_pRoot->OnUpdate(this, elapsedTime);
}


//
// Scene::OnRender					- Chapter 16, page 539
//
bool Scene::OnRender()
{
	// The render passes usually go like this 
	// 1. Static objects & terrain
	// 2. Actors (dynamic objects that can move)
	// 3. The Sky
	// 4. Anything with Alpha
	
	if (m_pRoot && m_pCamera)
	{
		// The scene root could be anything, but it
		// is usually a SceneNode with the identity
		// matrix
		/*
		m_pCamera->SetViewTransform(this);

		//m_LightManager->CalcLighting(this);

		if (m_pRoot->PreRender(this)==true)
		{
			if (m_pLightManager)
				m_pLightManager->OnPreRender(m_RenderList[ERP_LIGHT]);

			m_pRoot->Render(this);
			m_pRoot->PostRender(this);
			
			if (m_pLightManager)
				m_pLightManager->OnPostRender();
		}
		*/
		RenderAlphaPass();
	}
	
	return true;
}	

//
// Scene::OnLostDevice						- not in the book
//
//    All Scene nodes implement VOnLostDevice, which is called in the D3D9 renderer.
//
bool Scene::OnLostDevice()
{
	if (m_pRoot)
	{
		return true;
		//m_pRoot->OnLostDevice(this);
	}
	return true;
}

//
// Scene::OnRestore					- Chapter 16, page 540
//
bool Scene::OnRestore()
{
	if (!m_pRoot)
		return true;

	//HRESULT hr;
	//V_RETURN(m_Renderer->OnRestore());

	return true;
	//m_pRoot->OnRestore(this);
}


//! Adds a scene node to the render queue.
void Scene::AddToRenderQueue(E_RENDER_PASS pass, const eastl::shared_ptr<SceneNode>& node)
{
	if (!node)
		return;

	m_RenderList[pass].push_back(node.get());
}


//! clears the render list
void Scene::ClearRenderList()
{
	for (int pass=0; pass < ERP_LAST; pass++)
		m_RenderList[pass].clear();
}

//! Adds a scene node to the deletion queue.
void Scene::AddToDeletionQueue(SceneNode* node)
{
	if (!node)
		return;

	m_DeletionList.push_back(node);
}


//! clears the deletion list
void Scene::ClearDeletionList()
{
	if (m_DeletionList.empty())
		return;

	m_DeletionList.clear();
}


//! Removes all children of this scene node
void Scene::RemoveAll()
{
	m_SceneNodeActors.clear();
	SetActiveCamera(0);
	// Make sure the driver is reset, might need a more complex method at some point
	/*
	if (m_pRenderer)
		m_pRenderer->SetMaterial(Material());
	*/
}


//! Clears the whole scene. All scene nodes are removed.
void Scene::Clear()
{
	RemoveAll();
}

//! Adds a child to this scene node.
/** If the scene node already has a parent it is first removed
from the other parent.
\param child A pointer to the new child. */
bool Scene::AddSceneNode(ActorId id, const eastl::shared_ptr<SceneNode>& node)
{ 
	if (id != INVALID_ACTOR_ID)
	{
		// This allows us to search for this later based on actor id
		m_SceneNodeActors[id] = node;	
	}
	/*
	shared_ptr<LightNode> pLight = dynamic_pointer_cast<LightNode>(kid);
	if (pLight != NULL && m_LightManager->m_Lights.size()+1 < MAXIMUM_LIGHTS_SUPPORTED)
	{
		m_LightManager->m_Lights.push_back(pLight);
	}
	*/
	return true;
	//m_pRoot->AddChild(node);
}

//! Removes a child from this scene node.
/** If found in the children list, the child pointer is also
dropped and might be deleted if no other grab exists.
\param child A pointer to the child which shall be removed.
\return True if the child was removed, and false if not,
e.g. because it couldn't be found in the children list. */
bool Scene::RemoveSceneNode(ActorId id)
{
	if (id == INVALID_ACTOR_ID)
		return false;
	/*
	shared_ptr<SceneNode> node = GetSceneNode(id);

	shared_ptr<LightNode> pLight = dynamic_pointer_cast<LightNode>(kid);
	if (pLight != NULL)
	{
		m_LightManager->m_Lights.remove(pLight);
	}
	*/
	m_SceneNodeActors.erase(id);
	return true;
	//m_pRoot->RemoveChild(id);
}

//! Returns the first scene node with the specified id.
eastl::shared_ptr<SceneNode> Scene::GetSceneNode(ActorId id)
{
	SceneNodeActorMap::iterator i = m_SceneNodeActors.find(id);
	if (i==m_SceneNodeActors.end())
	{
		return 0;
	}

	return i->second;
}

/*
void Scene::NewRenderComponentDelegate(BaseEventDataPtr pEventData)
{
    shared_ptr<EvtData_New_Render_Component> pCastEventData = 
		static_pointer_cast<EvtData_New_Render_Component>(pEventData);

    ActorId actorId = pCastEventData->GetActorId();
    shared_ptr<SceneNode> pSceneNode(pCastEventData->GetSceneNode());

    // FUTURE WORK: Add better error handling here.		
    if (FAILED(pSceneNode->OnRestore(this)))
    {
        GE_ERROR(eastl::string("Failed to restore scene node to the scene for actorid ")
				+ eastl::string(actorId));
        return;
    }

    AddChild(actorId, pSceneNode);
}

void Scene::ModifiedRenderComponentDelegate(BaseEventDataPtr pEventData)
{
    shared_ptr<EvtData_Modified_Render_Component> pCastEventData = 
		static_pointer_cast<EvtData_Modified_Render_Component>(pEventData);

    ActorId actorId = pCastEventData->GetActorId();
	if (actorId == INVALID_ACTOR_ID)
	{
		GE_ERROR("ModifiedRenderComponentDelegate - unknown actor id!");
		return;
	}

	if (g_pGameApp->GetGameLogic()->GetState()==BGS_LoadingGameEnvironment)
		return;

	shared_ptr<SceneNode> pSceneNode = FindActor(actorId);
    // FUTURE WORK: Add better error handling here.		
    if (!pSceneNode  || FAILED(pSceneNode->OnRestore(this)))
    {
        GE_ERROR(eastl::string("Failed to restore scene node to the scene for actorid ") 
				+ eastl::string(actorId));
    }
}

void Scene::DestroyActorDelegate(BaseEventDataPtr pEventData)
{
    shared_ptr<EvtData_Destroy_Actor> pCastEventData = 
		static_pointer_cast<EvtData_Destroy_Actor>(pEventData);
    RemoveChild(pCastEventData->GetId());
}

void Scene::MoveActorDelegate(BaseEventDataPtr pEventData)
{
    shared_ptr<EvtData_Move_Actor> pCastEventData = 
		static_pointer_cast<EvtData_Move_Actor>(pEventData);

    ActorId id = pCastEventData->GetId();
    matrix4 transform = pCastEventData->GetMatrix();

    shared_ptr<SceneNode> pNode = FindActor(id);
    if (pNode)
    {
        pNode->SetTransform(&transform);
    }
}
*/

//
// Scene::RenderAlphaPass			- Chapter 16, page 543
//
void Scene::RenderAlphaPass()
{
/*
	shared_ptr<IRenderState> alphaPass = m_Renderer->VPrepareAlphaPass();

	m_AlphaSceneNodes.sort();
	while (!m_AlphaSceneNodes.empty())
	{
		AlphaSceneNodes::reverse_iterator i = m_AlphaSceneNodes.rbegin();
		PushAndSetMatrix((*i)->m_Concat);
		(*i)->m_pNode->VRender(this);
		delete (*i);
		PopMatrix();
		m_AlphaSceneNodes.pop_back();
	}
*/
}


//
// Scene::IsCulled				- Chapter 16, page 533
//	
bool Scene::IsCulled(SceneNode* node)
{
	return true;
	/*
	if (!node->Get()->IsVisible())
		return true;
	
	// transform the location of this node into the camera space 
	// of the camera attached to the scene

	matrix4 toWorld, fromWorld;
	GetActiveCamera()->Get()->Transform(&toWorld, &fromWorld);
	ViewFrustum frustum(GetActiveCamera()->GetViewFrustum());

	bool isVisible = false;

	// has occlusion query information
	if (node->Get()->GetAutomaticCulling() & EAC_OCC_QUERY)
	{
		isVisible = 
			(m_pRenderer->GetOcclusionQueryResult(GetSceneNode(node->Get()->ActorId()))==0);
	}

	// can be seen by a bounding box ?
	if (!isVisible && (node->Get()->GetAutomaticCulling() & EAC_BOX))
	{
		AABBox3<f32> tbox = node->GetBoundingBox();
		toWorld.TransformBoxEx(tbox);
		isVisible = !(tbox.IntersectsWithBox(frustum.GetBoundingBox()));
	}

	// can be seen by a bounding sphere
	if (!isVisible && (node->Get()->GetAutomaticCulling() & EAC_FRUSTUM_SPHERE))
	{ // requires bbox diameter
	}

	// can be seen by cam pyramid planes ?
	if (!isVisible && (node->Get()->GetAutomaticCulling() & EAC_FRUSTUM_BOX))
	{
		//transform the frustum to the node's current absolute transformation
		matrix4 invTrans(toWorld, matrix4::EM4CONST_INVERSE);
		//invTrans.makeInverse();
		frustum.Transform(invTrans);

		Vector3f edges[8];
		node->GetBoundingBox().GetEdges(edges);

		for (s32 i=0; i< ViewFrustum::VF_PLANE_COUNT; ++i)
		{
			bool boxInFrustum=false;
			for (u32 j=0; j<8; ++j)
			{
				if (frustum.Planes[i].ClassifyPointRelation(edges[j]) != ISREL3D_FRONT)
				{
					boxInFrustum=true;
					break;
				}
			}

			if (!boxInFrustum)
			{
				isVisible = true;
				break;
			}
		}
	}
	return isVisible;
	*/
}



