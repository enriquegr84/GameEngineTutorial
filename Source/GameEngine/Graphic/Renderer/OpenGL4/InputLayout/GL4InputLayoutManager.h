// David Eberly, Geometric Tools, Redmond WA 98052
// Copyright (c) 1998-2018
// Distributed under the Boost Software License, Version 1.0.
// http://www.boost.org/LICENSE_1_0.txt
// http://www.geometrictools.com/License/Boost/LICENSE_1_0.txt
// File Version: 3.0.0 (2016/06/19)

#ifndef GL4INPUTLAYOUTMANAGER_H
#define GL4INPUTLAYOUTMANAGER_H

#include "Core/Threading/ThreadSafeMap.h"
#include "Graphic/InputLayout/InputLayoutManager.h"
#include "GL4InputLayout.h"

class GRAPHIC_ITEM GL4InputLayoutManager : public InputLayoutManager
{
public:
    // Construction and destruction.
    virtual ~GL4InputLayoutManager();
    GL4InputLayoutManager();

    // Management functions.  The Unbind(vbuffer) removes all layouts that
    // involve vbuffer.  The Unbind(vshader) is stubbed out because GL4 does
    // not require it, but we wish to have Unbind(GraphicObject const*) as
    // a base-class GraphicsEngine function.
    GL4InputLayout* Bind(GLuint programHandle, GLuint vbufferHandle, VertexBuffer const* vbuffer);
    virtual bool Unbind(VertexBuffer const* vbuffer) override;
    virtual bool Unbind(Shader const* vshader) override;
    virtual void UnbindAll() override;
    virtual bool HasElements() const override;

private:
    typedef eastl::pair<VertexBuffer const*, GLuint> VBPPair;

    class LayoutMap : public ThreadSafeMap<VBPPair, eastl::shared_ptr<GL4InputLayout>>
    {
    public:
        virtual ~LayoutMap();
        LayoutMap();

        void GatherMatch(VertexBuffer const* vbuffer, eastl::vector<VBPPair>& matches);
    };

    LayoutMap mMap;
};

#endif