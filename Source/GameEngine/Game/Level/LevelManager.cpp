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

#include "LevelManager.h"

#include "Core/IO/FileSystem.h"
#include "Core/IO/ResourceCache.h"
#include "Core/Logger/Logger.h"

LevelManager* LevelManager::LevelMngr = NULL;
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
/** Adds a directory from which levels are loaded. The demos manager checks if
 *  either this directory itself contains a level, and if any subdirectory
 *  contains a level.
 *  \param dir The directory to add.
 */
void LevelManager::AddLevelSearchDir(const eastl::wstring &dir)
{
    mLevelSearchPaths.push_back(dir);
}   // addLevelSearchDir

//-----------------------------------------------------------------------------
/** Get LevelData by the level identifier.
 *  \param ident Identifier = basename of the directory the level is in.
 *  \return      The corresponding level object, or NULL if not found
 */
Level* LevelManager::GetLevel(const eastl::wstring& ident) const
{
    for(LevelList::const_iterator i = mLevels.begin(); i != mLevels.end(); ++i)
    {
        if ((*i)->GetID() == ident)
            return *i;
    }

    return NULL;

}   // getTrack

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
}   // removeAllCachedData
//-----------------------------------------------------------------------------
/** Sets all levels that are not in the list a to be unavailable. This is used
 *  by the network manager upon receiving the list of available levels from
 *  a client.
 *  \param demos List of all levels identifiers (available on a client).
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
            fwprintf(stderr,
				L"Demo '%s' not available on all clients, disabled.\n", id.c_str());
        }   // if id not in tracks
    }   // for all available tracks in track manager

}   // setUnavailableLevels

//-----------------------------------------------------------------------------
/** Returns a list with all level identifiers.
 */
eastl::vector<eastl::wstring> LevelManager::GetAllLevelIdentifiers()
{
    eastl::vector<eastl::wstring> all;
    for(LevelList::const_iterator i = mLevels.begin(); i != mLevels.end(); ++i)
    {
        all.push_back((*i)->GetID());
    }
    return all;
}   // getAllDemoNames

//-----------------------------------------------------------------------------
/** Loads all levels from the level directory (world/).
 */
void LevelManager::LoadLevelList(const eastl::wstring& levelname)
{
    mAllLevelDirs.clear();
    mLevelGroupNames.clear();
    mLevelGroups.clear();
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
        for(eastl::vector<eastl::wstring>::iterator itFile = files.begin(); 
			itFile != files.end(); itFile++)
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
    catch(std::exception) //(std::exception& e)
    {
		/*
        fprintf(stderr, "[LevelManager] ERROR: Cannot load level <%s> : %s\n",
			Utils::GetFileBasename(levelname).c_str(), e.what());
		*/
        return false;
    }
	/*
    if (demo->getVersion()<stkConfig->mMinTrackVersion ||
        demo->getVersion()>stkConfig->mMaxTrackVersion)
    {
        fprintf(stderr, "[TrackManager] Warning: track '%s' is not supported "
                        "by this binary, ignored. (Track is version %i, this "
                        "executable supports from %i to %i)\n",
                track->getIdent().c_str(), track->getVersion(),
                stkConfig->mMinTrackVersion,
                stkConfig->mMaxTrackVersion);
        delete track;
        return false;
    }
	*/
	mAllLevelDirs.push_back(FileSystem::Get()->GetFileDir(levelname));
    mLevels.push_back(level);
    mLevelAvailables.push_back(true);
    UpdateGroups(level);
    return true;
}   // loadTrack

// ----------------------------------------------------------------------------
/** Removes a level.
 *  \param ident Identifier of the level (i.e. the name of the directory).
 */
void LevelManager::RemoveLevel(const eastl::wstring& ident)
{
    Level* level = GetLevel(ident);
    if (level == NULL)
    {
		wchar_t error[128];
        wsprintf(error, L"[LevelManager] ERROR: There is no level named '%s'!!\n", ident.c_str());
        LogError(error);
        return;
    }

    //if (level->isInternal()) return;

    eastl::vector<Level*>::iterator it = eastl::find(mLevels.begin(), mLevels.end(), level);
    if (it == mLevels.end())
    {
		wchar_t error[128];
		wsprintf(error, L"[LevelsManager] INTERNAL ERROR: Cannot find level '%s' in map!!\n", ident.c_str());
		LogError(error);
        return;
    }
    int index = it - mLevels.begin();

    // Remove the demo from all groups it belongs to
    Group2Indices &groupToIndices = mLevelGroups;

    eastl::vector<eastl::wstring> &groupNames = mLevelGroupNames;
    const eastl::vector<eastl::wstring>& groups = level->GetGroups();
    for(unsigned int i=0; i<groups.size(); i++)
    {
        eastl::vector<int> &indices = groupToIndices[groups[i]];
        eastl::vector<int>::iterator j;
        j = eastl::find(indices.begin(), indices.end(), index);
        LogAssert(j!=indices.end(), "error indice");
        indices.erase(j);

        // If the demo was the last member of a group,
        // completely remove the group
        if(indices.size()==0)
        {
            groupToIndices.erase(groups[i]);
            eastl::vector<eastl::wstring>::iterator it_g;
            it_g = eastl::find(groupNames.begin(), groupNames.end(), groups[i]);
			LogAssert(it_g!=groupNames.end(), "error indice");
            groupNames.erase(it_g);
        }   // if complete group must be removed
    }   // for i in groups

    // Adjust all indices of level with an index number higher than
    // the removed level, since they have been moved down. This must
    // be done for all levels
    unsigned int i=2; // i=2: levels
    {
        Group2Indices &g2i = mLevelGroups;
        Group2Indices::iterator j;
        for(j=g2i.begin(); j!=g2i.end(); j++)
        {
            for(unsigned int i=0; i<(*j).second.size(); i++)
                if((*j).second[i]>index) (*j).second[i]--;
        }   // for j in group_2_indices
    }   // for levels

    mLevels.erase(it);
    mAllLevelDirs.erase(mAllLevelDirs.begin()+index);
    mLevelAvailables.erase(mLevelAvailables.begin()+index);
    delete level;
}   // removeDemo

// ----------------------------------------------------------------------------
/** \brief Updates the groups after a level was read in.
  * \param demo Pointer to the new level, whose groups are now analysed.
  */
void LevelManager::UpdateGroups(const Level* level)
{
    //if (level->isInternal()) return;

    const eastl::vector<eastl::wstring>& newGroups = level->GetGroups();

    Group2Indices &groupToIndices = mLevelGroups;

    eastl::vector<eastl::wstring> &groupNames = mLevelGroupNames;

    const unsigned int groupsAmount = newGroups.size();
    for(unsigned int i=0; i<groupsAmount; i++)
    {
        bool groupExists = 
			groupToIndices.find(newGroups[i]) != groupToIndices.end();
        if(!groupExists)
            groupNames.push_back(newGroups[i]);
        groupToIndices[newGroups[i]].push_back(mLevels.size()-1);
    }
}   // updateGroups

// ----------------------------------------------------------------------------
