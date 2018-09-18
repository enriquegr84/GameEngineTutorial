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
	: mShadowColor{ 150 / 255.f,0.f,0.f,0.f }, mAmbientLight{ 0.f, 0.f, 0.f, 0.f }, mCurrentRenderPass(RP_NONE), mPVWUpdater(),
	mBufferUpdater([this](eastl::shared_ptr<Buffer> const& buffer) { Renderer::Get()->Update(buffer); })
{
	mLightManager.reset(new LightManager());
	mRoot.reset(new RootNode());

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
    pEventMgr->RemoveListener(MakeDelegate(this, &Scene::MoveActorDelegate), EventDataMoveActor::skEventType);
    pEventMgr->RemoveListener(MakeDelegate(this, &Scene::ModifiedRenderComponentDelegate), EventDataModifiedRenderComponent::skEventType);
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
	
	if (mRoot && mPVWUpdater.GetCamera())
	{

		//if (mFixedHeightRig.Move())
		{
			// The scene root could be anything, but it
			// is usually a SceneNode with the identity
			// matrix
			mLightManager->UpdateLighting(this);
			mPVWUpdater.Update();
			mCuller.ComputeVisibleSet(mPVWUpdater.GetCamera(), mRoot);
		}


		if (mRoot->PreRender(this)==true)
		{
			if (mLightManager)
				mLightManager->OnPreRender(mRenderList[RP_LIGHT]);

			mRoot->Render(this);

			/*
			for (auto const& visual : mCuller.GetVisibleSet())
			{
				//if (visual->name.find("Water") == std::string::npos)
				{
					Renderer::Get()->Draw(visual);
				}
			}

			Renderer::Get()->SetBlendState(mBlendState);
			Renderer::Get()->Draw(mWaterMesh[0]);
			Renderer::Get()->Draw(mWaterMesh[1]);
			Renderer::Get()->SetDefaultBlendState();
			*/

			mRoot->PostRender(this);
			
			if (mLightManager)
				mLightManager->OnPostRender();
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


//! Adds an empty scene node to the scene graph.
/** Can be used for doing advanced transformations
or structuring the scene graph. */
eastl::shared_ptr<Node> Scene::AddEmptyNode(
	WeakBaseRenderComponentPtr renderComponent, const eastl::shared_ptr<Node>& parent, int id)
{
	eastl::shared_ptr<Node> node(new EmptyNode(id, &mPVWUpdater, renderComponent));
	if (!parent) 
		AddSceneNode(id, node);
	else 
		parent->AttachChild(node);

	return node;
}

//! adds a rectangle scene node to the scene graph.
//! the returned pointer must not be dropped.
eastl::shared_ptr<Node> Scene::AddRectangleNode(
	WeakBaseRenderComponentPtr renderComponent, const eastl::shared_ptr<Node>& parent,
	const eastl::shared_ptr<Texture2>& texture, float texxScale, float texyScale, 
	float xSize, float ySize, int xPolyCount, int yPolyCount, int id)
{
	eastl::shared_ptr<Node> node(new RectangleNode(
		id, &mPVWUpdater, renderComponent, texture, texxScale, texyScale, xSize, ySize, xPolyCount, yPolyCount));
	if (!parent)
		AddSceneNode(id, node);
	else
		parent->AttachChild(node);

	return node;
}

//! adds a cube scene node to the scene graph.
//! the returned pointer must not be dropped.
eastl::shared_ptr<Node> Scene::AddCubeNode(
	WeakBaseRenderComponentPtr renderComponent, const eastl::shared_ptr<Node>& parent, 
	float size, int id)
{
	eastl::shared_ptr<Node> node(new CubeNode(id, &mPVWUpdater, renderComponent, size));
	if (!parent) 
		AddSceneNode(id, node);
	else 
		parent->AttachChild(node);

	return node;
}


//! Adds a sphere scene node for test purposes to the scene.
eastl::shared_ptr<Node> Scene::AddSphereNode(
	WeakBaseRenderComponentPtr renderComponent, const eastl::shared_ptr<Node>& parent, 
	float radius, int polyCount, int id)
{
	eastl::shared_ptr<Node> node(
		new SphereNode(id, &mPVWUpdater, renderComponent, radius, polyCount, polyCount));
	if (!parent) 
		AddSceneNode(id, node);
	else 
		parent->AttachChild(node);

	return node;
}

//! adds Volume Lighting Scene Node.
//! the returned pointer must not be dropped.
eastl::shared_ptr<Node> Scene::AddVolumeLightNode(
	WeakBaseRenderComponentPtr renderComponent, const eastl::shared_ptr<Node>& parent, int id, 
	const unsigned int subdivU, const unsigned int subdivV, eastl::array<float, 4> const foot, 
	eastl::array<float, 4> const tail)
{
	eastl::shared_ptr<Node> node(
		new VolumeLightNode(id, &mPVWUpdater, renderComponent, subdivU, subdivV, foot, tail));
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
eastl::shared_ptr<Node> Scene::AddBillboardNode(
	WeakBaseRenderComponentPtr renderComponent, const eastl::shared_ptr<Node>& parent,
	const eastl::shared_ptr<Texture2>& texture, const Vector2<float>& size, int id)
{
	eastl::shared_ptr<Node> node(
		new BillboardNode(id, &mPVWUpdater, renderComponent, texture, size));

	if (!parent) 
		AddSceneNode(id, node);
	else 
		parent->AttachChild(node);

	return node;
}

eastl::shared_ptr<Node> Scene::AddParticleSystemNode(
	WeakBaseRenderComponentPtr renderComponent, const eastl::shared_ptr<Node>& parent, 
	int id, bool withDefaultEmitter)
{
	eastl::shared_ptr<Node> node(
		new ParticleSystemNode(id, &mPVWUpdater, renderComponent, withDefaultEmitter));

	if (!parent) 
		AddSceneNode(id, node);
	else 
		parent->AttachChild(node);

	return node;
}

//! Adds a skydome scene node. A skydome is a large (half-) sphere with a
//! panoramic texture on it and is drawn around the camera position.
eastl::shared_ptr<Node> Scene::AddSkyDomeNode(
	WeakBaseRenderComponentPtr renderComponent, const eastl::shared_ptr<Node>& parent,
	const eastl::shared_ptr<Texture2>& sky, unsigned int horiRes, unsigned int vertRes,
	float texturePercentage, float spherePercentage, float radius, int id)
{

	eastl::shared_ptr<Node> node(new SkyDomeNode(id, &mPVWUpdater, renderComponent, 
		sky, horiRes, vertRes, texturePercentage, spherePercentage, radius));

	if (!parent) 
		AddSceneNode(id, node);
	else 
		parent->AttachChild(node);

	return node;
}

//! adds a scene node for rendering a static mesh
//! the returned pointer must not be dropped.
eastl::shared_ptr<Node> Scene::AddMeshNode(
	WeakBaseRenderComponentPtr renderComponent, const eastl::shared_ptr<Node>& parent, 
	eastl::shared_ptr<BaseMesh> mesh, int id, bool alsoAddIfMeshPointerZero)
{
	if (!alsoAddIfMeshPointerZero && !mesh)
		return nullptr;

	eastl::shared_ptr<Node> node(new MeshNode(id, &mPVWUpdater, renderComponent, mesh));

	if (!parent) 
		AddSceneNode(id, node);
	else 
		parent->AttachChild(node);

	return node;
}


//! adds a scene node for rendering an animated mesh model
eastl::shared_ptr<Node> Scene::AddAnimatedMeshNode(
	WeakBaseRenderComponentPtr renderComponent, const eastl::shared_ptr<Node>& parent,
	eastl::shared_ptr<BaseAnimatedMesh> mesh, int id, bool alsoAddIfMeshPointerZero)
{
	if (!alsoAddIfMeshPointerZero && !mesh)
		return nullptr;

	eastl::shared_ptr<Node> node(
		new AnimatedMeshNode(id, &mPVWUpdater, renderComponent, mesh));

	if (!parent) 
		AddSceneNode(id, node);
	else 
		parent->AttachChild(node);

	return node;
}

//! Adds a dynamic light scene node. The light will cast dynamic light on all
//! other scene nodes in the scene, which have the material flag MTF_LIGHTING
//! turned on. (This is the default setting in most scene nodes).
eastl::shared_ptr<Node> Scene::AddLightNode(
	WeakBaseRenderComponentPtr renderComponent, const eastl::shared_ptr<Node>& parent, 
	const eastl::shared_ptr<Light>& light, int id)
{
	eastl::shared_ptr<Node> node(
		new LightNode(id, &mPVWUpdater, renderComponent, light));

	if (!parent) 
		AddSceneNode(id, node);
	else 
		parent->AttachChild(node);

	return node;
}

//! creates a rotation animator, which rotates the attached scene node around itself.
eastl::shared_ptr<NodeAnimator> Scene::CreateRotationAnimator(const Vector3<float>& rotationPerSecond)
{
	return eastl::shared_ptr<NodeAnimator>(
		new NodeAnimatorRotation(Timer::GetTime(), rotationPerSecond));
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
	const eastl::vector<Texture2*>& textures, int timePerFrame, bool loop)
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
	if (!node->IsVisible())
		return true;

	// transform the location of this node into the camera space 
	// of the camera attached to the scene
	/*
	ViewFrustum frustum(mPVWUpdater.GetCamera()->GetViewFrustum());
	*/
	float const* frustum = mPVWUpdater.GetCamera()->GetFrustum();

	bool isVisible = false;
	/*
	// has occlusion query information
	if (node->GetAutomaticCulling() & AC_OCC_QUERY)
	{
		isVisible =
			(Renderer::Get()->GetOcclusionQueryResult(GetSceneNode(node->GetId())) == 0);
	}
	*/
	// can be seen by a bounding box ?
	if (!isVisible && (node->GetAutomaticCulling() & AC_BOX))
	{
		/*
		AlignedBox3<float> tbox = node->GetBoundingBox();
		toWorld.TransformBoxEx(tbox);
		isVisible = !(tbox.IntersectsWithBox(frustum.GetBoundingBox()));
		*/
		isVisible = mCuller.IsVisible(node->GetAbsoulteBound());
	}

	// can be seen by a bounding sphere
	if (!isVisible && (node->GetAutomaticCulling() & AC_FRUSTUM_SPHERE))
	{ 
		// requires bbox diameter
		isVisible = mCuller.IsVisible(node->GetAbsoulteBound());
	}

	// can be seen by cam pyramid planes ?
	if (!isVisible && (node->GetAutomaticCulling() & AC_FRUSTUM_BOX))
	{
		/*
		//transform the frustum to the node's current absolute transformation
		Matrix4x4<float> invTrans(
			node->GetAbsoluteTransform().GetMatrix(), 
			Matrix4x4<float>::EM4CONST_INVERSE);
		//invTrans.makeInverse();
		frustum.Transform(invTrans);

		Vector3<float> edges[8];
		node->GetBoundingBox().GetEdges(edges);

		for (int i = 0; i< ViewFrustum::VF_PLANE_COUNT; ++i)
		{
			bool boxInFrustum = false;
			for (unsigned int j = 0; j<8; ++j)
			{
				if (frustum.Planes[i].ClassifyPointRelation(edges[j]) != ISREL3D_FRONT)
				{
					boxInFrustum = true;
					break;
				}
			}

			if (!boxInFrustum)
			{
				isVisible = true;
				break;
			}
		}
		*/
	}

	return isVisible;
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

void Scene::MoveActorDelegate(BaseEventDataPtr pEventData)
{
    eastl::shared_ptr<EventDataMoveActor> pCastEventData = 
		eastl::static_pointer_cast<EventDataMoveActor>(pEventData);

    ActorId id = pCastEventData->GetId();
    const Transform& transform = pCastEventData->GetTransform();

    eastl::shared_ptr<Node> pNode = GetSceneNode(id);
    if (pNode)
		pNode->GetRelativeTransform() = transform;

}
