//========================================================================
// RealtimeProcess.h : Defines real time processes that can be used with ProcessManager
//
// Part of the GameCode4 Application
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

#ifndef REALTIMEPROCESS_H
#define REALTIMEPROCESS_H

#include "Process.h"

class RealtimeProcess : public Process
{
protected:

	DWORD mThreadID;
	int mThreadPriority;
	HANDLE mHandleThread;

public:
	// Other prioities can be:
	// THREAD_PRIORITY_ABOVE_NORMAL
	// THREAD_PRIORITY_BELOW_NORMAL
	// THREAD_PRIORITY_HIGHEST
	// THREAD_PRIORITY_LOWEST
	// THREAD_PRIORITY_IDLE
	// THREAD_PRIORITY_TIME_CRITICAL
	//
	//
	RealtimeProcess(int priority = THREAD_PRIORITY_NORMAL);
    virtual ~RealtimeProcess(void) { CloseHandle(mHandleThread); }
	static DWORD WINAPI ThreadProc( LPVOID lpParam );

protected:
	virtual void OnInit(void);
	virtual void OnUpdate(unsigned long deltaMs) override { }   // do nothing.
	virtual void ThreadProc(void) = 0;							 // must be defined for all real time processes.
};

#endif