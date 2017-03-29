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

	static LevelManager* LevelMngr;

    static void AddLevelSearchDir(const eastl::wstring &dir);
    /** Returns a list of all level identifiers. */
    eastl::vector<eastl::wstring> GetAllLevelIdentifiers();

    /** Load all level files from all directories */
    void  LoadLevelList(const eastl::wstring& levelname);
    void  RemoveLevel(const eastl::wstring &ident);
    bool  LoadLevel(const eastl::wstring& dirname);
    void  RemoveAllCachedData();
    Level* GetLevel(const eastl::wstring& ident) const;
    // ------------------------------------------------------------------------
    /** Sets a list of levels as being unavailable (e.g. in network mode the
     *  demo is not on all connected machines.
     *  \param level List of levels to mark as unavilable. */
    void SetUnavailableLevels(const eastl::vector<eastl::wstring> &levels);
    // ------------------------------------------------------------------------
    /** \brief Returns a list of all directories that contain a level. */
    const eastl::vector<eastl::wstring>* GetAllLevelDirs() const
    {
        return &mAllLevelDirs;
    }   // getAllLevelDirs
    // ------------------------------------------------------------------------
    /** \brief Returns a list of the names of all used level groups. */
    const eastl::vector<eastl::wstring>& GetAllLevelGroups() const
    {
        return mLevelGroupNames;
    }   // getAllLevelGroups
    // ------------------------------------------------------------------------
    /** Returns the number of levels. */
    size_t GetNumberOfLevels() const { return mLevels.size(); }
    // ------------------------------------------------------------------------
    /** Returns the level view with a given index number.
     *  \param index The index number of the level. */
    Level* GetLevel(unsigned int index) const { return mLevels[index];}
    // ------------------------------------------------------------------------
    /** Checks if a certain level is available.
     *  \param n Index of the level to check. */
    bool IsAvailable(unsigned int n) const {return mLevelAvailables[n];}
    // ------------------------------------------------------------------------
    /** Returns a list of all levels in a given group.
     *  \param g Name of the group. */
    const eastl::vector<int>& GetLevelInGroup(const eastl::wstring& g)
    {
        return mLevelGroups[g];
    }   // getLevelInGroup

private:
	/** All directories in which level are searched. */
	static eastl::vector<eastl::wstring> mLevelSearchPaths;

	/** All directories in which levels were found. */
	eastl::vector<eastl::wstring> mAllLevelDirs;

	typedef eastl::vector<Level*> LevelList;

	/** All track objects. */
	LevelList mLevels;

	typedef eastl::map<eastl::wstring, eastl::vector<int> > Group2Indices;
	/** List of all level groups. */
	Group2Indices mLevelGroups;

	/** List of all groups (for any kind of level) */
	//eastl::vector<eastl::string> mAllGroupNames;

	/** List of the names of all groups containing levels */
	eastl::vector<eastl::wstring> mLevelGroupNames;

	/** Flag if this level is available or not. Levels are set unavailable
	*  if they are not available on all clients (applies only to network mode)
	*/
	eastl::vector<bool> mLevelAvailables;

	void UpdateGroups(const Level* demo);

};   // LevelManager

#endif
