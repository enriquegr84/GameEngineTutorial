// Copyright (C) 2002-2012 Nikolaus Gebhardt
// This file is part of the "Irrlicht Engine".
// For conditions of distribution and use, see copyright notice in irrlicht.h

#ifndef MESHCACHE_H
#define MESHCACHE_H

#include "GameEngineStd.h"

class BaseMesh;
class BaseMeshLoader;
class BaseAnimatedMesh;


//! The mesh cache stores already loaded meshes and provides an interface to them.
/** You can access it using ISceneManager::GetMeshCache(). All existing
scene managers will return a pointer to the same mesh cache, because it
is shared between them. With this interface, it is possible to manually
add new loaded meshes (if ISceneManager::GetMesh() is not sufficient),
to remove them and to iterate through already loaded meshes. */
class BaseMeshCache
{
public:

	//! Destructor
	virtual ~BaseMeshCache() {}

	//! Adds a mesh to the internal list of loaded meshes.
	/** Usually, ISceneManager::GetMesh() is called to load a mesh
	from a file. That method searches the list of loaded meshes if
	a mesh has already been loaded and returns a pointer to if it
	is in that list and already in memory. Otherwise it loads the
	mesh. With IMeshCache::addMesh(), it is possible to pretend
	that a mesh already has been loaded. This method can be used
	for example by mesh loaders who need to load more than one mesh
	with one call. They can add additional meshes with this method
	to the scene manager. The COLLADA loader for example uses this
	method.
	\param name Name of the mesh. When calling
	ISceneManager::getMesh() with this name it will return the mesh
	set by this method.
	\param mesh Pointer to a mesh which will now be referenced by
	this name. */
	virtual void AddMesh(const eastl::string& name,
		const eastl::shared_ptr<BaseAnimatedMesh>& mesh) = 0;

	//! Removes the mesh from the cache.
	/** After loading a mesh with getMesh(), the mesh can be
	removed from the cache using this method, freeing a lot of
	memory.
	\param mesh Pointer to the mesh which shall be removed. */
	virtual void RemoveMesh(const eastl::shared_ptr<BaseMesh>& mesh) = 0;

	//! Returns amount of loaded meshes in the cache.
	/** You can load new meshes into the cache using getMesh() and
	addMesh(). If you ever need to access the internal mesh cache,
	you can do this using removeMesh(), getMeshNumber(),
	getMeshByIndex() and getMeshName().
	\return Number of meshes in cache. */
	virtual unsigned int GetMeshCount() const = 0;

	//! Returns current index number of the mesh or -1 when not found.
	/** \param mesh Pointer to the mesh to search for.
	\return Index of the mesh in the cache, or -1 if not found. */
	virtual int GetMeshIndex(const eastl::shared_ptr<BaseMesh>& mesh) const = 0;

	//! Returns a mesh based on its index number.
	/** \param index: Index of the mesh, number between 0 and
	getMeshCount()-1.
	Note that this number is only valid until a new mesh is loaded
	or removed.
	\return Pointer to the mesh or 0 if there is none with this
	number. */
	virtual eastl::shared_ptr<BaseAnimatedMesh> GetMeshByIndex(unsigned int index) = 0;

	//! Returns a mesh based on its name.
	/** \param name Name of the mesh. Usually a filename.
	\return Pointer to the mesh or 0 if there is none with this number. */
	virtual eastl::shared_ptr<BaseAnimatedMesh> GetMeshByName(const eastl::string& name) = 0;

	//! Get the name of a loaded mesh, based on its index.
	/** \param index: Index of the mesh, number between 0 and getMeshCount()-1.
	\return The name if mesh was found and has a name, else the path is empty. */
	virtual const eastl::string& GetMeshName(unsigned int index) const = 0;

	//! Get the name of the loaded mesh if there is any.
	/** \param mesh Pointer to mesh to query.
	\return The name if mesh was found and has a name, else the path is empty. */
	virtual const eastl::string& GetMeshName(const eastl::shared_ptr<BaseMesh>& mesh) const = 0;

	//! Renames a loaded mesh.
	/** Note that renaming meshes might change the ordering of the
	meshes, and so the index of the meshes as returned by
	getMeshIndex() or taken by some methods will change.
	\param index The index of the mesh in the cache.
	\param name New name for the mesh.
	\return True if mesh was renamed. */
	virtual bool RenameMesh(unsigned int index, const eastl::string& name) = 0;

	//! Renames the loaded mesh
	/** Note that renaming meshes might change the ordering of the
	meshes, and so the index of the meshes as returned by
	getMeshIndex() or taken by some methods will change.
	\param mesh Mesh to be renamed.
	\param name New name for the mesh.
	\return True if mesh was renamed. */
	virtual bool RenameMesh(const eastl::shared_ptr<BaseMesh>& mesh, const eastl::string& name) = 0;

	//! Check if a mesh was already loaded.
	/** \param name Name of the mesh. Usually a filename.
	\return True if the mesh has been loaded, else false. */
	virtual bool IsMeshLoaded(const eastl::string& name) = 0;

	//! Clears the whole mesh cache, removing all meshes.
	/** All meshes will be reloaded completely when using ISceneManager::getMesh()
	after calling this method.
	Warning: If you have pointers to meshes that were loaded with ISceneManager::getMesh()
	and you did not grab them, then they may become invalid. */
	virtual void Clear() = 0;

	//! Clears all meshes that are held in the mesh cache but not used anywhere else.
	/** Warning: If you have pointers to meshes that were loaded with ISceneManager::getMesh()
	and you did not grab them, then they may become invalid. */
	virtual void ClearUnusedMeshes() = 0;
};

class MeshCache : public BaseMeshCache
{
public:

	virtual ~MeshCache();

	//! Adds a mesh to the internal list of loaded meshes.
	/** Usually, ISceneManager::getMesh() is called to load a mesh from file.
	That method searches the list of loaded meshes if a mesh has already been loaded and
	returns a pointer to if	it is in that list and already in memory. Otherwise it loads
	the mesh. With IMeshCache::addMesh(), it is possible to pretend that a mesh already
	has been loaded. This method can be used for example by mesh loaders who need to
	load more than one mesh with one call. They can add additional meshes with this
	method to the scene manager. The COLLADA loader for example uses this method.
	\param filename: Filename of the mesh. When called ISceneManager::getMesh() with this
	parameter, the method will return the mesh parameter given with this method.
	\param mesh: Pointer to a mesh which will now be referenced by this name. */
	virtual void AddMesh(const eastl::string& filename, const eastl::shared_ptr<BaseAnimatedMesh>& mesh);

	//! Removes a mesh from the cache.
	/** After loading a mesh with getMesh(), the mesh can be removed from the cache
	using this method, freeing a lot of memory. */
	virtual void RemoveMesh(const eastl::shared_ptr<BaseMesh>& mesh);

	//! Returns amount of loaded meshes in the cache.
	/** You can load new meshes into the cache using getMesh() and addMesh().
	If you ever need to access the internal mesh cache, you can do this using
	removeMesh(), getMeshNumber(), getMeshByIndex() and getMeshFilename() */
	virtual unsigned int GetMeshCount() const;

	//! Returns current index number of the mesh, and -1 if it is not in the cache.
	virtual int GetMeshIndex(const eastl::shared_ptr<BaseMesh>& mesh) const;

	//! Returns a mesh based on its index number.
	/** \param index: Index of the mesh, number between 0 and getMeshCount()-1.
	Note that this number is only valid until a new mesh is loaded or removed *
	\return Returns pointer to the mesh or 0 if there is none with this number. */
	virtual eastl::shared_ptr<BaseAnimatedMesh> GetMeshByIndex(unsigned int index);

	//! Returns a mesh based on its name.
	/** \param name Name of the mesh. Usually a filename.
	\return Pointer to the mesh or 0 if there is none with this number. */
	virtual eastl::shared_ptr<BaseAnimatedMesh> GetMeshByName(const eastl::string& name);

	//! Get the name of a loaded mesh, based on its index.
	/** \param index: Index of the mesh, number between 0 and getMeshCount()-1.
	\return The name if mesh was found and has a name, else	the path is empty. */
	virtual const eastl::string& GetMeshName(unsigned int index) const;

	//! Get the name of a loaded mesh, if there is any.
	/** \param mesh Pointer to mesh to query.
	\return The name if mesh was found and has a name, else	the path is empty. */
	virtual const eastl::string& GetMeshName(const eastl::shared_ptr<BaseMesh>& mesh) const;

	//! Renames a loaded mesh.
	/** Note that renaming meshes might change the ordering of the
	meshes, and so the index of the meshes as returned by
	getMeshIndex() or taken by some methods will change.
	\param index The index of the mesh in the cache.
	\param name New name for the mesh.
	\return True if mesh was renamed. */
	virtual bool RenameMesh(unsigned int index, const eastl::string& name);

	//! Renames a loaded mesh.
	/** Note that renaming meshes might change the ordering of the
	meshes, and so the index of the meshes as returned by
	getMeshIndex() or taken by some methods will change.
	\param mesh Mesh to be renamed.
	\param name New name for the mesh.
	\return True if mesh was renamed. */
	virtual bool RenameMesh(const eastl::shared_ptr<BaseMesh>& mesh, const eastl::string& name);

	//! returns if a mesh already was loaded
	virtual bool IsMeshLoaded(const eastl::string& name);

	//! Clears the whole mesh cache, removing all meshes.
	virtual void Clear();

	//! Clears all meshes that are held in the mesh cache but not used anywhere else.
	virtual void ClearUnusedMeshes();

protected:

	struct MeshEntry
	{
		MeshEntry ( const eastl::string& name ) : mMeshNamedPath ( name )
		{
		}
		eastl::string mMeshNamedPath;
		eastl::shared_ptr<BaseAnimatedMesh> mMesh;

		bool operator < (const MeshEntry& other) const
		{
			return (mMeshNamedPath < other.mMeshNamedPath);
		}
	};

	//! loaded meshes
	eastl::vector<MeshEntry> mMeshes;
};


#endif

