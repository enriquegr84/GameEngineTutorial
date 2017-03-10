// Copyright (C) 2002-2012 Nikolaus Gebhardt
// This file is part of the "Irrlicht Engine".
// For conditions of distribution and use, see copyright notice in irrlicht.h

#include "LimitReadFile.h"

LimitReadFile::LimitReadFile(BaseReadFile* alreadyOpenedFile, 
	long pos, long areaSize, const eastl::wstring& name)
:	m_Filename(name), m_AreaStart(0), m_AreaEnd(0), m_Pos(0), m_File(alreadyOpenedFile)
{
	if (m_File)
	{
		m_AreaStart = pos;
		m_AreaEnd = m_AreaStart + areaSize;
	}
}


LimitReadFile::~LimitReadFile()
{
}


//! returns how much was read
int LimitReadFile::Read(void* buffer, unsigned int sizeToRead)
{
	if (0 == m_File)
		return 0;

	int r = m_AreaStart + m_Pos;
	int toRead = 
		eastl::min(m_AreaEnd, (long)(r + sizeToRead)) - 
		eastl::max(m_AreaStart, (long)r);
	if (toRead < 0)
		return 0;
	m_File->Seek(r);
	r = m_File->Read(buffer, toRead);
	m_Pos += r;
	return r;
}


//! changes position in file, returns true if successful
bool LimitReadFile::Seek(long finalPos, bool relativeMovement)
{
	m_Pos = eastl::clamp(finalPos + (relativeMovement ? m_Pos : 0 ), (long)0, m_AreaEnd - m_AreaStart);
	return true;
}


//! returns size of file
long LimitReadFile::GetSize() const
{
	return m_AreaEnd - m_AreaStart;
}


//! returns where in the file we are.
long LimitReadFile::GetPos() const
{
	return m_Pos;
}


//! returns name of file
const eastl::wstring& LimitReadFile::GetFileName() const
{
	return m_Filename;
}


BaseReadFile* CreateLimitReadFile(
	const eastl::wstring& fileName, BaseReadFile* alreadyOpenedFile, long pos, long areaSize)
{
	return new LimitReadFile(alreadyOpenedFile, pos, areaSize, fileName);
}

