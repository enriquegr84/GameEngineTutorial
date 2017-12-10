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

#include "Core/Event/EventManager.h"
#include "Core/Event/Event.h"

#include "LightManager.h"


////////////////////////////////////////////////////
// Scene Implementation
////////////////////////////////////////////////////


//
// Scene::Scene						- Chapter 16, page 539
//
//    Note: The shared_ptr<IRenderer> was added to allow for both D3D9 and D3D11 renderer implementations.
//          The book only describes D3D11, so to find all the differences, just search for mRenderer!
//
Scene::Scene()
	: mShadowColor{ 150,0,0,0 }, mAmbientLight{ 0, 0, 0, 0 }
{
	mRoot.reset(new Node());
	mCurrentRenderPass = ERP_NONE;
	mLightManager = new LightManager();

	// [mrmike] - event delegates were added post-press
    BaseEventManager* pEventMgr = BaseEventManager::Get();
    pEventMgr->AddListener(MakeDelegate(this, &Scene::NewRenderComponentDelegate), EventDataNewRenderComponent::skEventType);
    pEventMgr->AddListener(MakeDelegate(this, &Scene::DestroyActorDelegate), EventDataDestroyActor::skEventType);
    pEventMgr->AddListener(MakeDelegate(this, &Scene::MoveActorDelegate), EventDataMoveActor::skEventType);
    pEventMgr->AddListener(MakeDelegate(this, &Scene::ModifiedRenderComponentDelegate), EventDataModifiedRenderComponent::skEventType);
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
	if (mRenderer)
		mRenderer->RemoveAllHardwareBuffers();

	// remove all nodes and animators before dropping the driver
	// as render targets may be destroyed twice

	RemoveAll();
	RemoveAnimators();

	// [mrmike] - event delegates were added post-press!
    BaseEventManager* pEventMgr = BaseEventManager::Get();
    pEventMgr->RemoveListener(MakeDelegate(this, &Scene::NewRenderComponentDelegate), EventDataNewRenderComponent::skEventType);
    pEventMgr->RemoveListener(MakeDelegate(this, &Scene::DestroyActorDelegate), EventDataDestroyActor::skEventType);
    pEventMgr->RemoveListener(MakeDelegate(this, &Scene::MoveActorDelegate), EventDataMoveActor::skEventType);
    pEventMgr->RemoveListener(MakeDelegate(this, &Scene::ModifiedRenderComponentDelegate), EventDataModifiedRenderComponent::skEventType);

	delete mLightManager;
}

//
// Scene::OnUpdate					- Chapter 16, page 540
//
bool Scene::OnAnimate(unsigned int uTime)
{
	if (!mRoot)
		return true;

	return mRoot->OnAnimate(this, uTime);
}

//
// OnUpdate					- Chapter 16, page 540
//
bool Scene::OnUpdate(unsigned long elapsedTime)
{
	if (!mRoot)
		return true;

	return mRoot->OnUpdate(this, elapsedTime);
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
	
	if (mRoot && mCamera)
	{
		// The scene root could be anything, but it
		// is usually a SceneNode with the identity
		// matrix
		mCamera->SetViewTransform(this);

		mLightManager->CalcLighting(this);

		if (mRoot->PreRender(this)==true)
		{
			if (mLightManager)
				mLightManager->OnPreRender(mRenderList[ERP_LIGHT]);

			mRoot->Render(this);
			mRoot->PostRender(this);
			
			if (mLightManager)
				mLightManager->OnPostRender();
		}

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
	if (mRoot)
		return mRoot->OnLostDevice(this);

	return true;
}

//
// Scene::OnRestore					- Chapter 16, page 540
//
bool Scene::OnRestore()
{
	if (!mRoot)
		return true;

	return mRoot->OnRestore(this);
}


//! Adds a scene node to the render queue.
void Scene::AddToRenderQueue(E_RENDER_PASS pass, const eastl::shared_ptr<Node>& node)
{
	if (!node)
		return;

	mRenderList[pass].push_back(node.get());
}


//! clears the render list
void Scene::ClearRenderList()
{
	for (int pass=0; pass < ERP_LAST; pass++)
		mRenderList[pass].clear();
}

//! Adds a scene node to the deletion queue.
void Scene::AddToDeletionQueue(Node* node)
{
	if (!node)
		return;

	mDeletionList.push_back(node);
}


//! clears the deletion list
void Scene::ClearDeletionList()
{
	if (mDeletionList.empty())
		return;

	mDeletionList.clear();
}


//! Removes all children of this scene node
void Scene::RemoveAll()
{
	mSceneNodeActors.clear();
	SetActiveCamera(0);
	// Make sure the driver is reset, might need a more complex method at some point

	if (mRenderer)
		mRenderer->SetMaterial(Material());
}


//! Clears the whole scene. All scene nodes are removed.
void Scene::Clear()
{
	RemoveAll();
}


//! Adds an empty scene node to the scene graph.
/** Can be used for doing advanced transformations
or structuring the scene graph. */
eastl::shared_ptr<Node> Scene::AddEmptyNode(const ActorId actorId,
	WeakBaseRenderComponentPtr renderComponent, const eastl::shared_ptr<Node>& parent, int id)
{
	eastl::shared_ptr<Node> node(new EmptyNode(actorId, renderComponent));
	node->Get()->SetId(id);
	if (!parent) 
		AddSceneNode(actorId, node);
	else 
		parent->AddChild(node);

	return node;
}

//! adds a test scene node for test purposes to the scene. It is a simple cube of (1,1,1) size.
//! the returned pointer must not be dropped.
eastl::shared_ptr<Node> Scene::AddCubeNode(const ActorId actorId,
	WeakBaseRenderComponentPtr renderComponent, const eastl::shared_ptr<Node>& parent, 
	float size, int id)
{
	eastl::shared_ptr<Node> node(new CubeNode(actorId, renderComponent, size));
	node->Get()->SetId(id);
	if (!parent) 
		AddSceneNode(actorId, node);
	else 
		parent->AddChild(node);

	return node;
}


//! Adds a sphere scene node for test purposes to the scene.
eastl::shared_ptr<Node> Scene::AddSphereNode(const ActorId actorId,
	WeakBaseRenderComponentPtr renderComponent, const eastl::shared_ptr<Node>& parent, 
	float radius, int polyCount, int id)
{
	eastl::shared_ptr<Node> node(
		new SphereNode(actorId, renderComponent, radius, polyCount, polyCount));
	node->Get()->SetId(id);
	if (!parent) 
		AddSceneNode(actorId, node);
	else 
		parent->AddChild(node);

	return node;
}

//! adds Volume Lighting Scene Node.
//! the returned pointer must not be dropped.
eastl::shared_ptr<Node> Scene::AddVolumeLightNode(const ActorId actorId,
	WeakBaseRenderComponentPtr renderComponent, const eastl::shared_ptr<Node>& parent, int id, 
	const unsigned int subdivU, const unsigned int subdivV, eastl::array<float, 4> const foot, 
	eastl::array<float, 4> const tail)
{
	eastl::shared_ptr<Node> node(
		new VolumeLightNode(actorId, renderComponent, subdivU, subdivV, foot, tail));
	node->Get()->SetId(id);
	if (!parent) 
		AddSceneNode(actorId, node);
	else 
		parent->AddChild(node);

	return node;
}

//! Adds a camera scene node to the tree and sets it as active camera.
//! \param position: Position of the space relative to its parent where the camera will be placed.
//! \param lookat: Position where the camera will look at. Also known as target.
//! \param parent: Parent scene node of the camera. Can be null. If the parent moves,
//! the camera will move too.
//! \return Returns pointer to interface to camera
eastl::shared_ptr<Node> Scene::AddCameraSceneNode(const ActorId actorId,
	WeakBaseRenderComponentPtr renderComponent, int id, bool makeActive)
{
	eastl::shared_ptr<Node> node(new CameraNode(actorId, this));
	node->Get()->SetId(id);
	AddNode(actorId, node);

	if (makeActive)
		SetActiveCamera(node);

	return node;
}

//! Adds a billboard scene node to the scene. A billboard is like a 3d sprite: A 2d element,
//! which always looks to the camera. It is usually used for things like explosions, fire,
//! lensflares and things like that.
eastl::shared_ptr<Node> Scene::AddBillboardNode(const ActorId actorId,
	WeakBaseRenderComponentPtr renderComponent, const eastl::shared_ptr<Node>& parent, int id,
	const Vector2<float>& size, const Vector3<float>& position,
	eastl::array<float, 4> const colorTop, eastl::array<float, 4> const colorBottom)
{

	eastl::shared_ptr<Node> node(
		new BillboardNode(actorId, renderComponent, size, colorTop, colorBottom));
	node->Get()->SetId(id);
	node->SetPosition(position);

	if (!parent) 
		AddSceneNode(actorId, node);
	else 
		parent->AddChild(node);

	return node;
}

eastl::shared_ptr<Node> Scene::AddParticleSystemNode(const ActorId actorId,
	WeakBaseRenderComponentPtr renderComponent, const eastl::shared_ptr<Node>& parent, 
	int id, bool withDefaultEmitter)
{
	eastl::shared_ptr<Node> node(
		new ParticleSystemNode(actorId, renderComponent, withDefaultEmitter));
	node->Get()->SetId(id);
	if (!parent) 
		AddNode(actorId, node);
	else 
		parent->AddChild(node);

	return node;
}

//! Adds a skybox scene node. A skybox is a big cube with 6 textures on it and
//! is drawn around the camera position.
eastl::shared_ptr<Node> Scene::AddSkyBoxNode(
	const ActorId actorId, WeakBaseRenderComponentPtr renderComponent, 
	const eastl::shared_ptr<Node>& parent,
	const eastl::shared_ptr<Texture2>& top, const eastl::shared_ptr<Texture2>& bottom,
	const eastl::shared_ptr<Texture2>& left, const eastl::shared_ptr<Texture2>& right,
	const eastl::shared_ptr<Texture2>& front, const eastl::shared_ptr<Texture2>& back, int id)
{

	eastl::shared_ptr<Node> node(
		new SkyBoxNode(actorId, renderComponent, top, bottom, left, right, front, back));

	node->Get()->SetId(id);
	if (!parent) 
		AddNode(actorId, node);
	else 
		parent->AddChild(node);

	return node;
}

//! adds a scene node for rendering a static mesh
//! the returned pointer must not be dropped.
eastl::shared_ptr<Node> Scene::AddMeshNode(const ActorId actorId,
	WeakBaseRenderComponentPtr renderComponent, const eastl::shared_ptr<Node>& parent, int id,
	bool alsoAddIfMeshPointerZero)
{
	if (!alsoAddIfMeshPointerZero && !mesh)
		return 0;

	eastl::shared_ptr<Node> node(new MeshNode(actorId, renderComponent, mesh));
	node->Get()->SetId(id);
	if (!parent) 
		AddNode(actorId, node);
	else 
		parent->AddChild(node);

	return node;
}


//! adds a scene node for rendering an animated mesh model
eastl::shared_ptr<Node> Scene::AddAnimatedMeshNode(const ActorId actorId,
	WeakBaseRenderComponentPtr renderComponent, const eastl::shared_ptr<Node>& parent,
	int id, bool alsoAddIfMeshPointerZero)
{
	if (!alsoAddIfMeshPointerZero && !mesh)
		return 0;

	eastl::shared_ptr<Node> node(new AnimatedMeshNode(actorId, renderComponent, mesh));
	node->Get()->SetId(id);
	if (!parent) 
		AddSceneNode(actorId, node);
	else 
		parent->AddChild(node);

	return node;
}

//! Adds a dynamic light scene node. The light will cast dynamic light on all
//! other scene nodes in the scene, which have the material flag MTF_LIGHTING
//! turned on. (This is the default setting in most scene nodes).
eastl::shared_ptr<Node> Scene::AddLightNode(const ActorId actorId,
	WeakBaseRenderComponentPtr renderComponent, const eastl::shared_ptr<Node>& parent, 
	eastl::array<float, 4> const color, float radius)
{
	eastl::shared_ptr<Node> node(new LightNode(actorId, renderComponent, color, radius));

	if (!parent) 
		AddSceneNode(actorId, node);
	else 
		parent->AddChild(node);

	return node;
}

//! Adds a child to this scene node.
/** If the scene node already has a parent it is first removed
from the other parent.
\param child A pointer to the new child. */
bool Scene::AddSceneNode(ActorId id, const eastl::shared_ptr<Node>& node)
{ 
	if (id != INVALID_ACTOR_ID)
	{
		// This allows us to search for this later based on actor id
		mSceneNodeActors[id] = node;	
	}

	eastl::shared_ptr<Light> pLight = eastl::dynamic_pointer_cast<Light>(kid);
	if (pLight != NULL && mLightManager->mLights.size()+1 < MAXIMUM_LIGHTS_SUPPORTED)
	{
		mLightManager->mLights.push_back(pLight);
	}

	return mRoot->AddChild(node);
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

	eastl::shared_ptr<Node> node = GetSceneNode(id);

	eastl::shared_ptr<Node> pLight = eastl::dynamic_pointer_cast<Node>(kid);
	if (pLight != NULL)
	{
		mLightManager->mLights.remove(pLight);
	}

	mSceneNodeActors.erase(id);
	return mRoot->RemoveChild(id);
}

//! Returns the first scene node with the specified id.
eastl::shared_ptr<Node> Scene::GetSceneNode(ActorId id)
{
	SceneNodeActorMap::iterator i = mSceneNodeActors.find(id);
	if (i==mSceneNodeActors.end())
		return 0;

	return i->second;
}

void Scene::NewRenderComponentDelegate(BaseEventDataPtr pEventData)
{
    eastl::shared_ptr<EventDataNewRenderComponent> pCastEventData = 
		eastl::static_pointer_cast<EventDataNewRenderComponent>(pEventData);

    ActorId actorId = pCastEventData->GetActorId();
    eastl::shared_ptr<Node> pSceneNode(pCastEventData->GetSceneNode());

    // FUTURE WORK: Add better error handling here.	
	/*
    if (FAILED(pSceneNode->OnRestore(this)))
    {
        LogError(eastl::string("Failed to restore scene node to the scene for actorid ") + eastl::to_string(actorId));
        return;
    }
	*/
    AddSceneNode(actorId, pSceneNode);
}

void Scene::ModifiedRenderComponentDelegate(BaseEventDataPtr pEventData)
{
    eastl::shared_ptr<EventDataModifiedRenderComponent> pCastEventData = 
		eastl::static_pointer_cast<EventDataModifiedRenderComponent>(pEventData);

    ActorId actorId = pCastEventData->GetActorId();
	if (actorId == INVALID_ACTOR_ID)
	{
		LogError("ModifiedRenderComponentDelegate - unknown actor id!");
		return;
	}
	/*
	if (GameLogic::Get()->GetState()==BGS_LOADINGGAMEENVIRONMENT)
		return;

	eastl::shared_ptr<Node> pSceneNode = GetSceneNode(actorId);
    // FUTURE WORK: Add better error handling here.		

    if (!pSceneNode  || FAILED(pSceneNode->OnRestore(this)))
    {
		LogError(eastl::string("Failed to restore scene node to the scene for actorid ") + eastl::to_string(actorId));
    }
	*/
}

void Scene::DestroyActorDelegate(BaseEventDataPtr pEventData)
{
    eastl::shared_ptr<EventDataDestroyActor> pCastEventData = 
		eastl::static_pointer_cast<EventDataDestroyActor>(pEventData);
    RemoveSceneNode(pCastEventData->GetId());
}

void Scene::MoveActorDelegate(BaseEventDataPtr pEventData)
{
    eastl::shared_ptr<EventDataMoveActor> pCastEventData = 
		eastl::static_pointer_cast<EventDataMoveActor>(pEventData);

    ActorId id = pCastEventData->GetId();
    const Transform& transform = pCastEventData->GetTransform();

    eastl::shared_ptr<Node> pNode = GetSceneNode(id);
    if (pNode)
		pNode->SetTransform(&transform);
}

//
// Scene::RenderAlphaPass			- Chapter 16, page 543
//
void Scene::RenderAlphaPass()
{
/*
	eastl::shared_ptr<BaseRenderState> alphaPass = mRenderer->PrepareAlphaPass();

	mAlphaSceneNodes.sort();
	while (!mAlphaSceneNodes.empty())
	{
		AlphaSceneNodes::reverse_iterator i = mAlphaSceneNodes.rbegin();
		PushAndSetMatrix((*i)->mConcat);
		(*i)->mNode->VRender(this);
		delete (*i);
		PopMatrix();
		mAlphaSceneNodes.pop_back();
	}
*/
}


//
// Scene::IsCulled				- Chapter 16, page 533
//	
bool Scene::IsCulled(Node* node)
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
			(mRenderer->GetOcclusionQueryResult(GetSceneNode(node->Get()->ActorId()))==0);
	}

	// can be seen by a bounding box ?
	if (!isVisible && (node->Get()->GetAutomaticCulling() & EAC_BOX))
	{
		AABBox3<float> tbox = node->GetBoundingBox();
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

		Vector3<float> edges[8];
		node->GetBoundingBox().GetEdges(edges);

		for (int i=0; i< ViewFrustum::VF_PLANE_COUNT; ++i)
		{
			bool boxInFrustum=false;
			for (unsigned int j=0; j<8; ++j)
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



