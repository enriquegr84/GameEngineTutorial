// DemoCameraController.cpp - Controller class for the player 
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

#include "GameDemoStd.h"

#include "Core/Logger/Logger.h"

#include "Core/Event/EventManager.h"

#include "Physic/PhysicEventListener.h"

#include "GameDemoCameraController.h"
#include "GameDemoEvents.h"
#include "GameDemoApp.h"

////////////////////////////////////////////////////
// GameDemoCameraController Implementation
////////////////////////////////////////////////////


GameDemoCameraController::GameDemoCameraController(const eastl::shared_ptr<CameraNode>& camera,
	float initialYaw, float initialPitch, bool rotateWhenLButtonDown)
	: mCamera(camera)
{
	mYaw = (float)GE_C_RAD_TO_DEG * initialYaw;
	mPitch = (float)GE_C_RAD_TO_DEG * -initialPitch;

	mMaxMoveSpeed = 25.0f;
	mMaxRotateSpeed = 120.0f;
	mMoveSpeed = 0.0f;
	mRotateSpeed = 0.0f;

	//Point cursor;
	System* system = System::Get();
	system->GetCursorControl()->SetPosition(0.5f, 0.5f);
	Vector2<unsigned int> cursorPosition = system->GetCursorControl()->GetPosition();
	mLastMousePos = Vector2<int>{ (int)cursorPosition[0], (int)cursorPosition[1]};

	memset(mKey, 0x00, sizeof(mKey));

	mMouseLButtonDown = false;
	mRotateWhenLButtonDown = rotateWhenLButtonDown;
}

//
// GameDemoCameraController::OnMouseButtonDown		- Chapter 10, page 282
//
bool GameDemoCameraController::OnMouseButtonDown(
	const Vector2<int> &mousePos, const int radius, const eastl::string &buttonName)
{
	if (buttonName == "PointerLeft")
	{
		mMouseLButtonDown = true;

		// We want mouse movement to be relative to the position
		// the cursor was at when the user first presses down on
		// the left button
		mLastMousePos = mousePos;
		return true;
	}
	return false;
}

bool GameDemoCameraController::OnMouseButtonUp(
	const Vector2<int> &mousePos, const int radius, const eastl::string &buttonName)
{
	if (buttonName == "PointerLeft")
	{
		mMouseLButtonDown = false;
		return true;
	}
	return false;
}


//  class GameDemoCameraController::OnMouseMove		- Chapter 10, page 282

bool GameDemoCameraController::OnMouseMove(const Vector2<int> &mousePos, const int radius)
{
	// There are two modes supported by this controller.

	if (mRotateWhenLButtonDown)
	{
		// Mode 1 - rotate the view only when the left mouse button is down
		// Only look around if the left button is down
		if (mLastMousePos != mousePos && mMouseLButtonDown)
		{
			mRotateSpeed = mMaxRotateSpeed;

			System* system = System::Get();
			mYaw += ((mLastMousePos[0] - mousePos[0]) / (float)system->GetWidth()) * mRotateSpeed;
			mPitch += ((mousePos[1] - mLastMousePos[1]) / (float)system->GetHeight()) * mRotateSpeed;
			mLastMousePos = mousePos;
		}
	}
	else if (mLastMousePos != mousePos)
	{
		mRotateSpeed = mMaxRotateSpeed;

		// Mode 2 - rotate the controller when the mouse buttons are up
		System* system = System::Get();
		mYaw += ((mLastMousePos[0] - mousePos[0]) / (float)system->GetWidth()) * mRotateSpeed;
		mPitch += ((mousePos[1] - mLastMousePos[1]) / (float)system->GetHeight()) * mRotateSpeed;
		mLastMousePos = mousePos;
	}

	return true;
}

//  class GameDemoCameraController::OnUpdate			- Chapter 10, page 283

void GameDemoCameraController::OnUpdate(unsigned int timeMs, unsigned long deltaMs)
{
	// Special case, mouse is whipped outside of window before it can update.
	if (mEnabled)
	{
		System* system = System::Get();
		Vector2<unsigned int> cursorPosition = system->GetCursorControl()->GetPosition();
		Vector2<int> mousePosition{ (int)cursorPosition[0], (int)cursorPosition[1] };

		Renderer* renderer = Renderer::Get();
		Vector2<unsigned int> screenSize(renderer->GetScreenSize());
		RectangleShape<2, int> screenRectangle;
		screenRectangle.mCenter[0] = screenSize[0] / 2;
		screenRectangle.mCenter[1] = screenSize[1] / 2;
		screenRectangle.mExtent[0] = (int)screenSize[0];
		screenRectangle.mExtent[1] = (int)screenSize[1];

		// Only if we are moving outside quickly.
		bool reset = !screenRectangle.IsPointInside(mousePosition);

		if (reset)
		{
			// Force a reset.
			system->GetCursorControl()->SetPosition(0.5f, 0.5f);
			cursorPosition = system->GetCursorControl()->GetPosition();
			mLastMousePos = Vector2<int>{ (int)cursorPosition[0], (int)cursorPosition[1] };
		}
	}

	//Handling rotation as a result of mouse position
	{
		mPitch = eastl::max(-89.f, eastl::min(89.f, mPitch));

		// Calculate the new rotation matrix from the camera
		// yaw and pitch (zrotate and xrotate).
		Matrix4x4<float> yawRotation = Rotation<4, float>(
			AxisAngle<4, float>(Vector4<float>::Unit(2), mYaw * (float)GE_C_DEG_TO_RAD));
		Matrix4x4<float> pitchRotation = Rotation<4, float>(
			AxisAngle<4, float>(Vector4<float>::Unit(0), -mPitch * (float)GE_C_DEG_TO_RAD));

		mAbsoluteTransform.SetRotation(yawRotation * pitchRotation);
		mAbsoluteTransform.SetTranslation(mCamera->GetAbsoluteTransform().GetTranslation());
	}

	bool isTranslating = false;
	Vector4<float> atWorld = Vector4<float>::Zero();
	Vector4<float> rightWorld = Vector4<float>::Zero();
	Vector4<float> upWorld = Vector4<float>::Zero();

	if (mKey[KEY_KEY_W] || mKey[KEY_KEY_S])
	{
		// This will give us the "look at" vector 
		// in world space - we'll use that to move
		// the camera.
		atWorld = Vector4<float>::Unit(1); // forward vector
#if defined(GE_USE_MAT_VEC)
		atWorld = mAbsoluteTransform * atWorld;
#else
		atWorld = atWorld * mAbsoluteTransform;
#endif

		if (mKey[KEY_KEY_S])
			atWorld *= -1.f;

		isTranslating = true;
	}

	if (mKey[KEY_KEY_A] || mKey[KEY_KEY_D])
	{
		// This will give us the "look right" vector 
		// in world space - we'll use that to move
		// the camera.
		rightWorld = Vector4<float>::Unit(0); // right vector
#if defined(GE_USE_MAT_VEC)
		rightWorld = mAbsoluteTransform * rightWorld;
#else
		rightWorld = rightWorld * mAbsoluteTransform;
#endif

		if (mKey[KEY_KEY_A])
			rightWorld *= -1.f;

		isTranslating = true;
	}


	if (mKey[KEY_SPACE] || mKey[KEY_KEY_C] || mKey[KEY_KEY_X])
	{
		//Unlike strafing, Up is always up no matter
		//which way you are looking
		upWorld = Vector4<float>::Unit(2); // up vector
#if defined(GE_USE_MAT_VEC)
		upWorld = mAbsoluteTransform * upWorld;
#else
		upWorld = upWorld * mAbsoluteTransform;
#endif

		if (!mKey[KEY_SPACE])
			upWorld *= -1.f;

		isTranslating = true;
	}

	if (mEnabled && isTranslating)
	{
		float elapsedTime = (float)deltaMs / 1000.0f;

		Vector4<float> direction = atWorld + rightWorld + upWorld;
		Normalize(direction);

		mMoveSpeed = mMaxMoveSpeed;
		direction *= mMoveSpeed * elapsedTime;
		Vector4<float> pos = mCamera->GetAbsoluteTransform().GetTranslationW0() + direction;
		mAbsoluteTransform.SetTranslation(pos);
	}

	// update transform matrix
	mCamera->GetRelativeTransform() = mAbsoluteTransform;
}
