//========================================================================
// Initialization.cpp : Defines utility functions for game initialization
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

#include "GameOption.h"

#include "Core/IO/XmlResource.h"

GameOption::GameOption()
{
	// set all the options to decent default valu
	mLevel = "";

	mRendererType = RendererType::RT_DIRECT3D11;
	mScreenSize = Vector2<int>{ 800,600 };
	mFullScreen = false;
	mAntiAlias = 0;
	mTesselation = 0;

	mSoundEffectsVolume = 1.0f;			
	mMusicVolume = 1.0f;	

	mGameHost = "GameHost";
	mExpectedPlayers = 1;
	mListenPort = -1;					
	mNumAIs = 1;
	mMaxAIs = 4;
	mMaxPlayers = 4;

	mRoot = NULL;
}

void GameOption::Init(const wchar_t* xmlFileName)
{
	// read the XML file
	// if needed, override the XML file with options passed in on the command line.
	mRoot = XmlResourceLoader::LoadAndReturnRootXMLElement(xmlFileName);
    if (!mRoot)
    {
        LogError(
			L"Failed to load game options from file: " + eastl::wstring(xmlFileName));
        return;
    }
	else
	{
        // Loop through each child element and load the component
        XMLElement* pNode = NULL;
		pNode = mRoot->FirstChildElement("Graphics"); 
		if (pNode)
		{
			eastl::string attribute;
			attribute = pNode->Attribute("Renderer");
			if (attribute != "Direct3D 11")
			{
				LogError("Bad Renderer setting in Graphics options.");
			}
			else
			{
				mRendererType = RendererType::RT_DIRECT3D11;
			}

			if (pNode->Attribute("Width"))
			{
				mScreenSize[0] = pNode->IntAttribute("Width", mScreenSize[0]);
				if (mScreenSize[0] < 800) mScreenSize[0] = 800;
			}

			if (pNode->Attribute("Height"))
				mScreenSize[1] = pNode->IntAttribute("Height", mScreenSize[1]);
		}

		pNode = mRoot->FirstChildElement("Sound"); 
		if (pNode)
		{
			mMusicVolume = atoi(pNode->Attribute("MusicVolume")) / 100.0f;
			mSoundEffectsVolume = atoi(pNode->Attribute("SFXVolume")) / 100.0f;
		}

		pNode = mRoot->FirstChildElement("Multiplayer"); 
		if (pNode)
		{
			mExpectedPlayers = atoi(pNode->Attribute("ExpectedPlayers"));
			mNumAIs = atoi(pNode->Attribute("NumAIs"));
			mMaxAIs = atoi(pNode->Attribute("MaxAIs"));
			mMaxPlayers = atoi(pNode->Attribute("MaxPlayers"));
			
			mListenPort = atoi(pNode->Attribute("ListenPort"));
			mGameHost = pNode->Attribute("GameHost");
		}
	}
}