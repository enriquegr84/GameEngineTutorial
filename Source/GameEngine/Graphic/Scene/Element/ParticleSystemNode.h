// Copyright (C) 2002-2012 Nikolaus Gebhardt
// This file is part of the "Irrlicht Engine".
// For conditions of distribution and use, see copyright notice in irrlicht.h

#ifndef PARTICLESYSTEMNODE_H
#define PARTICLESYSTEMNODE_H

#include "AnimatedMeshNode.h"

#include "Graphic/Effect/Texture2ColorEffect.h"
#include "Graphic/Scene/Hierarchy/Node.h"

#include "Particle/ParticleAffector.h"
#include "Particle/ParticleEmitter.h"

#include "Particle/ParticleBoxEmitter.h"
#include "Particle/ParticleRingEmitter.h"
#include "Particle/ParticleMeshEmitter.h"
#include "Particle/ParticlePointEmitter.h"
#include "Particle/ParticleSphereEmitter.h"
#include "Particle/ParticleCylinderEmitter.h"

#include "Particle/ParticleScaleAffector.h"
#include "Particle/ParticleFadeOutAffector.h"
#include "Particle/ParticleGravityAffector.h"
#include "Particle/ParticleRotationAffector.h"
#include "Particle/ParticleAttractionAffector.h"

class ParticleAnimatedMeshNodeEmitter;

//! A particle system scene node.
/** A scene node controlling a particle system. The behavior of the particles
can be controlled by setting the right particle emitters and affectors.
*/
class ParticleSystemNode : public Node
{

public:

	//! Constructor
	ParticleSystemNode(const ActorId actorId, PVWUpdater* updater, 
		WeakBaseRenderComponentPtr renderComponent, bool createDefaultEmitter);

	//! destructor
	virtual ~ParticleSystemNode();

	//! Renders event
	virtual bool PreRender(Scene *pScene);
	virtual bool Render(Scene *pScene);

	//! Set effect for rendering.
	void SetEffect(int size);

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
	virtual NodeType GetType() const { return NT_PARTICLE_SYSTEM; }

	//! returns the material based on the zero based index i. To get the amount
	//! of materials used by this scene node, use GetMaterialCount().
	//! This function is needed for inserting the node into the scene hirachy on a
	//! optimal position for minimizing renderstate changes, but can also be used
	//! to directly modify the material of a scene node.
	virtual eastl::shared_ptr<Material> const& GetMaterial(unsigned int i);

	//! returns amount of materials used by this scene node.
	virtual unsigned int GetMaterialCount() const;

	//! Sets the texture of the specified layer in all materials of this scene node to the new texture.
	/** \param textureLayer Layer of texture to be set. Must be a
	value smaller than MATERIAL_MAX_TEXTURES.
	\param texture New texture to be used. */
	virtual void SetMaterialTexture(unsigned int textureLayer, eastl::shared_ptr<Texture2> texture);

	//! Sets the material type of all materials in this scene node to a new material type.
	/** \param newType New type of material to be set. */
	virtual void SetMaterialType(MaterialType newType);

	//! Creates a particle emitter for an animated mesh scene node
	ParticleAnimatedMeshNodeEmitter* CreateAnimatedMeshNodeEmitter(
		const ActorId actorId, const eastl::shared_ptr<AnimatedMeshNode>& node, 
		bool useNormalDirection = true, const Vector3<float>& direction = Vector3<float>{ 0.f, 0.03f, 0.f },
		float normalDirectionModifier = 100.0f, int mbNumber = -1, bool everyMeshVertex = false, 
		unsigned int minParticlesPerSecond = 5, unsigned int maxParticlesPerSecond = 10,
		const eastl::array<float, 4>& minStartColor = eastl::array<float, 4>{1.f, 0.f, 0.f, 0.f},
		const eastl::array<float, 4>& maxStartColor = eastl::array<float, 4>{1.f, 1.f, 1.f, 1.f},
		unsigned int lifeTimeMin = 2000, unsigned int lifeTimeMax = 4000, int maxAngleDegrees = 0,
		const Vector2<float>& minStartSize = Vector2<float>{ 5.f, 5.f },
		const Vector2<float>& maxStartSize = Vector2<float>{ 5.f, 5.f });

	//! Creates a box particle emitter.
	ParticleBoxEmitter* CreateBoxEmitter(
		const AlignedBox3<float>& box = AlignedBox3<float>(),//{ -10.f,0.f,-10.f,5.f,30.f,10.f },
		const Vector3<float>& direction = Vector3<float>{ 0.f, 0.03f, 0.f },
		unsigned int minParticlesPerSecond = 5, unsigned int maxParticlesPerSecond = 10,
		const eastl::array<float, 4>& minStartColor = eastl::array<float, 4>{1.f, 0.f, 0.f, 0.f},
		const eastl::array<float, 4>& maxStartColor = eastl::array<float, 4>{1.f, 1.f, 1.f, 1.f},
		unsigned int lifeTimeMin = 2000, unsigned int lifeTimeMax = 4000, int maxAngleDegrees = 0,
		const Vector2<float>& minStartSize = Vector2<float>{ 5.f, 5.f },
		const Vector2<float>& maxStartSize = Vector2<float>{ 5.f, 5.f });

	//! Creates a particle emitter for emitting from a cylinder
	ParticleCylinderEmitter* CreateCylinderEmitter(
		const Vector3<float>& center, float radius, const Vector3<float>& normal, float length,
		bool outlineOnly = false, const Vector3<float>& direction = Vector3<float>{ 0.f, 0.5f, 0.f },
		unsigned int minParticlesPerSecond = 5, unsigned int maxParticlesPerSecond = 10,
		const eastl::array<float, 4>& minStartColor = eastl::array<float, 4>{1.f, 0.f, 0.f, 0.f},
		const eastl::array<float, 4>& maxStartColor = eastl::array<float, 4>{1.f, 1.f, 1.f, 1.f},
		unsigned int lifeTimeMin = 2000, unsigned int lifeTimeMax = 4000, int maxAngleDegrees = 0,
		const Vector2<float>& minStartSize = Vector2<float>{ 5.f, 5.f },
		const Vector2<float>& maxStartSize = Vector2<float>{ 5.f, 5.f });

	//! Creates a mesh particle emitter.
	ParticleMeshEmitter* CreateMeshEmitter(
		const eastl::shared_ptr<BaseMesh>& mesh, bool useNormalDirection = true,
		const Vector3<float>& direction = Vector3<float>{ 0.f, 0.03f, 0.f },
		float normalDirectionModifier = 100.0f, int mbNumber = -1, bool everyMeshVertex = false,
		unsigned int minParticlesPerSecond = 5, unsigned int maxParticlesPerSecond = 10,
		const eastl::array<float, 4>& minStartColor = eastl::array<float, 4>{1.f, 0.f, 0.f, 0.f},
		const eastl::array<float, 4>& maxStartColor = eastl::array<float, 4>{1.f, 1.f, 1.f, 1.f},
		unsigned int lifeTimeMin = 2000, unsigned int lifeTimeMax = 4000, int maxAngleDegrees = 0,
		const Vector2<float>& minStartSize = Vector2<float>{ 5.f, 5.f },
		const Vector2<float>& maxStartSize = Vector2<float>{ 5.f, 5.f });

	//! Creates a point particle emitter.
	ParticlePointEmitter* CreatePointEmitter(
		const Vector3<float>& direction = Vector3<float>{ 0.f, 0.03f, 0.f },
		unsigned int minParticlesPerSecond = 5, unsigned int maxParticlesPerSecond = 10,
		const eastl::array<float, 4>& minStartColor = eastl::array<float, 4>{1.f, 0.f, 0.f, 0.f},
		const eastl::array<float, 4>& maxStartColor = eastl::array<float, 4>{1.f, 1.f, 1.f, 1.f},
		unsigned int lifeTimeMin=2000, unsigned int lifeTimeMax=4000, int maxAngleDegrees=0,
		const Vector2<float>& minStartSize = Vector2<float>{ 5.f, 5.f },
		const Vector2<float>& maxStartSize = Vector2<float>{ 5.f, 5.f });

	//! Creates a ring particle emitter.
	ParticleRingEmitter* CreateRingEmitter(
		const Vector3<float>& center, float radius, float ringThickness,
		const Vector3<float>& direction = Vector3<float>{ 0.f, 0.03f, 0.f },
		unsigned int minParticlesPerSecond = 5, unsigned int maxParticlesPerSecond = 10,
		const eastl::array<float, 4>& minStartColor = eastl::array<float, 4>{1.f, 0.f, 0.f, 0.f},
		const eastl::array<float, 4>& maxStartColor = eastl::array<float, 4>{1.f, 1.f, 1.f, 1.f},
		unsigned int lifeTimeMin=2000, unsigned int lifeTimeMax=4000, int maxAngleDegrees=0,
		const Vector2<float>& minStartSize = Vector2<float>{ 5.f, 5.f },
		const Vector2<float>& maxStartSize = Vector2<float>{ 5.f, 5.f });

	//! Creates a sphere particle emitter.
	ParticleSphereEmitter* CreateSphereEmitter(
		const Vector3<float>& center, float radius,
		const Vector3<float>& direction = Vector3<float>{ 0.f, 0.03f, 0.f },
		unsigned int minParticlesPerSecond = 5, unsigned int maxParticlesPerSecond = 10,
		const eastl::array<float, 4>& minStartColor = eastl::array<float, 4>{1.f, 0.f, 0.f, 0.f},
		const eastl::array<float, 4>& maxStartColor = eastl::array<float, 4>{1.f, 1.f, 1.f, 1.f},
		unsigned int lifeTimeMin=2000, unsigned int lifeTimeMax=4000, int maxAngleDegrees=0,
		const Vector2<float>& minStartSize = Vector2<float>{ 5.f, 5.f },
		const Vector2<float>& maxStartSize = Vector2<float>{ 5.f, 5.f });

	//! Creates a point attraction affector. This affector modifies the positions of the
	//! particles and attracts them to a specified point at a specified speed per second.
	ParticleAttractionAffector* CreateAttractionAffector(
		const Vector3<float>& point = Vector3<float>{ 0.f, 0.0f, 0.f }, 
		float speed = 1.0f, bool attract = true, 
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

	eastl::shared_ptr<BlendState> mBlendState;
	eastl::shared_ptr<DepthStencilState> mDepthStencilState;

	eastl::shared_ptr<Visual> mVisual;
	eastl::shared_ptr<MeshBuffer> mMeshBuffer;
	eastl::shared_ptr<Texture2ColorEffect> mEffect;
	eastl::list<eastl::shared_ptr<BaseParticleAffector>> mAffectorList;
	eastl::shared_ptr<BaseParticleEmitter> mEmitter;
	eastl::vector<Particle> mParticles;
	Vector2<float> mParticleSize;
	unsigned int mLastEmitTime;
	int mMaxParticles;

	enum GRAPHIC_ITEM ParticlePrimitive
	{
		PP_POINT=0,
		PP_BILLBOARD,
		PP_POINTSPRITE
	};
	ParticlePrimitive mParticlePrimitive;

	bool mParticlesAreGlobal;
};

#endif

