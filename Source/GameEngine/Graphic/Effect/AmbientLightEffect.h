// David Eberly, Geometric Tools, Redmond WA 98052
// Copyright (c) 1998-2017
// Distributed under the Boost Software License, Version 1.0.
// http://www.boost.org/LICENSE_1_0.txt
// http://www.geometrictools.com/License/Boost/LICENSE_1_0.txt
// File Version: 3.0.0 (2016/06/19)

#ifndef AMBIENTLIGHTEFFECT_H
#define AMBIENTLIGHTEFFECT_H

#include "LightingEffect.h"
#include "Graphic/Effect/VisualEffect.h"

class GRAPHIC_ITEM AmbientLightEffect : public LightingEffect
{
public:
    // Construction.
    AmbientLightEffect(eastl::shared_ptr<ProgramFactory> const& factory,
        BufferUpdater const& updater, eastl::vector<eastl::string> const& path,
		eastl::shared_ptr<Material> const& material,
        eastl::shared_ptr<Lighting> const& lighting);

    // After you set or modify 'material' or 'lighting', call the update to
    // inform any listener that the corresponding constant buffer has changed.
    virtual void UpdateMaterialConstant();
    virtual void UpdateLightingConstant();

private:
    struct InternalMaterial
    {
        Vector4<float> emissive;
        Vector4<float> ambient;
    };

    struct InternalLighting
    {
        Vector4<float> ambient;
        Vector4<float> attenuation;
    };
};

#endif