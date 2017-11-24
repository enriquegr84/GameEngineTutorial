//  SuperTuxKart - a fun racing game with go-kart
//
//  Copyright (C) 2004-2013 Steve Baker <sjbaker1@airmail.net>
//  Copyright (C) 2009-2013  Joerg Henrichs, Steve Baker
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
    int mFrom, mTo;
    eastl::wstring mText;

    Subtitle(int from, int to, eastl::wstring text)
    {
        mFrom = from;
        mTo = to;
        mText = text;
    }
    int getFrom() const { return mFrom; }
    int getTo()   const { return mTo;   }
    const eastl::wstring& getText() const { return mText; }
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
	/** Returns all groups this level belongs to. */
	const eastl::vector<eastl::wstring>& GetGroups() const { return mGroups; }
	// ------------------------------------------------------------------------
    /** Returns the filename of this level. */
    const eastl::wstring& GetFilename() const { return mFileName; }

private:

	/** A simple class to keep information about a level mode. */
	class LevelMode
	{
	public:
		eastl::wstring mName; /* Name / description of this mode. */
		eastl::wstring mScene; /* Name of the scene file to use.   */

							   /** Default constructor, sets default names for all fields. */
		LevelMode()
			: mName(L"default"), mScene(L"scene.xml")
		{

		}

		~LevelMode()
		{

		}

	};   // LevelMode


	eastl::wstring mID;
	eastl::wstring mScreenShot;
	double mGravity;

	eastl::vector<Subtitle> mSubtitles;

	eastl::vector<eastl::wstring> mGroups;

	/** The full filename of the config (xml) file. */
	eastl::wstring mFileName;

		 /** Name of the level to display. */
	eastl::wstring mName;

	void LoadLevelInfo();
	bool LoadMainLevel(const XMLElement* pNode);

};   // class Level

#endif