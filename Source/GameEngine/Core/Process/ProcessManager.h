//========================================================================
// ProcessManager.h : defines common game events
//
// Part of the GameEngine Application
//
// GameEngine is the sample application that encapsulates much of the source code
// discussed in "Game Coding Complete - 4th Edition" by Mike McShaffry and David
// "Rez" Graham, published by Charles River Media. 
// ISBN-10: 1133776574 | ISBN-13: 978-1133776574
//
// If this source code has found it's way to you, and you think it has helped you
// in any way, do the authors a favor and buy a new copy of the book - there are 
// detailed explanations in it that compliment this code well. Buy a copy at Amazon.com
// by clicking here: 
//    http://www.amazon.com/gp/product/1133776574/ref=olp_product_details?ie=UTF8&me=&seller=
//
// There's a companion web site at http://www.mcshaffry.com/GameCode/
// 
// The source code is managed and maintained through Google Code: 
//    http://code.google.com/p/GameEngine/
//
// (c) Copyright 2012 Michael L. McShaffry and David Graham
//
// This program is free software; you can redistribute it and/or
// modify it under the terms of the GNU Lesser GPL v3
// as published by the Free Software Foundation.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See 
// http://www.gnu.org/licenses/lgpl-3.0.txt for more details.
//
// You should have received a copy of the GNU Lesser GPL v3
// along with this program; if not, write to the Free Software
// Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
//
//========================================================================

#ifndef PROCESSMANAGER_H
#define PROCESSMANAGER_H

#include "GameEngineStd.h"

#include "Process.h"

class ProcessManager
{
	typedef eastl::list<eastl::shared_ptr<Process>> ProcessList;

	ProcessList m_processList;

public:
	// construction
	~ProcessManager(void);

	// interface
	unsigned int UpdateProcesses(unsigned long deltaMs);  // updates all attached processes
	eastl::weak_ptr<Process> AttachProcess(eastl::shared_ptr<Process> pProcess);  // attaches a process to the process mgr
	void AbortAllProcesses(bool immediate);

	// accessors
	unsigned int GetProcessCount(void) const { return (unsigned int)m_processList.size(); }

private:
	void ClearAllProcesses(void);  // should only be called by the destructor
};


#endif

