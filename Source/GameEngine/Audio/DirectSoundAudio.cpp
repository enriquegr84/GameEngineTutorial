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


#include "GameEngine/GameEngine.h"
#include "SoundResource.h"
#include "DirectSoundAudio.h"

#include <cguid.h>

//////////////////////////////////////////////////////////////////////
// 
// DirectSoundAudio::io Implementation
//
//////////////////////////////////////////////////////////////////////


//
// DirectSoundAudio::Initialize			- Chapter 13, page 415
//
bool DirectSoundAudio::Initialize(void* id)
{
	if(m_Initialized)
		return true;

	m_Initialized=false;
	m_AllSamples.clear();

	SAFE_RELEASE( m_pDS );

	HRESULT hr;

	HWND hWnd = static_cast<HWND>(id);

	// Create IDirectSound using the primary sound device
	if( FAILED( hr = DirectSoundCreate8( NULL, &m_pDS, NULL ) ) )
		return false;

	// Set DirectSound coop level 
	if( FAILED( hr = m_pDS->SetCooperativeLevel( hWnd, DSSCL_PRIORITY) ) )
		return false;

	if( FAILED( hr = SetPrimaryBufferFormat( 8, 44100, 16 ) ) )
		return false;

	m_Initialized = true;

	return true;
}

//
// DirectSoundAudio::SetPrimaryBufferFormat		- Chapter 13, page 416
//
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

	HRESULT             hr;
	LPDIRECTSOUNDBUFFER pDSBPrimary = NULL;

	if( ! m_pDS )
		return CO_E_NOTINITIALIZED;

	// Get the primary buffer 
	DSBUFFERDESC dsbd;
	ZeroMemory( &dsbd, sizeof(DSBUFFERDESC) );
	dsbd.dwSize        = sizeof(DSBUFFERDESC);
	dsbd.dwFlags       = DSBCAPS_PRIMARYBUFFER;
	dsbd.dwBufferBytes = 0;
	dsbd.lpwfxFormat   = NULL;

	if( FAILED( hr = m_pDS->CreateSoundBuffer( &dsbd, &pDSBPrimary, NULL ) ) )
		return S_OK;
		//return DXUT_ERR( L"CreateSoundBuffer", hr );

	WAVEFORMATEX wfx;
	ZeroMemory( &wfx, sizeof(WAVEFORMATEX) ); 
	wfx.wFormatTag      = (WORD) WAVE_FORMAT_PCM; 
	wfx.nChannels       = (WORD) dwPrimaryChannels; 
	wfx.nSamplesPerSec  = (unsigned long) dwPrimaryFreq; 
	wfx.wBitsPerSample  = (WORD) dwPrimaryBitRate; 
	wfx.nBlockAlign     = (WORD) (wfx.wBitsPerSample / 8 * wfx.nChannels);
	wfx.nAvgBytesPerSec = (unsigned long) (wfx.nSamplesPerSec * wfx.nBlockAlign);

	if( FAILED( hr = pDSBPrimary->SetFormat(&wfx) ) )
		return S_OK;
		//return DXUT_ERR( L"SetFormat", hr );

	SAFE_RELEASE( pDSBPrimary );

	return S_OK;
}


//
// DirectSoundAudio::Shutdown					- Chapter 13, page 417
//
void DirectSoundAudio::Shutdown()
{
	if(m_Initialized)
	{
		Audio::Shutdown();
		SAFE_RELEASE(m_pDS);
		m_Initialized = false;
	}
}


//
// DirectSoundAudio::InitAudioBuffer			- Chapter 13, page 418
//   Allocate a sample handle for the newborn sound (used by SoundResource) and tell you it's length
//
IAudioBuffer *DirectSoundAudio::InitAudioBuffer(shared_ptr<ResHandle> resHandle)//const
{
	shared_ptr<SoundResourceExtraData> extra = 
		static_pointer_cast<SoundResourceExtraData>(resHandle->GetExtra());

    if( ! m_pDS )
        return NULL;

	switch(extra->GetSoundType())
	{
		case SOUND_TYPE_OGG:
		case SOUND_TYPE_WAVE:
			// We support WAVs and OGGs
			break;

		case SOUND_TYPE_MP3:
		case SOUND_TYPE_MIDI:	//If it's a midi file, then do nothin at this time... maybe we will support this in the future
			GE_ASSERT(false && "MP3s and MIDI are not supported");
			return NULL;
			break;

		default:
			GE_ASSERT(false && "Unknown sound type");
			return NULL;
	}//end switch

    LPDIRECTSOUNDBUFFER sampleHandle;

    // Create the direct sound buffer, and only request the flags needed
    // since each requires some overhead and limits if the buffer can 
    // be hardware accelerated
    DSBUFFERDESC dsbd;
    ZeroMemory( &dsbd, sizeof(DSBUFFERDESC) );
    dsbd.dwSize          = sizeof(DSBUFFERDESC);
    dsbd.dwFlags         = DSBCAPS_CTRLVOLUME;
    dsbd.dwBufferBytes   = resHandle->Size();
    dsbd.guid3DAlgorithm = GUID_NULL;
    dsbd.lpwfxFormat     = const_cast<WAVEFORMATEX *>(extra->GetFormat());

	HRESULT hr;
    if( FAILED( hr = m_pDS->CreateSoundBuffer( &dsbd, &sampleHandle, NULL ) ) )
    {
        return NULL;
    }

	// Add handle to the list
	IAudioBuffer *audioBuffer = new DirectSoundAudioBuffer(sampleHandle, resHandle);
	m_AllSamples.push_front( audioBuffer);
	
	return audioBuffer;

}

//
// DirectSoundAudio::ReleaseAudioBuffer			- Chapter 13, page 419
//    Allocate a sample handle for the newborn sound (used by SoundResource)
//
void DirectSoundAudio::ReleaseAudioBuffer(IAudioBuffer *sampleHandle)//const
{
	sampleHandle->Stop();
	m_AllSamples.remove(sampleHandle);
}

//
// DirectSoundAudioBuffer::DirectSoundAudioBuffer	- Chapter 13, page 420
//
DirectSoundAudioBuffer::DirectSoundAudioBuffer(
	LPDIRECTSOUNDBUFFER sample, shared_ptr<ResHandle> resource)
 : AudioBuffer(resource) 
{ 
	m_Sample = sample; 
	FillBufferWithSound();
}

//
// DirectSoundAudioBuffer::Get						- Chapter 13, page 420
//
void *DirectSoundAudioBuffer::Get()
{
 	if (!OnRestore())
		return NULL;

	return m_Sample;
}


//
// DirectSoundAudioioBuffer::Play					- Chapter 13, page 421
//    Play a sound
//
bool DirectSoundAudioBuffer::Play(int volume, bool looping)
{
   if(!g_pAudio->Active())
      return false;

	Stop();

	m_Volume = volume;
	m_isLooping = looping;

	LPDIRECTSOUNDBUFFER pDSB = (LPDIRECTSOUNDBUFFER)Get();
	if (!pDSB)
		return false;

    pDSB->SetVolume( volume );
    
    unsigned long dwFlags = looping ? DSBPLAY_LOOPING : 0L;

    return (S_OK==pDSB->Play( 0, 0, dwFlags ) );

}//end Play


//
// DirectSoundAudioBuffer::Stop					- Chapter 13, page 421
//    Stop a sound and rewind play position to the beginning.
//
bool DirectSoundAudioBuffer::Stop()
{
	if(!g_pAudio->Active())
		return false;

	LPDIRECTSOUNDBUFFER pDSB = (LPDIRECTSOUNDBUFFER)Get();

   if( ! pDSB )
		return false;

	m_isPaused=true;
    pDSB->Stop();
	return true;
}


//
// DirectSoundAudioBuffer::Pause					- Chapter X, page Y
//    Pause a sound 
//
bool DirectSoundAudioBuffer::Pause()
{
	LPDIRECTSOUNDBUFFER pDSB = (LPDIRECTSOUNDBUFFER)Get();

	if(!g_pAudio->Active())
		return false;

   if( pDSB )
		return false;

	m_isPaused=true;
    pDSB->Stop();
	pDSB->SetCurrentPosition(0);	// rewinds buffer to beginning.
	return true;
}

//
// DirectSoundAudioBuffer::Resume					- Chapter 13, page 421
//    Resume a sound
bool DirectSoundAudioBuffer::Resume()
{
	m_isPaused=false;
	return Play(GetVolume(), IsLooping());
}

//
// DirectSoundAudioBuffer::TogglePause				- Chapter 13, page 421
//    Pause a sound or Resume a Paused sound
//
bool DirectSoundAudioBuffer::TogglePause()
{
	if(!g_pAudio->Active())
		return false;

	if(m_isPaused)
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
	if(!g_pAudio->Active())
		return false;

	unsigned long dwStatus = 0;
	LPDIRECTSOUNDBUFFER pDSB = (LPDIRECTSOUNDBUFFER)Get();
	pDSB->GetStatus( &dwStatus );
    bool bIsPlaying = ( ( dwStatus & DSBSTATUS_PLAYING ) != 0 );

	return bIsPlaying;
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

	if(!g_pAudio->Active())
		return;

	LPDIRECTSOUNDBUFFER pDSB = (LPDIRECTSOUNDBUFFER)Get();

	GE_ASSERT(volume>=0 && volume<=100 && "Volume must be a number between 0 and 100");

	// convert volume from 0-100 into range for DirectX - don't forget to use a logarithmic scale!

	float coeff = (float)volume / 100.0f;
	float logarithmicProportion = coeff >0.1f  ? 1+log10(coeff)  : 0;
	float range = float(DSBVOLUME_MAX - gccDSBVolumeMin);
	float fvolume = ( range * logarithmicProportion ) + gccDSBVolumeMin;

	GE_ASSERT(fvolume>=gccDSBVolumeMin && fvolume<=DSBVOLUME_MAX);
	HRESULT hr = pDSB->SetVolume( LONG(fvolume) );
	GE_ASSERT(hr==S_OK);

}

void DirectSoundAudioBuffer::SetPosition(unsigned long newPosition)
{
    m_Sample->SetCurrentPosition(newPosition);
}


//
// DirectSoundAudioBuffer::OnRestore		- Chapter 13, page 423
//    NOTE: Renamed from DirectSoundAudioBuffer::VRestore in the book
bool DirectSoundAudioBuffer::OnRestore()
{
   HRESULT hr;
   BOOL    bRestored;

    // Restore the buffer if it was lost
    if( FAILED( hr = RestoreBuffer( &bRestored ) ) )
        return NULL;

    if( bRestored )
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

    if( ! m_Sample )
        return CO_E_NOTINITIALIZED;
    if( pbWasRestored )
        *pbWasRestored = FALSE;

    unsigned long dwStatus;
    if( FAILED( hr = m_Sample->GetStatus( &dwStatus ) ) )
		return S_OK;
        //return DXUT_ERR( L"GetStatus", hr );

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
            hr = m_Sample->Restore();
            if( hr == DSERR_BUFFERLOST )
                Sleep( 10 );
        }
        while( ( hr = m_Sample->Restore() ) == DSERR_BUFFERLOST && ++count < 100 );

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
	int pcmBufferSize = m_Resource->Size();
	shared_ptr<SoundResourceExtraData> extra = static_pointer_cast<SoundResourceExtraData>(m_Resource->GetExtra());

	VOID*	pDSLockedBuffer = NULL;							// a pointer to the DirectSound buffer
    unsigned long   dwDSLockedBufferSize = 0;				// Size of the locked DirectSound buffer
    unsigned long   dwWavDataRead        = 0;				// Amount of data read from the wav file 

    if( ! m_Sample )
        return CO_E_NOTINITIALIZED;

    // Make sure we have focus, and we didn't just switch in from
    // an app which had a DirectSound device
    if( FAILED( hr = RestoreBuffer( NULL ) ) ) 
		return S_OK;
        //return DXUT_ERR( L"RestoreBuffer", hr );

	
    // Lock the buffer down
    if( FAILED( hr = m_Sample->Lock( 0, pcmBufferSize, 
                                 &pDSLockedBuffer, &dwDSLockedBufferSize, 
                                 NULL, NULL, 0L ) ) )
		return S_OK;
        //return DXUT_ERR( L"Lock", hr );

    if( pcmBufferSize == 0 )
    {
        // Wav is blank, so just fill with silence
        FillMemory( (BYTE*) pDSLockedBuffer, 
                    dwDSLockedBufferSize, 
                    (BYTE)(extra->GetFormat()->wBitsPerSample == 8 ? 128 : 0 ) );
    }
    else 
	{
		CopyMemory(pDSLockedBuffer, m_Resource->Buffer(), pcmBufferSize);
		if( pcmBufferSize < (int)dwDSLockedBufferSize )
		{
            // If the buffer sizes are different fill in the rest with silence 
            FillMemory( (BYTE*) pDSLockedBuffer + pcmBufferSize, 
                        dwDSLockedBufferSize - pcmBufferSize, 
                        (BYTE)(extra->GetFormat()->wBitsPerSample == 8 ? 128 : 0 ) );
        }
    }

    // Unlock the buffer, we don't need it anymore.
    m_Sample->Unlock( pDSLockedBuffer, dwDSLockedBufferSize, NULL, 0 );

    return S_OK;
}




//
// DirectSoundAudioBuffer::GetProgress				- Chapter 13, page 426
//
float DirectSoundAudioBuffer::GetProgress()
{
	LPDIRECTSOUNDBUFFER pDSB = (LPDIRECTSOUNDBUFFER)Get();	
	unsigned long progress = 0;

	pDSB->GetCurrentPosition(&progress, NULL);

	float length = (float)m_Resource->Size();

	return (float)progress / length;
}