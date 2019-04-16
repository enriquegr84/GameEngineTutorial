//========================================================================
// ProcessManager.cpp : defines common game events
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


#include "ProcessManager.h"

//---------------------------------------------------------------------------------------------------------------------
// Destructor
//---------------------------------------------------------------------------------------------------------------------
ProcessManager::~ProcessManager(void)
{
    ClearAllProcesses();
}


//---------------------------------------------------------------------------------------------------------------------
// The process update tick.  Called every logic tick.  This function returns the number of process chains that 
// succeeded in the upper 32 bits and the number of process chains that failed or were aborted in the lower 32 bits.
//---------------------------------------------------------------------------------------------------------------------
unsigned int ProcessManager::UpdateProcesses(unsigned long deltaMs)
{
    unsigned short int successCount = 0;
    unsigned short int failCount = 0;

    ProcessList::iterator it = mProcesses.begin();
    while (it != mProcesses.end())
    {
        // grab the next process
        eastl::shared_ptr<Process> currProcess = (*it);

        // save the iterator and increment the old one in case we need to remove this process from the list
        ProcessList::iterator thisIt = it;
        ++it;

        // process is uninitialized, so initialize it
        if (currProcess->GetState() == Process::STATE_UNINITIALIZED)
            currProcess->OnInit();

        // give the process an update tick if it's running
        if (currProcess->GetState() == Process::STATE_RUNNING)
            currProcess->OnUpdate(deltaMs);

        // check to see if the process is dead
        if (currProcess->IsDead())
        {
            // run the appropriate exit function
            switch (currProcess->GetState())
            {
                case Process::STATE_SUCCEEDED :
                {
                    currProcess->OnSuccess();
                    eastl::shared_ptr<Process> child = currProcess->RemoveChild();
                    if (child)
                        AttachProcess(child);
                    else
                        ++successCount;  // only counts if the whole chain completed
                    break;
                }

                case Process::STATE_FAILED :
                {
                    currProcess->OnFail();
                    ++failCount;
                    break;
                }

                case Process::STATE_ABORTED :
                {
                    currProcess->OnAbort();
                    ++failCount;
                    break;
                }
            }

            // remove the process and destroy it
            mProcesses.erase(thisIt);
        }
    }

    return ((successCount << 16) | failCount);
}


//---------------------------------------------------------------------------------------------------------------------
// Attaches the process to the process list so it can be run on the next update.
//---------------------------------------------------------------------------------------------------------------------
eastl::weak_ptr<Process> ProcessManager::AttachProcess(eastl::shared_ptr<Process> pProcess)
{
	mProcesses.push_front(pProcess);
    return eastl::weak_ptr<Process>(pProcess);
}

//---------------------------------------------------------------------------------------------------------------------
// Clears all processes (and DOESN'T run any exit code)
//---------------------------------------------------------------------------------------------------------------------
void ProcessManager::ClearAllProcesses(void)
{
    mProcesses.clear();
}

//---------------------------------------------------------------------------------------------------------------------
// Aborts all processes.  If immediate == true, it immediately calls each ones OnAbort() function and destroys all 
// the processes.
//---------------------------------------------------------------------------------------------------------------------
void ProcessManager::AbortAllProcesses(bool immediate)
{
    ProcessList::iterator it = mProcesses.begin();
    while (it != mProcesses.end())
    {
        ProcessList::iterator tempIt = it;
        ++it;

        eastl::shared_ptr<Process> process = *tempIt;
        if (process->IsAlive())
        {
            process->SetState(Process::STATE_ABORTED);
            if (immediate)
            {
                process->OnAbort();
                mProcesses.erase(tempIt);
            }
        }
    }
}


