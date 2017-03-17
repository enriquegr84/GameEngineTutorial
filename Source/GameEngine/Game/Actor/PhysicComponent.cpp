//========================================================================
// PhysicsComponent.cpp - Component for physical movement and collision detection
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

#include "PhysicComponent.h"

#include "Application/GameApplication.h"

// all times are units-per-second
const float DEFAULT_MAX_VELOCITY = 7.5f;
const float DEFAULT_MAX_ANGULAR_VELOCITY = 1.2f;


//---------------------------------------------------------------------------------------------------------------------
// PhysicsComponent implementation
//---------------------------------------------------------------------------------------------------------------------

const char *PhysicComponent::g_Name = "PhysicsComponent";


PhysicComponent::PhysicComponent(void)
{
	m_RigidBodyLocation.MakeZero();
	m_RigidBodyOrientation.MakeZero();
	m_RigidBodyScale.MakeUnit(3);

    m_acceleration = 0;
    m_angularAcceleration = 0;
    m_maxVelocity = DEFAULT_MAX_VELOCITY;
    m_maxAngularVelocity = DEFAULT_MAX_ANGULAR_VELOCITY;
}

PhysicComponent::~PhysicComponent(void)
{
	GameApplication* gameApp = (GameApplication*)Application::App;
	BaseGamePhysic* gamePhysics = gameApp->mGame->GetGamePhysics().get();
	gamePhysics->RemoveActor(m_pOwner->GetId());
}

bool PhysicComponent::Init(XMLElement* pData)
{
    // no point in having a physics component with no game physics
	GameApplication* gameApp = (GameApplication*)Application::App;
	BaseGamePhysic* gamePhysics = gameApp->mGame->GetGamePhysics().get();
    if (!gamePhysics)
        return false;

    // shape
    XMLElement* pShape = pData->FirstChildElement("Shape");
    if (pShape)
    {
		m_shape = pShape->FirstChild()->Value();
    }

    // density
    XMLElement* pDensity = pData->FirstChildElement("Density");
    if (pDensity)
        m_density = pDensity->FirstChild()->Value();

    // material
    XMLElement* pMaterial = pData->FirstChildElement("PhysicMaterial");
    if (pMaterial)
        m_material = pMaterial->FirstChild()->Value();

    // initial transform
    XMLElement* pRigidBodyTransform = pData->FirstChildElement("RigidBodyTransform");
    if (pRigidBodyTransform)
        BuildRigidBodyTransform(pRigidBodyTransform);

    return true;
}

XMLElement* PhysicComponent::GenerateXml(void)
{
	XMLDocument doc;

	// base element
	XMLElement* pBaseElement = doc.NewElement(GetName());

    // shape
	XMLElement* pShape = doc.NewElement("Shape");
    XMLText* pShapeText = doc.NewText(m_shape.c_str());
    pShape->LinkEndChild(pShapeText);
	pBaseElement->LinkEndChild(pShape);

    // density
    XMLElement* pDensity = doc.NewElement("Density");
	XMLText* pDensityText = doc.NewText(m_density.c_str());
    pDensity->LinkEndChild(pDensityText);
    pBaseElement->LinkEndChild(pDensity);

    // material
    XMLElement* pMaterial = doc.NewElement("Material");
	XMLText* pMaterialText = doc.NewText(m_material.c_str());
    pMaterial->LinkEndChild(pMaterialText);
    pBaseElement->LinkEndChild(pMaterial);

    // rigid body transform
    XMLElement* pInitialTransform = doc.NewElement("RigidBodyTransform");

    // initial transform -> position
    XMLElement* pPosition = doc.NewElement("Position");
    pPosition->SetAttribute("x", eastl::to_string(m_RigidBodyLocation[0]).c_str());
    pPosition->SetAttribute("y", eastl::to_string(m_RigidBodyLocation[1]).c_str());
    pPosition->SetAttribute("z", eastl::to_string(m_RigidBodyLocation[2]).c_str());
    pInitialTransform->LinkEndChild(pPosition);

    // initial transform -> orientation
    XMLElement* pOrientation = doc.NewElement("Orientation");
    pOrientation->SetAttribute("yaw", eastl::to_string(m_RigidBodyOrientation[0]).c_str());
    pOrientation->SetAttribute("pitch", eastl::to_string(m_RigidBodyOrientation[1]).c_str());
    pOrientation->SetAttribute("roll", eastl::to_string(m_RigidBodyOrientation[2]).c_str());
    pInitialTransform->LinkEndChild(pOrientation);

	// initial transform -> scale 
    XMLElement* pScale = doc.NewElement("Scale");
    pScale->SetAttribute("x", eastl::to_string(m_RigidBodyScale[0]).c_str());
    pScale->SetAttribute("y", eastl::to_string(m_RigidBodyScale[1]).c_str());
    pScale->SetAttribute("z", eastl::to_string(m_RigidBodyScale[2]).c_str());
    pInitialTransform->LinkEndChild(pScale);

    pBaseElement->LinkEndChild(pInitialTransform);

    return pBaseElement;
}

void PhysicComponent::PostInit(void)
{
    if (m_pOwner)
    {
		GameApplication* gameApp = (GameApplication*)Application::App;
		BaseGamePhysic* gamePhysics = gameApp->mGame->GetGamePhysics().get();

		if (m_shape == "Sphere")
		{
			gamePhysics->AddSphere((float)m_RigidBodyScale[0], m_pOwner, m_density, m_material);
		}
		else if (m_shape == "Box")
		{
			gamePhysics->AddBox(m_RigidBodyScale, m_pOwner, m_density, m_material);
		}
		else if (m_shape == "PointCloud")
		{
			LogError("Not supported yet!");
		}
	}
}

void PhysicComponent::Update(int deltaMs)
{
    // get the transform component
    eastl::shared_ptr<TransformComponent> pTransformComponent(
		m_pOwner->GetComponent<TransformComponent>(TransformComponent::g_Name));
    if (!pTransformComponent)
    {
        LogError("No transform component!");
        return;
    }

	GameApplication* gameApp = (GameApplication*)Application::App;
	BaseGamePhysic* gamePhysics = gameApp->mGame->GetGamePhysics().get();
	if (m_acceleration != 0)
    {
        // calculate the acceleration this frame

        float accelerationPerFrame = m_acceleration / 1000.f * (float)deltaMs;

        // Get the current velocity vector and convert to a scalar. The velocity vector 
		// is a combination of the direction this actor is going in and the speed of the 
		// actor. The scalar is just the speed component.
        Vector3<float> velocity(gamePhysics->GetVelocity(m_pOwner->GetId()));
        float velocityScalar = Length(velocity);

		EulerAngles<float> rotation;
		// get the direction the object is facing
		Transform transform = pTransformComponent->GetTransform();
		transform.GetRotation(rotation);

		Vector3<float> direction{ rotation.angle[0], rotation.angle[1], rotation.angle[2] };
		gamePhysics->ApplyForce(direction, accelerationPerFrame, m_pOwner->GetId());

        // logging
        // [rez] Comment this back in if you want to debug the physics thrust & rotation stuff. It spams quite 
        // a bit of info the output window so I'm commenting it out for now.
		eastl::string info(eastl::string("acceleration: ") + eastl::to_string(accelerationPerFrame) +
			"; velocityScalar: " + eastl::to_string(velocityScalar) + "; direction: " + 
			eastl::to_string(direction[0]) + eastl::string(" ") + eastl::to_string(direction[1]) + 
			eastl::string(" ") + eastl::to_string(direction[2]) + "; direction.Length: " + 
			eastl::to_string(Length(direction)) + "; velocity: " + eastl::to_string(velocity[0]) + 
			eastl::string(" ") + eastl::to_string(velocity[1]) + eastl::string(" ") + 
			eastl::to_string(velocity[2]) + "; velocity.getLength(): " + eastl::to_string(Length(velocity)));

        LogInformation(info);
    }

    if (m_angularAcceleration != 0)
    {
        // calculate the acceleration this frame
		Vector3<float> upVector{ 0.f,1.f,0.f };
        float angularAccelerationToApplyThisFrame = m_angularAcceleration / 1000.f * (float)deltaMs;
		gamePhysics->ApplyTorque(upVector, angularAccelerationToApplyThisFrame, m_pOwner->GetId());

        // logging
        // [rez] Comment this back in if you want to debug the physics thrust & rotation stuff.  It spams quite 
        // a bit of info the output window so I'm commenting it out for now.
        //LogInformation("Actor", "Angular Acceleration: " + eastl::string(angularAccelerationToApplyThisFrame) );
    }
}

void PhysicComponent::BuildRigidBodyTransform(XMLElement* pTransformElement)
{
    XMLElement* pPositionElement = pTransformElement->FirstChildElement("Position");
    if (pPositionElement)
    {
        float x = 0;
		float y = 0;
		float z = 0;
        x = pPositionElement->FloatAttribute("x", x);
        y = pPositionElement->FloatAttribute("y", y);
        z = pPositionElement->FloatAttribute("z", z);
		m_RigidBodyLocation = Vector3<float>{ x, y, z };
    }

    XMLElement* pOrientationElement = pTransformElement->FirstChildElement("Orientation");
    if (pOrientationElement)
    {
		float yaw = 0;
		float pitch = 0;
		float roll = 0;
        yaw = pPositionElement->FloatAttribute("yaw", yaw);
        pitch = pPositionElement->FloatAttribute("pitch", pitch);
        roll = pPositionElement->FloatAttribute("roll", roll);
		m_RigidBodyOrientation = Vector3<float>{ yaw, pitch, roll };
    }

    XMLElement* pScaleElement = pTransformElement->FirstChildElement("Scale");
    if (pScaleElement)
    {
		float x = 0;
		float y = 0;
		float z = 0;
        x = pScaleElement->FloatAttribute("x", x);
        y = pScaleElement->FloatAttribute("y", y);
        z = pScaleElement->FloatAttribute("z", z);
		m_RigidBodyScale = Vector3<float>{ x, y, z };
    }
}

void PhysicComponent::ApplyForce(const Vector3<float>& direction, float forceNewtons)
{
	GameApplication* gameApp = (GameApplication*)Application::App;
	BaseGamePhysic* gamePhysics = gameApp->mGame->GetGamePhysics().get();
    gamePhysics->ApplyForce(direction, forceNewtons, m_pOwner->GetId());
}

void PhysicComponent::ApplyTorque(const Vector3<float>& direction, float forceNewtons)
{
	GameApplication* gameApp = (GameApplication*)Application::App;
	BaseGamePhysic* gamePhysics = gameApp->mGame->GetGamePhysics().get();
    gamePhysics->ApplyTorque(direction, forceNewtons, m_pOwner->GetId());
}

bool PhysicComponent::KinematicMove(const Transform &transform)
{
	GameApplication* gameApp = (GameApplication*)Application::App;
	BaseGamePhysic* gamePhysics = gameApp->mGame->GetGamePhysics().get();
	return gamePhysics->KinematicMove(transform, m_pOwner->GetId());
}

void PhysicComponent::ApplyAcceleration(float acceleration)
{
    m_acceleration = acceleration;
}

void PhysicComponent::RemoveAcceleration(void)
{
    m_acceleration = 0;
}

void PhysicComponent::ApplyAngularAcceleration(float acceleration)
{
    m_angularAcceleration = acceleration;
}

void PhysicComponent::RemoveAngularAcceleration(void)
{
    m_angularAcceleration = 0;
}

Vector3<float> PhysicComponent::GetVelocity(void)
{
	GameApplication* gameApp = (GameApplication*)Application::App;
	BaseGamePhysic* gamePhysics = gameApp->mGame->GetGamePhysics().get();
    return gamePhysics->GetVelocity(m_pOwner->GetId());
}

void PhysicComponent::SetVelocity(const Vector3<float>& velocity)
{
	GameApplication* gameApp = (GameApplication*)Application::App;
	BaseGamePhysic* gamePhysics = gameApp->mGame->GetGamePhysics().get();
    gamePhysics->SetVelocity(m_pOwner->GetId(), velocity);
}

void PhysicComponent::RotateY(float angleRadians)
{
    eastl::shared_ptr<TransformComponent> pTransformComponent(
		m_pOwner->GetComponent<TransformComponent>(TransformComponent::g_Name));
    if (pTransformComponent)
    {
		Transform transform = pTransformComponent->GetTransform();
        Vector3<float> position = transform.GetTranslation();

		Transform rotateY;
		EulerAngles<float> rotation(0, 1, 0, 0, angleRadians, 0);
        rotateY.SetRotation(rotation);
        rotateY.SetTranslation(position);

        KinematicMove(rotateY);
    }
    else
        LogError("Attempting to call RotateY() on actor with no transform component");
}

void PhysicComponent::SetPosition(float x, float y, float z)
{
    eastl::shared_ptr<TransformComponent> pTransformComponent(
		m_pOwner->GetComponent<TransformComponent>(TransformComponent::g_Name));
    if (pTransformComponent)
    {
		Transform transform = pTransformComponent->GetTransform();
		Vector3<float> position = Vector3<float>{ x, y, z };
        transform.SetTranslation(position);

        KinematicMove(transform);
    }
    else
        LogError("Attempting to call RotateY() on actor with no trnasform component");
}

void PhysicComponent::Stop(void)
{
	GameApplication* gameApp = (GameApplication*)Application::App;
	BaseGamePhysic* gamePhysics = gameApp->mGame->GetGamePhysics().get();
    return gamePhysics->StopActor(m_pOwner->GetId());
}