// Geometric Tools, LLC
// Copyright (c) 1998-2014
// Distributed under the Boost Software License, Version 1.0.
// http://www.boost.org/LICENSE_1_0.txt
// http://www.geometrictools.com/License/Boost/LICENSE_1_0.txt
//
// File Version: 5.0.2 (2011/08/13)

#ifndef SYSTEM_H
#define SYSTEM_H

#include "Core/CoreStd.h"

#include "CursorControl.h"
#include "EventSystem.h"

/*
	Class System abstracts the platform implementation for different systems.
	It reads the state of the input device and report it to the game logic as an 
	event which is translated into game commands
*/
class System
{
public:

	System(int width, int height);

	~System();

	//! Sends a user created event to the engine.
	/** Is is usually not necessary to use this. However, if you are using an own input library
	for example for doing joystick input, you can use this to post key or mouse input events to
	the engine. Internally, this method only delegates the events further to the scene manager
	and the UI environment. */
	bool OnPostEvent(const Event& event);

	//! Sets a new user event listener which will receive events from the engine.
	/** Return true in EventListener::OnEvent to prevent the event from continuing along
	the chain of event listeners. The path that an event takes through the system depends
	on its type. See EventType for details.
	\param listener New receiver to be used. */
	void SetEventListener(EventListener* listener);

	//! Runs the device.
	/** Also increments the virtual timer by calling
	ITimer::tick();. You can prevent this
	by calling ITimer::stop(); before and ITimer::start() after
	calling GameEngineDevice::run(). Returns false if device wants
	to be deleted. Use it in this way:
	\code
	while(device->run())
	{
		// draw everything here
	}
	\endcode
	If you want the device to do nothing if the window is inactive
	(recommended), use the slightly enhanced code shown at isWindowActive().

	Note if you are running GameEngine inside an external, custom
	created window: Calling Device->run() will cause GameEngine to
	dispatch windows messages internally.
	If you are running GameEngine in your own custom window, you can
	also simply use your own message loop using GetMessage,
	DispatchMessage and whatever and simply don't use this method.
	But note that GameEngine will not be able to fetch user input
	then. See SGameEngineCreationParameters::WindowId for more
	informations and example code.
	*/
	virtual bool OnRun() = 0;

	//! Pause execution and let other processes to run for a specified amount of time.
	/** It may not wait the full given time, as sleep may be interrupted
	\param timeMs: Time to sleep for in milisecs.
	\param pauseTimer: If true, pauses the device timer while sleeping
	*/
	virtual void OnPause(unsigned int timeMs, bool pauseTimer = false) = 0;

	//! Notifies the device that it should close itself.
	/** GameEngineDevice::run() will always return false after closeDevice() was called. */
	virtual void OnClose() = 0;

	//! Process message
	virtual void ProcessMessage(int* HWndPtrAddress, int msg, int wParam, int lParam) = 0;

	//! Remove messages pending in the system message loop
	/** This function is usually used after messages have been buffered for a longer time, 
	for example when loading a large scene. Clearing the message loop prevents that mouse- 
	or buttonclicks which users have pressed in the meantime will now trigger unexpected 
	actions in the gui. */
	virtual void ClearSystemMessages() = 0;

	virtual void HandleSystemMessages() = 0;

	//! Gets a list with all video resolutions available.
	/*
	\return Pointer to a list with all video modes supported
	by the gfx adapter. */
	virtual eastl::vector<Vector2<unsigned int>> GetVideoResolutions() = 0;

	//! Notifies the device, that it has been resized
	virtual void OnResized() = 0;

	virtual void ResizeIfNecessary() = 0;

	//! Minimizes the window.
	virtual void OnMinimizeWindow() = 0;

	//! Maximizes the window.
	virtual void OnMaximizeWindow() = 0;

	//! Restore the window to normal size if possible.
	virtual void OnRestoreWindow() = 0;

	//! Sets the caption of the window.
	/** \param text: New text of the window caption. */
	virtual void SetWindowCaption(const wchar_t* text) = 0;

	//! Sets if the window should be resizable in windowed mode.
	/** The default is false. This method only works in windowed
	mode.
	\param resize Flag whether the window should be resizable. */
	virtual void SetResizable(bool resize=false) = 0;

	//! Returns if the window is active.
	/** If the window is inactive,
	nothing needs to be drawn. So if you don't want to draw anything
	when the window is inactive, create your drawing loop this way:
	\code
	while(device->run())
	{
		if (device->isWindowActive())
		{
			// draw everything here
		}
		else
			device->yield();
	}
	\endcode
	\return True if window is active. */
	virtual bool IsWindowActive() const = 0;

	//! Checks if the GameEngine window has focus
	/** \return True if window has focus. */
	virtual bool IsWindowFocused() const = 0;

	//! Checks if the GameEngine window is minimized
	/** \return True if window is minimized. */
	virtual bool IsWindowMinimized() const = 0;

	//! switchs to fullscreen
	virtual bool SwitchToFullScreen(bool reset = false) = 0;

	virtual bool IsOnlyInstance(const wchar_t* gameTitle) = 0;

	virtual void* GetID() const { return NULL; }

	virtual void GetSystemVersion(eastl::string& out) { }

	//! Checks if the window is running in fullscreen mode
	bool IsFullscreen() const { return mFullscreen; }

	void SetFullscreen(bool fullscreen) { mFullscreen = fullscreen; }

	int GetWidth() const { return mWidth; }
	int GetHeight() const { return mHeight; }

	//! returns the cursor control
	BaseCursorControl* GetCursorControl() const { return mCursorControl; }

	//	Set the maximal elapsed time between 2 clicks to generate doubleclicks for the mouse. 
	//	It also affects tripleclick behavior. When set to 0 no double- and tripleclicks will 
	//	be generated.
	//	\param timeMs maximal time in milliseconds for two consecutive clicks to be recognized 
	//	as double click
	void SetMouseDoubleClickTime( unsigned int timeMs );
	
	//	Get the maximal elapsed time between 2 clicks to generate double- and tripleclicks 
	//	for the mouse. When return value is 0 no double- and tripleclicks will be generated.
	//	return maximal time in milliseconds for two consecutive clicks to be recognized as 
	//	double click
	unsigned int GetMouseDoubleClickTime() const;

	//! Compares to the last call of this function to return double and triple clicks.
	/** Needed for Windows device event handling
	\return Returns only 1,2 or 3. A 4th click will start with 1 again.*/
	unsigned int CheckSuccessiveClicks(signed int mouseX, signed int mouseY, MouseInputEvent inputEvent);

	// Getter for the main global system. This is the system that is used by the majority of the 
	// engine, though you are free to define your own as long as you instantiate it.
	// It is not valid to have more than one global system.
	static System* Get(void);

protected:

	static System* mSystem;

	struct MouseMultiClicks
	{
		MouseMultiClicks()
		: mDoubleClickTime(500), mCountSuccessiveClicks(0), mLastMouseInputEvent(MIE_COUNT), mLastClickTime(0)
		{}

		unsigned int mDoubleClickTime;
		unsigned int mCountSuccessiveClicks;
		unsigned int mLastClickTime;
		//Position2i mLastClick;
		MouseInputEvent mLastMouseInputEvent;
	};
	MouseMultiClicks mMouseMultiClicks;

	EventListener* mEventListener;
	BaseCursorControl* mCursorControl;
	//VideoModeList mVideoModeList;

	//! Size of the window or the video mode in fullscreen mode. Default: 800x600
	//Dimension2<unsigned int> mWindowSize;
	unsigned int mWidth, mHeight;
	
	//! Minimum Bits per pixel of the color buffer in fullscreen mode. Ignored if windowed mode. Default: 16.
	unsigned char mBits;

	bool mClose;
	bool mResized;
	bool mFullscreen;
	bool mExternalWindow;
	bool mChangedToFullScreen;
};

#endif

