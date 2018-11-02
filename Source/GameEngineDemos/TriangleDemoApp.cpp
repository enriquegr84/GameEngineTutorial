// Geometric Tools, LLC
// Copyright (c) 1998-2014
// Distributed under the Boost Software License, Version 1.0.
// http://www.boost.org/LICENSE_1_0.txt
// http://www.geometrictools.com/License/Boost/LICENSE_1_0.txt
//
// File Version: 5.0.2 (2011/08/13)

#include "Core/Logger/LogReporter.h"
#include "Core/IO/Environment.h"

#include "Graphic/GraphicStd.h"

#include "TriangleDemoApp.h"
/*
//----------------------------------------------------------------------------
int main(int numArguments, char* arguments[])
{
#if defined(_DEBUG)
	LogReporter reporter(
		"",
		Logger::Listener::LISTEN_FOR_NOTHING,
		Logger::Listener::LISTEN_FOR_NOTHING,
		Logger::Listener::LISTEN_FOR_NOTHING,
		Logger::Listener::LISTEN_FOR_ALL);
#endif

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

	// Initialization
	TriangleDemoApp* demoApp = new TriangleDemoApp();
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

	// Termination
	delete Application::App;

	return exitCode;
}
*/
//----------------------------------------------------------------------------
TriangleDemoApp::TriangleDemoApp()
:	WindowApplication("TriangleDemo", 0, 0, 800, 600, { 0.392f, 0.584f, 0.929f, 1.0f }),
#if defined(_OPENGL_)
	mCamera(eastl::make_shared<Camera>(true, false))
#else
	mCamera(eastl::make_shared<Camera>(true, true))
#endif
{

}

//----------------------------------------------------------------------------
TriangleDemoApp::~TriangleDemoApp()
{

}

//----------------------------------------------------------------------------
void TriangleDemoApp::InitializeCamera(
	float upFovDegrees, float aspectRatio, float dmin, float dmax, float translationSpeed, float rotationSpeed,
	eastl::array<float, 3> const& pos, eastl::array<float, 3> const& dir, eastl::array<float, 3> const& up)
{
	mCamera->SetFrustum(upFovDegrees, aspectRatio, dmin, dmax);
	Vector4<float> camPosition{ pos[0], pos[1], pos[2], 1.0f };
	Vector4<float> camDVector{ dir[0], dir[1], dir[2], 0.0f };
	Vector4<float> camUVector{ up[0], up[1], up[2], 0.0f };
	Vector4<float> camRVector = Cross(camDVector, camUVector);
	mCamera->SetFrame(camPosition, camDVector, camUVector, camRVector);
}

//----------------------------------------------------------------------------
bool TriangleDemoApp::CreateScene()
{
	//LoadBuiltInFont
	eastl::vector<eastl::string> path;
#if defined(_OPENGL_)
	path.push_back(FileSystem::Get()->GetPath("Effects/TextEffectVS.glsl"));
	path.push_back(FileSystem::Get()->GetPath("Effects/TextEffectPS.glsl"));
#else
	path.push_back(FileSystem::Get()->GetPath("Effects/TextEffect.hlsl"));
#endif
	eastl::shared_ptr<Font> builtInFont = eastl::make_shared<FontArialW400H18>(ProgramFactory::Get(), path, 256);
	Renderer::Get()->SetDefaultFont(builtInFont);

	VertexFormat vformat;
	vformat.Bind(VA_POSITION, DF_R32G32B32_FLOAT, 0);
	vformat.Bind(VA_COLOR, DF_R32G32B32A32_FLOAT, 0);

	path.clear(); 
#if defined(_OPENGL_)
	path.push_back(FileSystem::Get()->GetPath("Effects/ColorEffectVS.glsl"));
	path.push_back(FileSystem::Get()->GetPath("Effects/ColorEffectPS.glsl"));
#else
	path.push_back(FileSystem::Get()->GetPath("Effects/ColorEffect.hlsl"));
#endif
	eastl::shared_ptr<ColorEffect> effect =
		eastl::make_shared<ColorEffect>(mProgramFactory, path);

	MeshFactory mf;
	mf.SetVertexFormat(vformat);

	// Create the rectangle representation of the model triangle
	int const numSamples = 2;
	mTriangle = mf.CreateTriangle(numSamples, 1.0f, 1.0f);
	mTriangle->SetEffect(effect);

	struct Vertex
	{
		Vector3<float> position;
		Vector4<float> color;
	};
	Vertex* vertex = mTriangle->GetVertexBuffer()->Get<Vertex>();
	vertex[0].color = Vector4<float>{ 1.0f, 0.0f, 0.0f, 1.0f };
	vertex[1].color = Vector4<float>{ 0.0f, 1.0f, 0.0f, 1.0f };
	vertex[2].color = Vector4<float>{ 0.0f, 0.0f, 1.0f, 1.0f };

	return true;
}

//----------------------------------------------------------------------------
bool TriangleDemoApp::OnInitialize()
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
	
	CreateScene();

    return true;
}

//----------------------------------------------------------------------------
void TriangleDemoApp::OnIdle()
{
	// clear the buffers before rendering
	mRenderer->ClearBuffers();
	
	mRenderer->Draw(mTriangle);

	wchar_t message[256];
	wsprintf(message, L"fps: %i", mFramesPerSecond);
	mRenderer->Draw(8, mHeight - 8, { 1.0f, 1.0f, 1.0f, 1.0f }, message);

	mRenderer->DisplayColorBuffer(0);
}

//----------------------------------------------------------------------------
void TriangleDemoApp::OnTerminate()
{
	mCamera = 0;

	WindowApplication::OnTerminate();
}

//----------------------------------------------------------------------------
void TriangleDemoApp::OnRun()
{
	WindowApplication::OnRun();
}

//----------------------------------------------------------------------------
bool TriangleDemoApp::OnResize(int width, int height)
{
	return true;
}
//----------------------------------------------------------------------------
bool TriangleDemoApp::OnEvent(const Event& event)
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

bool TriangleDemoApp::OnKeyDown(KeyCode key)
{
	return true;
}

bool TriangleDemoApp::OnKeyUp(KeyCode key)
{
	return true;
}

bool TriangleDemoApp::OnMouseMotion(MouseInputEvent button, unsigned int state, int x, int y)
{
	return true;
}

bool TriangleDemoApp::OnMouseClick(MouseInputEvent button, unsigned int state, int x, int y)
{
	return true;
}