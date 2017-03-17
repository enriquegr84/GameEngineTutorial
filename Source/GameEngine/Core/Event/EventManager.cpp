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

static BaseEventManager* g_pEventMgr = NULL;
GenericObjectFactory<BaseEventData, BaseEventType> g_eventFactory;

//GE_MEMORY_WATCHER_DEFINITION(EventData);

BaseEventManager* BaseEventManager::Get(void)
{
    LogAssert(g_pEventMgr, "Event manager doesn't exist");
	return g_pEventMgr;
}

BaseEventManager::BaseEventManager(const char* pName, bool setAsGlobal)
{
	if (setAsGlobal)
    {
        if (g_pEventMgr)
        {
            LogError("Attempting to create two global event managers! \
					The old one will be destroyed and overwritten with this one.");
            delete g_pEventMgr;
        }

		g_pEventMgr = this;
    }
}

BaseEventManager::~BaseEventManager(void)
{
	if (g_pEventMgr == this)
		g_pEventMgr = NULL;
}


//---------------------------------------------------------------------------------------------------------------------
// EventManager::EventManager
//---------------------------------------------------------------------------------------------------------------------
EventManager::EventManager(const char* pName, bool setAsGlobal)
	: BaseEventManager(pName, setAsGlobal)
{
	m_activeQueue = 0;
}


//---------------------------------------------------------------------------------------------------------------------
// EventManager::~EventManager
//---------------------------------------------------------------------------------------------------------------------
EventManager::~EventManager()
{
	//
}


//---------------------------------------------------------------------------------------------------------------------
// EventManager::AddListener
//---------------------------------------------------------------------------------------------------------------------
bool EventManager::AddListener(const EventListenerDelegate& eventDelegate, const BaseEventType& type)
{
	//GE_LOG("Events", eastl::string("Attempting to add delegate function for event type: ") + eastl::string(type, 16));
	LogInformation("Events " + eastl::string("Attempting to add delegate function for event type: ") + eastl::to_string(type));

	EventListenerList& eventListenerList = m_eventListeners[type];  // this will find or create the entry
	for (auto it = eventListenerList.begin(); it != eventListenerList.end(); ++it)
	{
		if (eventDelegate == (*it))
		{
			LogWarning("Attempting to double-register a delegate");
			return false;
		}
	}

	eventListenerList.push_back(eventDelegate);
	//GE_LOG("Events", eastl::string("Successfully added delegate for event type: ") + eastl::string(type, 16));
	LogInformation("Events " + eastl::string("Successfully added delegate for event type: ") + eastl::to_string(type));

	return true;
}


//---------------------------------------------------------------------------------------------------------------------
// EventManager::RemoveListener
//---------------------------------------------------------------------------------------------------------------------
bool EventManager::RemoveListener(const EventListenerDelegate& eventDelegate, const BaseEventType& type)
{
	//GE_LOG("Events", eastl::string("Attempting to remove delegate function from event type: ") + eastl::string(type, 16));
	LogInformation("Events " + eastl::string("Attempting to remove delegate function from event type: ") + eastl::to_string(type));
	bool success = false;

	auto findIt = m_eventListeners.find(type);
	if (findIt != m_eventListeners.end())
	{
		EventListenerList& listeners = findIt->second;
		for (auto it = listeners.begin(); it != listeners.end(); ++it)
		{
			if (eventDelegate == (*it))
			{
				listeners.erase(it);
				//GE_LOG("Events", eastl::string("Successfully removed delegate function from event type: ") + eastl::string(type, 16));
				LogInformation("Events " + eastl::string("Successfully removed delegate function from event type: ") + eastl::to_string(type));
				success = true;
				break;  // we don't need to continue because it should be impossible for the same delegate function to be registered for the same event more than once
			}
		}
	}

	return success;
}


//---------------------------------------------------------------------------------------------------------------------
// EventManager::VTrigger
//---------------------------------------------------------------------------------------------------------------------
bool EventManager::TriggerEvent(const BaseEventDataPtr& pEvent) const
{
	LogInformation("Events " + eastl::string("Attempting to trigger event ") + eastl::string(pEvent->GetName()));
	bool processed = false;

	auto findIt = m_eventListeners.find(pEvent->GetEventType());
	if (findIt != m_eventListeners.end())
	{
		const EventListenerList& eventListenerList = findIt->second;
		for (EventListenerList::const_iterator it = eventListenerList.begin(); it != eventListenerList.end(); ++it)
		{
			EventListenerDelegate listener = (*it);
			LogInformation("Events " + eastl::string("Sending Event ") + eastl::string(pEvent->GetName()) + eastl::string(" to delegate."));
			listener(pEvent);  // call the delegate
			processed = true;
		}
	}

	return processed;
}


//---------------------------------------------------------------------------------------------------------------------
// EventManager::QueueEvent
//---------------------------------------------------------------------------------------------------------------------
bool EventManager::QueueEvent(const BaseEventDataPtr& pEvent)
{
	LogAssert(m_activeQueue >= 0, "Queue active");
	LogAssert(m_activeQueue < EVENTMANAGER_NUM_QUEUES, "Queue active max");

	// make sure the event is valid
	if (!pEvent)
	{
		LogError("Invalid event in VQueueEvent()");
		return false;
	}

	LogInformation("Events " + eastl::string("Attempting to queue event: ") + eastl::string(pEvent->GetName()));

	auto findIt = m_eventListeners.find(pEvent->GetEventType());
	if (findIt != m_eventListeners.end())
	{
		m_queues[m_activeQueue].push_back(pEvent);
		LogInformation("Events " + eastl::string("Successfully queued event: ") + eastl::string(pEvent->GetName()));
		return true;
	}
	else
	{
		LogInformation("Events " + eastl::string("Skipping event since there are no delegates registered to receive it: ") + eastl::string(pEvent->GetName()));
		return false;
	}
}


//---------------------------------------------------------------------------------------------------------------------
// EventManager::ThreadSafeQueueEvent
//---------------------------------------------------------------------------------------------------------------------
bool EventManager::ThreadSafeQueueEvent(const BaseEventDataPtr& pEvent)
{
	m_realtimeEventQueue.Push(pEvent);
	return true;
}


//---------------------------------------------------------------------------------------------------------------------
// EventManager::AbortEvent
//---------------------------------------------------------------------------------------------------------------------
bool EventManager::AbortEvent(const BaseEventType& inType, bool allOfType)
{
	LogAssert(m_activeQueue >= 0, "Queue active");
	LogAssert(m_activeQueue < EVENTMANAGER_NUM_QUEUES, "Queue active max");

	bool success = false;
	EventListenerMap::iterator findIt = m_eventListeners.find(inType);

	if (findIt != m_eventListeners.end())
	{
		EventQueue& eventQueue = m_queues[m_activeQueue];
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


//---------------------------------------------------------------------------------------------------------------------
// EventManager::Tick
//---------------------------------------------------------------------------------------------------------------------
bool EventManager::Update(unsigned long maxMillis)
{
	unsigned long currMs = GetTickCount();
	unsigned long maxMs = ((maxMillis == BaseEventManager::kINFINITE) ? (BaseEventManager::kINFINITE) : (currMs + maxMillis));

	// This section added to handle events from other threads.  Check out Chapter 20.
	BaseEventDataPtr pRealtimeEvent;
	while (m_realtimeEventQueue.Pop(pRealtimeEvent))
	{
		QueueEvent(pRealtimeEvent);

		currMs = GetTickCount();
		if (maxMillis != BaseEventManager::kINFINITE)
		{
			if (currMs >= maxMs)
			{
				LogError("A realtime process is spamming the event manager!");
			}
		}
	}

	// swap active queues and clear the new queue after the swap
	int queueToProcess = m_activeQueue;
	m_activeQueue = (m_activeQueue + 1) % EVENTMANAGER_NUM_QUEUES;
	m_queues[m_activeQueue].clear();

	LogInformation("EventLoop " + eastl::string("Processing Event Queue ") + eastl::to_string(queueToProcess) + "; "
		+ eastl::to_string((unsigned long)m_queues[queueToProcess].size()) + eastl::string(" events to process"));

	// Process the queue
	while (!m_queues[queueToProcess].empty())
	{
		// pop the front of the queue
		BaseEventDataPtr pEvent = m_queues[queueToProcess].front();
		m_queues[queueToProcess].pop_front();
		LogInformation("EventLoop " + eastl::string("\t\tProcessing Event ") + eastl::string(pEvent->GetName()));

		const BaseEventType& eventType = pEvent->GetEventType();

		// find all the delegate functions registered for this event
		auto findIt = m_eventListeners.find(eventType);
		if (findIt != m_eventListeners.end())
		{
			const EventListenerList& eventListeners = findIt->second;
			LogInformation("EventLoop " + eastl::string("\t\tFound ") + eastl::to_string((unsigned long)eventListeners.size())
				+ eastl::string(" delegates"));

			// call each listener
			for (auto it = eventListeners.begin(); it != eventListeners.end(); ++it)
			{
				EventListenerDelegate listener = (*it);
				LogInformation("EventLoop " + eastl::string("\t\tSending event ") + eastl::string(pEvent->GetName())
					+ eastl::string(" to delegate"));
				listener(pEvent);
			}
		}

		// check to see if time ran out
		currMs = GetTickCount();
		if (maxMillis != BaseEventManager::kINFINITE && currMs >= maxMs)
		{
			LogInformation("EventLoop Aborting event processing; time ran out");
			break;
		}
	}

	// If we couldn't process all of the events, push the remaining events to the new active queue.
	// Note: To preserve sequencing, go back-to-front, inserting them at the head of the active queue
	bool queueFlushed = (m_queues[queueToProcess].empty());
	if (!queueFlushed)
	{
		while (!m_queues[queueToProcess].empty())
		{
			BaseEventDataPtr pEvent = m_queues[queueToProcess].back();
			m_queues[queueToProcess].pop_back();
			m_queues[m_activeQueue].push_front(pEvent);
		}
	}

	return queueFlushed;
}


