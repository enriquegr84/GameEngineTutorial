//========================================================================
// ResCache.cpp : Defines a simple resource cache
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

#include "ResourceCache.h"

#include "Core/Utility/StringUtil.h"

//
//  Resource::Resource
//
BaseResource::BaseResource(const eastl::wstring &resourceName) 
{
	mName = resourceName;
}

//
// ResHandle::ResHandle							- Chapter 8, page 223
//
ResHandle::ResHandle(BaseResource & resource, void *buffer, unsigned int size, bool isRawBuffer, ResCache *resCache)
: mResource(resource)
{
	mBuffer = buffer;
	mIsRawBuffer = isRawBuffer;
	mSize = size;
	mExtra = NULL;
	mResCache = resCache;
}

//
// ResHandle::ResHandle							- Chapter 8, page 223
//
ResHandle::~ResHandle()
{
	if (mIsRawBuffer)
		delete mBuffer;
	else
		delete[] mBuffer;
	
	mResCache->MemoryHasBeenFreed(mSize);
}


ResCache* ResCache::mResCache = NULL;

ResCache* ResCache::Get(void)
{
	LogAssert(ResCache::mResCache, "Resource cache doesn't exist");
	return ResCache::mResCache;
}

//
// ResCache::ResCache							- Chapter 8, page 227
//
ResCache::ResCache(const unsigned int sizeInMb, BaseResourceFile *resFile )
{
	mCacheSize = sizeInMb * 1024 * 1024; // total memory size
	mAllocated = 0; // total memory allocated
	mFile = resFile;

	if (ResCache::mResCache)
	{
		LogError("Attempting to create two global resource cache! \
					The old one will be destroyed and overwritten with this one.");
		delete ResCache::mResCache;
	}

	ResCache::mResCache = this;
}

//
// ResCache::~ResCache							- Chapter 8, page 227
//
ResCache::~ResCache()
{
	while (!mLRU.empty())
	{
		FreeOneResource();
	}
	delete mFile;

	if (ResCache::mResCache == this)
		ResCache::mResCache = nullptr;
}

//
// ResCache::Init								- Chapter 8, page 227
//
bool ResCache::Init()
{ 
	bool retValue = false;
	if ( mFile->Open() )
	{
		RegisterLoader(
			eastl::shared_ptr<BaseResourceLoader>(new DefaultResourceLoader()));
		retValue = true;
	}
	return retValue;
}

//
// ResCache::RegisterLoader						- Chapter 8, page 225
// 
//    The loaders are discussed on the page refereced above - this method simply adds the loader
//    to the resource cache.
//
void ResCache::RegisterLoader(const eastl::shared_ptr<BaseResourceLoader>& loader )
{
	mResourceLoaders.push_front(loader);
}


//
// ResCache::GetHandle							- Chapter 8, page 227
//
eastl::shared_ptr<ResHandle> ResCache::GetHandle(BaseResource * r)
{
	eastl::shared_ptr<ResHandle> handle(Find(r));
	if (handle==NULL)
	{
		handle = Load(r);
		//GE_ASSERT(handle);
	}
	else
	{
		Update(handle);
	}
	return handle;
}

/*
	Load a resource. First it is located the right resource loader using its name as identifier.
	If a loader isn't found an empty ResHandle is returned. Then the method grabas the size of the raw
	resource. If the resource doesn't need any processing, the memory is allocated from the cache
	through allocate() method; otherwise, a temporary buffer is created. If the memory allocation is
	sucessful, the raw resource bits are loaded with the call to GetRawResource(). If no further
	processing of the resource is needed, a ResHandle object is crated using pointers to the raw bits
	and the raw resource size. Othe resources need processing and might even be a different size after 
	they are loaded. This is the job of a specially defined resource loader, which loads the raw bits
	from the resource file, calculates the final size of the processed resource, allocates the right
	amount of memory in cache, and finally copies the processed resource into the new buffer.
	After the resource is loaded, the newly created ResHandle is pushed onto the LRU list, and the 
	resource name is entered into the resource name map.
*/
eastl::shared_ptr<ResHandle> ResCache::Load(BaseResource *r)
{
	// Create a new resource and add it to the lru list and map
	BaseResourceLoader* loader = 0;
	eastl::shared_ptr<ResHandle> handle = 0;

	for (ResourceLoaders::iterator it = mResourceLoaders.begin(); it != mResourceLoaders.end(); ++it)
	{
		BaseResourceLoader* testLoader = (*it).get();

		if (testLoader->MatchResourceFormat(r->mName))
		{
			loader = testLoader;
			break;
		}
	}

	if (!loader)
	{
		LogAssert(loader, "Default resource loader not found!");
		return nullptr;		// Resource not loaded!
	}

	void* rawBuffer = NULL;
	int rawSize = mFile->GetRawResource(*r, &rawBuffer);
	if (rawBuffer == NULL || rawSize < 0)
	{
		// resource cache out of memory
		LogAssert(false, eastl::wstring("Resource not found ") + r->mName);
		return nullptr;
	}
	/*
	if (loader->UseRawFile())
	{
		rawBuffer = new char[rawSize];
		memset(rawBuffer, 0, rawSize);
	}
	*/

	void *buffer = rawBuffer;
	unsigned int size = rawSize;

	{
		handle = eastl::shared_ptr<ResHandle>(new ResHandle(*r, buffer, size, true, this));
		bool success = loader->LoadResource(buffer, size, handle);

		// This was added after the chapter went to copy edit. It is used for those
		// resoruces that are converted to a useable format upon load, such as a compressed
		// file. If the raw buffer from the resource file isn't needed, it shouldn't take up
		// any additional memory, so we release it.
		/*
		if (loader->DiscardRawBufferAfterLoad())
		{
			delete[] buffer;
		}
		*/

		if (!success)
		{
			// resource cache out of memory
			return nullptr;
		}
	}

	if (handle)
	{
		mLRU.push_front(eastl::shared_ptr<ResHandle>(handle));
		mResources[r->mName] = mLRU.front();
	}

	LogAssert(loader, "Default resource loader not found!");
	return handle;		// ResCache is out of memory!
}

bool ResCache::ExistResource(BaseResource * r) 
{ 
	if (Find(r))
		return true;

	return mFile->ExistFile(r->mName);
}

bool ResCache::ExistDirectory(const eastl::wstring& dirname) 
{ 
	return mFile->ExistDirectory(dirname);
}

int ResCache::GetResource(BaseResource * r, void** buffer) 
{ 
	int size = mFile->GetRawResource(r->mName, buffer);
	if (buffer == NULL || size < 0)
	{
		// resource cache out of memory
		LogAssert(size < 0, "Resource size returned -1 - Resource not found");
		return 0;
	}
	return size;
}

//
// ResCache::Find									- Chapter 8, page 228
//
eastl::shared_ptr<ResHandle> ResCache::Find(BaseResource * r)
{
	ResHandleMap::iterator i = mResources.find(r->mName);
	if (i==mResources.end())
		return nullptr;

	return i->second;
}

/*
	Update removes a ResHandle from the LRU list and promotes it to the front,
	making sure that the LRU is always sorted properly
*/
void ResCache::Update(const eastl::shared_ptr<ResHandle>& handle)
{
	mLRU.remove(handle);
	mLRU.push_front(handle);
}

/*
	Allocate makes room in the cache when it is needed
*/
char* ResCache::Allocate(unsigned int size)
{
	if (!MakeRoom(size))
		return NULL;

	char *mem = new char[size];
	if (mem)
	{
		mAllocated += size;
	}

	return mem;
}


/*
	FreeOneResource removes the oldest resource and updates the cache data members. Note that the memory
	used by the cache isn't actually modified here, that's because any active shared_ptr<ResHandle> in
	use will need the bits until it actually goes out of scope.
*/
void ResCache::FreeOneResource()
{
	ResHandleList::iterator gonner = mLRU.end();
	gonner--;

	eastl::shared_ptr<ResHandle> handle = *gonner;

	mLRU.pop_back();							
	mResources.erase(handle->mResource.mName);
	// Note - you can't change the resource cache size yet - the resource bits could still actually be
	// used by some sybsystem holding onto the ResHandle. Only when it goes out of scope can the memory
	// be actually free again.
}



//
// ResCache::Flush									- not described in the book
//
//    Frees every handle in the cache - this would be good to call if you are loading a new
//    level, or if you wanted to force a refresh of all the data in the cache - which might be 
//    good in a development environment.
//
void ResCache::Flush()
{
	while (!mLRU.empty())
	{
		Free(*(mLRU.begin()));
		mLRU.pop_front();
	}
}


//
// ResCache::MakeRoom									- Chapter 8, page 231
//
bool ResCache::MakeRoom(unsigned int size)
{
	if (size > mCacheSize)
	{
		return false;
	}

	// return null if there's no possible way to allocate the memory
	while (size > (mCacheSize - mAllocated))
	{
		// The cache is empty, and there's still not enough room.
		if (mLRU.empty())
			return false;

		FreeOneResource();
	}

	return true;
}

//
//	ResCache::Free									- Chapter 8, page 228
//
void ResCache::Free(const eastl::shared_ptr<ResHandle>& gonner)
{
	mLRU.remove(gonner);
	mResources.erase(gonner->mResource.mName);
	// Note - the resource might still be in use by something,
	// so the cache can't actually count the memory freed until the
	// ResHandle pointing to it is destroyed.

	//m_allocated -= gonner->m_resource.m_size;
	//delete gonner;
}

//
//  ResCache::MemoryHasBeenFreed					- not described in the book
//
//     This is called whenever the memory associated with a resource is actually freed
//
void ResCache::MemoryHasBeenFreed(unsigned int size)
{
	mAllocated -= size;
}

//
// ResCache::Match									- not described in the book
//
//   Searches the resource cache assets for files matching the pattern. Useful for providing a 
//   a list of levels for a main menu screen, for example.
//
eastl::vector<eastl::wstring> ResCache::Match(const eastl::wstring pattern)
{
	eastl::vector<eastl::wstring> matchingNames;
	if (mFile==NULL)
		return matchingNames;

	/*
	FileSystem* fileSystem = FileSystem::Get();

	bool searchDirectory = false;
	eastl::string directory = FileSystem->GetFileDir(pattern);
	eastl::string filePattern = Utils::GetFileBasename(pattern);
	eastl::string currentDirectory = ".";
	if ( directory != ".")
		searchDirectory = true;
	*/

	int numFiles = mFile->GetNumResources();
	for (int i=0; i<numFiles; ++i)
	{
		eastl::wstring name(mFile->GetResourceName(i).c_str());
		/*
		if (searchDirectory)
			if (name.findLast ( '.' ) < 0)
				currentDirectory = name;

		if (!WildcardMatch(directory.c_str(), currentDirectory.c_str()))
			continue;
		*/
		if (WildcardMatch(pattern.c_str(), name.c_str()))
			matchingNames.push_back(name);
	}
	return matchingNames;
}


//
// ResCache::Preload								- Chapter 8, page 236
//
int ResCache::Preload(const eastl::wstring pattern, void (*progressCallback)(int, bool &))
{
	if (mFile==NULL)
		return 0;

	int numFiles = mFile->GetNumResources();
	int loaded = 0;
	bool cancel = false;

	for (int i=0; i<numFiles; ++i)
	{
		BaseResource resource(mFile->GetResourceName(i));

		if (WildcardMatch(pattern.c_str(), resource.mName.c_str()))
		{
			const eastl::shared_ptr<ResHandle>& handle = 
				ResCache::Get()->GetHandle(&resource);
			++loaded;
		}

		if (progressCallback != NULL)
		{
			progressCallback(i * 100/numFiles, cancel);
		}
	}
	return loaded;
}
