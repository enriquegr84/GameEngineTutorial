// David Eberly, Geometric Tools, Redmond WA 98052
// Copyright (c) 1998-2017
// Distributed under the Boost Software License, Version 1.0.
// http://www.boost.org/LICENSE_1_0.txt
// http://www.geometrictools.com/License/Boost/LICENSE_1_0.txt
// File Version: 3.0.0 (2016/06/19)

#ifndef DX11GRAPHICOBJECT_H
#define DX11GRAPHICOBJECT_H

#include "Graphic/GraphicStd.h"
#include "Graphic/Resource/GraphicObject.h"

#include "Core/Logger/Logger.h"

class GRAPHIC_ITEM DX11GraphicObject : public GraphicObject
{
public:
    virtual ~DX11GraphicObject();

	// Member access.
	inline GraphicObject* GetGraphicObject() const;
    inline ID3D11DeviceChild* GetDXDeviceChild() const;

protected:
	DX11GraphicObject(GraphicObject const* gtObject);

	GraphicObject* mGObject;
    ID3D11DeviceChild* mDXObject;
};

inline ID3D11DeviceChild* DX11GraphicObject::GetDXDeviceChild() const
{
    return mDXObject;
}

inline GraphicObject* DX11GraphicObject::GetGraphicObject() const
{
	return mGObject;
}

template <typename T>
inline ULONG SafeRelease(T*& object)
{
	if (object)
	{
		ULONG refs = object->Release();
		object = nullptr;
		return refs;
	}
	return 0;
}

template <typename T>
inline ULONG FinalRelease(T*& object)
{
	if (object)
	{
		ULONG refs = object->Release();
		object = nullptr;
		if (refs > 0)
		{
			LogError("Reference count is not zero after release.");
			return refs;
		}
	}
	return 0;
}

#endif