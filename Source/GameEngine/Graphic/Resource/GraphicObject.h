// David Eberly, Geometric Tools, Redmond WA 98052
// Copyright (c) 1998-2017
// Distributed under the Boost Software License, Version 1.0.
// http://www.boost.org/LICENSE_1_0.txt
// http://www.geometrictools.com/License/Boost/LICENSE_1_0.txt
// File Version: 3.0.0 (2016/06/19)

#ifndef GRAPHICOBJECT_H
#define GRAPHICOBJECT_H

#include "Graphic/GraphicStd.h"

#include <mutex>

#include <EASTL/set.h>
#include <EASTL/string.h>
#include <EASTL/shared_ptr.h>

// The current hierarchy of GameEngine graphics objects is
//  GraphicObject
//      Resource
//          Buffer
//              ConstantBuffer
//              TextureBuffer
//              VertexBuffer
//              IndexBuffer
//              StructuredBuffer
//              TypedBuffer
//              RawBuffer
//              IndirectArgumentsBuffer
//          Texture
//              TextureSingle
//                  Texture1
//                  Texture2
//                      TextureRT
//                      TextureDS
//                  Texture3
//              TextureArray
//                  Texture1Array
//                  Texture2Array
//                  TextureCube
//                  TextureCubeArray
//      Shader
//          VertexShader
//          GeometryShader
//          PixelShader
//          ComputeShader
//      DrawingState
//          SamplerState
//          BlendState
//          DepthStencilState
//          RasterizerState
//
// The corresponding hierarchy of GameEngine DX11* bridge objects is the same
// but with class names prepended by DX11 (e.g. DX11GraphicObject).
//
// The hierarchy of DX11 interfaces is the following
//  ID3D11DeviceChild
//      ID3D11Resource
//          ID3D11Buffer
//          ID3D11Texture1D
//          ID3D11Texture2D
//          ID3D11Texture3D
//      ID3D11VertexShader
//      ID3D11GeometryShader
//      ID3D11PixelShader
//      ID3D11ComputeShader
//      ID3D11SamplerState
//      ID3D11BlendState
//      ID3D11DepthStencilState
//      ID3D11RasterizerState
//
// The GraphicObjectType enumeration is for run-time type information.
// The typeid() mechanism does not work in DX11Engine::Unbind(...), because
// the listeners receive 'this' from a base class during a destructor call.

enum GRAPHIC_ITEM GraphicObjectType
{
    GE_GRAPHICS_OBJECT,  // abstract
        GE_RESOURCE,  // abstract
            GE_BUFFER,  // abstract
                GE_CONSTANT_BUFFER,
                GE_TEXTURE_BUFFER,
                GE_VERTEX_BUFFER,
                GE_INDEX_BUFFER,
                GE_STRUCTURED_BUFFER,
                GE_TYPED_BUFFER,
                GE_RAW_BUFFER,
                GE_INDIRECT_ARGUMENTS_BUFFER,
            GE_TEXTURE,  // abstract
                GE_TEXTURE_SINGLE,  // abstract
                    GE_TEXTURE1,
                    GE_TEXTURE2,
                        GE_TEXTURE_RT,
                        GE_TEXTURE_DS,
                    GE_TEXTURE3,
                GE_TEXTURE_ARRAY,  // abstract
                    GE_TEXTURE1_ARRAY,
                    GE_TEXTURE2_ARRAY,
                    GE_TEXTURE_CUBE,
                    GE_TEXTURE_CUBE_ARRAY,
        GE_SHADER,  // abstract
            GE_VERTEX_SHADER,
            GE_GEOMETRY_SHADER,
            GE_PIXEL_SHADER,
            GE_COMPUTE_SHADER,
        GE_DRAWING_STATE,  // abstract
            GE_SAMPLER_STATE,
            GE_BLEND_STATE,
            GE_DEPTH_STENCIL_STATE,
            GE_RASTERIZER_STATE,
    GE_NUM_TYPES
};

class GRAPHIC_ITEM GraphicObject
{
public:
    // This is an abstract base class that is used for bridging GameEngine
    // graphics objects with DX11 graphics objects.  The latter are
    // represented by the ID3D11DeviceChild interface.
    virtual ~GraphicObject();

    // Run-time type information.
    inline GraphicObjectType GetType() const;
    inline bool IsBuffer() const;
    inline bool IsTexture() const;
    inline bool IsTextureArray() const;
    inline bool IsShader() const;
    inline bool IsDrawingState() const;

    // Naming support, used in the DX11 debug layer.  The default name is "".
    // If you want the name to show up in the DX11 destruction messages when
    // the associated DX11GraphicsObject is destroyed, set the name to
    // something other than "".
    inline void SetName(eastl::wstring const& name);
    inline eastl::wstring const& GetName() const;

    // Listeners subscribe to receive notification when a GraphicsObject is
    // about to be destroyed.  The intended use is for the DX11Engine objects
    // to destroy corresponding ID3D11DeviceChild objects.
    class GRAPHIC_ITEM ListenerForDestruction
    {
    public:
        virtual ~ListenerForDestruction() { /**/ }
        ListenerForDestruction() { /**/ }
        virtual void OnDestroy(GraphicObject const*) { /**/ }
    };

    static void SubscribeForDestruction(eastl::shared_ptr<ListenerForDestruction> const& listener);
    static void UnsubscribeForDestruction(eastl::shared_ptr<ListenerForDestruction> const& listener);

protected:
    // This is an abstract base class.
    GraphicObject();

    GraphicObjectType mType;
	eastl::wstring mName;

private:
    // Support for listeners for destruction (LFD).
    static std::recursive_mutex msLFDMutex;
    static eastl::set<eastl::shared_ptr<ListenerForDestruction>> msLFDSet;
};

inline GraphicObjectType GraphicObject::GetType() const
{
    return mType;
}

inline bool GraphicObject::IsBuffer() const
{
	return GE_BUFFER <= mType && mType <= GE_INDIRECT_ARGUMENTS_BUFFER;
}

inline bool GraphicObject::IsTexture() const
{
	return GE_TEXTURE_SINGLE <= mType && mType <= GE_TEXTURE3;
}

inline bool GraphicObject::IsTextureArray() const
{
	return GE_TEXTURE_ARRAY <= mType && mType <= GE_TEXTURE_CUBE_ARRAY;
}

inline bool GraphicObject::IsShader() const
{
	return GE_SHADER <= mType && mType <= GE_COMPUTE_SHADER;
}

inline bool GraphicObject::IsDrawingState() const
{
	return GE_DRAWING_STATE <= mType && mType <= GE_RASTERIZER_STATE;
}

inline void GraphicObject::SetName(eastl::wstring const& name)
{
    mName = name;
}

inline eastl::wstring const& GraphicObject::GetName() const
{
    return mName;
}

#endif