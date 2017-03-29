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

#include "Core/Logger/Logger.h"

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

/*
	ResHandle tracks loaded resources. It is important for the cache to keep track of all the loaded
	resources. The ResHandle encapsulates the resource identified with the loaded resource data, when
	the cache loads a resource, it dynamically creates a ResHandle, allocates a buffer and reads the
	resource from the resource file. The ResHandle class exists in memory as long as the resource caches
	it in, or as long as any consumer fo the bits keeps a shared_ptr to a ResHandle object. The ResHandle
	also tracks the size of the memory block. If the resource cache gets full, the resource handle is
	discarded and removed from the resource cache. The destructor of ResHandle makes a call to a ResCache
	member, MemoryHasBeenFreed(). ResHandle objects are always managed through a shared_ptr and can
	therefore be actively in use at the moment the cache tries to free them. When ResHandle object goes
	out of scope, it needs to inform the resource cache that it is time to adjust the amount of memory in use.
*/
class ResHandle
{
	friend class ResCache;

protected:
	BaseResource	mResource;
	void*			mBuffer;	
	bool			mIsRawBuffer;
	unsigned int	mSize;
	ResCache*		mResCache;
	eastl::shared_ptr<BaseResourceExtraData> mExtra;

public:
	ResHandle(BaseResource & resource, void *buffer, unsigned int size, bool isRawBuffer, ResCache *resCache);

	virtual ~ResHandle();

	const eastl::wstring GetName() { return mResource.mName; }
	unsigned int Size() const { return mSize; } 
	bool IsRawBuffer() const { return mIsRawBuffer; }
	void* Buffer() const { return mBuffer; }
	void* WritableBuffer() { return mBuffer; }

	eastl::shared_ptr<BaseResourceExtraData> GetExtra() { return mExtra; }
	void SetExtra(const eastl::shared_ptr<BaseResourceExtraData>& extra) { mExtra = extra; }
};

//
// class DefaultResourceLoader
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


/*
	Resource Cache definitions. 
	While the resource is in memory, a pointer to the ResHandle exists in several data structures.
	1) ResHandleList, is a linked list which is managed such that the nodes appear in the order in
	which the resource was last used. Every time a resource is used, it is moved to the front of the list,
	so it can be found the most and least recently used resources.
	2) ResHandleMap is a map which provides a way to quickly find resource data with the unique resource
	identifier.
	3) ResourceLoaders is a list containing loaders
*/
typedef eastl::list<eastl::shared_ptr<ResHandle>> ResHandleList;					// lru list
typedef eastl::map<eastl::wstring, eastl::shared_ptr<ResHandle>> ResHandleMap;		// maps indentifiers to resource data
typedef eastl::list<eastl::shared_ptr<BaseResourceLoader>> ResourceLoaders;

/*
	Resource Cache manage memory and the process of loading resources, even predict resource requirements
	befor it is required. Resource caches work on similar principles as any other memory cache. Most of
	the bits are needed to display the next frame are probably ones used recently. As the game progresses
	from one state to the next, new resources are cached in and old ones are cached out to free space.
	When a cache miss occurs, the game has to wait while the hard drive reads the required data. Cache
	trashing occurs when a game consistently needs more resource data than can fit in the available memory
	space. The cache is forced to throw out resources that are still frequently referenced by the game.
*/
class ResCache
{
	friend class ResHandle;

	//lru (least recently used) list to track which resources are less frequently used than others
	ResHandleList	mLRU;
	ResHandleMap	mResources;
	ResourceLoaders mResourceLoaders;

	BaseResourceFile*	mFile;

	unsigned int	mCacheSize;			// total memory size
	unsigned int	mAllocated;			// total memory allocated

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

	int Preload(const eastl::wstring pattern, void (*progressCallback)(int, bool &));
	eastl::vector<eastl::wstring> Match(const eastl::wstring pattern);

	void Flush(void);

    bool IsUsingDevelopmentDirectories(void) const 
	{ 
		LogAssert(mFile, "Invalid file"); 
		return mFile->IsUsingDevelopmentDirectories(); 
	}
};

#endif