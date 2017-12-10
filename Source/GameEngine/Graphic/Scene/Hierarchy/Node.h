// David Eberly, Geometric Tools, Redmond WA 98052
// Copyright (c) 1998-2017
// Distributed under the Boost Software License, Version 1.0.
// http://www.boost.org/LICENSE_1_0.txt
// http://www.geometrictools.com/License/Boost/LICENSE_1_0.txt
// File Version: 3.0.0 (2016/06/19)

#ifndef NODE_H
#define NODE_H

#include "Spatial.h"


// This class represents grouping nodes in a spatial hiearchy.
class GRAPHIC_ITEM Node : public Spatial
{
public:
    // Construction and destruction.
    virtual ~Node();
    Node();

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
    int AttachChild(eastl::shared_ptr<Spatial> const& child);

    // Detach a child from this node.  If the 'child' is non-null and in the
    // array, the return value is the index in the array that had stored the
    // child.  Otherwise, the function returns -1.
    int DetachChild(eastl::shared_ptr<Spatial> const& child);

    // Detach a child from this node.  If 0 <= i < GetNumChildren(), the
    // return value is the child at index i; otherwise, the function returns
    // null.
	eastl::shared_ptr<Spatial> DetachChildAt(int i);

    // Detach all children from this node.
    void DetachAllChildren();

    // The same comments for AttachChild apply here regarding the inability
    // to have multiple parents.  If 0 <= i < GetNumChildren(), the function
    // succeeds and returns i.  If i is out of range, the function *still*
    // succeeds, appending the child to the end of the array.  The return
    // value is the previous child stored at index i.
	eastl::shared_ptr<Spatial> SetChild(int i, eastl::shared_ptr<Spatial> const& child);

    // Get the child at the specified index.  If 0 <= i < GetNumChildren(),
    // the function succeeds and returns the child at that index.  Keep in
    // mind that child[i] could very well be null.  If i is out of range, the
    // function returns null.
	eastl::shared_ptr<Spatial> GetChild(int i);

protected:
    // Support for geometric updates.
    virtual void UpdateWorldData(double applicationTime);
    virtual void UpdateWorldBound();

    // Support for hierarchical culling.
    virtual void GetVisibleSet(Culler& culler,
		eastl::shared_ptr<Camera> const& camera, bool noCull);

    // Child pointers.
	eastl::vector<eastl::shared_ptr<Spatial>> mChild;
};

// Forward declarations
class SceneNodeAnimator;
class SceneNode;
class Scene;
class RayCast;
class MovementController;
class IResourceExtraData;
class ActorComponent;
class BaseRenderComponent;

// FUTURE WORK - Smart pointers don't work right....going to use a naked pointer for now!
typedef BaseRenderComponent* WeakBaseRenderComponentPtr;

////////////////////////////////////////////////////
//
// AlphaType					- Chapter X, page Y
//
//   This enum defines the different types of alpha blending
//   types that can be set on a scene node.
//
//	
////////////////////////////////////////////////////

enum AlphaType
{
	AlphaOpaque,
	AlphaTexture,
	AlphaMaterial,
	AlphaVertex
};


//////////////////////////////////////////////////////////////////////
//  class SceneNodeProperties	- Chapter 16, page 527
//
//   This class is contained in the SceneNode class, and gains
//   easy access to common scene node properties such as its ActorId
//   or render pass, with a single SceneNode::VGet() method.
//
//////////////////////////////////////////////////////////////////////

class SceneNodeProperties
{
	friend class SceneNode;

protected:

	int						m_Id;
	ActorId                 m_ActorId;
	stringc					m_Name;
	matrix4					m_ToWorld, m_FromWorld;
	f32						m_Radius;
	u32						m_RenderPass;
	Material				m_Material;
	//AlphaType				m_AlphaType;
	Vector3<float>				m_Position, m_Rotation, m_Scale;

	bool					m_IsVisible;
	E_SCENE_NODE_TYPE		m_Type;
	bool					m_IsDebugObject;
	u32						m_DebugDataVisible;
	u32						m_AutomaticCullingState;

public:
	SceneNodeProperties(void);
	const ActorId &ActorId() const { return m_ActorId; }
	matrix4 const &ToWorld() const { return m_ToWorld; }
	matrix4 const &FromWorld() const { return m_FromWorld; }
	void Transform(matrix4 *toWorld, matrix4 *fromWorld) const;

	int GetId() const { return m_Id; }
	void SetId(int id) { m_Id = id; }
	const stringc& GetName() const { return m_Name; }
	void SetName(const stringc& name) { m_Name = name; }

	//bool HasAlpha() const { return m_Material.HasAlpha(); }						
	//float Alpha() const { return m_Material.GetAlpha(); }
	//void SetAlpha(const float alpha) { m_AlphaType=AlphaMaterial; m_Material.SetAlpha(alpha); } 
	//AlphaType AlphaType() const { return m_AlphaType; }

	//! Returns type of the scene node
	virtual E_SCENE_NODE_TYPE GetType() const { return m_Type; }
	bool IsVisible() const { return m_IsVisible; }
	void SetVisible(bool visible) { m_IsVisible = visible; }
	void SetRadius(const float radius) { m_Radius = radius; }
	f32 GetRadius() const { return m_Radius; }
	u32 GetRenderPass() const { return m_RenderPass; }

	//! Enables or disables automatic culling based on the bounding box.
	void SetAutomaticCulling(u32 state) { m_AutomaticCullingState = state; }
	//! Gets the automatic culling state.
	u32 GetAutomaticCulling() const { return m_AutomaticCullingState; }
	//! Sets if debug data like bounding boxes should be drawn.
	virtual void SetDebugDataVisible(u32 state) { m_DebugDataVisible = state; }
	//! Returns if debug data like bounding boxes are drawn.
	u32 DebugDataVisible() const { return m_DebugDataVisible; }
	//! Sets if this scene node is a debug object.
	void SetIsDebugObject(bool debugObject) { m_IsDebugObject = debugObject; }
	//! Returns if this scene node is a debug object.
	bool IsDebugObject() const { return m_IsDebugObject; }

	Material& GetMaterial() { return m_Material; }
};

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

typedef eastl::vector<shared_ptr<SceneNode> > SceneNodeList;
typedef eastl::vector<shared_ptr<SceneNodeAnimator> > SceneNodeAnimatorList;

typedef eastl::vector<SceneNode*> SceneNodeRenderList;
typedef eastl::vector<SceneNodeAnimator*> SceneNodeAnimateList;


//////////////////////////////////////////////////////////////
//
// SceneNode							- Chapter 16, page 529
//
//   Implements SceneNode. Forms the base class for any node
//   that can exist in the 3D scene graph managed by class Scene.
//
//////////////////////////////////////////////////////////////

class SceneNode : public std::enable_shared_from_this<SceneNode>
{
	friend class Scene;

protected:

	shared_ptr<SceneNode>			m_pParent;
	shared_ptr<ITriangleSelector>	m_TriangleSelector;
	SceneNodeAnimatorList			m_Animators;
	SceneNodeList					m_Children;
	SceneNodeProperties				m_Props;
	WeakBaseRenderComponentPtr		m_RenderComponent;


public:
	SceneNode(ActorId actorId, WeakBaseRenderComponentPtr renderComponent,
		E_RENDER_PASS renderPass, E_SCENE_NODE_TYPE nodeType,
		const matrix4 *to, const matrix4 *from = NULL);

	virtual ~SceneNode();

	virtual SceneNodeProperties* Get() { return &m_Props; }

	virtual void SetTransform(const matrix4 *toWorld, const matrix4 *fromWorld = NULL);

	virtual bool OnRestore(Scene *pScene);
	virtual bool OnLostDevice(Scene *pScene);

	virtual bool OnUpdate(Scene *, unsigned long const elapsedMs);
	virtual bool OnAnimate(Scene* pScene, u32 timeMs);

	virtual bool PreRender(Scene *pScene);
	virtual bool Render(Scene *pScene) { return true; }
	virtual bool RenderChildren(Scene *pScene);
	virtual bool PostRender(Scene *pScene);

	virtual bool AddChild(const shared_ptr<SceneNode>& kid);
	virtual bool RemoveChild(ActorId id);
	virtual shared_ptr<SceneNode> GetChild(ActorId id);
	virtual const SceneNodeList& GetChildren() const { return m_Children; }

	virtual bool AddAnimator(const shared_ptr<SceneNodeAnimator>& animator);
	virtual bool RemoveAnimator(const shared_ptr<SceneNodeAnimator>& animator);
	//! Get a list of all scene node animators.
	virtual const SceneNodeAnimatorList& GetAnimators() const { return m_Animators; }

	virtual void SetParent(const shared_ptr<SceneNode>& newParent);
	virtual const shared_ptr<SceneNode>& GetParent() { return m_pParent; }
	virtual void Remove();

	virtual bool Pick(Scene *pScene, RayCast *pRayCast);

	virtual const AABBox3<float>& GetBoundingBox() const { return *((AABBox3<float>*)0); }

	//! Returns the material based on the zero based index i.
	/** To get the amount of materials used by this scene node, use
	getMaterialCount(). This function is needed for inserting the
	node into the scene hierarchy at an optimal position for
	minimizing renderstate changes, but can also be used to
	directly modify the material of a scene node.
	\param num Zero based index. The maximal value is getMaterialCount() - 1.
	\return The material at that index. */
	virtual Material& GetMaterial(u32 num)
	{
		return m_Props.GetMaterial();
	}


	//! Get amount of materials used by this scene node.
	/** \return Current amount of materials of this scene node. */
	virtual u32 GetMaterialCount() const
	{
		return 1;
	}

	//void SetMaterial(const Material &mat) { m_Props.m_Material = mat; }

	//! Sets all material flags at once to a new value.
	/** Useful, for example, if you want the whole mesh to be
	affected by light.
	\param flag Which flag of all materials to be set.
	\param newvalue New value of that flag. */
	void SetMaterialFlag(E_MATERIAL_FLAG flag, bool newvalue)
	{
		for (u32 i = 0; i<GetMaterialCount(); ++i)
			GetMaterial(i).SetFlag(flag, newvalue);
	}


	//! Sets the texture of the specified layer in all materials of this scene node to the new texture.
	/** \param textureLayer Layer of texture to be set. Must be a
	value smaller than MATERIAL_MAX_TEXTURES.
	\param texture New texture to be used. */
	void SetMaterialTexture(u32 textureLayer, ITexture* texture)
	{
		if (textureLayer >= MATERIAL_MAX_TEXTURES)
			return;

		for (u32 i = 0; i<GetMaterialCount(); ++i)
			GetMaterial(i).SetTexture(textureLayer, texture);
	}


	//! Sets the material type of all materials in this scene node to a new material type.
	/** \param newType New type of material to be set. */
	void SetMaterialType(E_MATERIAL_TYPE newType)
	{
		for (u32 i = 0; i<GetMaterialCount(); ++i)
			GetMaterial(i).MaterialType = newType;
	}

	//! Returns the triangle selector attached to this scene node.
	/** The Selector can be used by the engine for doing collision
	detection. You can create a TriangleSelector with
	ISceneManager::createTriangleSelector() or
	ISceneManager::createOctreeTriangleSelector and set it with
	ISceneNode::setTriangleSelector(). If a scene node got no triangle
	selector, but collision tests should be done with it, a triangle
	selector is created using the bounding box of the scene node.
	\return A pointer to the TriangleSelector or 0, if there
	is none. */
	virtual const shared_ptr<ITriangleSelector>& GetTriangleSelector() const
	{
		return m_TriangleSelector;
	}


	//! Sets the triangle selector of the scene node.
	/** The Selector can be used by the engine for doing collision
	detection. You can create a TriangleSelector with
	ISceneManager::createTriangleSelector() or
	ISceneManager::createOctreeTriangleSelector(). Some nodes may
	create their own selector by default, so it would be good to
	check if there is already a selector in this node by calling
	ISceneNode::getTriangleSelector().
	\param selector New triangle selector for this scene node. */
	virtual void SetTriangleSelector(const shared_ptr<ITriangleSelector>& selector)
	{
		m_TriangleSelector = selector;
	}


	//void SetAlpha(float alpha);
	//float GetAlpha() const { return m_Props.Alpha(); }

	//! Gets the scale of the scene node relative to its parent.
	virtual const Vector3<float>& GetScale() const { return m_Props.m_Scale; }
	//! Sets the relative scale of the scene node.
	virtual void SetScale(const Vector3<float>& scale) { m_Props.m_Scale = scale; }
	//! Gets the rotation of the node relative to its parent.
	virtual const Vector3<float>& GetRotation() const { return m_Props.m_Rotation; }
	//! Sets the rotation of the node relative to its parent.
	virtual void SetRotation(const Vector3<float>& rotation) { m_Props.m_Rotation = rotation; }
	//! Gets the position of the node relative to its parent.
	virtual const Vector3<float>& GetPosition() const { return m_Props.m_Position; }
	//! Sets the position of the node relative to its parent.
	virtual void SetPosition(const Vector3<float>& position) { m_Props.m_Position = position; }

	//! Returns the relative transformation of the scene node.
	/** The relative transformation is stored internally as 3
	vectors: translation, rotation and scale. To get the relative
	transformation matrix, it is calculated from these values.
	\return The relative transformation matrix. */
	virtual matrix4 GetRelativeTransformation() const
	{
		matrix4 mat;
		mat.SetRotationDegrees(m_Props.m_Rotation);
		mat.SetTranslation(m_Props.m_Position);

		if (m_Props.m_Scale != Vector3<float>(1.f, 1.f, 1.f))
		{
			matrix4 smat;
			smat.SetScale(m_Props.m_Scale);
			mat *= smat;
		}

		return mat;
	}

	//! Updates transformation matrix of the node based on the relative and the parents transformation
	virtual void UpdateAbsoluteTransformation();

protected:

	void RemoveAll();

	void SortRenderList(Scene* pScene);

	struct DefaultNodeEntry
	{
		DefaultNodeEntry(SceneNode* n) :
			Node(n), TextureValue(0)
		{
			if (n->GetMaterialCount())
				TextureValue = (n->GetMaterial(0).GetTexture(0));
		}

		bool operator < (const DefaultNodeEntry& other) const
		{
			return (TextureValue < other.TextureValue);
		}

		SceneNode* Node;

	private:
		void* TextureValue;
	};

	//! sort on distance (center) to camera
	struct TransparentNodeEntry
	{
		TransparentNodeEntry(SceneNode* n, const Vector3<float>& cameraPos)
			: Node(n)
		{
			Distance = Node->Get()->ToWorld().GetTranslation().GetDistanceFromSQ(cameraPos);
		}

		bool operator < (const TransparentNodeEntry& other) const
		{
			return Distance > other.Distance;
		}

		SceneNode* Node;

	private:

		f64 Distance;
	};

	//! sort on distance (sphere) to camera
	struct DistanceNodeEntry
	{
		DistanceNodeEntry(SceneNode* n, const Vector3<float>& cameraPos)
			: Node(n)
		{
			SetNodeAndDistanceFromPosition(n, cameraPos);
		}

		bool operator < (const DistanceNodeEntry& other) const
		{
			return Distance < other.Distance;
		}

		void SetNodeAndDistanceFromPosition(SceneNode* n, const Vector3<float>& fromPosition)
		{
			Node = n;
			Distance = Node->Get()->ToWorld().GetTranslation().GetDistanceFromSQ(fromPosition);
			Distance -= Node->GetBoundingBox().GetExtent().GetLengthSQ() * 0.5;
		}

		SceneNode* Node;

	private:

		f64 Distance;
	};


};

//! Animates a scene node. Can animate position, rotation, material, and so on.
/** A scene node animator is able to animate a scene node in a very simple way. It may
change its position, rotation, scale and/or material. There are lots of animators
to choose from. You can create scene node animators with the ISceneManager interface. */
class SceneNodeAnimator
{
public:
	//! Animates a scene node.
	/** \param node Node to animate.
	\param timeMs Current time in milli seconds. */
	virtual void AnimateNode(SceneNode* node, u32 timeMs) = 0;

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
	virtual E_SCENE_NODE_ANIMATOR_TYPE GetType() const
	{
		return ESNAT_UNKNOWN;
	}

	//! Returns if the animator has finished.
	/** This is only valid for non-looping animators with a discrete end state.
	\return true if the animator has finished, false if it is still running. */
	virtual bool HasFinished(void) const
	{
		return false;
	}
};


////////////////////////////////////////////////////
//
// AlphaSceneNode Description						- Chapter 16, page 535
// AlphaSceneNodes Description						- Chapter 16, page 535
//
// A list of scene nodes that need to be drawn in the alpha pass;
// the list is defined as an STL list
////////////////////////////////////////////////////

struct AlphaSceneNode
{
	shared_ptr<SceneNode> m_pNode;
	matrix4 m_Concat;
	float m_ScreenZ;

	// For the STL sort...
	bool const operator <(AlphaSceneNode const &other) { return m_ScreenZ < other.m_ScreenZ; }
};

typedef eastl::list<AlphaSceneNode *> AlphaSceneNodes;


////////////////////////////////////////////////////
//
// Scene Description
//
// A heirarchical container of scene nodes, which
// are classes that implemente the SceneNode interface
//
////////////////////////////////////////////////////

//class CameraSceneNode;
//class SkyNode;


////////////////////////////////////////////////////
//
// class RootNode					- Chapter 16, page 545
//
//    This is the root node of the scene graph.
//
////////////////////////////////////////////////////

class RootNode : public SceneNode
{
public:
	RootNode();
	virtual const AABBox3<float>& GetBoundingBox() const;

	virtual bool PreRender(Scene *pScene);
	virtual bool Render(Scene *pScene);
	virtual bool RenderChildren(Scene *pScene);
	virtual bool PostRender(Scene *pScene);

};

#endif