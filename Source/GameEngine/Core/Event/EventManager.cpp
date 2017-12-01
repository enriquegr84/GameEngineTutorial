//========================================================================
// EventManager.cpp : implements a multi-listener multi-sender event system
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

#include "EventManager.h"

#include "Core/Logger/Logger.h"

BaseEventManager* BaseEventManager::mEventMgr = NULL;
GenericObjectFactory<BaseEventData, BaseEventType> mEventFactory;

//GE_MEMORY_WATCHER_DEFINITION(EventData);

BaseEventManager* BaseEventManager::Get(void)
{
    LogAssert(BaseEventManager::mEventMgr, "Event manager doesn't exist");
	return BaseEventManager::mEventMgr;
}

BaseEventManager::BaseEventManager(const char* pName, bool setAsGlobal)
{
	if (setAsGlobal)
    {
        if (BaseEventManager::mEventMgr)
        {
            LogError("Attempting to create two global event managers! \
					The old one will be destroyed and overwritten with this one.");
            delete BaseEventManager::mEventMgr;
        }

		BaseEventManager::mEventMgr = this;
    }
}

BaseEventManager::~BaseEventManager(void)
{
	if (BaseEventManager::mEventMgr == this)
		BaseEventManager::mEventMgr = nullptr;
}


//---------------------------------------------------------------------------------------------------------------------
// EventManager::EventManager
//---------------------------------------------------------------------------------------------------------------------
EventManager::EventManager(const char* pName, bool setAsGlobal)
	: BaseEventManager(pName, setAsGlobal)
{
	mActiveQueue = 0;
}


//---------------------------------------------------------------------------------------------------------------------
// EventManager::~EventManager
//---------------------------------------------------------------------------------------------------------------------
EventManager::~EventManager()
{
	//
}

/*
	EventManager AddListener walks through the list to see if the listener has already been registered. Registering
	the same delegate for the same event more than once is an error, since processing the event would end up calling
	the delegate function multiple times. If the delegate has never been registered for this event, it is added to 
	the list.
*/
bool EventManager::AddListener(const EventListenerDelegate& eventDelegate, const BaseEventType& type)
{
	//LogInformation("Events " + eastl::string("Attempting to add delegate function for event type: ") + eastl::to_string(type));

	EventListenerList& eventListenerList = mEventListeners[type];  // this will find or create the entry
	for (auto it = eventListenerList.begin(); it != eventListenerList.end(); ++it)
	{
		if (eventDelegate == (*it))
		{
			LogWarning("Attempting to double-register a delegate");
			return false;
		}
	}

	eventListenerList.push_back(eventDelegate);
	//LogInformation("Events " + eastl::string("Successfully added delegate for event type: ") + eastl::to_string(type));

	return true;
}


/*
	EventManager RemoveListener walks through the list of listener attempting to find the delegate. The FastDelegate
	classes all implement an overloaded comparison (==) operator, that way if the delegate is found, it is removed
	from the list.
*/
bool EventManager::RemoveListener(const EventListenerDelegate& eventDelegate, const BaseEventType& type)
{
	//LogInformation("Events " + eastl::string("Attempting to remove delegate function from event type: ") + eastl::to_string(type));
	bool success = false;

	auto findIt = mEventListeners.find(type);
	if (findIt != mEventListeners.end())
	{
		EventListenerList& listeners = findIt->second;
		for (auto it = listeners.begin(); it != listeners.end(); ++it)
		{
			if (eventDelegate == (*it))
			{
				listeners.erase(it);
				//LogInformation("Events " + eastl::string("Successfully removed delegate function from event type: ") + eastl::to_string(type));
				success = true;
				break;  // we don't need to continue because it should be impossible for the same delegate function to be registered for the same event more than once
			}
		}
	}

	return success;
}

/*
	TriggerEvent fires an event and have all listeners respond ot it inmediately, without using the event queue.
	It breaks the paradigm of remote event handling, but it is still necessary for certain operations. It
	tries to find the event listener list associated with this event type and then, iterates through all 
	the delegates and calls each one. It returns true if any listener handled the event.
	The most common and correct way of sending events is by using this method.
*/
bool EventManager::TriggerEvent(const BaseEventDataPtr& pEvent) const
{
	//LogInformation("Events " + eastl::string("Attempting to trigger event ") + eastl::string(pEvent->GetName()));
	bool processed = false;

	auto findIt = mEventListeners.find(pEvent->GetEventType());
	if (findIt != mEventListeners.end())
	{
		const EventListenerList& eventListenerList = findIt->second;
		for (EventListenerList::const_iterator it = eventListenerList.begin(); it != eventListenerList.end(); ++it)
		{
			EventListenerDelegate listener = (*it);
			//LogInformation("Events " + eastl::string("Sending Event ") + eastl::string(pEvent->GetName()) + eastl::string(" to delegate."));
			listener(pEvent);  // call the delegate
			processed = true;
		}
	}

	return processed;
}


/*
	EventManager QueueEvent finds the associated event listener list. If it finds this list, it adds the event to the
	currently active queue. This keeps the EventManager from processing events for which there are no listeners.
*/
bool EventManager::QueueEvent(const BaseEventDataPtr& pEvent)
{
	LogAssert(mActiveQueue >= 0, "Queue active");
	LogAssert(mActiveQueue < EVENTMANAGER_NUM_QUEUES, "Queue active max");

	// make sure the event is valid
	if (!pEvent)
	{
		LogError("Invalid event in VQueueEvent()");
		return false;
	}

	//LogInformation("Events " + eastl::string("Attempting to queue event: ") + eastl::string(pEvent->GetName()));

	auto findIt = mEventListeners.find(pEvent->GetEventType());
	if (findIt != mEventListeners.end())
	{
		mQueues[mActiveQueue].push_back(pEvent);
		//LogInformation("Events " + eastl::string("Successfully queued event: ") + eastl::string(pEvent->GetName()));
		return true;
	}
	else
	{
		//LogInformation("Events " + eastl::string("Skipping event since there are no delegates registered to receive it: ") + eastl::string(pEvent->GetName()));
		return false;
	}
}

//---------------------------------------------------------------------------------------------------------------------
// EventManager::ThreadSafeQueueEvent
//---------------------------------------------------------------------------------------------------------------------
bool EventManager::ThreadSafeQueueEvent(const BaseEventDataPtr& pEvent)
{
	mRealtimeEventQueue.Push(pEvent);
	return true;
}

/*
	EventManager AbortEvent method looks in the active queue for the event of a given type and erases it. Note that
	this method can erase the first event in the queue of a given type or all events of a given type, depending on the
	value of the second parameter. This method could be used to remove redundant messages from the queue.
*/
bool EventManager::AbortEvent(const BaseEventType& inType, bool allOfType)
{
	LogAssert(mActiveQueue >= 0, "Queue active");
	LogAssert(mActiveQueue < EVENTMANAGER_NUM_QUEUES, "Queue active max");

	bool success = false;
	EventListenerMap::iterator findIt = mEventListeners.find(inType);

	if (findIt != mEventListeners.end())
	{
		EventQueue& eventQueue = mQueues[mActiveQueue];
		auto it = eventQueue.begin();
		while (it != eventQueue.end())
		{
			// Removing an item from the queue will invalidate the iterator, so have it point to the next member.  All
			// work inside this loop will be done using thisIt.
			auto thisIt = it;
			++it;

			if ((*thisIt)->GetEventType() == inType)
			{
				eventQueue.erase(thisIt);
				success = true;
				if (!allOfType)
					break;
			}
		}
	}

	return success;
}

/*
	EventManager Update should be called on the main loop in order to process the queued messages. It takes
	all the queued messages and calls the registered delegate methods. There is a double queue processing in
	which one is used for events being actively processed and the other for new events, that way we keep track
	of events which continually creates new events. Events are being pulled from one of the queues and it can 
	be called with a maximum time allowed. If the amount of time is exceeded, the method exits, even if there 
	are messages still in the queue. This can be pretty useful for smoothing out the frame rate stutter if it 
	is attempted to handle too many events in one game loop.
*/
bool EventManager::Update(unsigned long maxTime)
{
	unsigned long currMs = GetTickCount();
	unsigned long maxMs = ((maxTime == BaseEventManager::CONS_INFINITE) ? 
		(BaseEventManager::CONS_INFINITE) : (currMs + maxTime));

	// This section added to handle events from other threads.  Check out Chapter 20.
	BaseEventDataPtr pRealtimeEvent;
	while (mRealtimeEventQueue.Pop(pRealtimeEvent))
	{
		QueueEvent(pRealtimeEvent);

		currMs = GetTickCount();
		if (maxTime != BaseEventManager::CONS_INFINITE)
		{
			if (currMs >= maxMs)
			{
				LogError("A realtime process is spamming the event manager!");
			}
		}
	}

	// swap active queues and clear the new queue after the swap
	int queueToProcess = mActiveQueue;
	mActiveQueue = (mActiveQueue + 1) % EVENTMANAGER_NUM_QUEUES;
	mQueues[mActiveQueue].clear();
	/*
	LogInformation("EventLoop " + eastl::string("Processing Event Queue ") + eastl::to_string(queueToProcess) + "; "
		+ eastl::to_string((unsigned long)mQueues[queueToProcess].size()) + eastl::string(" events to process"));
	*/
	// Process the queue
	while (!mQueues[queueToProcess].empty())
	{
		// pop the front of the queue
		BaseEventDataPtr pEvent = mQueues[queueToProcess].front();
		mQueues[queueToProcess].pop_front();
		//LogInformation("EventLoop " + eastl::string("\t\tProcessing Event ") + eastl::string(pEvent->GetName()));

		const BaseEventType& eventType = pEvent->GetEventType();

		// find all the delegate functions registered for this event
		auto findIt = mEventListeners.find(eventType);
		if (findIt != mEventListeners.end())
		{
			const EventListenerList& eventListeners = findIt->second;
			/*
			LogInformation("EventLoop " + eastl::string("\t\tFound ") + eastl::to_string((unsigned long)eventListeners.size())
				+ eastl::string(" delegates"));
			*/
			// call each listener
			for (auto it = eventListeners.begin(); it != eventListeners.end(); ++it)
			{
				EventListenerDelegate listener = (*it);
				/*
				LogInformation("EventLoop " + eastl::string("\t\tSending event ") + eastl::string(pEvent->GetName())
					+ eastl::string(" to delegate"));
				*/
				listener(pEvent);
			}
		}

		// check to see if time ran out
		currMs = GetTickCount();
		if (maxTime != BaseEventManager::CONS_INFINITE && currMs >= maxMs)
		{
			//LogInformation("EventLoop Aborting event processing; time ran out");
			break;
		}
	}

	// If we couldn't process all of the events, push the remaining events to the new active queue.
	// Note: To preserve sequencing, go back-to-front, inserting them at the head of the active queue
	bool queueFlushed = (mQueues[queueToProcess].empty());
	if (!queueFlushed)
	{
		while (!mQueues[queueToProcess].empty())
		{
			BaseEventDataPtr pEvent = mQueues[queueToProcess].back();
			mQueues[queueToProcess].pop_back();
			mQueues[mActiveQueue].push_front(pEvent);
		}
	}

	return queueFlushed;
}