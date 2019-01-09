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

#include "QuakeManager.h"
#include "QuakeApp.h"

#include "Game/Game.h"

eastl::vector<eastl::wstring>  QuakeManager::mQuakeSearchPath;

QuakeManager::QuakeManager()
{

}   // QuakeManager

//-----------------------------------------------------------------------------
/** Delete all quake.
 */
QuakeManager::~QuakeManager()
{
	/*
    for(QuakeList::iterator it = mQuake.begin(); it != mQuake.end(); ++it)
        delete (*it);
	*/
}   // ~QuakeManager

//-----------------------------------------------------------------------------
/** Adds a directory from which quake are loaded. The quake manager checks if
 *  either this directory itself contains a quake, and if any subdirectory
 *  contains a quake.
 *  \param dir The directory to add.
 */
void QuakeManager::AddQuakeSearchDir(const eastl::wstring &dir)
{
	mQuakeSearchPath.push_back(dir);
}

//-----------------------------------------------------------------------------
/** Get QuakeData by the quake identifier.
 *  \param ident Identifier = basename of the directory the quake is in.
 *  \return      The corresponding quake object, or NULL if not found
 */
Quake* QuakeManager::GetQuake(const eastl::wstring& ident) const
{
	/*
    for(QuakeList::const_iterator it = mQuake.begin(); it != mQuake.end(); ++it)
    {
        if ((*it)->GetIdent() == ident)
            return *it;
    }
	*/
    return NULL;

} 

//-----------------------------------------------------------------------------
/** Removes all cached data from all quake. This is called when the screen
 *  resolution is changed and all textures need to be bound again.
 */
void QuakeManager::RemoveAllCachedData()
{
	/*
	for (QuakeList::const_iterator it = mQuake.begin(); it != mQuake.end(); ++it)
        (*it)->RemoveCachedData();
	*/
}   // removeAllCachedData
//-----------------------------------------------------------------------------
/** Sets all quake that are not in the list a to be unavailable. This is used
 *  by the network manager upon receiving the list of available quake from
 *  a client.
 *  \param quake List of all quake identifiers (available on a client).
 */
void QuakeManager::SetUnavailableQuake(const eastl::vector<eastl::wstring> &quake)
{
	/*
	for (QuakeList::const_iterator it = mQuake.begin(); it != mQuake.end(); ++it)
    {
        if(!mQuakeAvailable[it - mQuake.begin()]) continue;
        const eastl::string id=(*it)->GetIdent();
        if (eastl::find(quake.begin(), quake.end(), id)==quake.end())
        {
			mQuakeAvailable[it - mQuake.begin()] = false;
            fprintf(stderr, "Quake '%s' not available on all clients, disabled.\n", id.c_str());
        }   // if id not in tracks
    } 
	*/
}   // setUnavailableQuake

//-----------------------------------------------------------------------------
/** Returns a list with all quake identifiers.
 */
eastl::vector<eastl::wstring> QuakeManager::GetAllQuakeIdentifiers()
{
    eastl::vector<eastl::wstring> all;
	/*
	for (QuakeList::const_iterator it = mQuake.begin(); it != mQuake.end(); ++it)
	{
        all.push_back((*it)->GetIdent());
    }
	*/
    return all;
}   // GetAllQuakeIdentifiers

//-----------------------------------------------------------------------------
/** Loads all quake from the data directory (data/quake).
 */
void QuakeManager::LoadQuakeList()
{
    mAllQuakeDirs.clear();
    mQuakeGroupNames.clear();
    mQuakeGroups.clear();
    mQuakeAvailable.clear();
    mQuake.clear();

    for(unsigned int i=0; i<mQuakeSearchPath.size(); i++)
    {
        const eastl::wstring &dir = mQuakeSearchPath[i];

        // ----------------------------------------------------
        if(LoadQuake(dir)) continue;  // data found, no more tests

        // Then see if a subdir of this dir contains data
        // ------------------------------------------------
        eastl::set<eastl::wstring> dirs;
		FileSystem::Get()->GetFileList(
			dirs, eastl::wstring("../../../Assets/Quake/") + dir);
        for(eastl::set<eastl::wstring>::iterator subdir = dirs.begin();
            subdir != dirs.end(); subdir++)
        {
            if(*subdir==L"." || *subdir==L"..") continue;
            LoadQuake(dir+*subdir+L"/");
        }   // for dir in dirs
    }   // for i <mQuakeSearchPath.size()
}  // loadQuakeList

// ----------------------------------------------------------------------------
/** Tries to load a quake from a single directory. Returns true if a quake was
 *  successfully loaded.
 *  \param dirname Name of the directory to load the quake from.
 */
bool QuakeManager::LoadQuake(const eastl::wstring& dirname)
{
    eastl::wstring configFile = dirname + L"quake.xml";
	if (!FileSystem::Get()->ExistFile(configFile))
        return false;

    Quake *quake;
    try
    {
		//quake = new Quake(configFile);
    }
    catch (std::exception& e)
    {
        fprintf(stderr, "[TrackManager] ERROR: Cannot load data <%ws> : %s\n",
                dirname.c_str(), e.what());
        return false;
    }

    mAllQuakeDirs.push_back(dirname);
    mQuake.push_back(quake);
    mQuakeAvailable.push_back(true);
    UpdateGroups(quake);
    return true;
}   // loadTrack

// ----------------------------------------------------------------------------
/** Removes a quake.
 *  \param ident Identifier of the quake (i.e. the name of the directory).
 */
void QuakeManager::RemoveQuake(const eastl::wstring& ident)
{
    Quake* quake = GetQuake(ident);
    if (NULL == NULL)
    {
        fprintf(stderr, "[TrackManager] ERROR: There is no quake named '%ws'!!\n", ident.c_str());
        return;
    }
	
}   // removeQuake

// ----------------------------------------------------------------------------
/** \brief Updates the groups after a quake was read in.
  * \param quake Pointer to the new quake, whose groups are now analysed.
  */
void QuakeManager::UpdateGroups(const Quake* quake)
{
	/*
    if (quake->isInternal()) return;

    const eastl::vector<eastl::wstring>& newGroups = quake->getGroups();

    GroupToIndices &groupToIndices = mQuakeGroups;

    eastl::vector<eastl::wstring> &groupNames = mQuakeGroupNames;

    const unsigned int groupsAmount = newGroups.size();
    for(unsigned int i=0; i<groupsAmount; i++)
    {
        bool groupExists = 
			groupToIndices.find(newGroups[i]) != groupToIndices.end();
        if(!groupExists)
            groupNames.push_back(newGroups[i]);
        groupToIndices[newGroups[i]].push_back(mQuake.size()-1);
    }
	*/
}   // UpdateGroups

// ----------------------------------------------------------------------------
