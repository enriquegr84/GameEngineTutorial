// Geometric Tools, LLC
// Copyright (c) 1998-2014
// Distributed under the Boost Software License, Version 1.0.
// http://www.boost.org/LICENSE_1_0.txt
// http://www.geometrictools.com/License/Boost/LICENSE_1_0.txt
//
// File Version: 5.0.0 (2010/01/01)

#include "Core/Core.h"
#include "Core/Logger/Logger.h"

#include "FileIO.h"

// TODO.  Add the file handling for Macintosh.
#include <sys/stat.h>

//----------------------------------------------------------------------------
FileIO::FileIO (const eastl::string& filename, int mode)
    :
    mFile(0),
    mMode(FM_NONE),
	mFileName(filename)
{
    Open(filename, mode);
}
//----------------------------------------------------------------------------
FileIO::~FileIO ()
{
    if (mMode != FM_NONE)
    {
        Close();
    }
}
//----------------------------------------------------------------------------
bool FileIO::Open (const eastl::string& filename, int mode)
{
    if (mMode == FM_NONE)
    {
        if (mode == FM_READ)
        {
            mFile = fopen(filename.c_str(), "rb");
        }
		else if (mode == FM_READ_AND_SWAP)
		{
            mFile = fopen(filename.c_str(), "rbS");
		}

        if (mFile)
        {
            mMode = mode;
            return true;
        }

		char message[256];
		sprintf(message, "Failed to open file %s\n", filename.c_str());
        LogError(message);
    }
    else
    {
		char message[256];
		sprintf(message, "File %s is already opened\n", filename.c_str());
		LogError(message);
    }

    return false;
}
//----------------------------------------------------------------------------
bool FileIO::Close ()
{
    mMode = FM_NONE;
    if (fclose(mFile) == 0)
    {
        return true;
    }

    LogError("Failed to close file\n");
    return false;
}
//----------------------------------------------------------------------------
FileIO::operator bool () const
{
    return mMode != FM_NONE;
}
//----------------------------------------------------------------------------
long FileIO::Size(const eastl::string& name)
{
	struct stat filestatus;
	stat( name.c_str(), &filestatus );
	return filestatus.st_size;
}
//----------------------------------------------------------------------------
bool FileIO::Exists (const eastl::string& name) 
{
  struct stat buffer;   
  return (stat (name.c_str(), &buffer) == 0); 
}
//----------------------------------------------------------------------------
bool FileIO::Seek(long offset)
{
	return fseek(mFile, offset, SEEK_SET) == 0;
}
//----------------------------------------------------------------------------
bool FileIO::Read (size_t itemSize, void* datum)
{
#ifdef FILEIO_VALIDATE_OPERATION
    if ((mMode != FM_READ && mMode != FM_READ_AND_SWAP) || !mFile || !datum)
    {
        LogError("Invalid FileIO::Read\n");
        return false;
    }
#endif

    int numRead = (int)fread(datum, itemSize, 1, mFile);

#ifdef FILEIO_VALIDATE_OPERATION
    if (numRead != 1)
    {
        LogError("Invalid FileIO::Read\n");
        return false;
    }
#else
    UNUSED(numRead);
#endif

    return true;
}
//----------------------------------------------------------------------------
bool FileIO::Read (size_t itemSize, int numItems, void* data)
{
#ifdef FILEIO_VALIDATE_OPERATION
    if ((mMode != FM_READ && mMode != FM_READ_AND_SWAP) || !mFile || numItems <= 0 || !data)
    {
        LogError("Invalid FileIO::Read\n");
        return false;
    }
#endif

    int numRead = (int)fread(data, itemSize, numItems, mFile);

#ifdef FILEIO_VALIDATE_OPERATION
    if (numRead != numItems)
    {
        LogError("Invalid FileIO::Read\n");
        return false;
    }
#else
    UNUSED(numRead);
#endif

    return true;
}
//----------------------------------------------------------------------------
