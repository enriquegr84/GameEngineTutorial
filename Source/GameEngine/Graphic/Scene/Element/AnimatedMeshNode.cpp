// Copyright (C) 2002-2012 Nikolaus Gebhardt
// This file is part of the "Irrlicht Engine".
// For conditions of distribution and use, see copyright notice in irrlicht.h

#include "AnimatedMeshNode.h"

#include "Graphic/Renderer/Renderer.h"
#include "Graphic/Effect/Material.h"

#include "Core/OS/OS.h"

#include "Graphic/Scene/Scene.h"

#include "Application/GameApplication.h"

//! constructor
AnimatedMeshNode::AnimatedMeshNode(const ActorId actorId, PVWUpdater* updater,
	WeakBaseRenderComponentPtr renderComponent, const eastl::shared_ptr<BaseAnimatedMesh>& mesh)
:	Node(actorId, renderComponent, NT_ANIMATED_MESH),
	mMesh(0), mStartFrame(0), mEndFrame(0), mFramesPerSecond(0.025f), mCurrentFrameNr(0.f), 
	mLastTime(0), mLooping(true), mReadOnlyMaterials(false), mRenderFromIdentity(false), 
	mLoopCallBack(0), mPassCount(0), mShadow(0)
{
	mPVWUpdater = updater;

	mRasterizerState = eastl::make_shared<RasterizerState>();

	SetMesh(mesh);
}

//! Sets a new mesh
void AnimatedMeshNode::SetMesh(const eastl::shared_ptr<BaseAnimatedMesh>& mesh)
{
	if (!mesh)
		return; // won't set null mesh

	mMesh = mesh;
	eastl::vector<eastl::shared_ptr<BaseMeshBuffer>> meshBuffers;
	if (dynamic_cast<AnimateMeshMD3*>(mMesh.get()))
	{
		AnimateMeshMD3* animMeshMD3 = dynamic_cast<AnimateMeshMD3*>(mMesh.get());

		eastl::vector<eastl::shared_ptr<MD3Mesh>> meshes;
		animMeshMD3->GetMD3Mesh()->GetMeshes(meshes);

		for (eastl::shared_ptr<MD3Mesh> mesh : meshes)
			for (unsigned int i = 0; i < mesh->GetMeshBufferCount(); ++i)
				meshBuffers.push_back(mesh->GetMeshBuffer(i));
	}
	else
	{
		for (unsigned int i = 0; i<mMesh->GetMeshBufferCount(); ++i)
			meshBuffers.push_back(mMesh->GetMeshBuffer(i));
	}
		
	mVisuals.clear();
	for (unsigned int i = 0; i < meshBuffers.size(); ++i)
	{
		const eastl::shared_ptr<BaseMeshBuffer>& meshBuffer = meshBuffers[i];
		if (meshBuffer)
		{
			mBlendStates.push_back(eastl::make_shared<BlendState>());
			mDepthStencilStates.push_back(eastl::make_shared<DepthStencilState>());

			eastl::shared_ptr<Texture2> textureDiffuse = meshBuffer->GetMaterial()->GetTexture(TT_DIFFUSE);
			if (textureDiffuse)
			{
				eastl::vector<eastl::string> path;
#if defined(_OPENGL_)
				path.push_back("Effects/Texture2EffectVS.glsl");
				path.push_back("Effects/Texture2EffectPS.glsl");
#else
				path.push_back("Effects/Texture2EffectVS.hlsl");
				path.push_back("Effects/Texture2EffectPS.hlsl");
#endif

				eastl::shared_ptr<Texture2Effect> effect = eastl::make_shared<Texture2Effect>(
					ProgramFactory::Get(), path, textureDiffuse,
					meshBuffer->GetMaterial()->mTextureLayer[TT_DIFFUSE].mFilter,
					meshBuffer->GetMaterial()->mTextureLayer[TT_DIFFUSE].mModeU,
					meshBuffer->GetMaterial()->mTextureLayer[TT_DIFFUSE].mModeV);

				eastl::shared_ptr<Visual> visual = eastl::make_shared<Visual>(
					meshBuffer->GetVertice(), meshBuffer->GetIndice(), effect);
				visual->UpdateModelBound();
				mVisuals.push_back(visual);
				mPVWUpdater->Subscribe(mWorldTransform, effect->GetPVWMatrixConstant());
			}
			else
			{
				eastl::vector<eastl::string> path;
#if defined(_OPENGL_)
				path.push_back("Effects/ConstantColorEffectVS.glsl");
				path.push_back("Effects/ConstantColorEffectPS.glsl");
#else
				path.push_back("Effects/ConstantColorEffectVS.hlsl");
				path.push_back("Effects/ConstantColorEffectPS.hlsl");
#endif
				eastl::shared_ptr<ConstantColorEffect> effect =
					eastl::make_shared<ConstantColorEffect>(ProgramFactory::Get(), path, Vector4<float>::Zero());

				eastl::shared_ptr<Visual> visual = eastl::make_shared<Visual>(
					meshBuffer->GetVertice(), meshBuffer->GetIndice(), effect);
				visual->UpdateModelBound();
				mVisuals.push_back(visual);
				mPVWUpdater->Subscribe(mWorldTransform, effect->GetPVWMatrixConstant());
			}
		}
	}

	// clean up joint nodes
	if (mJointsUsed)
	{
		mJointsUsed = false;
		CheckJoints();
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

	BeginTransition(); //transit to this frame if enabled
}

//! Returns the currently displayed frame number.
float AnimatedMeshNode::GetFrameNr() const
{
	return mCurrentFrameNr;
}

//! Get CurrentFrameNr and update transiting settings
void AnimatedMeshNode::BuildFrameNr(unsigned int timeMs)
{
	if (mTransiting != 0.f)
	{
		mTransitingBlend += timeMs * mTransiting;
		if (mTransitingBlend > 1.f)
		{
			mTransiting = 0.f;
			mTransitingBlend = 0.f;
		}
	}

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
			{
				mCurrentFrameNr = (float)mStartFrame;
			}
		}
		else //backwards...
		{
			if (mCurrentFrameNr < mStartFrame)
			{
				mCurrentFrameNr = (float)mEndFrame;
			}
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
	if (mMesh->GetMeshType() == MT_SKINNED)
	{
		// As multiple scene nodes may be sharing the same skinned mesh, we have to
		// re-animate it every frame to ensure that this node gets the mesh that it needs.

		eastl::shared_ptr<SkinnedMesh> skinnedMesh =
			eastl::dynamic_shared_pointer_cast<SkinnedMesh>(mMesh);

		if (mJointMode == JUOR_CONTROL)//write to mesh
			skinnedMesh->TransferJointsToMesh(mJointChildSceneNodes);
		else
			skinnedMesh->AnimateMesh(GetFrameNr(), 1.0f);

		// Update the skinned mesh for the current joint transforms.
		skinnedMesh->SkinMesh();

		if (mJointMode == JUOR_READ)//read from mesh
		{
			skinnedMesh->RecoverJointsFromMesh(mJointChildSceneNodes);

			//---slow---
			for (unsigned int n = 0; n<mJointChildSceneNodes.size(); ++n)
				if (mJointChildSceneNodes[n]->GetParent() == this)
					mJointChildSceneNodes[n]->UpdateAbsoluteTransformationChildren();
		}

		if (mJointMode == JUOR_CONTROL)
		{
			// For meshes other than JUOR_CONTROL, this is done by calling animateMesh()
			//skinnedMesh->UpdateBoundingModel();
		}

		return skinnedMesh;
	}
	else
	{
		int frameNr, frameBlend, beginFrame, endFrame;
		if (dynamic_cast<AnimateMeshMD3*>(mMesh.get()))
		{
			AnimateMeshMD3* animMeshMD3 = dynamic_cast<AnimateMeshMD3*>(mMesh.get());

			eastl::vector<eastl::shared_ptr<MD3Mesh>> meshes;
			animMeshMD3->GetMD3Mesh()->GetMeshes(meshes);
			for (eastl::shared_ptr<MD3Mesh> mesh : meshes)
			{
				if (mesh->GetAnimationCount())
				{
					frameNr = (int)mesh->GetCurrentFrame();
					frameBlend = (int)(Function<float>::Fract(mesh->GetCurrentFrame() * 1000.f));
					beginFrame = mesh->GetAnimation(mesh->GetCurrentAnimation()).mBeginFrame;
					endFrame = mesh->GetAnimation(mesh->GetCurrentAnimation()).mEndFrame;
					mesh->UpdateMesh(frameNr, frameBlend, beginFrame, endFrame);
				}
			}
		}
		
		frameNr = (int)GetFrameNr();
		frameBlend = (int)(Function<float>::Fract(GetFrameNr()) * 1000.f);

		return mMesh->GetMesh(frameNr, frameBlend, mStartFrame, mEndFrame);
	}
}

//! OnAnimate() is called just before rendering the whole scene.
bool AnimatedMeshNode::OnAnimate(Scene* pScene, unsigned int time)
{
	if (mLastTime==0)	// first frame
		mLastTime = time;

	if (dynamic_cast<AnimateMeshMD3*>(mMesh.get()))
	{
		AnimateMeshMD3* animMeshMD3 = dynamic_cast<AnimateMeshMD3*>(mMesh.get());

		eastl::vector<eastl::shared_ptr<MD3Mesh>> meshes;
		animMeshMD3->GetMD3Mesh()->GetMeshes(meshes);
		for (eastl::shared_ptr<MD3Mesh> mesh : meshes)
			mesh->BuildFrameNr(mLooping, time - mLastTime);
	}

	// set CurrentFrameNr
	BuildFrameNr(time - mLastTime);

	mLastTime = time;

	return Node::OnAnimate(pScene, time);
}

bool AnimatedMeshNode::PreRender(Scene* pScene)
{
	if (IsVisible())
	{
		mCurrentFrameMesh = GetMeshForCurrentFrame();

		// update bbox
		for (auto visual : mVisuals)
			visual->UpdateModelBound();

		// because this node supports rendering of mixed mode meshes consisting of
		// transparent and solid material at the same time, we need to go through all
		// materials, check of what type they are and register this node for the right
		// render pass according to that.

		mPassCount = 0;
		int transparentCount = 0;
		int solidCount = 0;

		eastl::vector<eastl::shared_ptr<Material>> materials;
		if (dynamic_cast<AnimateMeshMD3*>(mCurrentFrameMesh.get()))
		{
			AnimateMeshMD3* animMeshMD3 = dynamic_cast<AnimateMeshMD3*>(mCurrentFrameMesh.get());

			eastl::vector<eastl::shared_ptr<MD3Mesh>> meshes;
			animMeshMD3->GetMD3Mesh()->GetMeshes(meshes);

			for (eastl::shared_ptr<MD3Mesh> mesh : meshes)
				for (unsigned int i = 0; i < mesh->GetMeshBufferCount(); ++i)
					materials.push_back(mesh->GetMeshBuffer(i)->GetMaterial());
		}
		else
		{
			for (unsigned int i = 0; i<GetMaterialCount(); ++i)
				materials.push_back(GetMaterial(i));
		}

		// count transparent and solid materials in this scene node
		for (unsigned int i = 0; i < materials.size(); ++i)
		{
			if (materials[i]->IsTransparent())
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

	return Node::PreRender(pScene);
}

void AnimatedMeshNode::Render(unsigned int& visual, bool isTransparentPass,
	Scene *pScene, eastl::vector<Transform> interpolations, eastl::shared_ptr<MD3Mesh> pMesh)
{
	if (pMesh->IsTagMesh())
	{
		Transform tagInterpolation;
		tagInterpolation.SetRotation(pMesh->GetTagInterpolation().mRotation);
		tagInterpolation.SetTranslation(pMesh->GetTagInterpolation().mPosition);
		interpolations.push_back(tagInterpolation);
	}

	for (unsigned int i = 0; i < pMesh->GetMeshBufferCount(); ++i, ++visual)
	{
		if (pMesh->IsRenderMesh())
		{
			const eastl::shared_ptr<BaseMeshBuffer>& meshBuffer = pMesh->GetMeshBuffer(i);
			if (meshBuffer)
			{
				eastl::shared_ptr<Material> material = meshBuffer->GetMaterial();

				// only render transparent buffer if this is the transparent render pass
				// and solid only in solid pass
				bool transparent = (material->IsTransparent());
				if (transparent == isTransparentPass)
				{
					if (material->Update(mBlendStates[i]))
						Renderer::Get()->Unbind(mBlendStates[i]);
					if (material->Update(mDepthStencilStates[i]))
						Renderer::Get()->Unbind(mDepthStencilStates[i]);
					if (material->Update(mRasterizerState))
						Renderer::Get()->Unbind(mRasterizerState);

					Renderer::Get()->SetBlendState(mBlendStates[i]);
					Renderer::Get()->SetDepthStencilState(mDepthStencilStates[i]);
					Renderer::Get()->SetRasterizerState(mRasterizerState);

					eastl::shared_ptr<ConstantBuffer> cbuffer;
					Matrix4x4<float> pvMatrix = pScene->GetActiveCamera()->Get()->GetProjectionViewMatrix();
					cbuffer = mVisuals[visual]->GetEffect()->GetVertexShader()->Get<ConstantBuffer>("PVWMatrix");

					Matrix4x4<float> interpolation = Matrix4x4<float>::Identity();
					eastl::vector<Transform>::reverse_iterator it;
					for (it = interpolations.rbegin(); it != interpolations.rend(); it++)
					{
#if defined(GE_USE_MAT_VEC)
						interpolation = (*it) * interpolation;
#else
						interpolation = interpolation * (*it);
#endif
					}

#if defined(GE_USE_MAT_VEC)
					*cbuffer->Get<Matrix4x4<float>>() = pvMatrix * interpolation;
#else
					*cbuffer->Get<Matrix4x4<float>>() = interpolation * pvMatrix;
#endif

					Renderer* renderer = Renderer::Get();
					renderer->Update(cbuffer);
					renderer->Update(meshBuffer->GetVertice());
					renderer->Draw(mVisuals[visual]);

					Renderer::Get()->SetDefaultBlendState();
					Renderer::Get()->SetDefaultDepthStencilState();
					Renderer::Get()->SetDefaultRasterizerState();
				}
			}
		}
	}

	for (unsigned int n = 0; n < pMesh->GetChildren().size(); n++)
	{
		eastl::shared_ptr<MD3Mesh> pChild = pMesh->GetChildren()[n];
		Render(visual, isTransparentPass, pScene, interpolations, pChild);
	}
}

//! renders the node.
bool AnimatedMeshNode::Render(Scene* pScene)
{
	if (!mMesh || !Renderer::Get())
		return false;

	bool isTransparentPass = 
		pScene->GetCurrentRenderPass() == RP_TRANSPARENT;
	++mPassCount;

	if (mShadow && mPassCount==1)
		mShadow->UpdateShadowVolumes(pScene);

	if (dynamic_cast<AnimateMeshMD3*>(mCurrentFrameMesh.get()))
	{
		unsigned int visual = 0;
		AnimateMeshMD3* animMeshMD3 = dynamic_cast<AnimateMeshMD3*>(mCurrentFrameMesh.get());

		eastl::vector<Transform> interpolations{ mWorldTransform };
		Render(visual, isTransparentPass, pScene, interpolations, animMeshMD3->GetMD3Mesh());
	}
	else
	{
		for (unsigned int i = 0; i < mCurrentFrameMesh->GetMeshBufferCount(); ++i)
		{
			const eastl::shared_ptr<BaseMeshBuffer>& meshBuffer = mCurrentFrameMesh->GetMeshBuffer(i);
			if (meshBuffer)
			{
				eastl::shared_ptr<Material> material = meshBuffer->GetMaterial();

				// only render transparent buffer if this is the transparent render pass
				// and solid only in solid pass
				bool transparent = (material->IsTransparent());
				if (transparent == isTransparentPass)
				{
					if (material->Update(mBlendStates[i]))
						Renderer::Get()->Unbind(mBlendStates[i]);
					if (material->Update(mDepthStencilStates[i]))
						Renderer::Get()->Unbind(mDepthStencilStates[i]);
					if (material->Update(mRasterizerState))
						Renderer::Get()->Unbind(mRasterizerState);

					Renderer::Get()->SetBlendState(mBlendStates[i]);
					Renderer::Get()->SetDepthStencilState(mDepthStencilStates[i]);
					Renderer::Get()->SetRasterizerState(mRasterizerState);

					Renderer* renderer = Renderer::Get();
					renderer->Update(meshBuffer->GetVertice());
					renderer->Draw(mVisuals[i]);

					Renderer::Get()->SetDefaultBlendState();
					Renderer::Get()->SetDefaultDepthStencilState();
					Renderer::Get()->SetDefaultRasterizerState();
				}
			}
		}
	}

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

//! Returns a pointer to a child node, which has the same transformation as
//! the corresponding joint, if the mesh in this scene node is a skinned mesh.
eastl::shared_ptr<BoneNode> AnimatedMeshNode::GetJointNode(const char* jointName)
{
	if (!mMesh || mMesh->GetMeshType() != MT_SKINNED)
	{
		LogWarning("No mesh, or mesh not of skinned mesh type");
		return NULL;
	}

	CheckJoints();

	eastl::shared_ptr<SkinnedMesh> skinnedMesh =
		eastl::dynamic_shared_pointer_cast<SkinnedMesh>(mMesh);

	const int number = skinnedMesh->GetJointNumber(jointName);

	if (number == -1)
	{
		LogWarning("Joint with specified name not found in skinned mesh " + eastl::string(jointName));
		return 0;
	}

	if ((int)mJointChildSceneNodes.size() <= number)
	{
		LogWarning("Joint was found in mesh, but is not loaded into node " + eastl::string(jointName));
		return 0;
	}

	return mJointChildSceneNodes[number];
}

//! Returns a pointer to a child node, which has the same transformation as
//! the corresponding joint, if the mesh in this scene node is a skinned mesh.
eastl::shared_ptr<BoneNode> AnimatedMeshNode::GetJointNode(unsigned int jointID)
{
	if (!mMesh || mMesh->GetMeshType() != MT_SKINNED)
	{
		LogWarning("No mesh, or mesh not of skinned mesh type");
		return NULL;
	}

	CheckJoints();

	if (mJointChildSceneNodes.size() <= jointID)
	{
		LogWarning("Joint not loaded into node");
		return 0;
	}

	return mJointChildSceneNodes[jointID];
}

//! Gets joint count.
unsigned int AnimatedMeshNode::GetJointCount() const
{

	if (!mMesh || mMesh->GetMeshType() != MT_SKINNED)
		return 0;

	eastl::shared_ptr<SkinnedMesh> skinnedMesh = 
		eastl::dynamic_shared_pointer_cast<SkinnedMesh>(mMesh);

	return skinnedMesh->GetJointCount();
}

//! updates the joint positions of this mesh
void AnimatedMeshNode::AnimateJoints(bool calculateAbsolutePositions)
{
	if (mMesh && mMesh->GetMeshType() == MT_SKINNED)
	{
		CheckJoints();
		const float frame = GetFrameNr(); //old?

		eastl::shared_ptr<SkinnedMesh> skinnedMesh =
			eastl::dynamic_shared_pointer_cast<SkinnedMesh>(mMesh);

		skinnedMesh->TransferOnlyJointsHintsToMesh(mJointChildSceneNodes);
		skinnedMesh->AnimateMesh(frame, 1.0f);
		skinnedMesh->RecoverJointsFromMesh(mJointChildSceneNodes);

		//-----------------------------------------
		//		Transition
		//-----------------------------------------

		if (mTransiting != 0.f)
		{
			// Init additional matrices
			if (mPretransitingSave.size()<mJointChildSceneNodes.size())
			{
				for (unsigned int n = mPretransitingSave.size(); n<mJointChildSceneNodes.size(); ++n)
					mPretransitingSave.push_back(Transform());
			}

			for (unsigned int n = 0; n<mJointChildSceneNodes.size(); ++n)
			{
				//------Position------

				mJointChildSceneNodes[n]->GetAbsoluteTransform().
					SetTranslation(Function<float>::Lerp( 
						mPretransitingSave[n].GetTranslation(),
						mJointChildSceneNodes[n]->GetAbsoluteTransform().GetTranslation(), 
						mTransitingBlend));

				//------Rotation------

				//Code is slow, needs to be fixed up
				const Quaternion<float> rotationStart(
					Rotation<4, float>(mPretransitingSave[n].GetRotation()));
				const Quaternion<float> rotationEnd(
					Rotation<4, float>(mJointChildSceneNodes[n]->GetAbsoluteTransform().GetRotation()));

				Quaternion<float> qRotation = Slerp(mTransitingBlend, rotationStart, rotationEnd);
				mJointChildSceneNodes[n]->GetAbsoluteTransform().SetRotation(qRotation);

				//------Scale------
				/*
				mJointChildSceneNodes[n]->GetAbsoluteTransform().
					SetScale(Function<float>::Lerp(
						mPretransitingSave[n].GetScale(),
						mJointChildSceneNodes[n]->GetAbsoluteTransform().GetScale(),
						mTransitingBlend));
				*/
			}
		}

		if (calculateAbsolutePositions)
		{
			//---slow---
			for (unsigned int n = 0; n<mJointChildSceneNodes.size(); ++n)
			{
				if (mJointChildSceneNodes[n]->GetParent() == this)
				{
					mJointChildSceneNodes[n]->UpdateAbsoluteTransformationChildren(); //temp, should be an option
				}
			}
		}
	}
}

/*!
*/
void AnimatedMeshNode::CheckJoints()
{
	if (!mMesh || mMesh->GetMeshType() != MT_SKINNED)
		return;

	if (!mJointsUsed)
	{
		for (unsigned int i = 0; i<mJointChildSceneNodes.size(); ++i)
			DetachChild(mJointChildSceneNodes[i]);
		mJointChildSceneNodes.clear();

		//Create joints for SkinnedMesh
		eastl::shared_ptr<SkinnedMesh> skinnedMesh =
			eastl::dynamic_shared_pointer_cast<SkinnedMesh>(mMesh);

		GameApplication* gameApp = (GameApplication*)Application::App;
		const eastl::shared_ptr<ScreenElementScene>& pScene = gameApp->GetHumanView()->mScene;
		skinnedMesh->AddJoints(mJointChildSceneNodes, this, pScene.get());
		skinnedMesh->RecoverJointsFromMesh(mJointChildSceneNodes);

		mJointsUsed = true;
		mJointMode = JUOR_READ;
	}
}

/*!
*/
void AnimatedMeshNode::BeginTransition()
{
	if (!mJointsUsed)
		return;

	if (mTransitionTime != 0)
	{
		//Check the array is big enough
		if (mPretransitingSave.size()<mJointChildSceneNodes.size())
		{
			for (unsigned int n = mPretransitingSave.size(); n<mJointChildSceneNodes.size(); ++n)
				mPretransitingSave.push_back(Transform());
		}

		//Copy the position of joints
		for (unsigned int n = 0; n<mJointChildSceneNodes.size(); ++n)
			mPretransitingSave[n] = mJointChildSceneNodes[n]->GetRelativeTransform();

		mTransiting = mTransitionTime != 0 ? 1.f / (float)mTransitionTime : 0;
	}
	mTransitingBlend = 0.f;
}

//! Set the joint update mode (0-unused, 1-get joints only, 2-set joints only, 3-move and set)
void AnimatedMeshNode::SetJointMode(JointUpdateOnRender mode)
{
	CheckJoints();
	mJointMode = mode;
}

//! Sets the transition time in seconds (note: This needs to enable joints, and setJointmode maybe set to 2)
//! you must call animateJoints(), or the mesh will not animate
void AnimatedMeshNode::SetTransitionTime(float time)
{
	const unsigned int ttime = (unsigned int)Function<float>::Floor(time*1000.0f);
	if (mTransitionTime == ttime)
		return;
	mTransitionTime = ttime;
	if (ttime != 0)
		SetJointMode(JUOR_CONTROL);
	else
		SetJointMode(JUOR_NONE);
}

//! render mesh ignoring its transformation. Used with ragdolls. (culling is unaffected)
void AnimatedMeshNode::SetRenderFromIdentity(bool enable)
{
	mRenderFromIdentity = enable;
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
	{
		if (mJointsUsed) //stop weird bugs caused while changing parents as the joints are being created
		{
			for (unsigned int i = 0; i<mJointChildSceneNodes.size(); ++i)
			{
				if (mJointChildSceneNodes[i] == child)
				{
					mJointChildSceneNodes[i] = 0; //remove link to child
					break;
				}
			}
		}
		return true;
	}

	return false;
}

//! Returns the visual based on the zero based index i. To get the amount 
//! of visuals used by this scene node, use GetVisualCount(). 
//! This function is needed for inserting the node into the scene hierarchy 
//! at an optimal position for minimizing renderstate changes, but can also 
//! be used to directly modify the visual of a scene node.
eastl::shared_ptr<Visual> const& AnimatedMeshNode::GetVisual(unsigned int i)
{
	if (i >= mVisuals.size())
		return nullptr;

	return mVisuals[i];
}

//! return amount of visuals of this scene node.
unsigned int AnimatedMeshNode::GetVisualCount() const
{
	return mVisuals.size();
}

//! returns the material based on the zero based index i. To get the amount
//! of materials used by this scene node, use GetMaterialCount().
//! This function is needed for inserting the node into the scene hirachy on a
//! optimal position for minimizing renderstate changes, but can also be used
//! to directly modify the material of a scene node.
eastl::shared_ptr<Material> const& AnimatedMeshNode::GetMaterial(unsigned int i)
{
	if (dynamic_cast<AnimateMeshMD3*>(mMesh.get()))
	{
		AnimateMeshMD3* animMeshMD3 = dynamic_cast<AnimateMeshMD3*>(mMesh.get());

		eastl::vector<eastl::shared_ptr<MD3Mesh>> meshes;
		animMeshMD3->GetMD3Mesh()->GetMeshes(meshes);
		unsigned int mbCount = 0;
		for (eastl::shared_ptr<MD3Mesh> mesh : meshes)
			for (unsigned int mbIdx = 0; mbIdx < mesh->GetMeshBufferCount(); mbIdx++, mbCount++)
				if (mbCount == i)
					return mesh->GetMeshBuffer(mbIdx)->GetMaterial();

		return nullptr;
	}
	else
	{
		if (i >= mMesh->GetMeshBufferCount())
			return nullptr;

		return mMesh->GetMeshBuffer(i)->GetMaterial();
	}
}

//! returns amount of materials used by this scene node.
unsigned int AnimatedMeshNode::GetMaterialCount() const
{
	if (dynamic_cast<AnimateMeshMD3*>(mMesh.get()))
	{
		AnimateMeshMD3* animMeshMD3 = dynamic_cast<AnimateMeshMD3*>(mMesh.get());

		eastl::vector<eastl::shared_ptr<MD3Mesh>> meshes;
		animMeshMD3->GetMD3Mesh()->GetMeshes(meshes);
		unsigned int mbCount = 0;
		for (eastl::shared_ptr<MD3Mesh> mesh : meshes)
			mbCount += mesh->GetMeshBufferCount();
		return mbCount;
	}
	else return mMesh->GetMeshBufferCount();
}

//! Sets the texture of the specified layer in all materials of this scene node to the new texture.
/** \param textureLayer Layer of texture to be set. Must be a value smaller than MATERIAL_MAX_TEXTURES.
\param texture New texture to be used. */
void AnimatedMeshNode::SetMaterialTexture(unsigned int textureLayer, eastl::shared_ptr<Texture2> texture)
{
	if (textureLayer >= MATERIAL_MAX_TEXTURES)
		return;

	if (dynamic_cast<AnimateMeshMD3*>(mMesh.get()))
	{
		AnimateMeshMD3* animMeshMD3 = dynamic_cast<AnimateMeshMD3*>(mMesh.get());

		eastl::vector<eastl::shared_ptr<MD3Mesh>> meshes;
		animMeshMD3->GetMD3Mesh()->GetMeshes(meshes);
		for (eastl::shared_ptr<MD3Mesh> mesh : meshes)
			for (unsigned int mbIdx = 0; mbIdx < mesh->GetMeshBufferCount(); mbIdx++)
				mesh->GetMeshBuffer(mbIdx)->GetMaterial()->SetTexture(textureLayer, texture);
	}
	else
	{
		for (unsigned int i = 0; i<GetMaterialCount(); ++i)
			GetMaterial(i)->SetTexture(textureLayer, texture);
	}

	for (unsigned int i = 0; i < GetVisualCount(); ++i)
	{
		eastl::shared_ptr<Visual> visual = GetVisual(i);
		if (visual)
		{
			eastl::shared_ptr<Texture2Effect> textureEffect =
				eastl::dynamic_shared_pointer_cast<Texture2Effect>(visual->GetEffect());
			if (textureEffect)
				textureEffect->SetTexture(texture);
		}
	}
}

//! Sets the material type of all materials in this scene node to a new material type.
/** \param newType New type of material to be set. */
void AnimatedMeshNode::SetMaterialType(MaterialType newType)
{
	if (dynamic_cast<AnimateMeshMD3*>(mMesh.get()))
	{
		AnimateMeshMD3* animMeshMD3 = dynamic_cast<AnimateMeshMD3*>(mMesh.get());

		eastl::vector<eastl::shared_ptr<MD3Mesh>> meshes;
		animMeshMD3->GetMD3Mesh()->GetMeshes(meshes);
		for (eastl::shared_ptr<MD3Mesh> mesh : meshes)
			for (unsigned int mbIdx = 0; mbIdx < mesh->GetMeshBufferCount(); mbIdx++)
				mesh->GetMeshBuffer(mbIdx)->GetMaterial()->mType = newType;
	}
	else
	{
		for (unsigned int i = 0; i<GetMaterialCount(); ++i)
			GetMaterial(i)->mType = newType;
	}
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
