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

#include "LevelManager.h"

#include "Core/IO/FileSystem.h"
#include "Core/IO/ResourceCache.h"
#include "Core/Logger/Logger.h"


eastl::vector<eastl::wstring> LevelManager::mLevelSearchPaths;

/** Constructor (currently empty). The real work happens in loadLevelList.
 */
LevelManager::LevelManager()
{

} // LevelManager

//-----------------------------------------------------------------------------
/*
Delete all levels.
 */
LevelManager::~LevelManager()
{
    for(LevelList::iterator i = mLevels.begin(); i != mLevels.end(); ++i)
        delete *i;
}// ~LevelManager

//-----------------------------------------------------------------------------
/** Adds a directory from which levels are loaded. The manager checks if
 *  either this directory itself contains a level, and if any subdirectory
 *  contains a level.
 *  \param dir The directory to add.
 */
void LevelManager::AddLevelSearchDir(const eastl::wstring &dir)
{
    mLevelSearchPaths.push_back(dir);
}   // addLevelSearchDir

//-----------------------------------------------------------------------------
/** Get LevelData by the level id.
 *  \param id = basename of the directory the level is in.
 *  \return The corresponding level object, or NULL if not found
 */
Level* LevelManager::GetLevel(const eastl::wstring& id) const
{
    for(LevelList::const_iterator i = mLevels.begin(); i != mLevels.end(); ++i)
    {
        if ((*i)->GetID() == id)
            return *i;
    }

    return NULL;

}   // GetLevel

//-----------------------------------------------------------------------------
/** Removes all cached data from all levels. This is called when the screen
 *  resolution is changed and all textures need to be bound again.
 */
void LevelManager::RemoveAllCachedData()
{
/*
    for(LevelList::const_iterator i = mLevels.begin(); i != mLevels.end(); ++i)
        (*i)->RemoveCachedData();
*/
}   // RemoveAllCachedData
//-----------------------------------------------------------------------------
/** Sets all levels that are not in the list a to be unavailable.
 *  \param demos List of all levels id (available on a client).
 */
void LevelManager::SetUnavailableLevels(const eastl::vector<eastl::wstring> &levels)
{
    for(LevelList::const_iterator i = mLevels.begin(); i != mLevels.end(); ++i)
    {
        if(!mLevelAvailables[i-mLevels.begin()]) continue;
        const eastl::wstring id=(*i)->GetID();
        if (eastl::find(levels.begin(), levels.end(), id)==levels.end())
        {
            mLevelAvailables[i-mLevels.begin()] = false;
            LogError(L"Level " + id + L"not available on all clients, disabled");
        } 
    } 

}   // SetUnavailableLevels

//-----------------------------------------------------------------------------
/** Returns a list with all level ids.
 */
eastl::vector<eastl::wstring> LevelManager::GetAllLevelIds()
{
    eastl::vector<eastl::wstring> all;
    for(LevelList::const_iterator i = mLevels.begin(); i != mLevels.end(); ++i)
    {
        all.push_back((*i)->GetID());
    }
    return all;
}   // GetAllLevelIds

//-----------------------------------------------------------------------------
/** Loads all levels from the level directory (world/).
 */
void LevelManager::LoadLevelList(const eastl::wstring& levelname)
{
    mAllLevelDirs.clear();
    mLevelAvailables.clear();
    mLevels.clear();

    for(unsigned int i=0; i<mLevelSearchPaths.size(); i++)
    {
        const eastl::wstring &dir = mLevelSearchPaths[i];

        // First test if the directory itself contains a level:
        // ----------------------------------------------------
        //if(LoadLevel(dir)) continue;  // level found, no more tests

        // Then see if a subdir of this dir contains levels
        // ------------------------------------------------

		eastl::vector<eastl::wstring> files = ResCache::Get()->Match(dir + levelname);
		eastl::vector<eastl::wstring>::iterator itFile = files.begin();
        for(; itFile != files.end(); itFile++)
        {
            LoadLevel(*itFile);
        }   // for dir in dirs
    }
}  // loadLevelsList

// ----------------------------------------------------------------------------
/** Tries to load a level from a single directory. Returns true if a level was
 *  successfully loaded.
 *  \param dirname Name of the directory to load the level from.
 */
bool LevelManager::LoadLevel(const eastl::wstring& levelname)
{
	if (!FileSystem::Get()->ExistFile(levelname))
		return false;

    Level *level;

    try
    {
        level = new Level(levelname);
    }
    catch(std::exception)
    {
		LogError(L"Cannot load level " + levelname);
        return false;
    }

	mAllLevelDirs.push_back(FileSystem::Get()->GetFileDir(levelname));
    mLevels.push_back(level);
    mLevelAvailables.push_back(true);
    return true;
}   // LoadLevel

// ----------------------------------------------------------------------------
/** Removes a level.
 *  \param id of the level (i.e. the name of the directory).
 */
void LevelManager::RemoveLevel(const eastl::wstring& id)
{
    Level* level = GetLevel(id);
    if (level == NULL)
    {
		wchar_t error[128];
        wsprintf(error, L"There is no level named '%s'!!", id.c_str());
        LogError(error);
        return;
    }

    //if (level->isInternal()) return;

    eastl::vector<Level*>::iterator it = eastl::find(mLevels.begin(), mLevels.end(), level);
    if (it == mLevels.end())
    {
		wchar_t error[128];
		wsprintf(error, L" Cannot find level '%s' in map!!", id.c_str());
		LogError(error);
        return;
    }
    int index = it - mLevels.begin();

    mLevels.erase(it);
    mAllLevelDirs.erase(mAllLevelDirs.begin()+index);
    mLevelAvailables.erase(mLevelAvailables.begin()+index);
    delete level;
}   // RemoveLevel

// ----------------------------------------------------------------------------
