// Geometric Tools, LLC
// Copyright (c) 1998-2014
// Distributed under the Boost Software License, Version 1.0.
// http://www.boost.org/LICENSE_1_0.txt
// http://www.geometrictools.com/License/Boost/LICENSE_1_0.txt
//
// File Version: 5.0.2 (2010/05/06)

#ifndef DEMOAPPLICATION_H
#define DEMOAPPLICATION_H

#include "Game/View/HumanView.h"

#include "Application/WindowApplication.h"

class AreaLightsApp : public WindowApplication
{

public:
    // Abstract base class.
    AreaLightsApp();
    virtual ~AreaLightsApp();

	// Event callbacks.
    virtual bool OnInitialize();
    virtual void OnTerminate();
	virtual void OnRun();

	virtual bool OnEvent(const Event& ev);

	virtual void OnIdle() override;

	HumanView* GetHumanView();	// it was convenient to grab the HumanView attached to the application.

	virtual void AddView(const eastl::shared_ptr<BaseGameView>& pView);
	virtual void RemoveView(const eastl::shared_ptr<BaseGameView>& pView);
	virtual void RemoveViews();
	virtual void RemoveView();

protected:

	bool CreateScene();

	// The camera frustum is modified.  Any subscribers to the pvw-matrix
	// update system of the camera rig must be updated.  No call is made to
	// OnDisplay() or OnIdle().  The base class is unaware of which display
	// method you use, so to have a visual update you must override OnResize
	//    bool MyApplication::OnResize(int xSize, int ySize)
	//    {
	//        if (Window3::OnResize(xSize, ySize))
	//        {
	//            OnIdle();  // or OnDisplay() or MyOwnDrawFunction() ...
	//        }
	//    }
	bool OnResize(int width, int height);

	void OnUpdateView(unsigned int timeMs, unsigned int elapsedTime);
	void OnRender(unsigned int elapsedTime);

	// views that are attached to our game
	GameViewList mGameViews;
};

#endif