// Copyright (C) 2002-2012 Nikolaus Gebhardt
// This file is part of the "Irrlicht Engine".
// For conditions of distribution and use, see copyright notice in irrlicht.h

#include "GameEngineStd.h"

#include "MeshFileLoader.h"

//! Constructor
MeshFileLoader::MeshFileLoader( )
{
}


//! destructor
MeshFileLoader::~MeshFileLoader()
{
}


// CreateMeshResourceLoader
eastl::shared_ptr<BaseResourceLoader> CreateMeshResourceLoader()
{
	return eastl::shared_ptr<MeshFileLoader>(new MeshFileLoader());
}

// MD3MeshFileLoader::GetLoadedResourceSize
unsigned int MeshFileLoader::GetLoadedResourceSize(void *rawBuffer, unsigned int rawSize)
{
	// This will keep the resource cache from allocating memory for the texture, so DirectX can manage it on it's own.
	return rawSize;
}

//
// MeshFileLoader::LoadResource				- Chapter 14, page 492
//
bool MeshFileLoader::LoadResource(void *rawBuffer, unsigned int rawSize, const eastl::shared_ptr<ResHandle>& handle)
{
	eastl::shared_ptr<MeshResourceExtraData> pExtraData(new MeshResourceExtraData());
	pExtraData->SetMesh( 0 );

	// try to load file based on file extension
	BaseReadFile* file = (BaseReadFile*)rawBuffer;
	if (IsALoadableFileExtension(file->GetFileName()))
	{
		pExtraData->SetMesh( CreateMesh(file) );
		if (pExtraData->GetMesh())
		{
			handle->SetExtra(eastl::shared_ptr<MeshResourceExtraData>(pExtraData));
			return true;
		}
	}

	return false;
}


//! returns true if the file maybe is able to be loaded by this class
//! based on the file extension (e.g. ".bsp")
bool MeshFileLoader::IsALoadableFileExtension(const eastl::wstring& fileName) const
{
	if (fileName.rfind('.') != eastl::string::npos)
	{
		eastl::wstring fileExtension = fileName.substr(fileName.rfind('.') + 1);
		return fileExtension.compare(L"*") == 0;
	}
	else return false;
}


AnimatedMesh* MeshFileLoader::CreateMesh(BaseReadFile* file)
{
	AnimatedMesh * mesh = new AnimatedMesh();
	/*
	if ( mesh->LoadModelFile ( 0, file ) )
		return mesh;
	*/
	delete mesh;
	return nullptr;
}