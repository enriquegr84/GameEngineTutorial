// Geometric Tools, LLC
// Copyright (c) 1998-2014
// Distributed under the Boost Software License, Version 1.0.
// http://www.boost.org/LICENSE_1_0.txt
// http://www.geometrictools.com/License/Boost/LICENSE_1_0.txt
//
// File Version: 5.0.2 (2011/08/13)

#ifndef RENDERER_H
#define RENDERER_H

#include "Core/Threading/ThreadSafeMap.h"

#include "Graphic/Graphic.h"

// The Renderer class is an abstract interface that is implemented by each
// platform of interest (Direct3D, OpenGL, software, embedded devices, etc.).
// The class description is factored into two sections. The first section
// lists the platform-independent functions and data. These are implemented
// in LibGraphics files Renderer.{h,inl,cpp}. The second section lists
// the platform-dependent functions and data.  These are implemented by each
// platform of interest in the LibRenderers folder.

class GRAPHIC_ITEM Renderer
{
public:

	// Construction and destruction.
	Renderer();
	~Renderer();

	// Support for drawing to offscreen memory (i.e. not to the back buffer).
	// The DrawTarget object encapsulates render targets (color information)
	// and depth-stencil target.
	virtual void Enable(eastl::shared_ptr<DrawTarget> const& target) = 0;
	virtual void Disable(eastl::shared_ptr<DrawTarget> const& target) = 0;

	// Viewport management.  The measurements are in window coordinates.  The
	// origin of the window is (x,y), the window width is w, and the window
	// height is h.  The depth range for the view volume is [zmin,zmax].  The
	// DX12 viewport is left-handed with origin the upper-left corner of the
	// window, the x-axis is directed rightward, the y-axis is directed
	// downward, and the depth range is a subset of [0,1].
	virtual void SetViewport(int x, int y, int w, int h) = 0;
	virtual void GetViewport(int& x, int& y, int& w, int& h) const = 0;
	virtual void SetDepthRange(float zmin, float zmax) = 0;
	virtual void GetDepthRange(float& zmin, float& zmax) const = 0;

	// Window resizing.
	virtual bool Resize(unsigned int w, unsigned int h) = 0;

	// Support for clearing the color, depth, and stencil back buffers.
	virtual void ClearColorBuffer() = 0;
	virtual void ClearDepthBuffer() = 0;
	virtual void ClearStencilBuffer() = 0;
	virtual void ClearBuffers() = 0;

	// Support for drawing.  If occlusion queries are enabled, the return
	// values are the number of samples that passed the depth and stencil
	// tests, effectively the number of pixels drawn.  If occlusion queries
	// are disabled, the functions return 0.

	// Draw geometric primitives.
	uint64_t Draw(Visual* visual);
	uint64_t Draw(eastl::vector<Visual*> const& visuals);
	uint64_t Draw(eastl::shared_ptr<Visual> const& visual);
	uint64_t Draw(eastl::vector<eastl::shared_ptr<Visual>> const& visuals);

	// Draw 2D text.
	uint64_t Draw(int x, int y, eastl::array<float, 4> const& color, eastl::string const& message);

	inline const Vector2<unsigned int>& GetScreenSize() const;

	// Support for clearing the color, depth, and stencil back buffers.
	inline void SetClearColor(eastl::array<float, 4> const& clearColor);
	inline void SetClearDepth(float clearDepth);
	inline void SetClearStencil(unsigned int clearStencil);
	inline eastl::array<float, 4> const& GetClearColor() const;
	inline float GetClearDepth() const;
	inline unsigned int GetClearStencil() const;
	virtual void DisplayColorBuffer(unsigned int syncInterval) = 0;

	// Support for bitmapped fonts used in text rendering.  The default font
	// is Arial (height 18, no italics, no bold).
	virtual void SetFont(eastl::shared_ptr<Font> const& font);
	inline eastl::shared_ptr<Font> const& GetFont() const;
	inline void SetDefaultFont();
	inline eastl::shared_ptr<Font> const& GetDefaultFont() const;

	// Global drawing state.
	virtual void SetBlendState(eastl::shared_ptr<BlendState> const& state) = 0;
	inline eastl::shared_ptr<BlendState> const& GetBlendState() const;
	inline void SetDefaultBlendState();
	inline eastl::shared_ptr<BlendState> const& GetDefaultBlendState() const;

	virtual void SetDepthStencilState(eastl::shared_ptr<DepthStencilState> const& state) = 0;
	inline eastl::shared_ptr<DepthStencilState> const& GetDepthStencilState() const;
	inline void SetDefaultDepthStencilState();
	inline eastl::shared_ptr<DepthStencilState> const& GetDefaultDepthStencilState() const;

	virtual void SetRasterizerState(eastl::shared_ptr<RasterizerState> const& state) = 0;
	inline eastl::shared_ptr<RasterizerState> const& GetRasterizerState() const;
	inline void SetDefaultRasterizerState();
	inline eastl::shared_ptr<RasterizerState> const& GetDefaultRasterizerState() const;

	// Support for copying from CPU to GPU via mapped memory.
	virtual bool Update(eastl::shared_ptr<Buffer> const& buffer) = 0;
	virtual bool Update(eastl::shared_ptr<TextureSingle> const& texture) = 0;
	virtual bool Update(eastl::shared_ptr<TextureSingle> const& texture, unsigned int level) = 0;
	virtual bool Update(eastl::shared_ptr<TextureArray> const& textureArray) = 0;
	virtual bool Update(eastl::shared_ptr<TextureArray> const& textureArray, unsigned int item, unsigned int level) = 0;

	// Graphics object management.  The Bind function creates a graphics
	// API-specific object that corresponds to the input GameEngine object.
	// GraphicsEngine manages this bridge mapping internally.  The Unbind
	// function destroys the graphics API-specific object.  These may be
	// called explicitly, but the engine is designed to create on demand
	// and to destroy on device destruction.
	GraphicObject* Bind(eastl::shared_ptr<GraphicObject> const& object);
	DrawTarget* Bind(eastl::shared_ptr<DrawTarget> const& target);

	GraphicObject* Get(eastl::shared_ptr<GraphicObject> const& object) const;
	DrawTarget* Get(eastl::shared_ptr<DrawTarget> const& target) const;

	inline bool Unbind(eastl::shared_ptr<GraphicObject> const& object);
	inline bool Unbind(eastl::shared_ptr<DrawTarget> const& target);

protected:
	// Support for drawing.  If occlusion queries are enabled, the return
	// values are the number of samples that passed the depth and stencil
	// tests, effectively the number of pixels drawn.  If occlusion queries
	// are disabled, the functions return 0.
	virtual uint64_t DrawPrimitive(
		eastl::shared_ptr<VertexBuffer> const& vbuffer,
		eastl::shared_ptr<IndexBuffer> const& ibuffer,
		eastl::shared_ptr<VisualEffect> const& effect) = 0;

	// Support for GOListener::OnDestroy and DTListener::OnDestroy, because
	// they are passed raw pointers from resource destructors.  These are
	// also used by the Unbind calls whose inputs are eastl::shared_ptr<T>.
	bool Unbind(GraphicObject const* object);
	bool Unbind(DrawTarget const* target);

	// Bridge pattern to create graphics API-specific objects that correspond
	// to front-end objects. The Bind, Get, and Unbind operations act on
	// these maps.
	ThreadSafeMap<GraphicObject const*, eastl::shared_ptr<GraphicObject>> mGraphicObjects;
	ThreadSafeMap<DrawTarget const*, eastl::shared_ptr<DrawTarget>> mDrawTargets;
	eastl::unique_ptr<InputLayoutManager> mInputLayouts;

	// Creation functions for adding objects to the bridges. The function
	// pointers are assigned during construction.
	typedef eastl::shared_ptr<GraphicObject>(*CreateGraphicObject)(void*, GraphicObject const*);
	eastl::array<CreateGraphicObject, GE_NUM_TYPES> mCreateGraphicObject;
	void* mGraphicObjectCreator;

	typedef eastl::shared_ptr<DrawTarget>(*CreateGEDrawTarget)(DrawTarget const*,
		eastl::vector<GraphicObject*>&, GraphicObject*);
	CreateGEDrawTarget mCreateDrawTarget;

	// Helpers for construction and destruction.
	void CreateDefaultGlobalState();
	virtual void DestroyDefaultGlobalState();

    // Construction parameters.
	Vector2<unsigned int> mScreenSize;

    int mNumMultisamples;

    // Framebuffer clearing.
	eastl::array<float, 4> mClearColor;
    float mClearDepth;
    unsigned int mClearStencil;

	// Fonts for text rendering.
	eastl::shared_ptr<Font> mDefaultFont;
	eastl::shared_ptr<Font> mActiveFont;

	// Global state.
	eastl::shared_ptr<BlendState> mDefaultBlendState;
	eastl::shared_ptr<BlendState> mActiveBlendState;
	eastl::shared_ptr<DepthStencilState> mDefaultDepthStencilState;
	eastl::shared_ptr<DepthStencilState> mActiveDepthStencilState;
	eastl::shared_ptr<RasterizerState> mDefaultRasterizerState;
	eastl::shared_ptr<RasterizerState> mActiveRasterizerState;
};

inline bool Renderer::Unbind(eastl::shared_ptr<GraphicObject> const& object)
{
	return Unbind(object.get());
}

//! returns screen size
inline const Vector2<unsigned int>& Renderer::GetScreenSize() const
{
	return mScreenSize;
}
//----------------------------------------------------------------------------
inline void Renderer::SetClearStencil(unsigned int clearStencil)
{
	mClearStencil = clearStencil;
}
//----------------------------------------------------------------------------
inline void Renderer::SetClearColor(eastl::array<float, 4> const& clearColor)
{
	mClearColor = clearColor;
}
//----------------------------------------------------------------------------
inline void Renderer::SetClearDepth(float clearDepth)
{
	mClearDepth = clearDepth;
}
//----------------------------------------------------------------------------
inline eastl::array<float, 4> const& Renderer::GetClearColor() const
{
	return mClearColor;
}
//----------------------------------------------------------------------------
inline float Renderer::GetClearDepth() const
{
	return mClearDepth;
}
//----------------------------------------------------------------------------
inline unsigned int Renderer::GetClearStencil() const
{
	return mClearStencil;
}
//----------------------------------------------------------------------------
inline eastl::shared_ptr<Font> const& Renderer::GetFont() const
{
	return mActiveFont;
}
//----------------------------------------------------------------------------
inline void Renderer::SetDefaultFont()
{
	SetFont(mDefaultFont);
}
//----------------------------------------------------------------------------
inline eastl::shared_ptr<Font> const& Renderer::GetDefaultFont() const
{
	return mDefaultFont;
}
//----------------------------------------------------------------------------
inline eastl::shared_ptr<BlendState> const& Renderer::GetBlendState() const
{
	return mActiveBlendState;
}
//----------------------------------------------------------------------------
inline void Renderer::SetDefaultBlendState()
{
	SetBlendState(mDefaultBlendState);
}
//----------------------------------------------------------------------------
inline eastl::shared_ptr<BlendState> const& Renderer::GetDefaultBlendState() const
{
	return mDefaultBlendState;
}
//----------------------------------------------------------------------------
inline eastl::shared_ptr<DepthStencilState> const& Renderer::GetDepthStencilState() const
{
	return mActiveDepthStencilState;
}
//----------------------------------------------------------------------------
inline void Renderer::SetDefaultDepthStencilState()
{
	SetDepthStencilState(mDefaultDepthStencilState);
}
//----------------------------------------------------------------------------
inline eastl::shared_ptr<DepthStencilState> const& Renderer::GetDefaultDepthStencilState() const
{
	return mDefaultDepthStencilState;
}
//----------------------------------------------------------------------------
inline eastl::shared_ptr<RasterizerState> const& Renderer::GetRasterizerState() const
{
	return mActiveRasterizerState;
}
//----------------------------------------------------------------------------
inline void Renderer::SetDefaultRasterizerState()
{
	SetRasterizerState(mDefaultRasterizerState);
}
//----------------------------------------------------------------------------
inline eastl::shared_ptr<RasterizerState> const& Renderer::GetDefaultRasterizerState() const
{
	return mDefaultRasterizerState;
}

#endif