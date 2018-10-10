// David Eberly, Geometric Tools, Redmond WA 98052
// Copyright (c) 1998-2017
// Distributed under the Boost Software License, Version 1.0.
// http://www.boost.org/LICENSE_1_0.txt
// http://www.geometrictools.com/License/Boost/LICENSE_1_0.txt
// File Version: 3.0.0 (2016/06/19)

#include "Spatial.h"

#include "Core/OS/OS.h"

Spatial::Spatial()
    : mParent(nullptr), mWorldTransformIsCurrent(false), mWorldBoundIsCurrent(false),
    mCulling(CULL_DYNAMIC), mAutomaticCullingState(AC_OFF), mDebugDataVisible(DS_OFF)
{
}

Spatial::~Spatial()
{
	// The mParent member is not reference counted by Spatial, so do not
	// release it here.
}

void Spatial::Update(bool initiator)
{
	// Update any controllers associated with this object.
	double applicationTIme = (double)Timer::GetTime();

    UpdateWorldData(applicationTIme);
    UpdateWorldBound();
    if (initiator)
    {
        PropagateBoundToRoot();
    }
}

void Spatial::OnGetVisibleSet(Culler& culler, 
	eastl::shared_ptr<Camera> const& camera, bool noCull)
{
    if (mCulling == CULL_ALWAYS)
    {
        return;
    }

    if (mCulling == CULL_NEVER)
    {
        noCull = true;
    }

    unsigned int savePlaneState = culler.GetPlaneState();
    if (noCull || culler.IsVisible(mWorldBound))
    {
        GetVisibleSet(culler, camera, noCull);
    }
    culler.SetPlaneState(savePlaneState);
}

void Spatial::UpdateAbsoluteTransform()
{
	if (mParent)
	{
#if defined(GE_USE_MAT_VEC)
		mWorldTransform = mParent->mWorldTransform * mLocalTransform;
#else
		mWorldTransform = localTransform * mParent->mWorldTransform;
#endif
	}
	else
	{
		mWorldTransform = mLocalTransform;
	}
}

void Spatial::UpdateWorldData(double applicationTIme)
{
    UpdateControllers(applicationTIme);

    // Update world transforms.
    if (!mWorldTransformIsCurrent)
		UpdateAbsoluteTransform();
}

void Spatial::PropagateBoundToRoot ()
{
    if (mParent)
    {
        mParent->UpdateWorldBound();
        mParent->PropagateBoundToRoot();
    }
}

Spatial* Spatial::GetParent()
{
	return mParent;
}

void Spatial::SetParent(Spatial* parent)
{
	mParent = parent;
}