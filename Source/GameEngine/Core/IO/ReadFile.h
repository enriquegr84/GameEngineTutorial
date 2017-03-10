// Copyright (C) 2002-2012 Nikolaus Gebhardt
// This file is part of the "Irrlicht Engine".
// For conditions of distribution and use, see copyright notice in irrlicht.h

#ifndef READFILE_H
#define READFILE_H

#include "GameEngineStd.h"

#include "BaseReadFile.h"

/*!
	Class for reading a real file from disk. It provides read acess to a file.
*/
class ReadFile : public BaseReadFile
{
public:

	ReadFile(const eastl::wstring& fileName);

	~ReadFile();

	//! Reads an amount of bytes from the file.
	/** \param buffer Pointer to buffer where read bytes are written to.
	\param sizeToRead Amount of bytes to read from the file.
	\return How many bytes were read. */
	virtual int Read(void* buffer, unsigned int sizeToRead);

	//! Changes position in file
	/** \param finalPos Destination position in the file.
	\param relativeMovement If set to true, the position in the file is
	changed relative to current position. Otherwise the position is changed
	from beginning of file.
	\return True if successful, otherwise false. */
	virtual bool Seek(long finalPos, bool relativeMovement = false);

	//! Get size of file.
	/** \return Size of the file in bytes. */
	virtual long GetSize() const;

	//! returns if file is open
	virtual bool IsOpen() const { return m_File != 0; }

	//! Get the current position in the file.
	/** \return Current position in the file in bytes. */
	virtual long GetPos() const;

	//! Get name of file.
	/** \return File name as zero terminated character string. */
	virtual const eastl::wstring& GetFileName() const;

	//! create read file on disk.
	static BaseReadFile* CreateReadFile(const eastl::wstring& fileName);

private:

	//! opens the file
	void OpenFile();

	FILE* m_File;
	long m_FileSize;
	eastl::wstring m_Filename;
};


#endif

