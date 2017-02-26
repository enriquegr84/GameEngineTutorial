// Application.h

#ifndef APPLICATION_H
#define APPLICATION_H

#include "EASTL/string.h"

class Application
{
protected:
    // Abstract base class.
    Application ();

public:
    virtual ~Application ();

	// Event callbacks.
	virtual bool OnInitialize() = 0;
	virtual void OnTerminate() = 0;
	virtual void OnRun() = 0;

    // The unique application object.
    static Application* App;

    // The unique command-line arguments object.
    //static Command* TheCommand;

    // The installation directory of the application.
    static eastl::string ApplicationPath;

    // The path to the application project directory.
    static eastl::string ProjectApplicationPath;
};

#endif
