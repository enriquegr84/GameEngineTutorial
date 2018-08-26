// David Eberly, Geometric Tools, Redmond WA 98052
// Copyright (c) 1998-2017
// Distributed under the Boost Software License, Version 1.0.
// http://www.boost.org/LICENSE_1_0.txt
// http://www.geometrictools.com/License/Boost/LICENSE_1_0.txt
// File Version: 3.0.0 (2016/06/19)

#ifndef CONTROLLEDOBJECT_H
#define CONTROLLEDOBJECT_H

#include "Controller.h"

class GRAPHIC_ITEM ControlledObject
{
protected:
    // Abstract base class.
    ControlledObject();
public:
    virtual ~ControlledObject();

    // Access to the controllers that control this object.
    typedef eastl::list<eastl::shared_ptr<Controller>> ControllerList;

    inline ControllerList const& GetControllers() const;
    void AttachController(eastl::shared_ptr<Controller> const& controller);
    void DetachController(eastl::shared_ptr<Controller> const& controller);
    void DetachAllControllers();
    bool UpdateControllers();

private:
	ControllerList mControllers;
};


inline ControlledObject::ControllerList const& ControlledObject::GetControllers() const
{
    return mControllers;
}

#endif