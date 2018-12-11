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

#include "Graphic/3rdParty/stb/stb_image.h"

#include "Core/Utility/StringUtil.h"


//! returns true if the file maybe is able to be loaded by this class
//! based on the file extension
bool ImageResourceLoader::IsALoadableFileExtension(const eastl::wstring& fileName) const
{
	if (fileName.rfind('.') != eastl::string::npos)
	{
		eastl::wstring fileExtension = fileName.substr(fileName.rfind('.') + 1);
		return fileExtension.compare(L"bmp") == 0 || fileExtension.compare(L"psd") == 0 ||
			fileExtension.compare(L"hdr") == 0 || fileExtension.compare(L"jpeg") == 0 || 
			fileExtension.compare(L"jpg") == 0 || fileExtension.compare(L"pic") == 0 ||
			fileExtension.compare(L"png") == 0 || fileExtension.compare(L"tga") == 0;
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
	int width, height, components;

	unsigned char *imageData = stbi_load(
		ToString(fileName.c_str()).c_str(), &width, &height, &components, STBI_rgb_alpha);
	if (imageData == nullptr)
	{
		LogError("load texture failed.");
		return nullptr;
	}

	//     N=#comp     components
	//       1           grey
	//       2           grey, alpha
	//       3           red, green, blue
	//       4           red, green, blue, alpha

	// R8G8B8A8 format with texels converted from the source format.
	DFType gtformat = DF_R8G8B8A8_UNORM;
	
	// Create the 2D texture and compute the stride and image size.
	eastl::shared_ptr<Texture2> texture = 
		eastl::make_shared<Texture2>(gtformat, width, height, wantMipMaps);

	UINT const stride = width * texture->GetElementSize();
	UINT const imageSize = stride * height;

	// Copy the pixels from the decoder to the texture.
	std::memcpy(texture->Get<BYTE>(), imageData, imageSize);
	stbi_image_free(imageData);

	return texture;
}