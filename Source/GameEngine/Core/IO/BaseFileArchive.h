// Copyright (C) 2002-2012 Nikolaus Gebhardt/ Thomas Alten
// This file is part of the "Irrlicht Engine".
// For conditions of distribution and use, see copyright notice in irrlicht.h

#ifndef BASEFILEARCHIVE_H
#define BASEFILEARCHIVE_H

#include "BaseReadFile.h"
#include "BaseFileList.h"

//! Contains the different types of archives
enum E_FILE_ARCHIVE_TYPE
{
	//! A PKZIP archive
	EFAT_ZIP,

	//! A gzip archive
	EFAT_GZIP,

	//! A virtual directory
	EFAT_FOLDER,

	//! An ID Software PAK archive
	EFAT_PAK,

	//! A Nebula Device archive
	EFAT_NPK,

	//! A Tape ARchive
	EFAT_TAR,

	//! A wad Archive, Quake2, Halflife
	EFAT_WAD,

	//! The type of this archive is unknown
	EFAT_UNKNOWN
};

//! The FileArchive manages archives and provides access to files inside them.
class BaseFileArchive
{
public:

	//! Opens a file based on its name
	/** Creates and returns a new ReadFile for a file in the archive.
	\param filename The file to open
	\return Returns A pointer to the created file on success,
	or 0 on failure. */
	virtual BaseReadFile* CreateAndOpenFile(const eastl::wstring& filename) =0;

	//! Opens a file based on its position in the file list.
	/** Creates and returns
	\param index The zero based index of the file.
	\return Returns a pointer to the created file on success, or 0 on failure. */
	virtual BaseReadFile* CreateAndOpenFile(unsigned int index) =0;

	//! Returns the complete file tree
	/** \return Returns the complete directory tree for the archive,
	including all files and folders */
	virtual const BaseFileList* GetFileList() =0;

	//! get the archive type
	virtual E_FILE_ARCHIVE_TYPE GetType() const { return EFAT_UNKNOWN; }

	//! An optionally used password string
	/** This variable is publicly accessible from the interface in order to
	avoid single access patterns to this place, and hence allow some more
	obscurity. */
	eastl::wstring m_Password;
};

#endif

