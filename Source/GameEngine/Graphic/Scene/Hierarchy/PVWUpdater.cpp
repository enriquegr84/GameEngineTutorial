// David Eberly, Geometric Tools, Redmond WA 98052
// Copyright (c) 1998-2018
// Distributed under the Boost Software License, Version 1.0.
// http://www.boost.org/LICENSE_1_0.txt
// http://www.geometrictools.com/License/Boost/LICENSE_1_0.txt
// File Version: 3.0.0 (2016/06/19)

#include "PVWUpdater.h"

PVWUpdater::~PVWUpdater()
{
}

PVWUpdater::PVWUpdater()
{
    Set(nullptr, [](eastl::shared_ptr<Buffer> const&) {});
}

PVWUpdater::PVWUpdater(eastl::shared_ptr<Camera> const& camera, BufferUpdater const& updater)
{
    Set(camera, updater);
}

void PVWUpdater::Set(eastl::shared_ptr<Camera> const& camera, BufferUpdater const& updater)
{
    mCamera = camera;
    mBufferUpdater = updater;
}

bool PVWUpdater::Subscribe(Matrix4x4<float> const& worldMatrix,
    eastl::shared_ptr<ConstantBuffer> const& cbuffer,
    eastl::string const& pvwMatrixName)
{
    if (cbuffer && cbuffer->HasMember(pvwMatrixName))
    {
		for (PVWIterator it = mSubscribers.begin(); it != mSubscribers.end(); it++)
			if ((*it).second.first == cbuffer)
				return false;

		mSubscribers.insert(eastl::make_pair(&worldMatrix,
			eastl::make_pair(cbuffer, pvwMatrixName)));
		return true;
    }
    return false;
}

bool PVWUpdater::Unsubscribe(eastl::shared_ptr<ConstantBuffer> const& cbuffer)
{
	for (PVWIterator it = mSubscribers.begin(); it != mSubscribers.end(); it++)
	{
		if ((*it).second.first == cbuffer)
		{
			mSubscribers.erase(it);
			return true;
		}
	}
	return false;
}

void PVWUpdater::UnsubscribeAll()
{
    mSubscribers.clear();
}

void PVWUpdater::Update()
{
    // The function is called knowing that mCamera is not null.
    Matrix4x4<float> pvMatrix = mCamera->GetProjectionViewMatrix();
    for (auto& element : mSubscribers)
    {
        // Compute the new projection-view-world matrix.  The matrix
        // *element.first is the model-to-world matrix for the associated
        // object.
#if defined(GE_USE_MAT_VEC)
        Matrix4x4<float> pvwMatrix = pvMatrix * (*element.first);
#else
        Matrix4x4<float> pvwMatrix = (*element.first) * pvMatrix;
#endif
        // Copy the source matrix into the system memory of the constant
        // buffer.
        element.second.first->SetMember(element.second.second, pvwMatrix);

        // Allow the caller to update GPU memory as desired.
        mBufferUpdater(element.second.first);
    }
}
