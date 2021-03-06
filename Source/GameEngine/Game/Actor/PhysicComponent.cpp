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
#include "TransformComponent.h"

#include "Physic/Importer/PhysicResource.h"

#include "Game/GameLogic.h"

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
	BaseGamePhysic* gamePhysics = GameLogic::Get()->GetGamePhysics().get();
	gamePhysics->RemoveActor(mOwner->GetId());
}

bool PhysicComponent::Init(tinyxml2::XMLElement* pData)
{
    // no point in having a physics component with no game physics
	BaseGamePhysic* gamePhysics = GameLogic::Get()->GetGamePhysics().get();
    if (!gamePhysics)
        return false;

    // shape
	tinyxml2::XMLElement* pShape = pData->FirstChildElement("Shape");
    if (pShape)
		mShape = pShape->FirstChild()->Value();

    // density
	tinyxml2::XMLElement* pDensity = pData->FirstChildElement("Density");
    if (pDensity)
        mDensity = pDensity->FirstChild()->Value();

    // material
	tinyxml2::XMLElement* pMaterial = pData->FirstChildElement("PhysicsMaterial");
    if (pMaterial)
        mMaterial = pMaterial->FirstChild()->Value();

    // initial transform
	tinyxml2::XMLElement* pRigidBodyTransform = pData->FirstChildElement("RigidBodyTransform");
    if (pRigidBodyTransform)
        BuildRigidBodyTransform(pRigidBodyTransform);

	// physic mesh
	tinyxml2::XMLElement* pMesh = pData->FirstChildElement("Mesh");
	if (pMesh)
		mMesh = pMesh->FirstChild()->Value();

    return true;
}

tinyxml2::XMLElement* PhysicComponent::GenerateXml(void)
{
	tinyxml2::XMLDocument doc;

	// base element
	tinyxml2::XMLElement* pBaseElement = doc.NewElement(GetName());

    // shape
	tinyxml2::XMLElement* pShape = doc.NewElement("Shape");
	tinyxml2::XMLText* pShapeText = doc.NewText(mShape.c_str());
    pShape->LinkEndChild(pShapeText);
	pBaseElement->LinkEndChild(pShape);

    // density
	tinyxml2::XMLElement* pDensity = doc.NewElement("Density");
	tinyxml2::XMLText* pDensityText = doc.NewText(mDensity.c_str());
    pDensity->LinkEndChild(pDensityText);
    pBaseElement->LinkEndChild(pDensity);

    // material
	tinyxml2::XMLElement* pMaterial = doc.NewElement("Material");
	tinyxml2::XMLText* pMaterialText = doc.NewText(mMaterial.c_str());
    pMaterial->LinkEndChild(pMaterialText);
    pBaseElement->LinkEndChild(pMaterial);

    // rigid body transform
	tinyxml2::XMLElement* pInitialTransform = doc.NewElement("RigidBodyTransform");

    // initial transform -> position
	tinyxml2::XMLElement* pPosition = doc.NewElement("Position");
    pPosition->SetAttribute("x", eastl::to_string(mRigidBodyLocation[0]).c_str());
    pPosition->SetAttribute("y", eastl::to_string(mRigidBodyLocation[1]).c_str());
    pPosition->SetAttribute("z", eastl::to_string(mRigidBodyLocation[2]).c_str());
    pInitialTransform->LinkEndChild(pPosition);

    // initial transform -> orientation
	tinyxml2::XMLElement* pOrientation = doc.NewElement("YawPitchRoll");
    pOrientation->SetAttribute("x", eastl::to_string(mRigidBodyOrientation[0]).c_str());
    pOrientation->SetAttribute("y", eastl::to_string(mRigidBodyOrientation[1]).c_str());
    pOrientation->SetAttribute("z", eastl::to_string(mRigidBodyOrientation[2]).c_str());
    pInitialTransform->LinkEndChild(pOrientation);

	// initial transform -> scale 
	tinyxml2::XMLElement* pScale = doc.NewElement("Scale");
    pScale->SetAttribute("x", eastl::to_string(mRigidBodyScale[0]).c_str());
    pScale->SetAttribute("y", eastl::to_string(mRigidBodyScale[1]).c_str());
    pScale->SetAttribute("z", eastl::to_string(mRigidBodyScale[2]).c_str());
    pInitialTransform->LinkEndChild(pScale);

    pBaseElement->LinkEndChild(pInitialTransform);

	// mesh
	tinyxml2::XMLElement* pMesh = doc.NewElement("Mesh");
	tinyxml2::XMLText* pMeshText = doc.NewText(mMesh.c_str());
	pMesh->LinkEndChild(pMeshText);
	pBaseElement->LinkEndChild(pMesh);

    return pBaseElement;
}

void PhysicComponent::PostInit(void)
{
    if (mOwner)
    {
		BaseGamePhysic* gamePhysics = GameLogic::Get()->GetGamePhysics().get();
		if (mShape == "Sphere")
		{
			gamePhysics->AddSphere((float)mRigidBodyScale[0], mOwner, mDensity, mMaterial);
		}
		else if (mShape == "Box")
		{
			gamePhysics->AddBox(mRigidBodyScale, mOwner, mDensity, mMaterial);
		}
		else if (mShape == "Trigger")
		{
			gamePhysics->AddTrigger(mRigidBodyScale, mOwner, mMaterial);
		}
		else if (mShape == "Controller")
		{
			gamePhysics->AddCharacterController(mRigidBodyScale, mOwner, mDensity, mMaterial);
		}
		else if (mShape == "BSP")
		{
			eastl::shared_ptr<ResHandle>& resHandle =
				ResCache::Get()->GetHandle(&BaseResource(ToWideString(mMesh.c_str())));
			if (resHandle)
			{
				const eastl::shared_ptr<BspResourceExtraData>& extra =
					eastl::static_pointer_cast<BspResourceExtraData>(resHandle->GetExtra());
				gamePhysics->AddBSP(extra->GetLoader(), mOwner, mDensity, mMaterial);
			}
		}
		else if (mShape == "PointCloud")
		{
			LogError("Not supported yet!");
		}
	}
}

void PhysicComponent::Update(float deltaMs)
{
    // get the transform component
    eastl::shared_ptr<TransformComponent> pTransformComponent(
		mOwner->GetComponent<TransformComponent>(TransformComponent::Name).lock());
    if (!pTransformComponent)
    {
        LogError("No transform component!");
        return;
    }

	BaseGamePhysic* gamePhysics = GameLogic::Get()->GetGamePhysics().get();
	if (mAcceleration != 0)
    {
        // calculate the acceleration this frame

        float accelerationPerFrame = mAcceleration / 1000.f * (float)deltaMs;

        // Get the current velocity vector and convert to a scalar. The velocity vector 
		// is a combination of the direction this actor is going in and the speed of the 
		// actor. The scalar is just the speed component.
        Vector3<float> velocity(gamePhysics->GetVelocity(mOwner->GetId()));
        float velocityScalar = Length(velocity);

		Vector4<float> direction;
		// get the direction the object is facing
#if defined(GE_USE_MAT_VEC)
		direction = GetTransform() * direction;
#else
		direction = direction * GetTransform();
#endif
		gamePhysics->ApplyForce(mOwner->GetId(), HProject(direction) * accelerationPerFrame);
    }

    if (mAngularAcceleration != 0)
    {
        // calculate the acceleration this frame
		Vector3<float> upVector = Vector3<float>::Unit(2); // up vector
        float accelerationPerFrame = mAngularAcceleration / 1000.f * (float)deltaMs;
		gamePhysics->ApplyTorque(mOwner->GetId(), upVector * accelerationPerFrame);
    }
}

void PhysicComponent::BuildRigidBodyTransform(tinyxml2::XMLElement* pTransformElement)
{
	tinyxml2::XMLElement* pPositionElement = pTransformElement->FirstChildElement("Position");
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

	tinyxml2::XMLElement* pOrientationElement = pTransformElement->FirstChildElement("YawPitchRoll");
    if (pOrientationElement)
    {
		float yaw = 0;
		float pitch = 0;
		float roll = 0;
        roll = pOrientationElement->FloatAttribute("x", roll);
		pitch = pOrientationElement->FloatAttribute("y", pitch);
		yaw = pOrientationElement->FloatAttribute("z", yaw);
		mRigidBodyOrientation = Vector3<float>{ roll, pitch, yaw };
    }

	tinyxml2::XMLElement* pScaleElement = pTransformElement->FirstChildElement("Scale");
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

void PhysicComponent::ApplyForce(const Vector3<float>& velocity)
{
	BaseGamePhysic* gamePhysics = GameLogic::Get()->GetGamePhysics().get();
    gamePhysics->ApplyForce(mOwner->GetId(), velocity);
}

void PhysicComponent::ApplyTorque(const Vector3<float>& velocity)
{
	BaseGamePhysic* gamePhysics = GameLogic::Get()->GetGamePhysics().get();
    gamePhysics->ApplyTorque(mOwner->GetId(), velocity);
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

void PhysicComponent::KinematicJump(const Vector3<float>& direction)
{
	BaseGamePhysic* gamePhysics = GameLogic::Get()->GetGamePhysics().get();
	gamePhysics->Jump(mOwner->GetId(), direction);
}

void PhysicComponent::KinematicMove(const Vector3<float>& direction)
{
	BaseGamePhysic* gamePhysics = GameLogic::Get()->GetGamePhysics().get();
	gamePhysics->WalkDirection(mOwner->GetId(), direction);
}

void PhysicComponent::KinematicFall(const Vector3<float>& direction)
{
	BaseGamePhysic* gamePhysics = GameLogic::Get()->GetGamePhysics().get();
	gamePhysics->FallDirection(mOwner->GetId(), direction);
}

bool PhysicComponent::OnGround()
{
	BaseGamePhysic* gamePhysics = GameLogic::Get()->GetGamePhysics().get();
	return gamePhysics->OnGround(mOwner->GetId());
}

Vector3<float> PhysicComponent::GetCenter(void)
{
	BaseGamePhysic* gamePhysics = GameLogic::Get()->GetGamePhysics().get();
	return gamePhysics->GetCenter(mOwner->GetId());
}

Vector3<float> PhysicComponent::GetScale(void)
{
	BaseGamePhysic* gamePhysics = GameLogic::Get()->GetGamePhysics().get();
	return gamePhysics->GetScale(mOwner->GetId());
}

Vector3<float> PhysicComponent::GetVelocity(void)
{
	BaseGamePhysic* gamePhysics = GameLogic::Get()->GetGamePhysics().get();
    return gamePhysics->GetVelocity(mOwner->GetId());
}

float PhysicComponent::GetJumpSpeed()
{
	BaseGamePhysic* gamePhysics = GameLogic::Get()->GetGamePhysics().get();
	return gamePhysics->GetJumpSpeed(mOwner->GetId());
}

void PhysicComponent::SetGravity(const Vector3<float>& gravity)
{
	BaseGamePhysic* gamePhysics = GameLogic::Get()->GetGamePhysics().get();
	gamePhysics->SetGravity(mOwner->GetId(), gravity);
}

void PhysicComponent::SetVelocity(const Vector3<float>& velocity)
{
	BaseGamePhysic* gamePhysics = GameLogic::Get()->GetGamePhysics().get();
    gamePhysics->SetVelocity(mOwner->GetId(), velocity);
}

Transform PhysicComponent::GetTransform(void)
{
	BaseGamePhysic* gamePhysics = GameLogic::Get()->GetGamePhysics().get();
	return gamePhysics->GetTransform(mOwner->GetId());
}

void PhysicComponent::SetTransform(const Transform& transform)
{
	BaseGamePhysic* gamePhysics = GameLogic::Get()->GetGamePhysics().get();
	gamePhysics->SetTransform(mOwner->GetId(), transform);
}

void PhysicComponent::SetRotation(const Transform& transform)
{
	BaseGamePhysic* gamePhysics = GameLogic::Get()->GetGamePhysics().get();
	gamePhysics->SetRotation(mOwner->GetId(), transform);
}

void PhysicComponent::SetPosition(float x, float y, float z)
{
	BaseGamePhysic* gamePhysics = GameLogic::Get()->GetGamePhysics().get();
	gamePhysics->SetPosition(mOwner->GetId(), Vector3<float>{x, y, z});
}

void PhysicComponent::Stop(void)
{
	BaseGamePhysic* gamePhysics = GameLogic::Get()->GetGamePhysics().get();
	gamePhysics->StopActor(mOwner->GetId());
}

void PhysicComponent::SetIgnoreCollision(ActorId actorId, bool ignoreCollision)
{
	BaseGamePhysic* gamePhysics = GameLogic::Get()->GetGamePhysics().get();
	gamePhysics->SetIgnoreCollision(mOwner->GetId(), actorId, ignoreCollision);
}