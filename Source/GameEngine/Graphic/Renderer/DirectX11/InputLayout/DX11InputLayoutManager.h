// David Eberly, Geometric Tools, Redmond WA 98052
// Copyright (c) 1998-2017
// Distributed under the Boost Software License, Version 1.0.
// http://www.boost.org/LICENSE_1_0.txt
// http://www.geometrictools.com/License/Boost/LICENSE_1_0.txt
// File Version: 3.0.0 (2016/06/19)

#ifndef DX11INPUTLAYOUTMANAGER_H
#define DX11INPUTLAYOUTMANAGER_H

#include "Core/Threading/ThreadSafeMap.h"
#include "Graphic/InputLayout/InputLayoutManager.h"
#include "DX11InputLayout.h"

class GRAPHIC_ITEM DX11InputLayoutManager : public InputLayoutManager
{
public:
    // Construction and destruction.
    ~DX11InputLayoutManager();
    DX11InputLayoutManager();

    // Management functions. The Unbind(vbuffer) removes all pairs that
    // involve vbuffer. The Unbind(vshader) removes all pairs that involve vshader.
    DX11InputLayout* Bind(ID3D11Device* device, VertexBuffer const* vbuffer, Shader const* vshader);
	virtual bool Unbind(VertexBuffer const* vbuffer) override;
	virtual bool Unbind(Shader const* vshader) override;
	virtual void UnbindAll() override;
	virtual bool HasElements() const override;

private:
    typedef eastl::pair<VertexBuffer const*, Shader const*> VBSPair;

    class LayoutMap : public ThreadSafeMap<VBSPair, eastl::shared_ptr<DX11InputLayout>>
    {
    public:
        virtual ~LayoutMap();
        LayoutMap();

        void GatherMatch(VertexBuffer const* vbuffer, eastl::vector<VBSPair>& matches);
        void GatherMatch(Shader const* vshader, eastl::vector<VBSPair>& matches);
    };

    LayoutMap mMap;
};

#endif