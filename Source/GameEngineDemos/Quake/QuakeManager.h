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

#ifndef QUAKEMANAGER_H
#define QUAKEMANAGER_H

#include "QuakeStd.h"

#include "Game/Game.h"

class Quake;

/**
  * \brief Simple class to load and manage data, names and such
  */
class QuakeManager
{
private:
    /** All directories. */
    static eastl::vector<eastl::wstring> mQuakeSearchPath;

    eastl::vector<eastl::wstring> mAllQuakeDirs;

    typedef eastl::vector<Quake*> QuakeList;

    /** All data objects. */
    QuakeList mQuake;

    typedef eastl::map<eastl::wstring, eastl::vector<int> > GroupToIndices;
    /** List of all groups. */
    GroupToIndices mQuakeGroups;

    /** List of all groups (for both normal and arenas) */
    eastl::vector<eastl::wstring> mAllQuakeGroupNames;

    /** List of the names of all groups */
    eastl::vector<eastl::wstring> mQuakeGroupNames;

    eastl::vector<bool> mQuakeAvailable;

    void UpdateGroups(const Quake* quake);

public:
	QuakeManager();
	~QuakeManager();

    static void AddQuakeSearchDir(const eastl::wstring &dir);
    /** Returns a list of all quake identifiers. */
    eastl::vector<eastl::wstring> GetAllQuakeIdentifiers();

    /** Load all quake files from all directories */
    void  LoadQuakeList();
    void  RemoveQuake(const eastl::wstring &ident);
    bool  LoadQuake(const eastl::wstring& dirname);
    void  RemoveAllCachedData();
	Quake* GetQuake(const eastl::wstring& ident) const;
    // ------------------------------------------------------------------------
    /** Sets a list of quake as being unavailable (e.g. in network mode the
     *  quake is not on all connected machines.
     *  \param quake List of quake to mark as unavilable. */
    void SetUnavailableQuake(const eastl::vector<eastl::wstring> &tracks);
    // ------------------------------------------------------------------------
    const eastl::vector<eastl::wstring>* GetAllQuakeDirs() const
    {
        return &mAllQuakeDirs;
    }   // getAllQuakeDirs
    // ------------------------------------------------------------------------
    /** \brief Returns a list of the names of all used quake groups. */
    const eastl::vector<eastl::wstring>& GetAllQuakeGroups() const
    {
        return mAllQuakeGroupNames;
    }   // getAllQuakeGroups
    // ------------------------------------------------------------------------
    /** Returns the number of quake. */
    size_t GetNumberOfQuake() const { return mQuake.size(); }
    // ------------------------------------------------------------------------
    /** Returns the quake view with a given index number.
     *  \param index The index number of the quake. */
    Quake* GetQuake(unsigned int index) const { return mQuake[index];}
    // ------------------------------------------------------------------------
    /** Checks if a certain quake is available.
     *  \param n Index of the quake to check. */
    bool IsAvailable(unsigned int n) const {return mQuakeAvailable[n];}
    // ------------------------------------------------------------------------
    /** Returns a list of all quake in a given group.
     *  \param g Name of the group. */
    const eastl::vector<int>& GetQuakeInGroup(const eastl::wstring& g)
    {
        return mQuakeGroups[g];
    }

};   // QuakeManager

#endif
