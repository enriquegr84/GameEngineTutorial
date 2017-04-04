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

#include "GameDemoManager.h"
#include "GameDemoApp.h"

#include "Game/Game.h"
/*
#include "audio/music_manager.hpp"
#include "config/stk_config.hpp"
#include "io/file_manager.hpp"
#include "tracks/track.hpp"
*/

eastl::vector<eastl::wstring>  GameDemoManager::mGameDemoSearchPath;

/** Constructor (currently empty). The real work happens in loadTrackList.
 */
GameDemoManager::GameDemoManager()
{

}   // GameDemoManager

//-----------------------------------------------------------------------------
/** Delete all demos.
 */
GameDemoManager::~GameDemoManager()
{
	/*
    for(GameDemoList::iterator it = mGameDemos.begin(); it != mGameDemos.end(); ++it)
        delete (*it);
	*/
}   // ~GameDemoManager

//-----------------------------------------------------------------------------
/** Adds a directory from which demos are loaded. The demos manager checks if
 *  either this directory itself contains a demo, and if any subdirectory
 *  contains a demo.
 *  \param dir The directory to add.
 */
void GameDemoManager::AddGameDemoSearchDir(const eastl::wstring &dir)
{
	mGameDemoSearchPath.push_back(dir);
}   // addTrackDir

//-----------------------------------------------------------------------------
/** Get DemoData by the demo identifier.
 *  \param ident Identifier = basename of the directory the demo is in.
 *  \return      The corresponding demo object, or NULL if not found
 */
GameDemo* GameDemoManager::GetGameDemo(const eastl::wstring& ident) const
{
	/*
    for(GameDemoList::const_iterator it = mGameDemos.begin(); it != mGameDemos.end(); ++it)
    {
        if ((*it)->GetIdent() == ident)
            return *it;
    }
	*/
    return NULL;

}   // getTrack

//-----------------------------------------------------------------------------
/** Removes all cached data from all demos. This is called when the screen
 *  resolution is changed and all textures need to be bound again.
 */
void GameDemoManager::RemoveAllCachedData()
{
	/*
	for (GameDemoList::const_iterator it = mGameDemos.begin(); it != mGameDemos.end(); ++it)
        (*it)->RemoveCachedData();
	*/
}   // removeAllCachedData
//-----------------------------------------------------------------------------
/** Sets all demos that are not in the list a to be unavailable. This is used
 *  by the network manager upon receiving the list of available demos from
 *  a client.
 *  \param demos List of all demos identifiers (available on a client).
 */
void GameDemoManager::SetUnavailableGameDemos(const eastl::vector<eastl::wstring> &demos)
{
	/*
	for (GameDemoList::const_iterator it = mGameDemos.begin(); it != mGameDemos.end(); ++it)
    {
        if(!mGameDemosAvailable[it - mGameDemos.begin()]) continue;
        const eastl::string id=(*it)->GetIdent();
        if (eastl::find(demos.begin(), demos.end(), id)==demos.end())
        {
			mGameDemosAvailable[it - mGameDemos.begin()] = false;
            fprintf(stderr, "GameDemo '%s' not available on all clients, disabled.\n", id.c_str());
        }   // if id not in tracks
    }   // for all available tracks in track manager
	*/
}   // setUnavailableDemos

//-----------------------------------------------------------------------------
/** Returns a list with all demos identifiers.
 */
eastl::vector<eastl::wstring> GameDemoManager::GetAllGameDemoIdentifiers()
{
    eastl::vector<eastl::wstring> all;
	/*
	for (GameDemoList::const_iterator it = mGameDemos.begin(); it != mGameDemos.end(); ++it)
	{
        all.push_back((*it)->GetIdent());
    }
	*/
    return all;
}   // GetAllGameDemoIdentifiers

//-----------------------------------------------------------------------------
/** Loads all demos from the track directory (data/demo).
 */
void GameDemoManager::LoadGameDemosList()
{
    mAllGameDemoDirs.clear();
    mGameDemoGroupNames.clear();
    mGameDemoGroups.clear();
    mGameDemosAvailable.clear();
    mGameDemos.clear();

	GameApplication* gameApp = (GameApplication*)Application::App;
    for(unsigned int i=0; i<mGameDemoSearchPath.size(); i++)
    {
        const eastl::wstring &dir = mGameDemoSearchPath[i];

        // First test if the directory itself contains a track:
        // ----------------------------------------------------
        if(LoadGameDemo(dir)) continue;  // track found, no more tests

        // Then see if a subdir of this dir contains tracks
        // ------------------------------------------------
        eastl::set<eastl::wstring> dirs;
		gameApp->mFileSystem->ListFiles(
			dirs, eastl::wstring("../../../Assets/SuperTuxKart/") + dir);
        for(eastl::set<eastl::wstring>::iterator subdir = dirs.begin();
            subdir != dirs.end(); subdir++)
        {
            if(*subdir==L"." || *subdir==L"..") continue;
            LoadGameDemo(dir+*subdir+L"/");
        }   // for dir in dirs
    }   // for i <m_demo_search_path.size()
}  // loadDemosList

// ----------------------------------------------------------------------------
/** Tries to load a demo from a single directory. Returns true if a demo was
 *  successfully loaded.
 *  \param dirname Name of the directory to load the demo from.
 */
bool GameDemoManager::LoadGameDemo(const eastl::wstring& dirname)
{
	GameApplication* gameApp = (GameApplication*)Application::App;

    eastl::wstring configFile = dirname + L"demo.xml";
	if (!gameApp->mFileSystem->ExistFile(configFile))
        return false;

    GameDemo *gameDemo;
    try
    {
		//gameDemo = new GameDemo(configFile);
    }
    catch (std::exception& e)
    {
        fprintf(stderr, "[TrackManager] ERROR: Cannot load track <%ws> : %s\n",
                dirname.c_str(), e.what());
        return false;
    }
	/*
    if (gameDemo->getVersion()<stk_config->m_min_track_version ||
        gameDemo->getVersion()>stk_config->m_max_track_version)
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
    mAllGameDemoDirs.push_back(dirname);
    mGameDemos.push_back(gameDemo);
    mGameDemosAvailable.push_back(true);
    UpdateGroups(gameDemo);
    return true;
}   // loadTrack

// ----------------------------------------------------------------------------
/** Removes a demo.
 *  \param ident Identifier of the demo (i.e. the name of the directory).
 */
void GameDemoManager::RemoveGameDemo(const eastl::wstring& ident)
{
    GameDemo* gameDemo = GetGameDemo(ident);
    if (NULL == NULL)
    {
        fprintf(stderr, "[TrackManager] ERROR: There is no demo named '%ws'!!\n", ident.c_str());
        return;
    }
	/*
    if (gameDemo->IsInternal()) return;

    eastl::vector<GameDemo*>::iterator it = eastl::find(mGameDemos.begin(), mGameDemos.end(), gameDemo);
    if (it == mGameDemos.end())
    {
        fprintf(stderr, "[DemosManager] INTERNAL ERROR: Cannot find demo '%s' in map!!\n", ident.c_str());
        return;
    }
    int index = it - mGameDemos.begin();

    // Remove the demo from all groups it belongs to
    GroupToIndices &groupToIndices = mGameDemoGroups;

    eastl::vector<eastl::wstring> &groupNames = mGameDemoGroupNames;

    const eastl::vector<eastl::wstring>& groups= gameDemo->GetGroups();
    for(unsigned int i=0; i<groups.size(); i++)
    {
        eastl::vector<int> &indices = groupToIndices[groups[i]];
        eastl::vector<int>::iterator j;
        j = eastl::find(indices.begin(), indices.end(), index);
        LogAssert(j!=indices.end(), "group out of range");
        indices.erase(j);

        // If the demo was the last member of a group,
        // completely remove the group
        if(indices.size()==0)
        {
			groupToIndices.erase(groups[i]);
            eastl::vector<eastl::wstring>::iterator itg;
            itg = eastl::find(groupNames.begin(), groupNames.end(), groups[i]);
            LogAssert(itg != groupNames.end(), "group out of range");
            groupNames.erase(itg);
        }   // if complete group must be removed
    }   // for i in groups

    // Adjust all indices of demos with an index number higher than
    // the removed demo, since they have been moved down. This must
    // be done for all demos
    unsigned int i=2; // i=2: demos
    {
        GroupToIndices &gToi = mGameDemoGroups;
		GroupToIndices::iterator j;
        for(j=gToi.begin(); j!=gToi.end(); j++)
        {
            for(unsigned int i=0; i<(*j).second.size(); i++)
                if((*j).second[i]>index) (*j).second[i]--;
        }   // for j in group_2_indices
    }   // for i in arenas, demos

    mGameDemos.erase(it);
    mAllGameDemoDirs.erase(mAllGameDemoDirs.begin()+index);
    mGameDemosAvailable.erase(mGameDemosAvailable.begin()+index);
    delete gameDemo;
	*/
}   // removeDemo

// ----------------------------------------------------------------------------
/** \brief Updates the groups after a demo was read in.
  * \param demo Pointer to the new demo, whose groups are now analysed.
  */
void GameDemoManager::UpdateGroups(const GameDemo* gameDemo)
{
	/*
    if (gameDemo->isInternal()) return;

    const eastl::vector<eastl::wstring>& newGroups = gameDemo->getGroups();

    GroupToIndices &groupToIndices = mGameDemoGroups;

    eastl::vector<eastl::wstring> &groupNames = mGameDemoGroupNames;

    const unsigned int groupsAmount = newGroups.size();
    for(unsigned int i=0; i<groupsAmount; i++)
    {
        bool groupExists = 
			groupToIndices.find(newGroups[i]) != groupToIndices.end();
        if(!groupExists)
            groupNames.push_back(newGroups[i]);
        groupToIndices[newGroups[i]].push_back(mGameDemos.size()-1);
    }
	*/
}   // UpdateGroups

// ----------------------------------------------------------------------------
