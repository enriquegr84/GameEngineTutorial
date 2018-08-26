// David Eberly, Geometric Tools, Redmond WA 98052
// Copyright (c) 1998-2017
// Distributed under the Boost Software License, Version 1.0.
// http://www.boost.org/LICENSE_1_0.txt
// http://www.geometrictools.com/License/Boost/LICENSE_1_0.txt
// File Version: 3.0.0 (2016/06/19)

#include "Controller.h"

Controller::Controller() : 
	mRepeat(RT_CLAMP), mMinTime(0.0), mMaxTime(0.0), mPhase(0.0), 
	mFrequency(1.0), mActive(true), mObject(nullptr)
{
}

Controller::~Controller()
{
}

bool Controller::Update(double applicationTime)
{
    if (mActive)
        return true;

    return false;
}

void Controller::SetObject(ControlledObject* object)
{
    mObject = object;
}

double Controller::GetControlTime(double applicationTime)
{
    double controlTime = mFrequency * applicationTime + mPhase;

    if (mRepeat == RT_CLAMP)
    {
        // Clamp the time to the [min,max] interval.
        if (controlTime < mMinTime)
        {
            return mMinTime;
        }
        if (controlTime > mMaxTime)
        {
            return mMaxTime;
        }
        return controlTime;
    }

    double timeRange = mMaxTime - mMinTime;
    if (timeRange > 0.0)
    {
        double multiples = (controlTime - mMinTime) / timeRange;
        double integerTime = floor(multiples);
        double fractionTime = multiples - integerTime;
        if (mRepeat == RT_WRAP)
        {
            return mMinTime + fractionTime*timeRange;
        }

        // repeat == RT_CYCLE
        if (static_cast<int>(integerTime) & 1)
        {
            // Go backward in time.
            return mMaxTime - fractionTime * timeRange;
        }
        else
        {
            // Go forward in time.
            return mMinTime + fractionTime * timeRange;
        }
    }

    // The minimum and maximum times are the same, so return the minimum.
    return mMinTime;
}
