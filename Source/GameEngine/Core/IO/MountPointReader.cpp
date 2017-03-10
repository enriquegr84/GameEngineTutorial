// Copyright (C) 2002-2012 Nikolaus Gebhardt
// This file is part of the "Irrlicht Engine".
// For conditions of distribution and use, see copyright notice in irrlicht.h

#include "MountPointReader.h"

#include "ReadFile.h"

//! Constructor
ResourceMountPointFile::ResourceMountPointFile(BaseFileSystem* fs, const eastl::wstring resFileName)
	: mFileSystem(fs)
{ 
	m_pMountPointFile.reset(); 
	m_resFileName = resFileName; 
}

//! returns true if the file maybe is able to be loaded by this class
bool ResourceMountPointFile::IsALoadableFileFormat(const eastl::wstring& filename) const
{
	bool ret = false;
	eastl::wstring fname(filename);

	// delete path from filename
	if (fname.rfind('/') != eastl::string::npos)
		fname = fname.substr(fname.rfind('/')+1);

	if (!fname.size())
		return ret;

	E_FILESYSTEM_TYPE current = mFileSystem->SetFileSystemType(FILESYSTEM_NATIVE);

	const eastl::wstring save = mFileSystem->GetWorkingDirectory();
	eastl::wstring fullPath = mFileSystem->GetAbsolutePath(filename);

	if (mFileSystem->ChangeWorkingDirectoryTo(fullPath))
		ret = true;

	mFileSystem->ChangeWorkingDirectoryTo(save);
	mFileSystem->SetFileSystemType(current);

	return ret;
}

//! Check to see if the loader can create archives of this type.
bool ResourceMountPointFile::IsALoadableFileFormat(E_FILE_ARCHIVE_TYPE fileType) const
{
	return fileType == EFAT_FOLDER;
}

//! Check if the file might be loaded by this class
bool ResourceMountPointFile::IsALoadableFileFormat(BaseReadFile* file) const
{
	return false;
}

bool ResourceMountPointFile::ExistFile(const eastl::wstring& filename) const
{
	if (m_pMountPointFile->GetFileList()->FindFile(filename) ! = -1)
		return true;

	return false;
}

bool ResourceMountPointFile::ExistDirectory(const eastl::wstring& dir) const
{
	if (m_pMountPointFile->GetFileList()->FindFile(dir,true)!=-1)
		return true;

	return false;
}


bool ResourceMountPointFile::Open()
{
	m_pMountPointFile.reset();
	bool ignoreCase = true;
	bool ignorePaths = false;

	if (IsALoadableFileFormat(m_resFileName))
	{
		m_pMountPointFile.reset(dynamic_cast<MountPointReader*>(
			mFileSystem->CreateMountPointFileArchive(m_resFileName, ignoreCase, ignorePaths)));
		return true;
	}

	return false;
/*
	m_pMountPointFile = new PakFile;
    if (m_pMountPointFile)
    {
		return m_pMountPointFile->Init(m_resFileName.c_str());
	}
	return false;	
*/
}

int ResourceMountPointFile::GetRawResource(const BaseResource &r, void** buffer)
{
	int size = 0;
	BaseReadFile* file = m_pMountPointFile->CreateAndOpenFile(r.m_name);
	if (file)
	{
		size = file->GetSize();
		*buffer = file;
	}

	return size;

/*
	optional<int> resourceNum = m_pMountPointFile->Find(r.m_name.c_str());
	if (resourceNum.valid())
	{
		size = m_pMountPointFile->GetFileLen(*resourceNum);
		m_pMountPointFile->ReadFile(*resourceNum, buffer);
	}
	return size;	
*/
}


int ResourceMountPointFile::GetNumResources() const 
{ 
	return (m_pMountPointFile) ? m_pMountPointFile->GetFileCount() : 0;
}


eastl::wstring ResourceMountPointFile::GetResourceName(unsigned int num) const 
{ 
	eastl::wstring resName = L"";
	if (m_pMountPointFile && num >= 0 && num < m_pMountPointFile->GetFileCount())
		resName = m_pMountPointFile->GetFullFileName(num); 

	return resName;
}


//! compatible Folder Architecture
MountPointReader::MountPointReader(BaseFileSystem* fs, const eastl::wstring& basename, 
	bool ignoreCase, bool ignorePaths)
	: mFileSystem(fs), FileList(basename, ignoreCase, ignorePaths)
{
	//! ensure CFileList path ends in a slash
	if (m_FileListPath[m_FileListPath.size() - 1] != '/') 
		m_FileListPath += '/';

	const eastl::wstring& work = mFileSystem->GetWorkingDirectory();

	mFileSystem->ChangeWorkingDirectoryTo(basename);
	BuildDirectory();
	mFileSystem->ChangeWorkingDirectoryTo(work);

	Sort();
}


//! returns the list of files
const BaseFileList* MountPointReader::GetFileList()
{
	return this;
}

void MountPointReader::BuildDirectory()
{
	BaseFileList * list = mFileSystem->CreateFileList();
	if (!list)
		return;

	const unsigned int size = list->GetFileCount();
	for (unsigned int i=0; i < size; ++i)
	{
		eastl::wstring full = list->GetFullFileName(i);
		full = full.substr(m_FileListPath.size(), full.size() - m_FileListPath.size());

		if (!list->IsDirectory(i))
		{
			AddItem(full, list->GetFileOffset(i), list->GetFileSize(i), false, m_RealFileNames.size());
			m_RealFileNames.push_back(list->GetFullFileName(i));
		}
		else
		{
			const eastl::wstring rel = list->GetFileName(i);
			m_RealFileNames.push_back(list->GetFullFileName(i));

			eastl::wstring pwd  = mFileSystem->GetWorkingDirectory();
			if (pwd[pwd.size() - 1] != '/') pwd += '/';
			pwd.append(rel);

			if ( rel != L"." && rel != L".." )
			{
				AddItem(full, 0, 0, true, 0);
				mFileSystem->ChangeWorkingDirectoryTo(pwd);
				BuildDirectory();
				mFileSystem->ChangeWorkingDirectoryTo(L"..");
			}
		}
	}

	delete list;
}

//! opens a file by index
BaseReadFile* MountPointReader::CreateAndOpenFile(unsigned int index)
{
	if (index >= m_Files.size())
		return 0;

	return ReadFile::CreateReadFile(m_RealFileNames[m_Files[index].m_ID]);
}

//! opens a file by file name
BaseReadFile* MountPointReader::CreateAndOpenFile(const eastl::wstring& filename)
{
	int index = FindFile(filename, false);
	if (index != -1)
		return CreateAndOpenFile(index);
	else
		return 0;
}