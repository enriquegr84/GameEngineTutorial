// Geometric Tools, LLC
// Copyright (c) 1998-2014
// Distributed under the Boost Software License, Version 1.0.
// http://www.boost.org/LICENSE_1_0.txt
// http://www.geometrictools.com/License/Boost/LICENSE_1_0.txt
//
// File Version: 5.0.2 (2011/08/13)

#ifndef WINDOWSSYSTEM_H
#define WINDOWSSYSTEM_H

#include "Core/CoreStd.h"
#include "Mathematic/Algebra/Vector2.h"

#include "System.h"

/*
	Class WindowsSystem, platform-dependent implementation of Windows
*/
class WindowsSystem : public System
{
public:

	WindowsSystem(int width, int height);
	~WindowsSystem();

	virtual bool OnRun() override;
	virtual void OnPause(unsigned int timeMs, bool pauseTimer=false) override;
	virtual void OnClose() override;

	virtual void ProcessMessage(int* HWndPtrAddress, int msg, int wParam, int lParam) override;
	virtual void ClearSystemMessages() override;
	virtual void HandleSystemMessages() override;

	virtual void OnResized() override;
	virtual void ResizeIfNecessary() override;

	virtual void OnMinimizeWindow() override;
	virtual void OnMaximizeWindow() override;
	virtual void OnRestoreWindow() override;
	virtual void SetWindowCaption(const wchar_t* text) override;
	virtual void SetResizable(bool resize=false) override;
	virtual bool IsWindowActive() const override;
	virtual bool IsWindowFocused() const override;
	virtual bool IsWindowMinimized() const override;
	virtual bool SwitchToFullScreen(bool reset = false) override;

	virtual bool IsOnlyInstance(const wchar_t* gameTitle) override;
	virtual void* GetID() const override;
	virtual void GetSystemVersion(eastl::string& out) override;

protected:

	//	The window ID is platform-specific but hidden by an 'int' opaque handle.
	int mWindowID;
	DEVMODE mDesktopMode;

	static HWND GetHandleFromSystem(System* system);
	static System* GetSystemFromHandle(HWND hWnd);

	static eastl::map<HWND, System*> mHandleSystems;

	ATOM MyRegisterClass(HINSTANCE hInstance);
	static LRESULT CALLBACK WndProc(HWND, UINT, WPARAM, LPARAM);
	HWND InitInstance(HINSTANCE hInstance, int width, int height, bool fullscreen);

	//! Implementation of the Windows cursor control
	class CursorControl
	{
	public:

		//! Default icons for cursors
		enum CursorIcon
		{
			// Following cursors might be system specific, or might use an GameEngine icon-set. No guarantees so far.
			CI_NORMAL,	// arrow
			CI_CROSS,	// Crosshair
			CI_HAND,	// Hand
			CI_HELP,	// Arrow and question mark
			CI_IBEAM,	// typical text-selection cursor
			CI_NO,		// should not click icon
			CI_WAIT,	// hourclass
			CI_SIZEALL,	// arrow in all directions
			CI_SIZENESW,	// resizes in direction north-east or south-west
			CI_SIZENWSE,	// resizes in direction north-west or south-east
			CI_SIZENS,	// resizes in direction north or south
			CI_SIZEWE,	// resizes in direction west or east
			CI_UP,		// up-arrow

						// Implementer note: Should we add system specific cursors, which use guaranteed the system icons,
						// then I would recommend using a naming scheme like CI_W32_CROSS, CI_X11_CROSSHAIR and adding those
						// additionally.

						CI_COUNT		// maximal of defined cursors. Note that higher values can be created at runtime
		};

		CursorControl(const Vector2<unsigned int>& wsize, HWND hWnd, bool fullscreen);
		CursorControl();
		~CursorControl();

		//! Changes the visible state of the mouse cursor.
		void SetVisible(bool visible);

		//! Returns if the cursor is currently visible.
		bool IsVisible() const;

		//! Sets the new position of the cursor.
		void SetPosition(const Vector2<float> &pos);

		//! Sets the new position of the cursor.
		void SetPosition(const Vector2<unsigned int> &pos);

		//! Sets the new position of the cursor.
		void SetPosition(float x, float y);

		//! Sets the new position of the cursor.
		void SetPosition(signed int x, signed int y);

		//! Returns the current position of the mouse cursor.
		const Vector2<unsigned int>& GetPosition();

		//! Returns the current position of the mouse cursor.
		Vector2<float> GetRelativePosition();

		//! Used to notify the cursor that the window was resized.
		void OnResize(const Vector2<unsigned int>& size);

		//! Used to notify the cursor that the window resizable settings changed.
		void UpdateBorderSize(bool fullscreen, bool resizable);

	private:
		//! Updates the internal cursor position
		void UpdateInternalCursorPosition();

		Vector2<unsigned int> mCursorPos;
		Vector2<unsigned int> mWindowSize;
		Vector2<float> mInvWindowSize;
		HWND mHWnd;

		signed int mBorderX, mBorderY;
		bool mVisible;
	};
	CursorControl mCursorControl;

};

#endif

