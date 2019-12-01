// Geometric Tools, LLC
// Copyright (c) 1998-2014
// Distributed under the Boost Software License, Version 1.0.
// http://www.boost.org/LICENSE_1_0.txt
// http://www.geometrictools.com/License/Boost/LICENSE_1_0.txt
//
// File Version: 5.0.3 (2012/07/01)

#include "Core/Core.h"

#include "WindowsSystem.h"

#ifndef MOUSEWHEEL
#define MOUSEWHEEL 0x020A
#endif

#ifndef WHEEL_DELTA
#define WHEEL_DELTA 120
#endif

// We need to include windows.h
#include <winuser.h>

HKL KEYBOARD_INPUT_HKL=0;
unsigned int KEYBOARD_INPUT_CODEPAGE = 1252;
eastl::map<HWND, System*> WindowsSystem::mHandleSystems = 0;

// Windows programming steps:

// 1) Initialize a class
// 2) Register the class
// 3) Create the window
// 4) Get the messages from the WndProc.
// 5) Translate and Dispatch the Messages (Act of the messages)

//	Windows handles four types of device contexts -- They are:

//	Display -- These support drawing operations on a video display terminal
//	Printer -- These support writing/drawing operations on a printer
//	Memory -- These support drawing operations on a bitmap
//	Information -- These support retrieval of device data

//	In general, the main two we are going to concern ourselves are display and memory
//	device contexts.


/*
Platform-dependent class to Windows system. It contains members necessary for handling
Windows low level operations, create windows, process event messages...
*/
WindowsSystem::WindowsSystem(int width, int height)
	: System(width, height), mWindowID(0)
{
	// Store original desktop mode.
	memset(&mDesktopMode, 0, sizeof(mDesktopMode));
	mDesktopMode.dmSize = sizeof(mDesktopMode);

	EnumDisplaySettings(NULL, ENUM_CURRENT_SETTINGS, &mDesktopMode);

	// get handle to exe file
	HINSTANCE hInstance = GetModuleHandle(0);

	// The window is defined through the WNDCLASSEX type which can customize the window.
	if (!mWindowID)
	{
		MyRegisterClass(hInstance);

		// Perform application initialization:
		mWindowID = PtrToInt(InitInstance(hInstance, mWidth, mHeight, mFullscreen));

		// make sure everything gets updated to the real sizes
		mResized = true;
	}
	else
	{
		// attach external window
		HWND hWnd = (HWND)IntToPtr(mWindowID);
		RECT r;
		GetWindowRect(hWnd, &r);
		mWidth = r.right - r.left;
		mHeight = r.bottom - r.top;
		mFullscreen = false;
		mExternalWindow = true;
	}

	// create cursor control
	mCursorControl = new CursorControl(
		Vector2<unsigned int>{ mWidth, mHeight }, (HWND)IntToPtr(mWindowID), mFullscreen);

	// initialize doubleclicks with system values
	mMouseMultiClicks.mDoubleClickTime = GetMouseDoubleClickTime();

	// register system handle
	mHandleSystems[(HWND)IntToPtr(mWindowID)] = this;

	// set this as active window
	if (!mExternalWindow)
	{
		SetActiveWindow((HWND)IntToPtr(mWindowID));
		SetForegroundWindow((HWND)IntToPtr(mWindowID));
	}

	// get the codepage used for keyboard input
	KEYBOARD_INPUT_HKL = GetKeyboardLayout(0);
	KEYBOARD_INPUT_CODEPAGE = LocaleIdToCodepage(LOWORD(KEYBOARD_INPUT_HKL));

	// inform driver about the window size etc.
	//ResizeIfNecessary();
}

//! destructor
WindowsSystem::~WindowsSystem()
{
	mHandleSystems.erase((HWND)IntToPtr(mWindowID));

	// Just remember to call ChangeDisplaySettings() again before you close the program or
	// else you will be stuck in the changed mode until you manually change it in your
	// display settings from control panels.
	SwitchToFullScreen();
}

HWND WindowsSystem::GetHandleFromSystem(System* system)
{
	eastl::map<HWND, System*>::iterator it = mHandleSystems.begin();
	for (; it != mHandleSystems.end(); ++it)
		if ((*it).second == system)
			return (*it).first;

	return NULL;
}

System* WindowsSystem::GetSystemFromHandle(HWND hWnd)
{
	eastl::map<HWND, System*>::iterator it = mHandleSystems.begin();
	for (; it != mHandleSystems.end(); ++it)
		if ((*it).first == hWnd)
			return (*it).second;

	return NULL;
}

//
//  FUNCTION: MyRegisterClass()
//
//  PURPOSE: Registers the window class.
//	This function handles all the messages for windows and its usage are only necessary 
//	if you want this code to be compatible with Windows systems prior to the 'RegisterClassEx' 
//	It is important to call this function so that the application will get 'well formed' 
//	small icons associated with it.
//
//	HINSTANCE hInstance - Holds information on an instance of application. 
//	(2 of the same program can be running together...)
//	HINSTANCE hPrevInstance - Holds a previous instance or the application, 
//	which is always NULL (no information) for Windows apps.
//	PSTR szCmdLine - Holds the information from the command line. PSTR is a string.
//	int iCmdShow - This holds information on the window's state; maximized, minimized, hide...	
ATOM WindowsSystem::MyRegisterClass(HINSTANCE hInstance)
{
	const eastl::wstring className(L"WindowsSystem");

	// WNDCLASSEX - This variable will hold all the information about the window 
	// (The name, icon, cursor, color, menu bar...)
    WNDCLASSEX  wndclass;		
	// Set the size of the wndclass.
    wndclass.cbSize        = sizeof (wndclass);					
	// The style we want is Verticle-Redraw and Horizontal-Redraw. 
    wndclass.style         = CS_HREDRAW | CS_VREDRAW;						
	//	Assing CALLBACK function. "WndProc" tells windows which function it needs 
	//	to call to check for window messages.
    wndclass.lpfnWndProc   = WndProc;	
	// no need to allocate any extra bytes for a class 
    wndclass.cbClsExtra    = 0;								
    wndclass.cbWndExtra    = 0;						
	//	Assign hInstance to our window. We can have several instances of a program, 
	//	this keeps track of the current one.
    wndclass.hInstance     = hInstance;
	//	LoadIcon that returns information about what icon we want, Wndows Logo. 
	//	The NULL is in place of a hInstance.
    wndclass.hIcon         = NULL; //LoadIcon (NULL, IDI_WINLOGO);
	//	LoadCursor that returns information about what cursor we want, an arrow. 
	//	The NULL is in place of a hInstance.
	wndclass.hCursor       = 0; //LoadCursor (NULL, IDC_ARROW);
	//	Set the background color.  GetStockObject() returns a void so we must "cast" 
	//	(turn it into) it as a HBRUSH to be compatible with the variable "hbrBackground".
	wndclass.hbrBackground = (HBRUSH) GetStockObject (WHITE_BRUSH);						
	// We don't have a menu, set it to NULL.
	wndclass.lpszMenuName  = NULL;			
	//	Name for our class, to distinguish it against others. 
	//	We need to use this name for later when we create the window.					
	wndclass.lpszClassName = className.c_str();	
	// the windows logo icon. It is the icon in the top left corner of the window.
	wndclass.hIconSm       = 0; //LoadIcon (NULL, IDI_WINLOGO);	

	//	The "hIcon" is the member of WNDCLASSEX we need to fill to have an icon with 
	//	our application.
	//	hinstance -- Handle to the instance to load the icon from, we're are passed 
	//	this by WinMain()
	//	Image handle -- a handle to the image to be loaded.
	//	IMAGE_ICON -- Specifies the type of image to be loaded
	//	0 -- Width in pixels of image to be loaded
	//	0 -- Height in pixels of image to be loaded
	//	LR_LOADFROMFILE -- load an icon
	wndclass.hIcon = (HICON)LoadImage(
		hInstance, L"GameEngine.ico", IMAGE_ICON, 0,0, LR_LOADFROMFILE);

	// We need to register our windows class with the windows OS.  
	// We also need to pass the memory address of wndclass, so we use the "&".
	return RegisterClassEx (&wndclass);	
}

//	WndProc stands for "Window Procedure". This function handles the messages from windows,
//	it simply passes all messages to the default windows procedure, except for destroy it.
//	As result, it returns a 32-bit call back or function pointer. You give the address of 
//	the function to the window class, and windows fills in all the parameters for us.
//	HWnd - a HWnd is a handle to a window. This is used to keep track of a certain window. 
//	Programs can have multiple windows. We only have one though.
//	MSG  - A MeSsaGe variable to hold what messages are being sent to the window. 
//	(If the window was clicked, closed, moved, etc...).
//	The WPARAM and LPARAM hold information about the message.
LRESULT CALLBACK WindowsSystem::WndProc (HWND hWnd, UINT iMsg, WPARAM wParam, LPARAM lParam)
{
	System* dev = NULL;
	Event evt;

	#ifndef WM_MOUSEWHEEL
	#define WM_MOUSEWHEEL 0x020A
	#endif
	#ifndef WHEEL_DELTA
	#define WHEEL_DELTA 120
	#endif

	static signed int ClickCount=0;
	if (GetCapture() != hWnd && ClickCount > 0)
		ClickCount = 0;

	struct messageMap
	{
		signed int group;
		UINT winMessage;
		signed int Message;
	};

	static messageMap mouseMap[] =
	{
		{0, WM_LBUTTONDOWN, MIE_LMOUSE_PRESSED_DOWN},
		{1, WM_LBUTTONUP,   MIE_LMOUSE_LEFT_UP},
		{0, WM_RBUTTONDOWN, MIE_RMOUSE_PRESSED_DOWN},
		{1, WM_RBUTTONUP,   MIE_RMOUSE_LEFT_UP},
		{0, WM_MBUTTONDOWN, MIE_MMOUSE_PRESSED_DOWN},
		{1, WM_MBUTTONUP,   MIE_MMOUSE_LEFT_UP},
		{2, WM_MOUSEMOVE,   MIE_MOUSE_MOVED},
		{3, WM_MOUSEWHEEL,  MIE_MOUSE_WHEEL},
		{-1, 0, 0}
	};

	// handle grouped events
	messageMap * m = mouseMap;
	while ( m->group >=0 && m->winMessage != iMsg )
		m += 1;

	if ( m->group >= 0 )
	{
		if ( m->group == 0 )	// down
		{
			ClickCount++;
			SetCapture(hWnd);
		}
		else
		if ( m->group == 1 )	// up
		{
			ClickCount--;
			if (ClickCount<1)
			{
				ClickCount=0;
				ReleaseCapture();
			}
		}

		evt.mEventType = ET_MOUSE_INPUT_EVENT;
		evt.mMouseInput.mEvent = (MouseInputEvent) m->Message;
		evt.mMouseInput.X = (short)LOWORD(lParam);
		evt.mMouseInput.Y = (short)HIWORD(lParam);
		evt.mMouseInput.mShift = ((LOWORD(wParam) & MK_SHIFT) != 0);
		evt.mMouseInput.mControl = ((LOWORD(wParam) & MK_CONTROL) != 0);
		// left and right mouse buttons
		evt.mMouseInput.mButtonStates = wParam & ( MK_LBUTTON | MK_RBUTTON);
		// middle and extra buttons
		if (wParam & MK_MBUTTON)
			evt.mMouseInput.mButtonStates |= MBSM_MIDDLE;
#if(_Windows_WINNT >= 0x0500)
		if (wParam & MK_XBUTTON1)
			evt.mMouseInput.mButtonStates |= MBSM_EXTRA1;
		if (wParam & MK_XBUTTON2)
			evt.mMouseInput.mButtonStates |= MBSM_EXTRA2;
#endif
		evt.mMouseInput.mWheel = 0.f;

		// wheel
		if ( m->group == 3 )
		{
			POINT p; // fixed by jox
			p.x = 0; p.y = 0;
			ClientToScreen(hWnd, &p);
			evt.mMouseInput.X -= p.x;
			evt.mMouseInput.Y -= p.y;
			evt.mMouseInput.mWheel = (float)(short)HIWORD(wParam) / (float)WHEEL_DELTA;
		}
		
		dev = mHandleSystems[hWnd];
		if (dev)
		{
			dev->OnPostEvent(evt);

			if ( evt.mMouseInput.mEvent >= MIE_LMOUSE_PRESSED_DOWN && 
				 evt.mMouseInput.mEvent <= MIE_MMOUSE_PRESSED_DOWN )
			{
				unsigned int clicks = dev->CheckSuccessiveClicks(
					evt.mMouseInput.X, evt.mMouseInput.Y, evt.mMouseInput.mEvent);
				if ( clicks == 2 )
				{
					evt.mMouseInput.mEvent = 
						(MouseInputEvent)(MIE_LMOUSE_DOUBLE_CLICK + 
						evt.mMouseInput.mEvent-MIE_LMOUSE_PRESSED_DOWN);
					dev->OnPostEvent(evt);
				}
				else if ( clicks == 3 )
				{
					evt.mMouseInput.mEvent = 
						(MouseInputEvent)(MIE_LMOUSE_TRIPLE_CLICK + 
						evt.mMouseInput.mEvent-MIE_LMOUSE_PRESSED_DOWN);
					dev->OnPostEvent(evt);
				}
			}

		}
		return 0;
	}

	//	This checks what the message is. Below is some of the message that windows 
	//	might return.	
    switch (iMsg)											
	{
		// This message is sent when the window is created.
		case WM_CREATE:
			break; // We break from the switch statement.

		// This Message happens when we resize the window.
		case WM_SIZE: 
		{
			// resize
			dev = mHandleSystems[hWnd];
			if (dev)
				dev->OnResized();
			return 0;
		}

		case WM_SYSCOMMAND:
		{
			// prevent screensaver or monitor powersave mode from starting
			if ((wParam & 0xFFF0) == SC_SCREENSAVE ||
				(wParam & 0xFFF0) == SC_MONITORPOWER ||
				(wParam & 0xFFF0) == SC_KEYMENU)
				return 0;
		}
		break;

		case WM_ACTIVATE:
		{
			// we need to take care for screen changes, e.g. Alt-Tab
			dev = mHandleSystems[hWnd];
			if (dev && dev->IsFullscreen())
			{
				if ((wParam&0xFF)==WA_INACTIVE)
				{
					// If losing focus we minimize the app to show other one
					ShowWindow(hWnd,SW_MINIMIZE);
					// and switch back to default resolution
					dev->SwitchToFullScreen();
				}
				else
				{
					// Otherwise we retore the fullscreen GameEngine App
					SetForegroundWindow(hWnd);
					ShowWindow(hWnd, SW_RESTORE);
					// and set the fullscreen resolution again
					dev->SwitchToFullScreen();
				}
			}
		}
		break;

		case WM_USER:
		{
			evt.mEventType = ET_USER_EVENT;
			evt.mUserEvent.mUserData1 = (signed int)wParam;
			evt.mUserEvent.mUserData2 = (signed int)lParam;
			dev = mHandleSystems[hWnd];

			if (dev)
				dev->OnPostEvent(evt);

			return 0;
		}

		case WM_INPUTLANGCHANGE:
		{
			// get the new codepage used for keyboard input
			KEYBOARD_INPUT_HKL = GetKeyboardLayout(0);
			KEYBOARD_INPUT_CODEPAGE = LocaleIdToCodepage( LOWORD(KEYBOARD_INPUT_HKL) );
			return 0;
		}

		//	This message is sent to the WndProc when the window needs to be repainted.  
		//	This might be if we moved the window, resized it, or maximized it, 
		//	or another window was over it.
		case WM_PAINT:
		{
			PAINTSTRUCT ps;
			BeginPaint(hWnd, &ps);
			EndPaint(hWnd, &ps);
		}
		break;

		// This message is sent when the user closes the window.
		case WM_DESTROY:
		{
			//	Here you would handle Deinitialization, freeing memory, etc..
			//	You must call this function or else you will need to do control-alt-delete 
			//	and manually close the program from the programs queue.  0 = WM_QUIT.
			//	This function actually puts a message on the message queue.  SendMessage() 
			//	sends a direct message, which is acted upon immediately. just for your information.
			PostQuitMessage(0);	
		}
		break;	// Break from the switch statement

		case WM_SYSKEYDOWN:
		case WM_SYSKEYUP:
		case WM_KEYDOWN:
		case WM_KEYUP:
		{
			BYTE allKeys[256];

			evt.mEventType = ET_KEY_INPUT_EVENT;
			evt.mKeyInput.mKey = (KeyCode)wParam;
			evt.mKeyInput.mPressedDown = (iMsg==WM_KEYDOWN || iMsg == WM_SYSKEYDOWN);

			// MAPVK_VSC_TO_VK_EX should be in SDK according to MSDN, but isn't in mine.
			const UINT MY_MAPVK_VSC_TO_VK_EX = 3; 
			if ( evt.mKeyInput.mKey == KEY_SHIFT )
			{
				//	this will fail on systems before windows NT/2000/XP, 
				//	not sure _what_ will return there instead.
				evt.mKeyInput.mKey = (KeyCode)
					MapVirtualKey( ((lParam>>16) & 255), MY_MAPVK_VSC_TO_VK_EX );
			}
			if ( evt.mKeyInput.mKey == KEY_CONTROL )
			{
				evt.mKeyInput.mKey = (KeyCode)
					MapVirtualKey( ((lParam>>16) & 255), MY_MAPVK_VSC_TO_VK_EX );
				//	some keyboards will just return LEFT for both - left and right keys. 
				//	So also check extend bit.
				if (lParam & 0x1000000)
					evt.mKeyInput.mKey = KEY_RCONTROL;
			}
			if ( evt.mKeyInput.mKey == KEY_MENU )
			{
				evt.mKeyInput.mKey = (KeyCode)
					MapVirtualKey( ((lParam>>16) & 255), MY_MAPVK_VSC_TO_VK_EX );
				if (lParam & 0x1000000)
					evt.mKeyInput.mKey = KEY_RMENU;
			}

			GetKeyboardState(allKeys);

			evt.mKeyInput.mShift = ((allKeys[VK_SHIFT] & 0x80)!=0);
			evt.mKeyInput.mControl = ((allKeys[VK_CONTROL] & 0x80)!=0);

			//	Handle unicode and deadkeys in a way that works since Windows 95 and nt4.0
			//	Using ToUnicode instead would be shorter, but would to my knowledge not run 
			//	on 95 and 98.
			WORD keyChars[2];
			UINT scanCode = HIWORD(lParam);
			int conversionResult = 
				ToAsciiEx(wParam,scanCode,allKeys,keyChars,0,KEYBOARD_INPUT_HKL);
			if (conversionResult == 1)
			{
				WORD unicodeChar;
				MultiByteToWideChar(
						KEYBOARD_INPUT_CODEPAGE,
						MB_PRECOMPOSED, // default
						(LPCSTR)keyChars,
						sizeof(keyChars),
						(WCHAR*)&unicodeChar,
						1 );
				evt.mKeyInput.mChar = unicodeChar;
			}
			else evt.mKeyInput.mChar = 0;

			// allow composing characters like '@' with Alt Gr on non-US keyboards
			if ((allKeys[VK_MENU] & 0x80) != 0)
				evt.mKeyInput.mControl = 0;

			dev = mHandleSystems[hWnd];
			if (dev)
				dev->OnPostEvent(evt);

			if (iMsg == WM_SYSKEYDOWN || iMsg == WM_SYSKEYUP)
				return DefWindowProc(hWnd, iMsg, wParam, lParam);
			else
				return 0;
		}

		case WM_SETCURSOR:

			dev = mHandleSystems[hWnd];
			if (dev)
			{
				dev->GetCursorControl()->SetActiveIcon(dev->GetCursorControl()->GetActiveIcon());
				dev->GetCursorControl()->SetVisible(dev->GetCursorControl()->IsVisible());
			}
			break;
	}												

	//	The DefWindowProc function calls the default window procedure to provide default 
	//	processing for any window messages that an application does not process. 
	//	This function ensures that every message is processed. DefWindowProc is called 
	//	with the same parameters received by the WndProc. End of the WndProc
	return DefWindowProc (hWnd, iMsg, wParam, lParam);
}

//----------------------------------------------------------------------------

//
//   FUNCTION: InitInstance(HINSTANCE, int)
//
//   PURPOSE: Saves instance handle and creates main window
//
//   COMMENTS:
//
//		In this function, we save the instance handle in a global variable and
//      create and display the main program window.
//
HWND WindowsSystem::InitInstance(HINSTANCE hInstance, int width, int height, bool fullscreen)
{
   	// calculate client size
	RECT clientSize;
	clientSize.top = 0;
	clientSize.left = 0;
	clientSize.right = width;		//mWindowSize.Width;
	clientSize.bottom = height;	//mWindowSize.Height;

	// CreateWindow() returns a handle to the window, which we store in our HWnd "HWnd".
	// ClassName. Window class name, tells CreateWindow() to use our class up above.

	const eastl::wstring className(L"WindowsSystem");

	// WindowsTitle. window's Title, it will be the name on the title bar of the window.
	// Window style flag to create a typical window, (options like resize, minimize, close, etc).

	unsigned long style = WS_POPUP;

	if (!fullscreen)
		style = WS_SYSMENU | WS_BORDER | WS_CAPTION | WS_CLIPCHILDREN | WS_CLIPSIBLINGS;

	AdjustWindowRect(&clientSize, style, FALSE);

	// Width. Initial x size. The bottom right corner X value of the window in screen coordinates.
	// Height. Initial y size. The bottom right corner Y value of the window in screen coordinates.

	const signed int realWidth = clientSize.right - clientSize.left;
	const signed int realHeight = clientSize.bottom - clientSize.top;

	// The Windows API GetSystemMetrics determines the screen's width and height, and returns a point
	// at the center of the screen
	// WindowLeft. Initial x position. The top left corner X value of the window in screen coordinates.
	// WindowTop. Initial y position. The top left corner Y value of the window in screen coordinates.
	signed int windowLeft = (GetSystemMetrics(SM_CXSCREEN) - realWidth) / 2;
	signed int windowTop = (GetSystemMetrics(SM_CYSCREEN) - realHeight) / 2;

	if ( windowLeft < 0 ) windowLeft = 0;
	if ( windowTop < 0 ) windowTop = 0;	// make sure window menus are in screen on creation

	if (fullscreen)
	{
		windowLeft = 0;
		windowTop = 0;
	}

	//	Window handle parent. Since we don't have a parent window, we set this to NULL
	//	Window menu handle. Since we don't have a menu, we set this to NULL.
	//	hInstance. This is the programs instance handle. We just pass it our hInstance from 
	//	our WinMain(). 
	//	variable or structure to the "WndProc", just pass in NULL.
	HWND hWnd = CreateWindow(className.c_str(), L"GameEngine", style, windowLeft, windowTop,
		realWidth, realHeight, NULL, NULL, hInstance, NULL);

	//	This shows our window.  We give it our handle to our window, which now has all our 
	//	windows info, and our WinMain() variable iCmdShow.
	ShowWindow(hWnd, SW_SHOWNORMAL);
	//	This pretty much paints our window to the screen.
	UpdateWindow(hWnd);

	return hWnd;								
}

/*
	Two general approaches are useful in collecting events. It can be either through polling 
	periodically or waiting to inform that its state has changed. It is necessary to take into 
	account the frequency of changes, the cost to poll and the cost to process an event. In our 
	scenario is most likely to expect a change every frame and therefore polling would make more 
	sense than processing a flood of events. The Windows API supports keyboard and mouse events 
	through the windows procedure WndProc(), any processed event will be informed to attached 
	listeners so they can respond to it.
*/
//! Process system events
void WindowsSystem::HandleSystemMessages()
{
	MSG msg;

	//	Here is our main loop.  This will continue to go until PeekMessage WM_QUIT, 
	//	which will close the program. This would happen if we closed the window. 
	//	We need to pass in the address of "msg" because GetMessage fills in the 
	//	structure "msg".  We pass in NULL for the HWnd because that makes PeekMessage 
	//	check ALL windows that use our WndProc. The rest we pass in 0, they are not important.
	while (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE))
	{

		//	The TranslateMessage() function translates virtual-key messages into character 
		//	messages. No message translation because we don't use WCHAR and it would conflict 
		//	with our deadkey handling.
		if (!mExternalWindow && msg.hwnd == (HWND)IntToPtr(mWindowID))
		{
			//	The DispatchMessage() function dispatches a message to a window procedure." 
			//	This means it handles the message, like it the message was to close the window, 
			//	it closes the window.
			DispatchMessage(&msg);
		}	
		else WndProc((HWND)IntToPtr(mWindowID), msg.message, msg.wParam, msg.lParam);
		
		if (msg.message == WM_QUIT)
			mClose = true;
	}
}


//! Remove all messages pending in the system message loop
void WindowsSystem::ClearSystemMessages()
{
	MSG msg;
	while (PeekMessage(&msg, NULL, WM_KEYFIRST, WM_KEYLAST, PM_REMOVE))
	{}
	while (PeekMessage(&msg, NULL, WM_MOUSEFIRST, WM_MOUSELAST, PM_REMOVE))
	{}
}

/*
	This is the game loop for platform. It process the events by using Windows API
	methods to look at the queue for new messages and dispatches them.
*/
//! runs the device. Returns false if device wants to be deleted
bool WindowsSystem::OnRun()
{
	Timer::Tick();

	//GetWindowsCursorControl()->Update();
	HandleSystemMessages();

	if (!mClose)
		ResizeIfNecessary();

	return !mClose;
}

//! Pause execution and let other processes to run for a specified amount of time.
void WindowsSystem::OnPause(unsigned int timeMs, bool pauseTimer)
{
	const bool wasStopped = Timer::IsStopped();
	if (pauseTimer && !wasStopped)
		Timer::StopTimer();

	Sleep(timeMs);

	if (pauseTimer && !wasStopped)
		Timer::StartTimer();
}

//! notifies the device that it should close itself
void WindowsSystem::OnClose()
{
	MSG msg;
	PeekMessage(&msg, NULL, WM_QUIT, WM_QUIT, PM_REMOVE);
	PostQuitMessage(0);
	PeekMessage(&msg, NULL, WM_QUIT, WM_QUIT, PM_REMOVE);
	if (!mExternalWindow)
	{
		DestroyWindow((HWND)IntToPtr(mWindowID));
		const eastl::wstring ClassName("WindowsSystem");
		HINSTANCE hInstance = GetModuleHandle(0);
		// We need to unregister our window class with the Windows OS.  
		// By doing this, we free up the memory allocated to register our window class.	
		UnregisterClass(ClassName.c_str(), hInstance);
	}
	mClose=true;
}

//! Process Messages
void WindowsSystem::ProcessMessage(int* HWndPtrAddress, int msg, int wParam, int lParam)
{
	WndProc( (HWND)HWndPtrAddress, msg, WPARAM(wParam), LPARAM(lParam) );
}


//! \return Returns a pointer to a list with all video modes supported
//! by the gfx adapter.
eastl::vector<Vector2<unsigned int>> WindowsSystem::GetVideoResolutions()
{
	// enumerate video modes.
	DWORD i = 0;
	DEVMODE mode;
	memset(&mode, 0, sizeof(mode));
	mode.dmSize = sizeof(mode);

	eastl::vector<Vector2<unsigned int>> resolutions;
	while (EnumDisplaySettings(NULL, i, &mode))
	{
		resolutions.push_back(Vector2<unsigned int>{ mode.dmPelsWidth, mode.dmPelsHeight });
		++i;
	}

	return resolutions;
}


//! Notifies the device, that it has been resized
void WindowsSystem::OnResized()
{
	mResized = true;
}

void WindowsSystem::ResizeIfNecessary()
{
	if (!mResized)
		return;

	RECT r;
	GetClientRect((HWND)IntToPtr(mWindowID), &r);

	char tmp[255];

	if (r.right < 2 || r.bottom < 2)
	{
		sprintf(tmp, "Ignoring resize operation to (%ld %ld)", r.right, r.bottom);
		//LogInformation(tmp);
	}
	else
	{
		sprintf(tmp, "Resizing window (%ld %ld)", r.right, r.bottom);
		//LogInformation(tmp);
	}

	mResized = false;
}

//! sets the caption of the window
void WindowsSystem::SetWindowCaption(const wchar_t* text)
{
	// We use SendMessage instead of SetText to ensure proper
	// function even in cases where the HWnd was created in a different thread
	DWORD_PTR dwResult;
	SendMessageTimeoutW((HWND)IntToPtr(mWindowID), WM_SETTEXT, 0,
			reinterpret_cast<LPARAM>(text),
			SMTO_ABORTIFHUNG, 2000, &dwResult);
}

//! returns if window is active. if not, nothing needs to be drawn
bool WindowsSystem::IsWindowActive() const
{
	return (GetActiveWindow() == (HWND)IntToPtr(mWindowID));
}

//! returns if window has focus
bool WindowsSystem::IsWindowFocused() const
{
	bool ret = (GetFocus() == (HWND)IntToPtr(mWindowID));
	return ret;
}

//! returns if window is minimized
bool WindowsSystem::IsWindowMinimized() const
{
	WINDOWPLACEMENT plc;
	plc.length=sizeof(WINDOWPLACEMENT);
	bool ret=false;
	if (GetWindowPlacement((HWND)IntToPtr(mWindowID),&plc))
		ret=(plc.showCmd & SW_SHOWMINIMIZED)!=0;
	return ret;
}

//
// IsOnlyInstance							- Chapter 5, page 137
//
bool WindowsSystem::IsOnlyInstance(const wchar_t* gameTitle)
{
	// Find the window.  If active, set and return false
	// Only one game instance may have this mutex at a time...

	HANDLE handle = CreateMutex(NULL, TRUE, gameTitle);

	// Does anyone else think 'ERROR_SUCCESS' is a bit of an oxymoron?
	if (GetLastError() != ERROR_SUCCESS)
	{
		HWND hWnd = FindWindow(gameTitle, NULL);
		if (hWnd)
		{
			// An instance of your game is already running.
			ShowWindow(hWnd, SW_SHOWNORMAL);
			SetFocus(hWnd);
			SetForegroundWindow(hWnd);
			SetActiveWindow(hWnd);
			return false;
		}
	}
	return true;
}

void* WindowsSystem::GetID() const
{
	return IntToPtr(mWindowID);
}

//! switches to fullscreen,
bool WindowsSystem::SwitchToFullScreen(bool reset)
{
	if (!mFullscreen)
		return true;

	if (reset)
	{
		//	When we close the program we need to restore the settings. To do this, we call the 
		//	ChangeDisplaySettings(), but pass in NULL and 0 for the parameters. It restores the 
		//	old settings before we changed it, which is nice that we don't need to keep track of 
		//	the old settings. The ChangeToFullScreen() function should be modular enough to plug 
		//	into any project.  It's simple to use and easy to cleanup.
		if (mChangedToFullScreen)
		{
			return (ChangeDisplaySettings(&mDesktopMode,0)==DISP_CHANGE_SUCCESSFUL);
		}
		else
			return true;
	}

	// In the ChangeToFullScreen() function, what we do is Enumerate the current display
	// settings. This means, that we fill in our DEVMODE structure with our current screen
	// settings. It holds the current screen resolution, along with other information after 
	// the function is called. But, before we pass that information into ChangeDisplaySettings(), 
	// we want to change the resolution to our desired resolution that was passed in.

	// use default values from current setting
	DEVMODE dm;		// Device Mode variable - Needed to change modes
	memset(&dm, 0, sizeof(dm)); // Makes Sure Memory's Cleared

	// This call is going to get us the current display settings -- By parameter:
	// NULL -- This specifies which display device (NULL means current display device)
	// ENUM_CURRENT_SETTINGS -- Retrieve the current settings for the display device
	// dev_mode -- Address of DEVMODE structure to fill with the display settings
	if(!EnumDisplaySettings(NULL,ENUM_CURRENT_SETTINGS,&dm))
	{
		// Display error message if we couldn't get display settings
		MessageBox(NULL, L"Could Not Enum Display Settings", L"Error", MB_OK);
		return false;
	}

	dm.dmSize = sizeof(dm);
	dm.dmPelsWidth	= mWidth;	//mWindowSize.Width;	// Set the desired Screen Width
	dm.dmPelsHeight	= mHeight;	//mWindowSize.Height;	// Set the desired Screen Height
	dm.dmBitsPerPel = mBits;
	dm.dmFields = DM_BITSPERPEL | DM_PELSWIDTH | DM_PELSHEIGHT | DM_DISPLAYFREQUENCY;
	// Set the flags saying we're changing the Screen Width and Height
	dm.dmFields = DM_PELSWIDTH | DM_PELSHEIGHT;	
	
	//	This function actually changes the screen to full screen CDS_FULLSCREEN 
	//	Gets Rid Of Start Bar. We always want to get a result from this function 
	//	to check if we failed
	LONG result = ChangeDisplaySettings(&dm,CDS_FULLSCREEN);	

	// Check if we didn't recieved a good return message From the function
	if(result != DISP_CHANGE_SUCCESSFUL)
	{
		// try again without forcing display frequency
		dm.dmFields = DM_BITSPERPEL | DM_PELSWIDTH | DM_PELSHEIGHT;
		result = ChangeDisplaySettings(&dm, CDS_FULLSCREEN);
	}

	bool ret = false;
	switch(result)
	{
		case DISP_CHANGE_SUCCESSFUL:
			mChangedToFullScreen = true;
			ret = true;
			break;
		case DISP_CHANGE_RESTART:
			LogError(	"Switch to fullscreen: The computer must be restarted "
						"in order for the graphics mode to work."	);
			break;
		case DISP_CHANGE_BADFLAGS:
			LogError(	"Switch to fullscreen: An invalid set of flags was passed in."	);
			break;
		case DISP_CHANGE_BADPARAM:
			LogError(	"Switch to fullscreen: An invalid parameter was passed in. " 
						"This can include an invalid flag or combination of flags."		);
			break;
		case DISP_CHANGE_FAILED:
			LogError(	"Switch to fullscreen: The display Renderer failed the "
						"specified graphics mode."	);
			break;
		case DISP_CHANGE_BADMODE:
			LogError(	"Switch to fullscreen: The graphics mode is not supported."	);
			break;
		default:
			LogError(	"An unknown error occured while changing to fullscreen."	);
			break;
	}
	return ret;
}

//! Sets if the window should be resizable in windowed mode.
void WindowsSystem::SetResizable(bool resize)
{
	if (mExternalWindow || mFullscreen)
		return;

	LONG_PTR style = WS_POPUP;

	if (!resize)
		style = WS_SYSMENU | WS_BORDER | WS_CAPTION | 
				WS_CLIPCHILDREN | WS_CLIPSIBLINGS;
	else
		style = WS_THICKFRAME | WS_SYSMENU | WS_CAPTION | WS_CLIPCHILDREN | 
				WS_CLIPSIBLINGS | WS_MINIMIZEBOX | WS_MAXIMIZEBOX;

	if (!SetWindowLongPtr((HWND)IntToPtr(mWindowID), GWL_STYLE, style))
		LogError(	"Could not change window style."	);

	RECT clientSize;
	clientSize.top = 0;
	clientSize.left = 0;
	clientSize.right = mWidth;
	clientSize.bottom = mHeight;

	AdjustWindowRect(&clientSize, style, FALSE);

	const signed int realWidth = clientSize.right - clientSize.left;
	const signed int realHeight = clientSize.bottom - clientSize.top;

	const signed int windowLeft = (GetSystemMetrics(SM_CXSCREEN) - realWidth) / 2;
	const signed int windowTop = (GetSystemMetrics(SM_CYSCREEN) - realHeight) / 2;

	SetWindowPos((HWND)IntToPtr(mWindowID), HWND_TOP, windowLeft, windowTop, realWidth, realHeight,
		SWP_FRAMECHANGED | SWP_NOMOVE | SWP_SHOWWINDOW);

	//GetWindowsCursorControl()->UpdateBorderSize(mFullscreen, resize);
}

//! Minimizes the window.
void WindowsSystem::OnMinimizeWindow()
{
	WINDOWPLACEMENT wndpl;
	wndpl.length = sizeof(WINDOWPLACEMENT);
	GetWindowPlacement((HWND)IntToPtr(mWindowID), &wndpl);
	wndpl.showCmd = SW_SHOWMINNOACTIVE;
	SetWindowPlacement((HWND)IntToPtr(mWindowID), &wndpl);
}

//! Maximizes the window.
void WindowsSystem::OnMaximizeWindow()
{
	WINDOWPLACEMENT wndpl;
	wndpl.length = sizeof(WINDOWPLACEMENT);
	GetWindowPlacement((HWND)IntToPtr(mWindowID), &wndpl);
	wndpl.showCmd = SW_SHOWMAXIMIZED;
	SetWindowPlacement((HWND)IntToPtr(mWindowID), &wndpl);
}

//! Restores the window to its original size.
void WindowsSystem::OnRestoreWindow()
{
	WINDOWPLACEMENT wndpl;
	wndpl.length = sizeof(WINDOWPLACEMENT);
	GetWindowPlacement((HWND)IntToPtr(mWindowID), &wndpl);
	wndpl.showCmd = SW_SHOWNORMAL;
	SetWindowPlacement((HWND)IntToPtr(mWindowID), &wndpl);
}


typedef BOOL (WINAPI *PGPI)(DWORD, DWORD, DWORD, DWORD, PDWORD);
// Needed for old windows apis
// depending on the SDK version and compilers some defines might be available
// or not
#ifndef PRODUCT_ULTIMATE
#define PRODUCT_ULTIMATE	0x00000001
#define PRODUCT_HOME_BASIC	0x00000002
#define PRODUCT_HOME_PREMIUM	0x00000003
#define PRODUCT_ENTERPRISE	0x00000004
#define PRODUCT_HOME_BASIC_N	0x00000005
#define PRODUCT_BUSINESS	0x00000006
#define PRODUCT_STARTER		0x0000000B
#endif
#ifndef PRODUCT_ULTIMATE_N
#define PRODUCT_BUSINESS_N	0x00000010
#define PRODUCT_HOME_PREMIUM_N	0x0000001A
#define PRODUCT_ENTERPRISE_N	0x0000001B
#define PRODUCT_ULTIMATE_N	0x0000001C
#endif
#ifndef PRODUCT_STARTER_N
#define PRODUCT_STARTER_N	0x0000002F
#endif
#ifndef PRODUCT_PROFESSIONAL
#define PRODUCT_PROFESSIONAL	0x00000030
#define PRODUCT_PROFESSIONAL_N	0x00000031
#endif
#ifndef PRODUCT_ULTIMATE_E
#define PRODUCT_STARTER_E	0x00000042
#define PRODUCT_HOME_BASIC_E	0x00000043
#define PRODUCT_HOME_PREMIUM_E	0x00000044
#define PRODUCT_PROFESSIONAL_E	0x00000045
#define PRODUCT_ENTERPRISE_E	0x00000046
#define PRODUCT_ULTIMATE_E	0x00000047
#endif

void WindowsSystem::GetSystemVersion(eastl::string& out)
{
	OSVERSIONINFOEX osvi;
	PGPI pGPI;
	BOOL bOsVersionInfoEx;

	ZeroMemory(&osvi, sizeof(OSVERSIONINFOEX));
	osvi.dwOSVersionInfoSize = sizeof(OSVERSIONINFOEX);

	bOsVersionInfoEx = GetVersionEx((OSVERSIONINFO*) &osvi);
	if (!bOsVersionInfoEx)
	{
		osvi.dwOSVersionInfoSize = sizeof(OSVERSIONINFO);
		if (! GetVersionEx((OSVERSIONINFO *) &osvi))
			return;
	}

	eastl::string osviszCSDVersion = ToString(osvi.szCSDVersion);

	switch (osvi.dwPlatformId)
	{
	case VER_PLATFORM_WIN32_NT:
		if (osvi.dwMajorVersion <= 4)
			out.append("Microsoft Windows NT ");
		else
		if (osvi.dwMajorVersion == 5 && osvi.dwMinorVersion == 0)
			out.append("Microsoft Windows 2000 ");
		else
		if (osvi.dwMajorVersion == 5 && osvi.dwMinorVersion == 1)
			out.append("Microsoft Windows XP ");
		else
		if (osvi.dwMajorVersion == 6 )
		{
			if (osvi.dwMinorVersion == 0)
			{
				if (osvi.wProductType == VER_NT_WORKSTATION)
					out.append("Microsoft Windows Vista ");
				else
					out.append("Microsoft Windows Server 2008 ");
			}
			else if (osvi.dwMinorVersion == 1)
			{
				if (osvi.wProductType == VER_NT_WORKSTATION)
					out.append("Microsoft Windows 7 ");
				else
					out.append("Microsoft Windows Server 2008 R2 ");
			}
		}

		if (bOsVersionInfoEx)
		{
			if (osvi.dwMajorVersion == 6)
			{
				DWORD dwType;
				pGPI = (PGPI)GetProcAddress(GetModuleHandle(TEXT("kernel32.dll")), "GetProductInfo");
				pGPI(osvi.dwMajorVersion, osvi.dwMinorVersion, 0, 0, &dwType);

				switch (dwType)
				{
				case PRODUCT_ULTIMATE:
				case PRODUCT_ULTIMATE_E:
				case PRODUCT_ULTIMATE_N:
					out.append("Ultimate Edition ");
					break;
				case PRODUCT_PROFESSIONAL:
				case PRODUCT_PROFESSIONAL_E:
				case PRODUCT_PROFESSIONAL_N:
					out.append("Professional Edition ");
					break;
				case PRODUCT_HOME_BASIC:
				case PRODUCT_HOME_BASIC_E:
				case PRODUCT_HOME_BASIC_N:
					out.append("Home Basic Edition ");
					break;
				case PRODUCT_HOME_PREMIUM:
				case PRODUCT_HOME_PREMIUM_E:
				case PRODUCT_HOME_PREMIUM_N:
					out.append("Home Premium Edition ");
					break;
				case PRODUCT_ENTERPRISE:
				case PRODUCT_ENTERPRISE_E:
				case PRODUCT_ENTERPRISE_N:
					out.append("Enterprise Edition ");
					break;
				case PRODUCT_BUSINESS:
				case PRODUCT_BUSINESS_N:
					out.append("Business Edition ");
					break;
				case PRODUCT_STARTER:
				case PRODUCT_STARTER_E:
				case PRODUCT_STARTER_N:
					out.append("Starter Edition ");
					break;
				}
			}
#ifdef VER_SUITE_ENTERPRISE
			else
			if (osvi.wProductType == VER_NT_WORKSTATION)
			{
#ifndef __BORLANDC__
				if( osvi.wSuiteMask & VER_SUITE_PERSONAL )
					out.append("Personal ");
				else
					out.append("Professional ");
#endif
			}
			else if (osvi.wProductType == VER_NT_SERVER)
			{
				if( osvi.wSuiteMask & VER_SUITE_DATACENTER )
					out.append("DataCenter Server ");
				else if( osvi.wSuiteMask & VER_SUITE_ENTERPRISE )
					out.append("Advanced Server ");
				else
					out.append("Server ");
			}
#endif
		}
		else
		{
			HKEY hKey;
			char szProductType[80];
			DWORD dwBufLen;

			RegOpenKeyEx( HKEY_LOCAL_MACHINE,
					__TEXT("SYSTEM\\CurrentControlSet\\Control\\ProductOptions"),
					0, KEY_QUERY_VALUE, &hKey );
			RegQueryValueEx( hKey, __TEXT("ProductType"), NULL, NULL,
					(LPBYTE) szProductType, &dwBufLen);
			RegCloseKey( hKey );

			if (_strcmpi( "WINNT", szProductType) == 0 )
				out.append("Professional ");
			if (_strcmpi( "LANMANNT", szProductType) == 0)
				out.append("Server ");
			if (_strcmpi( "SERVERNT", szProductType) == 0)
				out.append("Advanced Server ");
		}

		// Display version, service pack (if any), and build number.

		char tmp[255];

		if (osvi.dwMajorVersion <= 4)
		{
			sprintf(tmp, "version %ld.%ld %s (Build %ld)",
				osvi.dwMajorVersion,
				osvi.dwMinorVersion,
				osviszCSDVersion.c_str(),
				osvi.dwBuildNumber & 0xFFFF);
		}
		else
		{
			sprintf(tmp, "%s (Build %ld)", osviszCSDVersion.c_str(),
				osvi.dwBuildNumber & 0xFFFF);
		}

		out.append(tmp);
		break;

	case VER_PLATFORM_WIN32_WINDOWS:

		if (osvi.dwMajorVersion == 4 && osvi.dwMinorVersion == 0)
		{
			out.append("Microsoft Windows 95 ");
			if ( osvi.szCSDVersion[1] == 'C' || osvi.szCSDVersion[1] == 'B' )
				out.append("OSR2 " );
		}

		if (osvi.dwMajorVersion == 4 && osvi.dwMinorVersion == 10)
		{
			out.append("Microsoft Windows 98 ");
			if ( osvi.szCSDVersion[1] == 'A' )
				out.append( "SE " );
		}

		if (osvi.dwMajorVersion == 4 && osvi.dwMinorVersion == 90)
			out.append("Microsoft Windows Me ");

		break;

	case VER_PLATFORM_WIN32s:
		out.append("Microsoft Windows ");
		break;
	}
}

//----------------------------------------------------------------------------
// WindowsSystem::CursorControl
//----------------------------------------------------------------------------
WindowsSystem::CursorControl::CursorControl(
	const Vector2<unsigned int>& wsize, HWND hwnd, bool fullscreen)
	: mWindowSize(wsize), mHWnd(hwnd), mBorderX(0), mBorderY(0), mVisible(true), 
		mUseReferenceRect(false), mActiveIcon(CI_NORMAL), mActiveIconStartTime(0)
{
	mInvWindowSize[0] = 0.f;
	if (mWindowSize[0] != 0)
		mInvWindowSize[0] = 1.0f / mWindowSize[0];

	mInvWindowSize[1] = 0.f;
	if (mWindowSize[1] != 0)
		mInvWindowSize[1] = 1.0f / mWindowSize[1];

	UpdateBorderSize(fullscreen, false);
	InitCursors();
}

WindowsSystem::CursorControl::~CursorControl()
{
	for (int i = 0; i < (int)mCursors.size(); ++i)
		for (int f = 0; f < (int)mCursors[i].mFrames.size(); ++f)
			DestroyCursor(mCursors[i].mFrames[f].mIconHW);
}

void WindowsSystem::CursorControl::CursorControl::InitCursors()
{
	mCursors.push_back(Cursor(LoadCursor(NULL, IDC_ARROW)));
	mCursors.push_back(Cursor(LoadCursor(NULL, IDC_CROSS)));
	mCursors.push_back(Cursor(LoadCursor(NULL, IDC_HAND)));
	mCursors.push_back(Cursor(LoadCursor(NULL, IDC_HELP)));
	mCursors.push_back(Cursor(LoadCursor(NULL, IDC_IBEAM)));
	mCursors.push_back(Cursor(LoadCursor(NULL, IDC_NO)));
	mCursors.push_back(Cursor(LoadCursor(NULL, IDC_WAIT)));
	mCursors.push_back(Cursor(LoadCursor(NULL, IDC_SIZEALL)));
	mCursors.push_back(Cursor(LoadCursor(NULL, IDC_SIZENESW)));
	mCursors.push_back(Cursor(LoadCursor(NULL, IDC_SIZENWSE)));
	mCursors.push_back(Cursor(LoadCursor(NULL, IDC_SIZENS)));
	mCursors.push_back(Cursor(LoadCursor(NULL, IDC_SIZEWE)));
	mCursors.push_back(Cursor(LoadCursor(NULL, IDC_UPARROW)));
}

//! Changes the visible state of the mouse cursor.
void WindowsSystem::CursorControl::SetVisible(bool visible)
{
	CURSORINFO info;
	info.cbSize = sizeof(CURSORINFO);
	BOOL gotCursorInfo = GetCursorInfo(&info);
	while (gotCursorInfo)
	{
#ifdef CURSOR_SUPPRESSED
		// new flag for Windows 8, where cursor
		// might be suppressed for touch interface
		if (info.flags == CURSOR_SUPPRESSED)
		{
			visible = false;
			break;
		}
#endif
		if ((visible && info.flags == CURSOR_SHOWING) || // visible
			(!visible && info.flags == 0)) // hidden
		{
			break;
		}
		// this only increases an internal
		// display counter in windows, so it
		// might have to be called some more
		const int showResult = ShowCursor(visible);
		// if result has correct sign we can
		// stop here as well
		if ((!visible && showResult < 0) ||
			(visible && showResult >= 0))
			break;
		// yes, it really must be set each time
		info.cbSize = sizeof(CURSORINFO);
		gotCursorInfo = GetCursorInfo(&info);
	}
	mVisible = visible;
}

//! Returns if the cursor is currently visible.
bool WindowsSystem::CursorControl::IsVisible() const
{
	return mVisible;
}

//! Sets the new position of the cursor.
void WindowsSystem::CursorControl::SetPosition(const Vector2<float> &pos)
{
	SetPosition(pos[0], pos[1]);
}

//! Sets the new position of the cursor.
void WindowsSystem::CursorControl::SetPosition(const Vector2<int> &pos)
{
	SetPosition(pos[0], pos[1]);
}

//! Sets the new position of the cursor.
void WindowsSystem::CursorControl::SetPosition(float x, float y)
{
	if (!mUseReferenceRect)
		SetPosition((int)round(x*mWindowSize[0]), (int)round(y*mWindowSize[1]));
	else
		SetPosition((int)round(x*mReferenceRect.mExtent[0]), (int)round(y*mReferenceRect.mExtent[1]));
}

//! Sets the new position of the cursor.
void WindowsSystem::CursorControl::SetPosition(int x, int y)
{
	if (mUseReferenceRect)
	{
		SetCursorPos(
			mReferenceRect.mCenter[0] - (mReferenceRect.mExtent[0] / 2) + x, 
			mReferenceRect.mCenter[1] - (mReferenceRect.mExtent[1] / 2) + y);
	}
	else
	{
		RECT rect;
		if (GetWindowRect(mHWnd, &rect))
			SetCursorPos(x + rect.left + mBorderX, y + rect.top + mBorderY);
	}

	UpdateInternalCursorPosition();
}

//! Returns the current position of the mouse cursor.
const Vector2<unsigned int>& WindowsSystem::CursorControl::GetPosition()
{
	UpdateInternalCursorPosition();
	return mCursorPos;
}

//! Returns the current position of the mouse cursor.
Vector2<float> WindowsSystem::CursorControl::GetRelativePosition()
{
	UpdateInternalCursorPosition();

	if (!mUseReferenceRect)
	{
		return Vector2<float>{
			mCursorPos[0] * mInvWindowSize[0],
			mCursorPos[1] * mInvWindowSize[1]};
	}
	else
	{
		return Vector2<float>{
			mCursorPos[0] * mInvWindowSize[0],
			mCursorPos[1] * mInvWindowSize[1]};
	}

}

//! Sets an absolute reference rect for calculating the cursor position.
void WindowsSystem::CursorControl::SetReferenceRect(const RectangleShape<2, int>* rect)
{
	if (rect)
	{
		mReferenceRect = *rect;
		mUseReferenceRect = true;

		// prevent division through zero and uneven sizes
		if (!mReferenceRect.mExtent[1] || mReferenceRect.mExtent[1] % 2)
			mReferenceRect.mCenter[1] += 1;

		if (!mReferenceRect.mExtent[0] || mReferenceRect.mExtent[0] % 2)
			mReferenceRect.mCenter[0] += 1;
	}
	else mUseReferenceRect = false;
}

//! Used to notify the cursor that the window was resized.
void WindowsSystem::CursorControl::OnResize(const Vector2<unsigned int>& size)
{
	mWindowSize = size;
	if (size[0] != 0)
		mInvWindowSize[0] = 1.0f / size[0];
	else
		mInvWindowSize[0] = 0.f;

	if (size[1] != 0)
		mInvWindowSize[1] = 1.0f / size[1];
	else
		mInvWindowSize[1] = 0.f;
}

//! Used to notify the cursor that the window resizable settings changed.
void WindowsSystem::CursorControl::UpdateBorderSize(bool fullscreen, bool resizable)
{
	if (!fullscreen)
	{
		if (resizable)
		{
			mBorderX = GetSystemMetrics(SM_CXSIZEFRAME);
			mBorderY = GetSystemMetrics(SM_CYCAPTION) + GetSystemMetrics(SM_CYSIZEFRAME);
		}
		else
		{
			mBorderX = GetSystemMetrics(SM_CXDLGFRAME);
			mBorderY = GetSystemMetrics(SM_CYCAPTION) + GetSystemMetrics(SM_CYDLGFRAME);
		}
	}
	else
	{
		mBorderX = mBorderY = 0;
	}
}

//! Updates the internal cursor position
void WindowsSystem::CursorControl::UpdateInternalCursorPosition()
{
	POINT p;
	if (!GetCursorPos(&p))
	{
		DWORD xy = GetMessagePos();
		p.x = GET_X_LPARAM(xy);
		p.y = GET_Y_LPARAM(xy);
	}

	if (mUseReferenceRect)
	{
		mCursorPos[0] = p.x - mReferenceRect.mCenter[0] - (mReferenceRect.mExtent[0] / 2);
		mCursorPos[0] = p.y - mReferenceRect.mCenter[1] - (mReferenceRect.mExtent[1] / 2);
	}
	else
	{
		RECT rect;
		if (GetWindowRect(mHWnd, &rect))
		{
			mCursorPos[0] = p.x - rect.left - mBorderX;
			mCursorPos[1] = p.y - rect.top - mBorderY;
		}
		else
		{
			// window seems not to be existent, so set cursor to
			// a negative value
			mCursorPos[0] = -1;
			mCursorPos[1] = -1;
		}
	}
}

// Convert an texture to a Windows cursor
// Based on http://www.codeguru.com/cpp/w-p/win32/cursors/article.php/c4529/
HCURSOR WindowsSystem::CursorControl::TextureToCursor(HWND hwnd, Texture2 * tex, 
	const RectangleShape<2, int>& sourceRect, const Vector2<unsigned int> &hotspot)
{
	//
	// create the bitmaps needed for cursors from the texture
	HDC dc = GetDC(hwnd);
	HDC andDc = CreateCompatibleDC(dc);
	HDC xorDc = CreateCompatibleDC(dc);
	HBITMAP andBitmap = CreateCompatibleBitmap(dc, sourceRect.mExtent[0], sourceRect.mExtent[1]);
	HBITMAP xorBitmap = CreateCompatibleBitmap(dc, sourceRect.mExtent[0], sourceRect.mExtent[1]);

	HBITMAP oldAndBitmap = (HBITMAP)SelectObject(andDc, andBitmap);
	HBITMAP oldXorBitmap = (HBITMAP)SelectObject(xorDc, xorBitmap);

	DFType format = tex->GetFormat();
	unsigned int bytesPerPixel = DataFormat::GetNumBytesPerStruct(format);
	const char* data = (const char*)tex->GetData();
	for (int y = 0; y < sourceRect.mExtent[1]; ++y)
	{
		for (int x = 0; x < sourceRect.mExtent[0]; ++x)
		{
			unsigned int color = *(unsigned int*)data;
			data += bytesPerPixel;

			if (color >> 24 == 0)	// transparent
			{
				SetPixel(andDc, x, y, RGB(255, 255, 255));
				SetPixel(xorDc, x, y, RGB(0, 0, 0));
			}
			else	// color
			{
				SetPixel(andDc, x, y, RGB(0, 0, 0));
				SetPixel(xorDc, x, y, RGB((color >> 16) & 0xff, (color >> 8) & 0xff, color & 0xff));
			}
		}
	}

	SelectObject(andDc, oldAndBitmap);
	SelectObject(xorDc, oldXorBitmap);

	DeleteDC(xorDc);
	DeleteDC(andDc);

	ReleaseDC(hwnd, dc);

	// create the cursor

	ICONINFO iconinfo;
	iconinfo.fIcon = false;	// type is cursor not icon
	iconinfo.xHotspot = hotspot[0];
	iconinfo.yHotspot = hotspot[1];
	iconinfo.hbmMask = andBitmap;
	iconinfo.hbmColor = xorBitmap;

	HCURSOR cursor = CreateIconIndirect(&iconinfo);

	DeleteObject(andBitmap);
	DeleteObject(xorBitmap);

	return cursor;
}

void WindowsSystem::CursorControl::Update()
{
	if (!mCursors[mActiveIcon].mFrames.empty() && mCursors[mActiveIcon].mFrameTime)
	{
		// update animated cursors. This could also be done by X11 in case someone 
		// wants to figure that out (this way was just easier to implement)
		unsigned int now = Timer::GetRealTime();
		unsigned int frame = 
			((now - mActiveIconStartTime) / mCursors[mActiveIcon].mFrameTime) % 
			mCursors[mActiveIcon].mFrames.size();
		SetCursor(mCursors[mActiveIcon].mFrames[frame].mIconHW);
	}
}

//! Sets the active cursor icon
void WindowsSystem::CursorControl::SetActiveIcon(CursorIcon iconId)
{
	if (iconId >= (int)mCursors.size())
		return;

	mActiveIcon = iconId;
	mActiveIconStartTime = Timer::GetRealTime();
	if (mCursors[mActiveIcon].mFrames.size())
		SetCursor(mCursors[mActiveIcon].mFrames[0].mIconHW);
}

//! Gets the currently active icon
CursorIcon WindowsSystem::CursorControl::GetActiveIcon() const
{
	return mActiveIcon;
}

//! Add a custom sprite as cursor icon.
CursorIcon WindowsSystem::CursorControl::AddIcon(const CursorSprite& icon)
{
	return CI_NORMAL;
}

//! replace the given cursor icon.
void WindowsSystem::CursorControl::ChangeIcon(CursorIcon iconId, const CursorSprite& icon)
{
}

//! Return a system-specific size which is supported for cursors. Larger icons will fail, smaller icons might work.
Vector2<int> WindowsSystem::CursorControl::GetSupportedIconSize() const
{
	Vector2<int> result;

	result[0] = GetSystemMetrics(SM_CXCURSOR);
	result[1] = GetSystemMetrics(SM_CYCURSOR);

	return result;
}