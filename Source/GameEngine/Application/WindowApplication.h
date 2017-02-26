// Geometric Tools, LLC
// Copyright (c) 1998-2014
// Distributed under the Boost Software License, Version 1.0.
// http://www.boost.org/LICENSE_1_0.txt
// http://www.geometrictools.com/License/Boost/LICENSE_1_0.txt
//
// File Version: 5.0.2 (2010/05/06)

#ifndef WINDOWAPPLICATION_H
#define WINDOWAPPLICATION_H

#include "Application.h"

#include "System/System.h"
#include "Graphic/Renderer/Renderer.h"

//	Window abstracts the platform-dependent implementations
class WindowApplication : public Application, public EventListener
{
protected:
    // Abstract base class.
    WindowApplication (const char* windowTitle, int xPosition,
        int yPosition, int width, int height, const eastl::array<float, 4>& clearColor);
public:
    virtual ~WindowApplication ();
	
	inline float GetAspectRatio() const;

	// Main loop processing
	virtual bool OnInitialize();
	virtual void OnTerminate();
	virtual void OnRun();

	virtual void OnPreidle();
	virtual void OnIdle();

	void AbortGame() { mQuitting = true; }
	bool IsRunning() { return mIsRunning; }
	void SetQuitting(bool quitting) { mQuitting = quitting; }
	
	// Event callbacks.
	virtual bool OnEvent(const Event& ev);

protected:

	//Time
	void InitTime();
	void UpdateTime();
	void UpdateFrameCount();
	float GetLimitedDt();

	double mLastTime, mAccumulatedTime, mFrameRate;
	int mFrameCount, mFramesPerSecond, mTimer, mMaxTimer;

    // Window parameters (from the constructor).
    eastl::wstring mTitle;
    int mXOrigin, mYOrigin, mWidth, mHeight;

	eastl::array<float, 4> mClearColor;
    bool mAllowResize;

	//HINSTANCE m_hInstance;	//	the module instance
	bool mWindowedMode;			//	true if the app is windowed, false if fullscreen
	bool mIsRunning;			//	true if everything is initialized and the game is in the main loop
	bool mQuitRequested;		//	true if the app should run the exit sequence
	bool mQuitting;				//	true if the app is running the exit sequence

    // The window ID is platform-specific but hidden by an 'int' opaque
    // handle.
    int mWindowID;
	
	//main services
	eastl::shared_ptr<ProgramFactory> mProgramFactory;
	eastl::shared_ptr<Renderer> mRenderer;
	eastl::shared_ptr<System> mSystem;
};

inline float WindowApplication::GetAspectRatio() const
{
	return (float)mWidth / (float)mHeight;
}

#endif
