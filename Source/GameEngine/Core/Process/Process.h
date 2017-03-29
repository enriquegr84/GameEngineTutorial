//========================================================================
// Process.h : defines common game events
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

#ifndef PROCESS_H
#define PROCESS_H

#include "GameEngineStd.h"

#include "Core/Logger/Logger.h"

//---------------------------------------------------------------------------------------------------------------------
// Process class
// 
// Processes are ended by one of three methods: Success, Failure, or Aborted.
//	- Success means the process completed successfully. If the process has a child, it will be attached to 
//	the process mgr.
//	- Failure means the process started but failed in some way. If the process has a child, it will be 
//	aborted.
//	- Aborted processes are processes that are canceled while not submitted to the process mgr. Depending 
//	on the circumstances, they may or may not have gotten an OnInit() call. For example, a process can 
//	spawn another process and call AttachToParent() on itself. If the new process fails, the child will
//	get an Abort() call on it, even though its status is RUNNING.
//---------------------------------------------------------------------------------------------------------------------
class Process
{
	friend class ProcessManager;

public:
	enum State
	{
		// Processes that are neither dead nor alive
		STATE_UNINITIALIZED = 0,  // created but not running
		STATE_REMOVED,  		// removed from the process list but not destroyed; 
						// this can happen when a process that is already 
						// running is parented to another process

		// Living processes
		STATE_RUNNING,  // initialized and running
		STATE_PAUSED,  // initialized but paused
		
		// Dead processes
		STATE_SUCCEEDED,  // completed successfully
		STATE_FAILED,  // failed to complete
		STATE_ABORTED,  // aborted; may not have started
	};
	
private:
	State mState;  // the current state of the process
	eastl::shared_ptr<Process> mChild;  // the child process, if any

public:
	// construction
    Process(void);
	virtual ~Process(void);
	
protected:
	// interface; these functions should be overridden by the subclass as needed

	// called during the first update; responsible for setting the initial state (typically RUNNING)
	virtual void OnInit(void) { mState = STATE_RUNNING; } 
	virtual void OnUpdate(unsigned long deltaMs) = 0;  // called every frame
	virtual void OnSuccess(void) { }  // called if the process succeeds (see below)
	virtual void OnFail(void) { }  // called if the process fails (see below)
	virtual void OnAbort(void) { }  // called if the process is aborted (see below)

public:
	// Functions for ending the process.
	inline void Succeed(void);
	inline void Fail(void);
	
	// pause
	inline void Pause(void);
	inline void UnPause(void);

	// accessors
	State GetState(void) const { return mState; }
	bool IsAlive(void) const { return (mState == STATE_RUNNING || mState == STATE_PAUSED); }
	bool IsDead(void) const { return (mState == STATE_SUCCEEDED || mState == STATE_FAILED || mState == STATE_ABORTED); }
	bool IsRemoved(void) const { return (mState == STATE_REMOVED); }
	bool IsPaused(void) const { return mState == STATE_PAUSED; }

	// child functions
	inline void AttachChild(eastl::shared_ptr<Process> pChild);
	eastl::shared_ptr<Process> RemoveChild(void);  // releases ownership of the child
	eastl::shared_ptr<Process> PeekChild(void) { return mChild; }  // doesn't release ownership of the child

private:
	void SetState(State newState) { mState = newState; }
};


//---------------------------------------------------------------------------------------------------------------------
// Inline function definitions
//---------------------------------------------------------------------------------------------------------------------
inline void Process::Succeed(void)
{
	LogAssert(mState == STATE_RUNNING || mState == STATE_PAUSED, "Fail");
	mState = STATE_SUCCEEDED;
}

inline void Process::Fail(void)
{
	LogAssert(mState == STATE_RUNNING || mState == STATE_PAUSED, "Fail");
	mState = STATE_FAILED;
}

inline void Process::AttachChild(eastl::shared_ptr<Process> pChild)
{
	if (mChild)
		mChild->AttachChild(pChild);
	else
		mChild = pChild;
}

inline void Process::Pause(void)
{
	if (mState == STATE_RUNNING)
		mState = STATE_PAUSED;
	else
		LogWarning("Attempting to pause a process that isn't running");
}

inline void Process::UnPause(void)
{
	if (mState == STATE_PAUSED)
		mState = STATE_RUNNING;
	else
		LogWarning("Attempting to unpause a process that isn't paused");
}

#endif