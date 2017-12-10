//========================================================================
// File: Scene.h - implements the container class for 3D Graphics scenes
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

#ifndef SCENE_H
#define SCENE_H

#include "GameEngineStd.h"

#include "Graphic/ScreenElement.h"
#include "Graphic/Renderer/Renderer.h"

#include "Core/Event/EventManager.h"

class Node;
class Camera;
class LightManager;

// Forward declarations
////////////////////////////////////////////////////
//
// SceneActorMap Description				- Chapter 16, page Y
//
//   An STL map that allows fast lookup of a scene node given an ActorId.
//
////////////////////////////////////////////////////

typedef eastl::map<ActorId, eastl::shared_ptr<Node> > SceneNodeActorMap;
typedef eastl::vector<Node*> SceneNodeRenderList;


////////////////////////////////////////////////////
//
// Scene Description						- Chapter 16, page 536
//
// A heirarchical container of scene nodes, which
// are classes that implemente the SceneNode interface
//
////////////////////////////////////////////////////

//class SkyNode;
//class LightNode;
//class LightManager;
//class CameraSceneNode;


/////////////////////////////////////////////////////////////////////////////
// enum RenderPass							- Chapter 16, page 529
//
//   3D scenes are drawn in passes - this enum defines the render passes
//   supported by the 3D scene graph created by class Scene.
/////////////////////////////////////////////////////////////////////////////

//! Enumeration for render passes.
/** A parameter passed to the registerNodeForRendering() method of the ISceneManager,
specifying when the node wants to be drawn in relation to the other nodes. */
enum E_RENDER_PASS
{
	//! No pass currently active
	ERP_NONE = 0,

	//! Camera pass. The active view is set up here. The very first pass.
	ERP_CAMERA,

	//! Static nodes
	ERP_STATIC,

	//! In this pass, lights are transformed into camera space and added to the driver
	ERP_LIGHT,

	//! This is used for sky boxes.
	ERP_SKY_BOX,

	//! Solid scene nodes or special scene nodes without materials.
	ERP_SOLID,

	//! Transparent scene nodes, drawn after solid nodes. They are sorted from back to front and drawn in that order.
	ERP_TRANSPARENT,

	//! Transparent effect scene nodes, drawn after Transparent nodes. They are sorted from back to front and drawn in that order.
	ERP_TRANSPARENT_EFFECT,

	//! Drawn after the solid nodes, before the transparent nodes, the time for drawing shadow volumes
	ERP_SHADOW,

	//! Last render pass, it doesnt do anything
	ERP_LAST
};

//! An enumeration for all types of built-in scene node animators
enum E_SCENE_NODE_ANIMATOR_TYPE
{
	//! Fly circle scene node animator
	ESNAT_FLY_CIRCLE = 0,

	//! Fly straight scene node animator
	ESNAT_FLY_STRAIGHT,

	//! Follow spline scene node animator
	ESNAT_FOLLOW_SPLINE,

	//! Rotation scene node animator
	ESNAT_ROTATION,

	//! Texture scene node animator
	ESNAT_TEXTURE,

	//! Deletion scene node animator
	ESNAT_DELETION,

	//! Amount of built-in scene node animators
	ESNAT_COUNT,

	//! Unknown scene node animator
	ESNAT_UNKNOWN,

	//! This enum is never used, it only forces the compiler to compile this enumeration to 32 bit.
	ESNAT_FORCE_32_BIT = 0x7fffffff
};

//! An enumeration for all types of built-in scene nodes
/** A scene node type is represented by a four character code
such as 'cube' or 'mesh' instead of simple numbers, to avoid
name clashes with external scene nodes.*/
enum E_SCENE_NODE_TYPE
{

	//! simple object scene node
	ESNT_SHAPE,

	//! simple cube scene node
	ESNT_CUBE,

	//! Sphere scene node
	ESNT_SPHERE,

	//! Text Scene Node
	ESNT_TEXT,

	//! Water Surface Scene Node
	ESNT_WATER_SURFACE,

	//! Terrain Scene Node
	ESNT_TERRAIN,

	//! Sky Box Scene Node
	ESNT_SKY_BOX,

	//! Sky Dome Scene Node
	ESNT_SKY_DOME,

	//! Shadow Volume Scene Node
	ESNT_SHADOW_VOLUME,

	//! Octree Scene Node
	ESNT_OCTREE,

	//! Mesh Scene Node
	ESNT_MESH,

	//! Light Scene Node
	ESNT_LIGHT,

	//! Empty Scene Node
	ESNT_EMPTY,

	//! Dummy Transformation Scene Node
	ESNT_DUMMY_TRANSFORMATION,

	//! Camera Scene Node
	ESNT_CAMERA,

	//! Billboard Scene Node
	ESNT_BILLBOARD,

	//! Animated Mesh Scene Node
	ESNT_ANIMATED_MESH,

	//! Particle System Scene Node
	ESNT_PARTICLE_SYSTEM,

	//! Quake3 Shader Scene Node
	ESNT_Q3SHADER,

	//! Quake3 Model Scene Node ( has tag to link to )
	ESNT_MD3,

	//! Volume Light Scene Node
	ESNT_VOLUME_LIGHT,

	//! First Person Shooter Camera
	/** Legacy, for loading version <= 1.4.x .irr files */
	ESNT_CAMERA_FPS,

	//! Unknown scene node
	ESNT_UNKNOWN,

	//! Will match with any scene node when checking types
	ESNT_ANY
};

//! An enumeration for all types of automatic culling for built-in scene nodes
enum E_CULLING_TYPE
{
	EAC_OFF = 0,
	EAC_BOX = 1,
	EAC_FRUSTUM_BOX = 2,
	EAC_FRUSTUM_SPHERE = 4,
	EAC_OCC_QUERY = 8
};

//! An enumeration for all types of debug data for built-in scene nodes (flags)
enum E_DEBUG_SCENE_TYPE
{
	//! No Debug Data ( Default )
	EDS_OFF = 0,

	//! Show Bounding Boxes of SceneNode
	EDS_BBOX = 1,

	//! Show Vertex Normals
	EDS_NORMALS = 2,

	//! Shows Skeleton/Tags
	EDS_SKELETON = 4,

	//! Overlays Mesh Wireframe
	EDS_MESH_WIRE_OVERLAY = 8,

	//! Temporary use transparency Material Type
	EDS_HALF_TRANSPARENCY = 16,

	//! Show Bounding Boxes of all MeshBuffers
	EDS_BBOX_BUFFERS = 32,

	//! EDS_BBOX | EDS_BBOX_BUFFERS
	EDS_BBOX_ALL = EDS_BBOX | EDS_BBOX_BUFFERS,

	//! Show all debug infos
	EDS_FULL = 0xffffffff
};

class Scene
{
protected:
	eastl::shared_ptr<Node> mRoot;
	eastl::shared_ptr<Camera> mCamera;

	LightManager* mLightManager;

	SceneNodeActorMap mSceneNodeActors;		
	E_RENDER_PASS mCurrentRenderPass;
	
	eastl::array<float, 4> mShadowColor;
	eastl::array<float, 4> mAmbientLight;

	//! scene node lists
	SceneNodeRenderList mDeletionList;
	SceneNodeRenderList mRenderList[ERP_LAST];

	void RenderAlphaPass();

	void RemoveAll();
	void Clear();

public:

	Scene();
	virtual ~Scene();

	bool OnUpdate(unsigned long elapsedTime);
	bool OnAnimate(unsigned int uTime);
	bool OnRender();

	bool OnRestore();
	bool OnLostDevice();

	// event delegates
	void NewRenderComponentDelegate(BaseEventDataPtr pEventData);
	void ModifiedRenderComponentDelegate(BaseEventDataPtr pEventData);			// added post-press!
	void DestroyActorDelegate(BaseEventDataPtr pEventData);
	void MoveActorDelegate(BaseEventDataPtr pEventData);

	//! Adds an empty scene node to the scene graph.
	/** Can be used for doing advanced transformations
	or structuring the scene graph.
	\return Pointer to the created scene node. */
	eastl::shared_ptr<Node> AddEmptyNode(const ActorId actorId,
		WeakBaseRenderComponentPtr renderComponent, const eastl::shared_ptr<Node>& parent, 
		int id = -1);

	//! Adds a cube scene node
	/** \param size: Size of the cube, uniformly in each dimension.
	\param parent: Parent of the scene node. Can be 0 if no parent.
	\param id: Id of the node. This id can be used to identify the scene node.
	\param position: Position of the space relative to its parent
	where the scene node will be placed.
	\param rotation: Initital rotation of the scene node.
	\param scale: Initial scale of the scene node.
	\return Pointer to the created test scene node. This
	pointer should not be dropped. See IReferenceCounted::drop()
	for more information. */
	eastl::shared_ptr<Node> AddCubeNode(const ActorId actorId,
		WeakBaseRenderComponentPtr renderComponent, const eastl::shared_ptr<Node>& parent, 
		float size = 10.0f, int id = -1);

	//! Adds a sphere scene node of the given radius and detail
	/** \param radius: Radius of the sphere.
	\param polyCount: The number of vertices in horizontal and
	vertical direction. The total polyCount of the sphere is
	polyCount*polyCount. This parameter must be less than 256 to
	stay within the 16-bit limit of the indices of a meshbuffer.
	\param parent: Parent of the scene node. Can be 0 if no parent.
	\param id: Id of the node. This id can be used to identify the scene node.
	\param position: Position of the space relative to its parent
	where the scene node will be placed.
	\param rotation: Initital rotation of the scene node.
	\param scale: Initial scale of the scene node.
	\return Pointer to the created test scene node. This
	pointer should not be dropped. See IReferenceCounted::drop()
	for more information. */
	eastl::shared_ptr<Node> AddSphereNode(const ActorId actorId,
		WeakBaseRenderComponentPtr renderComponent, const eastl::shared_ptr<Node>& parent, 
		float radius = 5.0f, int polyCount = 16, int id = -1);

	//! Adds Volume Lighting Scene Node.
	/** Example Usage:
	IVolumeLightSceneNode * n = smgr->addVolumeLightSceneNode(0, -1,
	32, 32, //Subdivide U/V
	SColor(0, 180, 180, 180), //foot color
	SColor(0, 0, 0, 0) //tail color
	);
	if (n)
	{
	n->setScale(Vector3<float>(46.0f, 45.0f, 46.0f));
	n->getMaterial(0).setTexture(0, smgr->getVideoDriver()->getTexture("lightFalloff.png"));
	}
	\return Pointer to the volumeLight if successful, otherwise NULL.
	This pointer should not be dropped. See IReferenceCounted::drop() for more information. */
	eastl::shared_ptr<Node> AddVolumeLightNode(const ActorId actorId,
		WeakBaseRenderComponentPtr renderComponent, const eastl::shared_ptr<Node>& parent,
		int id = -1, const unsigned int subdivU = 32, const unsigned int subdivV = 32,
		eastl::array<float, 4> const foot = eastl::array<float, 4>{51.f, 0.f, 230.f, 180.f}, 
		eastl::array<float, 4> const tail = eastl::array<float, 4>{0.f, 0.f, 0.f, 0.f});

	//! Adds a camera scene node to the scene graph and sets it as active camera.
	/** This camera does not react on user input like for example the one created with
	addCameraSceneNodeFPS(). If you want to move or animate it, use animators or the
	ISceneNode::setPosition(), ICameraSceneNode::setTarget() etc methods.
	By default, a camera's look at position (set with setTarget()) and its scene node
	rotation (set with setRotation()) are independent. If you want to be able to
	control the direction that the camera looks by using setRotation() then call
	ICameraSceneNode::bindTargetAndRotation(true) on it.
	\param position: Position of the space relative to its parent where the camera will be placed.
	\param lookat: Position where the camera will look at. Also known as target.
	\param parent: Parent scene node of the camera. Can be null. If the parent moves,
	the camera will move too.
	\param id: id of the camera. This id can be used to identify the camera.
	\param makeActive Flag whether this camera should become the active one.
	Make sure you always have one active camera.
	\return Pointer to interface to camera if successful, otherwise 0.
	This pointer should not be dropped. See IReferenceCounted::drop() for more information. */
	eastl::shared_ptr<Node> AddCameraNode(const ActorId actorId,
		WeakBaseRenderComponentPtr renderComponent, int id = -1, bool makeActive = true);

	//! Adds a billboard scene node to the scene graph.
	/** A billboard is like a 3d sprite: A 2d element,
	which always looks to the camera. It is usually used for things
	like explosions, fire, lensflares and things like that.
	\param parent Parent scene node of the billboard. Can be null.
	If the parent moves, the billboard will move too.
	\param size Size of the billboard. This size is 2 dimensional
	because a billboard only has width and height.
	\param position Position of the space relative to its parent
	where the billboard will be placed.
	\param id An id of the node. This id can be used to identify
	the node.
	\param colorTop The color of the vertices at the top of the
	billboard (default: white).
	\param colorBottom The color of the vertices at the bottom of
	the billboard (default: white).
	\return Pointer to the billboard if successful, otherwise NULL.
	This pointer should not be dropped. See
	IReferenceCounted::drop() for more information. */
	eastl::shared_ptr<Node> AddBillboardNode(const ActorId actorId,
		WeakBaseRenderComponentPtr renderComponent, const eastl::shared_ptr<Node>& parent, int id = -1, 
		const Vector2<float>& size = Vector2<float>{ 10.0f, 10.0f }, 
		const Vector3<float>& position = Vector3<float>{ 0, 0, 0 }, 
		eastl::array<float, 4> const colorTop = eastl::array<float, 4>{255.f, 255.f, 255.f, 255.f},
		eastl::array<float, 4> const colorBottom = eastl::array<float, 4>{255.f, 255.f, 255.f, 255.f});

	//! Adds a particle system scene node to the scene graph.
	/** \param withDefaultEmitter: Creates a default working point emitter
	which emitts some particles. Set this to true to see a particle system
	in action. If set to false, you'll have to set the emitter you want by
	calling IParticleSystemSceneNode::setEmitter().
	\param parent: Parent of the scene node. Can be NULL if no parent.
	\param id: Id of the node. This id can be used to identify the scene node.
	\param position: Position of the space relative to its parent where the
	scene node will be placed.
	\param rotation: Initital rotation of the scene node.
	\param scale: Initial scale of the scene node.
	\return Pointer to the created scene node.
	This pointer should not be dropped. See IReferenceCounted::drop() for more information. */
	eastl::shared_ptr<Node> AddParticleSystemNode(
		const ActorId actorId, WeakBaseRenderComponentPtr renderComponent,
		const eastl::shared_ptr<Node>& parent, int id = -1, bool withDefaultEmitter = true);

	//! Adds a skybox scene node to the scene graph.
	/** A skybox is a big cube with 6 textures on it and
	is drawn around the camera position.
	\param top: Texture for the top plane of the box.
	\param bottom: Texture for the bottom plane of the box.
	\param left: Texture for the left plane of the box.
	\param right: Texture for the right plane of the box.
	\param front: Texture for the front plane of the box.
	\param back: Texture for the back plane of the box.
	\param parent: Parent scene node of the skybox. A skybox usually has no parent,
	so this should be null. Note: If a parent is set to the skybox, the box will not
	change how it is drawn.
	\param id: An id of the node. This id can be used to identify the node.
	\return Pointer to the sky box if successful, otherwise NULL.
	This pointer should not be dropped. See IReferenceCounted::drop() for more information. */
	eastl::shared_ptr<Node> AddSkyBoxNode(const ActorId actorId,
		WeakBaseRenderComponentPtr renderComponent, const eastl::shared_ptr<Node>& parent,
		const eastl::shared_ptr<Texture2>& top, const eastl::shared_ptr<Texture2>& bottom, const eastl::shared_ptr<Texture2>& left,
		const eastl::shared_ptr<Texture2>& right, const eastl::shared_ptr<Texture2>& front, const eastl::shared_ptr<Texture2>& back,
		int id = -1);

	//! Adds a scene node for rendering an animated mesh model.
	/** \param mesh: Pointer to the loaded animated mesh to be displayed.
	\param parent: Parent of the scene node. Can be NULL if no parent.
	\param id: Id of the node. This id can be used to identify the scene node.
	\param position: Position of the space relative to its parent where the
	scene node will be placed.
	\param rotation: Initital rotation of the scene node.
	\param scale: Initial scale of the scene node.
	\param alsoAddIfMeshPointerZero: Add the scene node even if a 0 pointer is passed.
	\return Pointer to the created scene node.
	This pointer should not be dropped. See IReferenceCounted::drop() for more information. */
	eastl::shared_ptr<Node> AddAnimatedMeshNode(const ActorId actorId,
		WeakBaseRenderComponentPtr renderComponent, const eastl::shared_ptr<Node>& parent,
		eastl::shared_ptr<AnimatedMesh> mesh, int id = -1, bool alsoAddIfMeshPointerZero = false);

	//! Adds a scene node for rendering a static mesh.
	/** \param mesh: Pointer to the loaded static mesh to be displayed.
	\param parent: Parent of the scene node. Can be NULL if no parent.
	\param id: Id of the node. This id can be used to identify the scene node.
	\param position: Position of the space relative to its parent where the
	scene node will be placed.
	\param rotation: Initital rotation of the scene node.
	\param scale: Initial scale of the scene node.
	\param alsoAddIfMeshPointerZero: Add the scene node even if a 0 pointer is passed.
	\return Pointer to the created scene node.
	This pointer should not be dropped. See IReferenceCounted::drop() for more information. */
	eastl::shared_ptr<Node> AddMeshNode(const ActorId actorId,
		WeakBaseRenderComponentPtr renderComponent, const eastl::shared_ptr<Node>& parent,
		eastl::shared_ptr<Mesh> mesh, int id = -1, bool alsoAddIfMeshPointerZero = false);

	//! Adds a dynamic light scene node to the scene graph.
	/** The light will cast dynamic light on all
	other scene nodes in the scene, which have the material flag MTF_LIGHTING
	turned on. (This is the default setting in most scene nodes).
	\param parent: Parent scene node of the light. Can be null. If the parent moves,
	the light will move too.
	\param position: Position of the space relative to its parent where the light will be placed.
	\param color: Diffuse color of the light. Ambient or Specular colors can be set manually with
	the ILightSceneNode::getLightData() method.
	\param radius: Radius of the light.
	\param id: id of the node. This id can be used to identify the node.
	\return Pointer to the interface of the light if successful, otherwise NULL.
	This pointer should not be dropped. See IReferenceCounted::drop() for more information. */
	eastl::shared_ptr<Node> AddLightNode(const ActorId actorId,
		WeakBaseRenderComponentPtr renderComponent, const eastl::shared_ptr<Node>& parent,
		eastl::array<float, 4> const color = eastl::array<float, 4>{255.f, 255.f, 255.f, 255.f},
		float radius = 100.f);

	//! Gets the root scene node.
	/** This is the scene node which is parent
	of all scene nodes. The root scene node is a special scene node which
	only exists to manage all scene nodes. It will not be rendered and cannot
	be removed from the scene.
	\return Pointer to the root scene node.
	This pointer should not be dropped. See IReferenceCounted::drop() for more information. */
	virtual const eastl::shared_ptr<Node>& GetRootSceneNode() { return mRoot; }

	bool RemoveSceneNode(ActorId id);

	bool AddSceneNode(ActorId id, const eastl::shared_ptr<Node>& node);

	//! Get the first scene node with the specified id.
	/** \param id: The id to search for
	\param start: Scene node to start from. All children of this scene
	node are searched. If null is specified, the root scene node is
	taken.
	\return Pointer to the first scene node with this id,
	and null if no scene node could be found.
	This pointer should not be dropped. See IReferenceCounted::drop() for more information. */
	virtual eastl::shared_ptr<Node> GetSceneNode(ActorId id);

	//! Get the current active camera.
	/** \return The active camera is returned. Note that this can
	be NULL, if there was no camera created yet.
	This pointer should not be dropped. See IReferenceCounted::drop() for more information. */
	virtual const eastl::shared_ptr<Camera>& GetActiveCamera() const { return mCamera; }

	//! Sets the currently active camera.
	/** The previous active camera will be deactivated.
	\param camera: The new camera which should be active. */
	virtual void SetActiveCamera(const eastl::shared_ptr<Camera>& camera) { mCamera = camera; }

	//! Get current render time.
	E_RENDER_PASS GetCurrentRenderPass() const { return mCurrentRenderPass; }

	//! Set current render time.
	void SetCurrentRenderPass(E_RENDER_PASS currentRenderPass) { mCurrentRenderPass = currentRenderPass; }

	//! Sets ambient color of the scene
	void SetAmbientLight(const eastl::array<float, 4>& ambientColor) { mAmbientLight = ambientColor; }

	//! Returns ambient color of the scene
	const eastl::array<float, 4>& GetAmbientLight() const { return mAmbientLight; }

	//! Sets the color of stencil buffers shadows drawn by the scene manager.
	void SetShadowColor(eastl::array<float, 4> color) { mShadowColor = color; }

	//! Returns the current color of shadows.
	eastl::array<float, 4> GetShadowColor() const { return mShadowColor; }

	SceneNodeRenderList& GetDeletionList() { return mDeletionList; }
	SceneNodeRenderList& GetRenderList(unsigned int pass) { return mRenderList[pass]; }

	//! Adds a scene node to the render queue.
	void AddToRenderQueue(E_RENDER_PASS renderPass, const eastl::shared_ptr<Node>& node);

	//! clears the render list
	void ClearRenderList();

	//! Adds a scene node to the deletion queue.
	void AddToDeletionQueue(Node* node);

	//! clears the deletion list
	void ClearDeletionList();

	/* Check if node is culled in current view frustum. Please note that depending on the used 
	culling method this check can be rather coarse, or slow. A positive result is correct, though, 
	i.e. if this method returns true the node is positively not visible. The node might still be 
	invisible even if this method returns false.*/
	virtual bool IsCulled(Node* node);
};



//
// class ScreenElementScene						- Chapter X, page Y
//
class ScreenElementScene : public BaseScreenElement, public Scene
{
public:
	ScreenElementScene() : Scene() { }
    virtual ~ScreenElementScene(void)
    {
        LogWarning("~ScreenElementScene()");
    }

	// IScreenElement Implementation
	virtual bool OnInit() { return true; }
	virtual bool OnRestore() { Scene::OnRestore(); return true; }
	virtual bool OnLostDevice() { Scene::OnLostDevice(); return true; } 

	virtual void OnUpdate(int deltaMS) { Scene::OnUpdate(deltaMS); };
	virtual void OnAnimate(unsigned int uTime) { Scene::OnAnimate(uTime); };
	virtual bool OnRender(double time, float elapsedTime) { Scene::OnRender(); return true; }

	virtual int GetZOrder() const { return 0; }
	virtual void SetZOrder(int const zOrder) { }

	// Don't handle any messages
	virtual bool OnMsgProc( const Event& event ) { return false; }

	virtual bool IsVisible() { return true; }
	virtual void SetVisible(bool visible) { }
	virtual bool AddChild(ActorId id, const eastl::shared_ptr<Node>& kid) { return AddSceneNode(id, kid); }
};

#endif