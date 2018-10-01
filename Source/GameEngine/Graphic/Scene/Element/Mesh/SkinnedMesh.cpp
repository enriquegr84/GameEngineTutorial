// Copyright (C) 2002-2012 Nikolaus Gebhardt
// This file is part of the "Irrlicht Engine".
// For conditions of distribution and use, see copyright notice in irrlicht.h


#include "SkinnedMesh.h"

#include "Mathematic/Function/Functions.h"

#include "Graphic/Scene/Scene.h"
#include "Graphic/Scene/Element/BoneNode.h"
#include "Graphic/Scene/Element/AnimatedMeshNode.h"

//! constructor
SkinnedMesh::SkinnedMesh()
: 
	mSkinningBuffers(0), mAnimationFrames(0.f), mFramesPerSecond(25.f),
	mLastAnimatedFrame(-1), mSkinnedLastFrame(false), mInterpolationMode(IM_LINEAR),
	mHasAnimation(false), mPreparedForSkinning(false),
	mAnimateNormals(true), mHardwareSkinning(false)
{
	#ifdef _DEBUG
	//SetDebugName("SkinnedMesh");
	#endif

	mSkinningBuffers=&mLocalBuffers;
}


//! destructor
SkinnedMesh::~SkinnedMesh()
{
	for (unsigned int i=0; i<mAllJoints.size(); ++i)
		delete mAllJoints[i];

	mLocalBuffers.clear();
}


//! returns the amount of frames in milliseconds.
//! If the amount is 1, it is a static (=non animated) mesh.
unsigned int SkinnedMesh::GetFrameCount() const
{
	return (unsigned int)Function<float>::Floor(mAnimationFrames);
}


//! Gets the default animation speed of the animated mesh.
/** \return Amount of frames per second. If the amount is 0, it is a static, non animated mesh. */
float SkinnedMesh::GetAnimationSpeed() const
{
	return mFramesPerSecond;
}


//! Gets the frame count of the animated mesh.
/** \param fps Frames per second to play the animation with. If the amount is 0, it is not animated.
The actual speed is set in the scene node the mesh is instantiated in.*/
void SkinnedMesh::SetAnimationSpeed(float fps)
{
	mFramesPerSecond=fps;
}


//! returns the animated mesh based on a detail level. 0 is the lowest, 255 the highest detail. Note, that some Meshes will ignore the detail level.
eastl::shared_ptr<BaseMesh> SkinnedMesh::GetMesh(int frame, int detailLevel, int startFrameLoop, int endFrameLoop)
{
	//animate(frame,startFrameLoop, endFrameLoop);
	if (frame==-1)
		return shared_from_this();

	AnimateMesh((float)frame, 1.0f);
	SkinMesh();

	return shared_from_this();
}


//--------------------------------------------------------------------------
//			Keyframe Animation
//--------------------------------------------------------------------------


//! Animates this mesh's joints based on frame input
//! blend: {0-old position, 1-New position}
void SkinnedMesh::AnimateMesh(float frame, float blend)
{
	if (!mHasAnimation || mLastAnimatedFrame==frame)
		return;

	mLastAnimatedFrame=frame;
	mSkinnedLastFrame=false;

	if (blend<=0.f)
		return; //No need to animate

	for (unsigned int i=0; i<mAllJoints.size(); ++i)
	{
		//The joints can be animated here with no input from their
		//parents, but for setAnimationMode extra checks are needed
		//to their parents
		Joint *joint = mAllJoints[i];

		const Vector3<float> oldPosition = joint->mAnimatedPosition;
		const Vector3<float> oldScale = joint->mAnimatedScale;
		const Quaternion<float> oldRotation = joint->mAnimatedRotation;

		Vector3<float> position = oldPosition;
		Vector3<float> scale = oldScale;
		Quaternion<float> rotation = oldRotation;

		GetFrameData(frame, joint,
				position, joint->mPositionHint,
				scale, joint->mScaleHint,
				rotation, joint->mRotationHint);

		if (blend==1.0f)
		{
			//No blending needed
			joint->mAnimatedPosition = position;
			joint->mAnimatedScale = scale;
			joint->mAnimatedRotation = rotation;
		}
		else
		{
			//Blend animation
			joint->mAnimatedPosition = Function<float>::Lerp(oldPosition, position, blend);
			joint->mAnimatedScale = Function<float>::Lerp(oldScale, scale, blend);
			joint->mAnimatedRotation = Slerp(blend, oldRotation, rotation);
		}
	}
	//Note:
	//LocalAnimatedMatrix needs to be built at some point, but this function may be called lots of times for
	//one render (to play two animations at the same time) LocalAnimatedMatrix only needs to be built once.
	//a call to buildAllLocalAnimatedMatrices is needed before skinning the mesh, and before the user gets the joints to move

	//----------------
	// Temp!
	BuildAllLocalAnimatedMatrices();
	//-----------------

	UpdateBoundingBox();
}


void SkinnedMesh::BuildAllLocalAnimatedMatrices()
{
	for (unsigned int i=0; i<mAllJoints.size(); ++i)
	{
		Joint *joint = mAllJoints[i];

		//Could be faster:

		if (joint->mUseAnimationFrom &&
			(joint->mUseAnimationFrom->mPositionKeys.size() ||
			 joint->mUseAnimationFrom->mScaleKeys.size() ||
			 joint->mUseAnimationFrom->mRotationKeys.size() ))
		{
			joint->mGlobalSkinningSpace=false;

			// IRR_TEST_BROKEN_QUATERNION_USE: TODO - switched to getMatrix_transposed instead of getMatrix for downward compatibility. 
			//								   Not tested so far if this was correct or wrong before quaternion fix!
			joint->mAnimatedRotation = Rotation<4, float>(
				Transpose(joint->mLocalAnimatedTransform.GetRotation()));

			// --- joint->mLocalAnimatedMatrix *= joint->mAnimatedRotation.GetMatrix() ---
			Matrix4x4<float> localAnimatedMatrix = joint->mLocalAnimatedTransform.GetRotation();
			Vector3<float> localAnimatedTraslation = joint->mLocalAnimatedTransform.GetTranslation();
			localAnimatedMatrix[0] += joint->mAnimatedPosition[0] * localAnimatedMatrix[3];
			localAnimatedMatrix[1] += joint->mAnimatedPosition[1] * localAnimatedMatrix[3];
			localAnimatedMatrix[2] += joint->mAnimatedPosition[2] * localAnimatedMatrix[3];
			localAnimatedMatrix[4] += joint->mAnimatedPosition[0] * localAnimatedMatrix[7];
			localAnimatedMatrix[5] += joint->mAnimatedPosition[1] * localAnimatedMatrix[7];
			localAnimatedMatrix[6] += joint->mAnimatedPosition[2] * localAnimatedMatrix[7];
			localAnimatedMatrix[8] += joint->mAnimatedPosition[0] * localAnimatedMatrix[11];
			localAnimatedMatrix[9] += joint->mAnimatedPosition[1] * localAnimatedMatrix[11];
			localAnimatedMatrix[10] += joint->mAnimatedPosition[2] * localAnimatedMatrix[11];
			localAnimatedTraslation[0] += joint->mAnimatedPosition[0] * localAnimatedMatrix[15];
			localAnimatedTraslation[1] += joint->mAnimatedPosition[1] * localAnimatedMatrix[15];
			localAnimatedTraslation[2] += joint->mAnimatedPosition[2] * localAnimatedMatrix[15];
			// -----------------------------------

			if (joint->mScaleKeys.size())
			{
				// Matrix4x4 scaleMatrix;
				// scaleMatrix.SetScale(joint->mAnimatedscale);
				// joint->mLocalAnimatedMatrix *= scaleMatrix;

				// -------- joint->mLocalAnimatedMatrix *= scaleMatrix -----------------
				localAnimatedMatrix[0] *= joint->mAnimatedScale[0];
				localAnimatedMatrix[1] *= joint->mAnimatedScale[0];
				localAnimatedMatrix[2] *= joint->mAnimatedScale[0];
				localAnimatedMatrix[3] *= joint->mAnimatedScale[0];
				localAnimatedMatrix[4] *= joint->mAnimatedScale[1];
				localAnimatedMatrix[5] *= joint->mAnimatedScale[1];
				localAnimatedMatrix[6] *= joint->mAnimatedScale[1];
				localAnimatedMatrix[7] *= joint->mAnimatedScale[1];
				localAnimatedMatrix[8] *= joint->mAnimatedScale[2];
				localAnimatedMatrix[9] *= joint->mAnimatedScale[2];
				localAnimatedMatrix[10] *= joint->mAnimatedScale[2];
				localAnimatedMatrix[11] *= joint->mAnimatedScale[2];
				// -----------------------------------
			}
			joint->mLocalAnimatedTransform.SetRotation(localAnimatedMatrix);
			joint->mLocalAnimatedTransform.SetTranslation(localAnimatedTraslation);
		}
		else
		{
			joint->mLocalAnimatedTransform = joint->mLocalTransform;
		}
	}
	mSkinnedLastFrame=false;
}


void SkinnedMesh::BuildAllGlobalAnimatedMatrices(Joint *joint, Joint *parentJoint)
{
	if (!joint)
	{
		for (unsigned int i=0; i<mRootJoints.size(); ++i)
			BuildAllGlobalAnimatedMatrices(mRootJoints[i], 0);
		return;
	}
	else
	{
		// Find global matrix...
		if (!parentJoint || joint->mGlobalSkinningSpace)
			joint->mGlobalAnimatedTransform = joint->mLocalAnimatedTransform;
		else
			joint->mGlobalAnimatedTransform = parentJoint->mGlobalAnimatedTransform * joint->mLocalAnimatedTransform;
	}

	for (unsigned int j=0; j<joint->mChildren.size(); ++j)
		BuildAllGlobalAnimatedMatrices(joint->mChildren[j], joint);
}


void SkinnedMesh::GetFrameData(float frame, Joint *joint,
				Vector3<float> &position, int &positionHint,
				Vector3<float> &scale, int &scaleHint,
				Quaternion<float> &rotation, int &rotationHint)
{
	int foundPositionIndex = -1;
	int foundScaleIndex = -1;
	int foundRotationIndex = -1;

	if (joint->mUseAnimationFrom)
	{
		const eastl::vector<PositionKey> &positionKeys=joint->mUseAnimationFrom->mPositionKeys;
		const eastl::vector<ScaleKey> &scaleKeys=joint->mUseAnimationFrom->mScaleKeys;
		const eastl::vector<RotationKey> &rotationKeys=joint->mUseAnimationFrom->mRotationKeys;

		if (positionKeys.size())
		{
			foundPositionIndex = -1;

			//Test the Hints...
			if (positionHint>=0 && (unsigned int)positionHint < positionKeys.size())
			{
				//check this hint
				if (positionHint > 0 &&
					positionKeys[positionHint].mFrame >= frame &&
					positionKeys[positionHint - 1].mFrame < frame)
				{
					foundPositionIndex = positionHint;
				}
				else if (positionHint+1 < (int)positionKeys.size())
				{
					//check the next index
					if (positionKeys[positionHint+1].mFrame>=frame &&
						positionKeys[positionHint+0].mFrame<frame)
					{
						positionHint++;
						foundPositionIndex=positionHint;
					}
				}
			}

			//The hint test failed, do a full scan...
			if (foundPositionIndex==-1)
			{
				for (unsigned int i=0; i<positionKeys.size(); ++i)
				{
					if (positionKeys[i].mFrame >= frame) //Keys should to be sorted by frame
					{
						foundPositionIndex=i;
						positionHint=i;
						break;
					}
				}
			}

			//Do interpolation...
			if (foundPositionIndex!=-1)
			{
				if (mInterpolationMode==IM_CONSTANT || foundPositionIndex==0)
				{
					position = positionKeys[foundPositionIndex].mPosition;
				}
				else if (mInterpolationMode==IM_LINEAR)
				{
					const PositionKey& keyA = positionKeys[foundPositionIndex];
					const PositionKey& keyB = positionKeys[foundPositionIndex-1];

					const float fd1 = frame - keyA.mFrame;
					const float fd2 = keyB.mFrame - frame;
					position = ((keyB.mPosition-keyA.mPosition)/(fd1+fd2))*fd1 + keyA.mPosition;
				}
			}
		}

		//------------------------------------------------------------

		if (scaleKeys.size())
		{
			foundScaleIndex = -1;

			//Test the Hints...
			if (scaleHint>=0 && (unsigned int)scaleHint < scaleKeys.size())
			{
				//check this hint
				if (scaleHint > 0 &&
					scaleKeys[scaleHint].mFrame >= frame &&
					scaleKeys[scaleHint - 1].mFrame < frame)
				{
					foundScaleIndex = scaleHint;
				}
				else if (scaleHint+1 < (int)scaleKeys.size())
				{
					//check the next index
					if (scaleKeys[scaleHint+1].mFrame>=frame &&
						scaleKeys[scaleHint+0].mFrame<frame)
					{
						scaleHint++;
						foundScaleIndex=scaleHint;
					}
				}
			}


			//The hint test failed, do a full scan...
			if (foundScaleIndex==-1)
			{
				for (unsigned int i=0; i<scaleKeys.size(); ++i)
				{
					if (scaleKeys[i].mFrame >= frame) //Keys should to be sorted by frame
					{
						foundScaleIndex=i;
						scaleHint=i;
						break;
					}
				}
			}

			//Do interpolation...
			if (foundScaleIndex!=-1)
			{
				if (mInterpolationMode==IM_CONSTANT || foundScaleIndex==0)
				{
					scale = scaleKeys[foundScaleIndex].mScale;
				}
				else if (mInterpolationMode==IM_LINEAR)
				{
					const ScaleKey& keyA = scaleKeys[foundScaleIndex];
					const ScaleKey& keyB = scaleKeys[foundScaleIndex-1];

					const float fd1 = frame - keyA.mFrame;
					const float fd2 = keyB.mFrame - frame;
					scale = ((keyB.mScale-keyA.mScale)/(fd1+fd2))*fd1 + keyA.mScale;
				}
			}
		}

		//-------------------------------------------------------------

		if (rotationKeys.size())
		{
			foundRotationIndex = -1;

			//Test the Hints...
			if (rotationHint>=0 && (unsigned int)rotationHint < rotationKeys.size())
			{
				//check this hint
				if (rotationHint > 0 &&
					rotationKeys[rotationHint].mFrame >= frame &&
					rotationKeys[rotationHint - 1].mFrame < frame)
				{
					foundRotationIndex = rotationHint;
				}
				else if (rotationHint+1 < (int)rotationKeys.size())
				{
					//check the next index
					if (rotationKeys[rotationHint+1].mFrame>=frame &&
						rotationKeys[rotationHint+0].mFrame<frame)
					{
						rotationHint++;
						foundRotationIndex=rotationHint;
					}
				}
			}


			//The hint test failed, do a full scan...
			if (foundRotationIndex==-1)
			{
				for (unsigned int i=0; i<rotationKeys.size(); ++i)
				{
					if (rotationKeys[i].mFrame >= frame) //Keys should be sorted by frame
					{
						foundRotationIndex=i;
						rotationHint=i;
						break;
					}
				}
			}

			//Do interpolation...
			if (foundRotationIndex!=-1)
			{
				if (mInterpolationMode==IM_CONSTANT || foundRotationIndex==0)
				{
					rotation = rotationKeys[foundRotationIndex].mRotation;
				}
				else if (mInterpolationMode==IM_LINEAR)
				{
					const RotationKey& keyA = rotationKeys[foundRotationIndex];
					const RotationKey& keyB = rotationKeys[foundRotationIndex-1];

					const float fd1 = frame - keyA.mFrame;
					const float fd2 = keyB.mFrame - frame;
					const float t = fd1/(fd1+fd2);

					/*
					float t = 0;
					if (keyA.mFrame!=keyB.mFrame)
						t = (frame-keyA.mFrame) / (keyB.mFrame - keyA.mFrame);
					*/

					rotation = Slerp(t, keyA.mRotation, keyB.mRotation);
				}
			}
		}
	}
}

//--------------------------------------------------------------------------
//				Software Skinning
//--------------------------------------------------------------------------

//! Preforms a software skin on this mesh based of joint positions
void SkinnedMesh::SkinMesh()
{
	if (!mHasAnimation || mSkinnedLastFrame)
		return;

	//----------------
	// This is marked as "Temp!".  A shiny dubloon to whomever can tell me why.
	BuildAllGlobalAnimatedMatrices();
	//-----------------

	mSkinnedLastFrame=true;
	if (!mHardwareSkinning)
	{
		//Software skin....
		unsigned int i;

		//rigid animation
		for (i=0; i<mAllJoints.size(); ++i)
		{
			for (unsigned int j=0; j<mAllJoints[i]->mAttachedMeshes.size(); ++j)
			{
				eastl::shared_ptr<SkinMeshBuffer> buffer =
					(*mSkinningBuffers)[ mAllJoints[i]->mAttachedMeshes[j] ];
				buffer->GetTransform().SetRotation(
					mAllJoints[i]->mGlobalAnimatedTransform.GetRotation());
				buffer->GetTransform().SetTranslation(
					mAllJoints[i]->mGlobalAnimatedTransform.GetTranslation());
			}
		}

		//clear skinning helper array
		for (i=0; i<mVerticesMoved.size(); ++i)
			for (unsigned int j=0; j<mVerticesMoved[i].size(); ++j)
				mVerticesMoved[i][j]=false;

		//skin starting with the root joints
		for (i=0; i<mRootJoints.size(); ++i)
			SkinJoint(mRootJoints[i], 0);
	}
	UpdateBoundingBox();
}


void SkinnedMesh::SkinJoint(Joint *joint, Joint *parentJoint)
{
	if (joint->mWeights.size())
	{
		//Find this joints pull on vertices...
		Matrix4x4<float> jointVertexPull;
		jointVertexPull = 
			joint->mGlobalAnimatedTransform.GetRotation() * 
			joint->mGlobalInversedTransform.GetRotation();

		Vector4<float> thisVertexMove = Vector4<float>::Zero();
		Vector4<float> thisNormalMove = Vector4<float>::Zero();

		eastl::vector<eastl::shared_ptr<SkinMeshBuffer>> &buffersUsed=*mSkinningBuffers;

		//Skin Vertices Positions and Normals...
		for (unsigned int i=0; i<joint->mWeights.size(); ++i)
		{
			Weight& weight = joint->mWeights[i];

			// Pull this vertex...
#if defined(GE_USE_MAT_VEC)
			thisVertexMove = jointVertexPull * HLift(weight.mStaticPos, 0.f);
#else
			thisVertexMove = HLift(weight.mStaticPos, 0.f) * jointVertexPull;
#endif
			thisVertexMove += joint->mGlobalAnimatedTransform.GetTranslationW1();

			if (mAnimateNormals)
			{
#if defined(GE_USE_MAT_VEC)
				thisNormalMove = jointVertexPull * HLift(weight.mStaticNormal, 0.f);
#else
				thisNormalMove = HLift(weight.mStaticNormal, 0.f) * jointVertexPull;
#endif
			}

			if (! (*(weight.mMoved)) )
			{
				*(weight.mMoved) = true;

				buffersUsed[weight.mBufferId]->Position(weight.mVertexId) = 
					HProject(thisVertexMove) * weight.mStrength;

				if (mAnimateNormals)
				{
					buffersUsed[weight.mBufferId]->Normal(weight.mVertexId) = 
						HProject(thisNormalMove) * weight.mStrength;
				}
				//*(weight.mPos) = thisVertexMove * weight.mStrength;
			}
			else
			{
				buffersUsed[weight.mBufferId]->Position(weight.mVertexId) += 
					HProject(thisVertexMove) * weight.mStrength;

				if (mAnimateNormals)
				{
					buffersUsed[weight.mBufferId]->Normal(weight.mVertexId) += 
						HProject(thisNormalMove) * weight.mStrength;
				}
				//*(weight.mPos) += thisVertexMove * weight.mStrength;
			}

			//buffersUsed[weight.mBufferId]->BoundingBoxNeedsRecalculated();
		}
	}

	//Skin all children
	for (unsigned int j=0; j<joint->mChildren.size(); ++j)
		SkinJoint(joint->mChildren[j], joint);
}


MeshType SkinnedMesh::GetMeshType() const
{
	return MT_SKINNED;
}


//! Gets joint count.
unsigned int SkinnedMesh::GetJointCount() const
{
	return mAllJoints.size();
}


//! Gets the name of a joint.
const char* SkinnedMesh::GetJointName(unsigned int number) const
{
	if (number >= mAllJoints.size())
		return 0;
	return mAllJoints[number]->mName.c_str();
}


//! Gets a joint number from its name
int SkinnedMesh::GetJointNumber(const char* name) const
{
	for (unsigned int i=0; i<mAllJoints.size(); ++i)
	{
		if (mAllJoints[i]->mName == name)
			return i;
	}

	return -1;
}


//! returns amount of mesh buffers.
unsigned int SkinnedMesh::GetMeshBufferCount() const
{
	return mLocalBuffers.size();
}


//! returns pointer to a mesh buffer
eastl::shared_ptr<BaseMeshBuffer> SkinnedMesh::GetMeshBuffer(unsigned int nr) const
{
	if (nr < mLocalBuffers.size())
		return mLocalBuffers[nr];
	else
		return 0;
}


//! Returns pointer to a mesh buffer which fits a material
eastl::shared_ptr<BaseMeshBuffer> SkinnedMesh::GetMeshBuffer(const Material &material) const
{
	for (unsigned int i=0; i<mLocalBuffers.size(); ++i)
	{
		if (&material == mLocalBuffers[i]->GetMaterial().get())
			return mLocalBuffers[i];
	}
	return 0;
}

//! uses animation from another mesh
bool SkinnedMesh::UseAnimationFrom(const BaseSkinnedMesh *mesh)
{
	bool unmatched=false;

	for(unsigned int i=0;i<mAllJoints.size();++i)
	{
		Joint *joint=mAllJoints[i];
		joint->mUseAnimationFrom=0;

		if (!joint->mName.empty())
		{
			for (unsigned int j = 0; j<mesh->GetAllJoints().size(); ++j)
			{
				Joint *otherJoint = mesh->GetAllJoints()[j];
				if (joint->mName == otherJoint->mName)
				{
					joint->mUseAnimationFrom = otherJoint;
				}
			}
			if (!joint->mUseAnimationFrom)
				unmatched = true;
		}
		else unmatched = true;
	}

	CheckForAnimation();

	return !unmatched;
}


//!Update Normals when Animating
//!False= Don't animate them, faster
//!True= Update normals (default)
void SkinnedMesh::UpdateNormalsWhenAnimating(bool on)
{
	mAnimateNormals = on;
}


//!Sets Interpolation Mode
void SkinnedMesh::SetInterpolationMode(InterpolationMode mode)
{
	mInterpolationMode = mode;
}


eastl::vector<eastl::shared_ptr<SkinMeshBuffer>> &SkinnedMesh::GetMeshBuffers()
{
	return mLocalBuffers;
}


eastl::vector<SkinnedMesh::Joint*> &SkinnedMesh::GetAllJoints()
{
	return mAllJoints;
}


const eastl::vector<SkinnedMesh::Joint*> &SkinnedMesh::GetAllJoints() const
{
	return mAllJoints;
}


//! (This feature is not implementated in irrlicht yet)
bool SkinnedMesh::SetHardwareSkinning(bool on)
{
	if (mHardwareSkinning!=on)
	{
		if (on)
		{
			//set mesh to static pose...
			for (unsigned int i=0; i<mAllJoints.size(); ++i)
			{
				Joint *joint=mAllJoints[i];
				for (unsigned int j=0; j<joint->mWeights.size(); ++j)
				{
					const unsigned int bufferId=joint->mWeights[j].mBufferId;
					const unsigned int vertexId=joint->mWeights[j].mVertexId;
					mLocalBuffers[bufferId]->Position(vertexId) = joint->mWeights[j].mStaticPos;
					mLocalBuffers[bufferId]->Normal(vertexId) = joint->mWeights[j].mStaticNormal;
					//mLocalBuffers[bufferId]->BoundingBoxNeedsRecalculated();
				}
			}
		}

		mHardwareSkinning=on;
	}
	return mHardwareSkinning;
}


void SkinnedMesh::CalculateGlobalMatrices(Joint *joint,Joint *parentJoint)
{
	if (!joint && parentJoint) // bit of protection from endless loops
		return;

	//Go through the root bones
	if (!joint)
	{
		for (unsigned int i=0; i<mRootJoints.size(); ++i)
			CalculateGlobalMatrices(mRootJoints[i],0);
		return;
	}

	if (!parentJoint)
		joint->mGlobalTransform = joint->mLocalTransform;
	else
		joint->mGlobalTransform = parentJoint->mGlobalTransform * joint->mLocalTransform;

	joint->mLocalAnimatedTransform=joint->mLocalTransform;
	joint->mGlobalAnimatedTransform=joint->mGlobalTransform;

	if (joint->mGlobalInversedTransform.GetMatrix() == Matrix4x4<float>::Identity())//might be pre calculated
	{
		joint->mGlobalInversedTransform = joint->mGlobalTransform;
		joint->mGlobalInversedTransform.SetRotation(Inverse(joint->mGlobalInversedTransform.GetRotation())); // slow
	}

	for (unsigned int j=0; j<joint->mChildren.size(); ++j)
		CalculateGlobalMatrices(joint->mChildren[j],joint);
	mSkinnedLastFrame=false;
}


void SkinnedMesh::CheckForAnimation()
{
	unsigned int i,j;
	//Check for animation...
	mHasAnimation = false;
	for(i=0;i<mAllJoints.size();++i)
	{
		if (mAllJoints[i]->mUseAnimationFrom)
		{
			if (mAllJoints[i]->mUseAnimationFrom->mPositionKeys.size() ||
				mAllJoints[i]->mUseAnimationFrom->mScaleKeys.size() ||
				mAllJoints[i]->mUseAnimationFrom->mRotationKeys.size() )
			{
				mHasAnimation = true;
			}
		}
	}

	//meshes with weights, are still counted as animated for ragdolls, etc
	if (!mHasAnimation)
	{
		for(i=0;i<mAllJoints.size();++i)
		{
			if (mAllJoints[i]->mWeights.size())
				mHasAnimation = true;
		}
	}

	if (mHasAnimation)
	{
		//--- Find the length of the animation ---
		mAnimationFrames=0;
		for(i=0;i<mAllJoints.size();++i)
		{
			if (mAllJoints[i]->mUseAnimationFrom)
			{
				if (mAllJoints[i]->mUseAnimationFrom->mPositionKeys.size())
					if (mAllJoints[i]->mUseAnimationFrom->mPositionKeys.back().mFrame > mAnimationFrames)
						mAnimationFrames=mAllJoints[i]->mUseAnimationFrom->mPositionKeys.back().mFrame;

				if (mAllJoints[i]->mUseAnimationFrom->mScaleKeys.size())
					if (mAllJoints[i]->mUseAnimationFrom->mScaleKeys.back().mFrame > mAnimationFrames)
						mAnimationFrames=mAllJoints[i]->mUseAnimationFrom->mScaleKeys.back().mFrame;

				if (mAllJoints[i]->mUseAnimationFrom->mRotationKeys.size())
					if (mAllJoints[i]->mUseAnimationFrom->mRotationKeys.back().mFrame > mAnimationFrames)
						mAnimationFrames=mAllJoints[i]->mUseAnimationFrom->mRotationKeys.back().mFrame;
			}
		}
	}

	if (mHasAnimation && !mPreparedForSkinning)
	{
		mPreparedForSkinning=true;

		//check for bugs:
		for(i=0; i < mAllJoints.size(); ++i)
		{
			Joint *joint = mAllJoints[i];
			for (j=0; j<joint->mWeights.size(); ++j)
			{
				const unsigned int bufferId=joint->mWeights[j].mBufferId;
				const unsigned int vertexId=joint->mWeights[j].mVertexId;

				//check for invalid ids
				if (bufferId>=mLocalBuffers.size())
				{
					LogWarning("Skinned Mesh: Weight buffer id too large");
					joint->mWeights[j].mBufferId = joint->mWeights[j].mVertexId = 0;
				}
				else if (vertexId>=mLocalBuffers[bufferId]->GetVertice()->GetNumElements())
				{
					LogWarning("Skinned Mesh: Weight vertex id too large");
					joint->mWeights[j].mBufferId = joint->mWeights[j].mVertexId = 0;
				}
			}
		}

		//An array used in skinning

		for (i=0; i<mVerticesMoved.size(); ++i)
			for (j=0; j<mVerticesMoved[i].size(); ++j)
				mVerticesMoved[i][j] = false;

		// For skinning: cache weight values for speed

		for (i=0; i<mAllJoints.size(); ++i)
		{
			Joint *joint = mAllJoints[i];
			for (j=0; j<joint->mWeights.size(); ++j)
			{
				const unsigned int bufferId=joint->mWeights[j].mBufferId;
				const unsigned int vertexId=joint->mWeights[j].mVertexId;

				joint->mWeights[j].mMoved = &mVerticesMoved[bufferId] [vertexId];
				joint->mWeights[j].mStaticPos = mLocalBuffers[bufferId]->Position(vertexId);
				joint->mWeights[j].mStaticNormal = mLocalBuffers[bufferId]->Normal(vertexId);

				//joint->mWeights[j].mPos=&Buffers[bufferId]->Position(vertexId);
			}
		}

		// normalize weights
		NormalizeWeights();
	}
	mSkinnedLastFrame=false;
}


//! called by loader after populating with mesh and bone data
void SkinnedMesh::Finalize()
{
	unsigned int i;

	// Make sure we recalc the next frame
	mLastAnimatedFrame=-1;
	mSkinnedLastFrame=false;

	//calculate bounding box
	/*
	for (i=0; i<mLocalBuffers.size(); ++i)
	{
		mLocalBuffers[i]->RecalculateBoundingBox();
	}
	*/
	if (mAllJoints.size() || mRootJoints.size())
	{
		// populate AllJoints or RootJoints, depending on which is empty
		if (!mRootJoints.size())
		{
			for(unsigned int jointIdx=0; jointIdx < mAllJoints.size(); ++jointIdx)
				if (mAllJoints[jointIdx]->mParent == nullptr)
					mRootJoints.push_back(mAllJoints[jointIdx]);
		}
		else mAllJoints=mRootJoints;
	}

	for(i=0; i < mAllJoints.size(); ++i)
		mAllJoints[i]->mUseAnimationFrom=mAllJoints[i];

	//Set array sizes...

	for (i=0; i<mLocalBuffers.size(); ++i)
	{
		mVerticesMoved.push_back(eastl::vector<bool>());
		mVerticesMoved[i].resize(mLocalBuffers[i]->GetVertice()->GetNumElements());
	}

	//Todo: optimise keys here...

	CheckForAnimation();

	if (mHasAnimation)
	{
		//--- optimize and check keyframes ---
		for(i=0;i<mAllJoints.size();++i)
		{
			eastl::vector<PositionKey> &positionKeys = mAllJoints[i]->mPositionKeys;
			eastl::vector<ScaleKey> &scaleKeys = mAllJoints[i]->mScaleKeys;
			eastl::vector<RotationKey> &rotationKeys = mAllJoints[i]->mRotationKeys;

			if (positionKeys.size()>2)
			{
				for(unsigned int j=0;j<positionKeys.size()-2;++j)
				{
					if (positionKeys[j].mPosition == positionKeys[j+1].mPosition &&
						positionKeys[j+1].mPosition == positionKeys[j+2].mPosition)
					{
						positionKeys.erase(positionKeys.begin()+j+1); //the middle key is unneeded
						--j;
					}
				}
			}

			if (positionKeys.size()>1)
			{
				for(unsigned int j=0;j<positionKeys.size()-1;++j)
				{
					if (positionKeys[j].mFrame >= positionKeys[j+1].mFrame) //bad frame, unneed and may cause problems
					{
						positionKeys.erase(positionKeys.begin()+j+1);
						--j;
					}
				}
			}

			if (scaleKeys.size()>2)
			{
				for(unsigned int j=0;j<scaleKeys.size()-2;++j)
				{
					if (scaleKeys[j].mScale == scaleKeys[j+1].mScale && 
						scaleKeys[j+1].mScale == scaleKeys[j+2].mScale)
					{
						scaleKeys.erase(scaleKeys.begin()+j+1); //the middle key is unneeded
						--j;
					}
				}
			}

			if (scaleKeys.size()>1)
			{
				for(unsigned int j=0;j<scaleKeys.size()-1;++j)
				{
					if (scaleKeys[j].mFrame >= scaleKeys[j+1].mFrame) //bad frame, unneed and may cause problems
					{
						scaleKeys.erase(scaleKeys.begin()+j+1);
						--j;
					}
				}
			}

			if (rotationKeys.size()>2)
			{
				for(unsigned int j=0;j<rotationKeys.size()-2;++j)
				{
					if (rotationKeys[j].mRotation == rotationKeys[j+1].mRotation &&
						rotationKeys[j+1].mRotation == rotationKeys[j+2].mRotation)
					{
						rotationKeys.erase(rotationKeys.begin()+j+1); //the middle key is unneeded
						--j;
					}
				}
			}

			if (rotationKeys.size()>1)
			{
				for(unsigned int j=0;j<rotationKeys.size()-1;++j)
				{
					if (rotationKeys[j].mFrame >= rotationKeys[j+1].mFrame) //bad frame, unneed and may cause problems
					{
						rotationKeys.erase(rotationKeys.begin()+j+1);
						--j;
					}
				}
			}


			//Fill empty keyframe areas
			if (positionKeys.size())
			{
				PositionKey *key;
				key =&positionKeys[0];//getFirst
				if (key->mFrame!=0)
				{
					positionKeys.insert(0, *key);
					key=&positionKeys[0];//getFirst
					key->mFrame=0;
				}

				key =&positionKeys.back();
				if (key->mFrame!=mAnimationFrames)
				{
					positionKeys.push_back(*key);
					key =&positionKeys.back();
					key->mFrame=mAnimationFrames;
				}
			}

			if (scaleKeys.size())
			{
				ScaleKey *key;
				key =&scaleKeys[0];//getFirst
				if (key->mFrame!=0)
				{
					scaleKeys.insert(0, *key);
					key =&scaleKeys[0];//getFirst
					key->mFrame=0;
				}

				key =&scaleKeys.back();
				if (key->mFrame!=mAnimationFrames)
				{
					scaleKeys.push_back(*key);
					key =&scaleKeys.back();
					key->mFrame=mAnimationFrames;
				}
			}

			if (rotationKeys.size())
			{
				RotationKey *key;
				key =&rotationKeys[0];//getFirst
				if (key->mFrame!=0)
				{
					rotationKeys.insert(0, *key);
					key =&rotationKeys.front();//getFirst
					key->mFrame=0;
				}

				key =&rotationKeys.back();
				if (key->mFrame!=mAnimationFrames)
				{
					rotationKeys.push_back(*key);
					key =&rotationKeys.back();
					key->mFrame=mAnimationFrames;
				}
			}
		}
	}

	//Needed for animation and skinning...

	CalculateGlobalMatrices(0,0);

	//AnimateMesh(0, 1);
	//BuildAllLocalAnimatedMatrices();
	//BuildAllGlobalAnimatedMatrices();

	//rigid animation for non animated meshes
	for (i=0; i<mAllJoints.size(); ++i)
	{
		for (unsigned int j=0; j<mAllJoints[i]->mAttachedMeshes.size(); ++j)
		{
			eastl::shared_ptr<SkinMeshBuffer> buffer=
				(*mSkinningBuffers)[ mAllJoints[i]->mAttachedMeshes[j] ];
			buffer->GetTransform().SetRotation(
				mAllJoints[i]->mGlobalAnimatedTransform.GetRotation());
			buffer->GetTransform().SetTranslation(
				mAllJoints[i]->mGlobalAnimatedTransform.GetTranslation());
		}
	}
	/*
	//calculate bounding box
	if (!mLocalBuffers.empty())
	{
		BoundingBox3<float> bb(mLocalBuffers[0]->mBoundingBox);
		mLocalBuffers[0]->mTransformation.transformBoxEx(bb);
		mBoundingBox.reset(bb);

		for (unsigned int j=1; j<mLocalBuffers.size(); ++j)
		{
			bb = mLocalBuffers[j]->mBoundingBox;
			mLocalBuffers[j]->mTransformation.transformBoxEx(bb);

			mBoundingBox.addInternalBox(bb);
		}
	}
	else mBoundingBox.reset(0, 0, 0);
	*/
}


void SkinnedMesh::UpdateBoundingBox(void)
{
	if(!mSkinningBuffers)
		return;
	/*
	eastl::array<SkinMeshBuffer*> & buffer = *mSkinningBuffers;
	mBoundingBox.reset(0,0,0);

	if (!buffer.empty())
	{
		for (unsigned int j=0; j<buffer.size(); ++j)
		{
			buffer[j]->RecalculateBoundingBox();
			BoundingBox<float> bb = buffer[j]->mBoundingBox;
			buffer[j]->mTransformation.transformBoxEx(bb);

			mBoundingBox.addInternalBox(bb);
		}
	}
	*/
}


void SkinnedMesh::AddMeshBuffer(BaseMeshBuffer* meshBuffer)
{
	mLocalBuffers.push_back(eastl::shared_ptr<SkinMeshBuffer>((SkinMeshBuffer*)meshBuffer));
}


SkinnedMesh::Joint *SkinnedMesh::AddJoint(Joint *parent)
{
	Joint *joint = new Joint();

	mAllJoints.push_back(joint);
	if (!parent)
	{
		//Add root joints to array in finalize()
	}
	else
	{
		//Set parent (Be careful of the mesh loader also setting the parent)
		parent->mChildren.push_back(joint);
	}

	return joint;
}


SkinnedMesh::PositionKey* SkinnedMesh::AddPositionKey(Joint *joint)
{
	if (!joint)
		return 0;

	joint->mPositionKeys.push_back(PositionKey());
	return &joint->mPositionKeys.back();
}


SkinnedMesh::ScaleKey* SkinnedMesh::AddScaleKey(Joint *joint)
{
	if (!joint)
		return 0;

	joint->mScaleKeys.push_back(ScaleKey());
	return &joint->mScaleKeys.back();
}


SkinnedMesh::RotationKey* SkinnedMesh::AddRotationKey(Joint *joint)
{
	if (!joint)
		return 0;

	joint->mRotationKeys.push_back(RotationKey());
	return &joint->mRotationKeys.back();
}


SkinnedMesh::Weight* SkinnedMesh::AddWeight(Joint *joint)
{
	if (!joint)
		return 0;

	joint->mWeights.push_back(Weight());
	return &joint->mWeights.back();
}


bool SkinnedMesh::IsStatic()
{
	return !mHasAnimation;
}


void SkinnedMesh::NormalizeWeights()
{
	// note: unsure if weights ids are going to be used.

	// Normalise the weights on bones....

	unsigned int i,j;
	eastl::vector<eastl::vector<float>> verticesTotalWeight;

	verticesTotalWeight.resize(mLocalBuffers.size());
	for (i=0; i<mLocalBuffers.size(); ++i)
	{
		verticesTotalWeight.push_back(eastl::vector<float>());
		verticesTotalWeight[i].resize(mLocalBuffers[i]->GetVertice()->GetNumElements());
	}

	for (i=0; i<verticesTotalWeight.size(); ++i)
		for (j=0; j<verticesTotalWeight[i].size(); ++j)
			verticesTotalWeight[i][j] = 0;

	for (i=0; i<mAllJoints.size(); ++i)
	{
		Joint *joint=mAllJoints[i];
		for (j=0; j<joint->mWeights.size(); ++j)
		{
			if (joint->mWeights[j].mStrength<=0)//Check for invalid weights
			{
				joint->mWeights.erase(joint->mWeights.begin()+j);
				--j;
			}
			else
			{
				verticesTotalWeight[joint->mWeights[j].mBufferId][joint->mWeights[j].mVertexId] += joint->mWeights[j].mStrength;
			}
		}
	}

	for (i=0; i<mAllJoints.size(); ++i)
	{
		Joint *joint=mAllJoints[i];
		for (j=0; j< joint->mWeights.size(); ++j)
		{
			const float total = verticesTotalWeight[joint->mWeights[j].mBufferId][joint->mWeights[j].mVertexId];
			if (total != 0 && total != 1)
				joint->mWeights[j].mStrength /= total;
		}
	}
}


void SkinnedMesh::RecoverJointsFromMesh(eastl::vector<eastl::shared_ptr<BoneNode>> &jointChildSceneNodes)
{
	for (unsigned int i=0; i<mAllJoints.size(); ++i)
	{
		eastl::shared_ptr<BoneNode> node=jointChildSceneNodes[i];
		Joint *joint=mAllJoints[i];
		node->GetRelativeTransform().SetTranslation(joint->mLocalAnimatedTransform.GetTranslation());
		node->GetRelativeTransform().SetRotation(joint->mLocalAnimatedTransform.GetRotation());
		node->GetRelativeTransform().SetScale(joint->mLocalAnimatedTransform.GetScale());

		node->mPositionHint=joint->mPositionHint;
		node->mScaleHint=joint->mScaleHint;
		node->mRotationHint=joint->mRotationHint;

		node->UpdateAbsoluteTransform();
	}
}


void SkinnedMesh::TransferJointsToMesh(const eastl::vector<eastl::shared_ptr<BoneNode>> &jointChildSceneNodes)
{
	for (unsigned int i=0; i<mAllJoints.size(); ++i)
	{
		eastl::shared_ptr<BoneNode> const node=jointChildSceneNodes[i];
		Joint *joint=mAllJoints[i];

		joint->mLocalAnimatedTransform.SetRotation(node->GetRelativeTransform().GetRotation());
		joint->mLocalAnimatedTransform.SetTranslation(node->GetRelativeTransform().GetTranslation());
		joint->mLocalAnimatedTransform.SetScale(node->GetRelativeTransform().GetScale());

		joint->mPositionHint=node->mPositionHint;
		joint->mScaleHint=node->mScaleHint;
		joint->mRotationHint=node->mRotationHint;

		joint->mGlobalSkinningSpace=(node->GetSkinningSpace()==BSS_GLOBAL);
	}
	// Make sure we recalc the next frame
	mLastAnimatedFrame=-1;
	mSkinnedLastFrame=false;
}


void SkinnedMesh::TransferOnlyJointsHintsToMesh(const eastl::vector<eastl::shared_ptr<BoneNode>> &jointChildSceneNodes)
{
	for (unsigned int i=0; i<mAllJoints.size(); ++i)
	{
		const eastl::shared_ptr<BoneNode> node=jointChildSceneNodes[i];
		Joint *joint=mAllJoints[i];

		joint->mPositionHint=node->mPositionHint;
		joint->mScaleHint=node->mScaleHint;
		joint->mRotationHint=node->mRotationHint;
	}
	mSkinnedLastFrame=false;
}


void SkinnedMesh::AddJoints(eastl::vector<eastl::shared_ptr<BoneNode>> &jointChildSceneNodes, AnimatedMeshNode* node, Scene* scene)
{
	//Create new joints
	for (unsigned int i=0; i<mAllJoints.size(); ++i)
	{
		jointChildSceneNodes.push_back(eastl::make_shared<BoneNode>(
			INVALID_ACTOR_ID, &scene->GetPVWUpdater(), WeakBaseRenderComponentPtr(), i, mAllJoints[i]->mName.c_str()));
	}

	//Match up parents
	for (unsigned int i=0; i<jointChildSceneNodes.size(); ++i)
	{
		const Joint* const joint=mAllJoints[i]; //should be fine

		int parentID=-1;

		for (unsigned int j=0;(parentID==-1)&&(j<mAllJoints.size());++j)
		{
			if (i!=j)
			{
				const Joint* const parentTest=mAllJoints[j];
				for (unsigned int n=0; n<parentTest->mChildren.size(); ++n)
				{
					if (parentTest->mChildren[n]==joint)
					{
						parentID=j;
						break;
					}
				}
			}
		}

		eastl::shared_ptr<BoneNode> bone=jointChildSceneNodes[i];
		if (parentID!=-1)
			bone->SetParent(jointChildSceneNodes[parentID].get());
		else
			bone->SetParent(node);
	}
	mSkinnedLastFrame=false;
}


void SkinnedMesh::ConvertMeshToTangents()
{
	// now calculate tangents
	for (unsigned int b=0; b < mLocalBuffers.size(); ++b)
	{
		if (mLocalBuffers[b])
		{
			mLocalBuffers[b]->ConvertToTangents();

			const int idxCnt = mLocalBuffers[b]->GetIndice()->GetNumElements();
			unsigned int* idx = mLocalBuffers[b]->GetIndice()->Get<unsigned int>();
			for (int i=0; i<idxCnt; i+=3)
			{
				CalculateTangents(
					mLocalBuffers[b]->Normal(idx[i+0]),
					mLocalBuffers[b]->Tangent(idx[i+0]),
					mLocalBuffers[b]->Bitangent(idx[i+0]),
					mLocalBuffers[b]->Position(idx[i+0]),
					mLocalBuffers[b]->Position(idx[i+1]),
					mLocalBuffers[b]->Position(idx[i+2]),
					mLocalBuffers[b]->TCoord(idx[i+0], 0),
					mLocalBuffers[b]->TCoord(idx[i+1], 0),
					mLocalBuffers[b]->TCoord(idx[i+2], 0));

				CalculateTangents(
					mLocalBuffers[b]->Normal(idx[i+1]),
					mLocalBuffers[b]->Tangent(idx[i+1]),
					mLocalBuffers[b]->Bitangent(idx[i+1]),
					mLocalBuffers[b]->Position(idx[i+1]),
					mLocalBuffers[b]->Position(idx[i+2]),
					mLocalBuffers[b]->Position(idx[i+0]),
					mLocalBuffers[b]->TCoord(idx[i+1], 0),
					mLocalBuffers[b]->TCoord(idx[i+2], 0),
					mLocalBuffers[b]->TCoord(idx[i+0], 0));

				CalculateTangents(
					mLocalBuffers[b]->Normal(idx[i+2]),
					mLocalBuffers[b]->Tangent(idx[i+2]),
					mLocalBuffers[b]->Bitangent(idx[i+2]),
					mLocalBuffers[b]->Position(idx[i+2]),
					mLocalBuffers[b]->Position(idx[i+0]),
					mLocalBuffers[b]->Position(idx[i+1]),
					mLocalBuffers[b]->TCoord(idx[i+2], 0),
					mLocalBuffers[b]->TCoord(idx[i+0], 0),
					mLocalBuffers[b]->TCoord(idx[i+1], 0));
			}
		}
	}
}


void SkinnedMesh::CalculateTangents(
	Vector3<float>& normal, Vector3<float>& tangent, Vector3<float>& binormal,
	Vector3<float>& vt1, Vector3<float>& vt2, Vector3<float>& vt3, // vertices
	Vector2<float>& tc1, Vector2<float>& tc2, Vector2<float>& tc3) // texture coords
{
	Vector3<float> v1 = vt1 - vt2;
	Vector3<float> v2 = vt3 - vt1;
	normal = Cross(v2, v1);
	Normalize(normal);

	// binormal

	float deltaX1 = tc1[0] - tc2[0];
	float deltaX2 = tc3[0] - tc1[0];
	binormal = (v1 * deltaX2) - (v2 * deltaX1);
	Normalize(binormal);

	// tangent

	float deltaY1 = tc1[1] - tc2[1];
	float deltaY2 = tc3[1] - tc1[1];
	tangent = (v1 * deltaY2) - (v2 * deltaY1);
	Normalize(tangent);

	// adjust

	Vector3<float> txb = Cross(tangent, binormal);
	if (Dot(txb, normal) < 0.0f)
	{
		tangent *= -1.0f;
		binormal *= -1.0f;
	}
}