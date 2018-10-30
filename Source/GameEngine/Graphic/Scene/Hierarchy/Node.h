// David Eberly, Geometric Tools, Redmond WA 98052
// Copyright (c) 1998-2017
// Distributed under the Boost Software License, Version 1.0.
// http://www.boost.org/LICENSE_1_0.txt
// http://www.geometrictools.com/License/Boost/LICENSE_1_0.txt
// File Version: 3.0.0 (2016/06/19)

#ifndef NODE_H
#define NODE_H

#include "Visual.h"
#include "Spatial.h"
#include "PVWUpdater.h"

#include "Graphic/Effect/Material.h"
#include "Graphic/Resource/Texture/Texture2.h"
#include "Graphic/Scene/Element/Mesh/Mesh.h"

#include "Application/System/EventSystem.h"

//! An enumeration for all types of built-in scene node animators
enum GRAPHIC_ITEM NodeAnimatorType
{
	//! Fly circle scene node animator
	NAT_FLY_CIRCLE = 0,

	//! Fly straight scene node animator
	NAT_FLY_STRAIGHT,

	//! Follow camera scene node animator
	NAT_FOLLOW_CAMERA,

	//! Follow spline scene node animator
	NAT_FOLLOW_SPLINE,

	//! Rotation scene node animator
	NAT_ROTATION,

	//! Texture scene node animator
	NAT_TEXTURE,

	//! Deletion scene node animator
	NAT_DELETION,

	//! Amount of built-in scene node animators
	NAT_COUNT,

	//! Unknown scene node animator
	NAT_UNKNOWN,

	//! This enum is never used, it only forces the compiler to compile this enumeration to 32 bit.
	NAT_FORCE_32_BIT = 0x7fffffff
};

//! An enumeration for all types of built-in scene nodes
/** A scene node type is represented by a four character code
such as 'cube' or 'mesh' instead of simple numbers, to avoid
name clashes with external scene nodes.*/
enum GRAPHIC_ITEM NodeType
{
	//! Root scene Node
	NT_ROOT,

	//! simple object scene node
	NT_SHAPE,

	//! simple cube scene node
	NT_CUBE,

	//! Sphere scene node
	NT_SPHERE,

	//! Text Scene Node
	NT_TEXT,

	//! Water Surface Scene Node
	NT_WATER_SURFACE,

	//! Terrain Scene Node
	NT_TERRAIN,

	//! Sky Scene Node
	NT_SKY,

	//! Shadow Volume Scene Node
	NT_SHADOW_VOLUME,

	//! Octree Scene Node
	NT_OCTREE,

	//! Mesh Scene Node
	NT_MESH,

	//! Light Scene Node
	NT_LIGHT,

	//! Empty Scene Node
	NT_EMPTY,

	//! Dummy Transformation Scene Node
	NT_DUMMY_TRANSFORMATION,

	//! Camera Scene Node
	NT_CAMERA,

	//! Billboard Scene Node
	NT_BILLBOARD,

	//! Animated Mesh Scene Node
	NT_ANIMATED_MESH,

	//! Particle System Scene Node
	NT_PARTICLE_SYSTEM,

	//! Quake3 Shader Scene Node
	NT_Q3SHADER,

	//! Quake3 Model Scene Node ( has tag to link to )
	NT_MD3,

	//! Volume Light Scene Node
	NT_VOLUME_LIGHT,

	//! First Person Shooter Camera
	/** Legacy, for loading version <= 1.4.x .irr files */
	NT_CAMERA_FPS,

	//! Unknown scene node
	NT_UNKNOWN,

	//! Will match with any scene node when checking types
	NT_ANY
};


////////////////////////////////////////////////////
//
// AlphaType					- Chapter X, page Y
//
//   This enum defines the different types of alpha blending
//   types that can be set on a scene node.
//
//	
////////////////////////////////////////////////////

/////////////////////////////////////////////////////////////////////////////
// enum RenderPass							- Chapter 16, page 529
//
//   3D scenes are drawn in passes - this enum defines the render passes
//   supported by the 3D scene graph created by class Scene.
/////////////////////////////////////////////////////////////////////////////

//! Enumeration for render passes.
/** A parameter passed to the registerNodeForRendering() method of the ISceneManager,
specifying when the node wants to be drawn in relation to the other nodes. */
enum GRAPHIC_ITEM RenderPass
{
	//! No pass currently active
	RP_NONE = 0,

	//! Camera pass. The active view is set up here. The very first pass.
	RP_CAMERA,

	//! Static nodes
	RP_STATIC,

	//! This is used for skies.
	RP_SKY,

	//! Solid scene nodes or special scene nodes without materials.
	RP_SOLID,

	//! In this pass, lights are transformed into camera space and added to the driver
	RP_LIGHT,

	//! Transparent scene nodes, drawn after solid nodes. They are sorted from back to front and drawn in that order.
	RP_TRANSPARENT,

	//! Transparent effect scene nodes, drawn after Transparent nodes. They are sorted from back to front and drawn in that order.
	RP_TRANSPARENT_EFFECT,

	//! Drawn after the solid nodes, before the transparent nodes, the time for drawing shadow volumes
	RP_SHADOW,

	//! Last render pass, it doesnt do anything
	RP_LAST
};

// Forward declarations
class Scene;
class Node;
class NodeAnimator;
class BaseRenderComponent;

// FUTURE WORK - Smart pointers don't work right....going to use a naked pointer for now!
typedef eastl::weak_ptr<BaseRenderComponent> WeakBaseRenderComponentPtr;

//////////////////////////////////////////////////////////////
//
// SceneNodeList						- Chapter 16, page 529
//
//   Every scene node has a list of its children - this 
//   is implemented as a vector since it is expected that
//   we won't add/delete nodes very often, and we'll want 
//   fast random access to each child.
//
//////////////////////////////////////////////////////////////

typedef eastl::vector<eastl::shared_ptr<Node>> SceneNodeList;
typedef eastl::vector<eastl::shared_ptr<NodeAnimator>> SceneNodeAnimatorList;

typedef eastl::vector<Node*> SceneNodeRenderList;
typedef eastl::vector<NodeAnimator*> SceneNodeAnimateList;


// This class represents grouping nodes in a spatial hiearchy.
class GRAPHIC_ITEM Node : public Spatial, public eastl::enable_shared_from_this<Node>
{
public:

	Node(int id, WeakBaseRenderComponentPtr renderComponent, NodeType nodeType);

	virtual ~Node();

	//! Returns type of the scene node
	int GetId() const { return mId; }
	void SetId(int id) { mId = id; }
	const eastl::string& GetName() const { return mName; }
	void SetName(const eastl::string& name) { mName = name; }

	virtual NodeType GetType() const { return mType; }

	bool OnRestore(Scene *pScene);
	bool OnLostDevice(Scene *pScene);

	bool OnUpdate(Scene *, unsigned int timeMs, unsigned long const elapsedMs);
	virtual bool OnAnimate(Scene* pScene, unsigned int timeMs);

	virtual bool PreRender(Scene *pScene);
	virtual bool Render(Scene *pScene) { return true; }
	
	bool RenderChildren(Scene *pScene);
	bool PostRender(Scene *pScene);

    // This is the current number of elements in the child array.  These
    // elements are not all guaranteed to be non-null.  Thus, when you
    // iterate over the array and access children with GetChild(...), you
    // should verify the child pointer is not null before dereferencing it.
    int GetNumChildren() const;

    // Attach a child to this node.  If the function succeeds, the return
    // value is the index i of the array where the child was stored, in which
    // case 0 <= i < GetNumChildren().  The first available empty slot of the
    // child array is used for storage.  If all slots are filled, the child
    // is appended to the array (potentially causing a reallocation of the
    // array).
    // 
    // The function fails when 'child' is null or when 'child' already has a
    // parent, in which case the return value is -1.  The nodes form a tree,
    // not a more general directed acyclic graph.  A consequence is that a
    // node cannot have more than one parent.  For example,
    //     Node* node0 = <some node>;
    //     Spatial* child = <some child>;
    //     int index = node0->AttachChild(child);
    //     Node* node1 = <some node>;
    //
    //     // This asserts because 'child' already has a parent (node0).
    //     node1->AttachChild(child);
    //
    //     // The following is the correct way to give 'child' a new parent.
    //     node0->DetachChild(child);  // or node0->DetachChildAt(index);
    //     node1->AttachChild(child);
    //
    //     // In the last example before the DetachChild call, if 'child' is
    //     // referenced only by node0, the detach will cause 'child' to be
    //     // deleted (Node internally reference counts its children).  If
    //     // you want to keep 'child' around for later use, do the following.
    //     Spatial::SP saveChild = SPCreate(node0->GetChild(0));
    //     node0->DetachChild(saveChild);
    //     node1->AttachChild(saveChild);
    int AttachChild(eastl::shared_ptr<Node> const& child);

    // Detach a child from this node.  If the 'child' is non-null and in the
    // array, the return value is the index in the array that had stored the
    // child.  Otherwise, the function returns -1.
    virtual int DetachChild(eastl::shared_ptr<Node> const& child);

	// Detach a child from this node.  If 0 <= i < GetNumChildren(), the
	// return value is the child at index i; otherwise, the function returns
	// null.
	eastl::shared_ptr<Node> DetachChildAt(int i);

    // Detach all children from this node.
    void DetachAllChildren();

    // The same comments for AttachChild apply here regarding the inability
    // to have multiple parents.  If 0 <= i < GetNumChildren(), the function
    // succeeds and returns i.  If i is out of range, the function *still*
    // succeeds, appending the child to the end of the array.  The return
    // value is the previous child stored at index i.
	eastl::shared_ptr<Node> SetChild(int i, eastl::shared_ptr<Node> const& child);

    // Get the child at the specified index.  If 0 <= i < GetNumChildren(),
    // the function succeeds and returns the child at that index.  Keep in
    // mind that child[i] could very well be null.  If i is out of range, the
    // function returns null.
	eastl::shared_ptr<Node> GetChild(int i);

	const SceneNodeList& GetChildren() const { return mChildren; }

	int AttachAnimator(eastl::shared_ptr<NodeAnimator> const& animator);

	int DetachAnimator(eastl::shared_ptr<NodeAnimator> const& animator);

	eastl::shared_ptr<NodeAnimator> DetachAnimatorAt(int i);

	// Detach all animators from this node.
	void DetachAllAnimators();

	eastl::shared_ptr<NodeAnimator> SetAnimator(int i, eastl::shared_ptr<NodeAnimator> const& animator);

	eastl::shared_ptr<NodeAnimator> GetAnimator(int i);

	const SceneNodeAnimatorList& GetAnimators() const { return mAnimators; }

	//! Returns the visual based on the zero based index i.
	/** To get the amount of visuals used by this scene node, use
	GetVisualCount(). This function is needed for inserting the
	node into the scene hierarchy at an optimal position for
	minimizing renderstate changes, but can also be used to
	directly modify the visual of a scene node.
	\param num Zero based index. The maximal value is GetVisualCount() - 1.
	\return The visual at that index. */
	virtual eastl::shared_ptr<Visual> const& GetVisual(unsigned int i){ return nullptr; }

	//! Get amount of visuals used by this scene node.
	/** \return Current amount of visuals of this scene node. */
	virtual unsigned int GetVisualCount() const { return 0; }

	//! Returns the material based on the zero based index i.
	/** To get the amount of materials used by this scene node, use
	getMaterialCount(). This function is needed for inserting the
	node into the scene hierarchy at an optimal position for
	minimizing renderstate changes, but can also be used to
	directly modify the material of a scene node.
	\param num Zero based index. The maximal value is getMaterialCount() - 1.
	\return The material at that index. */
	virtual eastl::shared_ptr<Material> const& GetMaterial(unsigned int i) { return nullptr; }

	//! Get amount of materials used by this scene node.
	/** \return Current amount of materials of this scene node. */
	virtual unsigned int GetMaterialCount() const { return 0; }

	//! Sets the texture of the specified layer in all materials of this scene node to the new texture.
	/** \param textureLayer Layer of texture to be set. Must be a
	value smaller than MATERIAL_MAX_TEXTURES.
	\param texture New texture to be used. */
	virtual void SetMaterialTexture(unsigned int textureLayer, eastl::shared_ptr<Texture2> texture) { }

	//! Sets the material type of all materials in this scene node to a new material type.
	/** \param newType New type of material to be set. */
	virtual void SetMaterialType(MaterialType newType) { }

protected:

	void SortRenderList(Scene* pScene);

	struct DefaultNodeEntry
	{
		DefaultNodeEntry(Node* n) :
			mNode(n), mTextureValue(0)
		{
			/*
			if (n->GetMaterialCount())
				mTextureValue = (n->GetMaterial(0).GetTexture(0));
			*/
		}

		bool operator < (const DefaultNodeEntry& other) const
		{
			return (mTextureValue < other.mTextureValue);
		}

		Node* mNode;

	private:
		void* mTextureValue;
	};

	//! sort on distance (center) to camera
	struct TransparentNodeEntry
	{
		TransparentNodeEntry(Node* n, const Vector4<float>& cameraPos)
			: mNode(n)
		{
			//mDistance = mNode->Get()->ToWorld().GetTranslation().GetDistanceFromSQ(cameraPos);
		}

		bool operator < (const TransparentNodeEntry& other) const
		{
			return mDistance > other.mDistance;
		}

		Node* mNode;

	private:

		float mDistance;
	};

	//! sort on distance (sphere) to camera
	struct DistanceNodeEntry
	{
		DistanceNodeEntry(Node* n, const Vector4<float>& cameraPos)
			: mNode(n)
		{
			SetNodeAndDistanceFromPosition(n, cameraPos);
		}

		bool operator < (const DistanceNodeEntry& other) const
		{
			return mDistance < other.mDistance;
		}

		void SetNodeAndDistanceFromPosition(Node* n, const Vector4<float>& fromPosition)
		{
			mNode = n;
			/*
			mDistance = mNode->Get()->ToWorld().GetTranslation().GetDistanceFromSQ(fromPosition);
			mDistance -= mNode->GetBoundingBox().GetExtent().GetLengthSQ() * 0.5;
			*/
		}

		Node* mNode;

	private:

		float mDistance;
	};

protected:

    // Support for geometric updates.
    virtual void UpdateWorldData();
    virtual void UpdateWorldBound();

    // Support for hierarchical culling.
    virtual void GetVisibleSet(Culler& culler,
		eastl::shared_ptr<Camera> const& camera, bool noCull);

	int mId;
	eastl::string mName;

	NodeType mType;
	PVWUpdater* mPVWUpdater;

	SceneNodeList mChildren;
	SceneNodeAnimatorList mAnimators;

	WeakBaseRenderComponentPtr mRenderComponent;
};


//! Animates a scene node. Can animate position, rotation, material, and so on.
/** A scene node animator is able to animate a scene node in a very simple way. It may
change its position, rotation, scale and/or material. There are lots of animators
to choose from. You can create scene node animators with the SceneManager interface. */
class GRAPHIC_ITEM NodeAnimator
{
public:
	//! Animates a scene node.
	/** \param node Node to animate.
	\param timeMs Current time in milli seconds. */
	virtual void AnimateNode(Scene* pScene, Node* node, unsigned int timeMs) = 0;

	//! Returns true if this animator receives events.
	/** When attached to an active camera, this animator will be
	able to respond to events such as mouse and keyboard events. */
	virtual bool IsEventReceiverEnabled() const
	{
		return false;
	}

	//! Event receiver, override this function for camera controlling animators
	virtual bool OnEvent(const Event& ev)
	{
		return false;
	}

	//! Returns type of the scene node animator
	virtual NodeAnimatorType GetType() const
	{
		return NAT_UNKNOWN;
	}

	//! Returns if the animator has finished.
	/** This is only valid for non-looping animators with a discrete end state.
	\return true if the animator has finished, false if it is still running. */
	virtual bool HasFinished(void) const
	{
		return false;
	}
};


#endif