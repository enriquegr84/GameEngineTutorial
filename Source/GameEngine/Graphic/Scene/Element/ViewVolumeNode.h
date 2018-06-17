// David Eberly, Geometric Tools, Redmond WA 98052
// Copyright (c) 1998-2018
// Distributed under the Boost Software License, Version 1.0.
// http://www.boost.org/LICENSE_1_0.txt
// http://www.geometrictools.com/License/Boost/LICENSE_1_0.txt
// File Version: 3.0.0 (2016/06/19)

#ifndef VIEWVOLUMENODE_H
#define VIEWVOLUMENODE_H

#include "Graphic/Scene/Hierarchy/ViewVolume.h"
#include "Graphic/Scene/Hierarchy/Node.h"

class GRAPHIC_ITEM ViewVolumeNode : public Node
{
public:
    // Construction.  The node's world translation is used as the view volume's
    // location.  The node's world rotation matrix is used for the view volume's
    // coordinate axes.  Column 0 of the world rotation matrix is the view volume's
    // direction vector, column 1 of the world rotation matrix is the view volume's
    // up vector, and column 2 of the world rotation matrix is the view volume's
    // right vector.
    //
    // On construction, the node's local transformation is set to the view volume's
    // coordinate system.
    //   local translation       = view volume location
    //   local rotation column 0 = view volume direction
    //   local rotation column 1 = view volume up
    //   local rotation column 2 = view volume right
    ViewVolumeNode(const ActorId actorid, eastl::shared_ptr<ViewVolume> const& viewVolume = nullptr);

	//! Renders event
	virtual bool PreRender(Scene *pScene);
	virtual bool Render(Scene *pScene);

	//! Returns type of the scene node
	virtual NodeType GetType() const { return NT_LIGHT; }

    // When you set the view volume, the node's local transformation is set to the
    // view volumes's current current coordinate system.  The node's world
    // transformation is computed, and the view volume's coordinate system is set
    // to use the node's world transformation.
    void SetViewVolume(eastl::shared_ptr<ViewVolume> const& viewVolume);
    inline eastl::shared_ptr<ViewVolume> const& GetViewVolume() const;

    // Additional semantics may be applied after UpdateWorldData updates the
    // view volume.
    inline void SetOnUpdate(eastl::function<void(ViewVolumeNode*)> const& onUpdate);
    inline eastl::function<void(ViewVolumeNode*)> const& GetOnUpdate() const;

protected:
    // Geometric updates.
    virtual void UpdateWorldData(double applicationTime);

	eastl::shared_ptr<ViewVolume> mViewVolume;
	eastl::function<void(ViewVolumeNode*)> mOnUpdate;
};

inline eastl::shared_ptr<ViewVolume> const& ViewVolumeNode::GetViewVolume() const
{
    return mViewVolume;
}

inline void ViewVolumeNode::SetOnUpdate(eastl::function<void(ViewVolumeNode*)> const& onUpdate)
{
    mOnUpdate = onUpdate;
}

inline eastl::function<void(ViewVolumeNode*)> const& ViewVolumeNode::GetOnUpdate() const
{
    return mOnUpdate;
}

#endif