//========================================================================
// RealtimeProcess.cpp : Defines process that can run in a thread
//
// GameCode4 is the sample application that encapsulates much of the source code
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
//    http://code.google.com/p/gamecode4/
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

#include "GameEngineStd.h"

#include "RealtimeProcess.h"
#include "CriticalSection.h"

#include "Core/Event/EventManager.h"

DWORD MaxLoops = 100000;
DWORD ProtectedTotal = 0;
CRITICAL_SECTION CriticalSection;

DWORD WINAPI ThreadProc( LPVOID lpParam )
{
	DWORD maxLoops = *static_cast<DWORD *>(lpParam);
	DWORD dwCount = 0;
    while( dwCount < maxLoops )
    { 
        ++dwCount;

		EnterCriticalSection(&CriticalSection);
		++ProtectedTotal;
		LeaveCriticalSection(&CriticalSection);
	}
	return TRUE;
}

void CreateThreads()
{
	InitializeCriticalSection(&CriticalSection); 

	for (int i=0; i<20; i++)
	{
		HANDLE m_hThread = CreateThread( 
					 NULL,       // default security attributes
					 0,          // default stack size
					 (LPTHREAD_START_ROUTINE) ThreadProc, 
					 &MaxLoops,  // thread parameter is how many loops
					 0, // default creation flags
					 NULL);      // receive thread identifier
	}
}

RealtimeProcess::RealtimeProcess( int priority )
{
	mThreadID = 0;
	mThreadPriority = priority;
}

DWORD WINAPI RealtimeProcess::ThreadProc( LPVOID lpParam )
{ 
	RealtimeProcess *proc = static_cast<RealtimeProcess *>(lpParam);
	proc->ThreadProc();
	return TRUE;
}

//------------------------------------------------------------------------------------------------
// RealtimeProcess::OnInitialize				- Chapter 18, page 667
//------------------------------------------------------------------------------------------------
void RealtimeProcess::OnInit(void)
{
	Process::OnInit();
    mHandleThread = CreateThread( 
                 NULL,         // default security attributes
                 0,            // default stack size
                 ThreadProc,   // thread process
                 this,         // thread parameter is a pointer to the process
                 0,            // default creation flags
                 &mThreadID);	// receive thread identifier

	if( mHandleThread == NULL )
    {
		LogError("Could not create thread!");
		Fail();
		return;
    }

	SetThreadPriority(mHandleThread, mThreadPriority);
}

class ProtectedProcess : public RealtimeProcess
{
public:

	static DWORD ProtectedTotal;
	static BaseCriticalSection ProtectedCriticalSection;
	DWORD mMaxLoops;
	ProtectedProcess(DWORD maxLoops) : RealtimeProcess() { mMaxLoops = maxLoops; }

	virtual void ThreadProc(void);
};

DWORD ProtectedProcess::ProtectedTotal = 0;
BaseCriticalSection ProtectedProcess::ProtectedCriticalSection;

void ProtectedProcess::ThreadProc( )
{ 
	DWORD dwCount = 0;

    while( dwCount < MaxLoops )
    { 
        ++dwCount;

		{
			// Request ownership of critical section.
			ScopedCriticalSection locker(ProtectedCriticalSection);
			++ProtectedTotal;
		}
    } 

	Succeed();
}

class UnprotectedProcess : public RealtimeProcess
{
public:
	static DWORD UnprotectedTotal;
	DWORD MaxLoops;
	UnprotectedProcess(DWORD maxLoops) : RealtimeProcess() { MaxLoops = maxLoops; }
	virtual void ThreadProc( );
};

DWORD UnprotectedProcess::UnprotectedTotal = 0;

void UnprotectedProcess::ThreadProc( void )
{
	DWORD dwCount = 0;
    while( dwCount < MaxLoops )
    { 
		++dwCount;
		++UnprotectedTotal;
	}
	Succeed();
}