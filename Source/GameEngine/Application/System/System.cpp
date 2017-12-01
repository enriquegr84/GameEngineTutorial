// Geometric Tools, LLC
// Copyright (c) 1998-2014
// Distributed under the Boost Software License, Version 1.0.
// http://www.boost.org/LICENSE_1_0.txt
// http://www.geometrictools.com/License/Boost/LICENSE_1_0.txt
//
// File Version: 5.0.2 (2011/08/13)

#include "System.h"

#include "Core/Core.h"

System* System::mSystem = NULL;

System* System::Get(void)
{
	LogAssert(System::mSystem, "System doesn't exist");
	return System::mSystem;
}

//----------------------------------------------------------------------------
System::System (int width, int height)
	: mClose(false), mFullscreen(false), mEventListener(0), mCursorControl(0), mWidth(width), mHeight(height)
{
    LogAssert(width > 0, "Width must be positive\n");

    LogAssert(height > 0, "Height must be positive\n");

    mWidth = width;
    mHeight = height;

	mResized = false;
	mExternalWindow = false;
	mChangedToFullScreen = false;

	if (System::mSystem)
	{
		LogError("Attempting to create two global system! \
					The old one will be destroyed and overwritten with this one.");
		delete System::mSystem;
	}

	System::mSystem = this;
}
//----------------------------------------------------------------------------

System::~System()
{
	delete mEventListener;
	delete mCursorControl;

	mEventListener = nullptr;
	mCursorControl = nullptr;

	if (System::mSystem == this)
		System::mSystem = nullptr;
}

//----------------------------------------------------------------------------
//! Compares to the last call of this function to return double and triple clicks.
unsigned int System::CheckSuccessiveClicks(signed int mouseX, signed int mouseY, MouseInputEvent inputEvent )
{
	// we just have to make it public
	const signed int MAX_MOUSEMOVE = 3;
	
	unsigned int clickTime = Timer::GetRealTime();

	if ( (clickTime-mMouseMultiClicks.mLastClickTime) < mMouseMultiClicks.mDoubleClickTime
		&& mMouseMultiClicks.mCountSuccessiveClicks < 3
		&& mMouseMultiClicks.mLastMouseInputEvent == inputEvent)
	{
		++mMouseMultiClicks.mCountSuccessiveClicks;
	}
	else
	{
		mMouseMultiClicks.mCountSuccessiveClicks = 1;
	}

	mMouseMultiClicks.mLastMouseInputEvent = inputEvent;
	mMouseMultiClicks.mLastClickTime = clickTime;

	return mMouseMultiClicks.mCountSuccessiveClicks;
}

//! send the event to the right listener
bool System::OnPostEvent(const Event& event)
{
	bool absorbed = false;

	if (mEventListener)
		absorbed = mEventListener->OnEvent(event);

	return absorbed;
}

//! Sets a new event listener to receive events
void System::SetEventListener(EventListener* listener)
{
	mEventListener = listener;
}


//! Checks if the window is running in fullscreen mode
bool System::IsFullscreen() const
{
	return mFullscreen;
}

//	Set the maximal elapsed time between 2 clicks to generate doubleclicks for the mouse. 
//	It also affects tripleclick behavior.
void System::SetMouseDoubleClickTime( unsigned int timeMs )
{
	mMouseMultiClicks.mDoubleClickTime = timeMs;
}

//	Get the maximal elapsed time between 2 clicks to generate double- and tripleclicks 
//	for the mouse.
unsigned int System::GetMouseDoubleClickTime() const
{
	return mMouseMultiClicks.mDoubleClickTime;
}