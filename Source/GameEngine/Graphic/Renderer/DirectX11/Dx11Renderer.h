// Geometric Tools, LLC
// Copyright (c) 1998-2014
// Distributed under the Boost Software License, Version 1.0.
// http://www.boost.org/LICENSE_1_0.txt
// http://www.geometrictools.com/License/Boost/LICENSE_1_0.txt
//
// File Version: 5.0.2 (2011/08/13)

#ifndef DX11RENDERER_H
#define DX11RENDERER_H

#include "Graphic/GraphicStd.h"
#include "Graphic/Graphic.h"

class DX11InputLayoutManager;
class DX11GeometryShader;
class DX11PixelShader;
class DX11Shader;
class DX11VertexShader;

/*
A Direct3D device represents a display adapter, an abstraction of the 
underlying graphics capability of the computer. A device context also 
represents the display adapter, but within a particular setting. The 
device context is encapsulated in the rendererdata interface, which 
generates rendering commands using the resources of a device

D3D11 Graphics Pipeline:

1. Input-Assembler Stage (IA) is the entry point of the graphics pipeline,
where you supply vertex and index data for the objects you want to render.
The IA stage "assaembles" this data into primitives and sends the
corresponding output to the vertex shader stage.
2. Vertex Shader Stage (VS) processes the primitives interpreted by the
input-assembler stage.
3. Tessellation Stage (TS) is a process that adds detail to objects directly
on the GPU. Generally, more geometric detail (that is, more vertices) yields
a better-looking render. Traditionally, it is used level of details system,
LOD are rendered based on the distance of the object from the camera.
Hardware tessellation enables you to subdivide an object dynamically and
without the cost of additional geometry passed to the input-assembler stage.
This allows for a dynamic LOD system and less utilization of the graphics bus.
This three stages correspond to tessellation:
-	The hull shader stage (HS programmable)
-	The tessellator stage
-	The domain shader stage (DS programmable)
4. Geometry Shader Stage (GS) operates on complete primitives (such as points,
lines and triangles).
5. Rasterizer Stage (RS) converts primitives into a raster image.
6. Pixels Shader Stage (PS) executes shader code against each pixel input from
the rasterizer stage and ouputs a color.
7. Output-Merger Stage (OM) produces the final rendered pixel. this stage isn't
programmable but behaves through customizable pipeline states. The OM stage
generates the final pixel through a combination of these states, the output
from the pixel shader stage, and the existing contents of the render target.
*/
class GRAPHIC_ITEM Dx11Renderer : public Renderer
{
public:

	// Construction and destruction.
	Dx11Renderer();
	~Dx11Renderer();

	// Constructors for computing.  The first constructor uses the default
	// adapter and tries for DX11.0 hardware acceleration (the maximum feature
	// level) without debugging support; it is equivalent to the second
	// constructor call:
	Dx11Renderer(D3D_FEATURE_LEVEL minFeatureLevel);
	Dx11Renderer(D3D_DRIVER_TYPE driverType,
		HMODULE softwareModule, UINT flags, D3D_FEATURE_LEVEL minFeatureLevel);

	// Constructors for graphics (automatically get computing capabilities).
	// The first constructor uses the default adapter and tries for DX11.0
	// hardware acceleration (the maximum feature level) without debugging
	// support; it is equivalent to the second constructor call:
	Dx11Renderer(HWND handle, UINT width, UINT height, D3D_FEATURE_LEVEL minFeatureLevel);
	Dx11Renderer(HWND handle, UINT width, UINT height, D3D_DRIVER_TYPE driverType, 
		HMODULE softwareModule, UINT flags, D3D_FEATURE_LEVEL minFeatureLevel);

	// Overrides from Renderer.

	// Viewport management.  The measurements are in window coordinates.  The
	// origin of the window is (x,y), the window width is w, and the window
	// height is h.  The depth range for the view volume is [zmin,zmax].  The
	// DX11 viewport is left-handed with origin the upper-left corner of the
	// window, the x-axis is directed rightward, the y-axis is directed
	// downward, and the depth range is a subset of [0,1].
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
	virtual void DisplayColorBuffer(unsigned int syncInterval) override;

	// Support for drawing.
	uint64_t DrawPrimitive(VertexBuffer const* vbuffer, IndexBuffer const* ibuffer);

	// Support for enabling and disabling resources used by shaders.
	bool EnableShaders(eastl::shared_ptr<VisualEffect> const& effect,
		DX11VertexShader*& dxVShader, DX11GeometryShader*& dxGShader, 
		DX11PixelShader*& dxPShader);
	void DisableShaders(eastl::shared_ptr<VisualEffect> const& effect,
		DX11VertexShader* dxVShader, DX11GeometryShader* dxGShader, 
		DX11PixelShader* dxPShader);
	void Enable(Shader const* shader, DX11Shader* dxShader);
	void Disable(Shader const* shader, DX11Shader* dxShader);
	void EnableCBuffers(Shader const* shader, DX11Shader* dxShader);
	void DisableCBuffers(Shader const* shader, DX11Shader* dxShader);
	void EnableTBuffers(Shader const* shader, DX11Shader* dxShader);
	void DisableTBuffers(Shader const* shader, DX11Shader* dxShader);
	void EnableSBuffers(Shader const* shader, DX11Shader* dxShader);
	void DisableSBuffers(Shader const* shader, DX11Shader* dxShader);
	void EnableRBuffers(Shader const* shader, DX11Shader* dxShader);
	void DisableRBuffers(Shader const* shader, DX11Shader* dxShader);
	void EnableTextures(Shader const* shader, DX11Shader* dxShader);
	void DisableTextures(Shader const* shader, DX11Shader* dxShader);
	void EnableTextureArrays(Shader const* shader, DX11Shader* dxShader);
	void DisableTextureArrays(Shader const* shader, DX11Shader* dxShader);
	void EnableSamplers(Shader const* shader, DX11Shader* dxShader);
	void DisableSamplers(Shader const* shader, DX11Shader* dxShader);

	// Global drawing state.
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
	virtual bool Update(eastl::shared_ptr<TextureSingle> const& texture, 
		unsigned int level) override;
	virtual bool Update(eastl::shared_ptr<TextureArray> const& textureArray) override;
	virtual bool Update(eastl::shared_ptr<TextureArray> const& textureArray, 
		unsigned int item, unsigned int level) override;

protected:
	// Support for drawing.  If occlusion queries are enabled, the return
	// value is the number of samples that passed the depth and stencil
	// tests, effectively the number of pixels drawn.  If occlusion queries
	// are disabled, the function returns 0.
	virtual uint64_t DrawPrimitive(
		eastl::shared_ptr<VertexBuffer> const& vbuffer,
		eastl::shared_ptr<IndexBuffer> const& ibuffer,
		eastl::shared_ptr<VisualEffect> const& effect) override;

private:
	// Helpers for construction and destruction.
	void Initialize(D3D_DRIVER_TYPE driverType,
		HMODULE softwareModule, UINT flags, D3D_FEATURE_LEVEL minFeatureLevel);

	bool CreateDevice();
	bool CreateSwapChain(HWND handle, UINT witdth, UINT height);
	bool CreateBackBuffer(UINT witdth, UINT height);
	void CreateDefaultObjects();
	void DestroyDefaultObjects();
	bool DestroyDevice();
	bool DestroySwapChain();
	bool DestroyBackBuffer();

	// Inputs to the constructors.
	D3D_DRIVER_TYPE mDriverType;
	HMODULE mSoftwareModule;
	UINT mFlags;
	D3D_FEATURE_LEVEL mMinFeatureLevel;

	// Objects created by the constructors.
	ID3D11Device* mDevice;
	ID3D11DeviceContext* mDeviceContext;
	D3D_FEATURE_LEVEL mFeatureLevel;

	UINT mFrameRate;
	bool mIsFullScreen;
	bool mDepthStencilBufferEnabled;
	bool mMultiSamplingEnabled;
	UINT mMultiSamplingCount;
	UINT mMultiSamplingQualityLevels;

	// Objects created by the constructors for graphics engines.
	IDXGISwapChain* mSwapChain;
	ID3D11Texture2D* mColorBuffer;
	ID3D11RenderTargetView* mColorView;
	ID3D11Texture2D* mDepthStencilBuffer;
	ID3D11DepthStencilView* mDepthStencilView;
	D3D11_VIEWPORT mViewport;

	// Support for draw target enabling and disabling.
	unsigned int mNumActiveRTs;
	eastl::array<ID3D11RenderTargetView*, D3D11_SIMULTANEOUS_RENDER_TARGET_COUNT> mActiveRT;
	ID3D11DepthStencilView* mActiveDS;
	D3D11_VIEWPORT mSaveViewport;
	eastl::array<ID3D11RenderTargetView*, D3D11_SIMULTANEOUS_RENDER_TARGET_COUNT> mSaveRT;
	ID3D11DepthStencilView* mSaveDS;
};

#endif

