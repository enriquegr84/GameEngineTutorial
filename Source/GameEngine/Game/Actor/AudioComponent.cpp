 
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

#include "Application/GameApplication.h"

const char* AudioComponent::Name = "AudioComponent";

AudioComponent::AudioComponent()
{
	mAudioResource = "";
	mLooping = false;
	mFadeInTime = 0.0f;
	mVolume = 100;
}

bool AudioComponent::Init(tinyxml2::XMLElement* pData)
{
	tinyxml2::XMLElement* pTexture = pData->FirstChildElement("Sound");
    if (pTexture)
	{
		mAudioResource = pTexture->FirstChild()->Value();
	}

	tinyxml2::XMLElement* pLooping = pData->FirstChildElement("Looping");
	if (pLooping)
	{
		eastl::string value = pLooping->FirstChild()->Value();
		mLooping = (value == "0") ? false : true;
	}

	tinyxml2::XMLElement* pFadeIn = pData->FirstChildElement("FadeIn");
	if (pFadeIn)
	{
		eastl::string value = pFadeIn->FirstChild()->Value();
		mFadeInTime = (float)atof(value.c_str());
	}

	tinyxml2::XMLElement* pVolume = pData->FirstChildElement("Volume");
	if (pVolume)
	{
		eastl::string value = pVolume->FirstChild()->Value();
		mVolume = atoi(value.c_str());
	}

	return true;
}

tinyxml2::XMLElement* AudioComponent::GenerateXml(void)
{
	tinyxml2::XMLDocument doc;

	// base element
	tinyxml2::XMLElement* pBaseElement = doc.NewElement(GetName());

	tinyxml2::XMLElement* pSoundNode = doc.NewElement("Sound");
	tinyxml2::XMLText* pSoundText = doc.NewText(mAudioResource.c_str());
    pSoundNode->LinkEndChild(pSoundText);
    pBaseElement->LinkEndChild(pSoundNode);

	tinyxml2::XMLElement* pLoopingNode = doc.NewElement("Looping");
	tinyxml2::XMLText* pLoopingText = doc.NewText(mLooping ? "1" : "0");
    pLoopingNode->LinkEndChild(pLoopingText);
    pBaseElement->LinkEndChild(pLoopingNode);

	tinyxml2::XMLElement* pFadeInNode = doc.NewElement("FadeIn");
	tinyxml2::XMLText* pFadeInText = doc.NewText(eastl::to_string(mFadeInTime).c_str());
    pFadeInNode->LinkEndChild(pFadeInText);
    pBaseElement->LinkEndChild(pFadeInNode);

	tinyxml2::XMLElement* pVolumeNode = doc.NewElement("Volume");
	tinyxml2::XMLText* pVolumeText = doc.NewText(eastl::to_string(mVolume).c_str());
    pVolumeNode->LinkEndChild(pVolumeText);
    pBaseElement->LinkEndChild(pVolumeNode);

	return pBaseElement;
}

void AudioComponent::PostInit()
{
	GameApplication* gameApp = (GameApplication*)Application::App;
	HumanView *humanView = gameApp->GetHumanView();
	if (!humanView)
	{
		LogError("Sounds need a human view to be heard!");
		return;
	}

	ProcessManager *processManager = humanView->GetProcessManager();
	if (!processManager)
	{
		LogError("Sounds need a process manager to attach!");
		return;
	}

	if (!gameApp->IsEditorRunning())
	{
		// The editor can play sounds, but it shouldn't run them when AudioComponents are initialized.
		/*
		BaseResource resource(mAudioResource);
		eastl::shared_ptr<ResHandle> rh = ResCache::Get()->GetHandle(&resource);
		eastl::shared_ptr<SoundProcess> sound(new SoundProcess(rh, 0, true));
		processManager->AttachProcess(sound);

		// fade process
		if (mFadeInTime > 0.0f)
		{
			eastl::shared_ptr<FadeProcess> fadeProc(new FadeProcess(sound, 10000, 100));
			processManager->AttachProcess(fadeProc);
		}
		*/
	}
}