// Copyright (C) 2002-2012 Nikolaus Gebhardt
// This file is part of the "Irrlicht Engine".
// For conditions of distribution and use, see copyright notice in irrlicht.h

#ifndef ANIMATEDMESHNODE_H
#define ANIMATEDMESHNODE_H

#include "BoneNode.h"
#include "ShadowVolumeNode.h"

enum GRAPHIC_ITEM JointUpdateOnRender
{
	//! do nothing
	JUOR_NONE = 0,

	//! get joints positions from the mesh (for attached nodes, etc)
	JUOR_READ,

	//! control joint positions in the mesh (eg. ragdolls, or set the animation from animateJoints() )
	JUOR_CONTROL
};

class AnimatedMeshNode;

//! Callback interface for catching events of ended animations.
/** Implement this interface and use
AnimatedMeshSceneNode::SetAnimationEndCallback to be able to
be notified if an animation playback has ended.
**/
class AnimationEndCallBack
{
public:

	//! Will be called when the animation playback has ended.
	/** See AnimatedMeshNode::SetAnimationEndCallback for
	more informations.
	\param node: Node of which the animation has ended. */
	virtual void OnAnimationEnd(AnimatedMeshNode* node) = 0;
};

//! Scene node capable of displaying an animated mesh and its shadow.
/** The shadow is optional: If a shadow should be displayed too, just
invoke the AnimatedMeshNode::CreateShadowVolumeNode().*/
class AnimatedMeshNode : public Node
{
public:

	//! Constructor
	AnimatedMeshNode(const ActorId actorId, PVWUpdater* updater,
		WeakBaseRenderComponentPtr renderComponent, const eastl::shared_ptr<BaseAnimatedMesh>& mesh);

	//! Destructor
	virtual ~AnimatedMeshNode() {}

	//! Sets the current frame number.
	/** From now on the animation is played from this frame.
	\param frame: Number of the frame to let the animation be started from.
	The frame number must be a valid frame number of the IMesh used by this
	scene node. Set IAnimatedMesh::getMesh() for details. */
	void SetCurrentFrame(float frame);

	//! Returns the currently displayed frame number.
	float GetFrameNr() const;

	//! Returns the current start frame number.
	int GetStartFrame() const;

	//! Returns the current end frame number.
	int GetEndFrame() const;

	//! Sets looping mode which is on by default.
	/** If set to false, animations will not be played looped. */
	void SetLoopMode(bool playAnimationLooped);

	//! returns the current loop mode
	/** When true the animations are played looped */
	bool GetLoopMode() const;

	//! Sets a callback interface which will be called if an animation playback has ended.
	/** Set this to 0 to disable the callback again.
	Please note that this will only be called when in non looped
	mode, see AnimatedMeshNode::SetLoopMode(). */
	void SetAnimationEndCallback(AnimationEndCallBack* callback = 0);

	//! Sets the frame numbers between the animation is looped.
	/** The default is 0 - MaximalFrameCount of the mesh.
	\param begin: Start frame number of the loop.
	\param end: End frame number of the loop.
	\return True if successful, false if not. */
	bool SetFrameLoop(int begin, int end);

	//! Sets the speed with which the animation is played.
	/** \param framesPerSecond: Frames per second played. */
	void SetAnimationSpeed(float framesPerSecond);

	//! Gets the speed with which the animation is played.
	/** \return Frames per second played. */
	float GetAnimationSpeed() const;

	//! Returns a pointer to a child node, which has the same transformation as
	//! the corrsesponding joint, if the mesh in this scene node is a skinned mesh.
	eastl::shared_ptr<BoneNode> GetJointNode(const char* jointName);

	//! same as GetJointNode(const char* jointName), but based on id
	eastl::shared_ptr<BoneNode> GetJointNode(unsigned int jointID);

	//! Gets joint count.
	unsigned int GetJointCount() const;

	//! Set the joint update mode (0-unused, 1-get joints only, 2-set joints only, 3-move and set)
	void SetJointMode(JointUpdateOnRender mode);

	//! Sets the transition time in seconds (note: This needs to enable joints, and setJointmode maybe set to 2)
	//! you must call animateJoints(), or the mesh will not animate
	void SetTransitionTime(float time);

	//! updates the joint positions of this mesh
	void AnimateJoints(bool calculateAbsolutePositions = true);

	//! render mesh ignoring its transformation. Used with ragdolls. (culling is unaffected)
	void SetRenderFromIdentity(bool on);


	virtual bool PreRender(Scene *pScene);
	virtual bool Render(Scene *pScene);
	
	//! Removes a child from this scene node.
	//! Implemented here, to be able to remove the shadow properly, if there is one,
	//! or to remove attached childs.
	virtual int DetachChild(eastl::shared_ptr<Node> const& child);

	//! OnAnimate() is called just before rendering the whole scene.
	virtual bool OnAnimate(Scene* pScene, unsigned int timeMs);

	//! Creates shadow volume scene node as child of this node.
	/** The shadow can be rendered using the ZPass or the zfail
	method. ZPass is a little bit faster because the shadow volume
	creation is easier, but with this method there occur ugly
	looking artifacs when the camera is inside the shadow volume.
	These error do not occur with the ZFail method.
	\param shadowMesh: Optional custom mesh for shadow volume.
	\param id: Id of the shadow scene node. This id can be used to
	identify the node later.
	\param zfailmethod: If set to true, the shadow will use the
	zfail method, if not, zpass is used.
	\param infinity: Value used by the shadow volume algorithm to
	scale the shadow volume (for zfail shadow volume we support only
	finite shadows, so camera zfar must be larger than shadow back cap,
	which is depend on infinity parameter).
	\return Pointer to the created shadow scene node. This pointer
	should not be dropped. */
	eastl::shared_ptr<ShadowVolumeNode> AddShadowVolumeNode(
		const ActorId actorId, Scene* pScene, 
		const eastl::shared_ptr<BaseMesh>& shadowMesh = 0,
		bool zfailmethod=true, float infinity=10000.0f);

	//! Sets a new mesh
	void SetMesh(const eastl::shared_ptr<BaseAnimatedMesh>& mesh);

	//! Returns the current mesh
	const eastl::shared_ptr<BaseAnimatedMesh>& GetMesh(void);

	//! Returns the visual based on the zero based index i. To get the amount 
	//! of visuals used by this scene node, use GetVisualCount(). 
	//! This function is needed for inserting the node into the scene hierarchy 
	//! at an optimal position for minimizing renderstate changes, but can also 
	//! be used to directly modify the visual of a scene node.
	virtual eastl::shared_ptr<Visual> const& GetVisual(unsigned int i);

	//! return amount of visuals of this scene node.
	virtual unsigned int GetVisualCount() const;

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

	//! Sets if the scene node should not copy the materials of the mesh but use them in a read only style.
	/** In this way it is possible to change the materials a mesh
	causing all mesh scene nodes referencing this mesh to change too. */
	void SetReadOnlyMaterials(bool readonly);

	//! Returns if the scene node should not copy the materials of the mesh but use them in a read only style
	bool IsReadOnlyMaterials() const;

private:

	//! Get a static mesh for the current frame of this animated mesh
	eastl::shared_ptr<BaseMesh> GetMeshForCurrentFrame();

	void BuildFrameNr(unsigned int timeMs);
	void CheckJoints();
	void BeginTransition();

	eastl::vector<eastl::shared_ptr<BlendState>> mBlendStates;
	eastl::vector<eastl::shared_ptr<DepthStencilState>> mDepthStencilStates;
	eastl::shared_ptr<RasterizerState> mRasterizerState;

	eastl::vector<eastl::shared_ptr<Visual>> mVisuals;
	eastl::shared_ptr<BaseMesh> mCurrentFrameMesh;
	eastl::shared_ptr<BaseAnimatedMesh> mMesh;

	int mStartFrame;
	int mEndFrame;
	float mFramesPerSecond;
	float mCurrentFrameNr;

	unsigned int mLastTime;
	unsigned int mTransitionTime; //Transition time in millisecs
	float mTransiting; //is mesh transiting (plus cache of TransitionTime)
	float mTransitingBlend; //0-1, calculated on buildFrameNr

	bool mLooping;
	bool mReadOnlyMaterials;
	bool mRenderFromIdentity;

	eastl::shared_ptr<AnimationEndCallBack> mLoopCallBack;
	int mPassCount;

	eastl::shared_ptr<ShadowVolumeNode> mShadow;

	//0-unused, 1-get joints only, 2-set joints only, 3-move and set
	JointUpdateOnRender mJointMode;
	bool mJointsUsed;

	eastl::vector<eastl::shared_ptr<BoneNode>> mJointChildSceneNodes;
	eastl::vector<Transform> mPretransitingSave;
};


#endif

