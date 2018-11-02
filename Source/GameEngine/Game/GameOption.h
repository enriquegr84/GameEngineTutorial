//========================================================================
// GameOptions.h : Defines utility functions for game options
//
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

#ifndef GAMEOPTION_H
#define GAMEOPTION_H

#include "GameEngineStd.h"

#include "Mathematic/Algebra/Vector2.h"

struct GameOption
{
	// Level option
	eastl::string mLevel;

	// Rendering options

	//! Size of the window or the video mode in fullscreen mode. Default: 800x600
	Vector2<int> mScreenSize;
	//! Should be set to true if the device should run in fullscreen.
	/* Otherwise the device runs in windowed mode. Default: false. */
	bool mFullScreen;
	//! Specifies if the device should use fullscreen anti aliasing
	/* Default value: 0 - disabled */
	unsigned int mAntiAlias;
	//! Specifies how smooth should curved surfaces be rendered
	int mTesselation;

	// Sound options
	float mSoundEffectsVolume;			
	float mMusicVolume;				

	// Multiplayer options				
	eastl::string mGameHost;
	eastl::string mGameHostListenPort;
	eastl::string mClientAttachPort;

	int mListenPort;
	int mExpectedPlayers;
	int mNumAIs;
	int mMaxAIs;
	int mMaxPlayers;

	// XMLElement - look at this to find other options added by the developer
	tinyxml2::XMLElement *mRoot;

	GameOption();
	~GameOption() { }

	void Init(const wchar_t* xmlFilePath);
};


#endif