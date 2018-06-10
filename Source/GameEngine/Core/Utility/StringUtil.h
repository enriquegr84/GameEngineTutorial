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

// Does a classic * & ? pattern match on a file name - this is case sensitive!
bool WildcardMatch(const wchar_t *pat, const wchar_t *str);

eastl::string ToString(const wchar_t *str);
eastl::wstring ToWideString(const char *str);

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