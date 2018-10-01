// Copyright (C) 2002-2012 Nikolaus Gebhardt
// This file is part of the "Irrlicht Engine".
// For conditions of distribution and use, see copyright notice in irrlicht.h

#ifndef MESHLOADER_H
#define MESHLOADER_H

#include "Core/IO/ResourceCache.h"

#include "Mesh.h"

class MeshResourceExtraData : public BaseResourceExtraData
{

public:
    virtual eastl::wstring ToString() { return L"MeshResourceExtraData"; }
	virtual BaseMesh* GetMesh() { return mMesh; }
	virtual void SetMesh(BaseMesh* mesh) { mMesh = mesh; }
	virtual ~MeshResourceExtraData() { delete mMesh; }

protected:
	BaseMesh* mMesh;

};


#endif

