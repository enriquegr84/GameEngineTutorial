// David Eberly, Geometric Tools, Redmond WA 98052
// Copyright (c) 1998-2017
// Distributed under the Boost Software License, Version 1.0.
// http://www.boost.org/LICENSE_1_0.txt
// http://www.geometrictools.com/License/Boost/LICENSE_1_0.txt
// File Version: 3.0.1 (2016/11/13)

#ifndef CONSTANTCOLOREFFECT_H
#define CONSTANTCOLOREFFECT_H

#include "Mathematic/Algebra/Matrix4x4.h"

#include "Graphic/Effect/VisualEffect.h"


class GRAPHIC_ITEM ConstantColorEffect : public VisualEffect
{
public:
    // Construction.
    ConstantColorEffect(eastl::shared_ptr<ProgramFactory> const& factory, eastl::string path, Vector4<float> const& color);

    // Member access.
    inline void SetPVWMatrix(Matrix4x4<float> const& pvwMatrix);
    inline Matrix4x4<float> const& GetPVWMatrix() const;

    // Required to bind and update resources.
    inline eastl::shared_ptr<ConstantBuffer> const& GetPVWMatrixConstant() const;
    inline eastl::shared_ptr<ConstantBuffer> const& GetColorConstant() const;

    void SetPVWMatrixConstant(eastl::shared_ptr<ConstantBuffer> const& pvwMatrix);

private:
    // Vertex shader parameters.
	eastl::shared_ptr<ConstantBuffer> mPVWMatrixConstant;
	eastl::shared_ptr<ConstantBuffer> mColorConstant;

    // Convenience pointers.
    Matrix4x4<float>* mPVWMatrix;
    Vector4<float>* mColor;
};


inline void ConstantColorEffect::SetPVWMatrix(Matrix4x4<float> const& pvwMatrix)
{
    *mPVWMatrix = pvwMatrix;
}

inline Matrix4x4<float> const& ConstantColorEffect::GetPVWMatrix() const
{
    return *mPVWMatrix;
}

inline eastl::shared_ptr<ConstantBuffer> const& ConstantColorEffect::GetPVWMatrixConstant() const
{
    return mPVWMatrixConstant;
}

inline eastl::shared_ptr<ConstantBuffer> const& ConstantColorEffect::GetColorConstant() const
{
    return mColorConstant;
}

#endif