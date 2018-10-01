// Copyright (C) 2002-2012 Nikolaus Gebhardt
// This file is part of the "Irrlicht Engine".
// For conditions of distribution and use, see copyright notice in irrlicht.h

#include "MeshCache.h"
#include "Mesh.h"

MeshCache::~MeshCache()
{
	Clear();
}


//! adds a mesh to the list
void MeshCache::AddMesh(const eastl::string& filename, const eastl::shared_ptr<BaseAnimatedMesh>& mesh)
{
	MeshEntry e ( filename );
	e.mMesh = mesh;
	mMeshes.push_back(e);
}


//! Removes a mesh from the cache.
void MeshCache::RemoveMesh(const eastl::shared_ptr<BaseMesh>& mesh)
{
	if ( !mesh )
		return;

	eastl::vector<MeshEntry>::iterator itMesh = mMeshes.begin();
	for (; itMesh < mMeshes.end(); itMesh++)
	{
		if (((*itMesh).mMesh == mesh) ||  
			((*itMesh).mMesh && (*itMesh).mMesh->GetMesh(0) == mesh))
		{
			mMeshes.erase(itMesh);
			return;
		}
	}
}


//! Returns amount of loaded meshes
unsigned int MeshCache::GetMeshCount() const
{
	return mMeshes.size();
}


//! Returns current number of the mesh
int MeshCache::GetMeshIndex(const eastl::shared_ptr<BaseMesh>& mesh) const
{

	for (unsigned int i=0; i<mMeshes.size(); ++i)
	{
		if ((mMeshes[i].mMesh &&
			mMeshes[i].mMesh->GetMesh(0) == mesh ||
			mMeshes[i].mMesh == mesh))
		{
			return (int)i;
		}
	}

	return -1;
}


//! Returns a mesh based on its index number
eastl::shared_ptr<BaseAnimatedMesh> MeshCache::GetMeshByIndex(unsigned int number)
{
	if (number >= mMeshes.size())
		return nullptr;

	return mMeshes[number].mMesh;
}


//! Returns a mesh based on its name.
eastl::shared_ptr<BaseAnimatedMesh> MeshCache::GetMeshByName(const eastl::string& name)
{
	eastl::vector<MeshEntry>::iterator itMesh = mMeshes.begin();
	for (; itMesh < mMeshes.end(); itMesh++)
	{
		if ((*itMesh).mMeshNamedPath == name)
			return (*itMesh).mMesh;
	}

	return nullptr;
}


//! Get the name of a loaded mesh, based on its index.
const eastl::string& MeshCache::GetMeshName(unsigned int index) const
{
	if (index >= mMeshes.size())
		return nullptr;

	return mMeshes[index].mMeshNamedPath;
}


//! Get the name of a loaded mesh, if there is any.
const eastl::string& MeshCache::GetMeshName(const eastl::shared_ptr<BaseMesh>& mesh) const
{
	if (!mesh)
		return nullptr;

	eastl::vector<MeshEntry>::const_iterator itMesh = mMeshes.begin();
	for (; itMesh < mMeshes.end(); itMesh++)
	{
		if (((*itMesh).mMesh == mesh) ||  
			((*itMesh).mMesh && (*itMesh).mMesh->GetMesh(0) == mesh))
			return (*itMesh).mMeshNamedPath;
	}

	return nullptr;
}

//! Renames a loaded mesh.
bool MeshCache::RenameMesh(unsigned int index, const eastl::string& name)
{
	if (index >= mMeshes.size())
		return false;

	mMeshes[index].mMeshNamedPath = name;
	return true;
}


//! Renames a loaded mesh.
bool MeshCache::RenameMesh(const eastl::shared_ptr<BaseMesh>& mesh, const eastl::string& name)
{
	eastl::vector<MeshEntry>::iterator itMesh = mMeshes.begin();
	for (; itMesh < mMeshes.end(); itMesh++)
	{
		if (((*itMesh).mMesh == mesh) ||  
			((*itMesh).mMesh && (*itMesh).mMesh->GetMesh(0) == mesh))
		{
			(*itMesh).mMeshNamedPath = name;
			return true;
		}
	}

	return false;
}


//! returns if a mesh already was loaded
bool MeshCache::IsMeshLoaded(const eastl::string& name)
{
	return GetMeshByName(name) != 0;
}


//! Clears the whole mesh cache, removing all meshes.
void MeshCache::Clear()
{
	mMeshes.clear();
}

//! Clears all meshes that are held in the mesh cache but not used anywhere else.
void MeshCache::ClearUnusedMeshes()
{
	eastl::vector<MeshEntry>::iterator itMesh = mMeshes.begin();
	for (; itMesh < mMeshes.end(); itMesh++)
	{
		if ((*itMesh).mMesh.use_count() == 1)
		{
			mMeshes.erase(itMesh);
			--itMesh;
		}
	}
}
