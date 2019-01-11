//
//  This program is free software; you can redistribute it and/or
//  modify it under the terms of the GNU General Public License
//  as published by the Free Software Foundation; either version 3
//  of the License, or (at your option) any later version.
//
//  This program is distributed in the hope that it will be useful,
//  but WITHOUT ANY WARRANTY; without even the implied warranty of
//  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//  GNU General Public License for more details.
//
//  You should have received a copy of the GNU General Public License
//  along with this program; if not, write to the Free Software
//  Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.

#ifndef LEVEL_H
#define LEVEL_H

#include "GameEngineStd.h"

/**
  * \defgroup demos
  * Contains information about demos
  * objects.
*/

struct Subtitle
{
    Subtitle(int from, int to, eastl::wstring text)
    {
        mFrom = from;
        mTo = to;
        mText = text;
    }

	int mFrom, mTo;
	eastl::wstring mText;
};

/**
  * \ingroup levels
  */
class Level
{

public:

	Level(const eastl::wstring& filename);
	~Level();
    void Update(float dt);
    void Reset();

    // ------------------------------------------------------------------------
    /** Returns a unique identifier for this level (the directory name). */
    const eastl::wstring& GetID() const {return mID;}
	// ------------------------------------------------------------------------
	/** Returns a unique identifier for this level (the directory name). */
	const eastl::wstring& GetName() const { return mName; }
	// ------------------------------------------------------------------------
    /** Returns the filename of this level. */
    const eastl::wstring& GetFileName() const { return mFileName; }

protected:

	/** A simple class to keep information about a level mode. */
	class LevelMode
	{
	public:
		eastl::wstring mName; /* Name / description of this mode. */
		eastl::wstring mScene; /* Name of the scene file to use.   */

							   /** Default constructor, sets default names for all fields. */
		LevelMode()
			: mName(L"default"), mScene(L"default")
		{

		}

		~LevelMode()
		{

		}

	};   // LevelMode


	eastl::wstring mID;
	eastl::wstring mScreenShot;

	eastl::vector<Subtitle> mSubtitles;

	/** The full filename of the config (xml) file. */
	eastl::wstring mFileName;

		 /** Name of the level to display. */
	eastl::wstring mName;

	void LoadLevelInfo();
	bool LoadMainLevel(const tinyxml2::XMLElement* pNode);

};   // class Level

#endif