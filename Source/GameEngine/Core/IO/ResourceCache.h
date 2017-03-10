//========================================================================
// ResCache.h : Defines a simple resource cache.
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

#ifndef RESCACHE_H
#define RESCACHE_H

#include "GameEngineStd.h"

#include "BaseResourceFile.h"
#include "BaseResourceLoader.h"

//
// class BaseResourceExtraData		- Chapter 8, page 224 (see notes below)
//
//   This isn't mentioned specifically on that page, but it is a class that can attach extra data to 
//   a particular resource. Best example is storing the length and format of a sound file. 
//   There's a great discussion of this in Chapter 13, "Game Audio"
//
class BaseResourceExtraData
{
public:
	virtual eastl::wstring ToString()=0;
};

//
//  class ResHandle			- Chapter 8, page 222
//
class ResHandle
{
	friend class ResCache;

protected:
	BaseResource		m_resource;
	void*			m_buffer;	
	bool			m_isRawBuffer;
	unsigned int	m_size;
	ResCache*		m_pResCache;
	eastl::shared_ptr<BaseResourceExtraData> m_extra;

public:
	ResHandle(BaseResource & resource, void *buffer, unsigned int size, bool isRawBuffer, ResCache *pResCache);

	virtual ~ResHandle();

	const eastl::wstring GetName() { return m_resource.m_name; }
	unsigned int Size() const { return m_size; } 
	bool IsRawBuffer() const { return m_isRawBuffer; }
	void* Buffer() const { return m_buffer; }
	void* WritableBuffer() { return m_buffer; }

	eastl::shared_ptr<BaseResourceExtraData> GetExtra() { return m_extra; }
	void SetExtra(const eastl::shared_ptr<BaseResourceExtraData>& extra) { m_extra = extra; }
};

//
// class DefaultResourceLoader							- Chapter 8, page 225
//
class DefaultResourceLoader : public BaseResourceLoader
{
public:
	virtual bool UseRawFile() { return true; }
	virtual bool DiscardRawBufferAfterLoad() { return true; }
	virtual unsigned int GetLoadedResourceSize(void* rawBuffer, unsigned int rawSize) { return rawSize; }
	virtual bool LoadResource(
		void* rawBuffer, unsigned int rawSize, const eastl::shared_ptr<ResHandle>& handle) { return true; }
	virtual bool MatchResourceFormat(eastl::wstring name){ return false; }

};

//
//  class ResCache										- Chapter 8, page 225
//
typedef eastl::list<eastl::shared_ptr<ResHandle>> ResHandleList;					// lru list
typedef eastl::map<eastl::wstring, eastl::shared_ptr<ResHandle>> ResHandleMap;		// maps indentifiers to resource data
typedef eastl::list<eastl::shared_ptr<BaseResourceLoader>> ResourceLoaders;

class ResCache
{
	friend class ResHandle;

	ResHandleList	m_lru;							// lru list
	ResHandleMap	m_resources;
	ResourceLoaders m_resourceLoaders;

	BaseResourceFile*	m_File;

	unsigned int	m_cacheSize;			// total memory size
	unsigned int	m_allocated;			// total memory allocated

protected:

	bool MakeRoom(unsigned int size);
	char *Allocate(unsigned int size);
	void Free(const eastl::shared_ptr<ResHandle>& gonner);

	eastl::shared_ptr<ResHandle> Load(BaseResource * r);
	eastl::shared_ptr<ResHandle> Find(BaseResource * r);
	void Update(const eastl::shared_ptr<ResHandle>& handle);

	void FreeOneResource();
	void MemoryHasBeenFreed(unsigned int size);

public:
	ResCache(const unsigned int sizeInMb, BaseResourceFile *file);
	virtual ~ResCache();

	bool Init(); 
	
	void RegisterLoader(const eastl::shared_ptr<BaseResourceLoader>& loader );
	
	bool ExistDirectory(const eastl::wstring& dirname);

	bool ExistResource(BaseResource * r);
	int GetResource(BaseResource* r, void** buffer);
	eastl::shared_ptr<ResHandle> GetHandle(BaseResource * r);

	int Preload(const eastl::string pattern, void (*progressCallback)(int, bool &));
	eastl::vector<eastl::wstring> Match(const eastl::string pattern);

	void Flush(void);

    bool IsUsingDevelopmentDirectories(void) const 
	{ 
		LogAssert(m_File, "Invalid file"); 
		return m_File->IsUsingDevelopmentDirectories(); 
	}

};


#endif