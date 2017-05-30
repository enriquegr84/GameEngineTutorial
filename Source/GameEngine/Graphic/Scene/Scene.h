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

//#include "Lights.h"

//#include "Events/EventManager.h"

class SceneNode;
class CameraSceneNode;

// Forward declarations
////////////////////////////////////////////////////
//
// SceneActorMap Description				- Chapter 16, page Y
//
//   An STL map that allows fast lookup of a scene node given an ActorId.
//
////////////////////////////////////////////////////

typedef eastl::map<ActorId, eastl::shared_ptr<SceneNode> > SceneNodeActorMap;
typedef eastl::vector<SceneNode*> SceneNodeRenderList;


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

class Scene
{
protected:
	eastl::shared_ptr<SceneNode>  					m_pRoot;
	eastl::shared_ptr<CameraSceneNode> 				m_pCamera;

	SceneNodeActorMap 						m_SceneNodeActors;		
	E_RENDER_PASS							m_CurrentRenderPass;
	
	eastl::array<float, 4>					m_ShadowColor;
	eastl::array<float, 4>					m_AmbientLight;

	//! scene node lists
	SceneNodeRenderList m_DeletionList;
	SceneNodeRenderList m_RenderList[ERP_LAST];

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

	//! Gets the root scene node.
	/** This is the scene node which is parent
	of all scene nodes. The root scene node is a special scene node which
	only exists to manage all scene nodes. It will not be rendered and cannot
	be removed from the scene.
	\return Pointer to the root scene node.
	This pointer should not be dropped. See IReferenceCounted::drop() for more information. */
	virtual const eastl::shared_ptr<SceneNode>& GetRootSceneNode() { return m_pRoot; }

	bool RemoveSceneNode(ActorId id);

	bool AddSceneNode(ActorId id, const eastl::shared_ptr<SceneNode>& node);

	//! Get the first scene node with the specified id.
	/** \param id: The id to search for
	\param start: Scene node to start from. All children of this scene
	node are searched. If null is specified, the root scene node is
	taken.
	\return Pointer to the first scene node with this id,
	and null if no scene node could be found.
	This pointer should not be dropped. See IReferenceCounted::drop() for more information. */
	virtual eastl::shared_ptr<SceneNode> GetSceneNode(ActorId id);

	//! Get the current active camera.
	/** \return The active camera is returned. Note that this can
	be NULL, if there was no camera created yet.
	This pointer should not be dropped. See IReferenceCounted::drop() for more information. */
	virtual const eastl::shared_ptr<CameraSceneNode>& GetActiveCamera() const { return m_pCamera; }

	//! Sets the currently active camera.
	/** The previous active camera will be deactivated.
	\param camera: The new camera which should be active. */
	virtual void SetActiveCamera(const eastl::shared_ptr<CameraSceneNode>& camera) { m_pCamera = camera; }

	//! Get current render time.
	E_RENDER_PASS GetCurrentRenderPass() const { return m_CurrentRenderPass; }

	//! Set current render time.
	void SetCurrentRenderPass(E_RENDER_PASS currentRenderPass) { m_CurrentRenderPass = currentRenderPass; }

	//! Sets ambient color of the scene
	void SetAmbientLight(const eastl::array<float, 4>& ambientColor) { m_AmbientLight = ambientColor; }

	//! Returns ambient color of the scene
	const eastl::array<float, 4>& GetAmbientLight() const { return m_AmbientLight; }

	//! Sets the color of stencil buffers shadows drawn by the scene manager.
	void SetShadowColor(eastl::array<float, 4> color) { m_ShadowColor = color; }

	//! Returns the current color of shadows.
	eastl::array<float, 4> GetShadowColor() const { return m_ShadowColor; }

	SceneNodeRenderList& GetDeletionList() { return m_DeletionList; }
	SceneNodeRenderList& GetRenderList(unsigned int pass) { return m_RenderList[pass]; }

	//! Adds a scene node to the render queue.
	void AddToRenderQueue(E_RENDER_PASS renderPass, const eastl::shared_ptr<SceneNode>& node);

	//! clears the render list
	void ClearRenderList();

	//! Adds a scene node to the deletion queue.
	void AddToDeletionQueue(SceneNode* node);

	//! clears the deletion list
	void ClearDeletionList();

	/* Check if node is culled in current view frustum. Please note that depending on the used 
	culling method this check can be rather coarse, or slow. A positive result is correct, though, 
	i.e. if this method returns true the node is positively not visible. The node might still be 
	invisible even if this method returns false.*/
	virtual bool IsCulled(SceneNode* node);
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
	virtual bool AddChild(ActorId id, const eastl::shared_ptr<SceneNode>& kid) { return AddSceneNode(id, kid); }
};

#endif