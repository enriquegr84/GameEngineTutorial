// David Eberly, Geometric Tools, Redmond WA 98052
// Copyright (c) 1998-2017
// Distributed under the Boost Software License, Version 1.0.
// http://www.boost.org/LICENSE_1_0.txt
// http://www.geometrictools.com/License/Boost/LICENSE_1_0.txt
// File Version: 3.0.0 (2016/06/19)

#ifndef CONTROLLER_H
#define CONTROLLER_H

#include "Graphic/GraphicStd.h"
#include "Core/CoreStd.h"

class ControlledObject;

class GRAPHIC_ITEM Controller
{
protected:
    // Abstract base class.
    Controller();
public:
    virtual ~Controller();

    // The animation update.
    virtual bool Update();

    // The class ControlledObject needs to set the object during a call to
    // AttachController.  Derived classes that manage a set of controllers
    // also need to set the objects during a call to AttachController.
    virtual void SetObject(ControlledObject* object);

protected:

	// Time management.  A controller may use its own time scale, and it
	// specifies how times are to be mapped to application time.
	enum GRAPHIC_ITEM RepeatType
	{
		RT_CLAMP,
		RT_WRAP,
		RT_CYCLE
	};

	// Member access.
	RepeatType mRepeat;  // default = RT_CLAMP
	double mMinTime;     // default = 0
	double mMaxTime;     // default = 0
	double mPhase;       // default = 0
	double mFrequency;   // default = 1
	bool mActive;        // default = true

    // Conversion from application time units to controller time units.
    // Derived classes may use this in their update routines.
    double GetControlTime(double applicationTime);

    // The controlled object.  This is a regular pointer rather than a
    // shared pointer to avoid the reference-count cycle between mObject
    // and 'this'.  Because the pointer links are set internally rather
    // than by an external manager, it is not possible to use std::weak_ptr
    // to avoid the cycle because we do not know the shared_ptr object
    // that owns mObject.
    ControlledObject* mObject;
};

#endif