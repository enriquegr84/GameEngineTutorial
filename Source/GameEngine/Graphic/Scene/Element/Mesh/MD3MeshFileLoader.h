// Copyright (C) 2002-2012 Nikolaus Gebhardt / Thomas Alten
// This file is part of the "Irrlicht Engine".
// For conditions of distribution and use, see copyright notice in irrlicht.h

#ifndef _MD3_MESH_FILE_LOADER_H_INCLUDED_
#define _MD3_MESH_FILE_LOADER_H_INCLUDED_

#include "IMeshLoader.h"
#include "Scenes/Scene.h"
#include "Graphics/IQ3Shader.h"

//! Meshloader capable of loading md3 files.
class MD3MeshFileLoader : public IResourceLoader
{
public:

	//! Constructor
	MD3MeshFileLoader( );

	//! destructor
	virtual ~MD3MeshFileLoader();

	virtual bool UseRawFile() { return false; }
	virtual bool DiscardRawBufferAfterLoad() { return false; }
	virtual unsigned int GetLoadedResourceSize(void *rawBuffer, unsigned int rawSize);
	virtual bool LoadResource(void *rawBuffer, unsigned int rawSize, const shared_ptr<ResHandle>& handle);
	virtual bool MatchResourceFormat(stringc name)  { return IsALoadableFileExtension(path(name.c_str())); }

protected:

	//! returns true if the file maybe is able to be loaded by this class
	//! based on the file extension (e.g. ".bsp")
	virtual bool IsALoadableFileExtension(const path& filename) const;

	//! creates/loads an animated mesh from the file.
	//! \return Pointer to the created mesh. Returns 0 if loading failed.
	virtual IAnimatedMesh* CreateMesh(IReadFile* file);

};

#endif

