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

eastl::vector<eastl::wstring>  GameDemoManager::mGameDemoSearchPath;

/*
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
/*
 */
void GameDemoManager::LoadGameDemosList()
{
    mAllGameDemoDirs.clear();
    mGameDemoGroupNames.clear();
    mGameDemoGroups.clear();
    mGameDemosAvailable.clear();
    mGameDemos.clear();

    for(unsigned int i=0; i<mGameDemoSearchPath.size(); i++)
    {
        const eastl::wstring &dir = mGameDemoSearchPath[i];

        // ----------------------------------------------------
        if(LoadGameDemo(dir)) continue;  // found, no more tests


        // ------------------------------------------------
        eastl::set<eastl::wstring> dirs;
		FileSystem::Get()->GetFileList(
			dirs, eastl::wstring("../../../Assets/Demo/") + dir);
        for(eastl::set<eastl::wstring>::iterator subdir = dirs.begin();
            subdir != dirs.end(); subdir++)
        {
            if(*subdir==L"." || *subdir==L"..") continue;
            LoadGameDemo(dir+*subdir+L"/");
        }   // for dir in dirs
    }   // for i <mDemoSearchPath.size()
}  // loadDemosList

// ----------------------------------------------------------------------------
/** Tries to load a demo from a single directory. Returns true if a demo was
 *  successfully loaded.
 *  \param dirname Name of the directory to load the demo from.
 */
bool GameDemoManager::LoadGameDemo(const eastl::wstring& dirname)
{
    eastl::wstring configFile = dirname + L"demo.xml";
	if (!FileSystem::Get()->ExistFile(configFile))
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
        fprintf(stderr, "There is no demo named '%ws'!!\n", ident.c_str());
        return;
    }
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
