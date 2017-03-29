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

const char *PhysicComponent::Name = "PhysicsComponent";


PhysicComponent::PhysicComponent(void)
{
	mRigidBodyLocation.MakeZero();
	mRigidBodyOrientation.MakeZero();
	mRigidBodyScale.MakeUnit(3);

    mAcceleration = 0;
    mAngularAcceleration = 0;
    mMaxVelocity = DEFAULT_MAX_VELOCITY;
    mMaxAngularVelocity = DEFAULT_MAX_ANGULAR_VELOCITY;
}

PhysicComponent::~PhysicComponent(void)
{
	GameApplication* gameApp = (GameApplication*)Application::App;
	BaseGamePhysic* gamePhysics = gameApp->mGame->GetGamePhysics().get();
	gamePhysics->RemoveActor(mOwner->GetId());
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
		mShape = pShape->FirstChild()->Value();
    }

    // density
    XMLElement* pDensity = pData->FirstChildElement("Density");
    if (pDensity)
        mDensity = pDensity->FirstChild()->Value();

    // material
    XMLElement* pMaterial = pData->FirstChildElement("PhysicMaterial");
    if (pMaterial)
        mMaterial = pMaterial->FirstChild()->Value();

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
    XMLText* pShapeText = doc.NewText(mShape.c_str());
    pShape->LinkEndChild(pShapeText);
	pBaseElement->LinkEndChild(pShape);

    // density
    XMLElement* pDensity = doc.NewElement("Density");
	XMLText* pDensityText = doc.NewText(mDensity.c_str());
    pDensity->LinkEndChild(pDensityText);
    pBaseElement->LinkEndChild(pDensity);

    // material
    XMLElement* pMaterial = doc.NewElement("Material");
	XMLText* pMaterialText = doc.NewText(mMaterial.c_str());
    pMaterial->LinkEndChild(pMaterialText);
    pBaseElement->LinkEndChild(pMaterial);

    // rigid body transform
    XMLElement* pInitialTransform = doc.NewElement("RigidBodyTransform");

    // initial transform -> position
    XMLElement* pPosition = doc.NewElement("Position");
    pPosition->SetAttribute("x", eastl::to_string(mRigidBodyLocation[0]).c_str());
    pPosition->SetAttribute("y", eastl::to_string(mRigidBodyLocation[1]).c_str());
    pPosition->SetAttribute("z", eastl::to_string(mRigidBodyLocation[2]).c_str());
    pInitialTransform->LinkEndChild(pPosition);

    // initial transform -> orientation
    XMLElement* pOrientation = doc.NewElement("Orientation");
    pOrientation->SetAttribute("yaw", eastl::to_string(mRigidBodyOrientation[0]).c_str());
    pOrientation->SetAttribute("pitch", eastl::to_string(mRigidBodyOrientation[1]).c_str());
    pOrientation->SetAttribute("roll", eastl::to_string(mRigidBodyOrientation[2]).c_str());
    pInitialTransform->LinkEndChild(pOrientation);

	// initial transform -> scale 
    XMLElement* pScale = doc.NewElement("Scale");
    pScale->SetAttribute("x", eastl::to_string(mRigidBodyScale[0]).c_str());
    pScale->SetAttribute("y", eastl::to_string(mRigidBodyScale[1]).c_str());
    pScale->SetAttribute("z", eastl::to_string(mRigidBodyScale[2]).c_str());
    pInitialTransform->LinkEndChild(pScale);

    pBaseElement->LinkEndChild(pInitialTransform);

    return pBaseElement;
}

void PhysicComponent::PostInit(void)
{
    if (mOwner)
    {
		GameApplication* gameApp = (GameApplication*)Application::App;
		BaseGamePhysic* gamePhysics = gameApp->mGame->GetGamePhysics().get();

		if (mShape == "Sphere")
		{
			gamePhysics->AddSphere((float)mRigidBodyScale[0], mOwner, mDensity, mMaterial);
		}
		else if (mShape == "Box")
		{
			gamePhysics->AddBox(mRigidBodyScale, mOwner, mDensity, mMaterial);
		}
		else if (mShape == "PointCloud")
		{
			LogError("Not supported yet!");
		}
	}
}

void PhysicComponent::Update(int deltaMs)
{
    // get the transform component
    eastl::shared_ptr<TransformComponent> pTransformComponent(
		mOwner->GetComponent<TransformComponent>(TransformComponent::Name));
    if (!pTransformComponent)
    {
        LogError("No transform component!");
        return;
    }

	GameApplication* gameApp = (GameApplication*)Application::App;
	BaseGamePhysic* gamePhysics = gameApp->mGame->GetGamePhysics().get();
	if (mAcceleration != 0)
    {
        // calculate the acceleration this frame

        float accelerationPerFrame = mAcceleration / 1000.f * (float)deltaMs;

        // Get the current velocity vector and convert to a scalar. The velocity vector 
		// is a combination of the direction this actor is going in and the speed of the 
		// actor. The scalar is just the speed component.
        Vector3<float> velocity(gamePhysics->GetVelocity(mOwner->GetId()));
        float velocityScalar = Length(velocity);

		EulerAngles<float> rotation;
		// get the direction the object is facing
		Transform transform = pTransformComponent->GetTransform();
		transform.GetRotation(rotation);

		Vector3<float> direction{ rotation.angle[0], rotation.angle[1], rotation.angle[2] };
		gamePhysics->ApplyForce(direction, accelerationPerFrame, mOwner->GetId());

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

    if (mAngularAcceleration != 0)
    {
        // calculate the acceleration this frame
		Vector3<float> upVector{ 0.f,1.f,0.f };
        float angularAccelerationToApplyThisFrame = mAngularAcceleration / 1000.f * (float)deltaMs;
		gamePhysics->ApplyTorque(upVector, angularAccelerationToApplyThisFrame, mOwner->GetId());

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
		mRigidBodyLocation = Vector3<float>{ x, y, z };
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
		mRigidBodyOrientation = Vector3<float>{ yaw, pitch, roll };
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
		mRigidBodyScale = Vector3<float>{ x, y, z };
    }
}

void PhysicComponent::ApplyForce(const Vector3<float>& direction, float forceNewtons)
{
	GameApplication* gameApp = (GameApplication*)Application::App;
	BaseGamePhysic* gamePhysics = gameApp->mGame->GetGamePhysics().get();
    gamePhysics->ApplyForce(direction, forceNewtons, mOwner->GetId());
}

void PhysicComponent::ApplyTorque(const Vector3<float>& direction, float forceNewtons)
{
	GameApplication* gameApp = (GameApplication*)Application::App;
	BaseGamePhysic* gamePhysics = gameApp->mGame->GetGamePhysics().get();
    gamePhysics->ApplyTorque(direction, forceNewtons, mOwner->GetId());
}

bool PhysicComponent::KinematicMove(const Transform &transform)
{
	GameApplication* gameApp = (GameApplication*)Application::App;
	BaseGamePhysic* gamePhysics = gameApp->mGame->GetGamePhysics().get();
	return gamePhysics->KinematicMove(transform, mOwner->GetId());
}

void PhysicComponent::ApplyAcceleration(float acceleration)
{
    mAcceleration = acceleration;
}

void PhysicComponent::RemoveAcceleration(void)
{
    mAcceleration = 0;
}

void PhysicComponent::ApplyAngularAcceleration(float acceleration)
{
    mAngularAcceleration = acceleration;
}

void PhysicComponent::RemoveAngularAcceleration(void)
{
    mAngularAcceleration = 0;
}

Vector3<float> PhysicComponent::GetVelocity(void)
{
	GameApplication* gameApp = (GameApplication*)Application::App;
	BaseGamePhysic* gamePhysics = gameApp->mGame->GetGamePhysics().get();
    return gamePhysics->GetVelocity(mOwner->GetId());
}

void PhysicComponent::SetVelocity(const Vector3<float>& velocity)
{
	GameApplication* gameApp = (GameApplication*)Application::App;
	BaseGamePhysic* gamePhysics = gameApp->mGame->GetGamePhysics().get();
    gamePhysics->SetVelocity(mOwner->GetId(), velocity);
}

void PhysicComponent::RotateY(float angleRadians)
{
    eastl::shared_ptr<TransformComponent> pTransformComponent(
		mOwner->GetComponent<TransformComponent>(TransformComponent::Name));
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
		mOwner->GetComponent<TransformComponent>(TransformComponent::Name));
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
    return gamePhysics->StopActor(mOwner->GetId());
}