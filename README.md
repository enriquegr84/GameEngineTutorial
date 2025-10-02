# Game Engine Tutorial

Unlike most Game Engines which are made for either professional use or as hobby project, we introduce a new engine 
for educational purpose. Despite of the traditional educational system in which a professor takes the responsibility 
for lecturing well known subjects to many students, we believe that a cooperative educational system in which a 
community takes part in the process of lecturing as well as learning will provide a better educational model. 
Our motivation is to create a game development educational network that allows anyone to participate for both coding 
and lecturing in many different ways: detecting or fixing mistakes, asking or solving questions and the most important 
individual contribution, **adding or updating content**. GitHub is the chosen framework because it fits our necessities, 
we can use it as a code repository, create a learning documentation that we will gradually update in the wiki section and 
contribute to the project in the issues section.

Our educational goal places priority on clean and well-designed code over a rather complicated but efficient code in terms 
of performance. As we know, game development is already a difficult and challenging area to work with, and that is precisely 
why we need to apply our efforts on maintaining a well-defined and structured engine so we can all work with and use for our 
personal projects. With this concern in mind, the engine was made as the result of merging three open source code projects:

-	https://www.mcshaffry.com/GameCode/ is a Game Engine which presents an architecture based on the Model-View-Controller (MVC) 
architectural pattern.
-	https://www.geometrictools.com/ are tools which provides a strong scientist background on graphics, mathematics and physics. 
It supports low level system, and separates OpenGL/Direct3D implementations.
-	http://irrlicht.sourceforge.net/ is a cross-platform Realtime 3D Engine which integrates many visual features implemented 
in OpenGL/Direct3D old versions.

These are the core projects which conforms the Game Engine, and they have been integrated in such a way that each project 
complements what the others are lacking. Also, it has been used partly the Quake III Arena open source code, 
https://github.com/id-Software/Quake-III-Arena for our showcase game, which demonstrates how the popular Quake III game can 
be implemented into a modern engine. Of course, there are many other 3rd Party libraries which the engine either integrate 
inside or use as external library. Bear in mind that this is an illustrative example of how to proceed with the game development, 
but by no means is necessarily the direction we are going to take. The idea is that we conveniently agree on the best way to lead 
the development following our educational philosophy. 

The Game Engine is a source code template meant to assist anyone for creating any educational project they like, not only for 
videogame but research and simulation projects in diverse scientist areas such as A.I, Audio, Networking, Graphics and Physics. 
As example, we show an A.I. research project which make use of this framework in the following link 
https://github.com/enriquegr84/QuakeAI

**Installation:**

1.	Clone repository to the project folder.
2.	Download GameEngineTutorial.rar file from the following link:
    -	https://www.dropbox.com/s/mv52rhzq52g0bfa/GameEngineTutorial.rar?dl=0
3.	Replace all the file content inside the GameEngineTutorial project folder.
4.	Open GameEngineDemos.sln from GameEngineTutorial\Source\GameEngineDemos\Msvc directory.
5.	Set the GameEngineTutorial as the startup project.
6.	Choose x86 platform and debug configuration options so we can rebuild the GameEngine project. Next, 
    we rebuild the GameEngineTutorial project and finally run the application.

We can run the application for an x86 platform using either DirectX or OpenGL in release or debug mode. They differ because OpenGL 
has the “GL” extension in debug and release mode. Next, we rebuild the project in any of these configurations, copy the 
GameEngineTutorial\bin\Win32Debug and GameEngineTutorial\Lib\Win32Debug content from the GameEngineTutorial.rar file, and paste them 
in the new created directories, for example, GameEngineTutorial\bin\Win32Release and GameEngineTutorial\Lib\Win32Release.
There are three initial demos in the GameEngineTutorial project which are TriangleDemoApp.cpp, QuakeApp.cpp and GameDemoApp.cpp. 
Only one of them can be executed by uncommenting the main function brackets and commenting the others file main function. 
Unfortunately, we couldn’t get permission to upload the required Quake assets which means that the QuakeApp can’t be executed.

![demo1](https://user-images.githubusercontent.com/4594154/76159538-9d56a780-6119-11ea-9e27-be55c8e3f7ed.png)
![demo2](https://user-images.githubusercontent.com/4594154/76159547-ad6e8700-6119-11ea-9923-f43dad3cc251.png)


