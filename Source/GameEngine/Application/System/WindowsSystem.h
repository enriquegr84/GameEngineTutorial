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
#include "Graphic/Resource/Texture/Texture2.h"

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

	//! \return Returns a pointer to a list with all video resolutions
	//! supported by the gfx adapter.
	virtual eastl::vector<Vector2<unsigned int>> GetVideoResolutions() override;

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
	class CursorControl : public BaseCursorControl
	{
	public:

		CursorControl(const Vector2<unsigned int>& wsize, HWND hWnd, bool fullscreen);
		~CursorControl();

		//! Changes the visible state of the mouse cursor.
		virtual void SetVisible(bool visible);

		//! Returns if the cursor is currently visible.
		virtual bool IsVisible() const;

		//! Sets the new position of the cursor.
		virtual void SetPosition(const Vector2<float> &pos);

		//! Sets the new position of the cursor.
		virtual void SetPosition(const Vector2<int> &pos);

		//! Sets the new position of the cursor.
		virtual void SetPosition(float x, float y);

		//! Sets the new position of the cursor.
		virtual void SetPosition(int x, int y);

		//! Returns the current position of the mouse cursor.
		virtual const Vector2<unsigned int>& GetPosition();

		//! Returns the current position of the mouse cursor.
		virtual Vector2<float> GetRelativePosition();

		//! Sets an absolute reference rect for calculating the cursor position.
		virtual void SetReferenceRect(const RectangleShape<2, int>* rect = 0);

		//! Used to notify the cursor that the window was resized.
		virtual void OnResize(const Vector2<unsigned int>& size);

		//! Used to notify the cursor that the window resizable settings changed.
		virtual void UpdateBorderSize(bool fullscreen, bool resizable);

		//! Sets the active cursor icon
		virtual void SetActiveIcon(CursorIcon iconId);

		//! Gets the currently active icon
		virtual CursorIcon GetActiveIcon() const;

		//! Add a custom sprite as cursor icon.
		virtual CursorIcon AddIcon(const CursorSprite& icon);

		//! replace the given cursor icon.
		virtual void ChangeIcon(CursorIcon iconId, const CursorSprite& sprite);

		//! Return a system-specific size which is supported for cursors. Larger icons will fail, smaller icons might work.
		virtual Vector2<int> GetSupportedIconSize() const;

		void Update();

	private:

		// convert an Irrlicht texture to a windows cursor
		HCURSOR TextureToCursor(HWND hwnd, Texture2 * tex, const RectangleShape<2, int>& sourceRect, const Vector2<unsigned int> &hotspot);

		//! Updates the internal cursor position
		void UpdateInternalCursorPosition();

		Vector2<unsigned int> mCursorPos;
		Vector2<unsigned int> mWindowSize;
		Vector2<float> mInvWindowSize;
		HWND mHWnd;

		signed int mBorderX, mBorderY;
		RectangleShape<2, int> mReferenceRect;
		bool mUseReferenceRect;
		bool mVisible;

		struct CursorFrame
		{
			CursorFrame() : mIconHW(0) {}
			CursorFrame(HCURSOR icon) : mIconHW(icon) {}

			HCURSOR mIconHW;	// hardware cursor
		};

		struct Cursor
		{
			Cursor() {}
			explicit Cursor(HCURSOR iconHw, unsigned int frameTime = 0) : mFrameTime(frameTime)
			{
				mFrames.push_back(CursorFrame(iconHw));
			}
			eastl::vector<CursorFrame> mFrames;
			unsigned int mFrameTime;
		};

		eastl::vector<Cursor> mCursors;
		CursorIcon mActiveIcon;
		unsigned int mActiveIconStartTime;

		void InitCursors();
	};
};

#endif

