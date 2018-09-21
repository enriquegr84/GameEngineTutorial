// David Eberly, Geometric Tools, Redmond WA 98052
// Copyright (c) 1998-2018
// Distributed under the Boost Software License, Version 1.0.
// http://www.boost.org/LICENSE_1_0.txt
// http://www.geometrictools.com/License/Boost/LICENSE_1_0.txt
// File Version: 3.0.0 (2016/06/19)

#ifndef PVWUPDATER_H
#define PVWUPDATER_H

#include "GameEngineStd.h"

#include "Graphic/Scene/Hierarchy/Camera.h"
#include "Graphic/Resource/Buffer/ConstantBuffer.h"

class GRAPHIC_ITEM PVWUpdater
{
public:
    // Construction and destruction.
    virtual ~PVWUpdater();
    PVWUpdater();
    PVWUpdater(eastl::shared_ptr<Camera> const& camera, BufferUpdater const& updater);

    // Member access.  The functions are for deferred construction after
    // a default construction of a PVWUpdater.
    void Set(eastl::shared_ptr<Camera> const& camera, BufferUpdater const& updater);
    inline eastl::shared_ptr<Camera> const& GetCamera() const;
    inline void SetUpdater(BufferUpdater const& updater);
    inline BufferUpdater const& GetUpdater() const;

    // Update the constant buffer's projection-view-world matrix (pvw-matrix)
    // when the camera's view or projection matrices change.  The input
    // 'pvwMatrixName' is the name specified in the shader program and is
    // used in calls to ConstantBuffer::SetMember<Matrix4x4<float>>(...).
    // If you modify the view or projection matrices directly through the
    // Camera interface, you are responsible for calling UpdatePVWMatrices().
    //
    // The Subscribe function uses the address of 'worldMatrix' as a key
    // to a std::map, so be careful to ensure that 'worldMatrix' persists
    // until a call to an Unsubscribe function.  The return value of Subscribe
    // is 'true' as long as 'cbuffer' is not already subscribed and actually
    // has a member named 'pvwMatrixName'.  The return value of Unsubscribe is
    // true if and only if the input matrix is currently subscribed.
    bool Subscribe(Matrix4x4<float> const& worldMatrix,
		eastl::shared_ptr<ConstantBuffer> const& cbuffer,
		eastl::string const& pvwMatrixName = "pvwMatrix");
    bool Unsubscribe(Matrix4x4<float> const& worldMatrix);
    void UnsubscribeAll();

    // After any camera modifictions that change the projection or view
    // matrices, call this function to update the constant buffers that
    // are subscribed.
    void Update();

protected:
	eastl::shared_ptr<Camera> mCamera;
    BufferUpdater mBufferUpdater;

    typedef Matrix4x4<float> const* PVWKey;
    typedef eastl::pair<eastl::shared_ptr<ConstantBuffer>, eastl::string> PVWValue;
	typedef eastl::multimap<PVWKey, PVWValue>::iterator PVWIterator;
	eastl::multimap<PVWKey, PVWValue> mSubscribers;
};


inline eastl::shared_ptr<Camera> const& PVWUpdater::GetCamera() const
{
    return mCamera;
}

inline void PVWUpdater::SetUpdater(BufferUpdater const& updater)
{
    mBufferUpdater = updater;
}

inline BufferUpdater const& PVWUpdater::GetUpdater() const
{
    return mBufferUpdater;
}

#endif