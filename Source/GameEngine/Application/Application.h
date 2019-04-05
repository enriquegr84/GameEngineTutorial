// Application.h

#ifndef APPLICATION_H
#define APPLICATION_H

#include "EASTL/string.h"

/*
	Application layer concerns itself with the machine the game runs on.
	Its contents are divided further into different areas that deal with 
	devices, operating system and game running. It handles user input devices:
	such as keyboard, mouse, gamepad, joystick, etc... 
	The application layer is initiated as global singleton object and is referred
	to throughout the code through a pointer.
*/

class Application
{
protected:
    // Abstract base class.
    Application ();

public:
    virtual ~Application ();

	virtual bool OnInitialize() = 0;
	virtual void OnTerminate() = 0;
	virtual void OnRun() = 0;

    // The unique application object.
    static Application* App;

    // The unique command-line arguments object.
    //static Command* TheCommand;

    // The installation directory of the application.
    static eastl::string ApplicationPath;
};

#endif
