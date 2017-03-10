//========================================================================
// TransformComponent.cpp - Component for managing transforms on actors
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

#include "TransformComponent.h"

const char* TransformComponent::g_Name = "TransformComponent";

bool TransformComponent::Init(XMLElement* pData)
{
    LogAssert(pData);

	// [mrmike] - this was changed post-press - because changes to the TransformComponents can come in partial definitions,
	//            such as from the editor, its better to grab the current values rather than clear them out.
    
	Vector3 yawPitchRoll = m_transform.GetRotationDegrees();
	//yawPitchRoll.x = RADIANS_TO_DEGREES(yawPitchRoll.x);
	//yawPitchRoll.y = RADIANS_TO_DEGREES(yawPitchRoll.y);
	//yawPitchRoll.z = RADIANS_TO_DEGREES(yawPitchRoll.z);

	Vector3 position = m_transform.GetTranslation();	

    XMLElement* pPositionElement = pData->FirstChildElement("Position");
    if (pPositionElement)
    {
        double x = 0;
        double y = 0;
        double z = 0;
        pPositionElement->Attribute("x", &x);
        pPositionElement->Attribute("y", &y);
        pPositionElement->Attribute("z", &z);
        position = Vector3(x, y, z);
    }

    XMLElement* pOrientationElement = pData->FirstChildElement("YawPitchRoll");
    if (pOrientationElement)
    {
        double yaw = 0;
        double pitch = 0;
        double roll = 0;
        pOrientationElement->Attribute("x", &yaw);
        pOrientationElement->Attribute("y", &pitch);
        pOrientationElement->Attribute("z", &roll);
		yawPitchRoll = Vector3(yaw, pitch, roll);
	}

	Matrix4x4 translation;
	translation.SetTranslation(position);

	Matrix4x4 rotation;
	rotation.SetRotationDegrees(yawPitchRoll);

	// This is not supported yet.
    XMLElement* pLookAtElement = pData->FirstChildElement("LookAt");
    if (pLookAtElement)
    {
        double x = 0;
        double y = 0;
        double z = 0;
        pLookAtElement->Attribute("x", &x);
        pLookAtElement->Attribute("y", &y);
        pLookAtElement->Attribute("z", &z);

		Vector3 lookAt(x, y, z);
		//rotation.buildCameraLookAtMatrixLH(translation.getTranslation(), lookAt, g_Up);
    }

    XMLElement* pScaleElement = pData->FirstChildElement("Scale");
    if (pScaleElement)
    {
        double x = 0;
        double y = 0;
        double z = 0;
        pScaleElement->Attribute("x", &x);
        pScaleElement->Attribute("y", &y);
        pScaleElement->Attribute("z", &z);
        //scale = Vector3(x, y, z);
    }

    m_transform = rotation * translation;
    
    return true;
}

XMLElement* TransformComponent::GenerateXml(void)
{
    XMLElement* pBaseElement = new XMLElement(GetName());

    // initial transform -> position
    XMLElement* pPosition = new XMLElement("Position");
    Vector3 pos(m_transform.GetTranslation());
    pPosition->SetAttribute("x", eastl::string(pos.X).c_str());
    pPosition->SetAttribute("y", eastl::string(pos.Y).c_str());
    pPosition->SetAttribute("z", eastl::string(pos.Z).c_str());
    pBaseElement->LinkEndChild(pPosition);

    // initial transform -> LookAt
    XMLElement* pDirection = new XMLElement("YawPitchRoll");
	Vector3 orient(m_transform.GetRotationDegrees());
    pDirection->SetAttribute("x", eastl::string(orient.X).c_str());
    pDirection->SetAttribute("y", eastl::string(orient.Y).c_str());
    pDirection->SetAttribute("z", eastl::string(orient.Z).c_str());
    pBaseElement->LinkEndChild(pDirection);

	// This is not supported yet
    // initial transform -> position
    XMLElement* pScale = new XMLElement("Scale");
	Vector3 scale(m_transform.GetScale());
    pPosition->SetAttribute("x", eastl::string(scale.X).c_str());
    pPosition->SetAttribute("y", eastl::string(scale.Y).c_str());
    pPosition->SetAttribute("z", eastl::string(scale.Z).c_str());
    pBaseElement->LinkEndChild(pScale);

    return pBaseElement;
}