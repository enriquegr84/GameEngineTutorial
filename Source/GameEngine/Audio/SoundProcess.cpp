//========================================================================
// SoundProcess.cpp : Defines sound processes.
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
#include "SoundProcess.h"
#include "SoundResource.h"

//////////////////////////////////////////////////////////////////////
// SoundProcess Implementation
//////////////////////////////////////////////////////////////////////

//
// SoundProcess::SoundProcess				- Chapter 13, page 428
//
SoundProcess::SoundProcess(eastl::shared_ptr<ResHandle> resource, int volume, bool looping) :
	mHandle(resource),
	mVolume(volume),
	mIsLooping(looping)
{
	InitializeVolume();
}


//
// SoundProcess::~SoundProcess				- Chapter 13, page 428
//
SoundProcess::~SoundProcess()
{
    if (IsPlaying())
        Stop();

	if (mAudioBuffer)
		Audio::AudioSystem->ReleaseAudioBuffer(mAudioBuffer.get());
}


void SoundProcess::InitializeVolume()
{
	// FUTURE WORK: Somewhere set an adjusted volume based on game options
	// m_volume = g_GraphicalApp->GetVolume(typeOfSound);
}

//
// SoundProcess::GetLengthMilli					- Chapter 13, page 430
//
int SoundProcess::GetLengthMilli()
{
	if ( mHandle && mHandle->GetExtra())
	{
		eastl::shared_ptr<SoundResourceExtraData> extra = 
			eastl::static_pointer_cast<SoundResourceExtraData>(mHandle->GetExtra());
		return extra->GetLength();
	}
	else
    {
		return 0;
    }
}

/*
	SoundProcess::OnInit
	One important concept to understand about sounds is that the code might create a sound 
	process long before the sound should be played or even loaded. Since sound effects tend 
	to require a lot of data, it’s a good idea to be careful about when you instantiate sound
	effects. After all, you don’t want the game to stutter or suffer wacky pauses.
*/
void SoundProcess::OnInit()
{
    Process::OnInit();

	//If the sound has never been... you know... then Play it for the very first time
	if ( mHandle == NULL || mHandle->GetExtra() == NULL)
		return;

	//This sound will manage it's own handle in the other thread
	BaseAudioBuffer *buffer = Audio::AudioSystem->InitAudioBuffer(mHandle);

	if (!buffer)
	{
        Fail();
		return;
	}

	mAudioBuffer.reset(buffer);	

	Play(mVolume, mIsLooping);
}

/*
	SoundProcess::OnUpdate
	The OnUpdate method monitors the sound effect as it’s being played. Once it is
	finished, it kills the process and releases the audio buffer. If the sound is 
	looping, it will play until some external call kills the process.
*/
void SoundProcess::OnUpdate(unsigned long deltaMs)
{
    if (!IsPlaying())
    {
        Succeed();
    }
}

//
// SoundProcess::IsPlaying						- Chapter 13, page 429
//
bool SoundProcess::IsPlaying()
{
	if ( ! mHandle || ! mAudioBuffer )
		return false;
	
	return mAudioBuffer->IsPlaying();
}

//
// SoundProcess::SetVolume						- Chapter 13, page 430
//
void SoundProcess::SetVolume(int volume)
{
	if(mAudioBuffer==NULL)
	{
		return;
	}

	LogAssert(volume>=0 && volume<=100, "Volume must be a number between 0 and 100");
	mVolume = volume;
	mAudioBuffer->SetVolume(volume);
}

//
// SoundProcess::GetVolume						- Chapter 13, page 430
//
int SoundProcess::GetVolume()
{
	if(mAudioBuffer==NULL)
	{
		return 0;
	}

	mVolume = mAudioBuffer->GetVolume();
	return mVolume;
}

//
// SoundProcess::PauseSound						- Chapter 13, page 430
//   NOTE: This is called TogglePause in te book
//
void SoundProcess::PauseSound()
{
	if (mAudioBuffer)
		mAudioBuffer->TogglePause();
}

//
// SoundProcess::Play							- Chapter 13, page 431
//
void SoundProcess::Play(const int volume, const bool looping)
{
	LogAssert(volume>=0 && volume<=100, "Volume must be a number between 0 and 100");

	if(!mAudioBuffer)
	{
		return;
	}
	
	mAudioBuffer->Play(volume, looping);
}

//
// SoundProcess::Stop							- Chapter 13, page 431
//
void SoundProcess::Stop()
{
	if (mAudioBuffer)
	{
		mAudioBuffer->Stop();
	}
}

//
// SoundProcess::GetProgress					- Chapter 13, page 431
//
float SoundProcess::GetProgress()
{
	if (mAudioBuffer)
	{
		return mAudioBuffer->GetProgress();
	}

	return 0.0f;
}

//
// ExplosionProcess::OnInit					- Chapter 13, page 433
//
void ExplosionProcess::OnInit()
{
	Process::OnInit();

	BaseResource resource(L"explosion.wav");

	eastl::shared_ptr<ResHandle> srh = ResCache::Get()->GetHandle(&resource);
	mSound.reset(new SoundProcess(srh));

	// Imagine cool explosion graphics setup code here!!!!
	//
	//
	//
}

//
// ExplosionProcess::OnUpdate					- Chapter 13, page 433
//
void ExplosionProcess::OnUpdate(unsigned long deltaMs)
{
	// Since the sound is the real pacing mechanism - we ignore deltaMilliseconds
	float progress = mSound->GetProgress();
	
	switch (mStage)
	{
		case 0:
        {
			if (progress > 0.55f)
			{
				++mStage;
				// Imagine secondary explosion effect launch right here!
			}
			break;
        }

		case 1:
        {
			if (progress > 0.75f)
			{
				++mStage;
				// Imagine tertiary explosion effect launch right here!
			}
			break;
        }

		default:
        {
			break;
        }
	}
}

/////////////////////////////////////////////////////////////////////////////
// 
// FadeProcess Implementation 
//
//////////////////////////////////////////////////////////////////////

//
// FadeProcess::FadeProcess						- Chapter 13, page 435
//
FadeProcess::FadeProcess(eastl::shared_ptr<SoundProcess> sound, int fadeTime, int endVolume)
{
	mSound = sound;
	mTotalFadeTime = fadeTime;
	mStartVolume = sound->GetVolume();
	mEndVolume = endVolume;
	mElapsedTime = 0;

	OnUpdate(0);
}

//
// FadeProcess::OnUpdate						- Chapter 13, page 435
//
void FadeProcess::OnUpdate(unsigned long deltaMs)
{
    mElapsedTime += deltaMs;

	if (mSound->IsDead())
		Succeed();

	float cooef = (float)mElapsedTime / mTotalFadeTime;
	if (cooef>1.0f)
		cooef = 1.0f;
	if (cooef<0.0f)
		cooef = 0.0f;

	int newVolume = mStartVolume + (int)( float(mEndVolume - mStartVolume) * cooef);

	if (mElapsedTime >= mTotalFadeTime)
	{
		newVolume = mEndVolume;
		Succeed();
	}

	mSound->SetVolume(newVolume);
}