// David Eberly, Geometric Tools, Redmond WA 98052
// Copyright (c) 1998-2017
// Distributed under the Boost Software License, Version 1.0.
// http://www.boost.org/LICENSE_1_0.txt
// http://www.geometrictools.com/License/Boost/LICENSE_1_0.txt
// File Version: 3.0.0 (2016/06/19)

#include "DX11GraphicObject.h"


DX11GraphicObject::DX11GraphicObject(GraphicObject const* gObject)
	:
	GraphicObject(),
	mGObject(const_cast<GraphicObject*>(gObject)),
	mDXObject(nullptr)
{
	mType = gObject->GetType();
	mName = gObject->GetName();
}

DX11GraphicObject::~DX11GraphicObject()
{
	if (IsDrawingState())
	{
		// Sampler, blend, depth-stencil, and rasterizer states have only a
		// finite number of possibilities in DX11.  If you create a state
		// whose settings duplicate one already in existence, DX11 gives you
		// a pointer to the existing one, incrementing the reference count
		// internally.  GTE does not track the duplicates, so we cannot
		// assert that the reference count is zero.
		if (mDXObject) 
			mDXObject->Release();
	}
	else
	{
		FinalRelease(mDXObject);
	}
}