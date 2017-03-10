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
    int m_from, m_to;
    eastl::wstring m_text;

    Subtitle(int from, int to, eastl::wstring text)
    {
        m_from = from;
        m_to = to;
        m_text = text;
    }
    int getFrom() const { return m_from; }
    int getTo()   const { return m_to;   }
    const eastl::wstring& getText() const { return m_text; }
};

/**
  * \ingroup levels
  */
class Level
{

public:

	Level(const eastl::string& filename);
	~Level();
    void Update(float dt);
    void Reset();
    // ------------------------------------------------------------------------
    /** Returns the length of the main driveline. */
    //float getDemoLength() const {return QuadGraph::get()->getLapLength();}
    // ------------------------------------------------------------------------
    /** Returns a unique identifier for this level (the directory name). */
    const eastl::string& getIdent() const {return m_ident;}
    // ------------------------------------------------------------------------
	/** Returns all groups this level belongs to. */
	const eastl::vector<eastl::string>& GetGroups() const { return m_groups; }
	// ------------------------------------------------------------------------
    /** Returns the filename of this level. */
    const eastl::string& getFilename() const { return m_filename; }

private:

	eastl::string m_ident;
	eastl::string m_screenshot;
	double m_gravity;

	eastl::vector<Subtitle> m_subtitles;

	eastl::vector<eastl::string> m_groups;

	/** The full filename of the config (xml) file. */
	eastl::string m_filename;

	/** A simple class to keep information about a level mode. */
	class LevelMode
	{
	public:
		eastl::string m_name; /* Name / description of this mode. */
		eastl::string m_scene; /* Name of the scene file to use.   */

		/** Default constructor, sets default names for all fields. */
		LevelMode()
			: m_name("default"), m_scene("scene.xml")
		{

		}

		~LevelMode()
		{

		}

	};   // LevelMode

		 /** Name of the level to display. */
	eastl::string m_name;

	void LoadLevelInfo();
	bool LoadMainLevel(const XMLElement* pNode);

};   // class Level

#endif