 
//========================================================================
// AudioComponent.cpp : A component for attaching sounds to an actor
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


#include "AudioComponent.h"

const char* AudioComponent::g_Name = "AudioComponent";

AudioComponent::AudioComponent()
{
	m_audioResource = "";
	m_looping = false;
	m_fadeInTime = 0.0f;
	m_volume = 100;
}

bool AudioComponent::Init(XMLElement* pData)
{
    XMLElement* pTexture = pData->FirstChildElement("Sound");
    if (pTexture)
	{
		m_audioResource = pTexture->FirstChild()->Value();
	}

	XMLElement* pLooping = pData->FirstChildElement("Looping");
	if (pLooping)
	{
		eastl::string value = pLooping->FirstChild()->Value();
		m_looping = (value == "0") ? false : true;
	}

	XMLElement* pFadeIn = pData->FirstChildElement("FadeIn");
	if (pFadeIn)
	{
		eastl::string value = pFadeIn->FirstChild()->Value();
		m_fadeInTime = (float)atof(value.c_str());
	}

	XMLElement* pVolume = pData->FirstChildElement("Volume");
	if (pVolume)
	{
		eastl::string value = pVolume->FirstChild()->Value();
		m_volume = atoi(value.c_str());
	}

	return true;
}

XMLElement* AudioComponent::GenerateXml(void)
{
	XMLDocument doc;

	// base element
    XMLElement* pBaseElement = doc.NewElement(GetName());

	XMLElement* pSoundNode = new XMLElement("Sound");
    XMLText* pSoundText = doc.NewText(m_audioResource.c_str());
    pSoundNode->LinkEndChild(pSoundText);
    pBaseElement->LinkEndChild(pSoundNode);

    XMLElement* pLoopingNode = new XMLElement("Looping");
	XMLText* pLoopingText = doc.NewText(m_looping ? "1" : "0");
    pLoopingNode->LinkEndChild(pLoopingText);
    pBaseElement->LinkEndChild(pLoopingNode);

    XMLElement* pFadeInNode = new XMLElement("FadeIn");
	XMLText* pFadeInText = doc.NewText(eastl::to_string(m_fadeInTime).c_str());
    pFadeInNode->LinkEndChild(pFadeInText);
    pBaseElement->LinkEndChild(pFadeInNode);

    XMLElement* pVolumeNode = new XMLElement("Volume");
	XMLText* pVolumeText = doc.NewText(eastl::to_string(m_volume).c_str());
    pVolumeNode->LinkEndChild(pVolumeText);
    pBaseElement->LinkEndChild(pVolumeNode);

	return pBaseElement;
}

void AudioComponent::PostInit()
{
	/*
	// The editor can play sounds, but it shouldn't run them when AudioComponents are initialized.
	BaseResource resource(m_audioResource);
	eastl::shared_ptr<ResHandle> rh = g_pGameApp->m_ResCache->GetHandle(&resource);
	eastl::shared_ptr<SoundProcess> sound(new SoundProcess(rh, 0, true));
	processManager->AttachProcess(sound);

	// fade process
	if (m_fadeInTime > 0.0f)
	{
		eastl::shared_ptr<FadeProcess> fadeProc(new FadeProcess(sound, 10000, 100)); 
		processManager->AttachProcess(fadeProc);
	}
	*/
}