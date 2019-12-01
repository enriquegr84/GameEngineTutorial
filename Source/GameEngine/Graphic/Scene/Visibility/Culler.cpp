// David Eberly, Geometric Tools, Redmond WA 98052
// Copyright (c) 1998-2017
// Distributed under the Boost Software License, Version 1.0.
// http://www.boost.org/LICENSE_1_0.txt
// http://www.geometrictools.com/License/Boost/LICENSE_1_0.txt
// File Version: 3.0.0 (2016/06/19)

#include "Graphic/Scene/Hierarchy/Camera.h"
#include "Graphic/Scene/Hierarchy/Spatial.h"

Culler::~Culler()
{
}

Culler::Culler()
    :
    mPlaneQuantity(6)
{
    // The data members mFrustum, mPlane, and mPlaneState are
    // uninitialized.  They are initialized in the GetVisibleSet call.
    mVisibleSet.reserve(INITIALLY_VISIBLE);
}

bool Culler::PushPlane(CullingPlane const& plane)
{
    if (mPlaneQuantity < MAX_PLANE_QUANTITY)
    {
        // The number of user-defined planes is limited.
        mPlane[mPlaneQuantity] = plane;
        ++mPlaneQuantity;
        return true;
    }
    return false;
}

bool Culler::PopPlane()
{
    if (mPlaneQuantity > Camera::VF_QUANTITY)
    {
        // Only non-view-frustum planes may be removed from the stack.
        --mPlaneQuantity;
        return true;
    }
    return false;
}

void Culler::ComputeVisibleSet(
	eastl::shared_ptr<Camera> const& camera,
    eastl::shared_ptr<Spatial> const& root)
{
    if (root)
    {
        PushViewFrustumPlanes(camera);
        mVisibleSet.clear();
		root->OnGetVisibleSet(*this, camera, false);
    }
    else
    {
        LogError("A node is required for culling.");
    }
}

bool Culler::IsVisible(BoundingSphere const& sphere)
{
    if (sphere.GetRadius() == 0.0f)
    {
        // The node is a dummy node and cannot be visible.
        return false;
    }

    // Start with the last pushed plane, which is potentially the most
    // restrictive plane.
    int index = mPlaneQuantity - 1;
    unsigned int mask = (1u << index);

    for (int i = 0; i < mPlaneQuantity; ++i, --index, mask >>= 1)
    {
        if (mPlaneState & mask)
        {
            int side = sphere.WhichSide(mPlane[index]);

            if (side < 0)
            {
                // The object is on the negative side of the plane, so
                // cull it.
                return false;
            }

            if (side > 0)
            {
                // The object is on the positive side of plane.  There is
                // no need to compare subobjects against this plane, so
                // mark it as inactive.
                mPlaneState &= ~mask;
            }
        }
    }

    return true;
}

bool Culler::IsVisible(Spatial* spatial)
{
	return eastl::find(mVisibleSet.begin(), mVisibleSet.end(), spatial) != mVisibleSet.end();
}

void Culler::Insert(Spatial* spatial)
{
    mVisibleSet.push_back(spatial);
}

void Culler::PushViewFrustumPlanes(eastl::shared_ptr<Camera> const& camera)
{
	Matrix4x4<float> pv = Transpose(camera->GetProjectionViewMatrix());
	mPlane[Camera::VF_DMIN].Set(Vector4<float>{
		pv[2], pv[7], pv[11], 0.f}, pv[14]);
	mPlane[Camera::VF_DMAX].Set(Vector4<float>{
		pv[3] - pv[2], pv[7] - pv[6], pv[11] - pv[10], 0.f}, pv[15] - pv[14]);
	mPlane[Camera::VF_UMIN].Set(Vector4<float>{
		pv[3] + pv[1], pv[7] + pv[5], pv[11] + pv[9], 0.f}, pv[15] + pv[13]);
	mPlane[Camera::VF_UMAX].Set(Vector4<float>{
		pv[3] - pv[1], pv[7] - pv[5], pv[11] - pv[9], 0.f}, pv[15] - pv[13]);
	mPlane[Camera::VF_RMIN].Set(Vector4<float>{
		pv[3] + pv[0], pv[7] + pv[4], pv[11] + pv[8], 0.f}, pv[15] + pv[12]);
	mPlane[Camera::VF_RMAX].Set(Vector4<float>{
		pv[3] - pv[0], pv[7] - pv[4], pv[11] - pv[8], 0.f}, pv[15] - pv[12]);

	for (unsigned int i = 0; i != Camera::VF_QUANTITY; ++i)
		mPlane[i].Normalize();

    // All planes are active initially.
    mPlaneState = 0xFFFFFFFFu;
}

