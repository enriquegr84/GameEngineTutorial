 //========================================================================
// EventManager.h : implements a multi-listener multi-sender event system
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


#ifndef EVENTMANAGER_H
#define EVENTMANAGER_H

#include <strstream>

//---------------------------------------------------------------------------------------------------------------------
// Forward declaration & typedefs
//---------------------------------------------------------------------------------------------------------------------
class EventData;

typedef unsigned long EventType;
typedef shared_ptr<EventData> EventDataPtr;
typedef fastdelegate::FastDelegate1<EventDataPtr> EventListenerDelegate;
typedef concurrent_queue<EventDataPtr> ThreadSafeEventQueue;


//---------------------------------------------------------------------------------------------------------------------
// Macro for event registration
//---------------------------------------------------------------------------------------------------------------------
extern GenericObjectFactory<BaseEventData, EventType> g_eventFactory;
#define REGISTER_EVENT(eventClass) g_eventFactory.Register<eventClass>(eventClass::sk_EventType)
#define CREATE_EVENT(eventType) g_eventFactory.Create(eventType)


//---------------------------------------------------------------------------------------------------------------------
// EventData                               - Chapter 11, page 310
// Base type for event object hierarchy, may be used itself for simplest event notifications such as those that do 
// not carry additional payload data. If any event needs to propagate with payload data it must be defined separately.
//---------------------------------------------------------------------------------------------------------------------
class EventData 
{
public:
	virtual ~EventData(void) {}
	virtual const EventType& GetEventType(void) const = 0;
	virtual float GetTimeStamp(void) const = 0;
	virtual void Serialize(std::ostrstream& out) const = 0;
    virtual void Deserialize(std::istrstream& in) = 0;
	virtual EventDataPtr Copy(void) const = 0;
    virtual const char* GetName(void) const = 0;

    //GE_MEMORY_WATCHER_DECLARATION();
};


//---------------------------------------------------------------------------------------------------------------------
// class BaseEventData		- Chapter 11, page 311
//---------------------------------------------------------------------------------------------------------------------
class BaseEventData : public EventData
{
    const float m_timeStamp;

public:
	explicit BaseEventData(const float timeStamp = 0.0f) : m_timeStamp(timeStamp) { }

	// Returns the type of the event
	virtual const EventType& GetEventType(void) const = 0;

	float GetTimeStamp(void) const { return m_timeStamp; }

	// Serializing for network input / output
	virtual void Serialize(std::ostrstream &out) const	{ }
    virtual void Deserialize(std::istrstream& in) { }
};


//---------------------------------------------------------------------------------------------------------------------
// BaseEventManager Description                        Chapter 11, page 314
//
// This is the object which maintains the list of registered events and their listeners.
//
// This is a many-to-many relationship, as both one listener can be configured to process multiple event types and 
// of course multiple listeners can be registered to each event type.
//
// The interface to this construct uses smart pointer wrapped objects, the purpose being to ensure that no object 
// that the registry is referring to is destroyed before it is removed from the registry AND to allow for the registry 
// to be the only place where this list is kept ... the application code does not need to maintain a second list.
//
// Simply tearing down the registry (e.g.: destroying it) will automatically clean up all pointed-to objects (so long 
// as there are no other outstanding references, of course).
//---------------------------------------------------------------------------------------------------------------------
class BaseEventManager
{
public:

	enum eConstants { kINFINITE = 0xffffffff };

	explicit BaseEventManager(const char* pName, bool setAsGlobal);
	virtual ~BaseEventManager(void);

    // Registers a delegate function that will get called when the event type is triggered.  Returns true if 
    // successful, false if not.
    virtual bool AddListener(const EventListenerDelegate& eventDelegate, const EventType& type) = 0;

	// Removes a delegate / event type pairing from the internal tables.  Returns false if the pairing was not found.
	virtual bool RemoveListener(const EventListenerDelegate& eventDelegate, const EventType& type) = 0;

	// Fire off event NOW.  This bypasses the queue entirely and immediately calls all delegate functions registered 
    // for the event.
	virtual bool TriggerEvent(const EventDataPtr& pEvent) const = 0;

	// Fire off event.  This uses the queue and will call the delegate function on the next call to VTick(), assuming
    // there's enough time.
	virtual bool QueueEvent(const EventDataPtr& pEvent) = 0;
	virtual bool ThreadSafeQueueEvent(const EventDataPtr& pEvent) = 0;

	// Find the next-available instance of the named event type and remove it from the processing queue.  This 
    // may be done up to the point that it is actively being processed ...  e.g.: is safe to happen during event
	// processing itself.
	//
	// if allOfType is true, then all events of that type are cleared from the input queue.
	//
	// returns true if the event was found and removed, false otherwise
	virtual bool AbortEvent(const EventType& type, bool allOfType = false) = 0;

	// Allow for processing of any queued messages, optionally specify a processing time limit so that the event 
    // processing does not take too long. Note the danger of using this artificial limiter is that all messages 
    // may not in fact get processed.
	//
	// returns true if all messages ready for processing were completed, false otherwise (e.g. timeout )
	virtual bool Update(unsigned long maxMillis = kINFINITE) = 0;

    // Getter for the main global event manager.  This is the event manager that is used by the majority of the 
    // engine, though you are free to define your own as long as you instantiate it with setAsGlobal set to false.
    // It is not valid to have more than one global event manager.
	static BaseEventManager* Get(void);

};

const unsigned int EVENTMANAGER_NUM_QUEUES = 2;

class EventManager : public BaseEventManager
{
	typedef eastl::list<EventListenerDelegate> EventListenerList;
	typedef eastl::map<EventType, EventListenerList> EventListenerMap;
	typedef eastl::list<EventDataPtr> EventQueue;

	EventListenerMap m_eventListeners;
	EventQueue m_queues[EVENTMANAGER_NUM_QUEUES];
	int m_activeQueue;  // index of actively processing queue; events enque to the opposing queue

	ThreadSafeEventQueue m_realtimeEventQueue;

public:
	explicit EventManager(const char* pName, bool setAsGlobal);
	virtual ~EventManager(void);

	virtual bool AddListener(const EventListenerDelegate& eventDelegate, const EventType& type);
	virtual bool RemoveListener(const EventListenerDelegate& eventDelegate, const EventType& type);

	virtual bool TriggerEvent(const EventDataPtr& pEvent) const;
	virtual bool QueueEvent(const EventDataPtr& pEvent);
	virtual bool ThreadSafeQueueEvent(const EventDataPtr& pEvent);
	virtual bool AbortEvent(const EventType& type, bool allOfType = false);

	virtual bool Update(unsigned long maxMillis = kINFINITE);
};


#endif