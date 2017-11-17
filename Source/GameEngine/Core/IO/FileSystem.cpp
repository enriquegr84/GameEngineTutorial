// Copyright (C) 2002-2012 Nikolaus Gebhardt
// This file is part of the "Irrlicht Engine".
// For conditions of distribution and use, see copyright notice in irrlicht.h

#include "FileSystem.h"
#include "FileList.h"

#include "MemoryFile.h"
#include "LimitReadFile.h"
#include "MountPointReader.h"

#include "Core/Logger/Logger.h"

#if !defined (_WINDOWS_API_)
	#if (defined(_POSIX_API_) || defined(_OSX_PLATFORM_))
		#include <stdio.h>
		#include <stdlib.h>
		#include <string.h>
		#include <limits.h>
		#include <sys/types.h>
		#include <dirent.h>
		#include <sys/stat.h>
		#include <unistd.h>
	#endif
#endif

FileSystem* FileSystem::mFileSystem = NULL;

FileSystem* FileSystem::Get(void)
{
	LogAssert(FileSystem::mFileSystem, "Filesystem doesn't exist");
	return FileSystem::mFileSystem;
}

//! constructor
FileSystem::FileSystem()
{
	SetFileSystemType(FILESYSTEM_NATIVE);
	//! reset current working directory
	GetWorkingDirectory();

	if (FileSystem::mFileSystem)
	{
		LogError("Attempting to create two global filesystem! \
					The old one will be destroyed and overwritten with this one.");
		delete FileSystem::mFileSystem;
	}

	FileSystem::mFileSystem = this;

	msDirectories = new eastl::vector<eastl::string>();
}

//! destructor
FileSystem::~FileSystem()
{
	if (FileSystem::mFileSystem == this)
		FileSystem::mFileSystem = nullptr;

	if (msDirectories)
	{
		delete(msDirectories);
		return;
	}

	LogError("No directory list to deallocate.\n");
}

//! Creates an ReadFile interface for treating memory like a file.
BaseReadFile* FileSystem::CreateMemoryReadFile(const void* memory, int len, 
	const eastl::wstring& fileName, bool deleteMemoryWhenDropped)
{
	if (memory)
		return new MemoryReadFile(memory, len, fileName, deleteMemoryWhenDropped);
	
	return 0;
}


//! Creates an ReadFile interface for reading files inside files
BaseReadFile* FileSystem::CreateLimitReadFile(const eastl::wstring& fileName, 
	BaseReadFile* alreadyOpenedFile, long pos, long areaSize)
{
	if (alreadyOpenedFile)
		return new LimitReadFile(alreadyOpenedFile, pos, areaSize, fileName);

	return 0;
}

//! Creates a list of files and directories in the current working directory
BaseFileList* FileSystem::CreateFileList()
{
	BaseFileList* r = 0;
	eastl::wstring filesPath = GetWorkingDirectory();
	eastl::replace(filesPath.begin(), filesPath.end(), '\\', '/');
	if (filesPath[filesPath.size() - 1] != '/') filesPath += '/';

	//! Construct from native filesystem
	if (mFileSystemType == FILESYSTEM_NATIVE)
	{
		// --------------------------------------------
		//! Windows version
		#ifdef _WINDOWS_API_
		#if !defined ( _WIN32_WCE )

		r = new FileList(filesPath, true, false);

		// TODO: Should be unified once mingw adapts the proper types
#if defined(__GNUC__)
		long hFile; //mingw return type declaration
#else
		intptr_t hFile;
#endif

		struct _tfinddata_t c_file;
		if( (hFile = _tfindfirst( _T("*"), &c_file )) != -1L )
		{
			do
			{
				r->AddItem(
					filesPath + c_file.name, 0, c_file.size, (_A_SUBDIR & c_file.attrib) != 0, 0);
			}
			while( _tfindnext( hFile, &c_file ) == 0 );

			_findclose( hFile );
		}
		#endif

		#endif

		// --------------------------------------------
		//! Linux version
		#if (defined(_POSIX_API_) || defined(_OSX_PLATFORM_))

		r = new FileList(FilesPath, false, false);
		r->addItem(FilesPath + _GE_TEXT(".."), 0, 0, true, 0);

		//! We use the POSIX compliant methods instead of scandir
		DIR* dirHandle=opendir(FilesPath.c_str());
		if (dirHandle)
		{
			struct dirent *dirEntry;
			while ((dirEntry=readdir(dirHandle)))
			{
				unsigned int size = 0;
				bool isDirectory = false;

				if((strcmp(dirEntry->d_name, ".")==0) ||
				   (strcmp(dirEntry->d_name, "..")==0))
				{
					continue;
				}
				struct stat buf;
				if (stat(dirEntry->d_name, &buf)==0)
				{
					size = buf.st_size;
					isDirectory = S_ISDIR(buf.st_mode);
				}
				#if !defined(_SOLARIS_PLATFORM_) && !defined(__CYGWIN__)
				// only available on some systems
				else
				{
					isDirectory = dirEntry->d_type == DT_DIR;
				}
				#endif

				r->addItem(FilesPath + dirEntry->d_name, 0, size, isDirectory, 0);
			}
			closedir(dirHandle);
		}
		#endif
	}
	else
	{
		//! create file list for the virtual filesystem
		r = new FileList(filesPath, false, false);

		//! add relative navigation
		FileListEntry e2;
		FileListEntry e3;

		//! PWD
		r->AddItem(filesPath + L".", 0, 0, true, 0);

		//! parent
		r->AddItem(filesPath + L"..", 0, 0, true, 0);
	}

	if (r)
		r->Sort();
	return r;
}

//! Creates an empty filelist
BaseFileList* FileSystem::CreateEmptyFileList(const eastl::wstring& filesPath, bool ignoreCase, bool ignorePaths)
{
	return new FileList(filesPath, ignoreCase, ignorePaths);
}

//! Creates an archive file.
BaseFileArchive* FileSystem::CreateMountPointFileArchive(const eastl::wstring& filename, bool ignoreCase, bool ignorePaths)
{
	BaseFileArchive *archive = 0;
	BaseFileSystemType current = SetFileSystemType(FILESYSTEM_NATIVE);

	const eastl::wstring save = GetWorkingDirectory();
	eastl::wstring fullPath = GetAbsolutePath(filename);

	if (ChangeWorkingDirectoryTo(fullPath))
		archive = new MountPointReader(fullPath, ignoreCase, ignorePaths);

	ChangeWorkingDirectoryTo(save);
	SetFileSystemType(current);

	return archive;
}

//! Returns the string of the current working directory
const eastl::wstring& FileSystem::GetWorkingDirectory()
{
	BaseFileSystemType type = mFileSystemType;

	if (type != FILESYSTEM_NATIVE)
	{
		type = FILESYSTEM_VIRTUAL;
	}
	else
	{
		#if defined(_WINDOWS_API_)
			wchar_t tmp[_MAX_PATH];
			_wgetcwd(tmp, _MAX_PATH);
			mWorkingDirectory[FILESYSTEM_NATIVE] = tmp;
			eastl::replace(
				mWorkingDirectory[FILESYSTEM_NATIVE].begin(), 
				mWorkingDirectory[FILESYSTEM_NATIVE].end(), L'\\', L'/');
		#endif

		#if (defined(_POSIX_API_) || defined(_OSX_PLATFORM_))

			// getting the CWD is rather complex as we do not know the size
			// so try it until the call was successful
			// Note that neither the first nor the second parameter may be 0 according to POSIX
			unsigned int pathSize=256;
			wchar_t *tmpPath = new wchar_t[pathSize];
			while ((pathSize < (1<<16)) && !(wgetcwd(tmpPath,pathSize)))
			{
				SAFE_DELETE_ARRAY( tmpPath );
				pathSize *= 2;
				tmpPath = new char[pathSize];
			}
			if (tmpPath)
			{
				WorkingDirectory[FILESYSTEM_NATIVE] = tmpPath;
				delete[] tmpPath;
			}
		#endif

		mWorkingDirectory[type].validate();
	}

	return mWorkingDirectory[type];
}

//! Changes the current Working Directory to the given string.
bool FileSystem::ChangeWorkingDirectoryTo(const eastl::wstring& newDirectory)
{
	bool success=false;

	if (mFileSystemType != FILESYSTEM_NATIVE)
	{
		mWorkingDirectory[FILESYSTEM_VIRTUAL] = newDirectory;
		success = true;
	}
	else
	{
		mWorkingDirectory[FILESYSTEM_NATIVE] = newDirectory;
		success = (_wchdir(newDirectory.c_str()) == 0);
	}

	return success;
}

eastl::wstring FileSystem::GetAbsolutePath(const eastl::wstring& filename) const
{
#if defined(_WINDOWS_API_)
	wchar_t *p=0;
	wchar_t fpath[_MAX_PATH];
	p = _wfullpath(fpath, filename.c_str(), _MAX_PATH);
	eastl::wstring tmp(p);
	eastl::replace(tmp.begin(), tmp.end(), L'\\', L'/');
	return tmp;
#elif (defined(_POSIX_API_) || defined(_OSX_PLATFORM_))
	wchar_t* p=0;
	wchar_t fpath[4096];
	fpath[0]=0;
	p = realpath(filename.c_str(), fpath);
	if (!p)
	{
		// content in fpath is unclear at this point
		if (!fpath[0]) // seems like fpath wasn't altered, use our best guess
		{
			path tmp(filename);
			return tmp
		}
		else
			return eastl::wstring(fpath);
	}
	if (filename[filename.size()-1]=='/')
		return eastl::wstring(p)+ L"/";
	else
		return eastl::wstring(p);
#else
	return eastl::wstring(filename);
#endif
}

//! returns the directory part of a filename, i.e. all until the first
//! slash or backslash, excluding it. If no directory path is prefixed, a '.'
//! is returned.
eastl::wstring FileSystem::GetFileDir(const eastl::wstring& filename) const
{
	// find last forward or backslash
	if (filename.rfind('/') != eastl::string::npos)
		return filename.substr(0, filename.rfind('/'));
	else 
		return L".";
}


//----------------------------------------------------------------------------
bool FileSystem::InsertDirectory(const eastl::string& directory)
{
	if (!msDirectories)
		msDirectories = new eastl::vector<eastl::string>();

	eastl::vector<eastl::string>::iterator iter = msDirectories->begin();
	eastl::vector<eastl::string>::iterator end = msDirectories->end();
	for (/**/; iter != end; ++iter)
	{
		if (directory == *iter)
		{
			return false;
		}
	}
	msDirectories->push_back(directory);
	return true;
}
//----------------------------------------------------------------------------
bool FileSystem::RemoveDirectory(const eastl::string& directory)
{
	if (!msDirectories)
		msDirectories = new eastl::vector<eastl::string>();

	eastl::vector<eastl::string>::iterator iter = msDirectories->begin();
	eastl::vector<eastl::string>::iterator end = msDirectories->end();
	for (/**/; iter != end; ++iter)
	{
		if (directory == *iter)
		{
			msDirectories->erase(iter);
			return true;
		}
	}
	return false;
}
//----------------------------------------------------------------------------
void FileSystem::RemoveAllDirectories()
{
	if (!msDirectories)
		msDirectories = new eastl::vector<eastl::string>();

	msDirectories->clear();
}
//----------------------------------------------------------------------------
eastl::string FileSystem::GetPath(const eastl::string& fileName)
{
	if (!msDirectories)
		msDirectories = new eastl::vector<eastl::string>();

	eastl::vector<eastl::string>::iterator iter = msDirectories->begin();
	eastl::vector<eastl::string>::iterator end = msDirectories->end();
	for (/**/; iter != end; ++iter)
	{
		eastl::string decorated = *iter + fileName;
		if (access(decorated.c_str(), 0) != -1)
		{
			return decorated;
		}
	}

	LogError("File not found : " + fileName);
	return eastl::string();
}
//----------------------------------------------------------------------------

//! determines if a directory exists and would be able to be opened.
bool FileSystem::ExistDirectory(const eastl::wstring& dirname) const
{
	return false;
}

//! determines if a file exists and would be able to be opened.
bool FileSystem::ExistFile(const eastl::wstring& filename) const
{
	return false;
}

//! Get the current file systen type
BaseFileSystemType FileSystem::GetFileSystemType( )
{
	return mFileSystemType;
}

//! Sets the current file systen type
BaseFileSystemType FileSystem::SetFileSystemType(BaseFileSystemType listType)
{
	BaseFileSystemType current = mFileSystemType;
	mFileSystemType = listType;
	return current;
}


// Returns a list of files in a given directory.
void FileSystem::ListFiles(eastl::set<eastl::wstring>& result, 
	const eastl::wstring& dir, bool makeFullPath)
{
    result.clear();
    eastl::wstring previousCWD = GetWorkingDirectory();

    if(!ChangeWorkingDirectoryTo( dir.c_str() ))
    {
        LogError("FileManager listFiles : Could not change CWD!");
        return;
    }

    BaseFileList* files = CreateFileList();
    for(int n=0; n<(int)files->GetFileCount(); n++)
    {
        result.insert(makeFullPath ? dir + L"/" + 
			files->GetFileName(n).c_str() : files->GetFileName(n).c_str());
    }

	ChangeWorkingDirectoryTo( previousCWD );
	delete files;
}   // listFiles