//========================================================================
// Physics.cpp : Implements the BaseGamePhysic interface with Bullet
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

 
#include "Physic.h"

#include "PhysicDebugDrawer.h"
#include "PhysicEventListener.h"

#include "Importer/Bsp/BspLoader.h"
#include "Importer/Bsp/BspConverter.h"

#include "Game/Actor/Actor.h"
#include "Game/Actor/TransformComponent.h"

#include "Core/IO/XmlResource.h"
#include "Core/Event/EventManager.h"
#include "Core/Event/Event.h"

#include "Application/GameApplication.h"

#include "LinearMath/btGeometryUtil.h"

#include "btBulletDynamicsCommon.h"
#include "btBulletCollisionCommon.h"
#include "BulletCollision/Gimpact/btBoxCollision.h"
#include "BulletCollision/CollisionDispatch/btGhostObject.h"
#include "BulletCollision/NarrowPhaseCollision/btRaycastCallback.h"
#include "BulletDynamics/Character/btKinematicCharacterController.h"

/////////////////////////////////////////////////////////////////////////////
//   Materials Description						- Chapter 17, page 579
//
//   Predefines some useful physics materials. Define new ones here, and 
//   have similar objects use it, so if you ever need to change it you'll 
//   only have to change it here.
//
/////////////////////////////////////////////////////////////////////////////
struct MaterialData
{
	float mRestitution;
	float mFriction;

    MaterialData(float restitution, float friction)
    {
        mRestitution = restitution;
        mFriction = friction;
    }

    MaterialData(const MaterialData& other)
    {
        mRestitution = other.mRestitution;
        mFriction = other.mFriction;
    }
};


/////////////////////////////////////////////////////////////////////////////////////////////////////////
// a physics implementation which does nothing.  used if physics is disabled.
//
class NullPhysics : public BaseGamePhysic
{
public:
	NullPhysics() { }
	virtual ~NullPhysics() { }

	// Initialization and Maintenance of the Physics World
	virtual bool Initialize() { return true; }
	virtual void SyncVisibleScene() { };
	virtual void OnUpdate( float ) { }

	// Initialization of Physics Objects
	virtual void AddTrigger(const Vector3<float>& dimensions, 
		eastl::weak_ptr<Actor> pGameActor, const eastl::string& physicMaterial) { }
	virtual void AddBSP(BspLoader& bspLoader, eastl::weak_ptr<Actor> actor,
		const eastl::string& densityStr, const eastl::string& physicMaterial) { }
	virtual void AddCharacterController(const Vector3<float>& dimensions, eastl::weak_ptr<Actor> actor,
		const eastl::string& densityStr, const eastl::string& physicMaterial) { }
	virtual void AddSphere(float radius, eastl::weak_ptr<Actor> gameActor, 
		const eastl::string& densityStr, const eastl::string& physicMaterial) { }
	virtual void AddBox(const Vector3<float>& dimensions, eastl::weak_ptr<Actor> gameActor,
		const eastl::string& densityStr, const eastl::string& physicMaterial) { }
	virtual void AddPointCloud(Vector3<float> *verts, int numPoints, eastl::weak_ptr<Actor> gameActor,
		const eastl::string& densityStr, const eastl::string& physicMaterial) { }
	virtual void AddPointCloud(Plane3<float> *planes, int numPlanes, eastl::weak_ptr<Actor> gameActor,
		const eastl::string& densityStr, const eastl::string& physicMaterial) { }
	virtual void RemoveActor(ActorId id) { }

	// Debugging
	virtual void RenderDiagnostics() { }

	// Physics world modifiers
	virtual void ApplyForce(const Vector3<float> &velocity, ActorId aid) { }
	virtual void ApplyTorque(const Vector3<float> &velocity, ActorId aid) { }

	// Physics actor states
	virtual bool OnGround(ActorId actorId) { return false; }
	virtual void Jump(ActorId actorId, const Vector3<float>& dir) { }
	virtual void FallDirection(ActorId actorId, const Vector3<float>& dir) { }
	virtual void WalkDirection(ActorId actorId, const Vector3<float>& dir) { }

	// Collisions
	virtual bool FindIntersection(ActorId actorId, const Vector3<float>& point) { return false; }
	virtual ActorId ConvexSweep(
		ActorId aId, const Transform& origin, const Transform& end,
		Vector3<float>& collisionPoint, Vector3<float>& collisionNormal) { return INVALID_ACTOR_ID; }
	virtual ActorId CastRay(
		const Vector3<float>& origin, const Vector3<float>& end,
		Vector3<float>& collisionPoint, Vector3<float>& collisionNormal) { return INVALID_ACTOR_ID; }
	virtual void CastRay(
		const Vector3<float>& origin, const Vector3<float>& end,
		eastl::vector<ActorId>& collisionActors,
		eastl::vector<Vector3<float>>& collisionPoints,
		eastl::vector<Vector3<float>>& collisionNormals) { }

	virtual void SetIgnoreCollision(ActorId actorId, ActorId ignoreActorId, bool ignoreCollision) { }
	virtual void StopActor(ActorId actorId) { }
	virtual Vector3<float> GetScale(ActorId actorId) { return Vector3<float>(); }
    virtual Vector3<float> GetVelocity(ActorId actorId) { return Vector3<float>(); }
	virtual float GetJumpSpeed(ActorId actorId) { return 0; }
	virtual void SetGravity(ActorId actorId, const Vector3<float>& g) { }
    virtual void SetVelocity(ActorId actorId, const Vector3<float>& vel) { }
	virtual void SetPosition(ActorId actorId, const Vector3<float>& pos) { }
	virtual void SetRotation(ActorId aid, const Transform &mat) { }
    virtual Vector3<float> GetAngularVelocity(ActorId actorId) { return Vector3<float>(); }
    virtual void SetAngularVelocity(ActorId actorId, const Vector3<float>& vel) { }
	virtual void Translate(ActorId actorId, const Vector3<float>& vec) { }
	virtual void SetTransform(const ActorId id, const Transform& mat) { }
    virtual Transform GetTransform(const ActorId id) { return Transform::Identity; }
};


/////////////////////////////////////////////////////////////////////////////
// helpers for conversion to and from Bullet's data types
static btVector3 Vector3TobtVector3( Vector3<float> const & vector3 )
{
	return btVector3(vector3[0], vector3[1], vector3[2] );
}

static Vector3<float> btVector3ToVector3( btVector3 const & btvec )
{
	return Vector3<float>{ btvec.x(), btvec.y(), btvec.z() };
}

static btTransform TransformTobtTransform( Transform const & transform)
{
	// convert from matrix4 (GameEngine) to btTransform (Bullet)
	btMatrix3x3 bulletRotation;
	btVector3 bulletPosition;
	
	// copy transform matrix
	Matrix4x4<float> transformMatrix = transform.GetRotation();
	for ( int row=0; row<3; ++row )
		for ( int column=0; column<3; ++column )
			bulletRotation[row][column] = transformMatrix(row, column);
			// note the reversed indexing (row/column vs. column/row)
			//  this is because matrix4s are row-major matrices and
			//  btMatrix3x3 are column-major.  This reversed indexing
			//  implicitly transposes (flips along the diagonal) 
			//  the matrix when it is copied.
	
	// copy position
	Vector3<float> translation = transform.GetTranslation();
	for ( int column=0; column<3; ++column )
		bulletPosition[column] = translation[column];
		
	return btTransform( bulletRotation, bulletPosition );
}

static Transform btTransformToTransform( btTransform const & trans )
{
	Transform returnTransform;

	// convert from btTransform (Bullet) to matrix4 (GameEngine)
	btMatrix3x3 const & bulletRotation = trans.getBasis();
	btVector3 const & bulletPosition = trans.getOrigin();
	
	// copy transform matrix
	Matrix4x4<float> transformMatrix = Matrix4x4<float>::Identity();
	for ( int row=0; row<3; ++row )
		for ( int column=0; column<3; ++column )
			transformMatrix(row,column) = bulletRotation[row][column];
			// note the reversed indexing (row/column vs. column/row)
			//  this is because matrix4s are row-major matrices and
			//  btMatrix3x3 are column-major.  This reversed indexing
			//  implicitly transposes (flips along the diagonal) 
			//  the matrix when it is copied.
	
	// copy position
	Vector3<float> translationVector;
	for (int column = 0; column<3; ++column)
		translationVector[column] = bulletPosition[column];

	returnTransform.SetRotation(transformMatrix);
	returnTransform.SetTranslation(translationVector);
	return returnTransform;
}

/////////////////////////////////////////////////////////////////////////////
// struct ActorMotionState						- Chapter 17, page 597
//
// Interface that Bullet uses to communicate position and orientation changes
//   back to the game.  note:  this assumes that the actor's center of mass
//   and world position are the same point.  If that was not the case,
//   an additional transformation would need to be stored here to represent
//   that difference.
//
struct ActorMotionState : public btMotionState
{
	Transform mWorldToPositionTransform;
	
	ActorMotionState(Transform const & startingTransform)
	  : mWorldToPositionTransform( startingTransform ) 
	{

	}
	
	// btMotionState interface:  Bullet calls these
	virtual void getWorldTransform( btTransform& worldTrans ) const
	{ 
		worldTrans = TransformTobtTransform( mWorldToPositionTransform ); 
	}

	virtual void setWorldTransform( const btTransform& worldTrans )
	{ 
		mWorldToPositionTransform = btTransformToTransform( worldTrans ); 
	}
};

// forward declaration
class BspToBulletConverter;

/////////////////////////////////////////////////////////////////////////////
// BaseGamePhysic								- Chapter 17, page 590
//
//   The implementation of BaseGamePhysic interface using the Bullet SDK.
//
/////////////////////////////////////////////////////////////////////////////

class BulletPhysics : public BaseGamePhysic
{
	friend class BspToBulletConverter;

	// use auto pointers to automatically call delete on these objects
	//   during ~BulletPhysics
	
	// these are all of the objects that Bullet uses to do its work.
	//   see BulletPhysics::VInitialize() for some more info.
	btDiscreteDynamicsWorld*			mDynamicsWorld;
	btBroadphaseInterface*				mBroadphase;
	btCollisionDispatcher*				mDispatcher;
	btConstraintSolver*					mSolver;
	btDefaultCollisionConfiguration*	mCollisionConfiguration;
	BulletDebugDrawer*					mDebugDrawer;

    // tables read from the XML
    typedef eastl::map<eastl::string, float> DensityTable;
    typedef eastl::map<eastl::string, MaterialData> MaterialTable;
    DensityTable mDensityTable;
    MaterialTable mMaterialTable;

	void LoadXml();
    float LookupSpecificGravity(const eastl::string& densityStr);
    MaterialData LookupMaterialData(const eastl::string& materialStr);

	// keep track of the existing actions:  To check them for updates
	//   to the actors' positions, and to remove them when their lives are over.
	typedef eastl::map<ActorId, btActionInterface*> ActorIDToBulletActionMap;
	ActorIDToBulletActionMap mActorIdToAction;
	btActionInterface * FindBulletAction(ActorId id) const;

	// keep track of the existing collision objects:  To check them for updates
	//   to the actors' positions, and to remove them when their lives are over.
	typedef eastl::map<ActorId, btCollisionObject*> ActorIDToBulletCollisionObjectMap;
	ActorIDToBulletCollisionObjectMap mActorIdToCollisionObject;
	btCollisionObject * FindBulletCollisionObject( ActorId id ) const;
	
	// also keep a map to get the actor id from the btCollisionObject*
	typedef eastl::map<btCollisionObject const *, ActorId> BulletCollisionObjectToActorIDMap;
	BulletCollisionObjectToActorIDMap mCollisionObjectToActorId;
	ActorId FindActorID(btCollisionObject const * ) const;
	
	// data used to store which collision pair (bodies that are touching) need
	//   Collision events sent.  When a new pair of touching bodies are detected,
	//   they are added to m_previousTickCollisionPairs and an event is sent.
	//   When the pair is no longer detected, they are removed and another event
	//   is sent.
	typedef eastl::pair<btRigidBody const *, btRigidBody const *> CollisionPair;
	typedef eastl::set<CollisionPair> CollisionPairs;
	CollisionPairs mPreviousTickCollisionPairs;
	
	// helpers for sending events relating to collision pairs
	void SendCollisionPairAddEvent( btPersistentManifold const * manifold, 
		btRigidBody const * body0, btRigidBody const * body1 );
	void SendCollisionPairRemoveEvent( btRigidBody const * body0, btRigidBody const * body1 );
	
	// common functionality used by AddSphere, AddBox, etc
	void AddShape(eastl::shared_ptr<Actor> pGameActor, btCollisionShape* shape, 
		float mass, const eastl::string& physicMaterial);

	// helper for cleaning up objects
	void RemoveCollisionObject( btCollisionObject * removeMe );

	// callback from bullet for each physics time step.  set in VInitialize
	static void BulletInternalTickCallback( btDynamicsWorld * const world, btScalar const timeStep );
	
public:
	BulletPhysics();				// [mrmike] This was changed post-press to add event registration!
	virtual ~BulletPhysics();

	// Initialiazation and Maintenance of the Physics World
	virtual bool Initialize() override;
	virtual void SyncVisibleScene() override; 
	virtual void OnUpdate( float deltaSeconds ) override; 

	// Initialization of Physics Objects
	virtual void AddTrigger(const Vector3<float> &dimension, 
		eastl::weak_ptr<Actor> pGameActor, const eastl::string& physicMaterial) override;
	virtual void AddBSP(BspLoader& bspLoader, eastl::weak_ptr<Actor> actor,
		const eastl::string& densityStr, const eastl::string& physicMaterial) override;
	virtual void AddCharacterController(const Vector3<float>& dimensions, eastl::weak_ptr<Actor> actor,
		const eastl::string& densityStr, const eastl::string& physicMaterial) override;
	virtual void AddSphere(float radius, eastl::weak_ptr<Actor> pGameActor, 
		const eastl::string& densityStr, const eastl::string& physicMaterial) override;
	virtual void AddBox(const Vector3<float>& dimensions, eastl::weak_ptr<Actor> pGameActor,
		const eastl::string& densityStr, const eastl::string& physicMaterial) override;
	virtual void AddPointCloud(Vector3<float> *verts, int numPoints, eastl::weak_ptr<Actor> pGameActor,
		const eastl::string& densityStr, const eastl::string& physicMaterial) override;
	virtual void AddPointCloud(Plane3<float> *planes, int numPlanes, eastl::weak_ptr<Actor> pGameActor,
		const eastl::string& densityStr, const eastl::string& physicMaterial) override;
	virtual void RemoveActor(ActorId id) override;

	// Debugging
	virtual void RenderDiagnostics() override;

	// Physics world modifiers
	virtual void ApplyForce(const Vector3<float> &velocity, ActorId aid) override;
	virtual void ApplyTorque(const Vector3<float> &velocity, ActorId aid) override;
	
	virtual bool OnGround(ActorId actorId);
	virtual void Jump(ActorId actorId, const Vector3<float>& dir);
	virtual void FallDirection(ActorId actorId, const Vector3<float>& dir);
	virtual void WalkDirection(ActorId actorId, const Vector3<float>& dir);

	// Collisions
	virtual bool FindIntersection(ActorId actorId, const Vector3<float>& point);
	virtual ActorId ConvexSweep(
		ActorId aId, const Transform& origin, const Transform& end,
		Vector3<float>& collisionPoint, Vector3<float>& collisionNormal);
	virtual ActorId CastRay(
		const Vector3<float>& origin, const Vector3<float>& end,
		Vector3<float>& collisionPoint, Vector3<float>& collisionNormal);
	virtual void CastRay(
		const Vector3<float>& origin, const Vector3<float>& end,
		eastl::vector<ActorId>& collisionActors,
		eastl::vector<Vector3<float>>& collisionPoints,
		eastl::vector<Vector3<float>>& collisionNormals);

	virtual void SetIgnoreCollision(ActorId actorId, ActorId ignoreActorId, bool ignoreCollision);
	virtual void StopActor(ActorId actorId);
	virtual Vector3<float> GetScale(ActorId actorId);
    virtual Vector3<float> GetVelocity(ActorId actorId);
	virtual float GetJumpSpeed(ActorId actorId);
	virtual void SetGravity(ActorId actorId, const Vector3<float>& g);
    virtual void SetVelocity(ActorId actorId, const Vector3<float>& vel);
	virtual void SetPosition(ActorId actorId, const Vector3<float>& pos);
	virtual void SetRotation(ActorId aid, const Transform &mat);
    virtual Vector3<float> GetAngularVelocity(ActorId actorId);
    virtual void SetAngularVelocity(ActorId actorId, const Vector3<float>& vel);
	virtual void Translate(ActorId actorId, const Vector3<float>& vec);

    virtual void SetTransform(const ActorId id, const Transform& mat);
	virtual Transform GetTransform(const ActorId id);
};

///BspToBulletConverter  extends the BspConverter to convert to Bullet datastructures
class BspToBulletConverter : public BspConverter
{
public:

	BspToBulletConverter(BulletPhysics*	physics, eastl::shared_ptr<Actor> pGameActor,
		btScalar mass, const eastl::string& physicMaterial)
		: mPhysics(physics), mGameActor(pGameActor), mMass(mass), mPhysicMaterial(physicMaterial)
	{
		LogAssert(mGameActor, "no actor");
	}

	virtual void AddConvexVerticesCollider(btAlignedObjectArray<btVector3>& vertices)
	{
		///perhaps we can do something special with entities (isEntity)
		///like adding a collision Triggering (as example)
		if (vertices.size() > 0)
		{
			btCollisionShape* shape = new btConvexHullShape(&(vertices[0].getX()), vertices.size());

			// lookup the material
			MaterialData material(mPhysics->LookupMaterialData(mPhysicMaterial));

			// localInertia defines how the object's mass is distributed
			btVector3 localInertia(0.f, 0.f, 0.f);
			if (mMass > 0.f)
				shape->calculateLocalInertia(mMass, localInertia);

			Transform transform;
			eastl::shared_ptr<TransformComponent> pTransformComponent =
				mGameActor->GetComponent<TransformComponent>(TransformComponent::Name).lock();
			LogAssert(pTransformComponent, "no transform");
			if (pTransformComponent)
			{
				transform = pTransformComponent->GetTransform();
			}
			else
			{
				// Physics can't work on an actor that doesn't have a TransformComponent!
				return;
			}

			// set the initial transform of the body from the actor
			ActorMotionState * const motionState = new ActorMotionState(transform);

			btRigidBody::btRigidBodyConstructionInfo rbInfo(mMass, motionState, shape, localInertia);

			// set up the materal properties
			rbInfo.m_restitution = material.mRestitution;
			rbInfo.m_friction = material.mFriction;

			btRigidBody* const body = new btRigidBody(rbInfo);
			mPhysics->mDynamicsWorld->addRigidBody(body);
		}
	}

protected:
	BulletPhysics* mPhysics;
	eastl::shared_ptr<Actor> mGameActor;
	eastl::string mPhysicMaterial;
	btScalar mMass;
};


BulletPhysics::BulletPhysics()
{
	// [mrmike] This was changed post-press to add event registration!
	REGISTER_EVENT(EventDataPhysTriggerEnter);
	REGISTER_EVENT(EventDataPhysTriggerLeave);
	REGISTER_EVENT(EventDataPhysCollision);
	REGISTER_EVENT(EventDataPhysSeparation);
}


/////////////////////////////////////////////////////////////////////////////
// BulletPhysics::~BulletPhysics				- Chapter 17, page 596
//
BulletPhysics::~BulletPhysics()
{
	// delete any physics objects which are still in the world
	
	// iterate backwards because removing the last object doesn't affect the
	//  other objects stored in a vector-type array
	for ( int ii=mDynamicsWorld->getNumCollisionObjects()-1; ii>=0; --ii )
	{
		btCollisionObject * const obj = mDynamicsWorld->getCollisionObjectArray()[ii];
		
		RemoveCollisionObject( obj );
	}
	
	mCollisionObjectToActorId.clear();

	delete mDebugDrawer;
	delete mDynamicsWorld;
	delete mSolver;
	delete mBroadphase;
	delete mDispatcher;
	delete mCollisionConfiguration;
}

/////////////////////////////////////////////////////////////////////////////
// BulletPhysics::LoadXml						- not described in the book
//
//    Loads the physics materials from an XML file
//
void BulletPhysics::LoadXml()
{
    // Load the physics config file and grab the root XML node
	tinyxml2::XMLElement* pRoot = XmlResourceLoader::LoadAndReturnRootXMLElement(L"config\\Physics.xml");
    LogAssert(pRoot, "Physcis xml doesn't exists");

    // load all materials
	tinyxml2::XMLElement* pParentNode = pRoot->FirstChildElement("PhysicsMaterials");
	LogAssert(pParentNode, "No materials");
    for (tinyxml2::XMLElement* pNode = pParentNode->FirstChildElement(); pNode; pNode = pNode->NextSiblingElement())
    {
        double restitution = 0;
        double friction = 0;
		restitution = pNode->DoubleAttribute("restitution", restitution);
		friction = pNode->DoubleAttribute("friction", friction);
        mMaterialTable.insert(eastl::make_pair(
			pNode->Value(), MaterialData((float)restitution, (float)friction)));
    }

    // load all densities
    pParentNode = pRoot->FirstChildElement("DensityTable");
	LogAssert(pParentNode, "No desinty table");
    for (tinyxml2::XMLElement* pNode = pParentNode->FirstChildElement(); pNode; pNode = pNode->NextSiblingElement())
    {
        mDensityTable.insert(eastl::make_pair(pNode->Value(), (float)atof(pNode->FirstChild()->Value())));
    }
}

/////////////////////////////////////////////////////////////////////////////
// BulletPhysics::Initialize					- Chapter 17, page 594
//
bool BulletPhysics::Initialize()
{
	LoadXml();

	// this controls how Bullet does internal memory management during the collision pass
	mCollisionConfiguration = new btDefaultCollisionConfiguration();

	// this manages how Bullet detects precise collisions between pairs of objects
	mDispatcher = new btCollisionDispatcher( mCollisionConfiguration);

	// Bullet uses this to quickly (imprecisely) detect collisions between objects.
	//   Once a possible collision passes the broad phase, it will be passed to the
	//   slower but more precise narrow-phase collision detection (btCollisionDispatcher).
	mBroadphase = new btDbvtBroadphase();

	// Manages constraints which apply forces to the physics simulation.  Used
	//  for e.g. springs, motors.  We don't use any constraints right now.
	mSolver = new btSequentialImpulseConstraintSolver();

	// This is the main Bullet interface point.  Pass in all these components to customize its behavior.
	mDynamicsWorld = new btDiscreteDynamicsWorld( 
		mDispatcher, mBroadphase, mSolver, mCollisionConfiguration );
	mDynamicsWorld->setGravity(btVector3(0, 0, -300.f));

	mDebugDrawer = new BulletDebugDrawer();
	GameApplication* gameApp = (GameApplication*)Application::App;
	mDebugDrawer->ReadOptions(gameApp->mOption.mRoot);

	if(!mCollisionConfiguration || !mDispatcher || !mBroadphase ||
			  !mSolver || !mDynamicsWorld || !mDebugDrawer)
	{
		LogError("BulletPhysics::Initialize failed!");
		return false;
	}

	mDynamicsWorld->setDebugDrawer( mDebugDrawer );
	
	// and set the internal tick callback to our own method "BulletInternalTickCallback"
	mDynamicsWorld->setInternalTickCallback( BulletInternalTickCallback );
	mDynamicsWorld->setWorldUserInfo( this );
	
	return true;
}

/////////////////////////////////////////////////////////////////////////////
// BulletPhysics::OnUpdate						- Chapter 17, page 596
//
void BulletPhysics::OnUpdate( float const deltaSeconds )
{
	// Bullet uses an internal fixed timestep (default 1/60th of a second)
	//   We pass in 4 as a max number of sub steps.  Bullet will run the simulation
	//   in increments of the fixed timestep until "deltaSeconds" amount of time has
	//   passed, but will only run a maximum of 4 steps this way.
	mDynamicsWorld->stepSimulation(deltaSeconds, 4 );
}

/////////////////////////////////////////////////////////////////////////////
// BulletPhysics::SyncVisibleScene				- Chapter 17, page 598
//
void BulletPhysics::SyncVisibleScene()
{
	// Keep physics & graphics in sync

	// check all the existing actor's collision object for changes. 
	//  If there is a change, send the appropriate event for the game system.
	for (	ActorIDToBulletCollisionObjectMap::const_iterator it = mActorIdToCollisionObject.begin();
			it != mActorIdToCollisionObject.end(); ++it )
	{ 
		ActorId const id = it->first;
		btCollisionObject* actorCollisionObject = it->second;
		
		eastl::shared_ptr<Actor> pGameActor(GameLogic::Get()->GetActor(id).lock());
		if (pGameActor)
		{
            eastl::shared_ptr<TransformComponent> pTransformComponent(
				pGameActor->GetComponent<TransformComponent>(TransformComponent::Name).lock());
            if (pTransformComponent)
            {
				Transform actorTransform = 
					btTransformToTransform(actorCollisionObject->getWorldTransform());

			    if (pTransformComponent->GetTransform().GetMatrix() != actorTransform.GetMatrix() ||
					pTransformComponent->GetTransform().GetTranslation() != actorTransform.GetTranslation())
                {
                    // Bullet has moved the actor's physics object. Sync and inform
					// about game actor transform 
					//pTransformComponent->SetTransform(actorTransform);
/*
					LogInformation("x = " + eastl::to_string(actorTransform.GetTranslation()[0]) +
									" y = " + eastl::to_string(actorTransform.GetTranslation()[1]) +
									" z = " + eastl::to_string(actorTransform.GetTranslation()[2]));
*/
                    eastl::shared_ptr<EventDataSyncActor> pEvent(new EventDataSyncActor(id, actorTransform));
                    BaseEventManager::Get()->TriggerEvent(pEvent);
			    }
            }
		}
	}
}

/////////////////////////////////////////////////////////////////////////////
// BulletPhysics::AddShape						- Chapter 17, page 600
//
void BulletPhysics::AddShape(eastl::shared_ptr<Actor> pGameActor, btCollisionShape* shape,
	float mass, const eastl::string& physicMaterial)
{
    LogAssert(pGameActor, "no actor");

    ActorId actorID = pGameActor->GetId();
	LogAssert(mActorIdToCollisionObject.find( actorID ) == mActorIdToCollisionObject.end(),
		"Actor with more than one physics body?");

    // lookup the material
    MaterialData material(LookupMaterialData(physicMaterial));

	// localInertia defines how the object's mass is distributed
	btVector3 localInertia( 0.f, 0.f, 0.f );
	if ( mass > 0.f )
		shape->calculateLocalInertia( mass, localInertia );

	Transform transform;
    eastl::shared_ptr<TransformComponent> pTransformComponent = 
		pGameActor->GetComponent<TransformComponent>(TransformComponent::Name).lock();
	LogAssert(pTransformComponent, "no transform");
    if (pTransformComponent)
    {
		transform = pTransformComponent->GetTransform();
	}
	else
	{
		// Physics can't work on an actor that doesn't have a TransformComponent!
		return;
	}

	// set the initial transform of the body from the actor
	ActorMotionState * const motionState = new ActorMotionState(transform);
	
	btRigidBody::btRigidBodyConstructionInfo rbInfo( mass, motionState, shape, localInertia );
	
	// set up the materal properties
	rbInfo.m_restitution = material.mRestitution;
	rbInfo.m_friction    = material.mFriction;
	
	btRigidBody* const body = new btRigidBody(rbInfo);

	mDynamicsWorld->addRigidBody( body );
	
	// add it to the collection to be checked for changes in SyncVisibleScene
	mActorIdToCollisionObject[actorID] = body;
	mCollisionObjectToActorId[body] = actorID;
}

/////////////////////////////////////////////////////////////////////////////
// BulletPhysics::RemoveCollisionObject			- not described in the book
//
//    Removes a collision object from the game world
//
void BulletPhysics::RemoveCollisionObject( btCollisionObject * const removeMe )
{
	// first remove the object from the physics sim
	mDynamicsWorld->removeCollisionObject( removeMe );
	
	// then remove the pointer from the ongoing contacts list
	for ( CollisionPairs::iterator it = mPreviousTickCollisionPairs.begin();
	      it != mPreviousTickCollisionPairs.end(); )
    {
		CollisionPairs::iterator next = it;
		++next;
		
		if ( it->first == removeMe || it->second == removeMe )
		{
			SendCollisionPairRemoveEvent( it->first, it->second );
			mPreviousTickCollisionPairs.erase( it );
		}
		
		it = next;
    }
	
	// if the object is a RigidBody (all of ours are RigidBodies, but it's good to be safe)
	if ( btRigidBody * const body = btRigidBody::upcast(removeMe) )
	{
		// delete the components of the object
		delete body->getMotionState();
		delete body->getCollisionShape();
		delete body->getUserPointer();
		delete body->getUserPointer();
		
		for ( int ii=body->getNumConstraintRefs()-1; ii >= 0; --ii )
		{
			btTypedConstraint * const constraint = body->getConstraintRef( ii );
			mDynamicsWorld->removeConstraint( constraint );
			delete constraint;
		}
	}
	
	delete removeMe;
}

/////////////////////////////////////////////////////////////////////////////
// BulletPhysics::FindBulletAction			- not described in the book
//    Finds a Bullet action given an actor ID
//
btActionInterface* BulletPhysics::FindBulletAction(ActorId const id) const
{
	ActorIDToBulletActionMap::const_iterator found = mActorIdToAction.find(id);
	if (found != mActorIdToAction.end())
		return found->second;

	return NULL;
}

/////////////////////////////////////////////////////////////////////////////
// BulletPhysics::FindBulletCollisionObject			- not described in the book
//    Finds a Bullet rigid body given an actor ID
//
btCollisionObject* BulletPhysics::FindBulletCollisionObject( ActorId const id ) const
{
	ActorIDToBulletCollisionObjectMap::const_iterator found = mActorIdToCollisionObject.find( id );
	if ( found != mActorIdToCollisionObject.end() )
		return found->second;

	return NULL;
}

/////////////////////////////////////////////////////////////////////////////
// BulletPhysics::FindActorID				- not described in the book
//    Finds an Actor ID given a Bullet collision object
//
ActorId BulletPhysics::FindActorID( btCollisionObject const * const collisionObject ) const
{
	BulletCollisionObjectToActorIDMap::const_iterator found = mCollisionObjectToActorId.find(collisionObject);
	if ( found != mCollisionObjectToActorId.end() )
		return found->second;
		
	return INVALID_ACTOR_ID;
}


/////////////////////////////////////////////////////////////////////////////
// BulletPhysics::AddTrigger
//
void BulletPhysics::AddTrigger(const Vector3<float> &dimension, 
	eastl::weak_ptr<Actor> pGameActor, const eastl::string& physicMaterial)
{
	eastl::shared_ptr<Actor> pStrongActor(pGameActor.lock());
	if (!pStrongActor)
		return;  // FUTURE WORK: Add a call to the error log here

				 // create the collision body, which specifies the shape of the object
	btBoxShape * const boxShape = new btBoxShape(Vector3TobtVector3(dimension));

	// triggers are immoveable.  0 mass signals this to Bullet.
	btScalar const mass = 0;

	Transform triggerTransform;
	eastl::shared_ptr<TransformComponent> pTransformComponent =
		pStrongActor->GetComponent<TransformComponent>(TransformComponent::Name).lock();
	LogAssert(pTransformComponent, "no transform");
	if (pTransformComponent)
	{
		triggerTransform = pTransformComponent->GetTransform();
	}
	else
	{
		// Physics can't work on an actor that doesn't have a TransformComponent!
		return;
	}
	ActorMotionState * const motionState = new ActorMotionState(triggerTransform);

	btRigidBody::btRigidBodyConstructionInfo rbInfo(mass, motionState, boxShape, btVector3(0, 0, 0));
	btRigidBody * const body = new btRigidBody(rbInfo);

	// lookup the material
	MaterialData material(LookupMaterialData(physicMaterial));

	// set up the materal properties
	rbInfo.m_restitution = material.mRestitution;
	rbInfo.m_friction = material.mFriction;

	mDynamicsWorld->addRigidBody(body);

	// a trigger is just a box that doesn't collide with anything.  That's what "CF_NO_CONTACT_RESPONSE" indicates.
	body->setCollisionFlags(body->getCollisionFlags() | btRigidBody::CF_NO_CONTACT_RESPONSE);
	body->setUserPointer(new int(pStrongActor->GetId()));

	mActorIdToCollisionObject[pStrongActor->GetId()] = body;
	mCollisionObjectToActorId[body] = pStrongActor->GetId();
}



/////////////////////////////////////////////////////////////////////////////
// BulletPhysics::AddBSP
//
void BulletPhysics::AddBSP(BspLoader& bspLoader, eastl::weak_ptr<Actor> pGameActor,
	const eastl::string& densityStr, const eastl::string& physicMaterial)
{
	eastl::shared_ptr<Actor> pStrongActor(pGameActor.lock());
	if (!pStrongActor)
		return;  // FUTURE WORK - Add a call to the error log here

	// triggers are immoveable.  0 mass signals this to Bullet.
	btScalar const mass = 0;

	BspToBulletConverter bspToBullet(this, pStrongActor, mass, physicMaterial);
	float bspScaling = 1.0f;
	bspToBullet.ConvertBsp(bspLoader, bspScaling);
}

/////////////////////////////////////////////////////////////////////////////
// BulletPhysics::AddCharacterController
//
void BulletPhysics::AddCharacterController(
	const Vector3<float>& dimensions, eastl::weak_ptr<Actor> pGameActor,
	const eastl::string& densityStr, const eastl::string& physicMaterial)
{
	eastl::shared_ptr<Actor> pStrongActor(pGameActor.lock());
	if (!pStrongActor)
		return;  // FUTURE WORK - Add a call to the error log here

	// create the collision body, which specifies the shape of the object
	float radius = eastl::max(dimensions[0], dimensions[1]) / 2.f;
	float height = dimensions[2] > 2 * radius ? dimensions[2] - 2 * radius : 0;
	btConvexShape* collisionShape = new btCapsuleShapeZ(radius, height);

	// calculate absolute mass from specificGravity
	float specificGravity = LookupSpecificGravity(densityStr);
	float const volume = dimensions[0] * dimensions[1] * dimensions[2];
	btScalar const mass = volume * specificGravity;

	ActorId actorID = pStrongActor->GetId();
	LogAssert(mActorIdToCollisionObject.find(actorID) == mActorIdToCollisionObject.end(),
		"Actor with more than one physics body?");

	// lookup the material
	MaterialData material(LookupMaterialData(physicMaterial));

	// localInertia defines how the object's mass is distributed
	btVector3 localInertia(0.f, 0.f, 0.f);
	if (mass > 0.f)
		collisionShape->calculateLocalInertia(mass, localInertia);

	Transform transform;
	eastl::shared_ptr<TransformComponent> pTransformComponent =
		pStrongActor->GetComponent<TransformComponent>(TransformComponent::Name).lock();
	LogAssert(pTransformComponent, "no transform");
	if (pTransformComponent)
	{
		transform = pTransformComponent->GetTransform();
	}
	else
	{
		// Physics can't work on an actor that doesn't have a TransformComponent!
		return;
	}

	btPairCachingGhostObject* ghostObject = new btPairCachingGhostObject();
	ghostObject->setWorldTransform(TransformTobtTransform(transform));
	mBroadphase->getOverlappingPairCache()->setInternalGhostPairCallback(new btGhostPairCallback());
	ghostObject->setCollisionShape(collisionShape);
	ghostObject->setCollisionFlags(btCollisionObject::CF_CHARACTER_OBJECT);
	btKinematicCharacterController* controller = new btKinematicCharacterController(ghostObject, collisionShape, 16.f);
	controller->setGravity(mDynamicsWorld->getGravity() * 600);
	controller->setFallSpeed(600);

	mDynamicsWorld->addCollisionObject(ghostObject, btBroadphaseProxy::CharacterFilter, btBroadphaseProxy::AllFilter);
	mDynamicsWorld->addAction(controller);

	// add it to the collection to be checked for changes in SyncVisibleScene
	mActorIdToAction[actorID] = controller;
	mActorIdToCollisionObject[actorID] = ghostObject;
	mCollisionObjectToActorId[ghostObject] = actorID;
}

/////////////////////////////////////////////////////////////////////////////
// BulletPhysics::AddSphere					- Chapter 17, page 599
//
void BulletPhysics::AddSphere(float const radius, eastl::weak_ptr<Actor> pGameActor, 
	const eastl::string& densityStr, const eastl::string& physicMaterial)
{
	eastl::shared_ptr<Actor> pStrongActor(pGameActor.lock());
    if (!pStrongActor)
        return;  // FUTURE WORK - Add a call to the error log here
	
	// create the collision body, which specifies the shape of the object
	btSphereShape * const collisionShape = new btSphereShape( radius );
	
	// calculate absolute mass from specificGravity
    float specificGravity = LookupSpecificGravity(densityStr);
	float const volume = (4.f / 3.f) * (float)GE_C_PI * radius * radius * radius;
	btScalar const mass = volume * specificGravity;
	
	AddShape(pStrongActor, collisionShape, mass, physicMaterial);
}

/////////////////////////////////////////////////////////////////////////////
// BulletPhysics::AddBox
//
void BulletPhysics::AddBox(const Vector3<float>& dimensions, eastl::weak_ptr<Actor> pGameActor,
	const eastl::string& densityStr, const eastl::string& physicMaterial)
{
	eastl::shared_ptr<Actor> pStrongActor(pGameActor.lock());
    if (!pStrongActor)
        return;  // FUTURE WORK: Add a call to the error log here

	// create the collision body, which specifies the shape of the object
	btBoxShape * const boxShape = new btBoxShape( Vector3TobtVector3( dimensions ) );
	
	// calculate absolute mass from specificGravity
    float specificGravity = LookupSpecificGravity(densityStr);
	float const volume = dimensions[0] * dimensions[1] * dimensions[2];
	btScalar const mass = volume * specificGravity;
	
	AddShape(pStrongActor, boxShape, mass, physicMaterial);
}

/////////////////////////////////////////////////////////////////////////////
// BulletPhysics::AddPointCloud				- Chapter 17, page 601
//
void BulletPhysics::AddPointCloud(Vector3<float> *verts, int numPoints, eastl::weak_ptr<Actor> pGameActor,
	const eastl::string& densityStr, const eastl::string& physicMaterial)
{
	eastl::shared_ptr<Actor> pStrongActor(pGameActor.lock());
    if (!pStrongActor)
        return;  // FUTURE WORK: Add a call to the error log here
	
	btConvexHullShape * const shape = new btConvexHullShape();
	
	// add the points to the shape one at a time
	for ( int i=0; i<numPoints; ++i )
		shape->addPoint( Vector3TobtVector3( verts[i] ) );
	
	// approximate absolute mass using bounding box
	btVector3 aabbMin(0,0,0), aabbMax(0,0,0);
	shape->getAabb( btTransform::getIdentity(), aabbMin, aabbMax );
	
	btVector3 const aabbExtents = aabbMax - aabbMin;
	
    float specificGravity = LookupSpecificGravity(densityStr);
	float const volume = aabbExtents.x() * aabbExtents.y() * aabbExtents.z();
	btScalar const mass = volume * specificGravity;
	
	AddShape(pStrongActor, shape, mass, physicMaterial);
}

/////////////////////////////////////////////////////////////////////////////
// BulletPhysics::AddPointCloud				- Chapter 17, page 601
//
void BulletPhysics::AddPointCloud(Plane3<float> *planes, int numPlanes, eastl::weak_ptr<Actor> pGameActor,
	const eastl::string& densityStr, const eastl::string& physicMaterial)
{
	eastl::shared_ptr<Actor> pStrongActor(pGameActor.lock());
	if (!pStrongActor)
		return;  // FUTURE WORK: Add a call to the error log here

	btAlignedObjectArray<btVector3> planeEquations;
	for (int i = 0; i < numPlanes; ++i)
	{
		btVector3 planeEq;
		planeEq.setValue(
			planes[i].mNormal[0],
			planes[i].mNormal[1],
			planes[i].mNormal[2]);
		planeEq[3] = -planes[i].mConstant;
		planeEquations.push_back(planeEq);
	}
	btAlignedObjectArray<btVector3>	vertices;
	btGeometryUtil::getVerticesFromPlaneEquations(planeEquations, vertices);

	btConvexHullShape * const shape = new btConvexHullShape();
	for (int i = 0; i < vertices.size(); ++i)
		shape->addPoint(vertices[i]);

	// approximate absolute mass using bounding box
	btVector3 aabbMin(0, 0, 0), aabbMax(0, 0, 0);
	shape->getAabb(btTransform::getIdentity(), aabbMin, aabbMax);

	btVector3 const aabbExtents = aabbMax - aabbMin;

	float specificGravity = LookupSpecificGravity(densityStr);
	float const volume = aabbExtents.x() * aabbExtents.y() * aabbExtents.z();
	btScalar const mass = volume * specificGravity;

	AddShape(pStrongActor, shape, mass, physicMaterial);
}


/////////////////////////////////////////////////////////////////////////////
// BulletPhysics::RemoveActor					- not described in the book
//
//    Implements the method to remove actors from the physics simulation
//
void BulletPhysics::RemoveActor(ActorId id)
{
	if ( btCollisionObject * const collisionObject = FindBulletCollisionObject( id ) )
	{
		// destroy the body and all its components
		RemoveCollisionObject(collisionObject);
		mActorIdToCollisionObject.erase ( id );
		mCollisionObjectToActorId.erase(collisionObject);
	}
}

/////////////////////////////////////////////////////////////////////////////
// BulletPhysics::RenderDiagnostics			- Chapter 17, page 604
//
void BulletPhysics::RenderDiagnostics()
{
	mDynamicsWorld->debugDrawWorld();

	mDebugDrawer->Render();
	mDebugDrawer->Clear();
}

/////////////////////////////////////////////////////////////////////////////
// BulletPhysics::ApplyForce					- Chapter 17, page 603
//
void BulletPhysics::ApplyForce(const Vector3<float> &velocity, ActorId aid)
{
	if (btCollisionObject * const collisionObject = FindBulletCollisionObject(aid))
	{
		if (collisionObject->getCollisionFlags() & btCollisionObject::CF_CHARACTER_OBJECT)
		{
			if (btKinematicCharacterController* const controller =
				dynamic_cast<btKinematicCharacterController*>(FindBulletAction(aid)))
			{
				controller->applyImpulse(Vector3TobtVector3(velocity));
			}
		}
		else
		{
			btRigidBody* const rigidBody = dynamic_cast<btRigidBody*>(collisionObject);
			rigidBody->applyCentralImpulse(Vector3TobtVector3(velocity));
		}
	}
}

/////////////////////////////////////////////////////////////////////////////
// BulletPhysics::ApplyTorque					- Chapter 17, page 603
//
void BulletPhysics::ApplyTorque(const Vector3<float> &velocity, ActorId aid)
{
	if (btRigidBody* const rigidBody = dynamic_cast<btRigidBody*>(FindBulletCollisionObject(aid)))
		rigidBody->applyTorqueImpulse( Vector3TobtVector3(velocity) );
}

/////////////////////////////////////////////////////////////////////////////
// BulletPhysics::GetTransform					- not described in the book
//
//   Returns the current transform of the phyics object
//
Transform BulletPhysics::GetTransform(const ActorId id)
{
	btCollisionObject * pCollisionObject = FindBulletCollisionObject(id);
    LogAssert(pCollisionObject, "no collision object");

    const btTransform& actorTransform = pCollisionObject->getWorldTransform();
    return btTransformToTransform(actorTransform);
}

/////////////////////////////////////////////////////////////////////////////
// BulletPhysics::SetTransform					- not described in the book
//
//   Sets the current transform of the phyics object
//
void BulletPhysics::SetTransform(ActorId actorId, const Transform& mat)
{
	if (btCollisionObject * const collisionObject = FindBulletCollisionObject(actorId))
	{
		// warp the body to the new position
		collisionObject->setWorldTransform(TransformTobtTransform(mat));
	}
}

/////////////////////////////////////////////////////////////////////////////
// BulletPhysics::StopActor					- Chapter 17, page 604
//
void BulletPhysics::StopActor(ActorId actorId)
{
	SetVelocity(actorId, Vector3<float>());
}

/////////////////////////////////////////////////////////////////////////////
// BulletPhysics::SetIgnoreCollision
//
void BulletPhysics::SetIgnoreCollision(ActorId actorId, ActorId ignoreActorId, bool ignoreCollision) 
{ 
	if (btCollisionObject * const collisionObject = FindBulletCollisionObject(actorId))
	{
		if (btCollisionObject * const ignoreCollisionObject = FindBulletCollisionObject(ignoreActorId))
		{
			collisionObject->setIgnoreCollisionCheck(ignoreCollisionObject, ignoreCollision);
		}
	}
}

/////////////////////////////////////////////////////////////////////////////
// BulletPhysics::FindIntersection		
bool BulletPhysics::FindIntersection(ActorId actorId, const Vector3<float>& point)
{
	if (btCollisionObject * const collisionObject = FindBulletCollisionObject(actorId))
	{
		if (collisionObject->getCollisionFlags() & btCollisionObject::CF_CHARACTER_OBJECT)
		{
			if (btKinematicCharacterController* const controller =
				dynamic_cast<btKinematicCharacterController*>(FindBulletAction(actorId)))
			{
				btCollisionShape* collisionShape = controller->getGhostObject()->getCollisionShape();

				btAABB aaBBox;
				collisionShape->getAabb(controller->getGhostObject()->getWorldTransform(), aaBBox.m_min, aaBBox.m_max);
				if (aaBBox.m_min[0] > point[0] || aaBBox.m_max[0] < point[0] ||
					aaBBox.m_min[1] > point[1] || aaBBox.m_max[1] < point[1] ||
					aaBBox.m_min[2] > point[2] || aaBBox.m_max[2] < point[2])
				{
					return false;
				}
				return true;
			}
		}
		else
		{
			btRigidBody* const rigidBody = dynamic_cast<btRigidBody*>(collisionObject);
			
			btAABB aaBBox;
			rigidBody->getAabb(aaBBox.m_min, aaBBox.m_max);
			if (aaBBox.m_min[0] > point[0] || aaBBox.m_max[0] < point[0] ||
				aaBBox.m_min[1] > point[1] || aaBBox.m_max[1] < point[1] ||
				aaBBox.m_min[2] > point[2] || aaBBox.m_max[2] < point[2])
			{
				return false;
			}
			return true;
		}
	}

	return false;
}

/////////////////////////////////////////////////////////////////////////////
// BulletPhysics::CastRay	
ActorId BulletPhysics::CastRay(
	const Vector3<float>& origin, const Vector3<float>& end, 
	Vector3<float>& collisionPoint, Vector3<float>& collisionNormal)
{
	btVector3 from = Vector3TobtVector3(origin);
	btVector3 to = Vector3TobtVector3(end);
	btCollisionWorld::ClosestRayResultCallback closestResults(from, to);
	closestResults.m_flags |= btTriangleRaycastCallback::kF_FilterBackfaces;

	mDynamicsWorld->rayTest(from, to, closestResults);

	if (closestResults.hasHit())
	{
		collisionPoint = btVector3ToVector3(closestResults.m_hitPointWorld);
		collisionNormal = btVector3ToVector3(closestResults.m_hitNormalWorld);
		return FindActorID(closestResults.m_collisionObject);
	}
	else
	{
		collisionPoint = NULL;
		collisionNormal = NULL;
		return INVALID_ACTOR_ID;
	}
}

/////////////////////////////////////////////////////////////////////////////
// BulletPhysics::CastRay	
void BulletPhysics::CastRay(
	const Vector3<float>& origin, const Vector3<float>& end,
	eastl::vector<ActorId>& collisionActors,
	eastl::vector<Vector3<float>>& collisionPoints, 
	eastl::vector<Vector3<float>>& collisionNormals)
{
	btVector3 from = Vector3TobtVector3(origin);
	btVector3 to = Vector3TobtVector3(end);
	btCollisionWorld::AllHitsRayResultCallback allHitsResults(from, to);
	allHitsResults.m_flags |= btTriangleRaycastCallback::kF_FilterBackfaces;

	mDynamicsWorld->rayTest(from, to, allHitsResults);

	if (allHitsResults.hasHit())
	{
		for (int i = 0; i<allHitsResults.m_collisionObjects.size(); i++)
		{
			const btCollisionObject* collisionObject = allHitsResults.m_collisionObjects[i];
			collisionActors.push_back(FindActorID(collisionObject));
			collisionPoints.push_back(btVector3ToVector3(allHitsResults.m_hitPointWorld[i]));
			collisionNormals.push_back(btVector3ToVector3(allHitsResults.m_hitNormalWorld[i]));
		}
	}
}

/////////////////////////////////////////////////////////////////////////////
// BulletPhysics::ConvexSweep	
ActorId BulletPhysics::ConvexSweep(
	ActorId aId, const Transform& origin, const Transform& end,
	Vector3<float>& collisionPoint, Vector3<float>& collisionNormal)
{
	btVector3 from = Vector3TobtVector3(origin.GetTranslation());
	btVector3 to = Vector3TobtVector3(end.GetTranslation());
	btCollisionWorld::ClosestConvexResultCallback closestResults(from, to);
	btCollisionObject* collisionObject = FindBulletCollisionObject(aId);
	btConvexShape* collisionShape = dynamic_cast<btConvexShape*>(collisionObject->getCollisionShape());

	mDynamicsWorld->convexSweepTest(
		collisionShape, TransformTobtTransform(origin), TransformTobtTransform(end), closestResults);

	if (closestResults.hasHit())
	{
		collisionPoint = btVector3ToVector3(closestResults.m_hitPointWorld);
		collisionNormal = btVector3ToVector3(closestResults.m_hitNormalWorld);
		return FindActorID(closestResults.m_hitCollisionObject);
	}
	else
	{
		collisionPoint = NULL;
		collisionNormal = NULL;
		return INVALID_ACTOR_ID;
	}
}

/////////////////////////////////////////////////////////////////////////////
// BulletPhysics::GetScale					
//
Vector3<float> BulletPhysics::GetScale(ActorId actorId)
{
	if (btCollisionObject * const collisionObject = FindBulletCollisionObject(actorId))
	{
		if (collisionObject->getCollisionFlags() & btCollisionObject::CF_CHARACTER_OBJECT)
		{
			if (btKinematicCharacterController* const controller =
				dynamic_cast<btKinematicCharacterController*>(FindBulletAction(actorId)))
			{
				btCollisionShape* collisionShape = controller->getGhostObject()->getCollisionShape();

				btVector3 aabbMin, aabbMax;
				collisionShape->getAabb(controller->getGhostObject()->getWorldTransform(), aabbMin, aabbMax);
				btVector3 const aabbExtents = aabbMax - aabbMin;
				return btVector3ToVector3(aabbExtents);
			}
		}
		else
		{
			btRigidBody* const rigidBody = dynamic_cast<btRigidBody*>(collisionObject);

			btVector3 aabbMin, aabbMax;
			rigidBody->getAabb(aabbMin, aabbMax);
			btVector3 const aabbExtents = aabbMax - aabbMin;
			return btVector3ToVector3(aabbExtents);
		}
	}
	return Vector3<float>();
}

/////////////////////////////////////////////////////////////////////////////
// BulletPhysics::GetVelocity					- Chapter 17, page 604
//
Vector3<float> BulletPhysics::GetVelocity(ActorId actorId)
{
	if (btCollisionObject * const collisionObject = FindBulletCollisionObject(actorId))
	{
		if (collisionObject->getCollisionFlags() & btCollisionObject::CF_CHARACTER_OBJECT)
		{
			if (btKinematicCharacterController* const controller =
				dynamic_cast<btKinematicCharacterController*>(FindBulletAction(actorId)))
			{
				btVector3 btVel = controller->getLinearVelocity();
				return btVector3ToVector3(btVel);
			}
		}
		else
		{
			btRigidBody* const rigidBody = dynamic_cast<btRigidBody*>(collisionObject);
			btVector3 btVel = rigidBody->getLinearVelocity();
			return btVector3ToVector3(btVel);
		}
	}
	return Vector3<float>();
}

/////////////////////////////////////////////////////////////////////////////
float BulletPhysics::GetJumpSpeed(ActorId actorId)
{
	float jumpSpeed = 0;
	if (btCollisionObject * const collisionObject = FindBulletCollisionObject(actorId))
	{
		if (collisionObject->getCollisionFlags() & btCollisionObject::CF_CHARACTER_OBJECT)
		{
			if (btKinematicCharacterController* const controller =
				dynamic_cast<btKinematicCharacterController*>(FindBulletAction(actorId)))
			{
				jumpSpeed = controller->getJumpSpeed();
			}
		}
	}
	return jumpSpeed;
}

/////////////////////////////////////////////////////////////////////////////
void BulletPhysics::SetGravity(ActorId actorId, const Vector3<float>& g)
{
	if (btCollisionObject * const collisionObject = FindBulletCollisionObject(actorId))
	{
		if (collisionObject->getCollisionFlags() & btCollisionObject::CF_CHARACTER_OBJECT)
		{
			if (btKinematicCharacterController* const controller =
				dynamic_cast<btKinematicCharacterController*>(FindBulletAction(actorId)))
			{
				btVector3 btGravity = Vector3TobtVector3(g);
				controller->setGravity(btGravity);
			}
		}
		else
		{
			btRigidBody* const rigidBody = dynamic_cast<btRigidBody*>(collisionObject);
			btVector3 btGravity = Vector3TobtVector3(g);
			rigidBody->setGravity(btGravity);
		}
	}
}

/////////////////////////////////////////////////////////////////////////////
void BulletPhysics::SetVelocity(ActorId actorId, const Vector3<float>& vel)
{
	if (btCollisionObject * const collisionObject = FindBulletCollisionObject(actorId))
	{
		if (collisionObject->getCollisionFlags() & btCollisionObject::CF_CHARACTER_OBJECT)
		{
			if (btKinematicCharacterController* const controller =
				dynamic_cast<btKinematicCharacterController*>(FindBulletAction(actorId)))
			{
				btVector3 btVel = Vector3TobtVector3(vel);
				controller->setLinearVelocity(btVel);
			}
		}
		else
		{
			btRigidBody* const rigidBody = dynamic_cast<btRigidBody*>(collisionObject);
			btVector3 btVel = Vector3TobtVector3(vel);
			rigidBody->setLinearVelocity(btVel);
		}
	}
}

/////////////////////////////////////////////////////////////////////////////
Vector3<float> BulletPhysics::GetAngularVelocity(ActorId actorId)
{
	if (btCollisionObject * const collisionObject = FindBulletCollisionObject(actorId))
	{
		if (collisionObject->getCollisionFlags() & btCollisionObject::CF_CHARACTER_OBJECT)
		{
			if (btKinematicCharacterController* const controller =
				dynamic_cast<btKinematicCharacterController*>(FindBulletAction(actorId)))
			{
				btVector3 btVel = controller->getAngularVelocity();
				return btVector3ToVector3(btVel);
			}
		}
		else
		{
			btRigidBody* const rigidBody = dynamic_cast<btRigidBody*>(collisionObject);
			btVector3 btVel = rigidBody->getAngularVelocity();
			return btVector3ToVector3(btVel);
		}
	}
	return Vector3<float>();
}

/////////////////////////////////////////////////////////////////////////////
void BulletPhysics::SetAngularVelocity(ActorId actorId, const Vector3<float>& vel)
{
	if (btCollisionObject * const collisionObject = FindBulletCollisionObject(actorId))
	{
		if (collisionObject->getCollisionFlags() & btCollisionObject::CF_CHARACTER_OBJECT)
		{
			if (btKinematicCharacterController* const controller =
				dynamic_cast<btKinematicCharacterController*>(FindBulletAction(actorId)))
			{
				btVector3 btVel = Vector3TobtVector3(vel);
				controller->setAngularVelocity(btVel);
			}
		}
		else
		{
			btRigidBody* const rigidBody = dynamic_cast<btRigidBody*>(collisionObject);
			btVector3 btVel = Vector3TobtVector3(vel);
			rigidBody->setAngularVelocity(btVel);
		}
	}
}

/////////////////////////////////////////////////////////////////////////////
void BulletPhysics::Translate(ActorId actorId, const Vector3<float>& vec)
{
	if (btRigidBody* const rigidBody = dynamic_cast<btRigidBody*>(FindBulletCollisionObject(actorId)))
	{
		btVector3 btVec = Vector3TobtVector3(vec);
		rigidBody->translate(btVec);
	}
}

/////////////////////////////////////////////////////////////////////////////
// BulletPhysics::OnGround
bool BulletPhysics::OnGround(ActorId aid)
{
	if (btKinematicCharacterController* const controller =
		dynamic_cast<btKinematicCharacterController*>(FindBulletAction(aid)))
	{
		return controller->onGround();
	}
	return false;
}

/////////////////////////////////////////////////////////////////////////////
// BulletPhysics::Jump
void BulletPhysics::Jump(ActorId aid, const Vector3<float> &dir)
{
	if (btKinematicCharacterController* const controller =
		dynamic_cast<btKinematicCharacterController*>(FindBulletAction(aid)))
	{
		controller->jump(Vector3TobtVector3(dir));
	}
}

/////////////////////////////////////////////////////////////////////////////
// BulletPhysics::FallDirection
void BulletPhysics::FallDirection(ActorId aid, const Vector3<float> &dir)
{
	if (btKinematicCharacterController* const controller =
		dynamic_cast<btKinematicCharacterController*>(FindBulletAction(aid)))
	{
		controller->setGravity(Vector3TobtVector3(dir));
		controller->setFallSpeed(Length(dir));
	}
}

/////////////////////////////////////////////////////////////////////////////
// BulletPhysics::WalkDirection
void BulletPhysics::WalkDirection(ActorId aid, const Vector3<float> &dir)
{
	if (btKinematicCharacterController* const controller =
		dynamic_cast<btKinematicCharacterController*>(FindBulletAction(aid)))
	{
		controller->setWalkDirection(Vector3TobtVector3(dir));
	}
}

/////////////////////////////////////////////////////////////////////////////
// BulletPhysics::SetPosition
void BulletPhysics::SetPosition(ActorId actorId, const Vector3<float>& pos)
{
	if (btCollisionObject * const collisionObject = FindBulletCollisionObject(actorId))
	{
		btTransform transform = collisionObject->getWorldTransform();
		transform.setOrigin(Vector3TobtVector3(pos));
		collisionObject->setWorldTransform(transform);
	}
}

/////////////////////////////////////////////////////////////////////////////
// BulletPhysics::SetRotation
void BulletPhysics::SetRotation(ActorId actorId, const Transform& mat)
{
	if (btCollisionObject * const collisionObject = FindBulletCollisionObject(actorId))
	{
		btTransform transform = TransformTobtTransform(mat);
		transform.setOrigin(collisionObject->getWorldTransform().getOrigin());
		collisionObject->setWorldTransform(transform);
	}
}

/////////////////////////////////////////////////////////////////////////////
// BulletPhysics::BulletInternalTickCallback		- Chapter 17, page 606
//
// This function is called after bullet performs its internal update.  We
//   use it to detect collisions between objects for Game code.
//
void BulletPhysics::BulletInternalTickCallback(btDynamicsWorld * const world, btScalar const timeStep )
{
	LogAssert( world, "invalid world ptr" );
	
	LogAssert( world->getWorldUserInfo(), "no world user info" );
	BulletPhysics * const bulletPhysics = static_cast<BulletPhysics*>( world->getWorldUserInfo() );
	
	CollisionPairs currentTickCollisionPairs;
	
	// look at all existing contacts
	btDispatcher * const dispatcher = world->getDispatcher();
	for ( int manifoldIdx=0; manifoldIdx<dispatcher->getNumManifolds(); ++manifoldIdx )
	{
		// get the "manifold", which is the set of data corresponding to a contact point
		//   between two physics objects
		btPersistentManifold const * const manifold = dispatcher->getManifoldByIndexInternal( manifoldIdx );
		LogAssert( manifold, "invalid manifold" );

		if (manifold->getNumContacts() == 0)
			continue; //we consider a collision after we get contact
		
		// get the two bodies used in the manifold.  Bullet stores them as void*, so we must cast
		//  them back to btRigidBody*s.  Manipulating void* pointers is usually a bad
		//  idea, but we have to work with the environment that we're given.  We know this
		//  is safe because we only ever add btRigidBodys to the simulation
		btRigidBody const * const body0 = static_cast<btRigidBody const *>(manifold->getBody0());
		btRigidBody const * const body1 = static_cast<btRigidBody const *>(manifold->getBody1());
		
		// always create the pair in a predictable order
		bool const swapped = body0 > body1;
		
		btRigidBody const * const sortedBodyA = swapped ? body1 : body0;
		btRigidBody const * const sortedBodyB = swapped ? body0 : body1;
		
		CollisionPair const thisPair = eastl::make_pair( sortedBodyA, sortedBodyB );
		currentTickCollisionPairs.insert( thisPair );
		
		if ( bulletPhysics->mPreviousTickCollisionPairs.find( thisPair ) == bulletPhysics->mPreviousTickCollisionPairs.end() )
		{
			// this is a new contact, which wasn't in our list before.  send an event to the game.
			bulletPhysics->SendCollisionPairAddEvent( manifold, body0, body1 );
		}
	}
	
	CollisionPairs removedCollisionPairs;
	
	// use the STL set difference function to find collision pairs that existed during the previous tick but not any more
	eastl::set_difference( 
		bulletPhysics->mPreviousTickCollisionPairs.begin(), 
		bulletPhysics->mPreviousTickCollisionPairs.end(),
		currentTickCollisionPairs.begin(), currentTickCollisionPairs.end(),
		eastl::inserter( removedCollisionPairs, removedCollisionPairs.begin() ) );
	
	for ( CollisionPairs::const_iterator it = removedCollisionPairs.begin(), 
         end = removedCollisionPairs.end(); it != end; ++it )
	{
		btRigidBody const * const body0 = it->first;
		btRigidBody const * const body1 = it->second;
		
		bulletPhysics->SendCollisionPairRemoveEvent( body0, body1 );
	}
	
	// the current tick becomes the previous tick.  this is the way of all things.
	bulletPhysics->mPreviousTickCollisionPairs = currentTickCollisionPairs;
}

//////////////////////////////////////////////////////////////////////////////////////////
void BulletPhysics::SendCollisionPairAddEvent( btPersistentManifold const * manifold, 
	btRigidBody const * const body0, btRigidBody const * const body1 )
{
	if ( body0->getUserPointer() || body1->getUserPointer() )
	{
		// only triggers have non-NULL userPointers
		
		// figure out which actor is the trigger
		btRigidBody const * triggerBody, * otherBody;
	
		if ( body0->getUserPointer() )
		{
			triggerBody = body0;
			otherBody = body1;
		}
		else
		{
			otherBody = body0;
			triggerBody = body1;
		}
		
		// send the trigger event.
		int const triggerId = *static_cast<int*>(triggerBody->getUserPointer());
        eastl::shared_ptr<EventDataPhysTriggerEnter> pEvent(
			new EventDataPhysTriggerEnter(triggerId, FindActorID(otherBody)));
        BaseEventManager::Get()->QueueEvent(pEvent);
	}
	else
	{
		ActorId const id0 = FindActorID( body0 );
		ActorId const id1 = FindActorID( body1 );

		if (id0 == INVALID_ACTOR_ID && id1 == INVALID_ACTOR_ID)
		{
			// collision is ending between some object(s) that don't have actors. 
			// we don't send events for that.
			return;
		}
		
		// this pair of colliding objects is new.  send a collision-begun event
		eastl::list<Vector3<float>> collisionPoints;
		Vector3<float> sumNormalForce = Vector3<float>::Zero();
		Vector3<float> sumFrictionForce = Vector3<float>::Zero();
		
		for ( int pointIdx = 0; pointIdx < manifold->getNumContacts(); ++pointIdx )
		{
			btManifoldPoint const & point = manifold->getContactPoint( pointIdx );
		
			collisionPoints.push_back( btVector3ToVector3( point.getPositionWorldOnB() ) );
			
			sumNormalForce += btVector3ToVector3( point.m_combinedRestitution * point.m_normalWorldOnB );
			sumFrictionForce += btVector3ToVector3( point.m_combinedFriction * point.m_lateralFrictionDir1 );
		}
		
		// send the event for the game
        eastl::shared_ptr<EventDataPhysCollision> pEvent(
			new EventDataPhysCollision(id0, id1, sumNormalForce, sumFrictionForce, collisionPoints));
        BaseEventManager::Get()->QueueEvent(pEvent);
	}
}

//////////////////////////////////////////////////////////////////////////////////////////
void BulletPhysics::SendCollisionPairRemoveEvent(
	btRigidBody const * const body0, btRigidBody const * const body1 )
{
	if ( body0->getUserPointer() || body1->getUserPointer() )
	{
		// figure out which actor is the trigger
		btRigidBody const * triggerBody, * otherBody;
	
		if ( body0->getUserPointer() )
		{
			triggerBody = body0;
			otherBody = body1;
		}
		else
		{
			otherBody = body0;
			triggerBody = body1;
		}
		
		// send the trigger event.
		int const triggerId = *static_cast<int*>(triggerBody->getUserPointer());
        eastl::shared_ptr<EventDataPhysTriggerLeave> pEvent(
			new EventDataPhysTriggerLeave(triggerId, FindActorID( otherBody)));
        BaseEventManager::Get()->QueueEvent(pEvent);
	}
	else
	{
		ActorId const id0 = FindActorID( body0 );
		ActorId const id1 = FindActorID( body1 );

		if (id0 == INVALID_ACTOR_ID && id1 == INVALID_ACTOR_ID)
		{
			// collision is ending between some object(s) that don't have actors. 
			// we don't send events for that.
			return;
		}

        eastl::shared_ptr<EventDataPhysSeparation> pEvent(new EventDataPhysSeparation(id0, id1));
        BaseEventManager::Get()->QueueEvent(pEvent);
	}
}

float BulletPhysics::LookupSpecificGravity(const eastl::string& densityStr)
{
    float density = 0;
    auto densityIt = mDensityTable.find(densityStr);
    if (densityIt != mDensityTable.end())
        density = densityIt->second;
    // else: dump error

    return density;
}

MaterialData BulletPhysics::LookupMaterialData(const eastl::string& materialStr)
{
    auto materialIt = mMaterialTable.find(materialStr);
    if (materialIt != mMaterialTable.end())
        return materialIt->second;
    else
        return MaterialData(0, 0);
}

/////////////////////////////////////////////////////////////////////////////
//
// CreateGamePhysics 
//   The free function that creates an object that implements the BaseGamePhysic interface.
//
BaseGamePhysic* CreateGamePhysics()
{
	std::auto_ptr<BaseGamePhysic> gamePhysics;
	gamePhysics.reset( new BulletPhysics );

	if (gamePhysics.get() && !gamePhysics->Initialize())
	{
		// physics failed to initialize.  delete it.
		gamePhysics.reset();
	}

	return gamePhysics.release();
}

BaseGamePhysic* CreateNullPhysics()
{
	std::auto_ptr<BaseGamePhysic> gamePhysics;
	gamePhysics.reset( new NullPhysics );
	if (gamePhysics.get() && !gamePhysics->Initialize())
	{
		// physics failed to initialize.  delete it.
		gamePhysics.reset();
	}

	return gamePhysics.release();
}