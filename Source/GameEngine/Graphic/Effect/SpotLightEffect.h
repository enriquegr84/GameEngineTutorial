// David Eberly, Geometric Tools, Redmond WA 98052
// Copyright (c) 1998-2017
// Distributed under the Boost Software License, Version 1.0.
// http://www.boost.org/LICENSE_1_0.txt
// http://www.geometrictools.com/License/Boost/LICENSE_1_0.txt
// File Version: 3.0.0 (2016/06/19)

#ifndef SPOTLIGHTEFFECT_H
#define SPOTLIGHTEFFECT_H

#include "LightingEffect.h"


class GRAPHIC_ITEM SpotLightEffect : public LightingEffect
{
public:
    // Construction.  Set 'select' to 0 for per-vertex effects or to 1 for per-pixel effects.
    SpotLightEffect(eastl::shared_ptr<ProgramFactory> const& factory, 
		BufferUpdater const& updater, eastl::vector<eastl::string> const& path, int select,
		eastl::shared_ptr<Material> const& material,
		eastl::shared_ptr<Lighting> const& lighting,
		eastl::shared_ptr<LightCameraGeometry> const& geometry);

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
};

#endif