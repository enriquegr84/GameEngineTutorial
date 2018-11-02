// Geometric Tools, LLC
// Copyright (c) 1998-2014
// Distributed under the Boost Software License, Version 1.0.
// http://www.boost.org/LICENSE_1_0.txt
// http://www.geometrictools.com/License/Boost/LICENSE_1_0.txt
//
// File Version: 5.0.2 (2010/05/06)

#ifndef DEMOAPPLICATION_H
#define DEMOAPPLICATION_H

#include "Application/WindowApplication.h"

class TriangleDemoApp : public WindowApplication
{

public:
    // Abstract base class.
    TriangleDemoApp();
    virtual ~TriangleDemoApp();

	// Event callbacks.
    virtual bool OnInitialize();
    virtual void OnTerminate();
	virtual void OnRun();

	virtual bool OnEvent(const Event& ev);

	virtual void OnIdle() override;

protected:

	// Create the camera and camera rig.
	void InitializeCamera(float upFovDegrees, float aspectRatio, float dmin, float dmax,
		float translationSpeed, float rotationSpeed, eastl::array<float, 3> const& pos,
		eastl::array<float, 3> const& dir, eastl::array<float, 3> const& up);

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

	// The appropriate camera rig motion is selected when 'key' is mapped
	// to a camera motion.
	bool OnKeyDown(KeyCode key);
	bool OnKeyUp(KeyCode key);

	// Control the rotation of the trackball.
	bool OnMouseClick(MouseInputEvent button, unsigned int state, int x, int y);
	bool OnMouseMotion(MouseInputEvent button, unsigned int state, int x, int y);

	eastl::shared_ptr<Visual> mTriangle;
	eastl::shared_ptr<Camera> mCamera;
};

#endif