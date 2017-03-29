// Geometric Tools, LLC
// Copyright (c) 1998-2014
// Distributed under the Boost Software License, Version 1.0.
// http://www.boost.org/LICENSE_1_0.txt
// http://www.geometrictools.com/License/Boost/LICENSE_1_0.txt
//
// File Version: 5.0.1 (2010/10/01)

#ifndef FILEIO_H
#define FILEIO_H

#include "Core/CoreStd.h"

// Support for reading or writing a binary file.  If you open the file for
// reading, calling the writing routines is an error.  If you open the file
// for writing, calling the reading routines is an error.  This class does
// not support read-write access.  The class wraps FILE operations of fopen,
// fclose, fread, and fwrite.  A hint is passed to fopen to optimize caching
// for sequential access from disk.
//
// The nested enumeration Mode supports endianness mismatches between the
// platform that created the file and the platform that is using it.  The
// flags are defined as follows:
//
// FM_READ
//   Read the data from disk as is (no byte swapping).
//
//
// FM_READ_AND_SWAP
//   Read from disk and swap 2-, 4-, and 8-byte inputs.
//

class CORE_ITEM FileIO
{
public:
    enum
    {
        FM_NONE,
        FM_READ,
        FM_READ_AND_SWAP,

        // All data files are in little endian format, because most platforms
        // these days are little endian.
#ifdef LITTLE_ENDIAN
        FM_DEFAULT_READ = FM_READ,
#else
        FM_DEFAULT_READ = FM_READ_AND_SWAP,
#endif
    };

    // Construction and destruction.
    FileIO (const eastl::string& filename, int mode);
    ~FileIO ();

    // Implicit conversion to allow testing for successful file open.
    operator bool () const;

    bool Open (const eastl::string& filename, int mode);
    bool Close ();

	static long Size (const eastl::string& name);
	static bool Exists (const eastl::string& name);

    // The return value is 'true' if and only if the operation was
    // successful, in which case the number of bytes read or written is
    // the item size times number of items.
    bool Read (size_t itemSize, void* datum);
    bool Read (size_t itemSize, int numItems, void* data);
	
	bool Seek (long offset);

    // Member access.
    inline int GetMode () const;

private:
	const eastl::string& mFileName;
    FILE* mFile;
    int mMode;
};

//----------------------------------------------------------------------------
inline int FileIO::GetMode() const
{
	return mMode;
}
//----------------------------------------------------------------------------

#endif
