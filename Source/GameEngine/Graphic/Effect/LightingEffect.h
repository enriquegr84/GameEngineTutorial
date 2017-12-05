// David Eberly, Geometric Tools, Redmond WA 98052
// Copyright (c) 1998-2017
// Distributed under the Boost Software License, Version 1.0.
// http://www.boost.org/LICENSE_1_0.txt
// http://www.geometrictools.com/License/Boost/LICENSE_1_0.txt
// File Version: 3.0.1 (2016/11/13)

#ifndef LIGHTINGEFFECT_H
#define LIGHTINGEFFECT_H

#include "Graphic/Effect/VisualEffect.h"
#include "Graphic/Effect/Material.h"
#include "Graphic/Effect/Lighting.h"
#include "Graphic/Effect/LightCameraGeometry.h"


class GRAPHIC_ITEM LightingEffect : public VisualEffect
{
protected:
    // Construction (abstract base class).  The shader source code string
    // arrays must contain strings for any supported graphics API.
    LightingEffect(eastl::shared_ptr<ProgramFactory> const& factory,
        BufferUpdater const& updater, eastl::string path, 
		eastl::shared_ptr<Material> const& material,
		eastl::shared_ptr<Lighting> const& lighting,
		eastl::shared_ptr<LightCameraGeometry> const& geometry);

public:
    // Member access.
    inline void SetMaterial(eastl::shared_ptr<Material> const& material);
    inline void SetLighting(eastl::shared_ptr<Lighting> const& lighting);
    inline void SetGeometry(eastl::shared_ptr<LightCameraGeometry> const& geometry);
    inline eastl::shared_ptr<Material> const& GetMaterial() const;
    inline eastl::shared_ptr<Lighting> const& GetLighting() const;
    inline eastl::shared_ptr<LightCameraGeometry> const& GetGeometry() const;
    inline eastl::shared_ptr<ConstantBuffer> const& GetPVWMatrixConstant() const;
    inline eastl::shared_ptr<ConstantBuffer> const& GetMaterialConstant() const;
    inline eastl::shared_ptr<ConstantBuffer> const& GetLightingConstant() const;
    inline eastl::shared_ptr<ConstantBuffer> const& GetGeometryConstant() const;

    void SetPVWMatrixConstant(eastl::shared_ptr<ConstantBuffer> const& pvwMatrix);

    // After you set or modify 'material', 'light', or 'geometry', call the update
    // to inform any listener that the corresponding constant buffer has changed.
    // The derived classes construct the constant buffers to store the minimal
    // information from Material, Light, or Camera.  The pvw-matrix constant update
    // requires knowledge of the world transform of the object to which the effect
    // is attached, so its update must occur outside of this class.  Derived
    // classes update the system memory of the constant buffers and the base class
    // updates video memory.
    virtual void UpdateMaterialConstant();
    virtual void UpdateLightingConstant();
    virtual void UpdateGeometryConstant();

protected:
	eastl::shared_ptr<Material> mMaterial;
	eastl::shared_ptr<Lighting> mLighting;
	eastl::shared_ptr<LightCameraGeometry> mGeometry;

	eastl::shared_ptr<ConstantBuffer> mPVWMatrixConstant;

    // The derived-class constructors are responsible for creating these
    // according to their needs.
	eastl::shared_ptr<ConstantBuffer> mMaterialConstant;
	eastl::shared_ptr<ConstantBuffer> mLightingConstant;
	eastl::shared_ptr<ConstantBuffer> mGeometryConstant;
};

inline void LightingEffect::SetMaterial(eastl::shared_ptr<Material> const& material)
{
    mMaterial = material;
}

inline void LightingEffect::SetLighting(eastl::shared_ptr<Lighting> const& lighting)
{
    mLighting = lighting;
}

inline void LightingEffect::SetGeometry(eastl::shared_ptr<LightCameraGeometry> const& geometry)
{
    mGeometry = geometry;
}

inline eastl::shared_ptr<Material> const& LightingEffect::GetMaterial() const
{
    return mMaterial;
}

inline eastl::shared_ptr<Lighting> const& LightingEffect::GetLighting() const
{
    return mLighting;
}

inline eastl::shared_ptr<LightCameraGeometry> const& LightingEffect::GetGeometry() const
{
    return mGeometry;
}

inline eastl::shared_ptr<ConstantBuffer> const& LightingEffect::GetPVWMatrixConstant() const
{
    return mPVWMatrixConstant;
}

inline eastl::shared_ptr<ConstantBuffer> const& LightingEffect::GetMaterialConstant() const
{
    return mMaterialConstant;
}

inline eastl::shared_ptr<ConstantBuffer> const& LightingEffect::GetLightingConstant() const
{
    return mLightingConstant;
}

inline eastl::shared_ptr<ConstantBuffer> const& LightingEffect::GetGeometryConstant() const
{
    return mGeometryConstant;
}

#endif