// Copyright (C) 2002-2012 Nikolaus Gebhardt
// This file is part of the "Irrlicht Engine".
// For conditions of distribution and use, see copyright notice in irrlicht.h

#include "MemoryFile.h"


MemoryReadFile::MemoryReadFile(const void* memory, long len, 
	const eastl::wstring& fileName, bool d)
:	m_Buffer(memory), m_Len(len), m_Pos(0), 
	m_Filename(fileName), m_DeleteMemoryWhenDropped(d)
{

}


MemoryReadFile::~MemoryReadFile()
{
	if (m_DeleteMemoryWhenDropped)
		delete m_Buffer;
}


//! returns how much was read
int MemoryReadFile::Read(void* buffer, unsigned int sizeToRead)
{
	int amount = static_cast<int>(sizeToRead);
	if (m_Pos + amount > m_Len)
		amount -= m_Pos + amount - m_Len;

	if (amount <= 0)
		return 0;

	char* p = (char*)m_Buffer;
	memcpy(buffer, p + m_Pos, amount);
	m_Pos += amount;

	return amount;
}

//! changes position in file, returns true if successful
//! if relativeMovement==true, the pos is changed relative to current pos,
//! otherwise from begin of file
bool MemoryReadFile::Seek(long finalPos, bool relativeMovement)
{
	if (relativeMovement)
	{
		if (m_Pos + finalPos > m_Len)
			return false;

		m_Pos += finalPos;
	}
	else
	{
		if (finalPos > m_Len)
			return false;

		m_Pos = finalPos;
	}

	return true;
}


//! returns size of file
long MemoryReadFile::GetSize() const
{
	return m_Len;
}


//! returns where in the file we are.
long MemoryReadFile::GetPos() const
{
	return m_Pos;
}


//! returns name of file
const eastl::wstring& MemoryReadFile::GetFileName() const
{
	return m_Filename;
}

BaseReadFile* CreateMemoryReadFile(const void* memory, long size, 
	const eastl::wstring& fileName, bool deleteMemoryWhenDropped)
{
	MemoryReadFile* file = new MemoryReadFile(
		memory, size, fileName, deleteMemoryWhenDropped);
	return file;
}

