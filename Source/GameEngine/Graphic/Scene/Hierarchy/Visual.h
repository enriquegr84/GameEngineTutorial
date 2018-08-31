// David Eberly, Geometric Tools, Redmond WA 98052
// Copyright (c) 1998-2017
// Distributed under the Boost Software License, Version 1.0.
// http://www.boost.org/LICENSE_1_0.txt
// http://www.geometrictools.com/License/Boost/LICENSE_1_0.txt
// File Version: 3.0.0 (2016/06/19)

#ifndef VISUAL_H
#define VISUAL_H

#include "Graphic/Resource/Buffer/IndexBuffer.h"
#include "Graphic/Resource/Buffer/VertexBuffer.h"
#include "Graphic/Effect/VisualEffect.h"

#include "BoundingSphere.h"

class GRAPHIC_ITEM Visual
{
public:
    // Construction and destruction.
    virtual ~Visual();
    Visual(
        eastl::shared_ptr<VertexBuffer> const& vbuffer = eastl::shared_ptr<VertexBuffer>(),
		eastl::shared_ptr<IndexBuffer> const& ibuffer = eastl::shared_ptr<IndexBuffer>(),
		eastl::shared_ptr<VisualEffect> const& effect = eastl::shared_ptr<VisualEffect>());

    // Member access.
    inline void SetVertexBuffer(eastl::shared_ptr<VertexBuffer> const& vbuffer);
    inline void SetIndexBuffer(eastl::shared_ptr<IndexBuffer> const& ibuffer);
    inline void SetEffect(eastl::shared_ptr<VisualEffect> const& effect);
    inline eastl::shared_ptr<VertexBuffer> const& GetVertexBuffer() const;
    inline eastl::shared_ptr<IndexBuffer> const& GetIndexBuffer() const;
    inline eastl::shared_ptr<VisualEffect> const& GetEffect() const;

    // Support for geometric updates.
    bool UpdateModelBound();
    bool UpdateModelNormals();

    // Public member access.
    BoundingSphere mModelBound;

protected:

	eastl::shared_ptr<VertexBuffer> mVBuffer;
	eastl::shared_ptr<IndexBuffer> mIBuffer;
	eastl::shared_ptr<VisualEffect> mEffect;
};


inline void Visual::SetVertexBuffer(eastl::shared_ptr<VertexBuffer> const& vbuffer)
{
    mVBuffer = vbuffer;
}

inline void Visual::SetIndexBuffer(eastl::shared_ptr<IndexBuffer> const& ibuffer)
{
    mIBuffer = ibuffer;
}

inline void Visual::SetEffect(eastl::shared_ptr<VisualEffect> const& effect)
{
    mEffect = effect;
}

inline eastl::shared_ptr<VertexBuffer> const& Visual::GetVertexBuffer() const
{
    return mVBuffer;
}

inline eastl::shared_ptr<IndexBuffer> const& Visual::GetIndexBuffer() const
{
    return mIBuffer;
}

inline eastl::shared_ptr<VisualEffect> const& Visual::GetEffect() const
{
    return mEffect;
}


#endif