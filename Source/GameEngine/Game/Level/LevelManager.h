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

    static void AddLevelSearchDir(const eastl::wstring &dir);
    /** Returns a list of all level identifiers. */
    eastl::vector<eastl::wstring> GetAllLevelIdentifiers();

	/* Level */
	virtual bool LoadLevel(const eastl::wstring& dirname);
	Level* GetLevel(const eastl::wstring& ident) const;
	void RemoveLevel(const eastl::wstring &ident);

    /** Load all level files from all directories */
    void  LoadLevelList(const eastl::wstring& levelname);
    void  RemoveAllCachedData();
    // ------------------------------------------------------------------------
    /** Sets a list of levels as being unavailable
     *  \param level List of levels to mark as unavilable. */
    void SetUnavailableLevels(const eastl::vector<eastl::wstring> &levels);
    // ------------------------------------------------------------------------
    /** \brief Returns a list of all directories that contain a level. */
    const eastl::vector<eastl::wstring>* GetAllLevelDirs() const
    {
        return &mAllLevelDirs;
    }   // GetAllLevelDirs
    // ------------------------------------------------------------------------
    /** Returns the number of levels. */
    size_t GetNumberOfLevels() const { return mLevels.size(); }
    // ------------------------------------------------------------------------
	// ------------------------------------------------------------------------
	/** Returns the number of levels. */
	eastl::vector<Level*> GetLevels() const { return mLevels; }
	// ------------------------------------------------------------------------
    /** Returns the level view with a given index number.
     *  \param index The index number of the level. */
    Level* GetLevel(unsigned int index) const { return mLevels[index];}
    // ------------------------------------------------------------------------
    /** Checks if a certain level is available.
     *  \param n Index of the level to check. */
    bool IsAvailable(unsigned int n) const {return mLevelAvailables[n];}
    // ------------------------------------------------------------------------

protected:

	/** All directories in which level are searched. */
	static eastl::vector<eastl::wstring> mLevelSearchPaths;

	/** All directories in which levels were found. */
	eastl::vector<eastl::wstring> mAllLevelDirs;

	typedef eastl::vector<Level*> LevelList;

	/** All track objects. */
	LevelList mLevels;

	/** Flag if this level is available or not. Levels are set unavailable
	*  if they are not available
	*/
	eastl::vector<bool> mLevelAvailables;

};   // LevelManager

#endif
