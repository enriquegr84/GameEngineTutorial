// Copyright (C) 2002-2012 Nikolaus Gebhardt
// This file is part of the "Irrlicht Engine".
// For conditions of distribution and use, see copyright notice in irrlicht.h

#ifndef SKINNEDMESH_H
#define SKINNEDMESH_H

#include "Graphic/Scene/Element/Mesh/Mesh.h"
#include "Graphic/Resource/Buffer/SkinMeshBuffer.h"

enum InterpolationMode
{
	// constant does use the current key-values without interpolation
	IM_CONSTANT = 0,

	// linear interpolation
	IM_LINEAR,

	//! count of all available interpolation modes
	IM_COUNT
};

class Scene;
class BoneNode;
class AnimatedMeshNode;

//! Interface for using some special functions of Skinned meshes
class BaseSkinnedMesh : public BaseAnimatedMesh
{
public:

	//! Gets joint count.
	/** \return Amount of joints in the skeletal animated mesh. */
	virtual unsigned int GetJointCount() const = 0;

	//! Gets the name of a joint.
	/** \param number: Zero based index of joint. The last joint
	has the number getJointCount()-1;
	\return Name of joint and null if an error happened. */
	virtual const char* GetJointName(unsigned int number) const = 0;

	//! Gets a joint number from its name
	/** \param name: Name of the joint.
	\return Number of the joint or -1 if not found. */
	virtual int GetJointNumber(const char* name) const = 0;

	//! Use animation from another mesh
	/** The animation is linked (not copied) based on joint names
	so make sure they are unique.
	\return True if all joints in this mesh were
	matched up (empty names will not be matched, and it's case
	sensitive). Unmatched joints will not be animated. */
	virtual bool UseAnimationFrom(const BaseSkinnedMesh *mesh) = 0;

	//! Update Normals when Animating
	/** \param on If false don't animate, which is faster.
	Else update normals, which allows for proper lighting of
	animated meshes. */
	virtual void UpdateNormalsWhenAnimating(bool on) = 0;

	//! Sets Interpolation Mode
	virtual void SetInterpolationMode(InterpolationMode mode) = 0;

	//! Animates this mesh's joints based on frame input
	virtual void AnimateMesh(float frame, float blend) = 0;

	//! Preforms a software skin on this mesh based of joint positions
	virtual void SkinMesh() = 0;

	//! converts the vertex type of all meshbuffers to tangents.
	/** E.g. used for bump mapping. */
	virtual void ConvertMeshToTangents() = 0;

	//! Allows to enable hardware skinning.
	/* This feature is not implementated in Irrlicht yet */
	virtual bool SetHardwareSkinning(bool on) = 0;

	//! A vertex weight
	struct Weight
	{
		//! Index of the mesh buffer
		unsigned int mBufferId; 

		//! Index of the vertex
		unsigned int mVertexId; //Store global ID here

		//! Weight Strength/Percentage (0-1)
		float mStrength;

		private:
			//! Internal members used by SkinnedMesh
			friend class SkinnedMesh;
			bool *mMoved;
			Vector3<float> mStaticPos;
			Vector3<float> mStaticNormal;
	};


	//! Animation keyframe which describes a new position
	struct PositionKey
	{
		float mFrame;
		Vector3<float> mPosition;
	};

	//! Animation keyframe which describes a new scale
	struct ScaleKey
	{
		float mFrame;
		Vector3<float> mScale;
	};

	//! Animation keyframe which describes a new rotation
	struct RotationKey
	{
		float mFrame;
		Quaternion<float> mRotation;
	};

	//! Joints
	struct Joint
	{
		Joint() : mParent(nullptr), mUseAnimationFrom(0), mGlobalSkinningSpace(false),
			mPositionHint(-1), mScaleHint(-1), mRotationHint(-1)
		{
			mAnimatedScale = Vector3<float>::Zero();
			mAnimatedPosition = Vector3<float>::Zero();
			mAnimatedRotation = Quaternion<float>::Zero();
		}

		//! The name of this joint
		eastl::string mName;

		//! Local transform of this joint
		Transform mLocalTransform;

		Joint* mParent;

		//! List of child joints
		eastl::vector<Joint*> mChildren;

		//! List of attached meshes
		eastl::vector<unsigned int> mAttachedMeshes;

		//! Animation keys causing translation change
		eastl::vector<PositionKey> mPositionKeys;

		//! Animation keys causing scale change
		eastl::vector<ScaleKey> mScaleKeys;

		//! Animation keys causing rotation change
		eastl::vector<RotationKey> mRotationKeys;

		//! Skin weights
		eastl::vector<Weight> mWeights;

		//! Unnecessary for loaders, will be overwritten on finalize
		Transform mGlobalTransform;
		Transform mGlobalAnimatedTransform;
		Transform mLocalAnimatedTransform;
		Vector3<float> mAnimatedPosition;
		Vector3<float> mAnimatedScale;
		Quaternion<float> mAnimatedRotation;

		Transform mGlobalInversedTransform; //the x format pre-calculates this

	private:
		//! Internal members used by CSkinnedMesh
		friend class SkinnedMesh;

		Joint *mUseAnimationFrom;
		bool mGlobalSkinningSpace;

		int mPositionHint;
		int mScaleHint;
		int mRotationHint;
	};

	//Interface for the mesh loaders (finalize should lock these functions, and they should have some prefix like loader_

	//these functions will use the needed arrays, set values, etc to help the loaders

	//! exposed for loaders: to add mesh buffers
	virtual eastl::vector<eastl::shared_ptr<SkinMeshBuffer>>& GetMeshBuffers() = 0;

	//! exposed for loaders: joints list
	virtual eastl::vector<Joint*>& GetAllJoints() = 0;

	//! exposed for loaders: joints list
	virtual const eastl::vector<Joint*>& GetAllJoints() const = 0;

	//! loaders should call this after populating the mesh
	virtual void Finalize() = 0;

	//! Adds a new joint to the mesh, access it as last one
	virtual Joint* AddJoint(Joint *parent = 0) = 0;

	//! Adds a new weight to the mesh, access it as last one
	virtual Weight* AddWeight(Joint *joint) = 0;

	//! Adds a new position key to the mesh, access it as last one
	virtual PositionKey* AddPositionKey(Joint *joint) = 0;
	//! Adds a new scale key to the mesh, access it as last one
	virtual ScaleKey* AddScaleKey(Joint *joint) = 0;
	//! Adds a new rotation key to the mesh, access it as last one
	virtual RotationKey* AddRotationKey(Joint *joint) = 0;

	//! Check if the mesh is non-animated
	virtual bool IsStatic() = 0;
};

//! Simple implementation of the IAnimatedMesh interface.
class SkinnedMesh : public BaseSkinnedMesh
{

public:

	//! constructor
	SkinnedMesh();

	//! destructor
	virtual ~SkinnedMesh();

	//! returns the amount of frames. If the amount is 1, it is a static (=non animated) mesh.
	virtual unsigned int GetFrameCount() const;

	//! Gets the default animation speed of the animated mesh.
	/** \return Amount of frames per second. If the amount is 0, it is a static, non animated mesh. */
	virtual float GetAnimationSpeed() const;

	//! Gets the frame count of the animated mesh.
	/** \param fps Frames per second to play the animation with. If the amount is 0, it is not animated.
	The actual speed is set in the scene node the mesh is instantiated in.*/
	virtual void SetAnimationSpeed(float fps);

	//! returns the animated mesh based on a detail level (which is ignored)
	virtual eastl::shared_ptr<BaseMesh> GetMesh(int frame, int detailLevel = 255,
		int startFrameLoop = -1, int endFrameLoop = -1);

	//! Animates this mesh's joints based on frame input
	//! blend: {0-old position, 1-New position}
	virtual void AnimateMesh(float frame, float blend);

	//! Preforms a software skin on this mesh based of joint positions
	virtual void SkinMesh();

	//! returns amount of mesh buffers.
	virtual unsigned int GetMeshBufferCount() const;

	//! returns pointer to a mesh buffer
	virtual eastl::shared_ptr<BaseMeshBuffer> GetMeshBuffer(unsigned int nr) const;

	//! Returns pointer to a mesh buffer which fits a material
	/** \param material: material to search for
	\return Returns the pointer to the mesh buffer or
	NULL if there is no such mesh buffer. */
	virtual eastl::shared_ptr<BaseMeshBuffer> GetMeshBuffer(const Material &material) const;

	//! Returns the type of the animated mesh.
	virtual MeshType GetMeshType() const;

	//! Gets joint count.
	virtual unsigned int GetJointCount() const;

	//! Gets the name of a joint.
	virtual const char* GetJointName(unsigned int number) const;

	//! Gets a joint number from its name
	virtual int GetJointNumber(const char* name) const;

	//! uses animation from another mesh
	virtual bool UseAnimationFrom(const BaseSkinnedMesh *mesh);

	//! Update Normals when Animating
	//! False= Don't (default)
	//! True = Update normals, slower
	virtual void UpdateNormalsWhenAnimating(bool on);

	//! Sets Interpolation Mode
	virtual void SetInterpolationMode(InterpolationMode mode);

	//! Convertes the mesh to contain tangent information
	virtual void ConvertMeshToTangents();

	//! Does the mesh have no animation
	virtual bool IsStatic();

	//! (This feature is not implemented in irrlicht yet)
	virtual bool SetHardwareSkinning(bool on);

	//Interface for the mesh loaders (finalize should lock these functions, and they should have some prefix like loader_
	//these functions will use the needed arrays, set values, etc to help the loaders

	//! exposed for loaders to add mesh buffers
	virtual eastl::vector<eastl::shared_ptr<SkinMeshBuffer>> &GetMeshBuffers();

	//! alternative method for adding joints
	virtual eastl::vector<Joint*> &GetAllJoints();

	//! alternative method for adding joints
	virtual const eastl::vector<Joint*> &GetAllJoints() const;

	//! loaders should call this after populating the mesh
	virtual void Finalize();

	//! Adds a new meshbuffer to the mesh, access it as last one
	virtual void AddMeshBuffer(BaseMeshBuffer* meshBuffer);

	//! Adds a new joint to the mesh, access it as last one
	virtual Joint *AddJoint(Joint *parent = 0);

	//! Adds a new position key to the mesh, access it as last one
	virtual PositionKey *AddPositionKey(Joint *joint);
	//! Adds a new rotation key to the mesh, access it as last one
	virtual RotationKey *AddRotationKey(Joint *joint);
	//! Adds a new scale key to the mesh, access it as last one
	virtual ScaleKey *AddScaleKey(Joint *joint);

	//! Adds a new weight to the mesh, access it as last one
	virtual Weight *AddWeight(Joint *joint);

	virtual void UpdateBoundingBox(void);

	//! Recovers the joints from the mesh
	void RecoverJointsFromMesh(eastl::vector<eastl::shared_ptr<BoneNode>> &jointChildSceneNodes);

	//! Tranfers the joint data to the mesh
	void TransferJointsToMesh(const eastl::vector<eastl::shared_ptr<BoneNode>> &jointChildSceneNodes);

	//! Tranfers the joint hints to the mesh
	void TransferOnlyJointsHintsToMesh(const eastl::vector<eastl::shared_ptr<BoneNode>> &jointChildSceneNodes);

	//! Creates an array of joints from this mesh as children of node
	void AddJoints(eastl::vector<eastl::shared_ptr<BoneNode>> &jointChildSceneNodes, AnimatedMeshNode* node, Scene* scene);

private:
	void CheckForAnimation();

	void NormalizeWeights();

	void BuildAllLocalAnimatedMatrices();

	void BuildAllGlobalAnimatedMatrices(Joint *joint = 0, Joint *parentJoint = 0);

	void GetFrameData(float frame, Joint *Node,
		Vector3<float> &position, int &positionHint,
		Vector3<float> &scale, int &scaleHint,
		Quaternion<float> &rotation, int &rotationHint);

	void CalculateGlobalMatrices(Joint *joint, Joint *parentJoint);

	void SkinJoint(Joint *joint, Joint *parentJoint);

	void CalculateTangents(Vector3<float>& normal,
		Vector3<float>& tangent, Vector3<float>& binormal,
		Vector3<float>& vt1, Vector3<float>& vt2, Vector3<float>& vt3,
		Vector2<float>& tc1, Vector2<float>& tc2, Vector2<float>& tc3);

	eastl::vector<eastl::shared_ptr<SkinMeshBuffer>>* mSkinningBuffers; //Meshbuffer to skin, default is to skin localBuffers

	eastl::vector<eastl::shared_ptr<SkinMeshBuffer>> mLocalBuffers;

	eastl::vector<Joint*> mAllJoints;
	eastl::vector<Joint*> mRootJoints;

	eastl::vector<eastl::vector<bool>> mVerticesMoved;

	//BoundingBox3<float> mBoundingBox;

	float mAnimationFrames;
	float mFramesPerSecond;

	float mLastAnimatedFrame;
	bool mSkinnedLastFrame;

	InterpolationMode mInterpolationMode : 8;

	bool mHasAnimation;
	bool mPreparedForSkinning;
	bool mAnimateNormals;
	bool mHardwareSkinning;
};

#endif

