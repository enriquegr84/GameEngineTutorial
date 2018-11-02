// David Eberly, Geometric Tools, Redmond WA 98052
// Copyright (c) 1998-2018
// Distributed under the Boost Software License, Version 1.0.
// http://www.boost.org/LICENSE_1_0.txt
// http://www.geometrictools.com/License/Boost/LICENSE_1_0.txt
// File Version: 3.0.0 (2016/06/19)

#ifndef GL4GRAPHICOBJECT_H
#define GL4GRAPHICOBJECT_H

#include "Graphic/Resource/GraphicObject.h"
#include "Graphic/Renderer/OpenGL4/OpenGL.h"

class GRAPHIC_ITEM GL4GraphicObject : public GraphicObject
{

public:
    // Member access.
    inline GLuint GetGLHandle() const;
	inline GraphicObject* GetGraphicObject() const;

protected:
	// Abstract base class.
	GL4GraphicObject(GraphicObject const* gtObject);

	GraphicObject * mGObject;
    GLuint mGLHandle;
};

inline GLuint GL4GraphicObject::GetGLHandle() const
{
    return mGLHandle;
}

inline GraphicObject* GL4GraphicObject::GetGraphicObject() const
{
	return mGObject;
}

#endif