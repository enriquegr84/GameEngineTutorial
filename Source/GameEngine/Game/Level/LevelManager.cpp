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
#include "Level.h"

#include "GameEngine/GameEngine.h"
/*
#include "audio/music_manager.hpp"
#include "config/stk_config.hpp"
#include "io/file_manager.hpp"
#include "tracks/track.hpp"
*/

LevelManager* level_manager = 0;
eastl::vector<eastl::string>  LevelManager::m_level_search_path;

/** Constructor (currently empty). The real work happens in loadLevelList.
 */
LevelManager::LevelManager()
{}   // LevelManager

//-----------------------------------------------------------------------------
/** Delete all levels.
 */
LevelManager::~LevelManager()
{
    for(LevelList::iterator i = m_levels.begin(); i != m_levels.end(); ++i)
        SAFE_DELETE( *i );
}   // ~TrackManager

//-----------------------------------------------------------------------------
/** Adds a directory from which levels are loaded. The demos manager checks if
 *  either this directory itself contains a level, and if any subdirectory
 *  contains a level.
 *  \param dir The directory to add.
 */
void LevelManager::addLevelSearchDir(const eastl::string &dir)
{
    m_level_search_path.push_back(dir);
}   // addLevelSearchDir

//-----------------------------------------------------------------------------
/** Get LevelData by the level identifier.
 *  \param ident Identifier = basename of the directory the level is in.
 *  \return      The corresponding level object, or NULL if not found
 */
Level* LevelManager::getLevel(const eastl::string& ident) const
{
    for(LevelList::const_iterator i = m_levels.begin(); i != m_levels.end(); ++i)
    {
        if ((*i)->getIdent() == ident)
            return *i;
    }

    return NULL;

}   // getTrack

//-----------------------------------------------------------------------------
/** Removes all cached data from all levels. This is called when the screen
 *  resolution is changed and all textures need to be bound again.
 */
void LevelManager::removeAllCachedData()
{
/*
    for(LevelList::const_iterator i = m_levels.begin(); i != m_levels.end(); ++i)
        (*i)->removeCachedData();
*/
}   // removeAllCachedData
//-----------------------------------------------------------------------------
/** Sets all levels that are not in the list a to be unavailable. This is used
 *  by the network manager upon receiving the list of available levels from
 *  a client.
 *  \param demos List of all levels identifiers (available on a client).
 */
void LevelManager::setUnavailableLevels(const eastl::vector<eastl::string> &levels)
{
    for(LevelList::const_iterator i = m_levels.begin(); i != m_levels.end(); ++i)
    {
        if(!m_level_avail[i-m_levels.begin()]) continue;
        const eastl::string id=(*i)->getIdent();
        if (eastl::find(levels.begin(), levels.end(), id)==levels.end())
        {
            m_level_avail[i-m_levels.begin()] = false;
            fprintf(stderr,
                    "Demo '%s' not available on all clients, disabled.\n",
                    id.c_str());
        }   // if id not in tracks
    }   // for all available tracks in track manager

}   // setUnavailableLevels

//-----------------------------------------------------------------------------
/** Returns a list with all level identifiers.
 */
eastl::vector<eastl::string> LevelManager::getAllLevelIdentifiers()
{
    eastl::vector<eastl::string> all;
    for(LevelList::const_iterator i = m_levels.begin(); i != m_levels.end(); ++i)
    {
        all.push_back((*i)->getIdent());
    }
    return all;
}   // getAllDemoNames

//-----------------------------------------------------------------------------
/** Loads all levels from the level directory (world/).
 */
void LevelManager::loadLevelList(const eastl::string& levelname)
{
    m_all_level_dirs.clear();
    m_level_group_names.clear();
    m_level_groups.clear();
    m_level_avail.clear();
    m_levels.clear();

    for(unsigned int i=0; i<m_level_search_path.size(); i++)
    {
        const eastl::string &dir = m_level_search_path[i];

        // First test if the directory itself contains a level:
        // ----------------------------------------------------
        //if(loadLevel(dir)) continue;  // level found, no more tests

        // Then see if a subdir of this dir contains levels
        // ------------------------------------------------
        eastl::vector<eastl::wstring> files = 
			g_pGameApp->m_ResCache->Match(eastl::string("world/*/") + levelname);

        for(eastl::vector<eastl::wstring>::iterator itFile = files.begin(); 
			itFile != files.end(); itFile++)
        {
            loadLevel(*itFile);
        }   // for dir in dirs
    }   // for i <m_level_search_path.size()
}  // loadLevelsList

// ----------------------------------------------------------------------------
/** Tries to load a level from a single directory. Returns true if a level was
 *  successfully loaded.
 *  \param dirname Name of the directory to load the level from.
 */
bool LevelManager::loadLevel(const eastl::string& levelname)
{
    eastl::string config_file = levelname;
	if (!g_pGameApp->m_pFileSystem->ExistFile(config_file))
        return false;

    Level *level;

    try
    {
        level = new Level(config_file);
    }
    catch (std::exception& e)
    {
        fprintf(stderr, "[LevelManager] ERROR: Cannot load level <%s> : %s\n",
			Utils::GetFileBasename(levelname).c_str(), e.what());
        return false;
    }
	/*
    if (demo->getVersion()<stk_config->m_min_track_version ||
        demo->getVersion()>stk_config->m_max_track_version)
    {
        fprintf(stderr, "[TrackManager] Warning: track '%s' is not supported "
                        "by this binary, ignored. (Track is version %i, this "
                        "executable supports from %i to %i)\n",
                track->getIdent().c_str(), track->getVersion(),
                stk_config->m_min_track_version,
                stk_config->m_max_track_version);
        delete track;
        return false;
    }
	*/
	m_all_level_dirs.push_back(g_pGameApp->m_pFileSystem->GetFileDir(levelname));
    m_levels.push_back(level);
    m_level_avail.push_back(true);
    updateGroups(level);
    return true;
}   // loadTrack

// ----------------------------------------------------------------------------
/** Removes a level.
 *  \param ident Identifier of the level (i.e. the name of the directory).
 */
void LevelManager::removeLevel(const eastl::string& ident)
{
    Level* level = getLevel(ident);
    if (level == NULL)
    {
        fprintf(stderr, "[LevelManager] ERROR: There is no level named '%s'!!\n", ident.c_str());
        GE_ASSERT(false);
        return;
    }

    //if (level->isInternal()) return;

    eastl::vector<Level*>::iterator it = eastl::find(m_levels.begin(), m_levels.end(), level);
    if (it == m_levels.end())
    {
        fprintf(stderr, 
			"[LevelsManager] INTERNAL ERROR: Cannot find level '%s' in map!!\n", ident.c_str());
        GE_ASSERT(false);
        return;
    }
    int index = it - m_levels.begin();

    // Remove the demo from all groups it belongs to
    Group2Indices &group_2_indices = m_level_groups;

    eastl::vector<eastl::string> &group_names = m_level_group_names;

    const eastl::vector<eastl::string>& groups = level->getGroups();
    for(unsigned int i=0; i<groups.size(); i++)
    {
        eastl::vector<int> &indices = group_2_indices[groups[i]];
        eastl::vector<int>::iterator j;
        j = eastl::find(indices.begin(), indices.end(), index);
        GE_ASSERT(j!=indices.end());
        indices.erase(j);

        // If the demo was the last member of a group,
        // completely remove the group
        if(indices.size()==0)
        {
            group_2_indices.erase(groups[i]);
            eastl::vector<eastl::string>::iterator it_g;
            it_g = eastl::find(group_names.begin(), group_names.end(), groups[i]);
            GE_ASSERT(it_g!=group_names.end());
            group_names.erase(it_g);
        }   // if complete group must be removed
    }   // for i in groups

    // Adjust all indices of level with an index number higher than
    // the removed level, since they have been moved down. This must
    // be done for all levels
    unsigned int i=2; // i=2: levels
    {
        Group2Indices &g2i = m_level_groups;
        Group2Indices::iterator j;
        for(j=g2i.begin(); j!=g2i.end(); j++)
        {
            for(unsigned int i=0; i<(*j).second.size(); i++)
                if((*j).second[i]>index) (*j).second[i]--;
        }   // for j in group_2_indices
    }   // for levels

    m_levels.erase(it);
    m_all_level_dirs.erase(m_all_level_dirs.begin()+index);
    m_level_avail.erase(m_level_avail.begin()+index);
    SAFE_DELETE( level );
}   // removeDemo

// ----------------------------------------------------------------------------
/** \brief Updates the groups after a level was read in.
  * \param demo Pointer to the new level, whose groups are now analysed.
  */
void LevelManager::updateGroups(const Level* level)
{
    //if (level->isInternal()) return;

    const eastl::vector<eastl::string>& new_groups = level->getGroups();

    Group2Indices &group_2_indices = m_level_groups;

    eastl::vector<eastl::string> &group_names = m_level_group_names;

    const unsigned int groups_amount = new_groups.size();
    for(unsigned int i=0; i<groups_amount; i++)
    {
        bool group_exists = 
			group_2_indices.find(new_groups[i]) != group_2_indices.end();
        if(!group_exists)
            group_names.push_back(new_groups[i]);
        group_2_indices[new_groups[i]].push_back(m_levels.size()-1);
    }
}   // updateGroups

// ----------------------------------------------------------------------------
