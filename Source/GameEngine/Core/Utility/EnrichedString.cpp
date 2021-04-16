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

#include "EnrichedString.h"
#include "StringUtil.h"

EnrichedString::EnrichedString()
{
	Clear();
}

EnrichedString::EnrichedString(const eastl::wstring &string, const eastl::vector<SColorF> &colors)
{
	Clear();
	mString = string;
	mColors = colors;
}

EnrichedString::EnrichedString(const eastl::wstring &s, const SColorF &color)
{
	Clear();
	AddAtEnd(s, color);
}

EnrichedString::EnrichedString(const wchar_t *str, const SColorF &color)
{
	Clear();
	AddAtEnd(str, color);
}

void EnrichedString::Clear()
{
	mString.clear();
	mColors.clear();
	mHasBackground = false;
	mDefaultLength = 0;
	mDefaultColor = SColorF(1.0f, 1.0f, 1.0f);
	mBackground = SColorF();
}

void EnrichedString::operator=(const wchar_t *str)
{
	Clear();
	AddAtEnd(str, mDefaultColor);
}

void EnrichedString::AddAtEnd(const eastl::wstring &s, SColorF initialColor)
{
	SColorF color(initialColor);
	bool useDefault = (mDefaultLength == mString.size() && color == mDefaultColor);

	mColors.reserve(mColors.size() + s.size());

	size_t i = 0;
	while (i < s.length()) {
		if (s[i] != L'\x1b') {
			mString += s[i];
			mColors.push_back(color);
			++i;
			continue;
		}
		++i;
		size_t startIndex = i;
		size_t length;
		if (i == s.length()) {
			break;
		}
		if (s[i] == L'(') {
			++i;
			++startIndex;
			while (i < s.length() && s[i] != L')') {
				if (s[i] == L'\\') {
					++i;
				}
				++i;
			}
			length = i - startIndex;
			++i;
		} else {
			++i;
			length = 1;
		}
		eastl::wstring escapeSequence(s, startIndex, length);
        eastl::vector<eastl::wstring> parts = Split(escapeSequence, L'@');
		if (parts[0] == L"c") {
			if (parts.size() < 2) {
				continue;
			}
			ParseColorString(ToString(parts[1].c_str()), color, true);

			// No longer use default color after first escape
			if (useDefault) {
				mDefaultLength = mString.size();
				useDefault = false;
			}
		} else if (parts[0] == L"b") {
			if (parts.size() < 2) {
				continue;
			}
			ParseColorString(ToString(parts[1].c_str()), mBackground, true);
			mHasBackground = true;
		}
	}

	// Update if no escape character was found
	if (useDefault)
		mDefaultLength = mString.size();
}

void EnrichedString::AddChar(const EnrichedString &source, size_t i)
{
	mString += source.mString[i];
	mColors.push_back(source.mColors[i]);
}

void EnrichedString::AddCharNoColor(wchar_t c)
{
	mString += c;
	if (mColors.empty()) 
		mColors.emplace_back(mDefaultColor);
    else 
		mColors.push_back(mColors[mColors.size() - 1]);
}

EnrichedString EnrichedString::operator+(const EnrichedString &other) const
{
	EnrichedString result = *this;
	result += other;
	return result;
}

void EnrichedString::operator+=(const EnrichedString &other)
{
	bool updateDefaultColor = mDefaultLength == mString.size();

	mString += other.mString;
	mColors.insert(mColors.end(), other.mColors.begin(), other.mColors.end());

	if (updateDefaultColor) {
		mDefaultLength += other.mDefaultLength;
		UpdateDefaultColor();
	}
}

EnrichedString EnrichedString::Substr(size_t pos, size_t len) const
{
	if (pos >= mString.length())
		return EnrichedString();

	if (len == eastl::string::npos || pos + len > mString.length())
		len = mString.length() - pos;

	EnrichedString str(mString.substr(pos, len),
		eastl::vector<SColorF>(mColors.begin() + pos, mColors.begin() + pos + len)
	);

	str.mHasBackground = mHasBackground;
	str.mBackground = mBackground;

	if (pos < mDefaultLength)
		str.mDefaultLength = eastl::min(mDefaultLength - pos, str.Size());
	str.SetDefaultColor(mDefaultColor);
	return str;
}

const wchar_t *EnrichedString::C_Str() const
{
	return mString.c_str();
}

const eastl::vector<SColorF> &EnrichedString::GetColors() const
{
	return mColors;
}

const eastl::wstring &EnrichedString::GetString() const
{
	return mString;
}

void EnrichedString::UpdateDefaultColor()
{
	SanityCheck(mDefaultLength <= mColors.size());

	for (size_t i = 0; i < mDefaultLength; ++i)
		mColors[i] = mDefaultColor;
}
