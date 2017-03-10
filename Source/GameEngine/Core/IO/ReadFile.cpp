// Copyright (C) 2002-2012 Nikolaus Gebhardt
// This file is part of the "Irrlicht Engine".
// For conditions of distribution and use, see copyright notice in irrlicht.h

#include "ReadFile.h"


ReadFile::ReadFile(const eastl::wstring& fileName)
: m_File(0), m_FileSize(0), m_Filename(fileName)
{
	OpenFile();
}


ReadFile::~ReadFile()
{
	if (m_File)
		fclose(m_File);
}


//! returns how much was read
int ReadFile::Read(void* buffer, unsigned int sizeToRead)
{
	if (!IsOpen())
		return 0;

	return (int)fread(buffer, 1, sizeToRead, m_File);
}


//! changes position in file, returns true if successful
//! if relativeMovement==true, the pos is changed relative to current pos,
//! otherwise from begin of file
bool ReadFile::Seek(long finalPos, bool relativeMovement)
{
	if (!IsOpen())
		return false;

	return fseek(m_File, finalPos, relativeMovement ? SEEK_CUR : SEEK_SET) == 0;
}


//! returns size of file
long ReadFile::GetSize() const
{
	return m_FileSize;
}


//! returns where in the file we are.
long ReadFile::GetPos() const
{
	return ftell(m_File);
}


//! opens the file
void ReadFile::OpenFile()
{
	if (m_Filename.size() == 0) // bugfix posted by rt
	{
		m_File = 0;
		return;
	}

	m_File = _wfopen(m_Filename.c_str(), L"rb");
	if (m_File)
	{
		// get FileSize
		fseek(m_File, 0, SEEK_END);
		m_FileSize = GetPos();
		fseek(m_File, 0, SEEK_SET);
	}
}


//! returns name of file
const eastl::wstring& ReadFile::GetFileName() const
{
	return m_Filename;
}


BaseReadFile* ReadFile::CreateReadFile(const eastl::wstring& fileName)
{
	ReadFile* file = new ReadFile(fileName);
	if (file->IsOpen())
		return file;

	return 0;
}

