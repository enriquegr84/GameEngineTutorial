// David Eberly, Geometric Tools, Redmond WA 98052
// Copyright (c) 1998-2017
// Distributed under the Boost Software License, Version 1.0.
// http://www.boost.org/LICENSE_1_0.txt
// http://www.geometrictools.com/License/Boost/LICENSE_1_0.txt
// File Version: 3.0.0 (2016/06/19)

#ifndef SKINCONTROLLER_H
#define SKINCONTROLLER_H

#include "Controller.h"

#include "Core/Utility/Array2.h"
#include "Mathematic/Algebra/Vector4.h"
#include "Graphic/Resource/Buffer/VertexBuffer.h"

class Node;

class GRAPHIC_ITEM SkinController : public Controller
{
public:
    // Construction and destruction.  The numbers of vertices and bones are
    // fixed for the lifetime of the object.  The controlled object must
    // have a vertex buffer with 'numVertices' elements, with 3D (x,y,z) or
    // 4D (x,y,z,1) positions, and the bind of positions is in unit 0.  The
    // post-update function is used to allow a graphics engine object to copy
    // the modified vertex buffer to graphics memory.
    typedef eastl::function<void(eastl::shared_ptr<VertexBuffer> const&)> Updater;
    virtual ~SkinController();
    SkinController(int numVertices, int numBones, Updater const& postUpdate);

    // Member access.  After calling the constructor, you must set the data
    // using these functions.  The bone array uses weak pointers to avoid
    // reference-count cycles in the scene graph.
    inline int GetNumVertices() const;
    inline int GetNumBones() const;
    inline eastl::vector<eastl::weak_ptr<Node>>& GetBones();
    inline Array2<float>& GetWeights();
    inline Array2<Vector4<float>>& GetOffsets();

    // The animation update.
    virtual bool Update();

protected:
    // On the first call to Update(...), the position channel and stride
    // are extracted from mObject's vertex buffer.  This is a deferred
    // construction, because we do not know mObject when SkinController
    // is constructed.
    void OnFirstUpdate();

    int mNumVertices;                           // nv
    int mNumBones;                              // nb
    eastl::vector<eastl::weak_ptr<Node>> mBones;    // bones[nb]
	Array2<float> mWeights;                     // weight[nv][nb], index b+nb*v
	Array2<Vector4<float>> mOffsets;            // offset[nv][nb], index b+nb*v
    Updater mPostUpdate;
    char* mPosition;
    unsigned int mStride;
    bool mFirstUpdate, mCanUpdate;
};


inline int SkinController::GetNumVertices() const
{
    return mNumVertices;
}

inline int SkinController::GetNumBones() const
{
    return mNumBones;
}

inline eastl::vector<eastl::weak_ptr<Node>>& SkinController::GetBones()
{
    return mBones;
}

inline Array2<float>& SkinController::GetWeights()
{
    return mWeights;
}

inline Array2<Vector4<float>>& SkinController::GetOffsets()
{
    return mOffsets;
}

#endif