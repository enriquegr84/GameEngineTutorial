// Copyright (C) 2002-2012 Nikolaus Gebhardt
// This file is part of the "Irrlicht Engine".
// For conditions of distribution and use, see copyright notice in irrlicht.h

#ifndef LIMITREADFILE_H
#define LIMITREADFILE_H

#include "GameEngineStd.h"
#include "ReadFile.h"

class UnicodeConverter;


/*! this is a read file, which is limited to some boundaries,
	so that it may only start from a certain file position
	and may only read until a certain file position.
	This can be useful, for example for reading uncompressed files
	in an archive (zip, tar).
!*/
class LimitReadFile : public BaseReadFile
{
public:

	LimitReadFile(BaseReadFile* alreadyOpenedFile, long pos, long areaSize, const eastl::wstring& name);

	virtual ~LimitReadFile();

	//! returns how much was read
	virtual int Read(void* buffer, unsigned int sizeToRead);

	//! changes position in file, returns true if successful
	//! if relativeMovement==true, the pos is changed relative to current pos,
	//! otherwise from begin of file
	virtual bool Seek(long finalPos, bool relativeMovement = false);

	//! returns size of file
	virtual long GetSize() const;

	//! returns where in the file we are.
	virtual long GetPos() const;

	//! returns name of file
	virtual const eastl::wstring& GetFileName() const;

private:

	eastl::wstring mFileName;
	long mAreaStart;
	long mAreaEnd;
	long mPos;
	eastl::shared_ptr<BaseReadFile> mFile;
};

#endif

