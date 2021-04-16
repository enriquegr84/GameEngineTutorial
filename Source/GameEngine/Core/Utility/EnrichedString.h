/*
Copyright (C) 2013 xyz, Ilya Zhuravlev <whatever@xyz.is>
Copyright (C) 2016 Nore, Nathanaël Courant <nore@mesecons.net>

This program is free software; you can redistribute it and/or modify
it under the terms of the GNU Lesser General Public License as published by
the Free Software Foundation; either version 2.1 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU Lesser General Public License for more details.

You should have received a copy of the GNU Lesser General Public License along
with this program; if not, write to the Free Software Foundation, Inc.,
51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
*/

#ifndef ENRICHEDSTRING_H
#define ENRICHEDSTRING_H

#include "Core/CoreStd.h"

#include "Graphic/Resource/Color.h"

class EnrichedString {
public:
	EnrichedString();
	EnrichedString(const eastl::wstring &s, const SColorF &color = SColorF(1.0f, 1.0f, 1.0f));
	EnrichedString(const wchar_t *str, const SColorF &color = SColorF(1.0f, 1.0f, 1.0f));
	EnrichedString(const eastl::wstring &string, const eastl::vector<SColorF> &colors);
	void operator=(const wchar_t *str);

	void Clear();

	void AddAtEnd(const eastl::wstring &s, SColorF color);

	// Adds the character source[i] at the end.
	// An EnrichedString should always be able to be copied
	// to the end of an existing EnrichedString that way.
	void AddChar(const EnrichedString &source, size_t i);

	// Adds a single character at the end, without specifying its
	// color. The color used will be the one from the last character.
	void AddCharNoColor(wchar_t c);

	EnrichedString Substr(size_t pos = 0, size_t len = eastl::string::npos) const;
	EnrichedString operator+(const EnrichedString &other) const;
	void operator+=(const EnrichedString &other);
	const wchar_t *C_Str() const;
	const eastl::vector<SColorF> &GetColors() const;
	const eastl::wstring &GetString() const;

	inline void SetDefaultColor(SColorF color)
	{
		mDefaultColor = color;
		UpdateDefaultColor();
	}
	void UpdateDefaultColor();
	inline const SColorF &GetDefaultColor() const
	{
		return mDefaultColor;
	}

	inline bool operator==(const EnrichedString &other) const
	{
		return (mString == other.mString && mColors == other.mColors);
	}
	inline bool operator!=(const EnrichedString &other) const
	{
		return !(*this == other);
	}
	inline bool Empty() const
	{
		return mString.empty();
	}
	inline size_t Size() const
	{
		return mString.size();
	}

	inline bool HasBackground() const
	{
		return mHasBackground;
	}
	inline SColorF GetBackground() const
	{
		return mBackground;
	}
	inline void SetBackground(SColorF color)
	{
		mBackground = color;
		mHasBackground = true;
	}

private:
	eastl::wstring mString;
	eastl::vector<SColorF> mColors;
	bool mHasBackground;
	SColorF mDefaultColor;
	SColorF mBackground;
	// This variable defines the length of the default-colored text.
	// Change this to a eastl::vector if an "end coloring" tag is wanted.
	size_t mDefaultLength = 0;
};

#endif