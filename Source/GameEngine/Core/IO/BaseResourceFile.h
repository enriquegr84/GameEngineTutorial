// Copyright (C) 2002-2012 Nikolaus Gebhardt
// This file is part of the "Irrlicht Engine".
// For conditions of distribution and use, see copyright notice in irrlicht.h

#ifndef BASERESOURCEFILE_H
#define BASERESOURCEFILE_H

#include "BaseFileArchive.h"

/*
	The resource is a unique asset which use the name as identifier.
	Two phases are involved in using a resource cache: creating the resource
	and using it. If the resource is already loaded in cache, accessing it
	will be fast otherwise there is a cache miss and it will have to load the
	resource file.
*/
class BaseResource
{
public:
	eastl::wstring mName;
	BaseResource(const eastl::wstring &name);
};


/*
	Resource file provides access to resource.
*/
class BaseResourceFile
{
public:
	virtual bool Open() = 0;
	virtual int GetRawResource(const BaseResource &r, void** buffer) = 0;
	virtual int GetNumResources() const = 0;
	virtual eastl::wstring GetResourceName(unsigned int num) const = 0;
	virtual bool IsUsingDevelopmentDirectories(void) const = 0;

	//! determines if a file exists and would be able to be opened.
	virtual bool ExistFile(const eastl::wstring& filename) const = 0;

	//! determines if a directory exists and would be able to be opened.
	virtual bool ExistDirectory(const eastl::wstring& dirname) const = 0;

	//! Check if the file might be loaded by this class
	/** Check is based on the file extension (e.g. ".tga")
	\param filename Name of file to check.
	\return True if file seems to be loadable. */
	virtual bool IsALoadableFileFormat(const eastl::wstring& filename) const = 0;

	//! Check if the file might be loaded by this class
	/** Check might look into the file.
	\param enum FILE_ARCHIVE_TYPE FileArchive type to check.
	\return True if file seems to be loadable. */
	virtual bool IsALoadableFileFormat(FileArchiveType fileType) const = 0;

	//! Check if the file might be loaded by this class
	/** Check might look into the file.
	\param file File handle to check.
	\return True if file seems to be loadable. */
	virtual bool IsALoadableFileFormat(BaseReadFile* file) const = 0;
};

#endif

