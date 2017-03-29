// Copyright (C) 2002-2012 Nikolaus Gebhardt
// This file is part of the "Irrlicht Engine".
// For conditions of distribution and use, see copyright notice in irrlicht.h

#include "FileList.h"

static const eastl::wstring emptyFileListEntry;

FileList::FileList(const eastl::wstring& filepath, bool ignoreCase, bool ignorePaths)
 : mIgnorePaths(ignorePaths), mIgnoreCase(ignoreCase), mFileListPath(filepath)
{
	eastl::replace(mFileListPath.begin(), mFileListPath.end(), '\\', '/');
}

FileList::~FileList()
{
	mFiles.clear();
}

unsigned int FileList::GetFileCount() const
{
	return mFiles.size();
}

void FileList::Sort()
{
	//Files.sort();
}

const eastl::wstring& FileList::GetFileName(unsigned int index) const
{
	if (index >= mFiles.size())
		return emptyFileListEntry;

	return mFiles[index].mName;
}


//! Gets the full name of a file in the list, path included, based on an index.
const eastl::wstring& FileList::GetFullFileName(unsigned int index) const
{
	if (index >= mFiles.size())
		return emptyFileListEntry;

	return mFiles[index].mFullName;
}

//! adds a file or folder
unsigned int FileList::AddItem(const eastl::wstring& fullPath, 
	unsigned int offset, unsigned int size, bool isDirectory, unsigned int id)
{
	FileListEntry entry;
	entry.mID   = id ? id : mFiles.size();
	entry.mOffset = offset;
	entry.mSize = size;
	entry.mName = fullPath;
	eastl::replace(entry.mName.begin(), entry.mName.end(), '\\', '/');
	entry.mIsDirectory = isDirectory;

	// remove trailing slash
	if (entry.mName[entry.mName.size()-1] == '/')
	{
		entry.mIsDirectory = true;
		entry.mName[entry.mName.size()-1] = 0;
		entry.mName.validate();
	}

	if (mIgnoreCase)
		entry.mName.make_lower();

	entry.mFullName = entry.mName;

	if (mIgnorePaths)
		entry.mFullName = entry.mName;

	if (entry.mName.rfind('/') != eastl::string::npos)
		entry.mName = entry.mName.substr(entry.mName.rfind('/') + 1);

	//LogInformation(m_Path.c_str() entry.m_FullName);
	mFiles.push_back(entry);

	return mFiles.size() - 1;
}

//! Returns the ID of a file in the file list, based on an index.
unsigned int FileList::GetID(unsigned int index) const
{
	return index < mFiles.size() ? mFiles[index].mID : 0;
}

bool FileList::IsDirectory(unsigned int index) const
{
	bool ret = false;
	if (index < mFiles.size())
		ret = mFiles[index].mIsDirectory;

	return ret;
}

//! Returns the size of a file
unsigned int FileList::GetFileSize(unsigned int index) const
{
	return index < mFiles.size() ? mFiles[index].mSize : 0;
}

//! Returns the size of a file
unsigned int FileList::GetFileOffset(unsigned int index) const
{
	return index < mFiles.size() ? mFiles[index].mOffset : 0;
}


//! Searches for a file or folder within the list, returns the index
int FileList::FindFile(const eastl::wstring& filename, bool isDirectory = false) const
{
	FileListEntry entry;
	// we only need FullName to be set for the search
	entry.mFullName = filename;
	entry.mIsDirectory = isDirectory;

	// exchange
	eastl::replace(entry.mFullName.begin(), entry.mFullName.end(), '\\', '/');

	// remove trailing slash
	if (entry.mFullName[entry.mFullName.size()-1] == '/')
	{
		entry.mIsDirectory = true;
		entry.mFullName[entry.mFullName.size()-1] = 0;
		entry.mFullName.validate();
	}

	if (isDirectory && !entry.mIsDirectory)
		return -1;

	if (mIgnoreCase)
		entry.mFullName.make_lower();

	if (mIgnorePaths)
		entry.mFullName = entry.mFullName.substr(entry.mFullName.rfind('/') + 1);


	for (unsigned int i=0; i < (unsigned int)mFiles.size(); i++)
	{
		FileListEntry fentry = mFiles[i];
		if (isDirectory)
		{
			if (fentry.mIsDirectory)
				if (fentry == entry) return i;
		}
		else	
		{
			if (!fentry.mIsDirectory)
				if (fentry == entry) return i;
		}
	}

	return -1;
}


//! Returns the base path of the file list
const eastl::wstring& FileList::GetPath() const
{
	return mFileListPath;
}