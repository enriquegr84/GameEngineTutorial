// Copyright (C) 2002-2012 Nikolaus Gebhardt
// This file is part of the "Irrlicht Engine".
// For conditions of distribution and use, see copyright notice in irrlicht.h

#include "GameEngineStd.h"

#ifdef _COMPILE_WITH_MD3_LOADER_

#include "MD3MeshFileLoader.h"
#include "AnimatedMeshMD3.h"


//! Constructor
MD3MeshFileLoader::MD3MeshFileLoader( )
{
}


//! destructor
MD3MeshFileLoader::~MD3MeshFileLoader()
{
}


// CreateMD3MeshResourceLoader
shared_ptr<IResourceLoader> CreateMD3MeshResourceLoader()
{
	return shared_ptr<IResourceLoader>(new MD3MeshFileLoader());
}

// MD3MeshFileLoader::GetLoadedResourceSize
unsigned int MD3MeshFileLoader::GetLoadedResourceSize(void *rawBuffer, unsigned int rawSize)
{
	// This will keep the resource cache from allocating memory for the texture, so DirectX can manage it on it's own.
	return rawSize;
}

//
// MD3MeshFileLoader::LoadResource				- Chapter 14, page 492
//
bool MD3MeshFileLoader::LoadResource(void *rawBuffer, unsigned int rawSize, const shared_ptr<ResHandle>& handle)
{
    shared_ptr<MeshResourceExtraData> pExtraData(new MeshResourceExtraData());
	pExtraData->SetMesh( 0 );

	// try to load file based on file extension
	IReadFile* file = (IReadFile*)rawBuffer;
	if (IsALoadableFileExtension(file->GetFileName()))
	{
		pExtraData->SetMesh( CreateMesh(file) );
		if (pExtraData->GetMesh())
		{
			handle->SetExtra(shared_ptr<MeshResourceExtraData>(pExtraData));
			return true;
		}
	}

	return false;
}


//! returns true if the file maybe is able to be loaded by this class
//! based on the file extension (e.g. ".bsp")
bool MD3MeshFileLoader::IsALoadableFileExtension(const path& filename) const
{
	return Utils::HasFileExtension ( filename, "md3" );
}


IAnimatedMesh* MD3MeshFileLoader::CreateMesh(IReadFile* file)
{
	AnimatedMeshMD3 * mesh = new AnimatedMeshMD3();

	if ( mesh->LoadModelFile ( 0, file ) )
		return mesh;

	SAFE_DELETE ( mesh );
	return 0;
}

#endif // _COMPILE_WITH_MD3_LOADER_
