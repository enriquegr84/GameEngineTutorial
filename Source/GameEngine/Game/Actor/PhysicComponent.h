//========================================================================
// PhysicsComponent.h - Component for physical movement and collision detection
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

#ifndef PHYSICCOMPONENT_H
#define PHYSICCOMPONENT_H

#include "GameEngineStd.h"

#include "ActorComponent.h"

#include "Physic/Physic.h"

#include "Mathematic/Algebra/Vector2.h"
#include "Mathematic/Algebra/Vector3.h"
#include "Mathematic/Algebra/Transform.h"

class PhysicComponent : public ActorComponent
{
public:
	const static char *Name;
	virtual const char *GetName() const override { return PhysicComponent::Name; }

public:
	PhysicComponent(void);
    virtual ~PhysicComponent(void);
    virtual tinyxml2::XMLElement* GenerateXml(void) override;

    // ActorComponent interface
    virtual bool Init(tinyxml2::XMLElement* pData) override;
    virtual void PostInit(void) override;
    virtual void Update(float deltaMs) override;

    // Physics functions
    void ApplyForce(const Vector3<float>& velocity);
    void ApplyTorque(const Vector3<float>& velocity);

    // acceleration
    void ApplyAcceleration(float acceleration);
    void RemoveAcceleration(void);
    void ApplyAngularAcceleration(float acceleration);
    void RemoveAngularAcceleration(void);

	//Character controller
	void KinematicJump(const Vector3<float>& direction);
	void KinematicMove(const Vector3<float>& direction);
	void KinematicFall(const Vector3<float>& direction);
	bool OnGround(void);

	Transform GetTransform(void);
	void SetTransform(const Transform& transform);

    Vector3<float> GetScale(void);
	Vector3<float> GetVelocity(void);
    void SetVelocity(const Vector3<float>& velocity);
    void SetPosition(float x, float y, float z);
	void SetRotation(const Transform& transform);
    void Stop(void);

	eastl::string GetMesh(void) { return mMesh; }
	eastl::string GetShape(void) { return mShape; }
	eastl::string GetDensity(void) { return mDensity; }
	eastl::string GetMaterial(void) { return mMaterial; }
	void SetMesh(eastl::string mesh) { mMesh = mesh; }
	void SetShape(eastl::string shape) { mShape = shape; }
	void SetDensity(eastl::string density) { mDensity = density; }
	void SetMaterial(eastl::string material) { mMaterial = material; }

	Vector3<float> GetScaleOffset(void) { return mRigidBodyScale; }
	Vector3<float> GetPositionOffset(void) { return mRigidBodyLocation; }
	Vector3<float> GetOrientationOffset(void) { return mRigidBodyOrientation; }

protected:
    void BuildRigidBodyTransform(tinyxml2::XMLElement* pTransformElement);

    float mAcceleration, mAngularAcceleration;
    float mMaxVelocity, mMaxAngularVelocity;

	eastl::string mMesh;
	eastl::string mShape;
    eastl::string mDensity;
    eastl::string mMaterial;
	
	Vector3<float> mRigidBodyLocation; // rigid body is offset from the position of the actor.
	Vector3<float> mRigidBodyOrientation;	// ditto, orientation
	Vector3<float> mRigidBodyScale;			// ditto, scale
};


#endif