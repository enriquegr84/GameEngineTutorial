// Copyright (C) 2002-2012 Nikolaus Gebhardt
// This file is part of the "Irrlicht Engine".
// For conditions of distribution and use, see copyright notice in irrlicht.h

#ifndef MEMORYREADFILE_H
#define MEMORYREADFILE_H

#include "GameEngineStd.h"

#include "BaseReadFile.h"


//	Class for reading from memory.
class MemoryReadFile : public BaseReadFile
{
public:

	//! Constructor
	MemoryReadFile(const void* memory, long len, 
		const eastl::wstring& fileName, bool deleteMemoryWhenDropped);

	//! Destructor
	virtual ~MemoryReadFile();

	//! returns how much was read
	virtual int Read(void* buffer, unsigned int sizeToRead);

	//! changes position in file, returns true if successful
	virtual bool Seek(long finalPos, bool relativeMovement = false);

	//! returns size of file
	virtual long GetSize() const;

	//! returns where in the file we are.
	virtual long GetPos() const;

	//! returns name of file
	virtual const eastl::wstring& GetFileName() const;

private:

	const void* mBuffer;
	long mLen;
	long mPos;
	eastl::wstring mFileName;
	bool mDeleteMemoryWhenDropped;
};

#endif

