// David Eberly, Geometric Tools, Redmond WA 98052
// Copyright (c) 1998-2018
// Distributed under the Boost Software License, Version 1.0.
// http://www.boost.org/LICENSE_1_0.txt
// http://www.geometrictools.com/License/Boost/LICENSE_1_0.txt
// File Version: 3.0.2 (2017/06/23)

#ifndef GL4RENDERER_H
#define GL4RENDERER_H

#include "Graphic/Renderer/Renderer.h"
#include "Graphic/Renderer/OpenGL4/InputLayout/GL4InputLayoutManager.h"

class GL4GraphicObject;
class GL4DrawTarget;

class GRAPHIC_ITEM GL4Renderer : public Renderer
{
// Interface specific to GL4.
public:
    // Abstract base class for platform-specific OpenGL engines.
    virtual ~GL4Renderer();
    GL4Renderer();

    // Currently, OpenGL 4.3 or later is required for compute shaders.
    // Because of deferred construction via Initialize(...), the
    // requirements are not known until that function is called.
    // TODO: Redesign the OpenGL system to allow for earlier versions of
    // OpenGL if the sample application does not require OpenGL 4.3.  This
    // is akin to setting the parameters.featureLevel for Direct3D 11
    // applications.
    inline bool MeetsRequirements() const;

    // Allow the user to switch between OpenGL contexts when there are
    // multiple instances of GL4Renderer in an application.
    virtual bool IsActive() const = 0;
    virtual void MakeActive() = 0;

protected:
    // Helpers for construction and destruction.
    virtual bool Initialize(int requiredMajor, int requiredMinor);
    void Terminate();

    // Engine GL4 requires OpenGL 4.3 or later for compute shaders.
    int mMajor, mMinor;
    bool mMeetsRequirements;

private:
    // Support for drawing.
    uint64_t DrawPrimitive(VertexBuffer const* vbuffer, IndexBuffer const* ibuffer);

    // Support for enabling and disabling resources used by shaders.
    bool EnableShaders(eastl::shared_ptr<VisualEffect> const& effect, GLuint program);
    void DisableShaders(eastl::shared_ptr<VisualEffect> const& effect, GLuint program);
    void Enable(Shader const* shader, GLuint program);
    void Disable(Shader const* shader, GLuint program);
    void EnableCBuffers(Shader const* shader, GLuint program);
    void DisableCBuffers(Shader const* shader, GLuint program);
    void EnableTBuffers(Shader const* shader, GLuint program);
    void DisableTBuffers(Shader const* shader, GLuint program);
    void EnableSBuffers(Shader const* shader, GLuint program);
    void DisableSBuffers(Shader const* shader, GLuint program);
    void EnableRBuffers(Shader const* shader, GLuint program);
    void DisableRBuffers(Shader const* shader, GLuint program);
    void EnableTextures(Shader const* shader, GLuint program);
    void DisableTextures(Shader const* shader, GLuint program);
    void EnableTextureArrays(Shader const* shader, GLuint program);
    void DisableTextureArrays(Shader const* shader, GLuint program);
    void EnableSamplers(Shader const* shader, GLuint program);
    void DisableSamplers(Shader const* shader, GLuint program);

    // A front-end object (hidden from the user) is created for each
    // atomic counter buffer object declared in use for a shader that
    // is executed.  After execution, these objects are left for use
    // the next time.  They are only destroyed to create new ones
    // when a larger buffer is required, but the buffer size never
    // becomes smaller.  A RawBuffer type is used here because it
    // is by definition 4-bytes per element where 4 bytes is the
    // size for each atomic_uint counter.
    eastl::vector<eastl::shared_ptr<RawBuffer>> mAtomicCounterRawBuffers;

    // Keep track of available texture sampler/image units
    // and uniform/shaderstorage buffer units.
    // If unit is in use, then link count is positive and
    // program+index bound to that unit will be stored.
    class ProgramIndexUnitMap
    {
    public:
        ~ProgramIndexUnitMap();
        ProgramIndexUnitMap();

        int AcquireUnit(GLint program, GLint index);
        int GetUnit(GLint program, GLint index) const;
        void ReleaseUnit(unsigned index);
        unsigned GetUnitLinkCount(unsigned unit) const;
        bool GetUnitProgramIndex(unsigned unit, GLint &program, GLint &index) const;
    private:
        struct LinkInfo
        {
            unsigned linkCount;
            GLint program;
            GLint index;
        };

        eastl::vector<LinkInfo> mLinkMap;
    };
    ProgramIndexUnitMap mTextureSamplerUnitMap;
    ProgramIndexUnitMap mTextureImageUnitMap;
    ProgramIndexUnitMap mUniformUnitMap;
    ProgramIndexUnitMap mShaderStorageUnitMap;


// Overrides from GraphicsEngine.
public:
    // Viewport management.  The measurements are in window coordinates.  The
    // origin of the window is (x,y), the window width is w, and the window
    // height is h.  The depth range for the view volume is [zmin,zmax].  The
    // OpenGL viewport is right-handed with origin the lower-left corner of
    // the window, the x-axis is directed rightward, the y-axis is directed
    // upward, and the depth range is a subset of [-1,1].
    virtual void SetViewport(int x, int y, int w, int h) override;
    virtual void GetViewport(int& x, int& y, int& w, int& h) const override;
    virtual void SetDepthRange(float zmin, float zmax) override;
    virtual void GetDepthRange(float& zmin, float& zmax) const override;

    // Window resizing.
    virtual bool Resize(unsigned int w, unsigned int h) override;

    // Support for clearing the color, depth, and stencil back buffers.
    virtual void ClearColorBuffer() override;
    virtual void ClearDepthBuffer() override;
    virtual void ClearStencilBuffer() override;
    virtual void ClearBuffers() override;

    // Global drawing state.  The default states are shown in GteBlendState.h,
    // GteDepthStencil.h, and GteRasterizerState.h.
    virtual void SetBlendState(eastl::shared_ptr<BlendState> const& state) override;
    virtual void SetDepthStencilState(eastl::shared_ptr<DepthStencilState> const& state) override;
    virtual void SetRasterizerState(eastl::shared_ptr<RasterizerState> const& state) override;

    // Support for drawing to offscreen memory (i.e. not to the back buffer).
    // The DrawTarget object encapsulates render targets (color information)
    // and depth-stencil target.
    virtual void Enable(eastl::shared_ptr<DrawTarget> const& target) override;
    virtual void Disable(eastl::shared_ptr<DrawTarget> const& target) override;

    // Support for copying from CPU to GPU via mapped memory.
    virtual bool Update(eastl::shared_ptr<Buffer> const& buffer) override;
    virtual bool Update(eastl::shared_ptr<TextureSingle> const& texture) override;
    virtual bool Update(eastl::shared_ptr<TextureSingle> const& texture, unsigned int level) override;
    virtual bool Update(eastl::shared_ptr<TextureArray> const& textureArray) override;
    virtual bool Update(eastl::shared_ptr<TextureArray> const& textureArray, unsigned int item, unsigned int level) override;

protected:
    // Support for drawing.  If occlusion queries are enabled, the return
    // value is the number of samples that passed the depth and stencil
    // tests, effectively the number of pixels drawn.  If occlusion queries
    // are disabled, the function returns 0.
    virtual uint64_t DrawPrimitive(
        eastl::shared_ptr<VertexBuffer> const& vbuffer,
        eastl::shared_ptr<IndexBuffer> const& ibuffer,
        eastl::shared_ptr<VisualEffect> const& effect) override;
};


inline bool GL4Renderer::MeetsRequirements() const
{
    return mMeetsRequirements;
}

#endif