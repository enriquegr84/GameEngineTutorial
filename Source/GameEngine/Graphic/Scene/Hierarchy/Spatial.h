// David Eberly, Geometric Tools, Redmond WA 98052
// Copyright (c) 1998-2017
// Distributed under the Boost Software License, Version 1.0.
// http://www.boost.org/LICENSE_1_0.txt
// http://www.geometrictools.com/License/Boost/LICENSE_1_0.txt
// File Version: 3.0.0 (2016/06/19)

#ifndef SPATIAL_H
#define SPATIAL_H

#include "Graphic/Scene/Visibility/Culler.h"

#include <EASTL/string.h>

class Camera;

// Support for a spatial hierarchy of objects.  Class Spatial has a parent
// pointer.  Class Node derives from Spatial has an array of child pointers.
class GRAPHIC_ITEM Spatial
{
public:
    // Abstract base class.
    virtual ~Spatial();

    // Update of geometric state.  The function computes world transformations
    // on the downward pass of the scene graph traversal and world bounding
    // volumes on the upward pass of the traversal.  The object that calls the
    // update is the initiator.  Other objects visited during the update are
    // not initiators.
    void Update(bool initiator = true);

    // Access to the parent object, which is null for the root of the
    // hierarchy.
	Spatial* GetParent();

	bool IsVisible() const { return mIsVisible; }
	void SetVisible(bool visible) { mIsVisible = visible; }

	//! Enables or disables culling.
	void SetCullingMode(CullingMode culling) { mCullMode = culling; }
	//! Gets the culling state.
	CullingMode GetCullingMode() const { return mCullMode; }

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

	//! Updates the absolute position based on the relative and the parents position
	/** Note: This does not recursively update the parents absolute positions, so if you have a deeper
	hierarchy you might want to update the parents first.*/
	void UpdateAbsoluteTransform();

	//! Returns the absoulte bound of the spatial node
	BoundingSphere& GetAbsoulteBound() { return mWorldBound; }

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
    virtual void UpdateWorldData();
    virtual void UpdateWorldBound() = 0;
    void PropagateBoundToRoot();

	// Local and world transforms.  In some situations you might need to set
	// the world transform directly and bypass the Spatial::Update()
	// mechanism, in which case worldTransformIsCurrent should be set to
	// 'true'.
	Transform mLocalTransform;
	Transform mWorldTransform;

	// World bound access.  In some situations you might want to set the
	// world bound directly and bypass the Spatial::Update() mechanism, in
	// which case worldBoundIsCurrent flag should be set to 'true'.
	BoundingSphere mWorldBound;

	CullingMode		mCullMode;
	bool			mIsVisible;

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