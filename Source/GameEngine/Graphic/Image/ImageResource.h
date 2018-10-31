//========================================================================
// XmlResource.h : API to use load Xml files from the Resource Cache
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

#ifndef IMAGERESOURCE_H
#define IMAGERESOURCE_H

#include "GameEngineStd.h"

#include "Graphic/Resource/Texture/Texture2.h"
#include "Core/IO/ResourceCache.h"


class ImageResourceExtraData : public BaseResourceExtraData
{

public:
    virtual eastl::wstring ToString() { return L"ImageResourceExtraData"; }
	virtual eastl::shared_ptr<Texture2> GetImage() { return mImage; }
	virtual void SetImage(eastl::shared_ptr<Texture2> img) { mImage = img; }

protected:
	eastl::shared_ptr<Texture2> mImage;
};


class ImageResourceLoader : public BaseResourceLoader
{
public:
    virtual bool UseRawFile() { return false; }
	virtual bool DiscardRawBufferAfterLoad() { return false; }
    virtual unsigned int GetLoadedResourceSize(void *rawBuffer, unsigned int rawSize) { return rawSize; }
    virtual bool LoadResource(void *rawBuffer, unsigned int rawSize, const eastl::shared_ptr<ResHandle>& handle);
	virtual bool MatchResourceFormat(eastl::wstring name) { return IsALoadableFileExtension(name.c_str()); }

protected:

	bool IsALoadableFileExtension(const eastl::wstring& filename) const;

	// Support for loading from BMP, GIF, ICON, JPEG, PNG, and TIFF.
	// The returned texture has a format that matches as close as possible
	// the format on disk.  If the load is not successful, the function
	// returns a null object.
	eastl::shared_ptr<Texture2> Load(eastl::wstring const& filename, bool wantMipmaps);

private:

	struct LoadFormatMap
	{
		DFType mGTFormat;
		GUID const* mWicInputGUID;
		GUID const* mWicConvertGUID;
	};
	enum { NUM_LOAD_FORMATS = 14 };
	static LoadFormatMap const mLoadFormatMap[NUM_LOAD_FORMATS];

	class ComInitializer
	{
	public:
		~ComInitializer();
		ComInitializer();
		bool IsInitialized() const;
	private:
		bool mInitialized;
	};

	template <typename T>
	class ComObject
	{
	public:
		ComObject();
		ComObject(T* inObject);
		~ComObject();
		operator T*() const;
		T& operator*() const;
		T** operator&();
		T* operator->() const;
	private:
		T* object;
	};
};


#endif
