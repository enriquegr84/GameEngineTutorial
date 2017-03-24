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

#include "Core/Utility/String.h"

#include "Application/GameApplication.h"

//
//  Resource::Resource
//
BaseResource::BaseResource(const eastl::wstring &resourceName) 
{
	m_name = eastl::wstring(resourceName.c_str());
}

//
// ResHandle::ResHandle							- Chapter 8, page 223
//
ResHandle::ResHandle(BaseResource & resource, void *buffer, unsigned int size, bool isRawBuffer, ResCache *pResCache)
: m_resource(resource)
{
	m_buffer = buffer;
	m_isRawBuffer = isRawBuffer;
	m_size = size;
	m_extra = NULL;
	m_pResCache = pResCache;
}

//
// ResHandle::ResHandle							- Chapter 8, page 223
//
ResHandle::~ResHandle()
{
	if (m_isRawBuffer)
		delete m_buffer;
	else
		delete[] m_buffer;
	
	m_pResCache->MemoryHasBeenFreed(m_size);
}


//
// ResCache::ResCache							- Chapter 8, page 227
//
ResCache::ResCache(const unsigned int sizeInMb, BaseResourceFile *resFile )
{
	m_cacheSize = sizeInMb * 1024 * 1024; // total memory size
	m_allocated = 0; // total memory allocated
	m_File = resFile;
}

//
// ResCache::~ResCache							- Chapter 8, page 227
//
ResCache::~ResCache()
{
	while (!m_lru.empty())
	{
		FreeOneResource();
	}
	delete m_File;
}

//
// ResCache::Init								- Chapter 8, page 227
//
bool ResCache::Init()
{ 
	bool retValue = false;
	if ( m_File->Open() )
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
	m_resourceLoaders.push_front(loader);
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

	for (ResourceLoaders::iterator it = m_resourceLoaders.begin(); it != m_resourceLoaders.end(); ++it)
	{
		BaseResourceLoader* testLoader = (*it).get();

		if (testLoader->MatchResourceFormat(r->m_name))
		{
			loader = testLoader;
			break;
		}
	}

	if (!loader)
	{
		LogAssert(loader, "Default resource loader not found!");
		return 0;		// Resource not loaded!
	}

	void* rawBuffer = NULL;
	int rawSize = m_File->GetRawResource(*r, &rawBuffer);
	if (rawBuffer == NULL || rawSize < 0)
	{
		// resource cache out of memory
		LogAssert(false, eastl::wstring("Resource not found ") + r->m_name);
		return 0;
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
			return 0;
		}
	}

	if (handle)
	{
		m_lru.push_front(eastl::shared_ptr<ResHandle>(handle));
		m_resources[r->m_name] = m_lru.front();
	}

	LogAssert(loader, "Default resource loader not found!");
	return handle;		// ResCache is out of memory!
}

bool ResCache::ExistResource(BaseResource * r) 
{ 
	if (Find(r))
		return true;

	return m_File->ExistFile(r->m_name);
}

bool ResCache::ExistDirectory(const eastl::wstring& dirname) 
{ 
	return m_File->ExistDirectory(dirname);
}

int ResCache::GetResource(BaseResource * r, void** buffer) 
{ 
	int size = m_File->GetRawResource(r->m_name, buffer);
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
	ResHandleMap::iterator i = m_resources.find(eastl::wstring(r->m_name.c_str()));
	if (i==m_resources.end())
		return 0;

	return i->second;
}

/*
	Update removes a ResHandle from the LRU list and promotes it to the front,
	making sure that the LRU is always sorted properly
*/
void ResCache::Update(const eastl::shared_ptr<ResHandle>& handle)
{
	m_lru.remove(handle);
	m_lru.push_front(handle);
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
		m_allocated += size;
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
	ResHandleList::iterator gonner = m_lru.end();
	gonner--;

	eastl::shared_ptr<ResHandle> handle = *gonner;

	m_lru.pop_back();							
	m_resources.erase(eastl::wstring(handle->m_resource.m_name.c_str()));
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
	while (!m_lru.empty())
	{
		Free(*(m_lru.begin()));
		m_lru.pop_front();
	}
}


//
// ResCache::MakeRoom									- Chapter 8, page 231
//
bool ResCache::MakeRoom(unsigned int size)
{
	if (size > m_cacheSize)
	{
		return false;
	}

	// return null if there's no possible way to allocate the memory
	while (size > (m_cacheSize - m_allocated))
	{
		// The cache is empty, and there's still not enough room.
		if (m_lru.empty())
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
	m_lru.remove(gonner);
	m_resources.erase(eastl::wstring(gonner->m_resource.m_name.c_str()));
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
	m_allocated -= size;
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
	if (m_File==NULL)
		return matchingNames;

	/*
	GameApplication* gameApp = (GameApplication*)Application::App;
	bool searchDirectory = false;
	eastl::string directory = gameApp->mFileSystem->GetFileDir(pattern);
	eastl::string filePattern = Utils::GetFileBasename(pattern);
	eastl::string currentDirectory = ".";
	if ( directory != ".")
		searchDirectory = true;
	*/

	int numFiles = m_File->GetNumResources();
	for (int i=0; i<numFiles; ++i)
	{
		eastl::wstring name(m_File->GetResourceName(i).c_str());
		/*
		if (searchDirectory)
			if (name.findLast ( '.' ) < 0)
				currentDirectory = name;

		if (!WildcardMatch(directory.c_str(), currentDirectory.c_str()))
			continue;
		*/
		if (WildcardMatch(pattern.c_str(), name.c_str()))
		{
			matchingNames.push_back(eastl::wstring(name.c_str()));
		}
	}
	return matchingNames;
}


//
// ResCache::Preload								- Chapter 8, page 236
//
int ResCache::Preload(const eastl::wstring pattern, void (*progressCallback)(int, bool &))
{
	if (m_File==NULL)
		return 0;

	int numFiles = m_File->GetNumResources();
	int loaded = 0;
	bool cancel = false;

	GameApplication* gameApp = (GameApplication*)Application::App;
	for (int i=0; i<numFiles; ++i)
	{
		BaseResource resource(m_File->GetResourceName(i));

		if (WildcardMatch(pattern.c_str(), resource.m_name.c_str()))
		{
			const eastl::shared_ptr<ResHandle>& handle = gameApp->mResCache->GetHandle(&resource);
			++loaded;
		}

		if (progressCallback != NULL)
		{
			progressCallback(i * 100/numFiles, cancel);
		}
	}
	return loaded;
}
