//========================================================================
// CriticalSection.h : Defines helper classes for multiprogramming
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

#ifndef CRITICALSECTION_H
#define CRITICALSECTION_H

#include "GameEngineStd.h"

#include <windows.h>
 
class BaseCriticalSection
{
public:
	BaseCriticalSection()
	{ 
		InitializeCriticalSection( &mCriticalSection);
	}
 
	~BaseCriticalSection()
	{
		DeleteCriticalSection( &mCriticalSection);
	}
 
	void Lock()
    { 
		EnterCriticalSection( &mCriticalSection);
	}

	void Unlock()
    { 
		LeaveCriticalSection( &mCriticalSection);
	}
 
protected:
       // the critical section itself
    mutable CRITICAL_SECTION mCriticalSection;
};
 
/*
 Description
      
       helper class
 
 
       allows automatic Locking/ Unlocking of a Resource,
       protected by a Critical Section:
       - locking when this object gets constructed
       - unlocking when this object is destructed
        (goes out of scope)
 
 
 Usage
 
 
       when you need protected access to a resource, do the following
       1. Create a Critical Section associated with the resource
       2. Embody the code accessing the resource in braces {}
       3. Construct an ScopedCriticalSection object
 
 
  Example:
       // we assume that m_CriticalSection
       // is a private variable, and we use it to protect
       // 'this' from being accessed while we need safe access to a resource
      
 
 
       // code that does not need resource locking
      
       {
              ScopedCriticalSection I_am_locked( m_cs);
             
              // code that needs thread locking
       }
      
       // code that does not need resource locking
 
 
*/
class ScopedCriticalSection
{
public:
	ScopedCriticalSection(BaseCriticalSection& csResource)
		: mCSResource(csResource)
	{
		mCSResource.Lock();
	}
  
	~ScopedCriticalSection()
	{
		mCSResource.Unlock();
	}

private:
    // the Critical Section for this resource
	BaseCriticalSection & mCSResource;
};
 


// concurrent_queue was grabbed from 
// http://www.justsoftwaresolutions.co.uk/threading/implementing-a-thread-safe-queue-using-condition-variables.html
// and was written by Anthony Williams
//
// class concurrent_queue					- Chapter 18, page 669
//

template<typename Data>
class ConcurrentQueue
{
private:
    eastl::queue<Data> mQueue;
	BaseCriticalSection mCriticalSection;
	HANDLE mDataPushed;
public:
	ConcurrentQueue()
	{
		mDataPushed = CreateEvent(NULL, TRUE, FALSE, NULL);
	}

    void Push(Data const& data)
    {
		ScopedCriticalSection locker(mCriticalSection);
		mQueue.push(data);
		PulseEvent(mDataPushed);
    }

    bool Empty() const
    {
		ScopedCriticalSection locker(mCriticalSection);
        return mQueue.empty();
    }

    bool TryPop(Data& poppedValue)
    {
		ScopedCriticalSection locker(mCriticalSection);
        if(mQueue.empty())
            return false;
        
		poppedValue = mQueue.front();
        mQueue.pop();
        return true;
    }

    void WaitAndPop(Data& poppedValue)
    {
		ScopedCriticalSection locker(mCriticalSection);
        while(mQueue.empty())
        {
			WaitForSingleObject(m_dataPushed);
        }
        
		poppedValue = mQueue.front();
		mQueue.pop();
    }
};

#endif