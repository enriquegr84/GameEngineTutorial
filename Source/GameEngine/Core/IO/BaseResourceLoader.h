// Copyright (C) 2002-2012 Nikolaus Gebhardt
// This file is part of the "Irrlicht Engine".
// For conditions of distribution and use, see copyright notice in irrlicht.h

#ifndef BASERESOURCELOADER_H
#define BASERESOURCELOADER_H

class ResHandle;

//! The core of a resource cache system
class BaseResourceLoader
{
public:
	virtual bool MatchResourceFormat(eastl::wstring name) = 0;
	virtual bool UseRawFile() = 0;
	virtual bool DiscardRawBufferAfterLoad() = 0;
	virtual bool AddNullZero() { return false; }
	virtual unsigned int GetLoadedResourceSize(void *rawBuffer, unsigned int rawSize) = 0;
	virtual bool LoadResource(
		void *rawBuffer, unsigned int rawSize, const eastl::shared_ptr<ResHandle>& handle) = 0;
};

#endif

