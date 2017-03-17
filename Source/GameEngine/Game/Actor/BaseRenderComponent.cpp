 
//========================================================================
//	BaseRenderComponent.cpp : classes that define renderable components of actors 
//	like Meshes, Skyboxes, Lights, etc.
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

#include "BaseRenderComponent.h"

//---------------------------------------------------------------------------------------------------------------------
// RenderComponent
//---------------------------------------------------------------------------------------------------------------------

bool BaseRenderComponent::Init(XMLElement* pData)
{
    // color
    XMLElement* pColorNode = pData->FirstChildElement("Color");
	/*
    if (pColorNode)
        m_color = LoadColor(pColorNode);
	*/
    return DelegateInit(pData);
}


void BaseRenderComponent::PostInit(void)
{
	/*
	eastl::shared_ptr<SceneNode> pSceneNode(GetSceneNode());
	eastl::shared_ptr<EvtData_New_Render_Component> pEvent(
		new EvtData_New_Render_Component(m_pOwner->GetId(), pSceneNode));
    BaseEventManager::Get()->TriggerEvent(pEvent);
	*/
}


void BaseRenderComponent::OnChanged(void)
{
	/*
	eastl::shared_ptr<EvtData_Modified_Render_Component> pEvent(
		new EvtData_Modified_Render_Component(m_pOwner->GetId()));
    BaseEventManager::Get()->TriggerEvent(pEvent);
	*/
}


XMLElement* BaseRenderComponent::GenerateXml(void)
{
	XMLDocument doc;

	// base element
	XMLElement* pBaseElement = doc.NewElement(GetName());

    // color
    XMLElement* pColor = doc.NewElement("Color");
	/*
    pColor->SetAttribute("r", eastl::string(m_color.GetRed()).c_str());
    pColor->SetAttribute("g", eastl::string(m_color.GetGreen()).c_str());
    pColor->SetAttribute("b", eastl::string(m_color.GetBlue()).c_str());
    pColor->SetAttribute("a", eastl::string(m_color.GetAlpha()).c_str());
	*/
    pBaseElement->LinkEndChild(pColor);

    // create XML for inherited classes
    CreateInheritedXMLElements(&doc, pBaseElement);

    return pBaseElement;
}
/*
const eastl::shared_ptr<SceneNode>& BaseRenderComponent::GetSceneNode(void)
{
    if (!m_pSceneNode)
        m_pSceneNode = CreateSceneNode();
    return m_pSceneNode;
}

Color BaseRenderComponent::LoadColor(XMLElement* pData)
{
	Color color;

    double r = 1.0;
    double g = 1.0;
    double b = 1.0;
    double a = 1.0;

    pData->Attribute("r", &r);
    pData->Attribute("g", &g);
    pData->Attribute("b", &b);
    pData->Attribute("a", &a);

    color.SetRed(r);
    color.SetGreen(g);
    color.SetBlue(b);
    color.SetAlpha(a);

	return color;
}
*/