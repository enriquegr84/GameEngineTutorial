// Copyright (C) 2002-2012 Nikolaus Gebhardt
// This file is part of the "Irrlicht Engine".
// For conditions of distribution and use, see copyright notice in irrlicht.h

#include "MeshCache.h"

#include "IAnimatedMesh.h"
#include "IMesh.h"


static const NamedPath emptyNamedPath;


MeshCache::~MeshCache()
{
	Clear();
}


//! adds a mesh to the list
void MeshCache::AddMesh(const path& filename, const shared_ptr<IAnimatedMesh>& mesh)
{
	MeshEntry e ( filename );
	e.m_Mesh = mesh;
	m_Meshes.push_back(e);
}


//! Removes a mesh from the cache.
void MeshCache::RemoveMesh(const shared_ptr<IMesh>& mesh)
{
	if ( !mesh )
		return;

	eastl::vector<MeshEntry>::iterator itMesh = m_Meshes.begin();
	for (; itMesh < m_Meshes.end(); itMesh++)
	{
		if (((*itMesh).m_Mesh == mesh) ||  
			((*itMesh).m_Mesh && (*itMesh).m_Mesh->GetMesh(0) == mesh))
		{
			m_Meshes.erase(itMesh);
			return;
		}
	}
}


//! Returns amount of loaded meshes
unsigned int MeshCache::GetMeshCount() const
{
	return m_Meshes.size();
}


//! Returns current number of the mesh
int MeshCache::GetMeshIndex(const shared_ptr<IMesh>& mesh) const
{

	for (unsigned int i=0; i<m_Meshes.size(); ++i)
	{
		if ((m_Meshes[i].m_Mesh && 
			m_Meshes[i].m_Mesh->GetMesh(0) == mesh || 
			m_Meshes[i].m_Mesh == mesh))
				return (int)i;
	}

	return -1;
}


//! Returns a mesh based on its index number
shared_ptr<IAnimatedMesh> MeshCache::GetMeshByIndex(unsigned int number)
{
	if (number >= m_Meshes.size())
		return 0;

	return m_Meshes[number].m_Mesh;
}


//! Returns a mesh based on its name.
shared_ptr<IAnimatedMesh> MeshCache::GetMeshByName(const path& name)
{
	eastl::vector<MeshEntry>::iterator itMesh = m_Meshes.begin();
	for (; itMesh < m_Meshes.end(); itMesh++)
	{
		if (stringc((*itMesh).m_MeshNamedPath) == stringc(name))
			return (*itMesh).m_Mesh;
	}

	return 0;
}


//! Get the name of a loaded mesh, based on its index.
const NamedPath& MeshCache::GetMeshName(unsigned int index) const
{
	if (index >= m_Meshes.size())
		return emptyNamedPath;

	return m_Meshes[index].m_MeshNamedPath;
}


//! Get the name of a loaded mesh, if there is any.
const NamedPath& MeshCache::GetMeshName(const shared_ptr<IMesh>& mesh) const
{
	if (!mesh)
		return emptyNamedPath;

	eastl::vector<MeshEntry>::const_iterator itMesh = m_Meshes.begin();
	for (; itMesh < m_Meshes.end(); itMesh++)
	{
		if (((*itMesh).m_Mesh == mesh) ||  
			((*itMesh).m_Mesh && (*itMesh).m_Mesh->GetMesh(0) == mesh))
			return (*itMesh).m_MeshNamedPath;
	}

	return emptyNamedPath;
}

//! Renames a loaded mesh.
bool MeshCache::RenameMesh(unsigned int index, const path& name)
{
	if (index >= m_Meshes.size())
		return false;

	m_Meshes[index].m_MeshNamedPath.SetPath(name);
	return true;
}


//! Renames a loaded mesh.
bool MeshCache::RenameMesh(const shared_ptr<IMesh>& mesh, const path& name)
{
	eastl::vector<MeshEntry>::iterator itMesh = m_Meshes.begin();
	for (; itMesh < m_Meshes.end(); itMesh++)
	{
		if (((*itMesh).m_Mesh == mesh) ||  
			((*itMesh).m_Mesh && 
			(*itMesh).m_Mesh->GetMesh(0) == mesh))
		{
			(*itMesh).m_MeshNamedPath.SetPath(name);
			return true;
		}
	}

	return false;
}


//! returns if a mesh already was loaded
bool MeshCache::IsMeshLoaded(const path& name)
{
	return GetMeshByName(name) != 0;
}


//! Clears the whole mesh cache, removing all meshes.
void MeshCache::Clear()
{
	m_Meshes.clear();
}

//! Clears all meshes that are held in the mesh cache but not used anywhere else.
void MeshCache::ClearUnusedMeshes()
{
	eastl::vector<MeshEntry>::iterator itMesh = m_Meshes.begin();
	for (; itMesh < m_Meshes.end(); itMesh++)
	{
		if ((*itMesh).m_Mesh.use_count() == 1)
		{
			m_Meshes.erase(itMesh);
			--itMesh;
		}
	}
}
