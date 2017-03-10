//
//  SuperTuxKart - a fun racing game with go-kart
//  Copyright (C) 2006-2013 SuperTuxKart-Team
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

#ifndef LEVELMANAGER_H
#define LEVELMANAGER_H

#include "GameEngineStd.h"

#include "Level.h"

/**
  * \brief Simple class to load and manage level data, level names and such
  * \ingroup tracks
  */
class LevelManager
{
public:
	LevelManager();
	~LevelManager();

    static void AddLevelSearchDir(const eastl::string &dir);
    /** Returns a list of all level identifiers. */
    eastl::vector<eastl::string> GetAllLevelIdentifiers();

    /** Load all level files from all directories */
    void  LoadLevelList(const eastl::string& levelname);
    void  RemoveLevel(const eastl::string &ident);
    bool  LoadLevel(const eastl::string& dirname);
    void  RemoveAllCachedData();
    Level* GetLevel(const eastl::string& ident) const;
    // ------------------------------------------------------------------------
    /** Sets a list of levels as being unavailable (e.g. in network mode the
     *  demo is not on all connected machines.
     *  \param level List of levels to mark as unavilable. */
    void SetUnavailableLevels(const eastl::vector<eastl::string> &levels);
    // ------------------------------------------------------------------------
    /** \brief Returns a list of all directories that contain a level. */
    const eastl::vector<eastl::string>* GetAllLevelDirs() const
    {
        return &m_all_level_dirs;
    }   // getAllLevelDirs
    // ------------------------------------------------------------------------
    /** \brief Returns a list of the names of all used level groups. */
    const eastl::vector<eastl::string>& GetAllLevelGroups() const
    {
        return m_level_group_names;
    }   // getAllLevelGroups
    // ------------------------------------------------------------------------
    /** Returns the number of levels. */
    size_t GetNumberOfLevels() const { return m_levels.size(); }
    // ------------------------------------------------------------------------
    /** Returns the level view with a given index number.
     *  \param index The index number of the level. */
    Level* GetLevel(unsigned int index) const { return m_levels[index];}
    // ------------------------------------------------------------------------
    /** Checks if a certain level is available.
     *  \param n Index of the level to check. */
    bool IsAvailable(unsigned int n) const {return m_level_avail[n];}
    // ------------------------------------------------------------------------
    /** Returns a list of all levels in a given group.
     *  \param g Name of the group. */
    const eastl::vector<int>& GetLevelInGroup(const eastl::string& g)
    {
        return m_level_groups[g];
    }   // getLevelInGroup

private:
	/** All directories in which level are searched. */
	static eastl::vector<eastl::string> m_level_search_path;

	/** All directories in which levels were found. */
	eastl::vector<eastl::string> m_all_level_dirs;

	typedef eastl::vector<Level*> LevelList;

	/** All track objects. */
	LevelList m_levels;

	typedef eastl::map<eastl::string, eastl::vector<int> > Group2Indices;
	/** List of all level groups. */
	Group2Indices m_level_groups;

	/** List of all groups (for any kind of level) */
	//eastl::vector<eastl::string> m_all_group_names;

	/** List of the names of all groups containing levels */
	eastl::vector<eastl::string> m_level_group_names;

	/** Flag if this level is available or not. Levels are set unavailable
	*  if they are not available on all clients (applies only to network mode)
	*/
	eastl::vector<bool> m_level_avail;

	void UpdateGroups(const Level* demo);
};   // LevelManager

extern LevelManager* level_manager;

#endif   // HEADER_TRACK_MANAGER_HPP
