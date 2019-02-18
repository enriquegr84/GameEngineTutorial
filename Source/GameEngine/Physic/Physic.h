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

#include "Importer/Bsp/BspLoader.h"

#include "Graphic/Scene/Hierarchy/PVWUpdater.h"

#include "Mathematic/Algebra/AxisAngle.h"
#include "Mathematic/Algebra/Transform.h"
#include "Mathematic/Algebra/Vector3.h"
#include "Mathematic/Geometric/Hyperplane.h"

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
	virtual void AddTrigger(const Vector3<float>& dimensions, 
		eastl::weak_ptr<Actor> pGameActor, const eastl::string& physicMaterial) = 0;
	virtual void AddBSP(BspLoader& bspLoader, eastl::weak_ptr<Actor> actor,
		/*const Matrix4x4<float>& initialTransform, */
		const eastl::string& densityStr, const eastl::string& physicMaterial) = 0;
	virtual void AddCharacterController(const Vector3<float>& dimensions, eastl::weak_ptr<Actor> actor,
		/*const Matrix4x4<float>& initialTransform, */
		const eastl::string& densityStr, const eastl::string& physicMaterial) = 0;
	virtual void AddSphere(float radius, eastl::weak_ptr<Actor> actor, 
		/*const Matrix4x4<float>& initialTransform, */
		const eastl::string& densityStr, const eastl::string& physicMaterial) = 0;
	virtual void AddBox(const Vector3<float>& dimensions, eastl::weak_ptr<Actor> gameActor, 
		/*const Matrix4x4<float>& initialTransform, */
		const eastl::string& densityStr, const eastl::string& physicMaterial) = 0;
	virtual void AddPointCloud(Vector3<float> *verts, int numPoints, eastl::weak_ptr<Actor> gameActor, 
		/*const Matrix4x4<float>& initialTransform, */
		const eastl::string& densityStr, const eastl::string& physicMaterial) = 0;
	virtual void AddPointCloud(Plane3<float> *planes, int numPlanes, eastl::weak_ptr<Actor> gameActor,
		/*const Matrix4x4<float>& initialTransform, */
		const eastl::string& densityStr, const eastl::string& physicMaterial) = 0;
	virtual void RemoveActor(ActorId id) = 0;

	// Debugging
	virtual void RenderDiagnostics() = 0;

	// Physics world modifiers
	virtual void ApplyForce(const Vector3<float> &velocity, ActorId aid) = 0;
	virtual void ApplyTorque(const Vector3<float> &velocity, ActorId aid) = 0;

	// Physics actor states
	virtual bool OnGround(ActorId actorId) = 0;
	virtual void Jump(ActorId actorId, const Vector3<float>& dir) = 0;
	virtual void FallDirection(ActorId actorId, const Vector3<float>& dir) = 0;
	virtual void WalkDirection(ActorId actorId, const Vector3<float>& dir) = 0;

	// Collisions
	virtual bool FindIntersection(ActorId actorId, const Vector3<float>& point) = 0;
	virtual ActorId CastRay(
		const Vector3<float>& origin, const Vector3<float>& end,
		Vector3<float>& collisionPoint, Vector3<float>& collision) = 0;
	virtual void CastRay(
		const Vector3<float>& origin, const Vector3<float>& end,
		eastl::vector<ActorId>& collisionActors,
		eastl::vector<Vector3<float>>& collisionPoints,
		eastl::vector<Vector3<float>>& collisionNormals) = 0;

	virtual void SetIgnoreCollision(ActorId actorId, ActorId ignoreActorId, bool ignoreCollision) = 0;
	virtual void StopActor(ActorId actorId) = 0;
	virtual Vector3<float> GetScale(ActorId actorId) = 0;
	virtual Vector3<float> GetVelocity(ActorId actorId) = 0;
	virtual float GetJumpSpeed(ActorId actorId) = 0;
	virtual void SetGravity(ActorId actorId, const Vector3<float>& g) = 0;
	virtual void SetVelocity(ActorId actorId, const Vector3<float>& vel) = 0;
	virtual void SetPosition(ActorId actorId, const Vector3<float>& pos) = 0;
	virtual void SetRotation(ActorId actorId, const Transform& mat) = 0;
	virtual Vector3<float> GetAngularVelocity(ActorId actorId) = 0;
	virtual void SetAngularVelocity(ActorId actorId, const Vector3<float>& vel) = 0;
	virtual void Translate(ActorId actorId, const Vector3<float>& vec) = 0;
	virtual void SetTransform(const ActorId id, const Transform& mat) = 0;
	virtual Transform GetTransform(const ActorId id) = 0;

	virtual ~BaseGamePhysic() { };
};

extern BaseGamePhysic *CreateGamePhysics();
extern BaseGamePhysic *CreateNullPhysics();

#endif