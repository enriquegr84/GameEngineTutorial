// Copyright (C) 2002-2012 Nikolaus Gebhardt
// This file is part of the "Irrlicht Engine".
// For conditions of distribution and use, see copyright notice in irrlicht.h

#ifndef PARTICLESYSTEMNODE_H
#define PARTICLESYSTEMNODE_H

#include "AnimatedMeshNode.h"
#include "ParticleAnimatedMeshNodeEmitter.h"

#include "Graphic/Scene/Hierarchy/Node.h"

#include "Graphic/Scene/Particle/IParticleAffector.h"
#include "Graphic/Scene/Particle/IParticleEmitter.h"

#include "Graphic/Scene/Particle/ParticleBoxEmitter.h"
#include "Graphic/Scene/Particle/ParticleRingEmitter.h"
#include "Graphic/Scene/Particle/ParticleMeshEmitter.h"
#include "Graphic/Scene/Particle/ParticlePointEmitter.h"
#include "Graphic/Scene/Particle/ParticleSphereEmitter.h"
#include "Graphic/Scene/Particle/ParticleCylinderEmitter.h"

#include "Graphic/Scene/Particle/ParticleScaleAffector.h"
#include "Graphic/Scene/Particle/ParticleFadeOutAffector.h"
#include "Graphic/Scene/Particle/ParticleGravityAffector.h"
#include "Graphic/Scene/Particle/ParticleRotationAffector.h"
#include "Graphic/Scene/Particle/ParticleAttractionAffector.h"


//! A particle system scene node.
/** A scene node controlling a particle system. The behavior of the particles
can be controlled by setting the right particle emitters and affectors.
*/
class ParticleSystemNode : public Node
{

public:

	//! Constructor
	ParticleSystemNode(const ActorId actorId, WeakBaseRenderComponentPtr renderComponent, bool createDefaultEmitter);

/*
			const Vector3<float>& position = Vector3<float>(0,0,0),
			const Vector3<float>& rotation = Vector3<float>(0,0,0),
			const Vector3<float>& scale = Vector3<float>(1.0f, 1.0f, 1.0f));
*/

	//! destructor
	~ParticleSystemNode();

	//! Renders event
	bool PreRender(Scene *pScene);
	bool Render(Scene *pScene);

	//! Gets the particle emitter, which creates the particles.
	eastl::shared_ptr<BaseParticleEmitter> GetEmitter();

	//! Sets the particle emitter, which creates the particles.
	void SetEmitter(const eastl::shared_ptr<BaseParticleEmitter>& emitter);

	//! Adds new particle affector to the particle system.
	void AddAffector(const eastl::shared_ptr<BaseParticleAffector>& affector);

	//! Get a list of all particle affectors.
	const eastl::list<eastl::shared_ptr<BaseParticleAffector>>& GetAffectors() const;

	//! Removes all particle affectors in the particle system.
	void RemoveAllAffectors();

	//! Returns the material based on the zero based index i.
	Material& GetMaterial(unsigned int i);

	//! Returns amount of materials used by this scene node.
	unsigned int GetMaterialCount() const;

	//! returns the axis aligned bounding box of this node
	const AABBox3<float>& GetBoundingBox() const;

	//! Sets if the particles should be global. If they are, the particles are affected by
	//! the movement of the particle system scene node too, otherwise they completely
	//! ignore it. Default is true.
	void SetParticlesAreGlobal(bool global=true);

	//! Remove all currently visible particles
	void ClearParticles();

	//! Do manually update the particles.
	//! This should only be called when you want to render the node outside the scenegraph,
	//! as the node will care about this otherwise automatically.
	void DoParticleSystem(unsigned int time);

	//! Returns type of the scene node
	E_SCENE_NODE_TYPE GetType() const { return ESNT_PARTICLE_SYSTEM; } 

	//! Creates a particle emitter for an animated mesh scene node
	ParticleAnimatedMeshSceneNodeEmitter* CreateAnimatedMeshNodeEmitter(
		const ActorId actorId, const eastl::shared_ptr<AnimatedMeshNode>& node, 
		bool useNormalDirection = true, const Vector3<float>& direction = Vector3<float>{ 0.f, 0.03f, 0.f },
		float normalDirectionModifier = 100.0f, int mbNumber = -1, bool everyMeshVertex = false, 
		unsigned int minParticlesPerSecond = 5, unsigned int maxParticlesPerSecond = 10,
		const eastl::array<float, 4>& minStartColor = eastl::array<float, 4>{255.f, 0.f, 0.f, 0.f},
		const eastl::array<float, 4>& maxStartColor = eastl::array<float, 4>{255.f, 255.f, 255.f, 255.f},
		unsigned int lifeTimeMin = 2000, unsigned int lifeTimeMax = 4000, int maxAngleDegrees = 0,
		const Vector2<float>& minStartSize = Vector2<float>{ 5.f, 5.f },
		const Vector2<float>& maxStartSize = Vector2<float>{ 5.f, 5.f });

	//! Creates a box particle emitter.
	ParticleBoxEmitter* CreateBoxEmitter(
		const AABBox3<float>& box = AABBox3<float>{ -10.f,0.f,-10.f,5.f,30.f,10.f },
		const Vector3<float>& direction = Vector3<float>{ 0.f, 0.03f, 0.f },
		unsigned int minParticlesPerSecond = 5, unsigned int maxParticlesPerSecond = 10,
		const eastl::array<float, 4>& minStartColor = eastl::array<float, 4>{255.f, 0.f, 0.f, 0.f},
		const eastl::array<float, 4>& maxStartColor = eastl::array<float, 4>{255.f, 255.f, 255.f, 255.f},
		unsigned int lifeTimeMin = 2000, unsigned int lifeTimeMax = 4000, int maxAngleDegrees = 0,
		const Vector2<float>& minStartSize = Vector2<float>{ 5.f, 5.f },
		const Vector2<float>& maxStartSize = Vector2<float>{ 5.f, 5.f });

	//! Creates a particle emitter for emitting from a cylinder
	ParticleCylinderEmitter* CreateCylinderEmitter(
		const Vector3<float>& center, f32 radius, const Vector3<float>& normal, float length,
		bool outlineOnly = false, const Vector3<float>& direction = Vector3<float>{ 0.f, 0.5f, 0.f },
		unsigned int minParticlesPerSecond = 5, unsigned int maxParticlesPerSecond = 10,
		const eastl::array<float, 4>& minStartColor = eastl::array<float, 4>{255.f, 0.f, 0.f, 0.f},
		const eastl::array<float, 4>& maxStartColor = eastl::array<float, 4>{255.f, 255.f, 255.f, 255.f},
		unsigned int lifeTimeMin = 2000, unsigned int lifeTimeMax = 4000, int maxAngleDegrees = 0,
		const Vector2<float>& minStartSize = Vector2<float>{ 5.f, 5.f },
		const Vector2<float>& maxStartSize = Vector2<float>{ 5.f, 5.f });

	//! Creates a mesh particle emitter.
	ParticleMeshEmitter* CreateMeshEmitter(
		const eastl::shared_ptr<Mesh>& mesh, bool useNormalDirection = true,
		const Vector3<float>& direction = Vector3<float>{ 0.f, 0.03f, 0.f },
		float normalDirectionModifier = 100.0f, int mbNumber = -1, bool everyMeshVertex = false,
		unsigned int minParticlesPerSecond = 5, unsigned int maxParticlesPerSecond = 10,
		const eastl::array<float, 4>& minStartColor = eastl::array<float, 4>{255.f, 0.f, 0.f, 0.f},
		const eastl::array<float, 4>& maxStartColor = eastl::array<float, 4>{255.f, 255.f, 255.f, 255.f},
		unsigned int lifeTimeMin = 2000, unsigned int lifeTimeMax = 4000, int maxAngleDegrees = 0,
		const Vector2<float>& minStartSize = Vector2<float>{ 5.f, 5.f },
		const Vector2<float>& maxStartSize = Vector2<float>{ 5.f, 5.f });

	//! Creates a point particle emitter.
	ParticlePointEmitter* CreatePointEmitter(
		const Vector3<float>& direction = Vector3<float>{ 0.f, 0.03f, 0.f },
		unsigned int minParticlesPerSecond = 5, unsigned int maxParticlesPerSecond = 10,
		const eastl::array<float, 4>& minStartColor = eastl::array<float, 4>{255.f, 0.f, 0.f, 0.f},
		const eastl::array<float, 4>& maxStartColor = eastl::array<float, 4>{255.f, 255.f, 255.f, 255.f},
		unsigned int lifeTimeMin=2000, unsigned int lifeTimeMax=4000, int maxAngleDegrees=0,
		const Vector2<float>& minStartSize = Vector2<float>{ 5.f, 5.f },
		const Vector2<float>& maxStartSize = Vector2<float>{ 5.f, 5.f });

	//! Creates a ring particle emitter.
	ParticleRingEmitter* CreateRingEmitter(
		const Vector3<float>& center, float radius, float ringThickness,
		const Vector3<float>& direction = Vector3<float>{ 0.f, 0.03f, 0.f },
		unsigned int minParticlesPerSecond = 5, unsigned int maxParticlesPerSecond = 10,
		const eastl::array<float, 4>& minStartColor = eastl::array<float, 4>{255.f, 0.f, 0.f, 0.f},
		const eastl::array<float, 4>& maxStartColor = eastl::array<float, 4>{255.f, 255.f, 255.f, 255.f},
		unsigned int lifeTimeMin=2000, unsigned int lifeTimeMax=4000, int maxAngleDegrees=0,
		const Vector2<float>& minStartSize = Vector2<float>{ 5.f, 5.f },
		const Vector2<float>& maxStartSize = Vector2<float>{ 5.f, 5.f });

	//! Creates a sphere particle emitter.
	ParticleSphereEmitter* CreateSphereEmitter(
		const Vector3<float>& center, float radius,
		const Vector3<float>& direction = Vector3<float>{ 0.f, 0.03f, 0.f },
		unsigned int minParticlesPerSecond = 5, unsigned int maxParticlesPerSecond = 10,
		const eastl::array<float, 4>& minStartColor = eastl::array<float, 4>{255.f, 0.f, 0.f, 0.f},
		const eastl::array<float, 4>& maxStartColor = eastl::array<float, 4>{255.f, 255.f, 255.f, 255.f},
		unsigned int lifeTimeMin=2000, unsigned int lifeTimeMax=4000, int maxAngleDegrees=0,
		const Vector2<float>& minStartSize = Vector2<float>{ 5.f, 5.f },
		const Vector2<float>& maxStartSize = Vector2<float>{ 5.f, 5.f });

	//! Creates a point attraction affector. This affector modifies the positions of the
	//! particles and attracts them to a specified point at a specified speed per second.
	ParticleAttractionAffector* CreateAttractionAffector(
		const Vector3<float>& point, float speed = 1.0f, bool attract = true,
		bool affectX = true, bool affectY = true, bool affectZ = true);

	//! Creates a scale particle affector.
	BaseParticleAffector* CreateScaleParticleAffector(
		const Vector2<float>& scaleTo = Vector2<float>{ 1.f, 1.f });

	//! Creates a fade out particle affector.
	ParticleFadeOutAffector* CreateFadeOutParticleAffector(
		const eastl::array<float, 4>& targetColor = eastl::array<float, 4>{0.f, 0.f, 0.f, 0.f},
		unsigned int timeNeededToFadeOut = 1000);

	//! Creates a gravity affector.
	ParticleGravityAffector* CreateGravityAffector(
		const Vector3<float>& gravity = Vector3<float>{ 0.f, -0.03f, 0.f }, 
		unsigned int timeForceLost = 1000);

	//! Creates a rotation affector. This affector rotates the particles
	//! around a specified pivot point. The speed is in Degrees per second.
	ParticleRotationAffector* CreateRotationAffector(
		const Vector3<float>& speed = Vector3<float>{ 5.f, 5.f, 5.f },
		const Vector3<float>& pivotPoint = Vector3<float>{ 0.f, 0.f, 0.f });

private:

	void ReallocateBuffers();

	eastl::shared_ptr<MeshBufferVertex> mBuffer;
	eastl::list<eastl::shared_ptr<BaseParticleAffector>> mAffectorList;
	eastl::shared_ptr<BaseParticleEmitter> mEmitter;
	eastl::vector<Particle> mParticles;
	Vector2<float> mParticleSize;
	unsigned int mLastEmitTime;
	int mMaxParticles;

	enum E_PARTICLES_PRIMITIVE
	{
		EPP_POINT=0,
		EPP_BILLBOARD,
		EPP_POINTSPRITE
	};
	E_PARTICLES_PRIMITIVE mParticlePrimitive;

	bool mParticlesAreGlobal;
};

#endif

