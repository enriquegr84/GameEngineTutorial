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

#ifndef AIMANAGER_H
#define AIMANAGER_H

#include "GameEngineStd.h"

#include "Pathing.h"
#include "KMeans.h"

/*
	Simple class to load and manage artificial intelligence
 */
class AIManager
{
public:
	AIManager();
	~AIManager();

	virtual void SavePathingGraph(const eastl::string& path) { }
	virtual void LoadPathingGraph(const eastl::wstring& path) { }

	virtual void OnUpdate(unsigned long deltaMs) { }

	virtual void CreateMap(ActorId playerId) { }

	const eastl::shared_ptr<PathingGraph>& GetPathingGraph() { return mPathingGraph; }

protected:

	eastl::shared_ptr<PathingGraph> mPathingGraph;

};   // AIManager

#endif
