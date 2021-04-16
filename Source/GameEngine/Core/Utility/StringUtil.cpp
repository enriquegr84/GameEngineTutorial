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

#include "StringUtil.h"

#include "Core/Logger/Logger.h"

struct ColorContainer {
    ColorContainer();
    eastl::map<const eastl::string, uint32_t> colors;
};

ColorContainer::ColorContainer()
{
    colors["aliceblue"] = 0xf0f8ff;
    colors["antiquewhite"] = 0xfaebd7;
    colors["aqua"] = 0x00ffff;
    colors["aquamarine"] = 0x7fffd4;
    colors["azure"] = 0xf0ffff;
    colors["beige"] = 0xf5f5dc;
    colors["bisque"] = 0xffe4c4;
    colors["black"] = 00000000;
    colors["blanchedalmond"] = 0xffebcd;
    colors["blue"] = 0x0000ff;
    colors["blueviolet"] = 0x8a2be2;
    colors["brown"] = 0xa52a2a;
    colors["burlywood"] = 0xdeb887;
    colors["cadetblue"] = 0x5f9ea0;
    colors["chartreuse"] = 0x7fff00;
    colors["chocolate"] = 0xd2691e;
    colors["coral"] = 0xff7f50;
    colors["cornflowerblue"] = 0x6495ed;
    colors["cornsilk"] = 0xfff8dc;
    colors["crimson"] = 0xdc143c;
    colors["cyan"] = 0x00ffff;
    colors["darkblue"] = 0x00008b;
    colors["darkcyan"] = 0x008b8b;
    colors["darkgoldenrod"] = 0xb8860b;
    colors["darkgray"] = 0xa9a9a9;
    colors["darkgreen"] = 0x006400;
    colors["darkgrey"] = 0xa9a9a9;
    colors["darkkhaki"] = 0xbdb76b;
    colors["darkmagenta"] = 0x8b008b;
    colors["darkolivegreen"] = 0x556b2f;
    colors["darkorange"] = 0xff8c00;
    colors["darkorchid"] = 0x9932cc;
    colors["darkred"] = 0x8b0000;
    colors["darksalmon"] = 0xe9967a;
    colors["darkseagreen"] = 0x8fbc8f;
    colors["darkslateblue"] = 0x483d8b;
    colors["darkslategray"] = 0x2f4f4f;
    colors["darkslategrey"] = 0x2f4f4f;
    colors["darkturquoise"] = 0x00ced1;
    colors["darkviolet"] = 0x9400d3;
    colors["deeppink"] = 0xff1493;
    colors["deepskyblue"] = 0x00bfff;
    colors["dimgray"] = 0x696969;
    colors["dimgrey"] = 0x696969;
    colors["dodgerblue"] = 0x1e90ff;
    colors["firebrick"] = 0xb22222;
    colors["floralwhite"] = 0xfffaf0;
    colors["forestgreen"] = 0x228b22;
    colors["fuchsia"] = 0xff00ff;
    colors["gainsboro"] = 0xdcdcdc;
    colors["ghostwhite"] = 0xf8f8ff;
    colors["gold"] = 0xffd700;
    colors["goldenrod"] = 0xdaa520;
    colors["gray"] = 0x808080;
    colors["green"] = 0x008000;
    colors["greenyellow"] = 0xadff2f;
    colors["grey"] = 0x808080;
    colors["honeydew"] = 0xf0fff0;
    colors["hotpink"] = 0xff69b4;
    colors["indianred"] = 0xcd5c5c;
    colors["indigo"] = 0x4b0082;
    colors["ivory"] = 0xfffff0;
    colors["khaki"] = 0xf0e68c;
    colors["lavender"] = 0xe6e6fa;
    colors["lavenderblush"] = 0xfff0f5;
    colors["lawngreen"] = 0x7cfc00;
    colors["lemonchiffon"] = 0xfffacd;
    colors["lightblue"] = 0xadd8e6;
    colors["lightcoral"] = 0xf08080;
    colors["lightcyan"] = 0xe0ffff;
    colors["lightgoldenrodyellow"] = 0xfafad2;
    colors["lightgray"] = 0xd3d3d3;
    colors["lightgreen"] = 0x90ee90;
    colors["lightgrey"] = 0xd3d3d3;
    colors["lightpink"] = 0xffb6c1;
    colors["lightsalmon"] = 0xffa07a;
    colors["lightseagreen"] = 0x20b2aa;
    colors["lightskyblue"] = 0x87cefa;
    colors["lightslategray"] = 0x778899;
    colors["lightslategrey"] = 0x778899;
    colors["lightsteelblue"] = 0xb0c4de;
    colors["lightyellow"] = 0xffffe0;
    colors["lime"] = 0x00ff00;
    colors["limegreen"] = 0x32cd32;
    colors["linen"] = 0xfaf0e6;
    colors["magenta"] = 0xff00ff;
    colors["maroon"] = 0x800000;
    colors["mediumaquamarine"] = 0x66cdaa;
    colors["mediumblue"] = 0x0000cd;
    colors["mediumorchid"] = 0xba55d3;
    colors["mediumpurple"] = 0x9370db;
    colors["mediumseagreen"] = 0x3cb371;
    colors["mediumslateblue"] = 0x7b68ee;
    colors["mediumspringgreen"] = 0x00fa9a;
    colors["mediumturquoise"] = 0x48d1cc;
    colors["mediumvioletred"] = 0xc71585;
    colors["midnightblue"] = 0x191970;
    colors["mintcream"] = 0xf5fffa;
    colors["mistyrose"] = 0xffe4e1;
    colors["moccasin"] = 0xffe4b5;
    colors["navajowhite"] = 0xffdead;
    colors["navy"] = 0x000080;
    colors["oldlace"] = 0xfdf5e6;
    colors["olive"] = 0x808000;
    colors["olivedrab"] = 0x6b8e23;
    colors["orange"] = 0xffa500;
    colors["orangered"] = 0xff4500;
    colors["orchid"] = 0xda70d6;
    colors["palegoldenrod"] = 0xeee8aa;
    colors["palegreen"] = 0x98fb98;
    colors["paleturquoise"] = 0xafeeee;
    colors["palevioletred"] = 0xdb7093;
    colors["papayawhip"] = 0xffefd5;
    colors["peachpuff"] = 0xffdab9;
    colors["peru"] = 0xcd853f;
    colors["pink"] = 0xffc0cb;
    colors["plum"] = 0xdda0dd;
    colors["powderblue"] = 0xb0e0e6;
    colors["purple"] = 0x800080;
    colors["red"] = 0xff0000;
    colors["rosybrown"] = 0xbc8f8f;
    colors["royalblue"] = 0x4169e1;
    colors["saddlebrown"] = 0x8b4513;
    colors["salmon"] = 0xfa8072;
    colors["sandybrown"] = 0xf4a460;
    colors["seagreen"] = 0x2e8b57;
    colors["seashell"] = 0xfff5ee;
    colors["sienna"] = 0xa0522d;
    colors["silver"] = 0xc0c0c0;
    colors["skyblue"] = 0x87ceeb;
    colors["slateblue"] = 0x6a5acd;
    colors["slategray"] = 0x708090;
    colors["slategrey"] = 0x708090;
    colors["snow"] = 0xfffafa;
    colors["springgreen"] = 0x00ff7f;
    colors["steelblue"] = 0x4682b4;
    colors["tan"] = 0xd2b48c;
    colors["teal"] = 0x008080;
    colors["thistle"] = 0xd8bfd8;
    colors["tomato"] = 0xff6347;
    colors["turquoise"] = 0x40e0d0;
    colors["violet"] = 0xee82ee;
    colors["wheat"] = 0xf5deb3;
    colors["white"] = 0xffffff;
    colors["whitesmoke"] = 0xf5f5f5;
    colors["yellow"] = 0xffff00;
    colors["yellowgreen"] = 0x9acd32;

}

static const ColorContainer named_colors;


template <typename T>
eastl::vector<eastl::basic_string<T> > Split(const eastl::basic_string<T> &s, T delim)
{
    eastl::vector<eastl::basic_string<T> > tokens;

    eastl::basic_string<T> current;
    bool lastWasEscape = false;
    for (size_t i = 0; i < s.length(); i++) {
        T si = s[i];
        if (lastWasEscape) {
            current += '\\';
            current += si;
            lastWasEscape = false;
        }
        else {
            if (si == delim) {
                tokens.push_back(current);
                current = eastl::basic_string<T>();
                lastWasEscape = false;
            }
            else if (si == '\\') {
                lastWasEscape = true;
            }
            else {
                current += si;
                lastWasEscape = false;
            }
        }
    }
    //push last element
    tokens.push_back(current);

    return tokens;
}


bool ParseColorString(
    const eastl::string &value, SColorF &color, bool quiet, unsigned char defaultAlpha)
{
    bool success;

    if (value[0] == '#')
        success = ParseHexColorString(value, color, defaultAlpha);
    else
        success = ParseNamedColorString(value, color);

    if (!success && !quiet)
        LogError("Invalid color: \"" + value + "\"" );

    return success;
}

static bool ParseHexColorString(const eastl::string &value, SColorF &color, unsigned char defaultAlpha)
{
    unsigned char components[] = { 0x00, 0x00, 0x00, defaultAlpha }; // R,G,B,A

    if (value[0] != '#')
        return false;

    size_t len = value.size();
    bool shortForm;

    if (len == 9 || len == 7) // #RRGGBBAA or #RRGGBB
        shortForm = false;
    else if (len == 5 || len == 4) // #RGBA or #RGB
        shortForm = true;
    else
        return false;

    bool success = true;

    for (size_t pos = 1, cc = 0; pos < len; pos++, cc++) {
        LogAssert(cc < sizeof components / sizeof components[0], "wrong components size");
        if (shortForm) {
            unsigned char d;
            if (!HexDigitDecode(value[pos], d)) {
                success = false;
                break;
            }
            components[cc] = (d & 0xf) << 4 | (d & 0xf);
        }
        else {
            unsigned char d1, d2;
            if (!HexDigitDecode(value[pos], d1) ||
                !HexDigitDecode(value[pos + 1], d2)) {
                success = false;
                break;
            }
            components[cc] = (d1 & 0xf) << 4 | (d2 & 0xf);
            pos++;	// skip the second digit -- it's already used
        }
    }

    if (success) {
        color.setRed(components[0]);
        color.setGreen(components[1]);
        color.setBlue(components[2]);
        color.setAlpha(components[3]);
    }

    return success;
}

static bool ParseNamedColorString(const eastl::string &value, SColorF &color)
{
    eastl::string colorName;
    eastl::string alphaString;

    /* If the string has a # in it, assume this is the start of a specified
     * alpha value (if it isn't the string is invalid and the error will be
     * caught later on, either because the color name won't be found or the
     * alpha value will fail conversion)
     */
    size_t alphaPos = value.find('#');
    if (alphaPos != eastl::string::npos) {
        colorName = value.substr(0, alphaPos);
        alphaString = value.substr(alphaPos + 1);
    }
    else {
        colorName = value;
    }

    colorName = lowercase(colorName);

    eastl::map<const eastl::string, unsigned>::const_iterator it;
    it = named_colors.colors.find(colorName);
    if (it == named_colors.colors.end())
        return false;

    uint32_t colorTemp = it->second;

    /* An empty string for alpha is ok (none of the color table entries
     * have an alpha value either). Color strings without an alpha specified
     * are interpreted as fully opaque
     *
     * For named colors the supplied alpha string (representing a hex value)
     * must be exactly two digits. For example:  colorname#08
     */
    if (!alphaString.empty()) {
        if (alphaString.length() != 2)
            return false;

        unsigned char d1, d2;
        if (!HexDigitDecode(alphaString.at(0), d1) || 
            !HexDigitDecode(alphaString.at(1), d2))
            return false;
        colorTemp |= ((d1 & 0xf) << 4 | (d2 & 0xf)) << 24;
    }
    else {
        colorTemp |= 0xff << 24;  // Fully opaque
    }

    color = SColorF(colorTemp);

    return true;
}


//	The following function was found on http://xoomer.virgilio.it/acantato/dev/wildcard/wildmatch.html, 
//	where it was attributed to the C/C++ Users Journal, written by Mike Cornelison. It is a little ugly, 
//	but it is FAST.

// Does a classic * & ? pattern match on a file name - this is case sensitive!
bool WildcardMatch(const wchar_t *pat, const wchar_t *str)
{
	int i, star;

newsegment:

	star = 0;
	if (*pat == '*') {
		star = 1;
		do { pat++; } while (*pat == '*'); // enddo
	} // endif 

testmatch:

	for (i = 0; pat[i] && (pat[i] != '*'); i++) {
		//if (mapCaseTable[str[i]] != mapCaseTable[pat[i]]) {
		if (str[i] != pat[i]) {
			if (!str[i]) return 0;
			if ((pat[i] == '?') && (str[i] != '.')) continue;
			if (!star) return 0;
			str++;
			goto testmatch;
		}
	}
	if (pat[i] == '*') {
		str += i;
		pat += i;
		goto newsegment;
	}
	if (!str[i]) return 1;
	if (i && pat[i - 1] == '*') return 1;
	if (!star) return 0;
	str++;
	goto testmatch;
}

void* HashedString::HashName(char const * pIdentStr)
{
	// Relatively simple hash of arbitrary text string into a
	// 32-bit identifier Output value is
	// input-valid-deterministic, but no guarantees are made
	// about the uniqueness of the output per-input
	//
	// Input value is treated as lower-case to cut down on false
	// separations cause by human mistypes. Sure, it could be
	// construed as a programming error to mix up your cases, and
	// it cuts down on permutations, but in Real World Usage
	// making this text case-sensitive will likely just lead to
	// Pain and Suffering.
	//
	// This code lossely based upon the adler32 checksum by Mark
	// Adler and published as part of the zlib compression
	// library sources.

	// largest prime smaller than 65536
	unsigned long BASE = 65521L;

	// NMAX is the largest n such that 255n(n+1)/2 +
	// (n+1)(BASE-1) <= 2^32-1
	unsigned long NMAX = 5552;

#define DO1(buf,i)  {s1 += tolower(buf[i]); s2 += s1;}
#define DO2(buf,i)  DO1(buf,i); DO1(buf,i+1);
#define DO4(buf,i)  DO2(buf,i); DO2(buf,i+2);
#define DO8(buf,i)  DO4(buf,i); DO4(buf,i+4);
#define DO16(buf)   DO8(buf,0); DO8(buf,8);

	if (pIdentStr == NULL)
		return NULL;

	unsigned long s1 = 0;
	unsigned long s2 = 0;

	for (size_t len = strlen(pIdentStr); len > 0; )
	{
		unsigned long k = len < NMAX ? len : NMAX;

		len -= k;

		while (k >= 16)
		{
			DO16(pIdentStr);
			pIdentStr += 16;
			k -= 16;
		}

		if (k != 0) do
		{
			s1 += tolower(*pIdentStr++);
			s2 += s1;
		} while (--k);

		s1 %= BASE;
		s2 %= BASE;
	}

#pragma warning(push)
#pragma warning(disable : 4312)

	return reinterpret_cast<void *>((s2 << 16) | s1);

#pragma warning(pop)
#undef DO1
#undef DO2
#undef DO4
#undef DO8
#undef DO16
}