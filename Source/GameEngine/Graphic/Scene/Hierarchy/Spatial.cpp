// David Eberly, Geometric Tools, Redmond WA 98052
// Copyright (c) 1998-2017
// Distributed under the Boost Software License, Version 1.0.
// http://www.boost.org/LICENSE_1_0.txt
// http://www.geometrictools.com/License/Boost/LICENSE_1_0.txt
// File Version: 3.0.0 (2016/06/19)

#include "Spatial.h"

#include "Core/OS/OS.h"

Spatial::Spatial()
    : mParent(nullptr), mCullMode(CULL_DYNAMIC)
{
}

Spatial::~Spatial()
{
	// The mParent member is not reference counted by Spatial, so do not
	// release it here.
}

void Spatial::Update(bool initiator)
{
    UpdateWorldData();
    UpdateWorldBound();
    if (initiator)
    {
        PropagateBoundToRoot();
    }
}

void Spatial::OnGetVisibleSet(Culler& culler, 
	eastl::shared_ptr<Camera> const& camera, bool noCull)
{
    if (mCullMode == CULL_ALWAYS)
        return;

    if (mCullMode == CULL_NEVER)
        noCull = true;

    unsigned int savePlaneState = culler.GetPlaneState();

	if (noCull || culler.IsVisible(mWorldBound))
	{
		culler.Insert(this);
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

void Spatial::UpdateWorldData()
{
    // Update world transforms.
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