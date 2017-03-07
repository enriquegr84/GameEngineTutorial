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

#ifndef PHYSICSCOMPONENT_H
#define PHYSICSCOMPONENT_H

class PhysicsComponent : public ActorComponent
{
public:
	const static char *g_Name;
	virtual const char *GetName() const override { return PhysicsComponent::g_Name; }

public:
    PhysicsComponent(void);
    virtual ~PhysicsComponent(void);
    virtual XmlElement* GenerateXml(void) override;

    // ActorComponent interface
    virtual bool Init(XmlElement* pData) override;
    virtual void PostInit(void) override;
    virtual void Update(int deltaMs) override;

    // Physics functions
    void ApplyForce(const Vector3& direction, float forceNewtons);
    void ApplyTorque(const Vector3& direction, float forceNewtons);
	bool KinematicMove(const Matrix4x4& transform);

    // acceleration
    void ApplyAcceleration(float acceleration);
    void RemoveAcceleration(void);
    void ApplyAngularAcceleration(float acceleration);
    void RemoveAngularAcceleration(void);

	//void RotateY(float angleRadians);
    Vector3 GetVelocity(void);
    void SetVelocity(const Vector3& velocity);
    void RotateY(float angleRadians);
    void SetPosition(float x, float y, float z);
    void Stop(void);


protected:
    void CreateShape();
    void BuildRigidBodyTransform(XmlElement* pTransformElement);

    float m_acceleration, m_angularAcceleration;
    float m_maxVelocity, m_maxAngularVelocity;

	eastl::string m_shape;
    eastl::string m_density;
    eastl::string m_material;
	
	Vector3 m_RigidBodyLocation;		// this isn't world position! This is how the rigid body is offset from the position of the actor.
	Vector3 m_RigidBodyOrientation;	// ditto, orientation
	Vector3 m_RigidBodyScale;			// ditto, scale
	
    eastl::shared_ptr<BaseGamePhysic> m_pGamePhysics;  // might be better as a weak ptr...
};


#endif