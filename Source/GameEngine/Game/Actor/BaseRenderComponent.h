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

#include "RenderComponentInterface.h"
#include "Scenes/SceneNodes.h"

//---------------------------------------------------------------------------------------------------------------------
// RenderComponent base class.  This class does most of the work except actually creating the scene, which is 
// delegated to the subclass through a factory method:
// http://en.wikipedia.org/wiki/Factory_method_pattern
//---------------------------------------------------------------------------------------------------------------------

class BaseRenderComponent : public RenderComponentInterface
{
protected:
    Color m_color;
    eastl::shared_ptr<SceneNode> m_pSceneNode;

public:
    virtual bool Init(XmlElement* pData) override;
    virtual void PostInit(void) override;
	virtual void OnChanged(void) override;
    virtual XmlElement* GenerateXml(void) override;
	const Color GetColor() const { return m_color; }

protected:
    // loads the SceneNode specific data (represented in the <SceneNode> tag)
    virtual bool DelegateInit(XmlElement* pData) { return true; }
    virtual eastl::shared_ptr<SceneNode> CreateSceneNode(void) = 0;  // factory method to create the appropriate scene node
    Color LoadColor(XmlElement* pData);

    // editor stuff
	virtual XmlElement* CreateBaseElement(void) { return new XmlElement(GetName()); }
    virtual void CreateInheritedXmlElements(XmlElement* pBaseElement) = 0;

private:
    virtual const eastl::shared_ptr<SceneNode>& GetSceneNode(void) override;
};


#endif
