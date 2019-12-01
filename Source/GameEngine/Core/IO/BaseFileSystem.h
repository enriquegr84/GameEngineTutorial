// Copyright (C) 2002-2012 Nikolaus Gebhardt
// This file is part of the "Irrlicht Engine".
// For conditions of distribution and use, see copyright notice in irrlicht.h

#ifndef BASEFILESYSTEM_H
#define BASEFILESYSTEM_H

#include "BaseReadFile.h"
#include "BaseFileList.h"
#include "BaseFileArchive.h"

//! BaseFileSystemType: which Filesystem should be used for e.g. browsing
enum CORE_ITEM BaseFileSystemType
{
	FILESYSTEM_NATIVE = 0,	// Native OS FileSystem
	FILESYSTEM_VIRTUAL	// Virtual FileSystem
};

/* 
	The FileSystem subsystem will generally be responsible for managing game files
	and archives and provides access to them. It manages where files are, so that 
	modules which use the the IO do not to know where every file is located. A file 
	could be in a compressed format or as file on disk, using the BaseFileSystem makes 
	no difference to this.
*/
class BaseFileSystem
{
public:

	//! Creates an BaseReadFile interface for accessing memory like a file.
	/** This allows you to use a pointer to memory where an BaseReadFile is requested.
	\param memory: A pointer to the start of the file in memory
	\param len: The length of the memory in bytes
	\param fileName: The name given to this file
	\param deleteMemoryWhenDropped: True if the memory should be deleted
	along with the BaseReadFile when it is dropped.
	\return Pointer to the created file interface.
	The returned pointer should be dropped when no longer needed.
	See IReferenceCounted::drop() for more information.
	*/
	virtual BaseReadFile* CreateMemoryReadFile(const void* memory, int len, 
		const eastl::wstring& fileName, bool deleteMemoryWhenDropped = false) = 0;

	//! Creates an BaseReadFile interface for accessing files inside files.
	/** This is useful e.g. for archives.
	\param fileName: The name given to this file
	\param alreadyOpenedFile: Pointer to the enclosing file
	\param pos: Start of the file inside alreadyOpenedFile
	\param areaSize: The length of the file
	\return A pointer to the created file interface.
	The returned pointer should be dropped when no longer needed.
	See IReferenceCounted::drop() for more information.
	*/
	virtual BaseReadFile* CreateLimitReadFile(const eastl::wstring& fileName, 
		BaseReadFile* alreadyOpenedFile, long pos, long areaSize) = 0;

	//! Creates an IWriteFile interface for accessing memory like a file.
	/** This allows you to use a pointer to memory where an IWriteFile is requested.
		You are responsible for allocating enough memory.
	\param memory: A pointer to the start of the file in memory (allocated by you)
	\param len: The length of the memory in bytes
	\param fileName: The name given to this file
	\param deleteMemoryWhenDropped: True if the memory should be deleted
	along with the IWriteFile when it is dropped.
	\return Pointer to the created file interface.
	The returned pointer should be dropped when no longer needed.
	See IReferenceCounted::drop() for more information.
	*/
	//virtual IWriteFile* CreateMemoryWriteFile(void* memory, int len, 
	//	const eastl::string& fileName, bool deleteMemoryWhenDropped=false) =0;

	//! Creates a list of files and directories in the current working directory and returns it.
	/** \return a Pointer to the created BaseFileList is returned. After the list has been used
	it has to be deleted using its BaseFileList::drop() method.
	See IReferenceCounted::drop() for more information. */
	virtual BaseFileList* CreateFileList() =0;

	//! Creates an empty filelist
	/** \return a Pointer to the created BaseFileList is returned. After the list has been used
	it has to be deleted using its BaseFileList::drop() method.
	See IReferenceCounted::drop() for more information. */
	virtual BaseFileList* CreateEmptyFileList(
		const eastl::wstring& path, bool ignoreCase, bool ignorePaths) =0;

	//! creates an mount point file archive (or basedirectory with subdirectories..) to the 
	//	file system.
	/*\param filename: Filename of the unzipped zip archive base directory to add 
	to the file system.
	\param ignoreCase: If set to true, files in the archive can be accessed without
	writing all letters in the right case.
	\param ignorePaths: If set to true, files in the added archive can be accessed
	without its complete path.
	\return True if the archive was added successful, false if not. */
	virtual BaseFileArchive* CreateMountPointFileArchive(
		const eastl::wstring& filename, bool ignoreCase=true, bool ignorePaths=true) = 0;

	//! Get the current working directory.
	/** \return Current working directory as a string. */
	virtual const eastl::wstring& GetWorkingDirectory() =0;

	//! Changes the current working directory.
	/** \param newDirectory: A string specifying the new working directory.
	The string is operating system dependent. Under Windows it has
	the form "<drive>:\<directory>\<sudirectory>\<..>". An example would be: "C:\Windows\"
	\return True if successful, otherwise false. */
	virtual bool ChangeWorkingDirectoryTo(const eastl::wstring& newDirectory) =0;

	/** Returns a list of files in a given directory.
	 *  \param result A reference to a eastl::vector<eastl::string> which will
	 *         hold all files in a directory. The vector will be cleared.
	 *  \param dir The director for which to get the directory listing.
	 *  \param makeFullPath If set to true, all listed files will be full paths.
	 */
    virtual void GetFileList(
		eastl::set<eastl::wstring>& result, const eastl::wstring& dir, bool makeFullPath=false) = 0;

	//! Determines if a directory exists and could be opened.
	/** \param dirname is the string identifying the directory which should be tested for existence.
	\return True if directory exists, and false if it does not exist or an error occured. */
	virtual bool ExistDirectory(const eastl::wstring& dirname) =0;

	//! Determines if a file exists and could be opened.
	/** \param filename is the string identifying the file which should be tested for existence.
	\return True if file exists, and false if it does not exist or an error occured. */
	virtual bool ExistFile(const eastl::wstring& filename) =0;

	//! Converts a relative path to an absolute (unique) path, resolving symbolic links if required
	/** \param filename Possibly relative file or directory name to query.
	\result Absolute filename which points to the same file. */
	virtual eastl::wstring GetAbsolutePath(const eastl::wstring& filename) const =0;

	//! Get the directory a file is located in.
	/** \param filename: The file to get the directory from.
	\return String containing the directory of the file. */
	virtual eastl::wstring GetFileDir(const eastl::wstring& filename) const =0;

	//! Set the active type of file system.
	virtual BaseFileSystemType SetFileSystemType(BaseFileSystemType listType) =0;

	//! Get the active type of file system.
	virtual BaseFileSystemType GetFileSystemType( ) =0;

	// Support for paths to locate files. For platform independence, use
	// "/" for the path separator. Please terminate the input 'directory'
	// values with "/". The Insert/Remove functions return 'true' iff the
	// operation was successful.
	virtual bool InsertDirectory(const eastl::string& directory) =0;
	virtual bool RemoveDirectory(const eastl::string& directory) =0;
	virtual void RemoveAllDirectories() =0;

	// The GetPath* function searches the list of directories and returns the
	// fully decorated file name, assuming it satisfies the required
	// conditions, or returns "" if conditions are not met. 
	virtual eastl::string GetPath(const eastl::string& filename) =0;
};

#endif

