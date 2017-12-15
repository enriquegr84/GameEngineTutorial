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

//! An enum for all types of renderes the Engine supports.
enum GRAPHIC_ITEM RendererType
{
	//! Null driver, useful for applications to run the engine without visualisation.
	/** The null device is able to load textures, but does not
	render and display any graphics. */
	RT_NULL,

	//! The Irrlicht Engine Software renderer.
	/** Runs on all platforms, with every hardware. It should only
	be used for 2d graphics, but it can also perform some primitive
	3d functions. These 3d drawing functions are quite fast, but
	very inaccurate, and don't even support clipping in 3D mode. */
	RT_SOFTWARE,

	//! Direct3D 11 device, only available on Win32 platforms.
	/** Performs hardware accelerated rendering of 3D and 2D
	primitives. */
	RT_DIRECT3D11,

	//! OpenGL device, available on most platforms.
	/** Performs hardware accelerated rendering of 3D and 2D
	primitives. */
	RT_OPENGL
};

/*
	3D Graphics Pipeline

	It describes the process of getting a 3D scene up on a screen. It implies a beginning that 
	accepts raw materials, a process that occurs along the way, and a conclusion from which the 
	refined result pours. This is analogous to what happens inside 3D game engines. The raw 
	materials or resources used in the pipeline include:
	-	Geometry: Everything you see on the screen starts with descriptions of their shape. Each 
		shape is broken down into triangles, each of which is composed of three vertices, which 
		is a basic drawable element in 3D engines. Some renderers support points, lines, and even 
		curved surfaces, but the triangle is by far the most common. 
		Meshes are collections of triangles.
	-	Materials: These elements describe appearance. You can imagine materials as paint that 
		you apply to the geometry. Each material can describe colors, translucency, and how the 
		material reflects light.
	-	Textures: These are images that can be applied to objects, just as you might have applied 
		decals to plastic models or wallpaper to your kitchen.
	-	Lights: You must have light to see anything. Light can affect an entire scene or have a 
		local effect that mimics a spotlight.
	-	Camera: Records the scene onto a render target, such as the screen. It even describes 
		what kind of lens is used, such as a wide or narrow angle lens. You can have multiple 
		cameras to split the screen for a multiplayer game or render a different view to create 
		a rearview mirror.
	-	Shader: A shader is a bit of code that runs on the video card. It is able to consume
		all of the above and calculate exactly what pixels to paint to the screen in the right 
		positions to faithfully render every triangle the camera can see. Shaders typically work 
		on either vertex positions or individual pixels inside a triangle, but in truth they can 
		be much more general than that.

	Some of the processes applied to the raw materials include the following:
	-	Transformations: The same object can appear in the world in different orientations and 
		locations. Objects are manipulated in 3D space via matrix multiplications.
	-	Culling: At the object level, visible objects are inserted into a list of objects that
		are in view of the camera; at the triangle level, individual triangles are removed if 
		they don’t meet certain criteria, such as facing the camera.
	-	Lighting: Each object in range of a light source is illuminated by calculating
		additional colors applied to each vertex.
	-	Rasterization: Polygons are drawn, sometimes in many passes, to handle additional
		effects such as lighting and shadows.
		
		Graphics pipelines also come in two flavors: fixed-function and programmable. The
		fixed-function pipeline sets rendering states and then uses those states to draw 
		elements with those exact states. A programmable pipeline is much more flexible, it 
		allows programmers to have detailed control over every pixel on the screen.
*/

/*
	The Renderer class is an abstract interface that is implemented by each platform of 
	interest (Direct3D, OpenGL, software, embedded devices, etc.). The class description 
	is factored into two sections. The first section lists the platform-independent 
	functions and data. These are implemented in LibGraphics files Renderer.{h,inl,cpp}. 
	The second section lists the platform-dependent functions and data. These are implemented 
	by each platform of interest in the LibRenderers folder.
*/
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

	// Draw 2D text
	uint64_t Draw(int x, int y, eastl::array<float, 4> const& color, eastl::wstring const& message);

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
	inline void SetDefaultFont(eastl::shared_ptr<Font> const& font);
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

	// Getter for the main global renderer. This is the renderer that is used by the majority of the 
	// engine, though you are free to define your own as long as you instantiate it.
	// It is not valid to have more than one global renderer.
	static Renderer* Get(void);

protected:

	static Renderer* mRenderer;

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
inline void Renderer::SetDefaultFont(eastl::shared_ptr<Font> const& font)
{
	mDefaultFont = font;
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