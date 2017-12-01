//========================================================================
// DirectSoundAudio.cpp : Defines a simple sound system that uses DirectSound
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


#include "SoundResource.h"
#include "DirectSoundAudio.h"

#include <cguid.h>

//////////////////////////////////////////////////////////////////////
// 
// DirectSoundAudio::Audio Implementation
//
//////////////////////////////////////////////////////////////////////


/*
	DirectSoundAudio::Initialize
	It is initialized by setting the cooperative level on the DirectSound object, informing
	to the sound driver you want more control over the sound system, specifically how the primary 
	sound buffer is structured and how other applications run at the same time. The DSSCL_PRIORITY 
	level is better than DSSCL_NORMAL because you can change the format of the output buffer. This 
	is a good setting for games that still want to allow background applications
*/
bool DirectSoundAudio::Initialize(void* id)
{
	if(mInitialized)
		return true;

	mInitialized=false;
	mAllSamples.clear();

	delete mDS;
	mDS = nullptr;

	HRESULT hr;

	HWND hWnd = static_cast<HWND>(id);

	// Create IDirectSound using the primary sound device
	if (FAILED(hr = DirectSoundCreate8(NULL, &mDS, NULL)))
	{
		LogError(GetErrorMessage(hr));
		return false;
	}

	// Set DirectSound coop level 
	if( FAILED( hr = mDS->SetCooperativeLevel( hWnd, DSSCL_PRIORITY) ) )
	{
		LogError(GetErrorMessage(hr));
		return false;
	}

	if( FAILED( hr = SetPrimaryBufferFormat( 2, 44100, 16 ) ) )
	{
		LogError(GetErrorMessage(hr));
		return false;
	}

	mInitialized = true;

	return true;
}

/*
	DirectSoundAudio::SetPrimaryBufferFormat
	It sets your primary buffer format to a flavor you want; most likely, it will be 44KHz, 
	16-bit, and some number of channels that is a good trade-off between memory use and the 
	number of simultaneous sound effects in a game. The memory which is spent with more channels 
	is dependent on the sound hardware, so take into account the maximum number of channels that
	the audio card support.
*/
HRESULT DirectSoundAudio::SetPrimaryBufferFormat( 
	unsigned long dwPrimaryChannels, 
    unsigned long dwPrimaryFreq, 
    unsigned long dwPrimaryBitRate )
{
	// !WARNING! - Setting the primary buffer format and then using this
	// it for DirectMusic messes up DirectMusic! 
	//
	// If you want your primary buffer format to be 22kHz stereo, 16-bit
	// call with these parameters:	SetPrimaryBufferFormat(2, 22050, 16);

	HRESULT hr;
	LPDIRECTSOUNDBUFFER dsbPrimary = NULL;

	if( ! mDS )
		return CO_E_NOTINITIALIZED;

	// Get the primary buffer 
	DSBUFFERDESC dsbd;
	ZeroMemory( &dsbd, sizeof(DSBUFFERDESC) );
	dsbd.dwSize = sizeof(DSBUFFERDESC);
	dsbd.dwFlags = DSBCAPS_PRIMARYBUFFER;
	dsbd.dwBufferBytes = 0;
	dsbd.lpwfxFormat = NULL;

	/*
		The method returns a pointer to the primary sound buffer where all the sound effects
		are mixed into a single sound stream that is rendered by the sound card
	*/
	if( FAILED( hr = mDS->CreateSoundBuffer( &dsbd, &dsbPrimary, NULL ) ) )
		return hr;

	WAVEFORMATEX wfx;
	ZeroMemory( &wfx, sizeof(WAVEFORMATEX) ); 
	wfx.wFormatTag = (WORD) WAVE_FORMAT_PCM; 
	wfx.nChannels = (WORD) dwPrimaryChannels; 
	wfx.nSamplesPerSec = (unsigned long) dwPrimaryFreq; 
	wfx.wBitsPerSample = (WORD) dwPrimaryBitRate; 
	wfx.nBlockAlign = (WORD) (wfx.wBitsPerSample / 8 * wfx.nChannels);
	wfx.nAvgBytesPerSec = (unsigned long) (wfx.nSamplesPerSec * wfx.nBlockAlign);

	/*
		SetFormat tells the sound driver to change the primary buffer's format to one
		that you specify.
	*/
	if( FAILED( hr = dsbPrimary->SetFormat(&wfx) ) )
		return hr;

	dsbPrimary->Release();

	return S_OK;
}

/*
	DirectSoundAudio::Shutdown
	It is called to stop and release all the sounds still active
*/
void DirectSoundAudio::Shutdown()
{
	if(mInitialized)
	{
		Audio::Shutdown();

		delete mDS;
		mInitialized = false;
	}
}

/*
	DirectSoundAudio::InitAudioBuffer
	Allocate a sample handle for the newborn sound (used by SoundResource) and tell you it's length.
	In our platform-agnostic design, an audio buffer is created from a sound resource, presumably
	something loaded from a file or more likely a resource file.
*/
BaseAudioBuffer *DirectSoundAudio::InitAudioBuffer(eastl::shared_ptr<ResHandle> resHandle)//const
{
	eastl::shared_ptr<SoundResourceExtraData> extra = 
		eastl::static_pointer_cast<SoundResourceExtraData>(resHandle->GetExtra());

    if( ! mDS )
        return NULL;

	/*
		It branches on the sound type, which signifies what kind of sound resource is about to play: 
		WAV, MP3, OGG, or MIDI. To extend this system to play any kind of sound format, the new code
		will be hooked in right here, in this case we are looking at WAV data or OGG data that has 
		been decompressed.
	*/
	switch(extra->GetSoundType())
	{
		case SOUND_TYPE_OGG:
		case SOUND_TYPE_WAVE:
			// We support WAVs and OGGs
			break;

		case SOUND_TYPE_MP3:
		case SOUND_TYPE_MIDI:
			//If it's a midi file, then do nothin at this time...
			//maybe we will support this in the future
			LogError("MP3s and MIDI are not supported");
			return NULL;
			break;

		default:
			LogError("Unknown sound type");
			return NULL;
	}//end switch

    LPDIRECTSOUNDBUFFER sampleHandle;

	/*
		The call to CreateSoundBuffer() is preceded by setting various values of a DSBUFFERDESC 
		structure that informs DirectSound what kind of sound is being created. It is only requested 
		the flags needed since each requires some overhead and limits if the buffer can be hardware 
		accelerated. The flags members controls what can happen to the sound. DSBCAPS_CTRLVOLUME flag,
		which tells DirectSound that we want to be able to control the volume of this sound
		effect. DSBCAPS_CTRL3D enables 3D sound, or DSBCAPS_CTRLPAN enables panning control.
	*/
    DSBUFFERDESC dsbd;
    ZeroMemory( &dsbd, sizeof(DSBUFFERDESC) );
    dsbd.dwSize = sizeof(DSBUFFERDESC);
    dsbd.dwFlags = DSBCAPS_CTRLVOLUME;
    dsbd.dwBufferBytes = resHandle->Size();
    dsbd.guid3DAlgorithm = GUID_NULL;
    dsbd.lpwfxFormat = const_cast<WAVEFORMATEX *>(extra->GetFormat());

	HRESULT hr;
    if( FAILED( hr = mDS->CreateSoundBuffer( &dsbd, &sampleHandle, NULL ) ) )
        return NULL;

	// Add handle to the list
	BaseAudioBuffer *audioBuffer = new DirectSoundAudioBuffer(sampleHandle, resHandle);
	mAllSamples.push_front( audioBuffer);
	
	return audioBuffer;
}

/*
	DirectSoundAudio::ReleaseAudioBuffer
	Deallocate a sample handle for the old sound (used by SoundResource). It call Stop method
	and remove it from the active sounds.
*/
void DirectSoundAudio::ReleaseAudioBuffer(BaseAudioBuffer *sampleHandle)//const
{
	sampleHandle->Stop();
	mAllSamples.remove(sampleHandle);
}

//
// DirectSoundAudioBuffer::DirectSoundAudioBuffer	- Chapter 13, page 420
//
DirectSoundAudioBuffer::DirectSoundAudioBuffer(
	LPDIRECTSOUNDBUFFER sample, eastl::shared_ptr<ResHandle> resource)
 : AudioBuffer(resource) 
{ 
	mSample = sample; 
	FillBufferWithSound();
}

//
// DirectSoundAudioBuffer::Get						- Chapter 13, page 420
//
void *DirectSoundAudioBuffer::Get()
{
 	if (!OnRestore())
		return NULL;

	return mSample;
}


//
// DirectSoundAudioioBuffer::Play					- Chapter 13, page 421
//    Play a sound
//
bool DirectSoundAudioBuffer::Play(int volume, bool looping)
{
   if(!Audio::AudioSystem->Active())
      return false;

	Stop();

	mVolume = volume;
	mIsLooping = looping;

	LPDIRECTSOUNDBUFFER dsb = (LPDIRECTSOUNDBUFFER)Get();
	if (!dsb)
		return false;

    dsb->SetVolume( volume );
    
    unsigned long dwFlags = looping ? DSBPLAY_LOOPING : 0L;

    return (S_OK==dsb->Play( 0, 0, dwFlags ) );

}//end Play


//
// DirectSoundAudioBuffer::Stop					- Chapter 13, page 421
//    Stop a sound and rewind play position to the beginning.
//
bool DirectSoundAudioBuffer::Stop()
{
	if(!Audio::AudioSystem->Active())
		return false;

	LPDIRECTSOUNDBUFFER dsb = (LPDIRECTSOUNDBUFFER)Get();

   if( ! dsb )
		return false;

	mIsPaused=true;
    dsb->Stop();
	return true;
}


//
// DirectSoundAudioBuffer::Pause					- Chapter X, page Y
//    Pause a sound 
//
bool DirectSoundAudioBuffer::Pause()
{
	if(!Audio::AudioSystem->Active())
		return false;

	LPDIRECTSOUNDBUFFER dsb = (LPDIRECTSOUNDBUFFER)Get();

   if( dsb )
		return false;

	mIsPaused=true;
    dsb->Stop();
	dsb->SetCurrentPosition(0);	// rewinds buffer to beginning.
	return true;
}

//
// DirectSoundAudioBuffer::Resume					- Chapter 13, page 421
//    Resume a sound
bool DirectSoundAudioBuffer::Resume()
{
	mIsPaused=false;
	return Play(GetVolume(), IsLooping());
}

//
// DirectSoundAudioBuffer::TogglePause				- Chapter 13, page 421
//    Pause a sound or Resume a Paused sound
//
bool DirectSoundAudioBuffer::TogglePause()
{
	if(!Audio::AudioSystem->Active())
		return false;

	if(mIsPaused)
	{
		Resume();
	}
	else
	{
		Pause();				// note that the book code calls VStop().
								//   It's better to call VPause() instead.
	}

	return true;
}//end TogglePause





//
// DirectSoundAudioBuffer::IsPlaying				- Chapter 13, page 422
//
bool DirectSoundAudioBuffer::IsPlaying() 
{
	if(!Audio::AudioSystem->Active())
		return false;

	unsigned long dwStatus = 0;
	LPDIRECTSOUNDBUFFER dsb = (LPDIRECTSOUNDBUFFER)Get();
	dsb->GetStatus( &dwStatus );
    bool IsPlaying = ( ( dwStatus & DSBSTATUS_PLAYING ) != 0 );

	return IsPlaying;
}

//
// DirectSoundAudioBuffer::SetVolume				- Chapter 13, page 422
//
void DirectSoundAudioBuffer::SetVolume(int volume)
{
	// DSBVOLUME_MIN, defined in dsound.h is set to as -10000, which is just way too silent for a 
	// lower bound and many programmers find -5000 to be a better minimum bound for the volume 
	// range to avoid an annoying silence for the lower 50% of a volume slider that uses a logarithmic scale.
	// This was contributed by BystanderKain!
	int gccDSBVolumeMin = DSBVOLUME_MIN;

	if(!Audio::AudioSystem->Active())
		return;

	LPDIRECTSOUNDBUFFER pDSB = (LPDIRECTSOUNDBUFFER)Get();

	LogAssert(volume>=0 && volume<=100, "Volume must be a number between 0 and 100");

	// convert volume from 0-100 into range for DirectX - don't forget to use a logarithmic scale!

	float coeff = (float)volume / 100.0f;
	float logarithmicProportion = coeff >0.1f  ? 1+log10(coeff)  : 0;
	float range = float(DSBVOLUME_MAX - gccDSBVolumeMin);
	float fvolume = ( range * logarithmicProportion ) + gccDSBVolumeMin;

	LogAssert(fvolume>=gccDSBVolumeMin && fvolume<=DSBVOLUME_MAX, "Volume DSB out of range");
	HRESULT hr = pDSB->SetVolume( LONG(fvolume) );
	LogAssert(hr==S_OK, "Couldn't set volume");

}

void DirectSoundAudioBuffer::SetPosition(unsigned long newPosition)
{
    mSample->SetCurrentPosition(newPosition);
}


//
// DirectSoundAudioBuffer::OnRestore		- Chapter 13, page 423
//    NOTE: Renamed from DirectSoundAudioBuffer::VRestore in the book
bool DirectSoundAudioBuffer::OnRestore()
{
   HRESULT hr;
   BOOL    restored;

    // Restore the buffer if it was lost
    if( FAILED( hr = RestoreBuffer( &restored ) ) )
        return NULL;

    if( restored )
    {
        // The buffer was restored, so we need to fill it with new data
        if( FAILED( hr = FillBufferWithSound( ) ) )
            return NULL;
    }

	return true;
}

//
// DirectSoundAudioBuffer::RestoreBuffer			- Chapter 13, page 423
//
//    Restores the lost buffer. *pbWasRestored returns TRUE if the buffer was 
//    restored.  It can also NULL if the information is not needed.
//
HRESULT DirectSoundAudioBuffer::RestoreBuffer( BOOL* pbWasRestored )
{
    HRESULT hr;

    if( ! mSample )
        return CO_E_NOTINITIALIZED;
    if( pbWasRestored )
        *pbWasRestored = FALSE;

    unsigned long dwStatus;
    if( FAILED( hr = mSample->GetStatus( &dwStatus ) ) )
		return hr;

    if( dwStatus & DSBSTATUS_BUFFERLOST )
    {
        // Since the app could have just been activated, then
        // DirectSound may not be giving us control yet, so 
        // the restoring the buffer may fail.  
        // If it does, sleep until DirectSound gives us control but fail if
        // if it goes on for more than 1 second
        int count = 0;
		do 
        {
            hr = mSample->Restore();
            if( hr == DSERR_BUFFERLOST )
                Sleep( 10 );
        }
        while( ( hr = mSample->Restore() ) == DSERR_BUFFERLOST && ++count < 100 );

        if( pbWasRestored != NULL )
            *pbWasRestored = TRUE;

        return S_OK;
    }
    else
    {
        return S_FALSE;
    }
}

//
// DirectSoundAudioBuffer::FillBufferWithSound				- Chapter 13, page 425
//
HRESULT DirectSoundAudioBuffer::FillBufferWithSound( void )
{
    HRESULT hr; 
	int pcmBufferSize = mResource->Size();
	eastl::shared_ptr<SoundResourceExtraData> extra = 
		eastl::static_pointer_cast<SoundResourceExtraData>(mResource->GetExtra());

	VOID* dsLockedBuffer = NULL; // a pointer to the DirectSound buffer
    unsigned long dwDSLockedBufferSize = 0; // Size of the locked DirectSound buffer
    unsigned long dwWavDataRead = 0; // Amount of data read from the wav file 

    if( ! mSample )
        return CO_E_NOTINITIALIZED;

    // Make sure we have focus, and we didn't just switch in from
    // an app which had a DirectSound device
    if( FAILED( hr = RestoreBuffer( NULL ) ) ) 
		return hr;

	
    // Lock the buffer down
    if(FAILED( hr = mSample->Lock(0, pcmBufferSize, 
		&dsLockedBuffer, &dwDSLockedBufferSize, NULL, NULL, 0L)))
		return hr;

    if( pcmBufferSize == 0 )
    {
        // Wav is blank, so just fill with silence
        FillMemory( (BYTE*) dsLockedBuffer, 
                    dwDSLockedBufferSize, 
                    (BYTE)(extra->GetFormat()->wBitsPerSample == 8 ? 128 : 0 ) );
    }
    else 
	{
		CopyMemory(dsLockedBuffer, mResource->Buffer(), pcmBufferSize);
		if( pcmBufferSize < (int)dwDSLockedBufferSize )
		{
            // If the buffer sizes are different fill in the rest with silence 
            FillMemory( (BYTE*) dsLockedBuffer + pcmBufferSize, 
                        dwDSLockedBufferSize - pcmBufferSize, 
                        (BYTE)(extra->GetFormat()->wBitsPerSample == 8 ? 128 : 0 ) );
        }
    }

    // Unlock the buffer, we don't need it anymore.
    mSample->Unlock( dsLockedBuffer, dwDSLockedBufferSize, NULL, 0 );

    return S_OK;
}

//
// DirectSoundAudioBuffer::GetProgress				- Chapter 13, page 426
//
float DirectSoundAudioBuffer::GetProgress()
{
	LPDIRECTSOUNDBUFFER dsb = (LPDIRECTSOUNDBUFFER)Get();	
	unsigned long progress = 0;

	dsb->GetCurrentPosition(&progress, NULL);

	float length = (float)mResource->Size();

	return (float)progress / length;
}