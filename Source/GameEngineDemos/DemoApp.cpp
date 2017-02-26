// Geometric Tools, LLC
// Copyright (c) 1998-2014
// Distributed under the Boost Software License, Version 1.0.
// http://www.boost.org/LICENSE_1_0.txt
// http://www.geometrictools.com/License/Boost/LICENSE_1_0.txt
//
// File Version: 5.0.2 (2011/08/13)

#include "Core/Logger/LogToStdout.h"
#include "Core/IO/Environment.h"

#include "DemoApp.h"

//----------------------------------------------------------------------------
int main(int numArguments, char* arguments[])
{
	// Application entry point. It is the startup function used for initialization
	// The application layer depends on the directory structure that ships with the 
	// libraries. You need to create the APP_PATH environment variable in order for 
	// the applications to find various data files.
#ifndef __APPLE__
	Application::ApplicationPath = Environment::GetAbsolutePath("../../../");
#else
	// Mac OS X Lion returns NULL on any getenv call (such as the one in
	// Environment::GetVariable).  This hack works around the problem.
	if (system("cp ~/.MacOSX/apppath.txt tempapppath.txt") == 0)
	{
		std::ifstream inFile("tempapppath.txt");
		if (inFile)
		{
			getline(inFile, Application::AppPath);
			inFile.close();
			system("rm tempapppath.txt");
		}
	}
#endif
	if (Application::ApplicationPath == "")
	{
		LogError("Please set the APP_PATH environment variable.\n");
		return INT_MAX;
	}
	Application::ApplicationPath += "/";

	// Always check the application directory.
	Environment::InsertDirectory(Application::ApplicationPath);

	// Initialization
	DemoApplication* demoApp = new DemoApplication();
	Application::App = demoApp;

	int exitCode = -1;
	try
	{
		Application::App->OnRun();
		exitCode = 0;
	}
	catch (...)
	{
		// Catch all exceptions – dangerous!!!  
		// Respond (perhaps only partially) to the exception, then  
		// re-throw to pass the exception to some other handler  
		// throw;
		LogError("An error happend during execution.\n");
	}
	//delete0(Application::TheCommand);

	Environment::RemoveAllDirectories();

	// Termination
	delete(Application::App);

	return exitCode;
}


//----------------------------------------------------------------------------
DemoApplication::DemoApplication()
:	WindowApplication("DemoApp", 0, 0, 1200, 800, { 0.9f, 0.9f, 0.9f, 1.0f }),
	mCamera(eastl::make_shared<Camera>(true, true))
{
#if defined(_DEBUG)
	LogToStdout reporter(Logger::Listener::LISTEN_FOR_ALL);
#endif
}

//----------------------------------------------------------------------------
DemoApplication::~DemoApplication()
{

}

//----------------------------------------------------------------------------
bool DemoApplication::OnInitialize()
{
    if (!WindowApplication::OnInitialize())
    {
        return false;
    }
	
	//Initialize timer
	InitTime();

	// InitializeCamera(...) occurs before CreateScene() because the billboard
	// node creation requires the camera to be initialized.
	InitializeCamera(
		60.0f, GetAspectRatio(), 0.1f, 100.0f, 0.005f, 0.002f,
		{ 0.0f, -1.0f, 0.25f }, { 0.0f, 1.0f, 0.0f },  { 0.0f, 0.0f, 1.0f });
	//CreateScene();

    return true;
}

//----------------------------------------------------------------------------
void DemoApplication::OnIdle()
{
	mRenderer->ClearBuffers();
	
	char message[256];
	sprintf(message, "fps: %i", mFramesPerSecond);
	mRenderer->Draw(8, mHeight - 8, { 1.0f, 1.0f, 1.0f, 1.0f }, message);
	mRenderer->DisplayColorBuffer(0);
}

//----------------------------------------------------------------------------
void DemoApplication::OnTerminate()
{
	mCamera = 0;

	WindowApplication::OnTerminate();
}

//----------------------------------------------------------------------------
void DemoApplication::OnRun()
{
	WindowApplication::OnRun();
}

//----------------------------------------------------------------------------
bool DemoApplication::OnResize(int width, int height)
{
	return true;
	/*
		float upFovDegrees, aspectRatio, dMin, dMax;
		mCamera->GetFrustum(upFovDegrees, aspectRatio, dMin, dMax);
		mCamera->SetFrustum(upFovDegrees, GetAspectRatio(), dMin, dMax);
		mPVWMatrices.Update();
		return true;
	*/
}

//----------------------------------------------------------------------------
void DemoApplication::InitializeCamera(float upFovDegrees, float aspectRatio, float dmin, float dmax,
	float translationSpeed, float rotationSpeed, eastl::array<float, 3> const& pos,
	eastl::array<float, 3> const& dir, eastl::array<float, 3> const& up)
{
	mCamera->SetFrustum(upFovDegrees, aspectRatio, dmin, dmax);
	Vector4<float> camPosition{ pos[0], pos[1], pos[2], 1.0f };
	Vector4<float> camDVector{ dir[0], dir[1], dir[2], 0.0f };
	Vector4<float> camUVector{ up[0], up[1], up[2], 0.0f };
	Vector4<float> camRVector = Cross(camDVector, camUVector);
	mCamera->SetFrame(camPosition, camDVector, camUVector, camRVector);

	mCameraRig.ComputeWorldAxes();
	mCameraRig.SetTranslationSpeed(translationSpeed);
	mCameraRig.SetRotationSpeed(rotationSpeed);
}

//----------------------------------------------------------------------------
bool DemoApplication::OnEvent(const Event& event)
{
	bool result = 0;
	switch (event.mEventType) 
	{
		case ET_KEY_INPUT_EVENT:
		{
			KeyCode virtualKey = event.mKeyInput.mKey;
			if (event.mKeyInput.mPressedDown)
				OnKeyDown(virtualKey);
			else
				OnKeyUp(virtualKey);
		}
		break;

		case ET_MOUSE_INPUT_EVENT:
		{
			switch (event.mMouseInput.mEvent)
			{
				case MIE_MOUSE_MOVED:
					OnMouseMotion(	event.mMouseInput.mEvent,
									event.mMouseInput.mButtonStates,
									event.mMouseInput.X, 
									event.mMouseInput.Y		);
					break;

				case MIE_MOUSE_WHEEL:
					/*
					OnMouseWheel(	event.mMouseInput.mWheel, 
									event.mMouseInput.X,
									event.mMouseInput.Y);
					*/
					break;
				case MIE_LMOUSE_PRESSED_DOWN:
				case MIE_LMOUSE_LEFT_UP:
					OnMouseClick(	event.mMouseInput.mEvent, 
									event.mMouseInput.mButtonStates,
									event.mMouseInput.X, 
									event.mMouseInput.Y	);
					break;
			}
		}
		break;

		default:
			return 0;
	}
	return result;
}

//----------------------------------------------------------------------------

bool DemoApplication::OnKeyDown(KeyCode key)
{
	switch (key)
	{
	case KEY_KEY_T:  // Slower camera translation.
		mCameraRig.SetTranslationSpeed(0.5f * mCameraRig.GetTranslationSpeed());
		return true;

	case KEY_KEY_Y:  // Faster camera translation.
		mCameraRig.SetTranslationSpeed(2.0f * mCameraRig.GetTranslationSpeed());
		return true;

	case KEY_KEY_R:  // Slower camera rotation.
		mCameraRig.SetRotationSpeed(0.5f * mCameraRig.GetRotationSpeed());
		return true;

	case KEY_KEY_E:  // Faster camera rotation.
		mCameraRig.SetRotationSpeed(2.0f * mCameraRig.GetRotationSpeed());
		return true;

	case KEY_KEY_P:
		/*
		if (mRenderer->GetRasterizerState() != mCullCWState)
		{
			Matrix4x4<float> xReflect = Matrix4x4<float>::Identity();
			xReflect(0, 0) = -1.0f;
			mCamera->SetPostProjectionMatrix(xReflect);
			mRenderer->SetRasterizerState(mCullCWState);
		}
		else
		{
			mCamera->SetPostProjectionMatrix(Matrix4x4<float>::Identity());
			mRenderer->SetDefaultRasterizerState();
		}
		mPVWMatrices.Update();
		*/
		return true;
	}

	return mCameraRig.PushMotion(key);
}

bool DemoApplication::OnKeyUp(KeyCode key)
{
	return mCameraRig.PopMotion(key);
}

bool DemoApplication::OnMouseMotion(MouseInputEvent button, unsigned int state, int x, int y)
{
	return true;
	/*
	mPVWMatrices.Update();
	mCuller.ComputeVisibleSet(mCamera, mScene);

	bool leftPressed = (0 != (state & MBSM_LEFT));
	if (leftPressed && mTrackball.GetActive())
	{
		mTrackball.SetFinalPoint(x, mYSize - 1 - y);
		mPVWMatrices.Update();
		return true;
	}
	return false;
	*/
}

bool DemoApplication::OnMouseClick(MouseInputEvent button, unsigned int state, int x, int y)
{
	return true;
	/*
	bool leftPressed = (0 != (state & MBSM_LEFT));
	if (leftPressed)
	{
		if (button == MIE_LMOUSE_PRESSED_DOWN)
		{
			mTrackball.SetActive(true);
			mTrackball.SetInitialPoint(x, mHeight - 1 - y);
		}
		else
		{
			mTrackball.SetActive(false);
		}

		return true;
	}

	return false;
	*/
}