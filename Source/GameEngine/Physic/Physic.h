//========================================================================
// Physics.h : Implements the BaseGamePhysic interface
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

#ifndef PHYSIC_H
#define PHYSIC_H

#include "GameEngineStd.h"

#include "Mathematic/Algebra/Matrix4x4.h"

/////////////////////////////////////////////////////////////////////////////
// class BaseGamePhysic							- Chapter 17, page 589
//
//   The interface defintion for a generic physics API.
/////////////////////////////////////////////////////////////////////////////
class BaseGamePhysic
{
public:

	// Initialiazation and Maintenance of the Physics World
	virtual bool Initialize() = 0;
	virtual void SyncVisibleScene() = 0;
	virtual void OnUpdate(float deltaSeconds) = 0;

	// Initialization of Physics Objects
	virtual void AddSphere(float radius, eastl::weak_ptr<Actor> actor, 
		/*const Matrix4x4<float>& initialTransform, */
		const eastl::string& densityStr, const eastl::string& physicMaterial) = 0;
	virtual void AddBox(const Vector3<float>& dimensions, eastl::weak_ptr<Actor> gameActor, 
		/*const Matrix4x4<float>& initialTransform, */
		const eastl::string& densityStr, const eastl::string& physicMaterial) = 0;
	virtual void AddPointCloud(Vector3<float> *verts, int numPoints, eastl::weak_ptr<Actor> gameActor, 
		/*const Matrix4x4<float>& initialTransform, */
		const eastl::string& densityStr, const eastl::string& physicMaterial) = 0;

	virtual void RemoveActor(ActorId id) = 0;

	// Debugging
	virtual void RenderDiagnostics() = 0;

	// Physics world modifiers
	virtual void CreateTrigger(eastl::weak_ptr<Actor> pGameActor, const Vector3<float> &pos, const float dim) = 0;
	virtual void ApplyForce(const Vector3<float> &dir, float newtons, ActorId aid) = 0;
	virtual void ApplyTorque(const Vector3<float> &dir, float newtons, ActorId aid) = 0;
	virtual bool KinematicMove(const Matrix4x4<float> &mat, ActorId aid) = 0;

	// Physics actor states
	virtual void RotateY(ActorId actorId, float angleRadians, float time) = 0;
	virtual float GetOrientationY(ActorId actorId) = 0;
	virtual void StopActor(ActorId actorId) = 0;
	virtual Vector3<float> GetVelocity(ActorId actorId) = 0;
	virtual void SetVelocity(ActorId actorId, const Vector3<float>& vel) = 0;
	virtual Vector3<float> GetAngularVelocity(ActorId actorId) = 0;
	virtual void SetAngularVelocity(ActorId actorId, const Vector3<float>& vel) = 0;
	virtual void Translate(ActorId actorId, const Vector3<float>& vec) = 0;

	virtual void SetTransform(const ActorId id, const Matrix4x4<float>& mat) = 0;
	virtual Matrix4x4<float> GetTransform(const ActorId id) = 0;

	virtual ~BaseGamePhysic() { };
};

extern BaseGamePhysic *CreateGamePhysics();
extern BaseGamePhysic *CreateNullPhysics();

#endif