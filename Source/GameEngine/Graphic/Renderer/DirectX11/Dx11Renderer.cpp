// Geometric Tools, LLC
// Copyright (c) 1998-2014
// Distributed under the Boost Software License, Version 1.0.
// http://www.boost.org/LICENSE_1_0.txt
// http://www.geometrictools.com/License/Boost/LICENSE_1_0.txt
//
// File Version: 5.0.2 (2011/08/13)

#include "Graphic/Graphic.h"

#include "Dx11Renderer.h"

#include "Resource/Buffer/DX11ConstantBuffer.h"
#include "Resource/Buffer/DX11IndexBuffer.h"
#include "Resource/Buffer/DX11RawBuffer.h"
#include "Resource/Buffer/DX11StructuredBuffer.h"
#include "Resource/Buffer/DX11TextureBuffer.h"
#include "Resource/Buffer/DX11VertexBuffer.h"

#include "Resource/Texture/DX11Texture.h"
#include "Resource/Texture/DX11Texture1.h"
#include "Resource/Texture/DX11Texture1Array.h"
#include "Resource/Texture/DX11Texture2.h"
#include "Resource/Texture/DX11Texture2Array.h"
#include "Resource/Texture/DX11Texture3.h"
#include "Resource/Texture/DX11TextureCube.h"
#include "Resource/Texture/DX11TextureCubeArray.h"
#include "Resource/Texture/DX11TextureSingle.h"
#include "Resource/Texture/DX11DrawTarget.h"
#include "Resource/Texture/DX11TextureDS.h"
#include "Resource/Texture/DX11TextureRT.h"

#include "InputLayout/DX11InputLayout.h"
#include "InputLayout/DX11InputLayoutManager.h"

#include "Shader/DX11ComputeShader.h"
#include "Shader/DX11GeometryShader.h"
#include "Shader/DX11VertexShader.h"
#include "Shader/DX11PixelShader.h"

#include "State/DX11BlendState.h"
#include "State/DX11DepthStencilState.h"
#include "State/DX11RasterizerState.h"
#include "State/DX11SamplerState.h"

#include "HLSL/HLSLProgramFactory.h"
#include "HLSL/HLSLComputeProgram.h"

/*
	Direct3D initialization steps:
	1.	Create Direct3D device and device context interfaces
	2.	Check for multisampling support.
	3.	Create swap chain.
	4.	Create render target view.
	5.	Create a depth-stencil view.
	6.	Associate the render target view and depth-stencil view
		with the output-merger stage of the Direct3D pipeline.
	7.	Set the viewport
*/

//----------------------------------------------------------------------------
Dx11Renderer::Dx11Renderer()
{
	Initialize(D3D_DRIVER_TYPE_HARDWARE, nullptr, 0, D3D_FEATURE_LEVEL_11_0);
	CreateDevice();
}

Dx11Renderer::Dx11Renderer(D3D_FEATURE_LEVEL minFeatureLevel = D3D_FEATURE_LEVEL_11_0)
{
	Initialize(D3D_DRIVER_TYPE_HARDWARE, nullptr, 0, minFeatureLevel);
	CreateDevice();
}

Dx11Renderer::Dx11Renderer(D3D_DRIVER_TYPE driverType, HMODULE softwareModule, UINT flags, 
	D3D_FEATURE_LEVEL minFeatureLevel = D3D_FEATURE_LEVEL_11_0)
{
	Initialize(D3D_DRIVER_TYPE_HARDWARE, nullptr, 0, minFeatureLevel);
	CreateDevice();
}

Dx11Renderer::Dx11Renderer(HWND handle, UINT width, UINT height,
	D3D_FEATURE_LEVEL minFeatureLevel = D3D_FEATURE_LEVEL_11_0)
{
	Initialize(D3D_DRIVER_TYPE_HARDWARE, nullptr, 0, minFeatureLevel);

	if (CreateDevice() && 
		CreateSwapChain(handle, width, height) &&
		CreateBackBuffer(width, height))
	{
		CreateDefaultObjects();
	}
	else
	{
		DestroyDefaultObjects();
		DestroyBackBuffer();
		DestroySwapChain();
		DestroyDevice();
	}
}

Dx11Renderer::Dx11Renderer(HWND handle, UINT width, UINT height,
	D3D_DRIVER_TYPE driverType, HMODULE softwareModule, UINT flags, 
	D3D_FEATURE_LEVEL minFeatureLevel = D3D_FEATURE_LEVEL_11_0)
{
	Initialize(D3D_DRIVER_TYPE_HARDWARE, nullptr, 0, minFeatureLevel);

	if (CreateDevice() && 
		CreateSwapChain(handle, width, height) &&
		CreateBackBuffer(width, height))
	{
		CreateDefaultObjects();
	}
	else
	{
		DestroyDefaultObjects();
		DestroyBackBuffer();
		DestroySwapChain();
		DestroyDevice();
	}
}

//----------------------------------------------------------------------------
Dx11Renderer::~Dx11Renderer()
{
	// The render state objects (and fonts) are destroyed first so that the
	// render state objects are removed from the bridges before they are
	// cleared later in the destructor.
	DestroyDefaultObjects();

	GraphicObject::UnsubscribeForDestruction(mGOListener);
	mGOListener = nullptr;

	DrawTarget::UnsubscribeForDestruction(mDTListener);
	mDTListener = nullptr;

	if (mGraphicObjects.HasElements())
	{
		if (mWarnOnNonemptyBridges)
		{
			LogWarning("Bridge map is nonempty on destruction.");
		}

		mGraphicObjects.RemoveAll();
	}

	if (mDrawTargets.HasElements())
	{
		if (mWarnOnNonemptyBridges)
		{
			LogWarning("Draw target map nonempty on destruction.");
		}

		mDrawTargets.RemoveAll();
	}

	if (mInputLayouts->HasElements())
	{
		if (mWarnOnNonemptyBridges)
		{
			LogWarning("Input layout map nonempty on destruction.");
		}

		mInputLayouts->UnbindAll();
	}
	mInputLayouts = nullptr;

	DestroyBackBuffer();
	DestroySwapChain();
	DestroyDevice();
}

//----------------------------------------------------------------------------
// Private interface specific to DX11.
//----------------------------------------------------------------------------
void Dx11Renderer::Initialize(D3D_DRIVER_TYPE driverType,
	HMODULE softwareModule, UINT flags, D3D_FEATURE_LEVEL minFeatureLevel)
{
	// Initialization of DX11Engine members.
	mDriverType = driverType;
	mSoftwareModule = softwareModule;
	mFlags = flags;
	#if defined(_DEBUG)
		// If the project is in a debug build, enable the debug layer.
		mFlags |= D3D11_CREATE_DEVICE_DEBUG;
	#endif
	mMinFeatureLevel = minFeatureLevel;
	mDevice = nullptr;
	mDeviceContext = nullptr;
	mFeatureLevel = D3D_FEATURE_LEVEL_9_1;

	mFrameRate = 60;
	mIsFullScreen = false;
	mDepthStencilBufferEnabled = false;
	mMultiSamplingEnabled = false;
	mMultiSamplingCount = 4;
	mMultiSamplingQualityLevels = 0;

	mSwapChain = nullptr;
	mColorBuffer = nullptr;
	mColorView = nullptr;
	mDepthStencilBuffer = nullptr;
	mDepthStencilView = nullptr;
	mViewport.TopLeftX = 0.0f;
	mViewport.TopLeftY = 0.0f;
	mViewport.Width = 0.0f;
	mViewport.Height = 0.0f;
	mViewport.MinDepth = 0.0f;
	mViewport.MaxDepth = 0.0f;

	mNumActiveRTs = 0;
	mActiveRT.fill(nullptr);
	mActiveDS = nullptr;
	mSaveViewport = mViewport;
	mSaveRT.fill(nullptr);
	mSaveDS = nullptr;

	// Initialization of GraphicsEngine members that depend on DX11.
	mInputLayouts = eastl::make_unique<DX11InputLayoutManager>();

	mCreateGraphicObject =
	{
		nullptr,    // GE_GRAPHICS_OBJECT (abstract base)
		nullptr,    // GE_RESOURCE (abstract base)
		nullptr,    // GE_BUFFER (abstract base)
		&DX11ConstantBuffer::Create,
		&DX11TextureBuffer::Create,
		&DX11VertexBuffer::Create,
		&DX11IndexBuffer::Create,
		&DX11StructuredBuffer::Create,
		nullptr,  // TODO:  Implement TypedBuffer
		&DX11RawBuffer::Create,
		nullptr,	// &DX11IndirectArgumentsBuffer::Create,
		nullptr,    // GE_TEXTURE (abstract base)
		nullptr,    // GE_TEXTURE_SINGLE (abstract base)
		&DX11Texture1::Create,
		&DX11Texture2::Create,
		&DX11TextureRT::Create,
		&DX11TextureDS::Create,
		&DX11Texture3::Create,
		nullptr,  // GE_TEXTURE_ARRAY (abstract base)
		&DX11Texture1Array::Create,
		&DX11Texture2Array::Create,
		&DX11TextureCube::Create,
		&DX11TextureCubeArray::Create,
		nullptr,    // GE_SHADER (abstract base)
		&DX11VertexShader::Create,
		&DX11GeometryShader::Create,
		&DX11PixelShader::Create,
		&DX11ComputeShader::Create,
		nullptr,    // GE_DRAWING_STATE (abstract base)
		&DX11SamplerState::Create,
		&DX11BlendState::Create,
		&DX11DepthStencilState::Create,
		&DX11RasterizerState::Create
	};

	mCreateDrawTarget = &DX11DrawTarget::Create;
}

bool Dx11Renderer::CreateDevice()
{
	// Determine the subarray for creating the device.
	UINT const maxFeatureLevels = 7;
	D3D_FEATURE_LEVEL const featureLevels[maxFeatureLevels] =
	{
		D3D_FEATURE_LEVEL_11_1,
		D3D_FEATURE_LEVEL_11_0,
		D3D_FEATURE_LEVEL_10_1,
		D3D_FEATURE_LEVEL_10_0,
		D3D_FEATURE_LEVEL_9_3,
		D3D_FEATURE_LEVEL_9_2,
		D3D_FEATURE_LEVEL_9_1
	};

	UINT numFeatureLevels = 0;
	for (UINT i = 0; i < maxFeatureLevels; ++i)
	{
		if (mMinFeatureLevel == featureLevels[i])
		{
			numFeatureLevels = i + 1;
			break;
		}
	}
	if (numFeatureLevels == 0)
	{
		LogError("Unsupported minimum feature level.");
		return false;
	}

	for (UINT i = 0; i < numFeatureLevels; ++i)
	{
		HRESULT hr = D3D11CreateDevice(NULL, mDriverType, mSoftwareModule, mFlags, 
			&featureLevels[i], 1, D3D11_SDK_VERSION, &mDevice, &mFeatureLevel, &mDeviceContext);
		if (SUCCEEDED(hr))
		{
			// Request multisampling.
			// Multisample Anti-Aliasing (MSAA) is a technique to improve image quelity. Aliasing 
			// refers to the jagged look of a line or triangle edge when presented on a computer 
			// monitor. It occurs because monitors use discrete points of light (pixels) to display 
			// virtual objects. As smalls as pixel might be, it is not infinitesimal. Anti-aliasing 
			// refers to techniques to remove the described effect. Reducing the size of each pixel 
			// can ameliorate aliasing, so increasing monitor's resolution can help. But even at 
			// maximum resolution, aliasing artifacts might still be unacceptable. Supersampling is 
			// a technique that reduces the effective size of a pixel by increasing the resolution 
			// of the render target. Thus, multiple subpixels are sampled and averaged together (in 
			// a process known as downsampling) to produce the final color of each pixel displayed.

			// A render target four times (4x) larger than the display resolution is commonly used 
			// for supersampling, although higher resolutions can be used at the cost of memory and
			// performance. With supersampling, the pixel shader is executed for each subpixel. Thus,
			// to produce a render target 4x larger than the display, the pixel shader must run 4x as
			// often. Multisampling is an optimization of supersampling that executes the pixel shader
			// just once per display resolution pixel (at the pixel's center) and uses that color for 
			// each of the subpixels.
			// Check for multisampling support. 
			mDevice->CheckMultisampleQualityLevels(
				DXGI_FORMAT_R8G8B8A8_UNORM, mMultiSamplingCount, &mMultiSamplingQualityLevels);
			if (mMultiSamplingQualityLevels == 0)
			{
				LogError("Unsupported multi-sampling quality");
				return false;
			}

			mGraphicObjectCreator = mDevice;
			if (i == 0 || i == 1)
			{
				HLSLProgramFactory::defaultVersion = "5_0";
			}
			else if (i == 2 || i == 3)
			{
				HLSLProgramFactory::defaultVersion = "4_0";
			}
			else if (i == 4)
			{
				HLSLProgramFactory::defaultVersion = "4_0_level_9_3";
			}
			else
			{
				HLSLProgramFactory::defaultVersion = "4_0_level_9_1";
			}

			return true;
		}
	}

	LogError("Failed to create device");
	return false;
}

/*
	Swap chain is a set of buffers used to display frames to the monitor.
	Flipswap effect might include multiple back buffers and is essentially a circular queue
	that includes the front buffer. Within this queue, the back buffers are always numbered
	sequentially from 0 to (n - 1), where n is the number of back buffers, so that 0 denotes
	the least recently presented buffer.

	Copyswap effect can be specified only for a swap chain that comprises a single back
	buffer. Whether the swap chain is windowed or full-screen, the runtime guarantees the
	semantics implied by a copy-based Present operation; specifically, that the operation
	leaves the content of the back buffer unchanged, instead of replacing it with the content
	of the front buffer as a flip-based Present operation would.

	When a swap chain is created with a swap effect of Flip or Copy, the runtime guarantees
	that a Present operation will not affect the content of any of the back buffers. However,
	meeting this guarantee can involve substantial video memory or processing overheads,
	especially when implementing flip semantics for a windowed swap chain or copy semantics
	for a full-screen swap chain.An application can use the Discard swap effect to avoid
	these overheads and to enable the display driver to choose the most efficient presentation
	technique for the swap chain.
*/
bool Dx11Renderer::CreateSwapChain(HWND handle, UINT width, UINT height)
{
	mScreenSize = Vector2<unsigned int>{ width, height };

	struct DXGIInterfaces
	{
		DXGIInterfaces()
			:
			device(nullptr),
			adapter(nullptr),
			factory(nullptr)
		{
		}

		~DXGIInterfaces()
		{
			SafeRelease(factory);
			SafeRelease(adapter);
			SafeRelease(device);
		}

		IDXGIDevice* device;
		IDXGIAdapter* adapter;
		IDXGIFactory1* factory;
	};

	DXGIInterfaces dxgi;
	HRESULT hr = mDevice->QueryInterface(__uuidof(IDXGIDevice), reinterpret_cast<void**>(&dxgi.device));
	if (FAILED(hr))
	{
		LogError("Failed to get DXGI device interface, hr = " + GetErrorMessage(hr));
	}

	hr = dxgi.device->GetAdapter(&dxgi.adapter);
	if (FAILED(hr))
	{
		LogError("Failed to get DXGI adapter, hr = " + GetErrorMessage(hr));
	}

	hr = dxgi.adapter->GetParent(__uuidof(IDXGIFactory1), reinterpret_cast<void**>(&dxgi.factory));
	if (FAILED(hr))
	{
		LogError("Failed to get DXGI factory, hr = " + GetErrorMessage(hr));
	}

	DXGI_SWAP_CHAIN_DESC desc;
	desc.BufferDesc.Width = width;
	desc.BufferDesc.Height = height;
	desc.BufferDesc.RefreshRate.Numerator = 0;
	desc.BufferDesc.RefreshRate.Denominator = 1;
	desc.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	desc.BufferDesc.ScanlineOrdering = DXGI_MODE_SCANLINE_ORDER_UNSPECIFIED;
	desc.BufferDesc.Scaling = DXGI_MODE_SCALING_UNSPECIFIED;

	if (mMultiSamplingEnabled)
	{
		desc.SampleDesc.Count = mMultiSamplingCount;
		desc.SampleDesc.Quality = mMultiSamplingQualityLevels - 1;
	}
	else
	{
		desc.SampleDesc.Count = 1;
		desc.SampleDesc.Quality = 0;
	}

	desc.BufferUsage = DXGI_USAGE_BACK_BUFFER | DXGI_USAGE_RENDER_TARGET_OUTPUT;
	desc.BufferCount = 2;
	desc.OutputWindow = handle;
	desc.Windowed = TRUE;
	desc.SwapEffect = DXGI_SWAP_EFFECT_DISCARD;
	desc.Flags = 0;

	hr = dxgi.factory->CreateSwapChain(dxgi.device, &desc, &mSwapChain);
	if (FAILED(hr))
	{
		LogError("Failed to create swap chain, hr = " + GetErrorMessage(hr));
	}

	return true;
}

/*
	Double buffering refers to a swap chain containing two buffers, a front buffer and a 
	back buffer. The front buffer is what's actually displayed while the back buffer is 
	being filled with new data. When the application presents the newly completed back 
	buffer, the two are swapped and the process starts over. The swap chain establishes 
	a back buffer, a texture to render to. The intent is to bind that texture to the 
	ouput-merger stage of the Direct3D pipeline
*/
bool Dx11Renderer::CreateBackBuffer(UINT xSize, UINT ySize)
{
	struct BackBuffer
	{
		BackBuffer()
		{
			SetToNull();
		}

		~BackBuffer()
		{
			SafeRelease(depthStencilView);
			SafeRelease(depthStencilBuffer);
			SafeRelease(colorView);
			SafeRelease(colorBuffer);
		}

		void SetToNull()
		{
			colorBuffer = nullptr;
			colorView = nullptr;
			depthStencilBuffer = nullptr;
			depthStencilView = nullptr;
		}

		ID3D11Texture2D* colorBuffer;
		ID3D11RenderTargetView* colorView;
		ID3D11Texture2D* depthStencilBuffer;
		ID3D11DepthStencilView* depthStencilView;
	};

	BackBuffer bb;

	// Create the color buffer and its view to be bound to the pipeline
	HRESULT hr = mSwapChain->GetBuffer(0, __uuidof(ID3D11Texture2D), reinterpret_cast<void**>(&bb.colorBuffer));
	if (FAILED(hr))
	{
		LogError("Failed to get color buffer, hr = " + GetErrorMessage(hr));
	}

	hr = mDevice->CreateRenderTargetView(bb.colorBuffer, nullptr, &bb.colorView);
	if (FAILED(hr))
	{
		LogError("Failed to create color view, hr = " + GetErrorMessage(hr));
	}

	/*
		Objects in a scene can overlap. An object nearer the camera can partially or completely
		occulde an object that is farther away. When a pixel is written to the render target,
		its depth (its distance from the camera) can be written to a depth buffer. The depth
		buffer is just a 2D texture, but instead of storing colors, it stores depths. The
		output-merger stage can use these depths to determine whether new pixels should overwrite
		existing colors already present in the render target (depth testing process). Stencil
		testing uses a mask to determine which pixels to update. The depth and stencil buffers
		and views are attached.
	*/
	// Create the depth-stencil buffer and its view.
	D3D11_TEXTURE2D_DESC desc;
	desc.Width = xSize;
	desc.Height = ySize;
	desc.MipLevels = 1;
	desc.ArraySize = 1;
	desc.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;

	if (mMultiSamplingEnabled)
	{
		desc.SampleDesc.Count = mMultiSamplingCount;
		desc.SampleDesc.Quality = mMultiSamplingQualityLevels - 1;
	}
	else
	{
		desc.SampleDesc.Count = 1;
		desc.SampleDesc.Quality = 0;
	}

	desc.Usage = D3D11_USAGE_DEFAULT;
	desc.BindFlags = D3D11_BIND_DEPTH_STENCIL;
	desc.CPUAccessFlags = D3D11_CPU_ACCESS_NONE;
	desc.MiscFlags = D3D11_RESOURCE_MISC_NONE;

	// Creating a swap chain, it implicitly creates the associated 2D texture for the back buffer, but a
	//	depth buffer was not created. It is built a 2D texture for the depth buffer with the described info
	hr = mDevice->CreateTexture2D(&desc, nullptr, &bb.depthStencilBuffer);
	if (FAILED(hr))
	{
		LogError("Failed to create depth-stencil buffer, hr = " + GetErrorMessage(hr));
	}

	hr = mDevice->CreateDepthStencilView(bb.depthStencilBuffer, nullptr,
		&bb.depthStencilView);
	if (FAILED(hr))
	{
		LogError("Failed to create depth-stencil view, hr = " + GetErrorMessage(hr));
	}

	// The back buffer has been created successfully.  Transfer the
	// resources to 'this' members.
	mColorBuffer = bb.colorBuffer;
	mColorView = bb.colorView;
	mDepthStencilBuffer = bb.depthStencilBuffer;
	mDepthStencilView = bb.depthStencilView;
	bb.SetToNull();

	//	A viewport is a rectangular area that can encompass the entire back buffer, or some portion of it; 
	//	scenes are rendered to this area. Viewports are commonly used for split-screen, where different 
	//	views of the game world are displayed to separate areas of the screen
	mViewport.Width = static_cast<float>(xSize);
	mViewport.Height = static_cast<float>(ySize);
	mViewport.TopLeftX = 0.0f;
	mViewport.TopLeftY = 0.0f;
	mViewport.MinDepth = 0.0f;
	mViewport.MaxDepth = 1.0f;
	mDeviceContext->RSSetViewports(1, &mViewport);

	//	Associating the views to the ouput-merger stage. With the render target view and the depth-stencil
	// view created, it is now possible to bind them to the output-merger stage of the Direct3D pipeline
	mNumActiveRTs = 1;
	mActiveRT[0] = mColorView;
	mActiveDS = mDepthStencilView;
	mDeviceContext->OMSetRenderTargets(1, mActiveRT.data(), mActiveDS);
	return true;
}

void Dx11Renderer::CreateDefaultObjects()
{
	CreateDefaultGlobalState();
}

void Dx11Renderer::DestroyDefaultObjects()
{
	if (mDefaultFont)
	{
		mDefaultFont = nullptr;
		mActiveFont = nullptr;
	}

	DestroyDefaultGlobalState();
}

bool Dx11Renderer::DestroyDevice()
{
	return FinalRelease(mDeviceContext) == 0 && FinalRelease(mDevice) == 0;
}

bool Dx11Renderer::DestroySwapChain()
{
	bool successful = true;
	ULONG refs;

	if (mSwapChain)
	{
		refs = mSwapChain->Release();
		if (refs > 0)
		{
			LogError("Swap chain not released.");
			successful = false;
		}
		mSwapChain = nullptr;
	}

	return successful;
}

bool Dx11Renderer::DestroyBackBuffer()
{
	if (mDeviceContext)
	{
		ID3D11RenderTargetView* rtView = nullptr;
		ID3D11DepthStencilView* dsView = nullptr;
		mDeviceContext->OMSetRenderTargets(1, &rtView, dsView);
	}

	mActiveRT[0] = nullptr;
	mActiveDS = nullptr;

	return FinalRelease(mColorView) == 0
		&& FinalRelease(mColorBuffer) == 0
		&& FinalRelease(mDepthStencilView) == 0
		&& FinalRelease(mDepthStencilBuffer) == 0;
}

uint64_t Dx11Renderer::DrawPrimitive(VertexBuffer const* vbuffer, IndexBuffer const* ibuffer)
{
	UINT numActiveVertices = vbuffer->GetNumActiveElements();
	UINT vertexOffset = vbuffer->GetOffset();

	UINT numActiveIndices = ibuffer->GetNumActiveIndices();
	UINT firstIndex = ibuffer->GetFirstIndex();
	IPType type = ibuffer->GetPrimitiveType();


	//	First step for rendering is to tell the input-assembler stage what topology to use
	//	for the incoming vertices
	switch (type)
	{
		case IPType::IP_POLYPOINT:
			mDeviceContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_POINTLIST);
			break;
		case IPType::IP_POLYSEGMENT_DISJOINT:
			mDeviceContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_LINELIST);
			break;
		case IPType::IP_POLYSEGMENT_CONTIGUOUS:
			mDeviceContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_LINESTRIP);
			break;
		case IPType::IP_TRIMESH:
			mDeviceContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
			break;
		case IPType::IP_TRISTRIP:
			mDeviceContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP);
			break;
		case IPType::IP_POLYSEGMENT_DISJOINT_ADJ:
			mDeviceContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_LINELIST_ADJ);
			break;
		case IPType::IP_POLYSEGMENT_CONTIGUOUS_ADJ:
			mDeviceContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_LINESTRIP_ADJ);
			break;
		case IPType::IP_TRIMESH_ADJ:
			mDeviceContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST_ADJ);
			break;
		case IPType::IP_TRISTRIP_ADJ:
			mDeviceContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP_ADJ);
			break;
		default:
			LogError("Unknown primitive topology = " + eastl::to_string(type));
			return 0;
	}

	uint64_t numPixelsDrawn = 0;
	if (ibuffer->IsIndexed())
	{
		if (numActiveIndices > 0)
		{
			mDeviceContext->DrawIndexed(numActiveIndices, firstIndex, vertexOffset);
		}
	}
	else
	{
		if (numActiveVertices > 0)
		{
			mDeviceContext->Draw(numActiveVertices, vertexOffset);
		}
	}

	return numPixelsDrawn;
}

bool Dx11Renderer::EnableShaders(eastl::shared_ptr<VisualEffect> const& effect,
	DX11VertexShader*& dxVShader, DX11GeometryShader*& dxGShader, DX11PixelShader*& dxPShader)
{
	dxVShader = nullptr;
	dxGShader = nullptr;
	dxPShader = nullptr;

	// Get the active vertex shader.
	if (!effect->GetVertexShader())
	{
		LogError("Effect does not have a vertex shader.");
		return false;
	}
	dxVShader = static_cast<DX11VertexShader*>(Bind(effect->GetVertexShader()));

	// Get the active geometry shader (if any).
	if (effect->GetGeometryShader())
	{
		dxGShader = static_cast<DX11GeometryShader*>(Bind(effect->GetGeometryShader()));
	}

	// Get the active pixel shader.
	if (!effect->GetPixelShader())
	{
		LogError("Effect does not have a pixel shader.");
		return false;
	}
	dxPShader = static_cast<DX11PixelShader*>(Bind(effect->GetPixelShader()));

	// Enable the shaders and resources.
	Enable(effect->GetVertexShader().get(), dxVShader);
	Enable(effect->GetPixelShader().get(), dxPShader);
	if (dxGShader)
	{
		Enable(effect->GetGeometryShader().get(), dxGShader);
	}

	return true;
}

void Dx11Renderer::DisableShaders(eastl::shared_ptr<VisualEffect> const& effect,
	DX11VertexShader* dxVShader, DX11GeometryShader* dxGShader, DX11PixelShader* dxPShader)
{
	if (dxGShader)
	{
		Disable(effect->GetGeometryShader().get(), dxGShader);
	}
	Disable(effect->GetPixelShader().get(), dxPShader);
	Disable(effect->GetVertexShader().get(), dxVShader);
}

void Dx11Renderer::Enable(Shader const* shader, DX11Shader* dxShader)
{
	dxShader->Enable(mDeviceContext);
	EnableCBuffers(shader, dxShader);
	EnableTBuffers(shader, dxShader);
	EnableSBuffers(shader, dxShader);
	EnableRBuffers(shader, dxShader);
	EnableTextures(shader, dxShader);
	EnableTextureArrays(shader, dxShader);
	EnableSamplers(shader, dxShader);
}

void Dx11Renderer::Disable(Shader const* shader, DX11Shader* dxShader)
{
	DisableSamplers(shader, dxShader);
	DisableTextureArrays(shader, dxShader);
	DisableTextures(shader, dxShader);
	DisableRBuffers(shader, dxShader);
	DisableSBuffers(shader, dxShader);
	DisableTBuffers(shader, dxShader);
	DisableCBuffers(shader, dxShader);
	dxShader->Disable(mDeviceContext);
}

void Dx11Renderer::EnableCBuffers(Shader const* shader, DX11Shader* dxShader)
{
	int const index = ConstantBuffer::mShaderDataLookup;
	for (auto const& cb : shader->GetData(index))
	{
		if (cb.object)
		{
			DX11ConstantBuffer* dxCB = static_cast<DX11ConstantBuffer*>(Bind(cb.object));
			if (dxCB)
			{
				dxShader->EnableCBuffer(mDeviceContext, cb.bindPoint, dxCB->GetDXBuffer());
			}
			else
			{
				LogError("Failed to bind constant buffer.");
			}
		}
		else
		{
			LogError(cb.name + " is null constant buffer.");
		}
	}
}

void Dx11Renderer::DisableCBuffers(Shader const* shader, DX11Shader* dxShader)
{
	int const index = ConstantBuffer::mShaderDataLookup;
	for (auto const& cb : shader->GetData(index))
	{
		dxShader->DisableCBuffer(mDeviceContext, cb.bindPoint);
	}
}

void Dx11Renderer::EnableTBuffers(Shader const* shader, DX11Shader* dxShader)
{
	int const index = TextureBuffer::mShaderDataLookup;
	for (auto const& tb : shader->GetData(index))
	{
		if (tb.object)
		{
			DX11TextureBuffer* dxTB = static_cast<DX11TextureBuffer*>(Bind(tb.object));
			if (dxTB)
			{
				dxShader->EnableSRView(mDeviceContext, tb.bindPoint, dxTB->GetSRView());
			}
			else
			{
				LogError("Failed to bind texture buffer.");
			}
		}
		else
		{
			LogError(tb.name + " is null texture buffer.");
		}
	}
}

void Dx11Renderer::DisableTBuffers(Shader const* shader, DX11Shader* dxShader)
{
	int const index = TextureBuffer::mShaderDataLookup;
	for (auto const& tb : shader->GetData(index))
	{
		dxShader->DisableSRView(mDeviceContext, tb.bindPoint);
	}
}

void Dx11Renderer::EnableSBuffers(Shader const* shader, DX11Shader* dxShader)
{
	int const index = StructuredBuffer::mShaderDataLookup;
	for (auto const& sb : shader->GetData(index))
	{
		if (sb.object)
		{
			DX11StructuredBuffer* dxSB = static_cast<DX11StructuredBuffer*>(Bind(sb.object));
			if (dxSB)
			{
				if (sb.isGpuWritable)
				{
					StructuredBuffer* gtSB = static_cast<StructuredBuffer*>(sb.object.get());

					unsigned int numActive = (gtSB->GetKeepInternalCount() ?
						0xFFFFFFFFu : gtSB->GetNumActiveElements());
					dxShader->EnableUAView(mDeviceContext, sb.bindPoint, dxSB->GetUAView(), numActive);
				}
				else
				{
					dxShader->EnableSRView(mDeviceContext, sb.bindPoint, dxSB->GetSRView());
				}
			}
			else
			{
				LogError("Failed to bind structured buffer.");
			}
		}
		else
		{
			LogError(sb.name + " is null structured buffer.");
		}
	}
}

void Dx11Renderer::DisableSBuffers(Shader const* shader, DX11Shader* dxShader)
{
	int const index = StructuredBuffer::mShaderDataLookup;
	for (auto const& sb : shader->GetData(index))
	{
		if (sb.isGpuWritable)
		{
			dxShader->DisableUAView(mDeviceContext, sb.bindPoint);
		}
		else
		{
			dxShader->DisableSRView(mDeviceContext, sb.bindPoint);
		}
	}
}

void Dx11Renderer::EnableRBuffers(Shader const* shader, DX11Shader* dxShader)
{
	int const index = RawBuffer::mShaderDataLookup;
	for (auto const& rb : shader->GetData(index))
	{
		if (rb.object)
		{
			DX11RawBuffer* dxRB = static_cast<DX11RawBuffer*>(Bind(rb.object));
			if (dxRB)
			{
				if (rb.isGpuWritable)
				{
					dxShader->EnableUAView(mDeviceContext, rb.bindPoint, dxRB->GetUAView(), 0xFFFFFFFFu);
				}
				else
				{
					dxShader->EnableSRView(mDeviceContext, rb.bindPoint, dxRB->GetSRView());
				}
			}
			else
			{
				LogError("Failed to bind byte-address buffer.");
			}
		}
		else
		{
			LogError(rb.name + " is null byte-address buffer.");
		}
	}
}

void Dx11Renderer::DisableRBuffers(Shader const* shader, DX11Shader* dxShader)
{
	int const index = RawBuffer::mShaderDataLookup;
	for (auto const& rb : shader->GetData(index))
	{
		if (rb.isGpuWritable)
		{
			dxShader->DisableUAView(mDeviceContext, rb.bindPoint);
		}
		else
		{
			dxShader->DisableSRView(mDeviceContext, rb.bindPoint);
		}
	}
}

void Dx11Renderer::EnableTextures(Shader const* shader, DX11Shader* dxShader)
{
	int const index = TextureSingle::mShaderDataLookup;
	for (auto const& tx : shader->GetData(index))
	{
		if (tx.object)
		{
			DX11TextureSingle* dxTX = static_cast<DX11TextureSingle*>(Bind(tx.object));
			if (dxTX)
			{
				if (tx.isGpuWritable)
				{
					dxShader->EnableUAView(mDeviceContext, tx.bindPoint, dxTX->GetUAView(), 0xFFFFFFFFu);
				}
				else
				{
					dxShader->EnableSRView(mDeviceContext, tx.bindPoint, dxTX->GetSRView());
				}
			}
			else
			{
				LogError("Failed to bind texture.");
			}
		}
		else
		{
			LogError(tx.name + " is null texture.");
		}
	}
}

void Dx11Renderer::DisableTextures(Shader const* shader, DX11Shader* dxShader)
{
	int const index = TextureSingle::mShaderDataLookup;
	for (auto const& tx : shader->GetData(index))
	{
		if (tx.isGpuWritable)
		{
			dxShader->DisableUAView(mDeviceContext, tx.bindPoint);
		}
		else
		{
			dxShader->DisableSRView(mDeviceContext, tx.bindPoint);
		}
	}
}

void Dx11Renderer::EnableTextureArrays(Shader const* shader, DX11Shader* dxShader)
{
	int const index = TextureArray::mShaderDataLookup;
	for (auto const& ta : shader->GetData(index))
	{
		if (ta.object)
		{
			DX11TextureArray* dxTA = static_cast<DX11TextureArray*>(Bind(ta.object));
			if (dxTA)
			{
				if (ta.isGpuWritable)
				{
					dxShader->EnableUAView(mDeviceContext, ta.bindPoint, dxTA->GetUAView(), 0xFFFFFFFFu);
				}
				else
				{
					dxShader->EnableSRView(mDeviceContext, ta.bindPoint, dxTA->GetSRView());
				}
			}
			else
			{
				LogError("Failed to bind texture array.");
			}
		}
		else
		{
			LogError(ta.name + " is null texture array.");
		}
	}
}

void Dx11Renderer::DisableTextureArrays(Shader const* shader,
	DX11Shader* dxShader)
{
	int const index = TextureArray::mShaderDataLookup;
	for (auto const& ta : shader->GetData(index))
	{
		if (ta.isGpuWritable)
		{
			dxShader->DisableUAView(mDeviceContext, ta.bindPoint);
		}
		else
		{
			dxShader->DisableSRView(mDeviceContext, ta.bindPoint);
		}
	}
}

void Dx11Renderer::EnableSamplers(Shader const* shader, DX11Shader* dxShader)
{
	int const index = SamplerState::mShaderDataLookup;
	for (auto const& ss : shader->GetData(index))
	{
		if (ss.object)
		{
			DX11SamplerState* dxSS = static_cast<DX11SamplerState*>(Bind(ss.object));
			if (dxSS)
			{
				dxShader->EnableSampler(mDeviceContext, ss.bindPoint, dxSS->GetDXSamplerState());
			}
			else
			{
				LogError("Failed to bind sampler state.");
			}
		}
		else
		{
			LogError(ss.name + " is null sampler state.");
		}
	}
}

void Dx11Renderer::DisableSamplers(Shader const* shader, DX11Shader* dxShader)
{
	int const index = SamplerState::mShaderDataLookup;
	for (auto const& ss : shader->GetData(index))
	{
		dxShader->DisableSampler(mDeviceContext, ss.bindPoint);
	}
}

bool Dx11Renderer::Update(eastl::shared_ptr<Buffer> const& buffer)
{
	if (!buffer->GetData())
	{
		LogWarning("Buffer does not have system memory, creating it.");
		buffer->CreateStorage();
	}

	DX11Buffer* dxBuffer = static_cast<DX11Buffer*>(Bind(buffer));
	return dxBuffer->Update(mDeviceContext);
}

bool Dx11Renderer::Update(eastl::shared_ptr<TextureSingle> const& texture)
{
	if (!texture->GetData())
	{
		LogWarning("Texture does not have system memory, creating it.");
		texture->CreateStorage();
	}

	DX11Texture* dxTexture = static_cast<DX11Texture*>(Bind(texture));
	return dxTexture->Update(mDeviceContext);
}

bool Dx11Renderer::Update(eastl::shared_ptr<TextureSingle> const& texture, unsigned int level)
{
	if (!texture->GetData())
	{
		LogWarning("Texture does not have system memory, creating it.");
		texture->CreateStorage();
	}

	DX11Texture* dxTexture = static_cast<DX11Texture*>(Bind(texture));
	unsigned int sri = texture->GetIndex(0, level);
	return dxTexture->Update(mDeviceContext, sri);
}

bool Dx11Renderer::Update(eastl::shared_ptr<TextureArray> const& textureArray)
{
	if (!textureArray->GetData())
	{
		LogWarning("Texture array does not have system memory, creating it.");
		textureArray->CreateStorage();
	}

	DX11TextureArray* dxTextureArray = static_cast<DX11TextureArray*>(Bind(textureArray));
	return dxTextureArray->Update(mDeviceContext);
}

bool Dx11Renderer::Update(eastl::shared_ptr<TextureArray> const& textureArray, 
	unsigned int item, unsigned int level)
{
	if (!textureArray->GetData())
	{
		LogWarning("Texture array does not have system memory, creating it.");
		textureArray->CreateStorage();
	}

	DX11TextureArray* dxTextureArray = static_cast<DX11TextureArray*>(
		Bind(textureArray));
	unsigned int sri = textureArray->GetIndex(item, level);
	return dxTextureArray->Update(mDeviceContext, sri);
}

uint64_t Dx11Renderer::DrawPrimitive(eastl::shared_ptr<VertexBuffer> const& vbuffer,
	eastl::shared_ptr<IndexBuffer> const& ibuffer, eastl::shared_ptr<VisualEffect> const& effect)
{
	uint64_t numPixelsDrawn = 0;
	DX11VertexShader* dxVShader;
	DX11GeometryShader* dxGShader;
	DX11PixelShader* dxPShader;
	if (EnableShaders(effect, dxVShader, dxGShader, dxPShader))
	{
		// Enable the vertex buffer and input layout.
		DX11VertexBuffer* dxVBuffer = nullptr;
		DX11InputLayout* dxLayout = nullptr;
		if (vbuffer->StandardUsage())
		{
			dxVBuffer = static_cast<DX11VertexBuffer*>(Bind(vbuffer));
			DX11InputLayoutManager* manager = static_cast<DX11InputLayoutManager*>(mInputLayouts.get());
			dxLayout = manager->Bind(mDevice, vbuffer.get(), effect->GetVertexShader().get());
			dxVBuffer->Enable(mDeviceContext);
			dxLayout->Enable(mDeviceContext);
		}
		else
		{
			mDeviceContext->IASetInputLayout(nullptr);
		}

		// Enable the index buffer.
		DX11IndexBuffer* dxIBuffer = nullptr;
		if (ibuffer->IsIndexed())
		{
			dxIBuffer = static_cast<DX11IndexBuffer*>(Bind(ibuffer));
			dxIBuffer->Enable(mDeviceContext);
		}

		numPixelsDrawn = DrawPrimitive(vbuffer.get(), ibuffer.get());

		// Disable the vertex buffer and input layout.
		if (vbuffer->StandardUsage())
		{
			dxVBuffer->Disable(mDeviceContext);
			dxLayout->Disable(mDeviceContext);
		}

		// Disable the index buffer.
		if (dxIBuffer)
		{
			dxIBuffer->Disable(mDeviceContext);
		}

		DisableShaders(effect, dxVShader, dxGShader, dxPShader);
	}
	return numPixelsDrawn;
}

//----------------------------------------------------------------------------
// Public overrides from Renderer
//----------------------------------------------------------------------------
void Dx11Renderer::SetViewport(int x, int y, int w, int h)
{
	UINT numViewports = 1;
	mDeviceContext->RSGetViewports(&numViewports, &mViewport);
	LogAssert(1 == numViewports, "Failed to get viewport.");

	mViewport.TopLeftX = static_cast<float>(x);
	mViewport.TopLeftY = static_cast<float>(y);
	mViewport.Width = static_cast<float>(w);
	mViewport.Height = static_cast<float>(h);
	mDeviceContext->RSSetViewports(1, &mViewport);
}

void Dx11Renderer::GetViewport(int& x, int& y, int& w, int& h) const
{
	UINT numViewports = 1;
	D3D11_VIEWPORT viewport;
	mDeviceContext->RSGetViewports(&numViewports, &viewport);
	LogAssert(1 == numViewports, "Failed to get viewport.");

	x = static_cast<unsigned int>(viewport.TopLeftX);
	y = static_cast<unsigned int>(viewport.TopLeftY);
	w = static_cast<unsigned int>(viewport.Width);
	h = static_cast<unsigned int>(viewport.Height);
}

void Dx11Renderer::SetDepthRange(float zmin, float zmax)
{
	UINT numViewports = 1;
	mDeviceContext->RSGetViewports(&numViewports, &mViewport);
	LogAssert(1 == numViewports, "Failed to get viewport.");

	mViewport.MinDepth = zmin;
	mViewport.MaxDepth = zmax;
	mDeviceContext->RSSetViewports(1, &mViewport);
}

void Dx11Renderer::GetDepthRange(float& zmin, float& zmax) const
{
	UINT numViewports = 1;
	D3D11_VIEWPORT viewport;
	mDeviceContext->RSGetViewports(&numViewports, &viewport);
	LogAssert(1 == numViewports, "Failed to get viewport.");

	zmin = viewport.MinDepth;
	zmax = viewport.MaxDepth;
}

bool Dx11Renderer::Resize(unsigned int w, unsigned int h)
{
	// Release the previous back buffer before resizing.
	if (DestroyBackBuffer())
	{
		// Attempt to resize the back buffer to the incoming width and height.
		DXGI_SWAP_CHAIN_DESC desc;
		mSwapChain->GetDesc(&desc);
		HRESULT hr = mSwapChain->ResizeBuffers(desc.BufferCount, w, h,
			DXGI_FORMAT_R8G8B8A8_UNORM, 0);
		if (SUCCEEDED(hr))
		{
			// The attempt succeeded, so create new color and depth-stencil
			// objects.
			return CreateBackBuffer(w, h);
		}

		// The attempt to resize failed, so restore the back buffer to its
		// previous width and height.
		w = desc.BufferDesc.Width;
		h = desc.BufferDesc.Height;
		hr = mSwapChain->ResizeBuffers(desc.BufferCount, w, h,
			DXGI_FORMAT_R8G8B8A8_UNORM, 0);
		if (SUCCEEDED(hr))
		{
			return CreateBackBuffer(w, h);
		}
	}
	return false;
}

/*
	Clear stage
*/
void Dx11Renderer::ClearColorBuffer()
{
	ID3D11RenderTargetView* rtViews[D3D11_SIMULTANEOUS_RENDER_TARGET_COUNT] = { nullptr };
	ID3D11DepthStencilView* dsView = nullptr;

	mDeviceContext->OMGetRenderTargets(mNumActiveRTs, rtViews, &dsView);
	SafeRelease(dsView);
	for (unsigned int i = 0; i < mNumActiveRTs; ++i)
	{
		if (rtViews[i])
		{
			mDeviceContext->ClearRenderTargetView(rtViews[i], mClearColor.data());
			rtViews[i]->Release();
		}
	}
}

void Dx11Renderer::ClearDepthBuffer()
{
	ID3D11DepthStencilView* dsView = nullptr;
	ID3D11RenderTargetView* rtView = nullptr;
	mDeviceContext->OMGetRenderTargets(1, &rtView, &dsView);
	SafeRelease(rtView);
	if (dsView)
	{
		mDeviceContext->ClearDepthStencilView(dsView, D3D11_CLEAR_DEPTH, mClearDepth, 0);
		dsView->Release();
	}
}

void Dx11Renderer::ClearStencilBuffer()
{
	ID3D11DepthStencilView* dsView = nullptr;
	ID3D11RenderTargetView* rtView = nullptr;
	mDeviceContext->OMGetRenderTargets(1, &rtView, &dsView);
	SafeRelease(rtView);
	if (dsView)
	{
		mDeviceContext->ClearDepthStencilView(dsView, D3D11_CLEAR_STENCIL, 0.0f,
			static_cast<UINT8>(mClearStencil));
		dsView->Release();
	}
}

void Dx11Renderer::ClearBuffers()
{
	/*
		Sets the entire render target to the specified color.
		Clear depth-stencil buffer
	*/
	ID3D11RenderTargetView* rtViews[D3D11_SIMULTANEOUS_RENDER_TARGET_COUNT] = { nullptr };
	ID3D11DepthStencilView* dsView = nullptr;

	mDeviceContext->OMGetRenderTargets(mNumActiveRTs, rtViews, &dsView);
	for (unsigned int i = 0; i < mNumActiveRTs; ++i)
	{
		if (rtViews[i])
		{
			mDeviceContext->ClearRenderTargetView(rtViews[i], mClearColor.data());
			rtViews[i]->Release();
		}
	}
	if (dsView)
	{
		mDeviceContext->ClearDepthStencilView(dsView,
			D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, mClearDepth,
			static_cast<UINT8>(mClearStencil));
		dsView->Release();
	}
}

void Dx11Renderer::DisplayColorBuffer(unsigned int syncInterval)
{
	// The swap must occur on the thread in which the device was created. When all game-specific
	// rendering has been completed, it is flipped the buffers in the swap chain with this call
	// The first parameter specifies how the frame is presented with respect to the monitor's 
	// vertical refresh synchronization. The second parameter is a set of bitwise OR'd DXGI_PRESENT
	// flags. A value of 0 simply represents the frame of the output without any special options.
	mSwapChain->Present(syncInterval, 0);
}

void Dx11Renderer::SetBlendState(eastl::shared_ptr<BlendState> const& state)
{
	if (state)
	{
		if (state != mActiveBlendState)
		{
			DX11BlendState* dxState = static_cast<DX11BlendState*>(Bind(state));
			if (dxState)
			{
				dxState->Enable(mDeviceContext);
				mActiveBlendState = state;
			}
			else
			{
				LogError("Failed to bind blend state.");
			}
		}
	}
	else
	{
		LogError("Input state is null.");
	}
}

void Dx11Renderer::SetDepthStencilState(eastl::shared_ptr<DepthStencilState> const& state)
{
	if (state)
	{
		if (state != mActiveDepthStencilState)
		{
			DX11DepthStencilState* dxState = static_cast<DX11DepthStencilState*>(Bind(state));
			if (dxState)
			{
				dxState->Enable(mDeviceContext);
				mActiveDepthStencilState = state;
			}
			else
			{
				LogError("Failed to bind depth-stencil state.");
			}
		}
	}
	else
	{
		LogError("Input state is null.");
	}
}

void Dx11Renderer::SetRasterizerState(eastl::shared_ptr<RasterizerState> const& state)
{
	if (state)
	{
		if (state != mActiveRasterizerState)
		{
			DX11RasterizerState* dxState = static_cast<DX11RasterizerState*>(Bind(state));
			if (dxState)
			{
				dxState->Enable(mDeviceContext);
				mActiveRasterizerState = state;
			}
			else
			{
				LogError("Failed to bind rasterizer state.");
			}
		}
	}
	else
	{
		LogError("Input state is null.");
	}
}

void Dx11Renderer::Enable(eastl::shared_ptr<DrawTarget> const& target)
{
	DX11DrawTarget* dxTarget = static_cast<DX11DrawTarget*>(Bind(target));
	dxTarget->Enable(mDeviceContext);
	mNumActiveRTs = target->GetNumTargets();
}

void Dx11Renderer::Disable(eastl::shared_ptr<DrawTarget> const& target)
{
	DX11DrawTarget* dxTarget = static_cast<DX11DrawTarget*>(Bind(target));
	if (dxTarget)
	{
		dxTarget->Disable(mDeviceContext);
		mNumActiveRTs = 1;

		// The assumption is that Disable is called after you have written
		// the draw target outputs.  If the render targets want automatic
		// mipmap generation, we do so here.
		if (target->IsAutogenerateRTMipmaps())
		{
			unsigned int const numTargets = target->GetNumTargets();
			for (unsigned int i = 0; i < numTargets; ++i)
			{
				DX11Texture* dxTexture = static_cast<DX11Texture*>(Get(target->GetRTTexture(i)));
				ID3D11ShaderResourceView* srView = dxTexture->GetSRView();
				if (srView)
				{
					mDeviceContext->GenerateMips(dxTexture->GetSRView());
				}
			}
		}
	}
}
