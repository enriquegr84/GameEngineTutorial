// Copyright (C) 2002-2012 Nikolaus Gebhardt
// This file is part of the "Irrlicht Engine".
// For conditions of distribution and use, see copyright notice in irrlicht.h

#include "ParticleSystemNode.h"

#include "Graphic/Renderer/Renderer.h"
#include "Graphic/Effect/Material.h"

#include "Core/OS/OS.h"

#include "Graphic/Scene/Scene.h"

#include "ParticleAnimatedMeshNodeEmitter.h"

//#include "Utilities/ViewFrustum.h"

//! constructor
ParticleSystemNode::ParticleSystemNode(const ActorId actorId, PVWUpdater* updater,
	WeakBaseRenderComponentPtr renderComponent, bool createDefaultEmitter)
:	Node(actorId, renderComponent, NT_PARTICLE_SYSTEM),
	mEmitter(0), mParticleSize(Vector2<float>{5.f, 5.f}), mLastEmitTime(0),
	mMaxParticles(0xffff), mParticlesAreGlobal(true)
{
	mPVWUpdater = updater;
	mMeshBuffer = eastl::make_shared<MeshBuffer>();

	mBlendState = eastl::make_shared<BlendState>();
	mDepthStencilState = eastl::make_shared<DepthStencilState>();

	if (createDefaultEmitter)
		SetEmitter(eastl::shared_ptr<ParticleBoxEmitter>(CreateBoxEmitter()));
}

//! Set effect for rendering.
void ParticleSystemNode::SetEffect(int size)
{
	struct Vertex
	{
		Vector3<float> position;
		Vector4<float> color;
		Vector2<float> tcoord;
	};
	VertexFormat vformat;
	vformat.Bind(VA_POSITION, DF_R32G32B32_FLOAT, 0);
	vformat.Bind(VA_COLOR, DF_R32G32B32A32_FLOAT, 0);
	vformat.Bind(VA_TEXCOORD, DF_R32G32_FLOAT, 0);

	MeshBuffer* meshBuffer = new MeshBuffer(
		vformat, size * 4, size, sizeof(unsigned int));
	for (unsigned int i = 0; i < GetMaterialCount(); ++i)
		meshBuffer->GetMaterial() = GetMaterial(i);
	mMeshBuffer.reset(meshBuffer);

	eastl::string path = FileSystem::Get()->GetPath("Effects/Texture2ColorEffect.hlsl");
	mEffect = eastl::make_shared<Texture2ColorEffect>(
		ProgramFactory::Get(), path, mMeshBuffer->GetMaterial()->GetTexture(0),
		SamplerState::MIN_L_MAG_L_MIP_P, SamplerState::WRAP, SamplerState::WRAP);
}


void ParticleSystemNode::ReallocateBuffers()
{
	if (mParticles.size() * 4 != mMeshBuffer->GetVertice()->GetNumElements() ||
		mParticles.size() * 2 != mMeshBuffer->GetIndice()->GetNumPrimitives())
	{
		MeshBuffer* meshBuffer = new MeshBuffer(mMeshBuffer->GetVertice()->GetFormat(), 
			mParticles.size() * 4, mParticles.size() * 2, sizeof(unsigned int));
		for (unsigned int i = 0; i < GetMaterialCount(); ++i)
			meshBuffer->GetMaterial() = GetMaterial(i);
		mMeshBuffer.reset(meshBuffer);

		// fill vertices
		for (unsigned int i = 0; i<mMeshBuffer->GetVertice()->GetNumElements(); i += 4)
		{
			mMeshBuffer->TCoord(0, 0 + i) = Vector2<float>{ 0.0f, 0.0f };
			mMeshBuffer->TCoord(0, 1 + i) = Vector2<float>{ 0.0f, 1.0f };
			mMeshBuffer->TCoord(0, 2 + i) = Vector2<float>{ 1.0f, 1.0f };
			mMeshBuffer->TCoord(0, 3 + i) = Vector2<float>{ 1.0f, 0.0f };
		}

		// fill indices
		int vertices = 0;
		for (unsigned int i = 0; i<mMeshBuffer->GetIndice()->GetNumPrimitives(); i+=2, vertices +=4)
		{
			mMeshBuffer->GetIndice()->SetTriangle(i,
				(unsigned int)0 + vertices, (unsigned int)2 + vertices, (unsigned int)1 + vertices);
			mMeshBuffer->GetIndice()->SetTriangle(i + 1,
				(unsigned int)0 + vertices, (unsigned int)3 + vertices, (unsigned int)2 + vertices);
		}

		if (mMeshBuffer)
		{
			mVisual.reset(new Visual(
				mMeshBuffer->GetVertice(), mMeshBuffer->GetIndice(), mEffect));
			mVisual->UpdateModelBound();
		}
	}
}


//! destructor
ParticleSystemNode::~ParticleSystemNode()
{
	RemoveAllAffectors();
}

//! prerender
bool ParticleSystemNode::PreRender(Scene *pScene)
{
	DoParticleSystem(Timer::GetTime());

	if (IsVisible() && (mParticles.size() != 0))
	{
		int transparentCount = 0;
		int solidCount = 0;

		// count transparent and solid materials in this scene node
		for (unsigned int i = 0; i < GetMaterialCount(); ++i)
		{
			if (GetMaterial(i)->IsTransparent())
				++transparentCount;
			else
				++solidCount;

			if (solidCount && transparentCount)
				break;
		}

		// register according to material types counted
		if (!pScene->IsCulled(this))
		{
			if (solidCount)
				pScene->AddToRenderQueue(RP_SOLID, shared_from_this());

			if (transparentCount)
				pScene->AddToRenderQueue(RP_TRANSPARENT, shared_from_this());
		}
	}

	return true;
}

//
// LightSceneNode::Render
//
bool ParticleSystemNode::Render(Scene *pScene)
{
	const eastl::shared_ptr<CameraNode>& cameraNode = pScene->GetActiveCamera();

	if (!cameraNode || !Renderer::Get())
		return false;

	// reallocate arrays, if they are too small
	ReallocateBuffers();

	int idx = 0;
	for (unsigned int i = 0; i < mParticles.size(); ++i)
	{
		const Particle& particle = mParticles[i];

		float f = 0.5f * particle.mSize[0];
		const Vector4<float> horizontal = cameraNode->Get()->GetRVector() * f;

		f = 0.5f * particle.mSize[1];
		const Vector4<float> vertical = cameraNode->Get()->GetUVector() * f;

		mMeshBuffer->Position(0 + idx) = particle.mPos + HProject(horizontal + vertical);
		mMeshBuffer->Color(0, 0 + idx) = particle.mColor;
		mMeshBuffer->Position(1 + idx) = particle.mPos + HProject(horizontal - vertical);
		mMeshBuffer->Color(0, 1 + idx) = particle.mColor;
		mMeshBuffer->Position(2 + idx) = particle.mPos + HProject(-horizontal - vertical);
		mMeshBuffer->Color(0, 2 + idx) = particle.mColor;
		mMeshBuffer->Position(3 + idx) = particle.mPos + HProject(-horizontal + vertical);
		mMeshBuffer->Color(0, 3 + idx) = particle.mColor;

		idx += 4;
	}

	eastl::shared_ptr<ConstantBuffer> cbuffer;
	cbuffer = mEffect->GetVertexShader()->Get<ConstantBuffer>("PVWMatrix");
	*cbuffer->Get<Matrix4x4<float>>() = cameraNode->Get()->GetProjectionViewMatrix();

	for (unsigned int i = 0; i < GetMaterialCount(); ++i)
	{
		GetMaterial(i)->Update(mBlendState);
		GetMaterial(i)->Update(mDepthStencilState);
	}

	Renderer::Get()->SetBlendState(mBlendState);
	Renderer::Get()->SetDepthStencilState(mDepthStencilState);

	Renderer* renderer = Renderer::Get();
	renderer->Update(mMeshBuffer->GetVertice());
	renderer->Update(cbuffer);
	renderer->Draw(mVisual);

	Renderer::Get()->SetDefaultBlendState();
	Renderer::Get()->SetDefaultDepthStencilState();

	/*
	// for debug purposes only:
	if (DebugDataVisible() & DS_BBOX )
	{
		Renderer::Get()->SetTransform(TS_WORLD, toWorld);
		Material debM;
		debM.mLighting = false;
		Renderer::Get()->SetMaterial(debM);
		Renderer::Get()->Draw3DBox(mBuffer->mBoundingBox, eastl::array<float, 4>{0.f, 255.f, 255.f, 255.f});
	}
	*/
	return Node::Render(pScene);
}


void ParticleSystemNode::DoParticleSystem(unsigned int time)
{
	if (mLastEmitTime==0)
	{
		mLastEmitTime = time;
		return;
	}

	unsigned int now = time;
	unsigned int timediff = time - mLastEmitTime;
	mLastEmitTime = time;

	// run emitter

	if (mEmitter && IsVisible())
	{
		Particle* array = 0;
		int newParticles = mEmitter->Emitt(now, timediff, array);

		if (newParticles && array)
		{
			int j = mParticles.size();
			if (newParticles > 16250 - j)
				newParticles = 16250 - j;

			mParticles.resize(j + newParticles);
			for (int i = j; i<j + newParticles; ++i)
			{
				mParticles[i] = array[i - j];

				Vector4<float> startVector;
				GetAbsoluteTransform().GetHMatrix().Transformation(
					HLift(mParticles[i].mStartVector, 0.f), startVector);
				mParticles[i].mStartVector = HProject(startVector);
				if (mParticlesAreGlobal)
				{
					Vector4<float> positionVector;
					GetAbsoluteTransform().GetHMatrix().Transformation(
						HLift(mParticles[i].mPos, 0.f), positionVector);
					mParticles[i].mPos = HProject(positionVector);
					mParticles[i].mPos += GetAbsoluteTransform().GetTranslation();
				}
			}
		}
	}

	// run affectors
	eastl::list<eastl::shared_ptr<BaseParticleAffector>>::iterator ait = mAffectorList.begin();
	for (; ait != mAffectorList.end(); ++ait)
		(*ait)->Affect(now, mParticles.data(), mParticles.size());

	// animate all particles
	float scale = (float)timediff;

	for (unsigned int i=0; i<mParticles.size();)
	{
		// erase is pretty expensive!
		if (now > mParticles[i].mEndTime)
		{
			// Particle order does not seem to matter.
			// So we can delete by switching with last particle and deleting that one.
			// This is a lot faster and speed is very important here as the erase otherwise
			// can cause noticable freezes.
			mParticles[i] = mParticles[mParticles.size()-1];
			mParticles.pop_back();
		}
		else
		{
			mParticles[i].mPos += (mParticles[i].mVector * scale);
			++i;
		}
	}
}


//! Sets if the particles should be global. If it is, the particles are affected by
//! the movement of the particle system scene node too, otherwise they completely
//! ignore it. Default is true.
void ParticleSystemNode::SetParticlesAreGlobal(bool global)
{
	mParticlesAreGlobal = global;
}

//! Remove all currently visible particles
void ParticleSystemNode::ClearParticles()
{
	mParticles.clear();
}

//! Gets the particle emitter, which creates the particles.
eastl::shared_ptr<BaseParticleEmitter> ParticleSystemNode::GetEmitter()
{
	return mEmitter;
}

//! Sets the particle emitter, which creates the particles.
void ParticleSystemNode::SetEmitter(const eastl::shared_ptr<BaseParticleEmitter>& emitter)
{
	mEmitter = emitter;
}

//! Adds new particle effector to the particle system.
void ParticleSystemNode::AddAffector(const eastl::shared_ptr<BaseParticleAffector>& affector)
{
	mAffectorList.push_back(affector);
}

//! Get a list of all particle affectors.
const eastl::list<eastl::shared_ptr<BaseParticleAffector>>& ParticleSystemNode::GetAffectors() const
{
	return mAffectorList;
}

//! Removes all particle affectors in the particle system.
void ParticleSystemNode::RemoveAllAffectors()
{
	eastl::list<eastl::shared_ptr<BaseParticleAffector>>::iterator it = mAffectorList.begin();
	while (it != mAffectorList.end())
		it = mAffectorList.erase(it);
}

//! Returns the visual based on the zero based index i. To get the amount 
//! of visuals used by this scene node, use GetVisualCount(). 
//! This function is needed for inserting the node into the scene hierarchy 
//! at an optimal position for minimizing renderstate changes, but can also 
//! be used to directly modify the visual of a scene node.
eastl::shared_ptr<Visual> const& ParticleSystemNode::GetVisual(unsigned int i)
{
	return mVisual;
}

//! return amount of visuals of this scene node.
unsigned int ParticleSystemNode::GetVisualCount() const
{
	return 1;
}

//! returns the material based on the zero based index i. To get the amount
//! of materials used by this scene node, use GetMaterialCount().
//! This function is needed for inserting the node into the scene hirachy on a
//! optimal position for minimizing renderstate changes, but can also be used
//! to directly modify the material of a scene node.
eastl::shared_ptr<Material> const& ParticleSystemNode::GetMaterial(unsigned int i)
{
	return mMeshBuffer->GetMaterial();
}

//! returns amount of materials used by this scene node.
unsigned int ParticleSystemNode::GetMaterialCount() const
{
	return 1;
}

//! Sets the texture of the specified layer in all materials of this scene node to the new texture.
/** \param textureLayer Layer of texture to be set. Must be a value smaller than MATERIAL_MAX_TEXTURES.
\param texture New texture to be used. */
void ParticleSystemNode::SetMaterialTexture(unsigned int textureLayer, eastl::shared_ptr<Texture2> texture)
{
	if (textureLayer >= MATERIAL_MAX_TEXTURES)
		return;

	for (unsigned int i = 0; i<GetMaterialCount(); ++i)
		GetMaterial(i)->SetTexture(textureLayer, texture);
}

//! Sets the material type of all materials in this scene node to a new material type.
/** \param newType New type of material to be set. */
void ParticleSystemNode::SetMaterialType(MaterialType newType)
{
	for (unsigned int i = 0; i<GetMaterialCount(); ++i)
		GetMaterial(i)->mType = newType;
}

//! Creates a particle emitter for an animated mesh scene node
ParticleAnimatedMeshNodeEmitter* ParticleSystemNode::CreateAnimatedMeshNodeEmitter(const ActorId actorId,
	const eastl::shared_ptr<AnimatedMeshNode>& node, bool useNormalDirection, const Vector3<float>& direction, 
	float normalDirectionModifier, int mbNumber, bool everyMeshVertex, unsigned int minParticlesPerSecond, 
	unsigned int maxParticlesPerSecond, const eastl::array<float, 4>& minStartColor, const eastl::array<float, 4>& maxStartColor, 
	unsigned int lifeTimeMin, unsigned int lifeTimeMax, int maxAngleDegrees, 
	const Vector2<float>& minStartSize, const Vector2<float>& maxStartSize )
{
	
	return new ParticleAnimatedMeshNodeEmitter(actorId, mPVWUpdater, WeakBaseRenderComponentPtr(),
		node, useNormalDirection, direction, normalDirectionModifier, mbNumber, everyMeshVertex, 
		minParticlesPerSecond, maxParticlesPerSecond, minStartColor, maxStartColor,
		lifeTimeMin, lifeTimeMax, maxAngleDegrees, minStartSize, maxStartSize );
}


//! Creates a box particle emitter.
ParticleBoxEmitter* ParticleSystemNode::CreateBoxEmitter(const AlignedBox3<float>& box,
	const Vector3<float>& direction, unsigned int minParticlesPerSecond, unsigned int maxParticlesPerSecond, 
	const eastl::array<float, 4>& minStartColor, const eastl::array<float, 4>& maxStartColor, unsigned int lifeTimeMin, 
	unsigned int lifeTimeMax, int maxAngleDegrees, const Vector2<float>& minStartSize, const Vector2<float>& maxStartSize )
{
	return new ParticleBoxEmitter(box, direction, minParticlesPerSecond, maxParticlesPerSecond, 
		minStartColor, maxStartColor, lifeTimeMin, lifeTimeMax, maxAngleDegrees, minStartSize, maxStartSize );
}


//! Creates a particle emitter for emitting from a cylinder
ParticleCylinderEmitter* ParticleSystemNode::CreateCylinderEmitter(
	const Vector3<float>& center, float radius, const Vector3<float>& normal, float length, bool outlineOnly, 
	const Vector3<float>& direction, unsigned int minParticlesPerSecond, unsigned int maxParticlesPerSecond,
	const eastl::array<float, 4>& minStartColor, const eastl::array<float, 4>& maxStartColor, unsigned int lifeTimeMin, 
	unsigned int lifeTimeMax, int maxAngleDegrees, const Vector2<float>& minStartSize, const Vector2<float>& maxStartSize )
{
	return new ParticleCylinderEmitter( center, radius, normal, length, outlineOnly, direction, minParticlesPerSecond, 
		maxParticlesPerSecond, minStartColor, maxStartColor, lifeTimeMin, lifeTimeMax, maxAngleDegrees, minStartSize, maxStartSize );
}


//! Creates a mesh particle emitter.
ParticleMeshEmitter* ParticleSystemNode::CreateMeshEmitter(
	const eastl::shared_ptr<BaseMesh>& mesh, bool useNormalDirection, const Vector3<float>& direction, 
	float normalDirectionModifier, int mbNumber, bool everyMeshVertex, unsigned int minParticlesPerSecond, 
	unsigned int maxParticlesPerSecond, const eastl::array<float, 4>& minStartColor, const eastl::array<float, 4>& maxStartColor, 
	unsigned int lifeTimeMin, unsigned int lifeTimeMax, int maxAngleDegrees, const Vector2<float>& minStartSize, const Vector2<float>& maxStartSize)
{
	return new ParticleMeshEmitter( mesh, useNormalDirection, direction, normalDirectionModifier, mbNumber, everyMeshVertex, 
		minParticlesPerSecond, maxParticlesPerSecond, minStartColor, maxStartColor, lifeTimeMin, lifeTimeMax, maxAngleDegrees, 
		minStartSize, maxStartSize );
}


//! Creates a point particle emitter.
ParticlePointEmitter* ParticleSystemNode::CreatePointEmitter(
	const Vector3<float>& direction, unsigned int minParticlesPerSecond, unsigned int maxParticlesPerSecond,
	const eastl::array<float, 4>& minStartColor, const eastl::array<float, 4>& maxStartColor, unsigned int lifeTimeMin, 
	unsigned int lifeTimeMax, int maxAngleDegrees, const Vector2<float>& minStartSize, const Vector2<float>& maxStartSize )
{
	return new ParticlePointEmitter(direction, minParticlesPerSecond, maxParticlesPerSecond, 
		minStartColor, maxStartColor, lifeTimeMin, lifeTimeMax, maxAngleDegrees, minStartSize, maxStartSize );
}


//! Creates a ring particle emitter.
ParticleRingEmitter* ParticleSystemNode::CreateRingEmitter(
	const Vector3<float>& center, float radius, float ringThickness, const Vector3<float>& direction,
	unsigned int minParticlesPerSecond, unsigned int maxParticlesPerSecond, const eastl::array<float, 4>& minStartColor, 
	const eastl::array<float, 4>& maxStartColor, unsigned int lifeTimeMin, unsigned int lifeTimeMax, int maxAngleDegrees, 
	const Vector2<float>& minStartSize, const Vector2<float>& maxStartSize )
{
	return new ParticleRingEmitter(center, radius, ringThickness, direction, minParticlesPerSecond, maxParticlesPerSecond, 
		minStartColor, maxStartColor, lifeTimeMin, lifeTimeMax, maxAngleDegrees, minStartSize, maxStartSize );
}


//! Creates a sphere particle emitter.
ParticleSphereEmitter* ParticleSystemNode::CreateSphereEmitter(
	const Vector3<float>& center, float radius, const Vector3<float>& direction, unsigned int minParticlesPerSecond, 
	unsigned int maxParticlesPerSecond, const eastl::array<float, 4>& minStartColor, const eastl::array<float, 4>& maxStartColor,
	unsigned int lifeTimeMin, unsigned int lifeTimeMax, int maxAngleDegrees, const Vector2<float>& minStartSize,
	const Vector2<float>& maxStartSize )
{
	return new ParticleSphereEmitter(center, radius, direction, minParticlesPerSecond, maxParticlesPerSecond, minStartColor, 
		maxStartColor, lifeTimeMin, lifeTimeMax, maxAngleDegrees, minStartSize, maxStartSize );
}


//! Creates a point attraction affector. This affector modifies the positions of the
//! particles and attracts them to a specified point at a specified speed per second.
ParticleAttractionAffector* ParticleSystemNode::CreateAttractionAffector(
	const Vector3<float>& point, float speed, bool attract, bool affectX, bool affectY, bool affectZ )
{
	return new ParticleAttractionAffector(point, speed, attract, affectX, affectY, affectZ );
}

//! Creates a scale particle affector.
BaseParticleAffector* ParticleSystemNode::CreateScaleParticleAffector(const Vector2<float>& scaleTo)
{
	return new ParticleScaleAffector(scaleTo);
}


//! Creates a fade out particle affector.
ParticleFadeOutAffector* ParticleSystemNode::CreateFadeOutParticleAffector(
	const eastl::array<float, 4>& targetColor, unsigned int timeNeededToFadeOut)
{
	return new ParticleFadeOutAffector(targetColor, timeNeededToFadeOut);
}


//! Creates a gravity affector.
ParticleGravityAffector* ParticleSystemNode::CreateGravityAffector(
	const Vector3<float>& gravity, unsigned int timeForceLost)
{
	return new ParticleGravityAffector(gravity, timeForceLost);
}


//! Creates a rotation affector. This affector rotates the particles around a specified pivot
//! point.  The speed represents Degrees of rotation per second.
ParticleRotationAffector* ParticleSystemNode::CreateRotationAffector(
	const Vector3<float>& speed, const Vector3<float>& pivotPoint )
{
	return new ParticleRotationAffector(speed, pivotPoint);
}

