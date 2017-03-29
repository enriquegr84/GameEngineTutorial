// Copyright (C) 2002-2012 Nikolaus Gebhardt
// This file is part of the "Irrlicht Engine".
// For conditions of distribution and use, see copyright notice in irrlicht.h

#include "MemoryFile.h"


MemoryReadFile::MemoryReadFile(const void* memory, long len, 
	const eastl::wstring& fileName, bool d)
:	mBuffer(memory), mLen(len), mPos(0), 
	mFileName(fileName), mDeleteMemoryWhenDropped(d)
{

}


MemoryReadFile::~MemoryReadFile()
{
	if (mDeleteMemoryWhenDropped)
		delete mBuffer;
}


//! returns how much was read
int MemoryReadFile::Read(void* buffer, unsigned int sizeToRead)
{
	int amount = static_cast<int>(sizeToRead);
	if (mPos + amount > mLen)
		amount -= mPos + amount - mLen;

	if (amount <= 0)
		return 0;

	char* p = (char*)mBuffer;
	memcpy(buffer, p + mPos, amount);
	mPos += amount;

	return amount;
}

//! changes position in file, returns true if successful
//! if relativeMovement==true, the pos is changed relative to current pos,
//! otherwise from begin of file
bool MemoryReadFile::Seek(long finalPos, bool relativeMovement)
{
	if (relativeMovement)
	{
		if (mPos + finalPos > mLen)
			return false;

		mPos += finalPos;
	}
	else
	{
		if (finalPos > mLen)
			return false;

		mPos = finalPos;
	}

	return true;
}


//! returns size of file
long MemoryReadFile::GetSize() const
{
	return mLen;
}


//! returns where in the file we are.
long MemoryReadFile::GetPos() const
{
	return mPos;
}


//! returns name of file
const eastl::wstring& MemoryReadFile::GetFileName() const
{
	return mFileName;
}

BaseReadFile* CreateMemoryReadFile(const void* memory, long size, 
	const eastl::wstring& fileName, bool deleteMemoryWhenDropped)
{
	MemoryReadFile* file = new MemoryReadFile(
		memory, size, fileName, deleteMemoryWhenDropped);
	return file;
}

