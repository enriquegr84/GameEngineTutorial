//========================================================================
// Audio.cpp : Defines a simple sound system.
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

#include "Audio.h"
#include "SoundResource.h"

#pragma comment( lib, "dsound" )

//////////////////////////////////////////////////////////////////////
// Globals
//////////////////////////////////////////////////////////////////////

Audio* Audio::AudioSystem = NULL;
char *SoundExtentions[] = { ".mp3", ".wav", ".midi", ".ogg" };

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

Audio::Audio():
	mInitialized(false),
	mAllPaused(false)
{
}

//
// Audio::Shutdown								- Chapter 13, page 413
//
void Audio::Shutdown()
{
	AudioBufferList::iterator it=mAllSamples.begin();

	while (it!=mAllSamples.end())
	{
		BaseAudioBuffer *audioBuffer = (*it);
		audioBuffer->Stop();
		mAllSamples.pop_front();
	}
}

//
// Audio::PauseAllSounds						- Chapter 13, page 413
//   Pause all active sounds, including music
//
void Audio::PauseAllSounds()
{
	AudioBufferList::iterator i;
	AudioBufferList::iterator end;
	for(i=mAllSamples.begin(), end=mAllSamples.end(); i!=end; ++i)
	{
		BaseAudioBuffer *audioBuffer = (*i);
		audioBuffer->Pause();
	}

	mAllPaused=true;
}

//
// Audio::ResumeAllSounds						- Chapter 13, page 413
//
void Audio::ResumeAllSounds()
{
	AudioBufferList::iterator i;
	AudioBufferList::iterator end;
	for(i=mAllSamples.begin(), end=mAllSamples.end(); i!=end; ++i)
	{
		BaseAudioBuffer *audioBuffer = (*i);
		audioBuffer->Resume();
	}

	mAllPaused=false;
}

//
// Audio::StopAllSounds						- Chapter 13, page 413
//
void Audio::StopAllSounds()
{
	BaseAudioBuffer *audioBuffer = NULL;

	AudioBufferList::iterator i;
	AudioBufferList::iterator end;
	for(i=mAllSamples.begin(), end=mAllSamples.end(); i!=end; ++i)
	{
		audioBuffer = (*i);
		audioBuffer->Stop();
	}

	mAllPaused=false;
}


//
// Audio::HasSoundCard							- not described in the book
//
//   A bit of an anachronism in name - but it simply returns true if the sound system is active.
//
bool Audio::HasSoundCard(void)
{
	return (Audio::AudioSystem && Audio::AudioSystem->Active());
}


