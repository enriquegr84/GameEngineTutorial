//========================================================================
// File: DemoCameraController.h
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

#ifndef GAMEDEMOCAMERACONTROLLER_H
#define GAMEDEMOCAMERACONTROLLER_H

#include "GameEngineStd.h"

#include "Application/System/EventSystem.h"

#include "Mathematic/Algebra/Transform.h"

class CameraNode;

class GameDemoCameraController : public BaseMouseHandler, public BaseKeyboardHandler
{
protected:
	bool mEnabled;
	BYTE mKey[256];			// Which keys are up and down

	Transform		mAbsoluteTransform;
	Vector2<int>	mLastMousePos;

	// Orientation Controls
	float		mYaw;
	float		mPitch;
	float		mPitchOnDown;
	float		mYawOnDown;

	// Speed Controls
	float		mMaxMoveSpeed;
	float		mMaxRotateSpeed;
	float		mMoveSpeed;
	float		mRotateSpeed;

	// Added for Ch19/20 refactor
	bool		mWheelRollDown;
	bool		mMouseLButtonDown;
	bool		mRotateWhenLButtonDown;

	eastl::shared_ptr<CameraNode> mCamera;

public:
	GameDemoCameraController(const eastl::shared_ptr<CameraNode>& camera,
		float initialYaw, float initialPitch, bool rotateWhenLButtonDown);

	//! Sets whether or not the controller is currently enabled.
	void SetEnabled(bool enabled) { mEnabled = enabled; }

	//! Gets whether or not the affector is currently enabled.
	bool GetEnabled() const { return mEnabled; }

	void OnUpdate(unsigned int timeMs, unsigned long deltaMs);

	const eastl::shared_ptr<CameraNode>& GetCamera() { return mCamera; }
	void SetCamera(const eastl::shared_ptr<CameraNode>& newCamera) { mCamera = newCamera; }

	bool OnMouseMove(const Vector2<int> &mousePos, const int radius);
	bool OnMouseButtonDown(const Vector2<int> &mousePos, const int radius, const eastl::string &buttonName);
	bool OnMouseButtonUp(const Vector2<int> &mousePos, const int radius, const eastl::string &buttonName);

	bool OnKeyDown(const KeyCode c) { mKey[c] = true; return true; }
	bool OnKeyUp(const KeyCode c) { mKey[c] = false; return true; }

	bool OnWheelRollDown() { mWheelRollDown = true; return true; }
	bool OnWheelRollUp() { mWheelRollDown = false; return true; }

};

#endif