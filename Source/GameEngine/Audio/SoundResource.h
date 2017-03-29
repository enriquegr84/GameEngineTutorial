/////////////////////////////////////////////////////////////////////////////
// File SoundResource.h
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

#ifndef SOUNDRESOURCE_H
#define SOUNDRESOURCE_H

#include "GameEngineStd.h"

#include "Core/IO/ResourceCache.h"

#include <mmsystem.h>

/*
	A Resource encapsulates sound data, presumably loaded from a file or resource cache.
	Sound resources are loaded exactly the same as other game resources; they will likely
	exist in a resource file. A resource cache is convenient if you have many simultaneous 
	sounds that use the same sound data. Once it is loaded, taking up only one block of
	memory, then the sound driver can create many "players" that will use the same resource.

	The sound system uses the resource cache to load the sound data from a resource file,
	decompresses it if necessary, and manages DirectSound audio buffers if the same sound
	is being played multiple times.
*/

/*
	SoundResourceExtraData class stores data that will be used by DirectSound. It is 
	initialized when the resource cache loads the sound. The member data describes
	sound properties such as, supported sound type, format (channels, sample rate, 
	bits per data, etc..), length of the sound.
	Any game would keep compressed sounds in memory and send bits and pieces of them
	into the audio hardware as they were needed, saving memory space. For longer pieces
	such as music, the system might even stream bits of the compressed music and then
	uncompress those bits as they were consumed by the audio card.
*/
class SoundResourceExtraData : public BaseResourceExtraData
{
	friend class WaveResourceLoader;
	friend class OggResourceLoader;

public: 	
	SoundResourceExtraData();
	virtual ~SoundResourceExtraData() { }
	virtual eastl::wstring ToString() { return L"SoundResourceExtraData"; }

	enum SoundType GetSoundType() { return mSoundType; }
	WAVEFORMATEX const *GetFormat() { return &mWavFormatEx; }
	int GetLength() const { return mLength; }

protected:
	enum SoundType mSoundType; // is this an Ogg, WAV, etc.?
	bool mIsInitialized; // has the sound been initialized
	WAVEFORMATEX mWavFormatEx; // description of the PCM format
	int mLength; // how long the sound is in milliseconds
};

/*
	The resource cache will use implementations of the BaseResourceLoader interface to 
	determine what kind of resource the sound is and the size of the loaded resource and
	to actually load the resource into the memory the resource cache allocates.
*/

/*
	WaveResourceLoader loads a wav format sound into the memory allocated by the cache.
	Wav format files are a chunky file structure and stores raw sound data. Each chunk is 
	preceded by a unique identifier, which you will use to parse the data in each chunk. 
	The chunks can also be hierarchical; that is, a chunk can exist within another chunk. 
	ParseWave() method loads a WAV stream already in memory.
*/
class WaveResourceLoader : public BaseResourceLoader
{
public:
	virtual bool UseRawFile() { return true; }
	virtual bool DiscardRawBufferAfterLoad() { return true; }
	virtual unsigned int GetLoadedResourceSize(void *rawBuffer, unsigned int rawSize);
	virtual bool LoadResource(
		void *rawBuffer, unsigned int rawSize, const eastl::shared_ptr<ResHandle>& handle);
	virtual bool MatchResourceFormat(eastl::wstring name) 
	{
		if (name.rfind('.') != eastl::string::npos)
			return name.substr(name.rfind('.') + 1) == eastl::wstring("wav");
		else
			return false;
	}

protected:
	bool ParseWave(char *wavStream, size_t length, eastl::shared_ptr<ResHandle> handle);
};

/*
	OggResourceLoader loads a ogg format sound into the memory allocated by the cache.
	OGG (and MP3) files are compressed sound file formats which can achieve certain compression
	ratio with only a barely perceptible loss in sound quality.
	ParseOgg() method decompresses an OGG memory buffer using the Vorbis API. The method will
	decompress the OGG stream into a PCM buffer.
*/
class OggResourceLoader : public BaseResourceLoader
{
public:
	virtual bool UseRawFile() { return true; }
	virtual bool DiscardRawBufferAfterLoad() { return true; }
	virtual unsigned int GetLoadedResourceSize(void *rawBuffer, unsigned int rawSize);
	virtual bool LoadResource(
		void *rawBuffer, unsigned int rawSize, const eastl::shared_ptr<ResHandle>& handle);
	virtual bool MatchResourceFormat(eastl::wstring name) 
	{
		if (name.rfind('.') != eastl::string::npos)
			return name.substr(name.rfind('.') + 1) == eastl::wstring("ogg");
		else
			return false;
	}

protected:
	bool ParseOgg(char *oggStream, size_t length, eastl::shared_ptr<ResHandle> handle);
};

#endif