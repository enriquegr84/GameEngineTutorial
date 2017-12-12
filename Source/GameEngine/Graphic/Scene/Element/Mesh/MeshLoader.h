// Copyright (C) 2002-2012 Nikolaus Gebhardt
// This file is part of the "Irrlicht Engine".
// For conditions of distribution and use, see copyright notice in irrlicht.h

#ifndef MESHLOADER_H
#define MESHLOADER_H

#include "Core/IO/ResourceCache.h"

#include "AnimatedMesh.h"

class MeshResourceExtraData : public BaseResourceExtraData
{

public:
    virtual eastl::string ToString() { return "MeshResourceExtraData"; }
	virtual BaseAnimatedMesh* GetMesh() { return mAnimatedMesh; }
	virtual void SetMesh(BaseAnimatedMesh* aMesh) { mAnimatedMesh = aMesh; }
	virtual ~MeshResourceExtraData() { delete mAnimatedMesh; }

protected:
	BaseAnimatedMesh* mAnimatedMesh;

};


#endif

