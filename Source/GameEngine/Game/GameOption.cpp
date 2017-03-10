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
	m_Level = "";
	m_Renderer = "Direct3D11";
	m_soundEffectsVolume = 1.0f;			
	m_musicVolume = 1.0f;				
	m_expectedPlayers = 1;
	m_listenPort = -1;					
	m_gameHost = "GameHost";
	m_numAIs = 1;
	m_maxAIs = 4;
	m_maxPlayers = 4;
	m_ScreenSize = Vector2<int>(800,600);

	m_pRoot = NULL;
}

void GameOption::Init(const wchar_t* xmlFileName)
{
	// read the XML file
	// if needed, override the XML file with options passed in on the command line.
	m_pRoot = XmlResourceLoader::LoadAndReturnRootXMLElement(xmlFileName);
    if (!m_pRoot)
    {
        LogError(
			eastl::string("Failed to load game options from file: ") + 
			eastl::string(xmlFileName));
        return;
    }

	if (m_pRoot)
	{
        // Loop through each child element and load the component
        XMLElement* pNode = NULL;
		pNode = m_pRoot->FirstChildElement("Graphics"); 
		if (pNode)
		{
			eastl::string attribute;
			attribute = pNode->Attribute("renderer");
			if (attribute != "Direct3D9" && attribute != "Direct3D11")
			{
				LogAssert(0 && "Bad Renderer setting in Graphics options.");
			}
			else
			{
				m_Renderer = attribute;
			}

			if (pNode->Attribute("width"))
			{
				pNode->Attribute("width", &m_ScreenSize.Width);
				if (m_ScreenSize.Width < 800) m_ScreenSize.Width = 800;
			}

			if (pNode->Attribute("height"))
				pNode->Attribute("height", &m_ScreenSize.Height);
		}

		pNode = m_pRoot->FirstChildElement("Sound"); 
		if (pNode)
		{
			m_musicVolume = atoi(pNode->Attribute("musicVolume")) / 100.0f;
			m_soundEffectsVolume = atoi(pNode->Attribute("sfxVolume")) / 100.0f;
		}

		pNode = m_pRoot->FirstChildElement("Multiplayer"); 
		if (pNode)
		{
			m_expectedPlayers = atoi(pNode->Attribute("expectedPlayers"));
			m_numAIs = atoi(pNode->Attribute("numAIs"));
			m_maxAIs = atoi(pNode->Attribute("maxAIs"));
			m_maxPlayers = atoi(pNode->Attribute("maxPlayers"));
			
			m_listenPort = atoi(pNode->Attribute("listenPort"));
			m_gameHost = pNode->Attribute("gameHost");
		}
	}
}