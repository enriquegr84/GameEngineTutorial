// David Eberly, Geometric Tools, Redmond WA 98052
// Copyright (c) 1998-2017
// Distributed under the Boost Software License, Version 1.0.
// http://www.boost.org/LICENSE_1_0.txt
// http://www.geometrictools.com/License/Boost/LICENSE_1_0.txt
// File Version: 3.0.0 (2016/06/19)

#ifndef TRANSFORMCONTROLLER_H
#define TRANSFORMCONTROLLER_H

#include "Controller.h"

#include "Mathematic/Algebra/Transform.h"


class GRAPHIC_ITEM TransformController : public Controller
{
public:
    TransformController(Transform const& localTransform);

    // Member access.
    inline void SetTransform(Transform const& localTransform);
    inline Transform const& GetTransform() const;

    // The animation update.
    // The update simply copies mLocalTransform to the Spatial mObject's
    // LocalTransform.  In this sense, TransformController represents a
    // transform that is constant for all time.
    virtual bool Update(double applicationTime);

protected:

    Transform mLocalTransform;
};


inline void TransformController::SetTransform(Transform const& localTransform)
{
    mLocalTransform = localTransform;
}

inline Transform const& TransformController::GetTransform() const
{
    return mLocalTransform;
}


#endif