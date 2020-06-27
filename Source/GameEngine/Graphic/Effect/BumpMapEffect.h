// David Eberly, Geometric Tools, Redmond WA 98052
// Copyright (c) 1998-2020
// Distributed under the Boost Software License, Version 1.0.
// https://www.boost.org/LICENSE_1_0.txt
// https://www.geometrictools.com/License/Boost/LICENSE_1_0.txt
// Version: 4.0.2019.08.13

#ifndef BUMPMAPEFFECT_H
#define BUMPMAPEFFECT_H

#include "Graphic/Scene/Hierarchy/Node.h"
#include "Graphic/Effect/VisualEffect.h"

#include "Mathematic/Algebra/Vector2.h"

class GRAPHIC_ITEM BumpMapEffect : public VisualEffect
{
public:
    // Construction.  The texture inputs must have mipmaps.
    BumpMapEffect(
		eastl::shared_ptr<ProgramFactory> const& factory, eastl::vector<eastl::string> const& path,
		eastl::shared_ptr<Texture2> const& baseTexture, eastl::shared_ptr<Texture2> const& normalTexture,
        SamplerState::Filter filter, SamplerState::Mode mode0, SamplerState::Mode mode1);

    // Member access.
    virtual void SetPVWMatrixConstant(eastl::shared_ptr<ConstantBuffer> const& buffer);

    inline eastl::shared_ptr<Texture2> const& GetBaseTexture() const
    {
        return mBaseTexture;
    }

    inline eastl::shared_ptr<Texture2> const& GetNormalTexture() const
    {
        return mNormalTexture;
    }

    inline eastl::shared_ptr<SamplerState> const& GetCommonSampler() const
    {
        return mCommonSampler;
    }

    // TODO: Move this into a compute shader to improve performance.
    static void ComputeLightVectors(eastl::shared_ptr<Node> const& node,
        Vector4<float> const& worldLightDirection);

private:
    // Compute a tangent at the vertex P0.  The triangle is
    // counterclockwise ordered, <P0,P1,P2>.
    static bool ComputeTangent(
        Vector3<float> const& position0, Vector2<float> const& tcoord0,
        Vector3<float> const& position1, Vector2<float> const& tcoord1,
        Vector3<float> const& position2, Vector2<float> const& tcoord2,
        Vector3<float>& tangent);

    // Pixel shader parameters.
	eastl::shared_ptr<Texture2> mBaseTexture;
	eastl::shared_ptr<Texture2> mNormalTexture;
	eastl::shared_ptr<SamplerState> mCommonSampler;
};

#endif