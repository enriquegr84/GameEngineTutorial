// Copyright (C) 2002-2012 Nikolaus Gebhardt
// This file is part of the "Irrlicht Engine".
// For conditions of distribution and use, see copyright notice in irrlicht.h

#include "LimitReadFile.h"

LimitReadFile::LimitReadFile(BaseReadFile* alreadyOpenedFile, 
	long pos, long areaSize, const eastl::wstring& name)
:	mFileName(name), mAreaStart(0), mAreaEnd(0), mPos(0), mFile(alreadyOpenedFile)
{
	if (mFile)
	{
		mAreaStart = pos;
		mAreaEnd = mAreaStart + areaSize;
	}
}


LimitReadFile::~LimitReadFile()
{
}


//! returns how much was read
int LimitReadFile::Read(void* buffer, unsigned int sizeToRead)
{
	if (0 == mFile)
		return 0;

	int r = mAreaStart + mPos;
	int toRead = 
		eastl::min(mAreaEnd, (long)(r + sizeToRead)) - 
		eastl::max(mAreaStart, (long)r);
	if (toRead < 0)
		return 0;
	mFile->Seek(r);
	r = mFile->Read(buffer, toRead);
	mPos += r;
	return r;
}


//! changes position in file, returns true if successful
bool LimitReadFile::Seek(long finalPos, bool relativeMovement)
{
	mPos = eastl::clamp(finalPos + (relativeMovement ? mPos : 0 ), (long)0, mAreaEnd - mAreaStart);
	return true;
}


//! returns size of file
long LimitReadFile::GetSize() const
{
	return mAreaEnd - mAreaStart;
}


//! returns where in the file we are.
long LimitReadFile::GetPos() const
{
	return mPos;
}


//! returns name of file
const eastl::wstring& LimitReadFile::GetFileName() const
{
	return mFileName;
}


BaseReadFile* CreateLimitReadFile(
	const eastl::wstring& fileName, BaseReadFile* alreadyOpenedFile, long pos, long areaSize)
{
	return new LimitReadFile(alreadyOpenedFile, pos, areaSize, fileName);
}

