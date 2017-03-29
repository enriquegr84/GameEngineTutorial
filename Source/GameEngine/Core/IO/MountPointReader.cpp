// Copyright (C) 2002-2012 Nikolaus Gebhardt
// This file is part of the "Irrlicht Engine".
// For conditions of distribution and use, see copyright notice in irrlicht.h

#include "MountPointReader.h"

#include "ReadFile.h"

#include "Application/GameApplication.h"

//! Constructor
ResourceMountPointFile::ResourceMountPointFile(const eastl::wstring resFileName)
{ 
	mMountPointFile.reset(); 
	mResFileName = resFileName; 
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

	GameApplication* gameApp = (GameApplication*)Application::App;
	FileSystem* fileSystem = gameApp->mFileSystem.get();
	BaseFileSystemType current = fileSystem->SetFileSystemType(FILESYSTEM_NATIVE);

	const eastl::wstring save = fileSystem->GetWorkingDirectory();
	eastl::wstring fullPath = fileSystem->GetAbsolutePath(filename);

	if (fileSystem->ChangeWorkingDirectoryTo(fullPath))
		ret = true;

	fileSystem->ChangeWorkingDirectoryTo(save);
	fileSystem->SetFileSystemType(current);

	return ret;
}

//! Check to see if the loader can create archives of this type.
bool ResourceMountPointFile::IsALoadableFileFormat(FileArchiveType fileType) const
{
	return fileType == FAT_FOLDER;
}

//! Check if the file might be loaded by this class
bool ResourceMountPointFile::IsALoadableFileFormat(BaseReadFile* file) const
{
	return false;
}

bool ResourceMountPointFile::ExistFile(const eastl::wstring& filename) const
{
	if (mMountPointFile->GetFileList()->FindFile(filename) != -1)
		return true;

	return false;
}

bool ResourceMountPointFile::ExistDirectory(const eastl::wstring& dir) const
{
	if (mMountPointFile->GetFileList()->FindFile(dir,true) != -1)
		return true;

	return false;
}


bool ResourceMountPointFile::Open()
{
	mMountPointFile.reset();
	bool ignoreCase = true;
	bool ignorePaths = false;

	GameApplication* gameApp = (GameApplication*)Application::App;
	FileSystem* fileSystem = gameApp->mFileSystem.get();
	if (IsALoadableFileFormat(mResFileName))
	{
		mMountPointFile.reset(dynamic_cast<MountPointReader*>(
			fileSystem->CreateMountPointFileArchive(mResFileName, ignoreCase, ignorePaths)));
		return true;
	}

	return false;
/*
	mMountPointFile = new PakFile;
    if (mMountPointFile)
    {
		return mMountPointFile->Init(mResFileName.c_str());
	}
	return false;	
*/
}

int ResourceMountPointFile::GetRawResource(const BaseResource &r, void** buffer)
{
	int size = 0;
	BaseReadFile* file = mMountPointFile->CreateAndOpenFile(r.mName);
	if (file)
	{
		size = file->GetSize();
		*buffer = file;
	}

	return size;

/*
	optional<int> resourceNum = mMountPointFile->Find(r.mName.c_str());
	if (resourceNum.valid())
	{
		size = mMountPointFile->GetFileLen(*resourceNum);
		mMountPointFile->ReadFile(*resourceNum, buffer);
	}
	return size;	
*/
}


int ResourceMountPointFile::GetNumResources() const 
{ 
	return (mMountPointFile) ? mMountPointFile->GetFileCount() : 0;
}


eastl::wstring ResourceMountPointFile::GetResourceName(unsigned int num) const 
{ 
	eastl::wstring resName = L"";
	if (mMountPointFile && num >= 0 && num < mMountPointFile->GetFileCount())
		resName = mMountPointFile->GetFullFileName(num); 

	return resName;
}


//! compatible Folder Architecture
MountPointReader::MountPointReader(const eastl::wstring& basename, bool ignoreCase, bool ignorePaths)
	: FileList(basename, ignoreCase, ignorePaths)
{
	//! ensure CFileList path ends in a slash
	if (mFileListPath[mFileListPath.size() - 1] != '/') 
		mFileListPath += '/';

	GameApplication* gameApp = (GameApplication*)Application::App;
	FileSystem* fileSystem = gameApp->mFileSystem.get();
	const eastl::wstring& work = fileSystem->GetWorkingDirectory();

	fileSystem->ChangeWorkingDirectoryTo(basename);
	BuildDirectory();
	fileSystem->ChangeWorkingDirectoryTo(work);

	Sort();
}


//! returns the list of files
const BaseFileList* MountPointReader::GetFileList()
{
	return this;
}

void MountPointReader::BuildDirectory()
{
	GameApplication* gameApp = (GameApplication*)Application::App;
	FileSystem* fileSystem = gameApp->mFileSystem.get();
	BaseFileList * list = fileSystem->CreateFileList();
	if (!list)
		return;

	const unsigned int size = list->GetFileCount();
	for (unsigned int i=0; i < size; ++i)
	{
		eastl::wstring full = list->GetFullFileName(i);
		full = full.substr(mFileListPath.size(), full.size() - mFileListPath.size());

		if (!list->IsDirectory(i))
		{
			AddItem(full, list->GetFileOffset(i), list->GetFileSize(i), false, mRealFileNames.size());
			mRealFileNames.push_back(list->GetFullFileName(i));
		}
		else
		{
			const eastl::wstring rel = list->GetFileName(i);
			mRealFileNames.push_back(list->GetFullFileName(i));

			eastl::wstring pwd  = fileSystem->GetWorkingDirectory();
			if (pwd[pwd.size() - 1] != '/') pwd += '/';
			pwd.append(rel);

			if ( rel != L"." && rel != L".." )
			{
				AddItem(full, 0, 0, true, 0);
				fileSystem->ChangeWorkingDirectoryTo(pwd);
				BuildDirectory();
				fileSystem->ChangeWorkingDirectoryTo(L"..");
			}
		}
	}

	delete list;
}

//! opens a file by index
BaseReadFile* MountPointReader::CreateAndOpenFile(unsigned int index)
{
	if (index >= mFiles.size())
		return 0;

	return ReadFile::CreateReadFile(mRealFileNames[mFiles[index].mID]);
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