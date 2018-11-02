// David Eberly, Geometric Tools, Redmond WA 98052
// Copyright (c) 1998-2017
// Distributed under the Boost Software License, Version 1.0.
// http://www.boost.org/LICENSE_1_0.txt
// http://www.geometrictools.com/License/Boost/LICENSE_1_0.txt
// File Version: 3.0.0 (2016/06/19)

#ifndef SPOTLIGHTTEXTUREEFFECT_H
#define SPOTLIGHTTEXTUREEFFECT_H

#include "LightingEffect.h"


class GRAPHIC_ITEM SpotLightTextureEffect : public LightingEffect
{
public:
    // Construction.
	SpotLightTextureEffect(eastl::shared_ptr<ProgramFactory> const& factory,
		BufferUpdater const& updater, eastl::vector<eastl::string> const& path,
		eastl::shared_ptr<Material> const& material,
		eastl::shared_ptr<Lighting> const& lighting,
		eastl::shared_ptr<LightCameraGeometry> const& geometry,
		eastl::shared_ptr<Texture2> const& texture, SamplerState::Filter filter,
		SamplerState::Mode mode0, SamplerState::Mode mode1);

	// Member access.
	inline eastl::shared_ptr<Texture2> const& GetTexture() const;
	inline eastl::shared_ptr<SamplerState> const& GetSampler() const;

    // After you set or modify 'material', 'light', or 'geometry', call the update
    // to inform any listener that the corresponding constant buffer has changed.
    virtual void UpdateMaterialConstant();
    virtual void UpdateLightingConstant();
    virtual void UpdateGeometryConstant();

private:
    struct InternalMaterial
    {
        Vector4<float> emissive;
        Vector4<float> ambient;
        Vector4<float> diffuse;
        Vector4<float> specular;
    };

    struct InternalLighting
    {
        Vector4<float> ambient;
        Vector4<float> diffuse;
        Vector4<float> specular;
        Vector4<float> spotCutoff;
        Vector4<float> attenuation;
    };

    struct InternalGeometry
    {
        Vector4<float> lightModelPosition;
        Vector4<float> lightModelDirection;
        Vector4<float> cameraModelPosition;
    };

	eastl::shared_ptr<Texture2> mTexture;
	eastl::shared_ptr<SamplerState> mSampler;
};

inline eastl::shared_ptr<Texture2> const& SpotLightTextureEffect::GetTexture() const
{
	return mTexture;
}

inline eastl::shared_ptr<SamplerState> const& SpotLightTextureEffect::GetSampler() const
{
	return mSampler;
}

#endif