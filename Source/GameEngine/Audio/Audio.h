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

#ifndef AUDIO_H
#define AUDIO_H

#include "GameEngineStd.h"

#include "Core/IO/ResourceCache.h"

class SoundResourceExtraData;


//////////////////////////////////////////////////////////////////////
// SoundType Description
//
// This is an enum that represents the different kinds of sound data
// streams the sound system can handle.
//
//////////////////////////////////////////////////////////////////////

enum SoundType
{
	SOUND_TYPE_FIRST,
	SOUND_TYPE_MP3 = SOUND_TYPE_FIRST,
	SOUND_TYPE_WAVE,
	SOUND_TYPE_MIDI,
	SOUND_TYPE_OGG,

	// This needs to be the last sound type
	SOUND_TYPE_COUNT,
	SOUND_TYPE_UNKNOWN,
};

extern char *gSoundExtentions[];

//////////////////////////////////////////////////////////////////////
// class IAudioBuffer							- Chapter 13, page 409
//
// The interface class that defines the public API for audio buffers.
// An audio buffer maps to one instance of a sound being played, 
// which ISNT the sound data. Two different sounds can be played from
// the same source data - such as two explosions in two different places.
//////////////////////////////////////////////////////////////////////

class BaseAudioBuffer
{
public:
	virtual ~BaseAudioBuffer() { }

	virtual void *Get()=0;
	virtual eastl::shared_ptr<ResHandle> GetResource()=0;
	virtual bool OnRestore()=0;

	virtual bool Play(int volume, bool looping)=0;
	virtual bool Pause()=0;
	virtual bool Stop()=0;
	virtual bool Resume()=0;

	virtual bool TogglePause()=0;
	virtual bool IsPlaying()=0;
	virtual bool IsLooping() const=0;
	virtual void SetVolume(int volume)=0;
    virtual void SetPosition(unsigned long newPosition)=0;
	virtual int GetVolume() const=0;
	virtual float GetProgress()=0;
};

//////////////////////////////////////////////////////////////////////
// class AudioBuffer							- Chapter 13, page 411 
//
// Implements IAudiobuffer interface using a smart pointer to SoundResource.
//
//////////////////////////////////////////////////////////////////////

class AudioBuffer : public BaseAudioBuffer
{
public: 
	virtual eastl::shared_ptr<ResHandle> GetResource() { return m_Resource; }
	virtual bool IsLooping() const { return m_isLooping; }
	virtual int GetVolume() const { return m_Volume; }
protected:
	AudioBuffer(eastl::shared_ptr<ResHandle>resource)
	{ 
		m_Resource = resource; 
		m_isPaused = false;
		m_isLooping = false;
		m_Volume = 0;
	}	// disable public construction

	eastl::shared_ptr<ResHandle> m_Resource;

	// Is the sound paused
	bool m_isPaused;

	// Is the sound looping
	bool m_isLooping;

	//the volume
	int m_Volume;
};


//////////////////////////////////////////////////////////////////////
// class BaseAudio									- Chapter 13, page 411
//
// This interface class describes the public interface for 
// a game's audio system. 
//////////////////////////////////////////////////////////////////////

class BaseAudio
{
public:
	virtual bool Active()=0;

	virtual BaseAudioBuffer* InitAudioBuffer(eastl::shared_ptr<ResHandle> handle)=0;
	virtual void ReleaseAudioBuffer(BaseAudioBuffer* audioBuffer)=0;

	virtual void StopAllSounds()=0;
	virtual void PauseAllSounds()=0;
	virtual void ResumeAllSounds()=0;

	virtual bool Initialize(void* id)=0;
	virtual void Shutdown()=0;
};

//////////////////////////////////////////////////////////////////////
// class Audio									- Chapter 13, page 412
//
// Implements BaseAudio interface - but not all the way - this is 
// still a base class. See class DirectSoundAudio.
// 
//////////////////////////////////////////////////////////////////////

class Audio : public BaseAudio
{
public:
	Audio();
	~Audio() { Shutdown(); }
	virtual void StopAllSounds();
	virtual void PauseAllSounds();
	virtual void ResumeAllSounds();

	virtual void Shutdown();
	static bool HasSoundCard(void);
	bool IsPaused() { return m_AllPaused; }

protected:

	typedef eastl::list<BaseAudioBuffer *> AudioBufferList;

	AudioBufferList m_AllSamples;	// List of all currently allocated audio buffers
	bool m_AllPaused;				// Has the sound system been paused?
	bool m_Initialized;				// Has the sound system been initialized?
};

extern Audio *g_pAudio;


#endif