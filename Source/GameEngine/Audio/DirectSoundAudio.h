//========================================================================
// DirectSoundAudio::io.h : Implements audio interfaces for DirectSound
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

#ifndef DIRECTSOUNDAUDIO_H
#define DIRECTSOUNDAUDIO_H

#include "GameEngineStd.h"

#include "Audio.h"

// DirectSound includes
#include <mmsystem.h>
#include <dsound.h>

/*
	DirectSoundAudio::AudioBuffer
	Platform-dependent implementation of the DirectSoundAudioBuffer. It picks up and defines
	the remaining unimplemented virtual functions from the AudioBuffer interface
*/
class DirectSoundAudioBuffer : public AudioBuffer
{
protected:
	LPDIRECTSOUNDBUFFER mSample;

public:
	DirectSoundAudioBuffer(LPDIRECTSOUNDBUFFER sample, eastl::shared_ptr<ResHandle> resource);
	virtual void *Get();
	virtual bool OnRestore();

	virtual bool Play(int volume, bool looping);
	virtual bool Pause();
	virtual bool Stop();
	virtual bool Resume();

	virtual bool TogglePause();
	virtual bool IsPlaying();
	virtual void SetVolume(int volume);
    virtual void SetPosition(unsigned long newPosition);

	virtual float GetProgress();

private:
	HRESULT FillBufferWithSound( );
	HRESULT RestoreBuffer( BOOL* pbWasRestored );
};


/*
	DirectSoundAudio::Audio
	Implements the rest of the BaseAudio interface left out by Audio.
	If you are interested in implementing a sound system using OpenAL
	you'd create a class OpenALAudioBuffer from AudioBuffer.
*/
class DirectSoundAudio : public Audio
{
public:
	DirectSoundAudio() { mDS = NULL; }
	virtual bool Active() { return mDS!=NULL; }

	virtual BaseAudioBuffer *InitAudioBuffer(eastl::shared_ptr<ResHandle> handle);
	virtual void ReleaseAudioBuffer(BaseAudioBuffer* audioBuffer);

	virtual void Shutdown();
	virtual bool Initialize(void* id);

protected:

    IDirectSound8* mDS;

	HRESULT SetPrimaryBufferFormat( unsigned long dwPrimaryChannels, 
									unsigned long dwPrimaryFreq, 
									unsigned long dwPrimaryBitRate );
};

#endif