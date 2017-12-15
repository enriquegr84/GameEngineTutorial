// David Eberly, Geometric Tools, Redmond WA 98052
// Copyright (c) 1998-2017
// Distributed under the Boost Software License, Version 1.0.
// http://www.boost.org/LICENSE_1_0.txt
// http://www.geometrictools.com/License/Boost/LICENSE_1_0.txt
// File Version: 3.0.0 (2016/06/19)

#include "Spatial.h"

Spatial::Spatial()
    : mWorldTransformIsCurrent(false), mWorldBoundIsCurrent(false),
    mCulling(CULL_DYNAMIC), mAutomaticCullingState(AC_OFF), mDebugDataVisible(DS_OFF)
{
	mPosition = Vector3<float>();
	mRotation = Vector3<float>();
	mScale = Vector3<float>{ 1.f, 1.f, 1.f };
}

Spatial::~Spatial()
{
	// The mParent member is not reference counted by Spatial, so do not
	// release it here.
}

//! Updates the absolute position based on the relative and the parents position
/** Note: This does not recursively update the parents absolute positions, so if
you have a deeper hierarchy you might want to update the parents first.*/
void Spatial::UpdateAbsoluteTransformation()
{
	/*
	if (mParent && mParent->GetType() != NT_ROOT)
		SetTransform(&(mParent->ToWorld() * GetRelativeTransformation()));
	else
		SetTransform(&GetRelativeTransformation());
	*/
}

void Spatial::Update(double applicationTime, bool initiator)
{
    UpdateWorldData(applicationTime);
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

void Spatial::UpdateWorldData(double applicationTime)
{
    // Update any controllers associated with this object.
    UpdateControllers(applicationTime);

    // Update world transforms.
    if (!mWorldTransformIsCurrent)
    {
        if (mParent)
        {
#if defined(GE_USE_MAT_VEC)
            mWorldTransform = mParent->mWorldTransform*mLocalTransform;
#else
            worldTransform = localTransform*mParent->worldTransform;
#endif
        }
        else
        {
            mWorldTransform = mLocalTransform;
        }
    }
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