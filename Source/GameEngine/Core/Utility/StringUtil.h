//========================================================================
// String.h : Defines some useful string utility functions
//
// Part of the GameEngine Application
//
// GameEngine is the sample application that encapsulates much of the source code
// discussed in "Game Coding Complete - 4th Edition" by Mike McShaffry and David
// "Rez" Graham, published by Charles River Media. 
// ISBN-10: 1133776574 | ISBN-13: 978-1133776574
//
// If this source code has found it's way to you, and you think it has helped you
// in any way, do the authors a favor and buy a new copy of the book - there are 
// detailed explanations in it that compliment this code well. Buy a copy at Amazon.com
// by clicking here: 
//    http://www.amazon.com/gp/product/1133776574/ref=olp_product_details?ie=UTF8&me=&seller=
//
// There's a companion web site at http://www.mcshaffry.com/GameCode/
// 
// The source code is managed and maintained through Google Code: 
//    http://code.google.com/p/GameEngine/
//
// (c) Copyright 2012 Michael L. McShaffry and David Graham
//
// This program is free software; you can redistribute it and/or
// modify it under the terms of the GNU Lesser GPL v3
// as published by the Free Software Foundation.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See 
// http://www.gnu.org/licenses/lgpl-3.0.txt for more details.
//
// You should have received a copy of the GNU Lesser GPL v3
// along with this program; if not, write to the Free Software
// Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
//
//========================================================================

#ifndef STRINGUTIL_H
#define STRINGUTIL_H

#include "Core/CoreStd.h"

#include "Graphic/Resource/Color.h"

static const char HexChars[] = "0123456789abcdef";

static inline eastl::string HexEncode(const char *data, unsigned int dataSize)
{
    eastl::string ret;
    ret.reserve(dataSize * 2);

    char buf2[3];
    buf2[2] = '\0';

    for (unsigned int i = 0; i < dataSize; i++) {
        unsigned char c = (unsigned char)data[i];
        buf2[0] = HexChars[(c & 0xf0) >> 4];
        buf2[1] = HexChars[c & 0x0f];
        ret.append(buf2);
    }

    return ret;
}

static inline eastl::string HexEncode(const eastl::string &data)
{
    return HexEncode(data.c_str(), data.size());
}

static inline bool HexDigitDecode(char hexdigit, unsigned char &value)
{
    if (hexdigit >= '0' && hexdigit <= '9')
        value = hexdigit - '0';
    else if (hexdigit >= 'A' && hexdigit <= 'F')
        value = hexdigit - 'A' + 10;
    else if (hexdigit >= 'a' && hexdigit <= 'f')
        value = hexdigit - 'a' + 10;
    else
        return false;
    return true;
}

inline eastl::string ToString(const wchar_t *str)
{
    std::wstring strSource(str);
    std::string strTarget(strSource.begin(), strSource.end());

    return eastl::string(strTarget.c_str());
}

inline eastl::wstring ToWideString(const char *str)
{
    std::string strSource(str);
    std::wstring strTarget(strSource.begin(), strSource.end());

    return eastl::wstring(strTarget.c_str());
}

/**
 * @param str
 * @return A copy of \p str converted to all lowercase characters.
 */
inline eastl::string lowercase(const eastl::string &str)
{
    eastl::string s2;

    s2.reserve(str.size());
    for (char i : str)
        s2 += tolower(i);

    return s2;
}

template <typename T>
eastl::vector<eastl::basic_string<T> > Split(const eastl::basic_string<T> &s, T delim);

static bool ParseColorString(
    const eastl::string &value, SColorF &color, bool quiet, unsigned char defaultAlpha = 0xff);
static bool ParseHexColorString(const eastl::string &value, SColorF &color, unsigned char defaultAlpha = 0xff);
static bool ParseNamedColorString(const eastl::string &value, SColorF &color);


// Does a classic * & ? pattern match on a file name - this is case sensitive!
bool WildcardMatch(const wchar_t *pat, const wchar_t *str);

//	A hashed string.  It retains the initial (ANSI) string in 
//	addition to the hash value for easy reference.

// class HashedString				- Chapter 10, page 274
class HashedString
{
public:
	explicit HashedString(char const * const pIdentstring)
		: mID(HashName(pIdentstring)), mIDStr(pIdentstring)
	{
	}

	unsigned long GetHashValue(void) const
	{

		return reinterpret_cast<unsigned long>(mID);
	}

	const eastl::string & GetStr() const
	{
		return mIDStr;
	}

	void* HashName(char const * pIdentStr);

	bool operator< (HashedString const & o) const
	{
		bool r = (GetHashValue() < o.GetHashValue());
		return r;
	}

	bool operator== (HashedString const & o) const
	{
		bool r = (GetHashValue() == o.GetHashValue());
		return r;
	}

private:

	// note: mID is stored as a void* not an int, so that in
	// the debugger it will show up as hex-values instead of
	// integer values. This is a bit more representative of what
	// we're doing here and makes it easy to allow external code
	// to assign event types as desired.

	void * mID;
	eastl::string mIDStr;
};

#endif