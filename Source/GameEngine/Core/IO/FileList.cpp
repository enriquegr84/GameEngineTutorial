// Copyright (C) 2002-2012 Nikolaus Gebhardt
// This file is part of the "Irrlicht Engine".
// For conditions of distribution and use, see copyright notice in irrlicht.h

#include "FileList.h"

static const eastl::wstring emptyFileListEntry;

FileList::FileList(const eastl::wstring& filepath, bool ignoreCase, bool ignorePaths)
 : m_IgnorePaths(ignorePaths), m_IgnoreCase(ignoreCase), m_FileListPath(filepath)
{
	eastl::replace(m_FileListPath.begin(), m_FileListPath.end(), '\\', '/');
}

FileList::~FileList()
{
	m_Files.clear();
}

unsigned int FileList::GetFileCount() const
{
	return m_Files.size();
}

void FileList::Sort()
{
	//Files.sort();
}

const eastl::wstring& FileList::GetFileName(unsigned int index) const
{
	if (index >= m_Files.size())
		return emptyFileListEntry;

	return m_Files[index].m_Name;
}


//! Gets the full name of a file in the list, path included, based on an index.
const eastl::wstring& FileList::GetFullFileName(unsigned int index) const
{
	if (index >= m_Files.size())
		return emptyFileListEntry;

	return m_Files[index].m_FullName;
}

//! adds a file or folder
unsigned int FileList::AddItem(const eastl::wstring& fullPath, 
	unsigned int offset, unsigned int size, bool isDirectory, unsigned int id)
{
	FileListEntry entry;
	entry.m_ID   = id ? id : m_Files.size();
	entry.m_Offset = offset;
	entry.m_Size = size;
	entry.m_Name = fullPath;
	eastl::replace(entry.m_Name.begin(), entry.m_Name.end(), '\\', '/');
	entry.m_IsDirectory = isDirectory;

	// remove trailing slash
	if (entry.m_Name[entry.m_Name.size()-1] == '/')
	{
		entry.m_IsDirectory = true;
		entry.m_Name[entry.m_Name.size()-1] = 0;
		entry.m_Name.validate();
	}

	if (m_IgnoreCase)
		entry.m_Name.make_lower();

	entry.m_FullName = entry.m_Name;

	if (m_IgnorePaths)
		entry.m_FullName = entry.m_Name;

	if (entry.m_Name.rfind('/') != eastl::string::npos)
		entry.m_Name = entry.m_Name.substr(entry.m_Name.rfind('/') + 1);

	LogInformation(m_Path.c_str(), entry.m_FullName);
	m_Files.push_back(entry);

	return m_Files.size() - 1;
}

//! Returns the ID of a file in the file list, based on an index.
unsigned int FileList::GetID(unsigned int index) const
{
	return index < m_Files.size() ? m_Files[index].m_ID : 0;
}

bool FileList::IsDirectory(unsigned int index) const
{
	bool ret = false;
	if (index < m_Files.size())
		ret = m_Files[index].m_IsDirectory;

	return ret;
}

//! Returns the size of a file
unsigned int FileList::GetFileSize(unsigned int index) const
{
	return index < m_Files.size() ? m_Files[index].m_Size : 0;
}

//! Returns the size of a file
unsigned int FileList::GetFileOffset(unsigned int index) const
{
	return index < m_Files.size() ? m_Files[index].m_Offset : 0;
}


//! Searches for a file or folder within the list, returns the index
int FileList::FindFile(const eastl::wstring& filename, bool isDirectory = false) const
{
	FileListEntry entry;
	// we only need FullName to be set for the search
	entry.m_FullName = filename;
	entry.m_IsDirectory = isDirectory;

	// exchange
	eastl::replace(entry.m_FullName.begin(), entry.m_FullName.end(), '\\', '/');

	// remove trailing slash
	if (entry.m_FullName[entry.m_FullName.size()-1] == '/')
	{
		entry.m_IsDirectory = true;
		entry.m_FullName[entry.m_FullName.size()-1] = 0;
		entry.m_FullName.validate();
	}

	if (isDirectory && !entry.m_IsDirectory)
		return -1;

	if (m_IgnoreCase)
		entry.m_FullName.make_lower();

	if (m_IgnorePaths)
		entry.m_FullName = entry.m_FullName.substr(entry.m_FullName.rfind('/') + 1);


	for (unsigned int i=0; i < (unsigned int)m_Files.size(); i++)
	{
		FileListEntry fentry = m_Files[i];
		if (isDirectory)
		{
			if (fentry.m_IsDirectory)
				if (fentry == entry) return i;
		}
		else	
		{
			if (!fentry.m_IsDirectory)
				if (fentry == entry) return i;
		}
	}

	return -1;
}


//! Returns the base path of the file list
const eastl::wstring& FileList::GetPath() const
{
	return m_FileListPath;
}