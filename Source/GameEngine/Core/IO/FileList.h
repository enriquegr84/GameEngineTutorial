// Copyright (C) 2002-2012 Nikolaus Gebhardt
// This file is part of the "Irrlicht Engine".
// For conditions of distribution and use, see copyright notice in irrlicht.h

#ifndef FILELIST_H
#define FILELIST_H

#include "GameEngineStd.h"

#include "BaseFileList.h"

//! An entry in a list of files, can be a folder or a file.
struct FileListEntry
{
	//! The name of the file
	/** If this is a file or folder in the virtual filesystem and the archive
	was created with the ignoreCase flag then the file name will be lower case. */
	eastl::wstring mName;

	//! The name of the file including the path
	/** If this is a file or folder in the virtual filesystem and the archive was
	created with the ignoreDirs flag then it will be the same as Name. */
	eastl::wstring mFullName;

	//! The size of the file in bytes
	unsigned int mSize;

	//! The ID of the file in an archive
	/** This is used to link the FileList entry to extra info held about this
	file in an archive, which can hold things like data offset and CRC. */
	unsigned int mID;

	//! FileOffset inside an archive
	unsigned int mOffset;

	//! True if this is a folder, false if not.
	bool mIsDirectory;

	//! The == operator is provided so that FileList can slowly search the list!
	bool operator ==(const struct FileListEntry& other) const
	{
		if (mIsDirectory != other.mIsDirectory)
			return false;

		return mFullName == other.mFullName;
	}

	//! The < operator is provided so that FileList can sort and quickly search the list.
	bool operator <(const struct FileListEntry& other) const
	{
		if (mIsDirectory != other.mIsDirectory)
			return mIsDirectory;

		return mFullName < other.mFullName;
	}
};


//! Implementation of a file list
class FileList : public BaseFileList
{
public:

	// FileList methods

	//! Constructor
	/** \param path The path of this file archive */
	FileList(const eastl::wstring& filepath, bool ignoreCase, bool ignorePaths);

	//! Destructor
	virtual ~FileList();

	//! Get the number of files in the filelist.
	/** \return Amount of files and directories in the file list. */
	virtual unsigned int GetFileCount() const;

	//! Gets the name of a file in the list, based on an index.
	/** The path is not included in this name. Use getFullFileName for this.
	\param index is the zero based index of the file which name should
	be returned. The index must be less than the amount getFileCount() returns.
	\return File name of the file. Returns 0, if an error occured. */
	virtual const eastl::wstring& GetFileName(unsigned int index) const;

	//! Gets the full name of a file in the list including the path, based on an index.
	/** \param index is the zero based index of the file which name should
	be returned. The index must be less than the amount getFileCount() returns.
	\return File name of the file. Returns 0 if an error occured. */
	virtual const eastl::wstring& GetFullFileName(unsigned int index) const;

	//! Returns the ID of a file in the file list, based on an index.
	/** This optional ID can be used to link the file list entry to information held
	elsewhere. For example this could be an index in an BaseFileArchive, linking the entry
	to its data offset, uncompressed size and CRC.
	\param index is the zero based index of the file which should be returned.
	The index must be less than the amount getFileCount() returns.
	\return The ID of the file. */
	virtual unsigned int GetID(unsigned int index) const;

	//! Check if the file is a directory
	/** \param index The zero based index which will be checked. The index
	must be less than the amount getFileCount() returns.
	\return True if the file is a directory, else false. */
	virtual bool IsDirectory(unsigned int index) const;

	//! Returns the size of a file in the file list, based on an index.
	/** \param index is the zero based index of the file which should be returned.
	The index must be less than the amount getFileCount() returns.
	\return The size of the file in bytes. */
	virtual unsigned int GetFileSize(unsigned int index) const;

	//! Returns the file offset of a file in the file list, based on an index.
	/** \param index is the zero based index of the file which should be returned.
	The index must be less than the amount getFileCount() returns.
	\return The offset of the file in bytes. */
	virtual unsigned int GetFileOffset(unsigned int index) const;

	//! Searches for a file or folder in the list
	/** Searches for a file by name
	\param filename The name of the file to search for.
	\param isFolder True if you are searching for a directory path, false if you are searching for a file
	\return Returns the index of the file in the file list, or -1 if
	no matching name name was found. */
	virtual int FindFile(const eastl::wstring& filename, bool isFolder) const;

	//! Returns the base path of the file list
	virtual const eastl::wstring& GetPath() const;

	//! Add as a file or folder to the list
	/** \param fullPath The file name including path, up to the root of the file list.
	\param isDirectory True if this is a directory rather than a file.
	\param offset The offset where the file is stored in an archive
	\param size The size of the file in bytes.
	\param id The ID of the file in the archive which owns it */
	virtual unsigned int AddItem(const eastl::wstring& fullPath, 
		unsigned int offset, unsigned int size, bool isDirectory, unsigned int id=0);

	//! Sorts the file list. You should call this after adding any items to the file list
	virtual void Sort();

protected:

	//! Ignore paths when adding or searching for files
	bool mIgnorePaths;

	//! Ignore case when adding or searching for files
	bool mIgnoreCase;

	//! Path to the file list
	eastl::wstring mFileListPath;

	//! List of files
	eastl::vector<FileListEntry> mFiles;
};


#endif

