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

const char* TransformComponent::Name = "TransformComponent";

bool TransformComponent::Init(tinyxml2::XMLElement* pData)
{
    LogAssert(pData, "Invalid data");

	// [mrmike] - this was changed post-press - because changes to the TransformComponents can come in partial definitions,
	//            such as from the editor, its better to grab the current values rather than clear them out.
	Vector3<float> scale = mTransform.GetScale();
	Vector3<float> position = mTransform.GetTranslation();	
	Matrix4x4<float> rotation = mTransform.GetRotation();

	tinyxml2::XMLElement* pPositionElement = pData->FirstChildElement("Position");
    if (pPositionElement)
    {
        float x = 0;
		float y = 0;
		float z = 0;
        x = pPositionElement->FloatAttribute("x", x);
        y = pPositionElement->FloatAttribute("y", y);
        z = pPositionElement->FloatAttribute("z", z);

		position = Vector3<float>{ x, y, z };
    }

	tinyxml2::XMLElement* pOrientationElement = pData->FirstChildElement("YawPitchRoll");
    if (pOrientationElement)
    {
        float yaw = 0;
		float pitch = 0;
		float roll = 0;
		roll = pOrientationElement->FloatAttribute("x", roll);
		pitch = pOrientationElement->FloatAttribute("y", pitch);
        yaw = pOrientationElement->FloatAttribute("z", yaw);

		Matrix4x4<float> yawRotation = Rotation<4, float>(
			AxisAngle<4, float>(Vector4<float>::Unit(2), yaw * (float)GE_C_DEG_TO_RAD));
		Matrix4x4<float> pitchRotation = Rotation<4, float>(
			AxisAngle<4, float>(Vector4<float>::Unit(1), pitch * (float)GE_C_DEG_TO_RAD));
		Matrix4x4<float> rollRotation = Rotation<4, float>(
			AxisAngle<4, float>(Vector4<float>::Unit(0), roll * (float)GE_C_DEG_TO_RAD));

		rotation = yawRotation * pitchRotation * rollRotation;
	};

	tinyxml2::XMLElement* pScaleElement = pData->FirstChildElement("Scale");
	if (pScaleElement)
	{
		float x = 0;
		float y = 0;
		float z = 0;
		x = pScaleElement->FloatAttribute("x", x);
		y = pScaleElement->FloatAttribute("y", y);
		z = pScaleElement->FloatAttribute("z", z);

		scale = Vector3<float>{ x, y, z };
	}

	mTransform.SetScale(scale);
	mTransform.SetTranslation(position);
	mTransform.SetRotation(rotation);

    return true;
}

tinyxml2::XMLElement* TransformComponent::GenerateXml(void)
{
	tinyxml2::XMLDocument doc;

	// base element
	tinyxml2::XMLElement* pBaseElement = doc.NewElement(GetName());

    // initial transform -> position
	tinyxml2::XMLElement* pPosition = doc.NewElement("Position");
    Vector3<float> pos(mTransform.GetTranslation());
    pPosition->SetAttribute("x", eastl::to_string(pos[0]).c_str());
    pPosition->SetAttribute("y", eastl::to_string(pos[1]).c_str());
    pPosition->SetAttribute("z", eastl::to_string(pos[3]).c_str());
    pBaseElement->LinkEndChild(pPosition);

    // initial transform -> LookAt
	tinyxml2::XMLElement* pDirection = doc.NewElement("YawPitchRoll");

	EulerAngles<float> yawPitchRoll;
	mTransform.GetRotation(yawPitchRoll);
    pDirection->SetAttribute("x", eastl::to_string(yawPitchRoll.mAngle[0]).c_str());
    pDirection->SetAttribute("y", eastl::to_string(yawPitchRoll.mAngle[1]).c_str());
    pDirection->SetAttribute("z", eastl::to_string(yawPitchRoll.mAngle[2]).c_str());
    pBaseElement->LinkEndChild(pDirection);

	// This is not supported yet
    // initial transform -> position
	tinyxml2::XMLElement* pScale = doc.NewElement("Scale");
	Vector3<float> scale(mTransform.GetScale());
    pPosition->SetAttribute("x", eastl::to_string(scale[0]).c_str());
    pPosition->SetAttribute("y", eastl::to_string(scale[1]).c_str());
    pPosition->SetAttribute("z", eastl::to_string(scale[2]).c_str());
    pBaseElement->LinkEndChild(pScale);

    return pBaseElement;
}