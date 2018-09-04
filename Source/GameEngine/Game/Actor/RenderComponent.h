//========================================================================
// RenderComponent.h : classes that define renderable components of actors like Meshes, Skies, Lights, etc.
// Author: David "Rez" Graham
//
// Part of the GameEngine Application
//
// GameEngine is the sample application that encapsulates much of the source code
// discussed in "Game Coding Complete - 4th Edition" by Mike McShaffry and David
// "Rez" Graham, published by Charles River Media. 
// ISBN-10: 1133776574 | ISBN-13: 978-1133776574
//
// If this source code has found it's way to you, and you think it has helped you
// in any way, do the authors a favor and buy a new copy of the book - there are 
// detailed explanations in it that compliment this code well. Buy a copy at Amazon.com
// by clicking here: 
//    http://www.amazon.com/gp/product/1133776574/ref=olp_product_details?ie=UTF8&me=&seller=
//
// There's a companion web site at http://www.mcshaffry.com/GameCode/
// 
// The source code is managed and maintained through Google Code: 
//    http://code.google.com/p/GameEngine/
//
// (c) Copyright 2012 Michael L. McShaffry and David Graham
//
// This program is free software; you can redistribute it and/or
// modify it under the terms of the GNU Lesser GPL v3
// as published by the Free Software Foundation.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See 
// http://www.gnu.org/licenses/lgpl-3.0.txt for more details.
//
// You should have received a copy of the GNU Lesser GPL v3
// along with this program; if not, write to the Free Software
// Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
//
//========================================================================

#ifndef RENDERCOMPONENT_H
#define RENDERCOMPONENT_H

#include "GameEngineStd.h"

#include "BaseRenderComponent.h"

#include "Mathematic/Algebra/Vector2.h"
#include "Mathematic/Algebra/Vector3.h"

//---------------------------------------------------------------------------------------------------------------------
// This class represents a render component built from a Mesh.  In a real game, this is the one you'll use 99% of the 
// time towards the end of the project.  The other classes are important for testing since programming tends to move 
// a lot faster than art in the early stages of development.
//---------------------------------------------------------------------------------------------------------------------
class MeshRenderComponent : public BaseRenderComponent
{
	eastl::string mMeshModelFile;;
	eastl::string mMeshTextureFile;

public:
	static const char *Name;
	virtual const char *GetName() const { return Name; }

    MeshRenderComponent(void);

protected:
    virtual bool DelegateInit(tinyxml2::XMLElement* pData) override;
    virtual eastl::shared_ptr<Node> CreateSceneNode(void) override;  // factory method to create the appropriate scene node

    // editor stuff
    virtual void CreateInheritedXMLElements(tinyxml2::XMLDocument doc, tinyxml2::XMLElement* pBaseElement);
};

//---------------------------------------------------------------------------------------------------------------------
// Spheres
//---------------------------------------------------------------------------------------------------------------------
class SphereRenderComponent : public BaseRenderComponent
{
	eastl::string mTextureResource;
	Vector2<float> mTextureScale;
    unsigned int mSegments;
	float mRadius;

public:
	static const char *Name;
	virtual const char *GetName() const { return Name; }

    SphereRenderComponent(void);

protected:
    virtual bool DelegateInit(tinyxml2::XMLElement* pData) override;
	virtual eastl::shared_ptr<Node> CreateSceneNode(void) override;  // factory method to create the appropriate scene node

    // editor stuff
    virtual void CreateInheritedXMLElements(tinyxml2::XMLDocument doc, tinyxml2::XMLElement* pBaseElement);
};


//---------------------------------------------------------------------------------------------------------------------
// Grids, which represent the world
//---------------------------------------------------------------------------------------------------------------------
class GridRenderComponent : public BaseRenderComponent
{
    eastl::string mTextureResource;
	Vector2<float> mTextureScale;
    Vector2<int> mSegments;
	Vector2<float> mExtent;

public:
	static const char *Name;
	virtual const char *GetName() const { return Name; }

    GridRenderComponent(void);
	const char* GetTextureResource() { return mTextureResource.c_str(); }
	const Vector2<float> GetTextureScale() { return mTextureScale; }
	const Vector2<int> GetSegments() { return mSegments; }
	const Vector2<float> GetExtent() { return mExtent; }

protected:
    virtual bool DelegateInit(tinyxml2::XMLElement* pData) override;
	virtual eastl::shared_ptr<Node> CreateSceneNode(void) override;  // factory method to create the appropriate scene node

    // editor stuff
    virtual void CreateInheritedXMLElements(tinyxml2::XMLDocument doc, tinyxml2::XMLElement* pBaseElement);
};


//---------------------------------------------------------------------------------------------------------------------
// Lights
//---------------------------------------------------------------------------------------------------------------------
class LightRenderComponent : public BaseRenderComponent
{
	Light mLightData; 

	float mAnimatorSpeed;
	eastl::string mAnimatorType;
	Vector3<float> mAnimatorCenter;

	bool mAddBillboard;
	eastl::string mBillboardMaterial;
	eastl::string mBillboardTexture;
	Vector2<float> mBillboardSize;

public:
	static const char *Name;
	virtual const char *GetName() const { return Name; }

    LightRenderComponent(void);

protected:
    virtual bool DelegateInit(tinyxml2::XMLElement* pData) override;
	virtual eastl::shared_ptr<Node> CreateSceneNode(void) override;  // factory method to create the appropriate scene node

    // editor stuff
    virtual void CreateInheritedXMLElements(tinyxml2::XMLDocument doc, tinyxml2::XMLElement* pBaseElement);
};


//---------------------------------------------------------------------------------------------------------------------
// ParticleSystem
//---------------------------------------------------------------------------------------------------------------------
class ParticleSystemRenderComponent : public BaseRenderComponent
{

public:
	static const char *Name;
	virtual const char *GetName() const { return Name; }

    ParticleSystemRenderComponent(void);

protected:
    virtual bool DelegateInit(tinyxml2::XMLElement* pData) override;
	virtual eastl::shared_ptr<Node> CreateSceneNode(void) override;  // factory method to create the appropriate scene node

    // editor stuff
    virtual void CreateInheritedXMLElements(tinyxml2::XMLDocument doc, tinyxml2::XMLElement* pBaseElement);
};


//---------------------------------------------------------------------------------------------------------------------
// Sky
//---------------------------------------------------------------------------------------------------------------------
class SkyRenderComponent : public BaseRenderComponent
{
	eastl::string mTextureResource;
	unsigned int mHoriRes, mVertRes;
	float mTexturePercentage, mSpherePercentage, mRadius;

public:
	static const char *Name;
	virtual const char *GetName() const { return Name; }

    SkyRenderComponent(void);

protected:
    virtual bool DelegateInit(tinyxml2::XMLElement* pData) override;
	virtual eastl::shared_ptr<Node> CreateSceneNode(void) override;  // factory method to create the appropriate scene node

    // editor stuff
    virtual void CreateInheritedXMLElements(tinyxml2::XMLDocument doc, tinyxml2::XMLElement* pBaseElement);
};

#endif
