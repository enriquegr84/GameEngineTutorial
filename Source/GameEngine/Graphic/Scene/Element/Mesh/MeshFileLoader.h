// Copyright (C) 2002-2012 Nikolaus Gebhardt / Thomas Alten
// This file is part of the "Irrlicht Engine".
// For conditions of distribution and use, see copyright notice in irrlicht.h

#ifndef MESHFILELOADER_H
#define MESHFILELOADER_H

#include "MeshLoader.h"

//! Meshloader capable of loading mesh files.
class MeshFileLoader : public BaseResourceLoader
{
public:

	//! Constructor
	MeshFileLoader( );

	//! destructor
	virtual ~MeshFileLoader();

	virtual bool UseRawFile() { return false; }
	virtual bool DiscardRawBufferAfterLoad() { return false; }
	virtual unsigned int GetLoadedResourceSize(void *rawBuffer, unsigned int rawSize);
	virtual bool LoadResource(void *rawBuffer, unsigned int rawSize, const eastl::shared_ptr<ResHandle>& handle);
	virtual bool MatchResourceFormat(eastl::wstring name) { return IsALoadableFileExtension(name.c_str()); }

protected:

	//! returns true if the file maybe is able to be loaded by this class
	//! based on the file extension
	virtual bool IsALoadableFileExtension(const eastl::wstring& fileName) const;

	//! creates/loads an animated mesh from the file.
	//! \return Pointer to the created mesh. Returns 0 if loading failed.
	BaseMesh* CreateMesh(BaseReadFile* file);

};

#endif

