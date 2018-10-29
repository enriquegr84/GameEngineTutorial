// David Eberly, Geometric Tools, Redmond WA 98052
// Copyright (c) 1998-2017
// Distributed under the Boost Software License, Version 1.0.
// http://www.boost.org/LICENSE_1_0.txt
// http://www.geometrictools.com/License/Boost/LICENSE_1_0.txt
// File Version: 3.0.0 (2016/06/19)

#ifndef CULLER_H
#define CULLER_H

#include "Graphic/Scene/Hierarchy/BoundingSphere.h"
#include "Graphic/Scene/Hierarchy/Camera.h"

#include <EASTL/vector.h>
#include <memory>

// Support for determining the potentially visible set of spatial objects in
// a scene.  The class is designed to support derived classes that provide
// specialized culling and sorting.  One example is a culler that stores
// opaque objects in one set, sorted from front to back, and semitransparent
// objects in another set, sorted from back to front.  Another example is a
// portal system (room-graph visibility) that must maintain a set of unique
// visible objects (one object viewed through two portals should not be
// inserted into the set twice).

class Spatial;

enum GRAPHIC_ITEM CullingMode
{
    // Determine visibility state by comparing the world bounding volume
    // to culling planes.
    CULL_DYNAMIC,

    // Force the object to be culled.  If a Node is culled, its entire
    // subtree is culled.
    CULL_ALWAYS,

    // Never cull the object.  If a Node is never culled, its entire
    // subtree is never culled.  To accomplish this, the first time such
    // a Node is encountered, the 'noCull' parameter is set to 'true' in
    // the recursive chain GetVisibleSet/OnGetVisibleSet.
    CULL_NEVER
};

//! An enumeration for all types of automatic culling for built-in scene nodes
enum GRAPHIC_ITEM CullingType
{
	AC_OFF = 0,
	AC_BOX = 1,
	AC_FRUSTUM_BOX = 2,
	AC_FRUSTUM_SPHERE = 4,
	AC_OCC_QUERY = 8
};

typedef eastl::vector<Spatial*> VisibleSet;

class GRAPHIC_ITEM Culler
{
public:
    // Construction and destruction.
    virtual ~Culler();
    Culler();

    // Access to the stack of world culling planes.  The first 6 planes are
    // those associated with the camera's view frustum.  You may push and pop
    // planes to be used in addition to the view frustum planes.  The return
    // value for PushPlane is 'true' as long as you have not exceeded the
    // stack capacity.  The return value for PopPlane is 'true' as long as
    // you pop planes you pushed; it is not possible to pop the 6 planes for
    // the view frustum.
    enum { MAX_PLANE_QUANTITY = 32 };
    bool PushPlane(CullingPlane const& plane);
    bool PopPlane();

    // This is the main function you should use for culling within a scene
    // graph.  Traverse the scene graph and construct the potentially visible
    // set relative to the world planes.
    void ComputeVisibleSet(eastl::shared_ptr<Camera> const& camera,
        eastl::shared_ptr<Spatial> const& scene);

    // Access to the potentially visible set.
    inline VisibleSet& GetVisibleSet();

	// Find the spatial object in the visible set
	bool IsVisible(Spatial* spatial);

protected:
    enum { INITIALLY_VISIBLE = 128 };

    // These classes must make calls into the Culler, but applications are not
    // allowed to.
    friend class Spatial;
	friend class Scene;

    // Compare the object's world bounding sphere against the culling planes.
    // Only Spatial calls this function.
    bool IsVisible(BoundingSphere const& sphere);

    // The base class behavior is to append the visible object to the end of
    // the visible set (stored as an array).  Derived classes may override
    // this behavior; for example, the array might be maintained as a sorted
    // array for minimizing render state changes or it might be maintained
    // as a unique list of objects for a portal system.  Only spatial calls
    // this function.
    virtual void Insert(Spatial* spatial);

    // See the comments before data member mPlaneState about the bit system
    // for enabling and disabling planes during culling.  Only Spatial calls
    // these functions (during a scene graph traversal).
    inline void SetPlaneState(unsigned int planeState);
    inline unsigned int GetPlaneState() const;

    void PushViewFrustumPlanes(eastl::shared_ptr<Camera> const& camera);

    // The world culling planes corresponding to the view frustum plus any
    // additional user-defined culling planes.  The member mPlaneState
    // represents bit flags to store whether or not a plane is active in the
    // culling system.  A bit of 1 means the plane is active; otherwise, the
    // plane is inactive.  An active plane is compared to bounding volumes,
    // whereas an inactive plane is not.  This supports an efficient culling
    // of a hierarchy.  For example, if a node's bounding volume is inside
    // the left plane of the view frustum, then the left plane is set to
    // inactive because the children of the node are automatically all inside
    // the left plane.
    int mPlaneQuantity;
    CullingPlane mPlane[MAX_PLANE_QUANTITY];
    unsigned int mPlaneState;

    // The potentially visible set generated by ComputeVisibleSet(scene).
    VisibleSet mVisibleSet;
};


inline VisibleSet& Culler::GetVisibleSet()
{
    return mVisibleSet;
}

inline void Culler::SetPlaneState(unsigned int planeState)
{
    mPlaneState = planeState;
}

inline unsigned int Culler::GetPlaneState() const
{
    return mPlaneState;
}

#endif