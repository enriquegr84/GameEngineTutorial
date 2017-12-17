// Copyright (C) 2002-2012 Nikolaus Gebhardt
// This file is part of the "Irrlicht Engine".
// For conditions of distribution and use, see copyright notice in irrlicht.h

#include "AnimatedMeshNode.h"

#include "Graphic/Renderer/Renderer.h"
#include "Graphic/Effect/Material.h"

#include "Core/OS/OS.h"

#include "Graphic/Scene/Scene.h"

/*
#include "Graphic/Scene/Mesh/MeshCache.h"
#include "Graphic/Scene/Mesh/AnimatedMesh.h"
#include "Graphic/Scene/Mesh/StandardMesh.h"
*/

//! constructor
AnimatedMeshNode::AnimatedMeshNode(const ActorId actorId, 
	WeakBaseRenderComponentPtr renderComponent, const eastl::shared_ptr<AnimatedMesh>& mesh)
:	Node(actorId, renderComponent, RP_TRANSPARENT, NT_ANIMATED_MESH),  
	mMesh(0), mStartFrame(0), mEndFrame(0), mFramesPerSecond(0.025f), mCurrentFrameNr(0.f), 
	mLastTime(0), mLooping(true), mReadOnlyMaterials(false), mRenderFromIdentity(false), 
	mLoopCallBack(0), mPassCount(0), mShadow(0)
{
	#ifdef _DEBUG
	//setDebugName("AnimatedMeshNode");
	#endif

	SetMesh(mesh);
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

		if (mesh)
			mBBox = mesh->GetBoundingBox();
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
		for (unsigned int i=0; i<mMaterials.size(); ++i)
		{
			if (mMaterials[i].IsTransparent())
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

	Matrix4x4<float> toWorld, fromWorld;
	//Transform(&toWorld, &fromWorld);

	bool isTransparentPass = 
		pScene->GetCurrentRenderPass() == RP_TRANSPARENT;
	++mPassCount;

	const eastl::shared_ptr<BaseMesh>& m = GetMeshForCurrentFrame();
	if(m)
	{
		mBBox = m->GetBoundingBox();
	}
	else
	{
		#ifdef _DEBUG
			LogWarning("Animated Mesh returned no mesh to render.");// + Mesh->GetDebugName());
		#endif
	}

	//Renderer::Get()->SetTransform(TS_WORLD, toWorld);

	if (mShadow && mPassCount==1)
		mShadow->UpdateShadowVolumes(pScene);

	// for debug purposes only:
	bool renderMeshes = true;
	Material mat;
	if (DebugDataVisible() && mPassCount==1)
	{
		// overwrite half transparency
		if (DebugDataVisible() & DS_HALF_TRANSPARENCY)
		{
			for (unsigned int i=0; i<m->GetMeshBufferCount(); ++i)
			{
				const eastl::shared_ptr<MeshBuffer<float>>& mb = m->GetMeshBuffer(i);
				mat = mReadOnlyMaterials ? mb->GetMaterial() : mMaterials[i];
				mat.mType = MT_TRANSPARENT_ADD_COLOR;
				/*
				if (mRenderFromIdentity)
					Renderer::Get()->SetTransform(TS_WORLD, Matrix4x4<float>::Identity );
				Renderer::Get()->SetMaterial(mat);
				Renderer::Get()->DrawMeshBuffer(mb);
				*/
			}
			renderMeshes = false;
		}
	}

	// render original meshes
	if (renderMeshes)
	{
		for (unsigned int i=0; i<m->GetMeshBufferCount(); ++i)
		{
			bool transparent = (mMaterials[i].IsTransparent());

			// only render transparent buffer if this is the transparent render pass
			// and solid only in solid pass
			if (transparent == isTransparentPass)
			{
				const shared_ptr<IMeshBuffer>& mb = m->GetMeshBuffer(i);
				const Material& material = mReadOnlyMaterials ? mb->GetMaterial() : mMaterials[i];
				if (m_RenderFromIdentity)
					renderer->SetTransform(ETS_WORLD, Matrix4x4<float>::Identity );
				renderer->SetMaterial(material);
				renderer->DrawMeshBuffer(mb);
			}
		}
	}

	renderer->SetTransform(ETS_WORLD, toWorld);

	// for debug purposes only:
	if (mProps.DebugDataVisible() && mPassCount==1)
	{
		Material debugMat;
		debugMat.Lighting = false;
		debugMat.AntiAliasing=0;
		renderer->SetMaterial(debugMat);
		// show normals
		if (mProps.DebugDataVisible() & EDS_NORMALS)
		{
			// draw normals
			//const float debugNormalLength = pScene->GetParameters()->GetAttributeAsFloat(DEBUG_NORMAL_LENGTH);
			//const eastl::array<float, 4> debugNormalColor = pScene->GetParameters()->GetAttributeAColor(DEBUG_NORMAL_COLOR);

			// draw normals
			const float debugNormalLength = 1.f;
			const eastl::array<float, 4> debugNormalColor{ 255.f, 34.f, 221.f, 221.f };
			const unsigned int count = m->GetMeshBufferCount();

			// draw normals
			for (u32 g=0; g < count; ++g)
			{
				renderer->DrawMeshBufferNormals(m->GetMeshBuffer(g), debugNormalLength, debugNormalColor);
			}
		}

		debugMat.ZBuffer = ECFN_DISABLED;
		debugMat.Lighting = false;
		renderer->SetMaterial(debugMat);

		if (mProps.DebugDataVisible() & EDS_BBOX)
			renderer->Draw3DBox(m_Box, eastl::array<float, 4>{255.f, 255.f, 255.f, 255.f});

		// show bounding box
		if (mProps.DebugDataVisible() & EDS_BBOX_BUFFERS)
		{
			for (u32 g=0; g< m->GetMeshBufferCount(); ++g)
			{
				const shared_ptr<IMeshBuffer>& mb = m->GetMeshBuffer(g);

				renderer->Draw3DBox(mb->GetBoundingBox(), eastl::array<float, 4>{255.f, 190.f, 128.f, 128.f});
			}
		}

		// show mesh
		if (mProps.DebugDataVisible() & EDS_MESH_WIRE_OVERLAY)
		{
			debugMat.Lighting = false;
			debugMat.Wireframe = true;
			debugMat.ZBuffer = ECFN_DISABLED;
			renderer->SetMaterial(debugMat);

			for (u32 g=0; g<m->GetMeshBufferCount(); ++g)
			{
				const eastl::shared_ptr<IMeshBuffer>& mb = m->GetMeshBuffer(g);
				if (mRenderFromIdentity)
					renderer->SetTransform(ETS_WORLD, Matrix4x4<float>::Identity );
				renderer->DrawMeshBuffer(mb);
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
void AnimatedMeshNode::SetAnimationSpeed(f32 framesPerSecond)
{
	mFramesPerSecond = framesPerSecond * 0.001f;
}


float AnimatedMeshNode::GetAnimationSpeed() const
{
	return mFramesPerSecond * 1000.f;
}


//! returns the axis aligned bounding box of this node
const AlignedBox3<float>& AnimatedMeshNode::GetBoundingBox() const
{
	return mBBox;
}


//! returns the material based on the zero based index i. To get the amount
//! of materials used by this scene node, use GetMaterialCount().
//! This function is needed for inserting the node into the scene hirachy on a
//! optimal position for minimizing renderstate changes, but can also be used
//! to directly modify the material of a scene node.
Material& AnimatedMeshNode::GetMaterial(unsigned int i)
{
	if (i >= mMaterials.size())
		return Matrix4x4<float>::Identity();

	return mMaterials[i];
}



//! returns amount of materials used by this scene node.
unsigned int AnimatedMeshNode::GetMaterialCount() const
{
	return mMaterials.size();
}


//! Creates shadow volume scene node as child of this node
//! and returns a pointer to it.
eastl::shared_ptr<ShadowVolumeNode> AnimatedMeshNode::AddShadowVolumeNode(const ActorId actorId,
	Scene* pScene, const eastl::shared_ptr<Mesh>& shadowMesh, bool zfailmethod, float infinity)
{
	const eastl::shared_ptr<Renderer>& renderer = pScene->GetRenderer();

	if (!renderer->QueryFeature(EVDF_STENCIL_BUFFER))
		return 0;

	mShadow = eastl::shared_ptr<ShadowVolumeNode>(
		new ShadowVolumeNode(actorId, WeakBaseRenderComponentPtr(), 
		&Matrix4x4<float>::Identity, shadowMesh ? shadowMesh : mMesh, zfailmethod, infinity));
	shared_from_this()->AddChild(mShadow);

	return mShadow;
}


//! Removes a child from this scene node.
//! Implemented here, to be able to remove the shadow properly, if there is one,
//! or to remove attached childs.
bool AnimatedMeshNode::RemoveChild(ActorId id)
{
	const eastl::shared_ptr<Node>& child = GetChild(id);
	if (child && mShadow == child)
		mShadow = 0;

	if (Node::RemoveChild(id))
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

//! Get a mesh
const eastl::shared_ptr<AnimatedMesh>& AnimatedMeshNode::GetMesh(void)
{
	return mMesh;
}

//! Sets a new mesh
void AnimatedMeshNode::SetMesh(const eastl::shared_ptr<AnimatedMesh>& mesh)
{
	if (!mesh)
		return; // won't set null mesh

	mMesh = mesh;

	// get materials and bounding box
	mBBox = mMesh->GetBoundingBox();

	const eastl::shared_ptr<Mesh>& m = mMesh->GetMesh(0,0);
	if (m)
	{
		mMaterials.clear();
		for (unsigned int i=0; i<m->GetMeshBufferCount(); ++i)
		{
			const eastl::shared_ptr<MeshBuffer>& mb = m->GetMeshBuffer(i);
			if (mb)
				mMaterials.push_back(mb->GetMaterial());
			else
				mMaterials.push_back(Material());
		}
	}

	// get start and begin time
	//SetAnimationSpeed(mMesh->GetAnimationSpeed());
	SetFrameLoop(0, mMesh->GetFrameCount());
}


//! updates the absolute position based on the relative and the parents position
void AnimatedMeshNode::UpdateAbsoluteTransformation()
{
	Node::UpdateAbsoluteTransformation();
}


//! render mesh ignoring its transformation. Used with ragdolls. (culling is unaffected)
void AnimatedMeshNode::SetRenderFromIdentity(bool enable)
{
	mRenderFromIdentity=enable;
}