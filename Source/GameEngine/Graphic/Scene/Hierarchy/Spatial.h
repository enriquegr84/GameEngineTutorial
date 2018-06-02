// David Eberly, Geometric Tools, Redmond WA 98052
// Copyright (c) 1998-2017
// Distributed under the Boost Software License, Version 1.0.
// http://www.boost.org/LICENSE_1_0.txt
// http://www.geometrictools.com/License/Boost/LICENSE_1_0.txt
// File Version: 3.0.0 (2016/06/19)

#ifndef SPATIAL_H
#define SPATIAL_H

#include "Graphic/Scene/Controller/ControlledObject.h"
#include "Graphic/Scene/Visibility/Culler.h"

#include <EASTL/string.h>

class Camera;

//! An enumeration for all types of debug data for built-in scene nodes (flags)
enum GRAPHIC_ITEM DebugSceneType
{
	//! No Debug Data ( Default )
	DS_OFF = 0,

	//! Show Bounding Boxes of SceneNode
	DS_BBOX = 1,

	//! Show Vertex Normals
	DS_NORMALS = 2,

	//! Shows Skeleton/Tags
	DS_SKELETON = 4,

	//! Overlays Mesh Wireframe
	DS_MESH_WIRE_OVERLAY = 8,

	//! Temporary use transparency Material Type
	DS_HALF_TRANSPARENCY = 16,

	//! Show Bounding Boxes of all MeshBuffers
	DS_BBOX_BUFFERS = 32,

	//! DS_BBOX | DS_BBOX_BUFFERS
	DS_BBOX_ALL = DS_BBOX | DS_BBOX_BUFFERS,

	//! Show all debug infos
	DS_FULL = 0xffffffff
};

// Support for a spatial hierarchy of objects.  Class Spatial has a parent
// pointer.  Class Node derives from Spatial has an array of child pointers.
// The leaf nodes of the hierarchy are either graphical or audial.  Class
// Visual derives from Spatial and represents graphical data.  Class Audial
// derives from Spatial and represents sound data.


class GRAPHIC_ITEM Spatial : public ControlledObject
{
public:
    // Abstract base class.
    virtual ~Spatial();

    // Update of geometric state.  The function computes world transformations
    // on the downward pass of the scene graph traversal and world bounding
    // volumes on the upward pass of the traversal.  The object that calls the
    // update is the initiator.  Other objects visited during the update are
    // not initiators.  The application time is in milliseconds.
    void Update(double applicationTime = 0.0, bool initiator = true);

    // Access to the parent object, which is null for the root of the
    // hierarchy.
	Spatial* GetParent();

	bool IsVisible() const { return mIsVisible; }
	void SetVisible(bool visible) { mIsVisible = visible; }

	//! Enables or disables automatic culling based on the bounding box.
	void SetAutomaticCulling(unsigned int state) { mAutomaticCullingState = state; }
	//! Gets the automatic culling state.
	unsigned int GetAutomaticCulling() const { return mAutomaticCullingState; }
	//! Sets if debug data like bounding boxes should be drawn.
	void SetDebugDataVisible(unsigned int state) { mDebugDataVisible = state; }
	//! Returns if debug data like bounding boxes are drawn.
	unsigned int DebugDataVisible() const { return mDebugDataVisible; }
	//! Sets if this spatial node is a debug object.
	void SetIsDebugObject(bool debugObject) { mIsDebugObject = debugObject; }
	//! Returns if this spatial node is a debug object.
	bool IsDebugObject() const { return mIsDebugObject; }

	//! Returns the relative transformation of the spatial node.
	/** The relative transformation is stored internally as 3
	vectors: translation, rotation and scale. To get the relative
	transformation matrix, it is calculated from these values.
	\return The relative transformation matrix. */
	Transform& GetRelativeTransform() { return mLocalTransform; }

	//! Returns the absolute transformation of the spatial node.
	/** The absolute transformation is stored internally as 3
	vectors: translation, rotation and scale. To get the absolute
	transformation matrix, it is calculated from these values.
	\return The relative transformation matrix. */
	Transform& GetAbsoluteTransform() { return mWorldTransform; }

	void SetCurrentAbsoluteTransform(bool enable) { mWorldTransformIsCurrent = enable; }

	//! Returns the absoulte bound of the spatial node
	BoundingSphere& GetAbsoulteBound() { return mWorldBound; }

	void SetCurrentAbsoluteBound(bool enable) { mWorldBoundIsCurrent = enable; }

    // Support for hierarchical culling.
    void OnGetVisibleSet(
		Culler& culler, eastl::shared_ptr<Camera> const& camera, bool noCull);
    virtual void GetVisibleSet(
		Culler& culler, eastl::shared_ptr<Camera> const& camera, bool noCull) = 0;

    // Access to the parent object.  Node calls this during attach/detach of
    // children.
    void SetParent(Spatial* parent);

protected:
    // Constructor accessible by Node, Visual, and Audial.
    Spatial();

    // Support for geometric updates.
    virtual void UpdateWorldData(double applicationTime);
    virtual void UpdateWorldBound() = 0;
    void PropagateBoundToRoot();

	// Local and world transforms.  In some situations you might need to set
	// the world transform directly and bypass the Spatial::Update()
	// mechanism, in which case worldTransformIsCurrent should be set to
	// 'true'.
	Transform mLocalTransform;
	Transform mWorldTransform;
	bool mWorldTransformIsCurrent;

	// World bound access.  In some situations you might want to set the
	// world bound directly and bypass the Spatial::Update() mechanism, in
	// which case worldBoundIsCurrent flag should be set to 'true'.
	BoundingSphere mWorldBound;
	bool mWorldBoundIsCurrent;

	CullingMode				mCulling;
	bool					mIsVisible;
	bool					mIsDebugObject;
	unsigned int			mDebugDataVisible;
	unsigned int			mAutomaticCullingState;

private:
    // Support for a hierarchical scene graph.  Spatial provides the parent
    // pointer.  Node provides the child pointers.  The parent pointer is not
    // shared to avoid reference-count cycles between mParent and 'this.
    // Because the pointer links are set internally rather than by an external
    // manager, it is not possible to use eastl::weak_ptr to avoid the cycle
    // because we do not know the shared_ptr object that owns mParent.
    Spatial* mParent;
};

#endif