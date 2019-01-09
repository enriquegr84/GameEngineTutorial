// Copyright (C) 2002-2012 Nikolaus Gebhardt
// This file is part of the "Irrlicht Engine".
// For conditions of distribution and use, see copyright notice in irrlicht.h

#ifndef FILESYSTEM_H
#define FILESYSTEM_H

#include "BaseFileSystem.h"
#include "BaseFileArchive.h"

//! The FileSystem manages files and archives and provides access to them.
/* It manages where files are, so that modules which use the the IO do not
need to know where every file is located. A file could be in a .zip-Archive or
as file on disk, using the FileSystem makes no difference to this. */
class FileSystem : public BaseFileSystem
{
public:

	//! constructor
	FileSystem();

	//! destructor
	virtual ~FileSystem();

	//! Creates an ReadFile interface for accessing memory like a file.
	/** This allows you to use a pointer to memory where an ReadFile is requested.
	\param memory: A pointer to the start of the file in memory
	\param len: The length of the memory in bytes
	\param fileName: The name given to this file
	\param deleteMemoryWhenDropped: True if the memory should be deleted
	along with the ReadFile when it is dropped.
	\return Pointer to the created file interface.*/
	virtual BaseReadFile* CreateMemoryReadFile(const void* memory, int len, 
		const eastl::wstring& fileName, bool deleteMemoryWhenDropped = false);

	//! Creates an ReadFile interface for accessing files inside files.
	/** This is useful e.g. for archives.
	\param fileName: The name given to this file
	\param alreadyOpenedFile: Pointer to the enclosing file
	\param pos: Start of the file inside alreadyOpenedFile
	\param areaSize: The length of the file
	\return A pointer to the created file interface.*/
	virtual BaseReadFile* CreateLimitReadFile(const eastl::wstring& fileName, 
		BaseReadFile* alreadyOpenedFile, long pos, long areaSize);

	//! Creates an ReadFile interface for accessing files.
	/** This is useful e.g. for archives.
	\param fileName: The name given to this file
	\return A pointer to the created file interface.*/
	virtual BaseReadFile* CreateReadFile(const eastl::wstring& fileName);

	//! Creates an empty filelist
	/** \return a Pointer to the created FileList is returned. */
	virtual BaseFileList* CreateFileList();

	//! Creates a list of files and directories in the current working directory and returns it.
	/** \return a Pointer to the created FileList is returned. */
	virtual BaseFileList* CreateEmptyFileList(
		const eastl::wstring& path, bool ignoreCase, bool ignorePaths);

	//! creates a mount point file archive (or basedirectory with subdirectories..) to the 
	virtual BaseFileArchive* CreateMountPointFileArchive(
		const eastl::wstring& filename, bool ignoreCase, bool ignorePaths);

	//! Get the current working directory.
	/** \return Current working directory as a string. */
	virtual const eastl::wstring& GetWorkingDirectory();

	//! Changes the current working directory.
	/** \param newDirectory: A string specifying the new working directory.
	The string is operating system dependent. Under Windows it has
	the form "<drive>:\<directory>\<sudirectory>\<..>". An example would be: "C:\Windows\"
	\return True if successful, otherwise false. */
	virtual bool ChangeWorkingDirectoryTo(const eastl::wstring& newDirectory);

	/** Returns a list of files in a given directory.
	 *  \param result A reference to a std::vector<std::string> which will
	 *         hold all files in a directory. The vector will be cleared.
	 *  \param dir The director for which to get the directory listing.
	 *  \param make_full_path If set to true, all listed files will be full paths.
	 */
    virtual void GetFileList(
		eastl::set<eastl::wstring>& result, const eastl::wstring& dir, bool make_full_path=false);

	//! Determines if a directory exists and could be opened.
	/** \param dirname is the string identifying the directory which should be tested for existence.
	\return True if directory exists, and false if it does not exist or an error occured. */
	virtual bool ExistDirectory(const eastl::wstring& dirname) const;

	//! Determines if a file exists and could be opened.
	/** \param filename is the string identifying the file which should be tested for existence.
	\return True if file exists, and false if it does not exist or an error occured. */
	virtual bool ExistFile(const eastl::wstring& filename) const;

	//! Converts a relative path to an absolute (unique) path, resolving symbolic links if required
	/** \param filename Possibly relative file or directory name to query.
	\result Absolute filename which points to the same file. */
	virtual eastl::wstring GetAbsolutePath(const eastl::wstring& filename) const;

	//! Get the directory a file is located in.
	/** \param filename: The file to get the directory from.
	\return String containing the directory of the file. */
	virtual eastl::wstring GetFileDir(const eastl::wstring& filename) const;

	//! Set the active type of file system.
	virtual BaseFileSystemType SetFileSystemType(BaseFileSystemType listType);

	//! Get the active type of file system.
	virtual BaseFileSystemType GetFileSystemType( );

	// Support for paths to locate files. For platform independence, use
	// "/" for the path separator. Please terminate the input 'directory'
	// values with "/". The Insert/Remove functions return 'true' iff the
	// operation was successful.
	virtual bool InsertDirectory(const eastl::string& directory);
	virtual bool RemoveDirectory(const eastl::string& directory);
	virtual void RemoveAllDirectories();

	// The GetPath* function searches the list of directories and returns the
	// fully decorated file name, assuming it satisfies the required
	// conditions, or returns "" if conditions are not met. 
	eastl::string GetPath(const eastl::string& filename);

	// Getter for the main global filesystem. This is the filesystem that is used by 
	// the majority of the engine, though you are free to define your own as long as 
	// you instantiate it. It is not valid to have more than one global filesystem.
	static FileSystem* Get(void);

protected:

	static FileSystem* mFileSystem;

private:

	//! Currently used FileSystemType
	BaseFileSystemType mFileSystemType;
	//! WorkingDirectory for Native and Virtual filesystems
	eastl::wstring mWorkingDirectory[2];

	// The list of directories for GetPath to search. The list is
	// allocated during InitTerm::ExecuteInitializers. As with other
	// classes in GameEngine, no dynamic allocation occurs pre-main to
	// assist in tracking memory problems.
	eastl::vector<eastl::string>* msDirectories;
};

#endif

