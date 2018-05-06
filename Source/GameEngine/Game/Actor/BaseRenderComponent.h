//========================================================================
// RenderComponent.h : classes that define renderable components of actors like Meshes, Skyboxes, Lights, etc.
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

#ifndef _BASERENDERCOMPONENT_H_INCLUDED_
#define _BASERENDERCOMPONENT_H_INCLUDED_

#include "GameEngineStd.h"

#include "ActorComponent.h"

//---------------------------------------------------------------------------------------------------------------------
// RenderComponent base class.  This class does most of the work except actually creating the scene, which is 
// delegated to the subclass through a factory method:
// http://en.wikipedia.org/wiki/Factory_method_pattern
//---------------------------------------------------------------------------------------------------------------------

class BaseRenderComponent : public ActorComponent
{
protected:
    //Color mColor;
    //eastl::shared_ptr<SceneNode> mSceneNode;

	// virtual ComponentId GetComponentId(void) const override { return COMPONENT_ID; }

	// RenderComponentInterface
	//virtual const eastl::shared_ptr<SceneNode>& GetSceneNode(void) = 0;

public:
    virtual bool Init(tinyxml2::XMLElement* pData) override;
    virtual void PostInit(void) override;
	virtual void OnChanged(void) override;
    virtual tinyxml2::XMLElement* GenerateXml(void) override;
	//const Color GetColor() const { return mColor; }

protected:
    // loads the SceneNode specific data (represented in the <SceneNode> tag)
    virtual bool DelegateInit(tinyxml2::XMLElement* pData) { return true; }
	// factory method to create the appropriate scene node
    //virtual eastl::shared_ptr<SceneNode> CreateSceneNode(void) = 0;  
    //Color LoadColor(XMLElement* pData);

    // editor stuff
    virtual void CreateInheritedXMLElements(
		tinyxml2::XMLDocument doc, tinyxml2::XMLElement* pBaseElement) = 0;

private:
    //virtual const eastl::shared_ptr<SceneNode>& GetSceneNode(void) override;
};


#endif
