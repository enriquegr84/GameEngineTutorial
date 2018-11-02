// David Eberly, Geometric Tools, Redmond WA 98052
// Copyright (c) 1998-2018
// Distributed under the Boost Software License, Version 1.0.
// http://www.boost.org/LICENSE_1_0.txt
// http://www.geometrictools.com/License/Boost/LICENSE_1_0.txt
// File Version: 3.0.0 (2016/06/19)

#ifndef GL4SAMPLERSTATE_H
#define GL4SAMPLERSTATE_H

#include "Graphic/State/SamplerState.h"
#include "GL4DrawingState.h"

class GRAPHIC_ITEM GL4SamplerState : public GL4DrawingState
{
public:
    // Construction and destruction.
    virtual ~GL4SamplerState();
    GL4SamplerState(SamplerState const* samplerState);
    static eastl::shared_ptr<GraphicObject> Create(void* unused, GraphicObject const* object);

    // Member access.
    inline SamplerState* GetSamplerState();

private:
    // Conversions from Engine values to GL4 values.
    static GLint const msMode[];
};

inline SamplerState* GL4SamplerState::GetSamplerState()
{
    return static_cast<SamplerState*>(mGObject);
}

#endif