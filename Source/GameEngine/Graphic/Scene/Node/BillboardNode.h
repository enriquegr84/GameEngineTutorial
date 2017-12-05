// David Eberly, Geometric Tools, Redmond WA 98052
// Copyright (c) 1998-2017
// Distributed under the Boost Software License, Version 1.0.
// http://www.boost.org/LICENSE_1_0.txt
// http://www.geometrictools.com/License/Boost/LICENSE_1_0.txt
// File Version: 3.0.0 (2016/06/19)

#ifndef BILLBOARDNODE_H
#define BILLBOARDNODE_H

#include "Graphic/Scene/Hierarchy/Node.h"

class GRAPHIC_ITEM BillboardNode : public Node
{
public:
    // The model space of the billboard has an up vector of (0,1,0) that is
    // chosen to be the billboard's axis of rotation.

    // Construction.
    BillboardNode(eastl::shared_ptr<Camera> const& camera);

    // The camera to which the billboard is aligned.
    inline void AlignTo(eastl::shared_ptr<Camera> const& camera);

protected:
    // Support for the geometric update.
    virtual void UpdateWorldData(double applicationTime);

	eastl::shared_ptr<Camera> mCamera;
};


inline void BillboardNode::AlignTo(eastl::shared_ptr<Camera> const& camera)
{
    mCamera = camera;
}


#endif