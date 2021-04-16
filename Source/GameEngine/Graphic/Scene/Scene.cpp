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

#include "Application/GameApplication.h"

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
	: mCurrentRenderPass(RP_NONE), mPVWUpdater(),
	mShadowColor{ 150 / 255.f,0.f,0.f,0.f }, mAmbientLight{ 0.f, 0.f, 0.f, 0.f }, 
	mBufferUpdater([this](eastl::shared_ptr<Buffer> const& buffer) { Renderer::Get()->Update(buffer); })
{
	mLightManager.reset(new LightManager());
	mRoot.reset(new RootNode());

	// [mrmike] - event delegates were added post-press
    BaseEventManager* pEventMgr = BaseEventManager::Get();
    pEventMgr->AddListener(MakeDelegate(this, &Scene::NewRenderComponentDelegate), EventDataNewRenderComponent::skEventType);
    pEventMgr->AddListener(MakeDelegate(this, &Scene::DestroyActorDelegate), EventDataDestroyActor::skEventType);
	pEventMgr->AddListener(MakeDelegate(this, &Scene::SyncActorDelegate), EventDataSyncActor::skEventType);
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
	//if (Renderer::Get())
	//	Renderer::Get()->RemoveAllHardwareBuffers();

	// remove all nodes and animators before dropping the driver
	// as render targets may be destroyed twice
	RemoveAll();
	//RemoveAnimators();

	// [mrmike] - event delegates were added post-press!
    BaseEventManager* pEventMgr = BaseEventManager::Get();
    pEventMgr->RemoveListener(MakeDelegate(this, &Scene::NewRenderComponentDelegate), EventDataNewRenderComponent::skEventType);
    pEventMgr->RemoveListener(MakeDelegate(this, &Scene::DestroyActorDelegate), EventDataDestroyActor::skEventType);
	pEventMgr->RemoveListener(MakeDelegate(this, &Scene::SyncActorDelegate), EventDataSyncActor::skEventType);
    pEventMgr->RemoveListener(MakeDelegate(this, &Scene::ModifiedRenderComponentDelegate), EventDataModifiedRenderComponent::skEventType);
}

//
// OnUpdate					- Chapter 16, page 540
//
bool Scene::OnUpdate(unsigned int timeMs, unsigned long elapsedTime)
{
	if (!mRoot)
		return true;

	return mRoot->OnUpdate(this, timeMs, elapsedTime);
}


//
// Scene::OnRender					- Chapter 16, page 539
//
bool Scene::OnRender()
{
	if (mRoot && mPVWUpdater.GetCamera())
	{
		if (mRoot->PreRender(this)==true)
		{
			mPVWUpdater.Update();
			mCuller.ComputeVisibleSet(mPVWUpdater.GetCamera(), mRoot);

			if (mLightManager)
				mLightManager->OnPreRender(mRenderList[RP_LIGHT]);

			mRoot->Render(this);

			if (mLightManager)
				mLightManager->OnPostRender();

			mRoot->PostRender(this);
		}
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
void Scene::AddToRenderQueue(RenderPass pass, const eastl::shared_ptr<Node>& node)
{
	if (!node)
		return;

	mRenderList[pass].push_back(node.get());
}


//! clears the render list
void Scene::ClearRenderList()
{
	for (int pass=0; pass < RP_LAST; pass++)
		mRenderList[pass].clear();
}

//! Adds a scene node to the deletion queue.
void Scene::AddToDeletionQueue(Node* node)
{
	if (!node)
		return;

	mDeletionList.push_back(node);

	eastl::shared_ptr<EventDataRequestDestroyActor> 
		pRequestDestroyActorEvent(new EventDataRequestDestroyActor(node->GetId()));
	BaseEventManager::Get()->QueueEvent(pRequestDestroyActorEvent);
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
	mPVWUpdater.UnsubscribeAll();
	// Make sure the driver is reset, might need a more complex method at some point
	/*
	if (mRenderer)
		mRenderer->SetMaterial(Material());
	*/
}


//! Clears the whole scene. All scene nodes are removed.
void Scene::Clear()
{
	RemoveAll();
}


//! adds a scene node for rendering a mesh
//! the returned pointer must not be dropped.
eastl::shared_ptr<Node> Scene::AddMeshNode(const eastl::shared_ptr<Node>& parent,
	eastl::shared_ptr<BaseMesh> mesh, int id, bool alsoAddIfMeshPointerZero)
{
	if (!alsoAddIfMeshPointerZero && !mesh)
		return nullptr;

	eastl::shared_ptr<Node> node(new MeshNode(id, &mPVWUpdater, mesh));

	if (!parent)
		AddSceneNode(id, node);
	else
		parent->AttachChild(node);

	return node;
}


//! adds a rectangle scene node to the scene graph.
//! the returned pointer must not be dropped.
eastl::shared_ptr<Node> Scene::AddRectangleNode(const eastl::shared_ptr<Node>& parent,
	const eastl::shared_ptr<Texture2>& texture, float texxScale, float texyScale, 
	float xSize, float ySize, int xPolyCount, int yPolyCount, int id)
{
	eastl::shared_ptr<Node> node(new RectangleNode(
		id, &mPVWUpdater, texture, texxScale, texyScale, xSize, ySize, xPolyCount, yPolyCount));
	if (!parent)
		AddSceneNode(id, node);
	else
		parent->AttachChild(node);

	return node;
}

//! adds a box scene node to the scene graph.
//! the returned pointer must not be dropped.
eastl::shared_ptr<Node> Scene::AddBoxNode(const eastl::shared_ptr<Node>& parent, 
	const eastl::shared_ptr<Texture2>& texture, Vector2<float> texScale, Vector3<float> size, int id)
{
	eastl::shared_ptr<Node> node(new BoxNode(id, &mPVWUpdater, texture, texScale, size));
	if (!parent) 
		AddSceneNode(id, node);
	else 
		parent->AttachChild(node);

	return node;
}


//! Adds a sphere scene node for test purposes to the scene.
eastl::shared_ptr<Node> Scene::AddSphereNode(const eastl::shared_ptr<Node>& parent, 
	const eastl::shared_ptr<Texture2>& texture, float radius, int polyCount, int id)
{
	eastl::shared_ptr<Node> node(
		new SphereNode(id, &mPVWUpdater, texture, radius, polyCount, polyCount));
	if (!parent) 
		AddSceneNode(id, node);
	else 
		parent->AttachChild(node);

	return node;
}

//! adds Volume Lighting Scene Node.
//! the returned pointer must not be dropped.
eastl::shared_ptr<Node> Scene::AddVolumeLightNode(const eastl::shared_ptr<Node>& parent,
	const Vector2<float>& textureSize, const eastl::shared_ptr<Texture2>& texture, 
	const Vector2<int>& subdivision, const SColorF& foot, 
	const SColorF& tail, int id)
{
	eastl::shared_ptr<Node> node(new VolumeLightNode(
		id, &mPVWUpdater, textureSize, texture, subdivision, foot, tail));
	if (!parent) 
		AddSceneNode(id, node);
	else 
		parent->AttachChild(node);

	return node;
}

//! Adds a camera scene node to the tree and sets it as active camera.
//! \param position: Position of the space relative to its parent where the camera will be placed.
//! \param lookat: Position where the camera will look at. Also known as target.
//! \param parent: Parent scene node of the camera. Can be null. If the parent moves,
//! the camera will move too.
//! \return Returns pointer to interface to camera
eastl::shared_ptr<Node> Scene::AddCameraNode(int id, bool makeActive)
{
	eastl::shared_ptr<Node> node(new CameraNode(id));
	AddSceneNode(id, node);
	/*
	if (makeActive)
		SetActiveCamera(node);
	*/
	return node;
}

//! Adds a billboard scene node to the scene. A billboard is like a 3d sprite: A 2d element,
//! which always looks to the camera. It is usually used for things like explosions, fire,
//! lensflares and things like that.
eastl::shared_ptr<Node> Scene::AddBillboardNode(const eastl::shared_ptr<Node>& parent,
	const eastl::shared_ptr<Texture2>& texture, const Vector2<float>& size, int id)
{
	eastl::shared_ptr<Node> node(
		new BillboardNode(id, &mPVWUpdater, texture, size));

	if (!parent) 
		AddSceneNode(id, node);
	else 
		parent->AttachChild(node);

	return node;
}

eastl::shared_ptr<Node> Scene::AddParticleSystemNode(
	const eastl::shared_ptr<Node>& parent, int id, bool withDefaultEmitter)
{
	eastl::shared_ptr<Node> node(
		new ParticleSystemNode(id, &mPVWUpdater, withDefaultEmitter));

	if (!parent) 
		AddSceneNode(id, node);
	else 
		parent->AttachChild(node);

	return node;
}

//! Adds a skydome scene node. A skydome is a large (half-) sphere with a
//! panoramic texture on it and is drawn around the camera position.
eastl::shared_ptr<Node> Scene::AddSkyDomeNode(const eastl::shared_ptr<Node>& parent,
	const eastl::shared_ptr<Texture2>& sky, unsigned int horiRes, unsigned int vertRes,
	float texturePercentage, float spherePercentage, float radius, int id)
{

	eastl::shared_ptr<Node> node(new SkyDomeNode(id, &mPVWUpdater, 
		sky, horiRes, vertRes, texturePercentage, spherePercentage, radius));

	if (!parent) 
		AddSceneNode(id, node);
	else 
		parent->AttachChild(node);

	return node;
}

//! adds a scene node for rendering a static mesh
//! the returned pointer must not be dropped.
eastl::shared_ptr<Node> Scene::AddStaticMeshNode(const eastl::shared_ptr<Node>& parent, 
	eastl::shared_ptr<BaseMesh> mesh, int id, bool alsoAddIfMeshPointerZero)
{
	if (!alsoAddIfMeshPointerZero && !mesh)
		return nullptr;

	eastl::shared_ptr<Node> node(new StaticMeshNode(id, &mPVWUpdater, mesh));

	if (!parent) 
		AddSceneNode(id, node);
	else 
		parent->AttachChild(node);

	return node;
}


//! adds a scene node for rendering an animated mesh model
eastl::shared_ptr<Node> Scene::AddAnimatedMeshNode(const eastl::shared_ptr<Node>& parent,
	eastl::shared_ptr<BaseAnimatedMesh> mesh, int id, bool alsoAddIfMeshPointerZero)
{
	if (!alsoAddIfMeshPointerZero && !mesh)
		return nullptr;

	eastl::shared_ptr<Node> node(
		new AnimatedMeshNode(id, &mPVWUpdater, mesh));

	if (!parent) 
		AddSceneNode(id, node);
	else 
		parent->AttachChild(node);

	return node;
}

//! Adds a dynamic light scene node. The light will cast dynamic light on all
//! other scene nodes in the scene, which have the material flag MTF_LIGHTING
//! turned on. (This is the default setting in most scene nodes).
eastl::shared_ptr<Node> Scene::AddLightNode(const eastl::shared_ptr<Node>& parent, 
	const eastl::shared_ptr<Light>& light, int id)
{
	eastl::shared_ptr<Node> node(new LightNode(id, &mPVWUpdater, light));

	if (!parent) 
		AddSceneNode(id, node);
	else 
		parent->AttachChild(node);

	return node;
}

//! creates a rotation animator, which rotates the attached scene node around itself.
eastl::shared_ptr<NodeAnimator> Scene::CreateRotationAnimator(const Vector4<float>& rotation, float rotationSpeed)
{
	return eastl::shared_ptr<NodeAnimator>(new NodeAnimatorRotation(Timer::GetTime(), rotation, rotationSpeed));
}


//! creates a fly circle animator, which lets the attached scene node fly around a center.
eastl::shared_ptr<NodeAnimator> Scene::CreateFlyCircleAnimator(const Vector3<float>& center,
	float radius, float speed, const Vector3<float>& direction, float startPosition, float radiusEllipsoid)
{
	const float orbitDurationMs = ((float)GE_C_DEG_TO_RAD * 360.f) / speed;
	const unsigned int effectiveTime = Timer::GetTime() + (unsigned int)(orbitDurationMs * startPosition);

	return eastl::shared_ptr<NodeAnimator>(
		new NodeAnimatorFlyCircle(effectiveTime, center, radius, speed, direction, radiusEllipsoid));
}


//! Creates a fly straight animator, which lets the attached scene node
//! fly or move along a line between two points.
eastl::shared_ptr<NodeAnimator> Scene::CreateFlyStraightAnimator(const Vector3<float>& startPoint,
	const Vector3<float>& endPoint, unsigned int timeForWay, bool loop, bool pingpong)
{
	return eastl::shared_ptr<NodeAnimator>(
		new NodeAnimatorFlyStraight(startPoint, endPoint, timeForWay, loop, Timer::GetTime(), pingpong));
}


//! Creates a texture animator, which switches the textures of the target scene
//! node based on a list of textures.
eastl::shared_ptr<NodeAnimator> Scene::CreateTextureAnimator(
	const eastl::vector<eastl::shared_ptr<Texture2>>& textures, int timePerFrame, bool loop)
{
	return eastl::shared_ptr<NodeAnimator>(
		new NodeAnimatorTexture(textures, timePerFrame, loop, Timer::GetTime()));
}


//! Creates a scene node animator, which deletes the scene node after
//! some time automaticly.
eastl::shared_ptr<NodeAnimator> Scene::CreateDeleteAnimator(unsigned int when)
{
	return eastl::shared_ptr<NodeAnimator>(new NodeAnimatorDelete(Timer::GetTime() + when));
}

//! Creates a follow spline animator.
eastl::shared_ptr<NodeAnimator> Scene::CreateFollowSplineAnimator(int startTime,
	const eastl::vector<Vector3<float>>& points, float speed, float tightness, bool loop, bool pingpong)
{
	return eastl::shared_ptr<NodeAnimator>(
		new NodeAnimatorFollowSpline(startTime, points, speed, tightness, loop, pingpong));
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

	eastl::shared_ptr<Light> pLight = eastl::dynamic_pointer_cast<Light>(node);
	if (pLight != NULL && mLightManager->mLights.size()+1 < MAXIMUM_LIGHTS_SUPPORTED)
	{
		mLightManager->mLights.push_back(pLight);
	}

	return mRoot->AttachChild(node);
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
	eastl::shared_ptr<Light> pLight = eastl::dynamic_pointer_cast<Light>(node);
	if (pLight != NULL)
	{
		mLightManager->mLights.remove(pLight);
	}

	mSceneNodeActors.erase(id);
	return mRoot->DetachChild(node);
}

//! Returns the first scene node with the specified id.
eastl::shared_ptr<Node> Scene::GetSceneNode(ActorId id)
{
	SceneNodeActorMap::iterator i = mSceneNodeActors.find(id);
	if (i==mSceneNodeActors.end())
		return nullptr;

	return i->second;
}

void Scene::SetActiveCamera(const eastl::shared_ptr<CameraNode>& camera)
{
	mCamera = camera;
	mPVWUpdater.Set(camera->Get(), mBufferUpdater);
}

//
// Scene::IsCulled				- Chapter 16, page 533
//	
bool Scene::IsCulled(Node* node)
{
	return !mCuller.IsVisible(node);
}

void Scene::NewRenderComponentDelegate(BaseEventDataPtr pEventData)
{
    eastl::shared_ptr<EventDataNewRenderComponent> pCastEventData = 
		eastl::static_pointer_cast<EventDataNewRenderComponent>(pEventData);

    ActorId actorId = pCastEventData->GetActorId();
    eastl::shared_ptr<Node> pSceneNode(pCastEventData->GetSceneNode());

    // FUTURE WORK: Add better error handling here.	
    if (!pSceneNode || FAILED(pSceneNode->OnRestore(this)))
    {
        LogError(eastl::string("Failed to restore scene node to the scene for actorid ") + eastl::to_string(actorId));
        return;
    }
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
	if (GameLogic::Get()->GetState()==BGS_LOADINGGAMEENVIRONMENT)
		return;

	eastl::shared_ptr<Node> pSceneNode = GetSceneNode(actorId);
    // FUTURE WORK: Add better error handling here.		

    if (!pSceneNode  || FAILED(pSceneNode->OnRestore(this)))
    {
		LogError(eastl::string("Failed to restore scene node to the scene for actorid ") + eastl::to_string(actorId));
    }
}

void Scene::DestroyActorDelegate(BaseEventDataPtr pEventData)
{
    eastl::shared_ptr<EventDataDestroyActor> pCastEventData = 
		eastl::static_pointer_cast<EventDataDestroyActor>(pEventData);
    RemoveSceneNode(pCastEventData->GetId());
}

void Scene::SyncActorDelegate(BaseEventDataPtr pEventData)
{
	eastl::shared_ptr<EventDataSyncActor> pCastEventData =
		eastl::static_pointer_cast<EventDataSyncActor>(pEventData);

	ActorId actorId = pCastEventData->GetId();
	eastl::shared_ptr<Node> pNode = GetSceneNode(actorId);
	if (pNode)
	{
		eastl::shared_ptr<Actor> pGameActor(GameLogic::Get()->GetActor(actorId).lock());
		eastl::shared_ptr<TransformComponent> pTransformComponent(
			pGameActor->GetComponent<TransformComponent>(TransformComponent::Name).lock());
		if (pTransformComponent)
			pTransformComponent->SetPosition(pCastEventData->GetTransform().GetTranslation());
		pNode->GetRelativeTransform().SetRotation(pCastEventData->GetTransform().GetRotation());
		pNode->GetRelativeTransform().SetTranslation(pCastEventData->GetTransform().GetTranslation());

		eastl::shared_ptr<PhysicComponent> pPhysicComponent(
			pGameActor->GetComponent<PhysicComponent>(PhysicComponent::Name).lock());
		if (pPhysicComponent)
		{
			Vector4<float> actorPosOffset = HLift(pPhysicComponent->GetPositionOffset(), 0.f);
			Vector3<float> actorTranslation = pNode->GetRelativeTransform().GetTranslation();
			Matrix4x4<float> actorRotation = pNode->GetRelativeTransform().GetRotation();
#if defined(GE_USE_MAT_VEC)
			actorTranslation -= HProject(actorRotation * actorPosOffset);
#else
			actorTranslation -= HProject(actorPosOffset * actorRotation);
#endif
			pNode->GetRelativeTransform().SetTranslation(actorTranslation);
		}
	}
}
