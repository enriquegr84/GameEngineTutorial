//========================================================================
// XmlResource.cpp : API to use load Xml files from the Resource Cache
//
// Part of the GameEngine Application
//
// GameEngine is the sample application that encapsulates much of the source code
// discussed in "Game Coding Complete - 4th Edition" by Mike McShaffry and David
// "Rez" Graham, published by Charles River Media. 
// ISBN-10: 1133776574 | ISBN-13: 978-1133776574
//
// If this source code has found it's way to you, and you think it has helped you
// in any way, do the authors a favor and buy a new copy of the book - there are 
// detailed explanations in it that compliment this code well. Buy a copy at Amazon.com
// by clicking here: 
//    http://www.amazon.com/gp/product/1133776574/ref=olp_product_details?ie=UTF8&me=&seller=
//
// There's a companion web site at http://www.mcshaffry.com/GameCode/
// 
// The source code is managed and maintained through Google Code: 
//    http://code.google.com/p/GameEngine/
//
// (c) Copyright 2012 Michael L. McShaffry and David Graham
//
// This program is free software; you can redistribute it and/or
// modify it under the terms of the GNU Lesser GPL v3
// as published by the Free Software Foundation.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See 
// http://www.gnu.org/licenses/lgpl-3.0.txt for more details.
//
// You should have received a copy of the GNU Lesser GPL v3
// along with this program; if not, write to the Free Software
// Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
//
//========================================================================

#include "ImageResource.h"

// wincodec.h includes windows.h, so we must turn off min/max macros
#include <wincodec.h>

#pragma comment(lib, "windowscodecs.lib")

ImageResourceLoader::LoadFormatMap const ImageResourceLoader::mLoadFormatMap[NUM_LOAD_FORMATS] =
{
	{ DF_B5G6R5_UNORM, &GUID_WICPixelFormat16bppBGR565, nullptr },
	{ DF_B5G5R5A1_UNORM, &GUID_WICPixelFormat16bppBGR555, nullptr },
	{ DF_R10G10B10A2_UNORM, &GUID_WICPixelFormat32bppRGBA1010102, nullptr },
	{ DF_R10G10B10_XR_BIAS_A2_UNORM, &GUID_WICPixelFormat32bppRGBA1010102XR, nullptr },
	{ DF_R1_UNORM, &GUID_WICPixelFormatBlackWhite, &GUID_WICPixelFormat8bppGray },
	{ DF_R8_UNORM, &GUID_WICPixelFormat2bppGray, &GUID_WICPixelFormat8bppGray },
	{ DF_R8_UNORM, &GUID_WICPixelFormat4bppGray, &GUID_WICPixelFormat8bppGray },
	{ DF_R8_UNORM, &GUID_WICPixelFormat8bppGray, nullptr },
	{ DF_R16_UNORM, &GUID_WICPixelFormat16bppGray, nullptr },
	{ DF_R32_FLOAT, &GUID_WICPixelFormat32bppGrayFloat, nullptr },
	{ DF_R8G8B8A8_UNORM, &GUID_WICPixelFormat32bppRGBA, nullptr },
	{ DF_R8G8B8A8_UNORM, &GUID_WICPixelFormat32bppBGRA, &GUID_WICPixelFormat32bppRGBA },
	{ DF_R16G16B16A16_UNORM, &GUID_WICPixelFormat64bppRGBA, nullptr },
	{ DF_R16G16B16A16_UNORM, &GUID_WICPixelFormat64bppBGRA, &GUID_WICPixelFormat64bppRGBA }

	// B8G8R8A8 is not supported for Texture2 in DX11.  We convert all
	// unmatched formats to R8G8B8A8.
	//{ DF_B8G8R8A8_UNORM, &GUID_WICPixelFormat32bppBGRA }
};

//! returns true if the file maybe is able to be loaded by this class
//! based on the file extension
bool ImageResourceLoader::IsALoadableFileExtension(const eastl::wstring& fileName) const
{
	if (fileName.rfind('.') != eastl::string::npos)
	{
		eastl::wstring fileExtension = fileName.substr(fileName.rfind('.') + 1);
		return fileExtension.compare(L"bmp") == 0 || fileExtension.compare(L"gif") == 0 ||
			fileExtension.compare(L"icon") == 0 || fileExtension.compare(L"jpeg") == 0 || 
			fileExtension.compare(L"png") == 0 || fileExtension.compare(L"tiff") == 0;
	}
	else return false;
}

bool ImageResourceLoader::LoadResource(
	void *rawBuffer, unsigned int rawSize, const eastl::shared_ptr<ResHandle>& handle)
{
	eastl::shared_ptr<ImageResourceExtraData> pExtraData(new ImageResourceExtraData());
	pExtraData->SetImage(0);

	// try to load file based on file extension
	BaseReadFile* file = (BaseReadFile*)rawBuffer;
	if (IsALoadableFileExtension(file->GetFileName()))
	{
		pExtraData->SetImage(Load(file->GetFileName(), true));
		if (pExtraData->GetImage())
		{
			handle->SetExtra(eastl::shared_ptr<ImageResourceExtraData>(pExtraData));
			return true;
		}
	}

	return false;
}


eastl::shared_ptr<BaseResourceLoader> CreateImageResourceLoader()
{
    return eastl::shared_ptr<BaseResourceLoader>(new ImageResourceLoader());
}

eastl::shared_ptr<Texture2> ImageResourceLoader::Load(eastl::wstring const& fileName, bool wantMipMaps)
{
	// Start COM and create WIC.
	ComInitializer comInitializer;
	if (!comInitializer.IsInitialized())
	{
		LogError("Unable to initialize COM for WIC.");
		return nullptr;
	}

	// Create a WIC imaging factory.
	ComObject<IWICImagingFactory> wicFactory;
	HRESULT hr = ::CoCreateInstance(CLSID_WICImagingFactory, nullptr,
		CLSCTX_INPROC_SERVER, IID_IWICImagingFactory,
		reinterpret_cast<LPVOID*>(&wicFactory));
	if (FAILED(hr))
	{
		LogError("Unable to create WIC imaging factory.");
		return nullptr;
	}

	// Create a decoder based on the file name.
	ComObject<IWICBitmapDecoder> wicDecoder;
	hr = wicFactory->CreateDecoderFromFilename(fileName.c_str(),
		nullptr, GENERIC_READ, WICDecodeMetadataCacheOnDemand, &wicDecoder);
	if (FAILED(hr))
	{
		LogError(L"WicFactory->CreateDecoderFromFilename failed (" + fileName + L").");
		return nullptr;
	}

	// Create a WIC decoder.
	ComObject<IWICBitmapFrameDecode> wicFrameDecode;
	hr = wicDecoder->GetFrame(0, &wicFrameDecode);
	if (FAILED(hr))
	{
		LogError("WicDecoder->GetFrame failed.");
		return nullptr;
	}

	// Get the pixel format of the image.
	WICPixelFormatGUID wicSourceGUID;
	hr = wicFrameDecode->GetPixelFormat(&wicSourceGUID);
	if (FAILED(hr))
	{
		LogError("WicFrameDecode->GetPixelFormat failed.");
		return nullptr;
	}

	// Find the supported WIC input pixel format that matches a Texture2
	// format.  If a matching format is not found, the returned texture
	// is an R8G8B8A8 format with texels converted from the source format.
	WICPixelFormatGUID wicConvertGUID = GUID_WICPixelFormat32bppRGBA;
	DFType gtformat = DF_R8G8B8A8_UNORM;
	for (int i = 0; i < NUM_LOAD_FORMATS; ++i)
	{
		if (IsEqualGUID(wicSourceGUID, *mLoadFormatMap[i].mWicInputGUID))
		{
			// Determine whether there is a conversion format.
			if (mLoadFormatMap[i].mWicConvertGUID)
			{
				wicConvertGUID = *mLoadFormatMap[i].mWicConvertGUID;
			}
			else
			{
				wicConvertGUID = *mLoadFormatMap[i].mWicInputGUID;
			}
			gtformat = mLoadFormatMap[i].mGTFormat;
			break;
		}
	}

	// The wicFrameDecode value is used for no conversion.  If the decoder
	// does not support the format in the texture, then a conversion is
	// required.
	IWICBitmapSource* wicBitmapSource = wicFrameDecode;
	ComObject<IWICFormatConverter> wicFormatConverter;
	if (!IsEqualGUID(wicSourceGUID, wicConvertGUID))
	{
		// Create a WIC format converter.
		hr = wicFactory->CreateFormatConverter(&wicFormatConverter);
		if (FAILED(hr))
		{
			LogError("WicFactory->CreateFormatConverter failed.");
			return nullptr;
		}

		// Initialize format converter to convert the input texture format
		// to the nearest format supported by the decoder.
		hr = wicFormatConverter->Initialize(wicFrameDecode, wicConvertGUID,
			WICBitmapDitherTypeNone, nullptr, 0.0,
			WICBitmapPaletteTypeCustom);
		if (FAILED(hr))
		{
			LogError("WicFormatConverter->Initialize failed.");
			return nullptr;
		}

		// Use the format converter.
		wicBitmapSource = wicFormatConverter;
	}

	// Get the image dimensions.
	UINT width, height;
	hr = wicBitmapSource->GetSize(&width, &height);
	if (FAILED(hr))
	{
		LogError("WicBitmapSource->GetSize failed.");
		return nullptr;
	}

	// Create the 2D texture and compute the stride and image size.
	eastl::shared_ptr<Texture2> texture = 
		eastl::make_shared<Texture2>(gtformat, width, height, wantMipMaps);
	UINT const stride = width * texture->GetElementSize();
	UINT const imageSize = stride * height;

	// Copy the pixels from the decoder to the texture.
	hr = wicBitmapSource->CopyPixels(nullptr, stride, imageSize,
		texture->Get<BYTE>());
	if (FAILED(hr))
	{
		LogError("WicBitmapSource->CopyPixels failed.");
		return nullptr;
	}

	return texture;
}

ImageResourceLoader::ComInitializer::~ComInitializer()
{
	if (mInitialized)
	{
		::CoUninitialize();
	}
}

ImageResourceLoader::ComInitializer::ComInitializer()
{
	HRESULT hr = ::CoInitializeEx(nullptr, COINIT_APARTMENTTHREADED);
	mInitialized = SUCCEEDED(hr);
}

bool ImageResourceLoader::ComInitializer::IsInitialized() const
{
	return mInitialized;
}

template <typename T>
ImageResourceLoader::ComObject<T>::ComObject()
	:
	object(nullptr)
{
}

template <typename T>
ImageResourceLoader::ComObject<T>::ComObject(T* inObject)
	:
	object(inObject)
{
	if (object != nullptr)
	{
		object->AddRef();
	}
}

template <typename T>
ImageResourceLoader::ComObject<T>::~ComObject()
{
	if (object)
	{
		object->Release();
	}
}

template <typename T>
ImageResourceLoader::ComObject<T>::operator T*() const
{
	return object;
}

template <typename T>
T& ImageResourceLoader::ComObject<T>::operator*() const
{
	return *object;
}

template <typename T>
T** ImageResourceLoader::ComObject<T>::operator&()
{
	return &object;
}

template <typename T>
T* ImageResourceLoader::ComObject<T>::operator->() const
{
	return object;
}