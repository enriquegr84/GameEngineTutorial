// David Eberly, Geometric Tools, Redmond WA 98052
// Copyright (c) 1998-2017
// Distributed under the Boost Software License, Version 1.0.
// http://www.boost.org/LICENSE_1_0.txt
// http://www.geometrictools.com/License/Boost/LICENSE_1_0.txt
// File Version: 3.0.0 (2016/06/19)

#ifndef DX11DRAWTARGET_H
#define DX11DRAWTARGET_H

#include "Graphic/Resource/Texture/DrawTarget.h"

#include "DX11TextureDS.h"
#include "DX11TextureRT.h"

class GRAPHIC_ITEM DX11DrawTarget : public DrawTarget
{
public:
    // Construction.
    DX11DrawTarget(DrawTarget const* target,
        eastl::vector<DX11TextureRT*>& rtTextures, DX11TextureDS* dsTexture);
    static eastl::shared_ptr<DrawTarget> Create(DrawTarget const* target,
		eastl::vector<GraphicObject*>& rtTextures, GraphicObject* dsTexture);

    // Member access.
	inline DrawTarget* GetDrawTarget() const;
    inline DX11TextureRT* GetRTTexture(unsigned int i) const;
    inline DX11TextureDS* GetDSTexture() const;

    // Used in the Renderer::Draw function.
    void Enable(ID3D11DeviceContext* context);
    void Disable(ID3D11DeviceContext* context);

protected:
	DrawTarget* mTarget;

private:
	eastl::vector<DX11TextureRT*> mRTTextures;
    DX11TextureDS* mDSTexture;

    // Convenient storage for enable/disable of targets.
	eastl::vector<ID3D11RenderTargetView*> mRTViews;
    ID3D11DepthStencilView* mDSView;

    // Temporary storage during enable/disable of targets.
    D3D11_VIEWPORT mSaveViewport;
	eastl::vector<ID3D11RenderTargetView*> mSaveRTViews;
    ID3D11DepthStencilView* mSaveDSView;
};

inline DrawTarget* DX11DrawTarget::GetDrawTarget() const
{
	return mTarget;
}

inline DX11TextureRT* DX11DrawTarget::GetRTTexture(unsigned int i) const
{
    return mRTTextures[i];
}

inline DX11TextureDS* DX11DrawTarget::GetDSTexture() const
{
    return mDSTexture;
}

#endif