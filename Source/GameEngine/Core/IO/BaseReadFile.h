// Copyright (C) 2002-2012 Nikolaus Gebhardt
// This file is part of the "Irrlicht Engine".
// For conditions of distribution and use, see copyright notice in irrlicht.h

#ifndef BASEREADFILE_H
#define BASEREADFILE_H

//! Interface providing read acess to a file.
class BaseReadFile
{
public:
	//! Reads an amount of bytes from the file.
	/** \param buffer Pointer to buffer where read bytes are written to.
	\param sizeToRead Amount of bytes to read from the file.
	\return How many bytes were read. */
	virtual int Read(void* buffer, unsigned int sizeToRead) = 0;

	//! Changes position in file
	/** \param finalPos Destination position in the file.
	\param relativeMovement If set to true, the position in the file is
	changed relative to current position. Otherwise the position is changed
	from beginning of file.
	\return True if successful, otherwise false. */
	virtual bool Seek(long finalPos, bool relativeMovement = false) = 0;

	//! Get size of file.
	/** \return Size of the file in bytes. */
	virtual long GetSize() const = 0;

	//! Get the current position in the file.
	/** \return Current position in the file in bytes. */
	virtual long GetPos() const = 0;

	//! Get name of file.
	/** \return File name as zero terminated character string. */
	virtual const eastl::wstring& GetFileName() const = 0;
};

#endif

