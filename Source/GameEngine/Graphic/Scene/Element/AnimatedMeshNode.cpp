// Copyright (C) 2002-2012 Nikolaus Gebhardt
// This file is part of the "Irrlicht Engine".
// For conditions of distribution and use, see copyright notice in irrlicht.h

#include "AnimatedMeshNode.h"

#include "Graphic/Renderer/Renderer.h"
#include "Graphic/Effect/Material.h"

#include "Core/OS/OS.h"

#include "Graphic/Scene/Scene.h"

//! constructor
AnimatedMeshNode::AnimatedMeshNode(const ActorId actorId, PVWUpdater* updater,
	WeakBaseRenderComponentPtr renderComponent, const eastl::shared_ptr<BaseAnimatedMesh>& mesh)
:	Node(actorId, renderComponent, RP_TRANSPARENT, NT_ANIMATED_MESH),
	mMesh(0), mStartFrame(0), mEndFrame(0), mFramesPerSecond(0.025f), mCurrentFrameNr(0.f), 
	mLastTime(0), mLooping(true), mReadOnlyMaterials(false), mRenderFromIdentity(false), 
	mLoopCallBack(0), mPassCount(0), mShadow(0)
{
	mPVWUpdater = updater;
	SetMesh(mesh);
}


//! Sets a new mesh
void AnimatedMeshNode::SetMesh(const eastl::shared_ptr<BaseAnimatedMesh>& mesh)
{
	if (!mesh)
		return; // won't set null mesh

	mMesh = mesh;

	const eastl::shared_ptr<BaseMesh>& baseMesh = mMesh->GetMesh(0, 0);
	if (baseMesh)
	{

		mVisuals.clear();
		for (unsigned int i = 0; i<baseMesh->GetMeshBufferCount(); ++i)
		{
			const eastl::shared_ptr<MeshBuffer>& meshBuffer = baseMesh->GetMeshBuffer(i);
			if (meshBuffer)
			{
				// Create the visual effect.  The world up-direction is (0,0,1).  Choose
				// the light to point down.
				eastl::shared_ptr<Lighting> lighting = eastl::make_shared<Lighting>();
				lighting->mAmbient = Renderer::Get()->GetClearColor();
				lighting->mAttenuation = { 1.0f, 0.0f, 0.0f, 1.0f };

				eastl::shared_ptr<LightCameraGeometry> geometry = eastl::make_shared<LightCameraGeometry>();

				eastl::string path = FileSystem::Get()->GetPath("Effects/PointLightTextureEffect.hlsl");
				eastl::shared_ptr<PointLightTextureEffect> effect = eastl::make_shared<PointLightTextureEffect>(
					ProgramFactory::Get(), mPVWUpdater->GetUpdater(), path, meshBuffer->GetMaterial(), lighting,
					geometry, eastl::make_shared<Texture2>(DF_UNKNOWN, 0, 0, true),
					SamplerState::MIN_L_MAG_L_MIP_L, SamplerState::WRAP, SamplerState::WRAP);

				eastl::shared_ptr<Visual> visual = eastl::make_shared<Visual>(
					meshBuffer->GetVertice(), meshBuffer->GetIndice(), effect);
				visual->UpdateModelNormals();
				visual->UpdateModelBound();
				mVisuals.push_back(visual);
				mPVWUpdater->Subscribe(mWorldTransform, effect->GetPVWMatrixConstant());
			}
		}
	}

	// get start and begin time
	//SetAnimationSpeed(mMesh->GetAnimationSpeed());
	SetFrameLoop(0, mMesh->GetFrameCount());
}


//! Get a mesh
const eastl::shared_ptr<BaseAnimatedMesh>& AnimatedMeshNode::GetMesh(void)
{
	return mMesh;
}


//! Sets the current frame. From now on the animation is played from this frame.
void AnimatedMeshNode::SetCurrentFrame(float frame)
{
	// if you pass an out of range value, we just clamp it
	mCurrentFrameNr = eastl::clamp ( frame, (float)mStartFrame, (float)mEndFrame );
}


//! Returns the currently displayed frame number.
float AnimatedMeshNode::GetFrameNr() const
{
	return mCurrentFrameNr;
}


//! Get CurrentFrameNr and update transiting settings
void AnimatedMeshNode::BuildFrameNr(unsigned int timeMs)
{
	if ((mStartFrame==mEndFrame))
	{
		mCurrentFrameNr = (float)mStartFrame; //Support for non animated meshes
	}
	else if (mLooping)
	{
		// play animation looped
		mCurrentFrameNr += timeMs * mFramesPerSecond;

		// We have no interpolation between EndFrame and StartFrame,
		// the last frame must be identical to first one with our current solution.
		if (mFramesPerSecond > 0.f) //forwards...
		{
			if (mCurrentFrameNr > mEndFrame)
				mCurrentFrameNr = mStartFrame + fmod(mCurrentFrameNr - mStartFrame, (float)(mEndFrame - mStartFrame));
		}
		else //backwards...
		{
			if (mCurrentFrameNr < mStartFrame)
				mCurrentFrameNr = mEndFrame - fmod(mEndFrame - mCurrentFrameNr, (float)(mEndFrame - mStartFrame));
		}
	}
	else
	{
		// play animation non looped
		mCurrentFrameNr += timeMs * mFramesPerSecond;
		if (mFramesPerSecond > 0.f) //forwards...
		{
			if (mCurrentFrameNr > (float)mEndFrame)
			{
				mCurrentFrameNr = (float)mEndFrame;
				if (mLoopCallBack)
					mLoopCallBack->OnAnimationEnd(this);
			}
		}
		else //backwards...
		{
			if (mCurrentFrameNr < (float)mStartFrame)
			{
				mCurrentFrameNr = (float)mStartFrame;
				if (mLoopCallBack)
					mLoopCallBack->OnAnimationEnd(this);
			}
		}
	}
}

eastl::shared_ptr<BaseMesh> AnimatedMeshNode::GetMeshForCurrentFrame()
{
	int frameNr = (int)GetFrameNr();
	int frameBlend = (int)(Function<float>::Fract( GetFrameNr() ) * 1000.f);
	return mMesh->GetMesh(frameNr, frameBlend, mStartFrame, mEndFrame);
}


//! OnAnimate() is called just before rendering the whole scene.
bool AnimatedMeshNode::OnAnimate(Scene* pScene, unsigned int time)
{
	if (mLastTime==0)	// first frame
		mLastTime = time;

	// set CurrentFrameNr
	BuildFrameNr(time - mLastTime);

	// update bbox
	if (mMesh)
	{
		BaseMesh * mesh = GetMeshForCurrentFrame().get();
	}
	mLastTime = time;

	return Node::OnAnimate(pScene, time);
}

bool AnimatedMeshNode::PreRender(Scene* pScene)
{
	if (IsVisible())
	{
		// because this node supports rendering of mixed mode meshes consisting of
		// transparent and solid material at the same time, we need to go through all
		// materials, check of what type they are and register this node for the right
		// render pass according to that.

		mPassCount = 0;
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
		if (solidCount && !pScene->IsCulled(this))
			pScene->AddToRenderQueue(RP_SOLID, shared_from_this());

		if (transparentCount && !pScene->IsCulled(this))
			pScene->AddToRenderQueue(RP_TRANSPARENT, shared_from_this());
	}

	return Node::PreRender(pScene);
}

//! renders the node.
bool AnimatedMeshNode::Render(Scene* pScene)
{
	if (!mMesh || !Renderer::Get())
		return false;

	bool isTransparentPass = 
		pScene->GetCurrentRenderPass() == RP_TRANSPARENT;
	++mPassCount;

	const eastl::shared_ptr<BaseMesh>& mesh = GetMeshForCurrentFrame();
	if(!mesh)
	{
		#ifdef _DEBUG
			LogWarning("Animated Mesh returned no mesh to render.");
		#endif
	}

	//Renderer::Get()->SetTransform(TS_WORLD, toWorld);

	if (mShadow && mPassCount==1)
		mShadow->UpdateShadowVolumes(pScene);

	// for debug purposes only:
	bool renderMeshes = true;
	if (DebugDataVisible() && mPassCount==1)
	{
		// overwrite half transparency
		if (DebugDataVisible() & DS_HALF_TRANSPARENCY)
		{
			for (unsigned int i=0; i<mesh->GetMeshBufferCount(); ++i)
			{
				eastl::shared_ptr<PointLightTextureEffect> effect =
					eastl::static_pointer_cast<PointLightTextureEffect>(mVisuals[i]->GetEffect());

				const eastl::shared_ptr<MeshBuffer>& mb = mesh->GetMeshBuffer(i);
				eastl::shared_ptr<Material> material = 
					mReadOnlyMaterials ? mb->GetMaterial() : effect->GetMaterial();
				material->mType = MT_TRANSPARENT_ADD_COLOR;

				//if (mRenderFromIdentity)
					//Renderer::Get()->SetTransform(TS_WORLD, Matrix4x4<float>::Identity );
				Renderer::Get()->SetBlendState(material->mBlendState);
				Renderer::Get()->SetRasterizerState(material->mRasterizerState);
				Renderer::Get()->SetDepthStencilState(material->mDepthStencilState);

				effect->SetMaterial(material);
				Renderer::Get()->Draw(mVisuals[i]);

				Renderer::Get()->SetDefaultDepthStencilState();
				Renderer::Get()->SetDefaultRasterizerState();
				Renderer::Get()->SetDefaultBlendState();
			}
			renderMeshes = false;
		}
	}

	// render original meshes
	if (renderMeshes)
	{
		for (unsigned int i=0; i<mesh->GetMeshBufferCount(); ++i)
		{
			eastl::shared_ptr<PointLightTextureEffect> effect =
				eastl::static_pointer_cast<PointLightTextureEffect>(mVisuals[i]->GetEffect());
			bool transparent = (effect->GetMaterial()->IsTransparent());

			// only render transparent buffer if this is the transparent render pass
			// and solid only in solid pass
			if (transparent == isTransparentPass)
			{
				const eastl::shared_ptr<MeshBuffer>& mb = mesh->GetMeshBuffer(i);
				eastl::shared_ptr<Material> material =
					mReadOnlyMaterials ? mb->GetMaterial() : effect->GetMaterial();

				//if (mRenderFromIdentity)
					//Renderer::Get()->SetTransform(TS_WORLD, Matrix4x4<float>::Identity );
				Renderer::Get()->SetBlendState(material->mBlendState);
				Renderer::Get()->SetRasterizerState(material->mRasterizerState);
				Renderer::Get()->SetDepthStencilState(material->mDepthStencilState);

				effect->SetMaterial(material);
				Renderer::Get()->Draw(mVisuals[i]);

				Renderer::Get()->SetDefaultDepthStencilState();
				Renderer::Get()->SetDefaultRasterizerState();
				Renderer::Get()->SetDefaultBlendState();
			}
		}
	}

	//Renderer::Get()->SetTransform(TS_WORLD, toWorld);

	// for debug purposes only:
	/*
	if (DebugDataVisible() && mPassCount==1)
	{
		Material debugMat;
		debugMat.mLighting = false;
		debugMat.mAntiAliasing=0;
		Renderer::Get()->SetMaterial(debugMat);
		// show normals
		if (DebugDataVisible() & DS_NORMALS)
		{
			// draw normals
			//const float debugNormalLength = pScene->GetParameters()->GetAttributeAsFloat(DEBUG_NORMAL_LENGTH);
			//const eastl::array<float, 4> debugNormalColor = pScene->GetParameters()->GetAttributeAColor(DEBUG_NORMAL_COLOR);

			// draw normals
			const float debugNormalLength = 1.f;
			const eastl::array<float, 4> debugNormalColor{ 255.f, 34.f, 221.f, 221.f };
			const unsigned int count = mesh->GetMeshBufferCount();

			// draw normals
			for (unsigned int g=0; g < count; ++g)
			{
				Renderer::Get()->DrawMeshBufferNormals(mesh->GetMeshBuffer(g), debugNormalLength, debugNormalColor);
			}
		}

		debugMat.mZBuffer = CFN_DISABLED;
		debugMat.mLighting = false;
		Renderer::Get()->SetMaterial(debugMat);

		if (DebugDataVisible() & DS_BBOX)
			Renderer::Get()->Draw3DBox(mBox, eastl::array<float, 4>{255.f, 255.f, 255.f, 255.f});

		// show bounding box
		if (DebugDataVisible() & DS_BBOX_BUFFERS)
		{
			for (unsigned int g=0; g< mesh->GetMeshBufferCount(); ++g)
			{
				const eastl::shared_ptr<MeshBuffer>& mb = mesh->GetMeshBuffer(g);

				Renderer::Get()->Draw3DBox(mb->GetBoundingBox(), eastl::array<float, 4>{255.f, 190.f, 128.f, 128.f});
			}
		}

		// show mesh
		if (DebugDataVisible() & DS_MESH_WIRE_OVERLAY)
		{
			debugMat.mLighting = false;
			debugMat.mWireframe = true;
			debugMat.mZBuffer = CFN_DISABLED;
			Renderer::Get()->SetMaterial(debugMat);

			for (unsigned int g=0; g<mesh->GetMeshBufferCount(); ++g)
			{
				const eastl::shared_ptr<MeshBuffer>& mb = mesh->GetMeshBuffer(g);
				if (mRenderFromIdentity)
					Renderer::Get()->SetTransform(TS_WORLD, Matrix4x4<float>::Identity );
				Renderer::Get()->DrawMeshBuffer(mb);
			}
		}
	}
	*/
	return true;
}


//! Returns the current start frame number.
int AnimatedMeshNode::GetStartFrame() const
{
	return mStartFrame;
}


//! Returns the current start frame number.
int AnimatedMeshNode::GetEndFrame() const
{
	return mEndFrame;
}


//! sets the frames between the animation is looped.
//! the default is 0 - MaximalFrameCount of the mesh.
bool AnimatedMeshNode::SetFrameLoop(int begin, int end)
{
	const int maxFrameCount = mMesh->GetFrameCount() - 1;
	if (end < begin)
	{
		mStartFrame = eastl::clamp(end, 0, maxFrameCount);
		mEndFrame = eastl::clamp(begin, mStartFrame, maxFrameCount);
	}
	else
	{
		mStartFrame = eastl::clamp(begin, 0, maxFrameCount);
		mEndFrame = eastl::clamp(end, mStartFrame, maxFrameCount);
	}
	if (mFramesPerSecond < 0)
		SetCurrentFrame((float)mEndFrame);
	else
		SetCurrentFrame((float)mStartFrame);

	return true;
}


//! sets the speed with witch the animation is played
void AnimatedMeshNode::SetAnimationSpeed(float framesPerSecond)
{
	mFramesPerSecond = framesPerSecond * 0.001f;
}


float AnimatedMeshNode::GetAnimationSpeed() const
{
	return mFramesPerSecond * 1000.f;
}

//! Creates shadow volume scene node as child of this node
//! and returns a pointer to it.
eastl::shared_ptr<ShadowVolumeNode> AnimatedMeshNode::AddShadowVolumeNode(const ActorId actorId,
	Scene* pScene, const eastl::shared_ptr<BaseMesh>& shadowMesh, bool zfailmethod, float infinity)
{
	/*
	if (!Renderer::Get()->QueryFeature(VDF_STENCIL_BUFFER))
		return nullptr;
	*/
	mShadow = eastl::shared_ptr<ShadowVolumeNode>(
		new ShadowVolumeNode(actorId, mPVWUpdater, WeakBaseRenderComponentPtr(), 
			shadowMesh ? shadowMesh : mMesh, zfailmethod, infinity));
	shared_from_this()->AttachChild(mShadow);

	return mShadow;
}

//! Removes a child from this scene node.
//! Implemented here, to be able to remove the shadow properly, if there is one,
//! or to remove attached childs.
int AnimatedMeshNode::DetachChild(eastl::shared_ptr<Node> const& child)
{
	if (child && mShadow == child)
		mShadow = 0;

	if (Node::DetachChild(child))
		return true;

	return false;
}

//! Sets looping mode which is on by default. If set to false,
//! animations will not be looped.
void AnimatedMeshNode::SetLoopMode(bool playAnimationLooped)
{
	mLooping = playAnimationLooped;
}

//! returns the current loop mode
bool AnimatedMeshNode::GetLoopMode() const
{
	return mLooping;
}

//! Sets a callback interface which will be called if an animation
//! playback has ended. Set this to 0 to disable the callback again.
void AnimatedMeshNode::SetAnimationEndCallback(AnimationEndCallBack* callback)
{
	if (callback == mLoopCallBack.get())
		return;
	
	mLoopCallBack.reset(callback);
}

//! returns the material based on the zero based index i. To get the amount
//! of materials used by this scene node, use GetMaterialCount().
//! This function is needed for inserting the node into the scene hirachy on a
//! optimal position for minimizing renderstate changes, but can also be used
//! to directly modify the material of a scene node.
eastl::shared_ptr<Material> const& AnimatedMeshNode::GetMaterial(unsigned int i)
{
	if (i >= mMesh->GetMeshBufferCount())
		return nullptr;

	return mMesh->GetMeshBuffer(i)->GetMaterial();
}

//! returns amount of materials used by this scene node.
unsigned int AnimatedMeshNode::GetMaterialCount() const
{
	return mMesh->GetMeshBufferCount();
}

//! Sets the texture of the specified layer in all materials of this scene node to the new texture.
/** \param textureLayer Layer of texture to be set. Must be a value smaller than MATERIAL_MAX_TEXTURES.
\param texture New texture to be used. */
void AnimatedMeshNode::SetMaterialTexture(unsigned int textureLayer, Texture2* texture)
{
	if (textureLayer >= MATERIAL_MAX_TEXTURES)
		return;

	for (unsigned int i = 0; i<GetMaterialCount(); ++i)
		GetMaterial(i)->SetTexture(textureLayer, texture);
}

//! Sets the material type of all materials in this scene node to a new material type.
/** \param newType New type of material to be set. */
void AnimatedMeshNode::SetMaterialType(MaterialType newType)
{
	for (unsigned int i = 0; i<GetMaterialCount(); ++i)
		GetMaterial(i)->mType = newType;
}

//! Sets if the scene node should not copy the materials of the mesh but use them in a read only style.
void AnimatedMeshNode::SetReadOnlyMaterials(bool readonly)
{
	mReadOnlyMaterials = readonly;
}

//! Returns if the scene node should not copy the materials of the mesh but use them in a read only style
bool AnimatedMeshNode::IsReadOnlyMaterials() const
{
	return mReadOnlyMaterials;
}

//! render mesh ignoring its transformation. Used with ragdolls. (culling is unaffected)
void AnimatedMeshNode::SetRenderFromIdentity(bool enable)
{
	mRenderFromIdentity=enable;
}