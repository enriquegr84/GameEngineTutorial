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

#ifndef GAMEDEMOSMANAGER_H
#define GAMEDEMOSMANAGER_H

#include "GameDemoStd.h"

#include "Game/Game.h"

class GameDemo;

/**
  * \brief Simple class to load and manage track data, track names and such
  * \ingroup tracks
  */
class GameDemoManager
{
private:
    /** All directories in which tracks are searched. */
    static eastl::vector<eastl::string> mGameDemoSearchPath;

    /** All directories in which tracks were found. */
    eastl::vector<eastl::string> mAllGameDemoDirs;

    typedef eastl::vector<GameDemo*> GameDemoList;

    /** All track objects. */
    GameDemoList mGameDemos;

    typedef eastl::map<eastl::string, eastl::vector<int> > GroupToIndices;
    /** List of all racing track groups. */
    GroupToIndices mGameDemoGroups;

    /** List of all groups (for both normal tracks and arenas) */
    //eastl::vector<eastl::string> mAllGroupNames;

    /** List of the names of all groups containing tracks */
    eastl::vector<eastl::string> mDemoGroupNames;

    /** Flag if this track is available or not. Tracks are set unavailable
     *  if they are not available on all clients (applies only to network mode)
     */
    eastl::vector<bool> mGameDemosAvailable;

    void UpdateGroups(const GameDemo* demo);

public:
	GameDemoManager();
	~GameDemoManager();

    static void AddGameDemoSearchDir(const eastl::string &dir);
    /** Returns a list of all demo identifiers. */
    eastl::vector<eastl::string> GetAllGameDemosIdentifiers();

    /** Load all demo files from all directories */
    void  LoadGameDemosList();
    void  RemoveGameDemo(const eastl::string &ident);
    bool  LoadGameDemo(const eastl::string& dirname);
    void  RemoveAllCachedData();
	GameDemo* GetGameDemo(const eastl::string& ident) const;
    // ------------------------------------------------------------------------
    /** Sets a list of demos as being unavailable (e.g. in network mode the
     *  demo is not on all connected machines.
     *  \param demos List of demos to mark as unavilable. */
    void SetUnavailableGameDemos(const eastl::vector<eastl::string> &tracks);
    // ------------------------------------------------------------------------
    /** \brief Returns a list of all directories that contain a track. */
    const eastl::vector<eastl::string>* GetAllGameDemoDirs() const
    {
        return &mAllGameDemoDirs;
    }   // getAllDemoDirs
    // ------------------------------------------------------------------------
    /** \brief Returns a list of the names of all used demo groups. */
    const eastl::vector<eastl::string>& GetAllGameDemoGroups() const
    {
        return mDemoGroupNames;
    }   // getAllDemoGroups
    // ------------------------------------------------------------------------
    /** Returns the number of demos. */
    size_t GetNumberOfDemos() const { return mGameDemos.size(); }
    // ------------------------------------------------------------------------
    /** Returns the demo view with a given index number.
     *  \param index The index number of the demo. */
    GameDemo* GetDemo(unsigned int index) const { return mGameDemos[index];}
    // ------------------------------------------------------------------------
    /** Checks if a certain demo is available.
     *  \param n Index of the demo to check. */
    bool IsAvailable(unsigned int n) const {return mGameDemosAvailable[n];}
    // ------------------------------------------------------------------------
    /** Returns a list of all demos in a given group.
     *  \param g Name of the group. */
    const eastl::vector<int>& GetGameDemosInGroup(const eastl::string& g)
    {
        return mGameDemoGroups[g];
    }   // getTracksInGroup

};   // DemosManager

#endif   // HEADER_TRACK_MANAGER_HPP
