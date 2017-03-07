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

#include "PhysicsComponent.h"

// all times are units-per-second
const float DEFAULT_MAX_VELOCITY = 7.5f;
const float DEFAULT_MAX_ANGULAR_VELOCITY = 1.2f;


//---------------------------------------------------------------------------------------------------------------------
// PhysicsComponent implementation
//---------------------------------------------------------------------------------------------------------------------

const char *PhysicsComponent::g_Name = "PhysicsComponent";


PhysicsComponent::PhysicsComponent(void)
{
    m_RigidBodyLocation = Vector3(0.f,0.f,0.f);
	m_RigidBodyOrientation = Vector3(0.f,0.f,0.f);
	m_RigidBodyScale = Vector3(1.f,1.f,1.f);

    m_acceleration = 0;
    m_angularAcceleration = 0;
    m_maxVelocity = DEFAULT_MAX_VELOCITY;
    m_maxAngularVelocity = DEFAULT_MAX_ANGULAR_VELOCITY;
}

PhysicsComponent::~PhysicsComponent(void)
{
    m_pGamePhysics->RemoveActor(m_pOwner->GetId());
}

bool PhysicsComponent::Init(XmlElement* pData)
{
    // no point in having a physics component with no game physics
	m_pGamePhysics = g_pGameApp->GetGameLogic()->GetGamePhysics();
    if (!m_pGamePhysics)
        return false;

    // shape
    XmlElement* pShape = pData->FirstChildElement("Shape");
    if (pShape)
    {
		m_shape = pShape->FirstChild()->Value();
    }

    // density
    XmlElement* pDensity = pData->FirstChildElement("Density");
    if (pDensity)
        m_density = pDensity->FirstChild()->Value();

    // material
    XmlElement* pMaterial = pData->FirstChildElement("PhysicMaterial");
    if (pMaterial)
        m_material = pMaterial->FirstChild()->Value();

    // initial transform
    XmlElement* pRigidBodyTransform = pData->FirstChildElement("RigidBodyTransform");
    if (pRigidBodyTransform)
        BuildRigidBodyTransform(pRigidBodyTransform);

    return true;
}

XmlElement* PhysicsComponent::GenerateXml(void)
{
    XmlElement* pBaseElement = new XmlElement(GetName());

    // shape
	XmlElement* pShape = new XmlElement("Shape");
    XmlText* pShapeText = new XmlText(m_shape.c_str());
    pShape->LinkEndChild(pShapeText);
	pBaseElement->LinkEndChild(pShape);

    // density
    XmlElement* pDensity = new XmlElement("Density");
    XmlText* pDensityText = new XmlText(m_density.c_str());
    pDensity->LinkEndChild(pDensityText);
    pBaseElement->LinkEndChild(pDensity);

    // material
    XmlElement* pMaterial = new XmlElement("Material");
    XmlText* pMaterialText = new XmlText(m_material.c_str());
    pMaterial->LinkEndChild(pMaterialText);
    pBaseElement->LinkEndChild(pMaterial);

    // rigid body transform
    XmlElement* pInitialTransform = new XmlElement("RigidBodyTransform");

    // initial transform -> position
    XmlElement* pPosition = new XmlElement("Position");
    pPosition->SetAttribute("x", eastl::string(m_RigidBodyLocation.X).c_str());
    pPosition->SetAttribute("y", eastl::string(m_RigidBodyLocation.Y).c_str());
    pPosition->SetAttribute("z", eastl::string(m_RigidBodyLocation.Z).c_str());
    pInitialTransform->LinkEndChild(pPosition);

    // initial transform -> orientation
    XmlElement* pOrientation = new XmlElement("Orientation");
    pOrientation->SetAttribute("yaw", eastl::string(m_RigidBodyOrientation.X).c_str());
    pOrientation->SetAttribute("pitch", eastl::string(m_RigidBodyOrientation.Y).c_str());
    pOrientation->SetAttribute("roll", eastl::string(m_RigidBodyOrientation.Z).c_str());
    pInitialTransform->LinkEndChild(pOrientation);

	// initial transform -> scale 
    XmlElement* pScale = new XmlElement("Scale");
    pScale->SetAttribute("x", eastl::string(m_RigidBodyScale.X).c_str());
    pScale->SetAttribute("y", eastl::string(m_RigidBodyScale.Y).c_str());
    pScale->SetAttribute("z", eastl::string(m_RigidBodyScale.Z).c_str());
    pInitialTransform->LinkEndChild(pScale);

    pBaseElement->LinkEndChild(pInitialTransform);

    return pBaseElement;
}

void PhysicsComponent::PostInit(void)
{
    if (m_pOwner)
    {
		if (m_shape == "Sphere")
		{
			m_pGamePhysics->AddSphere((float)m_RigidBodyScale.X, m_pOwner, m_density, m_material);
		}
		else if (m_shape == "Box")
		{
			m_pGamePhysics->AddBox(m_RigidBodyScale, m_pOwner, m_density, m_material);
		}
		else if (m_shape == "PointCloud")
		{
			LogError("Not supported yet!");
		}
	}
}

void PhysicsComponent::Update(int deltaMs)
{
    // get the transform component
    shared_ptr<TransformComponent> pTransformComponent = 
		eastl::make_shared(m_pOwner->GetComponent<TransformComponent>(TransformComponent::g_Name));
    if (!pTransformComponent)
    {
        LogError("No transform component!");
        return;
    }

    // get the direction the object is facing
    matrix4 transform = pTransformComponent->GetTransform();

	if (m_acceleration != 0)
    {
        // calculate the acceleration this frame

        float accelerationToApplyThisFrame = m_acceleration / 1000.f * (float)deltaMs;

        // Get the current velocity vector and convert to a scalar.  The velocity vector is a combination of 
        // the direction this actor is going in and the speed of the actor.  The scalar is just the speed 
        // component.
        Vector3 velocity(m_pGamePhysics->GetVelocity(m_pOwner->GetId()));
        float velocityScalar = velocity.GetLength();

		Vector3 direction;
		transform.TransformVect(direction);
		direction.Normalize();
		m_pGamePhysics->ApplyForce(direction, accelerationToApplyThisFrame, m_pOwner->GetId());

        // logging
        // [rez] Comment this back in if you want to debug the physics thrust & rotation stuff.  It spams quite 
        // a bit of info the output window so I'm commenting it out for now.

        LogInformation("Actor", eastl::string("Acceleration: ") + eastl::string(accelerationToApplyThisFrame) + \
				"; velocityScalar: " + eastl::string(velocityScalar) + "; direction: " + eastl::string(direction.X) + \
				eastl::string(" ") + eastl::string(direction.Y) + eastl::string(" ") + eastl::string(direction.Z) + \
				"; direction.getLength(): " + eastl::string(direction.GetLength()) + "; velocity: " + \
				eastl::string(velocity.X) + eastl::string(" ") + eastl::string(velocity.Y) + eastl::string(" ") + eastl::string(velocity.Z) + \
				"; velocity.getLength(): " + eastl::string(velocity.GetLength()));

    }

    if (m_angularAcceleration != 0)
    {
        // calculate the acceleration this frame
		Vector3 upVector(0.f,1.f,0.f);
        float angularAccelerationToApplyThisFrame = m_angularAcceleration / 1000.f * (float)deltaMs;
		m_pGamePhysics->ApplyTorque(upVector, angularAccelerationToApplyThisFrame, m_pOwner->GetId());

        // logging
        // [rez] Comment this back in if you want to debug the physics thrust & rotation stuff.  It spams quite 
        // a bit of info the output window so I'm commenting it out for now.
        //LogInformation("Actor", "Angular Acceleration: " + eastl::string(angularAccelerationToApplyThisFrame) );
    }
}

void PhysicsComponent::BuildRigidBodyTransform(XmlElement* pTransformElement)
{
	// FUTURE WORK Mrmike - this should be exactly the same as the TransformComponent - maybe factor into a helper method?
    LogAssert(pTransformElement);

    XmlElement* pPositionElement = pTransformElement->FirstChildElement("Position");
    if (pPositionElement)
    {
        double x = 0;
        double y = 0;
        double z = 0;
        pPositionElement->Attribute("x", &x);
        pPositionElement->Attribute("y", &y);
        pPositionElement->Attribute("z", &z);
        m_RigidBodyLocation = Vector3(x, y, z);
    }

    XmlElement* pOrientationElement = pTransformElement->FirstChildElement("Orientation");
    if (pOrientationElement)
    {
        double yaw = 0;
        double pitch = 0;
        double roll = 0;
        pPositionElement->Attribute("yaw", &yaw);
        pPositionElement->Attribute("pitch", &pitch);
        pPositionElement->Attribute("roll", &roll);
        m_RigidBodyOrientation = Vector3(yaw, pitch, roll);
    }

    XmlElement* pScaleElement = pTransformElement->FirstChildElement("Scale");
    if (pScaleElement)
    {
        double x = 0;
        double y = 0;
        double z = 0;
        pScaleElement->Attribute("x", &x);
        pScaleElement->Attribute("y", &y);
        pScaleElement->Attribute("z", &z);
        m_RigidBodyScale = Vector3(x, y, z);
    }
}

void PhysicsComponent::ApplyForce(const Vector3& direction, float forceNewtons)
{
    m_pGamePhysics->ApplyForce(direction, forceNewtons, m_pOwner->GetId());
}

void PhysicsComponent::ApplyTorque(const Vector3& direction, float forceNewtons)
{
    m_pGamePhysics->ApplyTorque(direction, forceNewtons, m_pOwner->GetId());
}

bool PhysicsComponent::KinematicMove(const Matrix4x4 &transform)
{
	return m_pGamePhysics->KinematicMove(transform, m_pOwner->GetId());
}

void PhysicsComponent::ApplyAcceleration(float acceleration)
{
    m_acceleration = acceleration;
}

void PhysicsComponent::RemoveAcceleration(void)
{
    m_acceleration = 0;
}

void PhysicsComponent::ApplyAngularAcceleration(float acceleration)
{
    m_angularAcceleration = acceleration;
}

void PhysicsComponent::RemoveAngularAcceleration(void)
{
    m_angularAcceleration = 0;
}

Vector3 PhysicsComponent::GetVelocity(void)
{
    return m_pGamePhysics->GetVelocity(m_pOwner->GetId());
}

void PhysicsComponent::SetVelocity(const Vector3& velocity)
{
    m_pGamePhysics->SetVelocity(m_pOwner->GetId(), velocity);
}

void PhysicsComponent::RotateY(float angleRadians)
{
    shared_ptr<TransformComponent> pTransformComponent = eastl::make_shared(m_pOwner->GetComponent<TransformComponent>(TransformComponent::g_Name));
    if (pTransformComponent)
    {
		Matrix4x4 transform = pTransformComponent->GetTransform();
        Vector3 position = transform.GetTranslation();

		Matrix4x4 rotateY;
        rotateY.SetRotationRadians(Vector3(0,angleRadians,0));
        rotateY.SetTranslation(position);

        KinematicMove(rotateY);
    }
    else
        LogError("Attempting to call RotateY() on actor with no transform component");
}

void PhysicsComponent::SetPosition(float x, float y, float z)
{
    eastl::shared_ptr<TransformComponent> pTransformComponent = 
		eastl::make_shared(m_pOwner->GetComponent<TransformComponent>(TransformComponent::g_Name));
    if (pTransformComponent)
    {
		Matrix4x4 transform = pTransformComponent->GetTransform();
        Vector3 position = Vector3(x, y, z);
        transform.SetTranslation(position);

        KinematicMove(transform);
    }
    else
        LogError("Attempting to call RotateY() on actor with no trnasform component");
}

void PhysicsComponent::Stop(void)
{
    return m_pGamePhysics->StopActor(m_pOwner->GetId());
}