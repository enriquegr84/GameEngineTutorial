// David Eberly, Geometric Tools, Redmond WA 98052
// Copyright (c) 1998-2017
// Distributed under the Boost Software License, Version 1.0.
// http://www.boost.org/LICENSE_1_0.txt
// http://www.geometrictools.com/License/Boost/LICENSE_1_0.txt
// File Version: 3.0.0 (2016/06/19)

#include "TransformController.h"

#include "Graphic/Scene/Hierarchy/Spatial.h"

TransformController::TransformController(Transform const& localTransform)
    :
    mLocalTransform(localTransform)
{
}

bool TransformController::Update()
{
    if (!Controller::Update())
    {
        return false;
    }

    Spatial* spatial = reinterpret_cast<Spatial*>(mObject);
    spatial->GetRelativeTransform() = mLocalTransform;
    return true;
}

