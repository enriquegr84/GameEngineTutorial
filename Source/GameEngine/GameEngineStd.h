//========================================================================
//	GameEngineStd.h : include file for app configuration, it is set up the
//	the project to run under the specific platform, OS, libraries and 
//	specific includes files that are used frequently, but are changed 
//	infrequently.

#ifndef GAMEENGINESTD_H
#define GAMEENGINESTD_H

#include "Core/CoreStd.h"
#include "Game/GameStd.h"
#include "Graphic/GraphicStd.h"
#include "Mathematic/MathematicStd.h"

/*
	Game Engine is based on the Model-View Controller software architectural
	pattern, it seeks to separate the logic of a system from the interface to 
	present or request changes to data. It is an event-driven architecture
	composed of serveral subsystems which provides specific services to the 
	engine and every subsystem in a game is classified in three main layers: 
	- The application layer deals with hardware and operating system. 
	- The game logic layer manages the game state and how it changes over time.
	- The game view layer presents the game state for each attached view.
*/

#endif // GAMEENGINESTD_H