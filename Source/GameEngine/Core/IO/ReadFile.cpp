// Copyright (C) 2002-2012 Nikolaus Gebhardt
// This file is part of the "Irrlicht Engine".
// For conditions of distribution and use, see copyright notice in irrlicht.h

#include "ReadFile.h"


ReadFile::ReadFile(const eastl::wstring& fileName)
: mFile(0), mFileSize(0), mFileName(fileName)
{
	OpenFile();
}


ReadFile::~ReadFile()
{
	if (mFile)
		fclose(mFile);
}


//! returns how much was read
int ReadFile::Read(void* buffer, unsigned int sizeToRead)
{
	if (!IsOpen())
		return 0;

	return (int)fread(buffer, 1, sizeToRead, mFile);
}


//! changes position in file, returns true if successful
//! if relativeMovement==true, the pos is changed relative to current pos,
//! otherwise from begin of file
bool ReadFile::Seek(long finalPos, bool relativeMovement)
{
	if (!IsOpen())
		return false;

	return fseek(mFile, finalPos, relativeMovement ? SEEK_CUR : SEEK_SET) == 0;
}


//! returns size of file
long ReadFile::GetSize() const
{
	return mFileSize;
}


//! returns where in the file we are.
long ReadFile::GetPos() const
{
	return ftell(mFile);
}


//! opens the file
void ReadFile::OpenFile()
{
	if (mFileName.size() == 0) // bugfix posted by rt
	{
		mFile = 0;
		return;
	}

	mFile = _wfopen(mFileName.c_str(), L"rb");
	if (mFile)
	{
		// get FileSize
		fseek(mFile, 0, SEEK_END);
		mFileSize = GetPos();
		fseek(mFile, 0, SEEK_SET);
	}
}


//! returns name of file
const eastl::wstring& ReadFile::GetFileName() const
{
	return mFileName;
}


BaseReadFile* ReadFile::CreateReadFile(const eastl::wstring& fileName)
{
	ReadFile* file = new ReadFile(fileName);
	if (file->IsOpen())
		return file;

	return nullptr;
}

