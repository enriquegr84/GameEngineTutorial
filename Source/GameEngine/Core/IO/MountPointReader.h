// Copyright (C) 2002-2012 Nikolaus Gebhardt
// This file is part of the "Irrlicht Engine".
// For conditions of distribution and use, see copyright notice in irrlicht.h

#ifndef MOUNTREADER_H
#define MOUNTREADER_H

#include "GameEngineStd.h"

#include "BaseFileSystem.h"
#include "BaseFileArchive.h"
#include "BaseResourceFile.h"
#include "BaseReadFile.h"
#include "FileList.h"

//! A File Archive which uses a mountpoint
class MountPointReader : public virtual BaseFileArchive, public virtual FileList
{
public:

	//! Constructor
	MountPointReader(BaseFileSystem* fs, const eastl::wstring& filename, 
		bool ignoreCase, bool ignorePaths);

	//! opens a file by index
	virtual BaseReadFile* CreateAndOpenFile(unsigned int index);

	//! opens a file by file name
	virtual BaseReadFile* CreateAndOpenFile(const eastl::wstring& filename);

	//! returns the list of files
	virtual const BaseFileList* GetFileList();

	//! get the class Type
	virtual E_FILE_ARCHIVE_TYPE GetType() const { return EFAT_FOLDER; }

	//! return the name (id) of the file Archive
	virtual const eastl::wstring& GetArchiveName() const {return m_FileListPath;}

private:

	BaseFileSystem* mFileSystem;
	eastl::vector<eastl::wstring> m_RealFileNames;

	void BuildDirectory();
};

//! Archiveloader capable of loading MountPoint Archives
class ResourceMountPointFile : public BaseResourceFile
{
public:

	ResourceMountPointFile(BaseFileSystem* fs, const eastl::wstring resFileName);

	virtual bool Open();
	virtual int GetRawResource(const BaseResource &r, void** buffer);
	virtual int GetNumResources() const;
	virtual eastl::wstring GetResourceName(unsigned int num) const;
    virtual bool IsUsingDevelopmentDirectories(void) const { return false; }

	//! determines if a file exists and would be able to be opened.
	virtual bool ExistFile(const eastl::wstring& filename) const;

	//! determines if a directory exists and would be able to be opened.
	virtual bool ExistDirectory(const eastl::wstring& dirname) const;

protected:

	//! returns true if the file maybe is able to be loaded by this class
	//! based on the file extension (e.g. ".zip")
	virtual bool IsALoadableFileFormat(const eastl::wstring& filename) const;

	//! Check if the file might be loaded by this class
	/** Check might look into the file.
	\param file File handle to check.
	\return True if file seems to be loadable. */
	virtual bool IsALoadableFileFormat(BaseReadFile* file) const;

	//! Check to see if the loader can create archives of this type.
	/** Check based on the archive type.
	\param fileType The archive type to check.
	\return True if the archile loader supports this type, false if not */
	virtual bool IsALoadableFileFormat(E_FILE_ARCHIVE_TYPE fileType) const;

private:

	BaseFileSystem* mFileSystem;
	eastl::wstring m_resFileName;
	eastl::shared_ptr<MountPointReader> m_pMountPointFile;
};

#endif
