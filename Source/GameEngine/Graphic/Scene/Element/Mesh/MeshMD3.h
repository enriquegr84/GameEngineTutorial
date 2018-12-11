// Copyright (C) 2002-2012 Nikolaus Gebhardt / Thomas Alten
// This file is part of the "Irrlicht Engine".
// For conditions of distribution and use, see copyright notice in irrlicht.h

#ifndef MESHMD3_H
#define MESHMD3_H

#include "Graphic/Scene/Element/Mesh/Mesh.h"

#include "Mathematic/Algebra/Rotation.h"

enum MD3Model
{
	MD3_HEAD = 0,
	MD3_UPPER,
	MD3_LOWER,
	MD3_WEAPON,
	MD3_NUMMODELS
};

// This enumeration stores all the animations in order from the config file (.cfg).
enum MD3AnimationType
{
	// If one model is set to one of the BOTH_* animations, the other one should be too,
	// otherwise it looks really bad and confusing.

	BOTH_DEATH1 = 0,		// The first twirling death animation
	BOTH_DEAD1,				// The end of the first twirling death animation
	BOTH_DEATH2,			// The second twirling death animation
	BOTH_DEAD2,				// The end of the second twirling death animation
	BOTH_DEATH3,			// The back flip death animation
	BOTH_DEAD3,				// The end of the back flip death animation

	// The next block is the animations that the upper body performs

	TORSO_GESTURE,			// The torso's gesturing animation
	
	TORSO_ATTACK,			// The torso's attack1 animation
	TORSO_ATTACK2,			// The torso's attack2 animation

	TORSO_DROP,				// The torso's weapon drop animation
	TORSO_RAISE,			// The torso's weapon pickup animation

	TORSO_STAND,			// The torso's idle stand animation
	TORSO_STAND2,			// The torso's idle stand2 animation

	// The final block is the animations that the legs perform

	LEGS_WALKCR,			// The legs's crouching walk animation
	LEGS_WALK,				// The legs's walk animation
	LEGS_RUN,				// The legs's run animation
	LEGS_BACK,				// The legs's running backwards animation
	LEGS_SWIM,				// The legs's swimming animation
	
	LEGS_JUMP,				// The legs's jumping animation
	LEGS_LAND,				// The legs's landing animation

	LEGS_JUMPB,				// The legs's jumping back animation
	LEGS_LANDB,				// The legs's landing back animation

	LEGS_IDLE,				// The legs's idle stand animation
	LEGS_IDLECR,			// The legs's idle crouching animation

	LEGS_TURN,				// The legs's turn animation

	//! Not an animation, but amount of animation types.
	ANIMATION_COUNT
};

struct AnimationInfo
{
	AnimationInfo() :
		mBeginFrame(0), mEndFrame(0), mLoopFrame(0),
		mCurrentFrameNr(0.f), mFramesPerSecond(0.f)
	{

	}

	int mBeginFrame;
	int mEndFrame;
	int mLoopFrame;
	float mCurrentFrameNr;
	float mFramesPerSecond;
};

//! An attachment point for another MD3 model.
struct MD3Tag
{
	eastl::string mName;		//name of 'tag' as it's usually called in the md3 files 
								//try to see it as a sub-mesh/seperate mesh-part.
	float mPosition[3];			//relative position of tag
	float mRotationMatrix[9];	//3x3 rotation direction of tag
};

//! hold a tag info for connecting meshes
/** Basically its an alternate way to describe a transformation. */
struct MD3QuaternionTag
{
	virtual ~MD3QuaternionTag()
	{
		mPosition[0] = 0.f;
	}

	// construct copy constructor
	MD3QuaternionTag(const MD3QuaternionTag & copyMe)
	{
		*this = copyMe;
	}

	// construct for searching
	MD3QuaternionTag(const eastl::string& name)
		: mName(name) {}

	// construct from a position and euler angles in degrees
	MD3QuaternionTag(const Vector3<float> &pos, Vector3<float> &angle)
		: mPosition(pos)
	{
		Vector3<float> rotation = angle * (float)GE_C_DEG_TO_RAD;
		mRotation = Quaternion<float>(rotation[0], rotation[1], rotation[2], 0.f);
	}

	// set to matrix
	void Setto(Matrix4x4<float> &m)
	{
		m = Matrix4x4<float>(Rotation<4, float>(mRotation));
		m.SetRow(3, HLift(mPosition, 0.f));
	}

	bool operator == (const MD3QuaternionTag &other) const
	{
		return mName == other.mName;
	}

	MD3QuaternionTag & operator=(const MD3QuaternionTag & copyMe)
	{
		mName = copyMe.mName;
		mPosition = copyMe.mPosition;
		mRotation = copyMe.mRotation;
		return *this;
	}

	eastl::string mName;
	Vector3<float> mPosition;
	Quaternion<float> mRotation;
};

//! holds a associative list of named quaternions
struct MD3QuaternionTagList
{
	MD3QuaternionTagList()
	{
	}

	// construct copy constructor
	MD3QuaternionTagList(const MD3QuaternionTagList& copyMe)
	{
		*this = copyMe;
	}

	virtual ~MD3QuaternionTagList() {}

	const MD3QuaternionTag* Get(const eastl::string& name)
	{
		eastl::vector<MD3QuaternionTag>::const_iterator itContainer;
		for (itContainer = mContainer.begin(); itContainer != mContainer.end(); itContainer++)
			if ((*itContainer).mName == name)
				return &(*itContainer);

		return 0;
	}

	unsigned int Size() const
	{
		return mContainer.size();
	}

	const MD3QuaternionTag& operator[](unsigned int index) const
	{
		return mContainer[index];
	}

	MD3QuaternionTag& operator[](unsigned int index)
	{
		return mContainer[index];
	}

	void Pushback(const MD3QuaternionTag& other)
	{
		mContainer.push_back(other);
	}

	MD3QuaternionTagList& operator = (const MD3QuaternionTagList & copyMe)
	{
		mContainer = copyMe.mContainer;
		return *this;
	}

private:
	eastl::vector<MD3QuaternionTag> mContainer;
};


struct FrameInfo
{
	FrameInfo() : mAnimType(BOTH_DEATH1)
	{

	}

	bool operator == (const FrameInfo &other) const
	{
		return 0 == memcmp(this, &other, sizeof(FrameInfo));
	}

	//! Sets the current frame. From now on the animation is played from this frame.
	void SetCurrentFrame(float frame)
	{
		// if you pass an out of range value, we just clamp it
		mAnimations[mAnimType].mCurrentFrameNr = eastl::clamp(frame,
			(float)mAnimations[mAnimType].mBeginFrame, (float)mAnimations[mAnimType].mEndFrame);
	}

	//! Returns the currently displayed frame number.
	float GetFrameNr()
	{
		return mAnimations[mAnimType].mCurrentFrameNr;
	}

	//! Returns the current start frame number.
	int GetBeginFrame()
	{
		return mAnimations[mAnimType].mBeginFrame;
	}

	//! Returns the current start frame number.
	int GetEndFrame()
	{
		return mAnimations[mAnimType].mEndFrame;
	}

	//! sets the frames between the animation is looped.
	//! the default is 0 - MaximalFrameCount of the mesh.
	bool SetFrameLoop(int frames, int begin, int end)
	{
		const int maxFrameCount = frames - 1;
		if (end < begin)
		{
			mAnimations[mAnimType].mBeginFrame = eastl::clamp(end, 0, maxFrameCount);
			mAnimations[mAnimType].mEndFrame = eastl::clamp(begin,
				mAnimations[mAnimType].mBeginFrame, maxFrameCount);
		}
		else
		{
			mAnimations[mAnimType].mBeginFrame = eastl::clamp(begin, 0, maxFrameCount);
			mAnimations[mAnimType].mEndFrame = eastl::clamp(end,
				mAnimations[mAnimType].mBeginFrame, maxFrameCount);
		}
		if (mAnimations[mAnimType].mFramesPerSecond < 0)
			SetCurrentFrame((float)mAnimations[mAnimType].mEndFrame);
		else
			SetCurrentFrame((float)mAnimations[mAnimType].mBeginFrame);

		return true;
	}

	//! Get CurrentFrameNr and update transiting settings
	void BuildFrameNr(unsigned int timeMs)
	{
		if (mAnimations[mAnimType].mBeginFrame == mAnimations[mAnimType].mEndFrame)
		{
			mAnimations[mAnimType].mCurrentFrameNr =
				(float)mAnimations[mAnimType].mBeginFrame; //Support for non animated meshes
		}
		else if (mAnimations[mAnimType].mLoopFrame)
		{
			// play animation looped
			mAnimations[mAnimType].mCurrentFrameNr += timeMs * mAnimations[mAnimType].mFramesPerSecond;

			// We have no interpolation between EndFrame and StartFrame,
			// the last frame must be identical to first one with our current solution.
			if (mAnimations[mAnimType].mFramesPerSecond > 0.f) //forwards...
			{
				if (mAnimations[mAnimType].mCurrentFrameNr > mAnimations[mAnimType].mEndFrame)
				{
					mAnimations[mAnimType].mCurrentFrameNr = mAnimations[mAnimType].mBeginFrame +
						fmod(mAnimations[mAnimType].mCurrentFrameNr - mAnimations[mAnimType].mBeginFrame,
						(float)(mAnimations[mAnimType].mEndFrame - mAnimations[mAnimType].mBeginFrame));
				}
			}
			else //backwards...
			{
				if (mAnimations[mAnimType].mCurrentFrameNr < mAnimations[mAnimType].mBeginFrame)
				{
					mAnimations[mAnimType].mCurrentFrameNr = mAnimations[mAnimType].mEndFrame -
						fmod(mAnimations[mAnimType].mEndFrame - mAnimations[mAnimType].mCurrentFrameNr,
						(float)(mAnimations[mAnimType].mEndFrame - mAnimations[mAnimType].mBeginFrame));
				}
			}
		}
		else
		{
			// play animation non looped
			mAnimations[mAnimType].mCurrentFrameNr += timeMs * mAnimations[mAnimType].mFramesPerSecond;
			if (mAnimations[mAnimType].mFramesPerSecond > 0.f) //forwards...
			{
				if (mAnimations[mAnimType].mCurrentFrameNr > (float)mAnimations[mAnimType].mEndFrame)
				{
					mAnimations[mAnimType].mCurrentFrameNr = (float)mAnimations[mAnimType].mEndFrame;
				}
			}
			else //backwards...
			{
				if (mAnimations[mAnimType].mCurrentFrameNr < (float)mAnimations[mAnimType].mBeginFrame)
				{
					mAnimations[mAnimType].mCurrentFrameNr = (float)mAnimations[mAnimType].mBeginFrame;
				}
			}
		}
	}

	MD3AnimationType mAnimType;
	eastl::map<MD3AnimationType, AnimationInfo> mAnimations;
};

//! md3 mesh data
struct MD3Mesh : public eastl::enable_shared_from_this<MD3Mesh>
{
	MD3Mesh() : mInterPolShift(0), mLoopMode(0), mNumFrames(0), mNumTags(0)
	{

	}

	MD3Mesh(eastl::string name) : 
		mName(name), mInterPolShift(0), mLoopMode(0), mNumFrames(0), mNumTags(0)
	{

	}

	virtual ~MD3Mesh()
	{
		mMesh.clear();
		mMeshInterPol.clear();

		// delete all children
		DetachAllChildren();
	}

	void SetInterpolationShift(unsigned int shift, unsigned int loopMode);

	// MD3 Mesh
	eastl::shared_ptr<MD3Mesh> GetMesh(eastl::string meshName);
	void GetMeshes(eastl::vector<eastl::shared_ptr<MD3Mesh>>& meshes);

	bool UpdateMesh(int frame, int detailLevel, int startFrameLoop, int endFrameLoop);
	eastl::shared_ptr<MD3Mesh> CreateMesh(eastl::string parentMesh, eastl::string newMesh);

	void DetachAllChildren();

	//! returns amount of mesh buffers.
	unsigned int GetMeshBufferCount() const;

	//! returns pointer to a mesh buffer
	eastl::shared_ptr<BaseMeshBuffer> GetMeshBuffer(unsigned int nr) const;

	//! Returns pointer to a mesh buffer which fits a material
	eastl::shared_ptr<BaseMeshBuffer> GetMeshBuffer(const Material &material) const;

	//! Adds a new meshbuffer to the mesh, access it as last one
	void AddMeshBuffer(BaseMeshBuffer* meshBuffer);

	eastl::shared_ptr<MeshBuffer> CreateMeshBuffer(const eastl::shared_ptr<MeshBuffer>& source);

	void BuildVertexArray(unsigned int frameA, unsigned int frameB, float interpolate,
		eastl::shared_ptr<MeshBuffer>& source, eastl::shared_ptr<MeshBuffer>& dest);

	void BuildTagArray(unsigned int frameA, unsigned int frameB, float interpolate);

	//! tags
	bool LoadTag(MD3Tag& import);

	MD3QuaternionTagList* GetTagList(int frame, int detailLevel, int startFrameLoop, int endFrameLoop);

	//! Gets the tag count of the mesh.
	unsigned int GetTagCount()
	{
		return mNumTags;
	}

	//! Set the tag count of the animated mesh.
	void SetTagCount(unsigned int tagCount)
	{
		mNumTags = tagCount;
	}

	//! Gets the frame count of the mesh.
	unsigned int GetFrameCount() const
	{
		return mNumFrames << mInterPolShift;
	}

	//! Set the frame count of the animated mesh.
	void SetFrameCount(unsigned int frameCount)
	{
		mNumFrames = frameCount;
	}

	//! Adds a new meshbuffer to the mesh, access it as last one
	void AddFrame(FrameInfo frame)
	{
		mFrames.push_back(frame);
	}

	const eastl::vector<FrameInfo>& GetFrames()
	{
		return mFrames;
	}

	inline unsigned int Conditional(const int condition, const unsigned int a, const unsigned int b)
	{
		return ((-condition >> 31) & (a ^ b)) ^ b;
	}

	//! md3 normal
	inline Vector3<float> GetMD3Normal(float i, float j)
	{
		const float lng = i * 2.0f * (float)GE_C_PI / 255.0f;
		const float lat = j * 2.0f * (float)GE_C_PI / 255.0f;
		return Vector3<float>{
			cosf(lat) * sinf(lng), sinf(lat) * sinf(lng), cosf(lng)};
	}

	//! Cache Animation Info
	struct CacheAnimationInfo
	{
		CacheAnimationInfo(int frame = -1, int start = -1, int end = -1) :
			mFrame(frame), mStartFrameLoop(start), mEndFrameLoop(end)
		{

		}

		bool operator == (const CacheAnimationInfo &other) const
		{
			return 0 == memcmp(this, &other, sizeof(CacheAnimationInfo));
		}
		int mFrame;				// The current index into animations 
		int mStartFrameLoop;	// The start index into animations 
		int mEndFrameLoop;		// The end index into animations 
	};
	CacheAnimationInfo mCurrent;

	eastl::string mName;
	eastl::shared_ptr<MD3Mesh> mParent;
	eastl::vector<eastl::shared_ptr<MD3Mesh>> mChildren;

	unsigned int mLoopMode;
	unsigned int mInterPolShift;	// The next frame of animation to interpolate too
	unsigned int mNumFrames;
	unsigned int mNumTags;

	eastl::vector<FrameInfo> mFrames;

	//! interpolated data
	MD3QuaternionTagList mTagInterPol;
	eastl::vector<eastl::shared_ptr<MeshBuffer>> mMeshInterPol;

	//! original data
	MD3QuaternionTagList mTag;
	eastl::vector<eastl::shared_ptr<MeshBuffer>> mMesh;
};

class AnimateMeshMD3 : public BaseAnimatedMesh
{
public:

	//! constructor
	AnimateMeshMD3();

	//! destructor
	virtual ~AnimateMeshMD3();

	virtual eastl::shared_ptr<BaseMesh> GetMesh(
		int frame, int detailLevel, int startFrameLoop, int endFrameLoop);

	virtual MeshType GetMeshType() const;

	//! returns amount of mesh buffers.
	virtual unsigned int GetMeshBufferCount() const;

	//! returns pointer to a mesh buffer
	virtual eastl::shared_ptr<BaseMeshBuffer> GetMeshBuffer(unsigned int nr) const;

	//! Returns pointer to a mesh buffer which fits a material
	virtual eastl::shared_ptr<BaseMeshBuffer> GetMeshBuffer(const Material &material) const;

	//! Adds a new meshbuffer to the mesh, access it as last one
	virtual void AddMeshBuffer(BaseMeshBuffer* meshBuffer);

	void SetMD3Mesh(eastl::shared_ptr<MD3Mesh>& md3Mesh)
	{
		mRootMesh = md3Mesh;
	}

	eastl::shared_ptr<MD3Mesh> GetMD3Mesh() const
	{
		return mRootMesh;
	}

	eastl::shared_ptr<MD3Mesh> GetMD3Mesh(eastl::string meshName) const
	{
		return mRootMesh->GetMesh(meshName);
	}

	MD3QuaternionTagList* GetTagList(
		int frame, int detailLevel, int startFrameLoop, int endFrameLoop);

	//! Gets the default animation speed of the animated mesh.
	/** \return Amount of frames per second. If the amount is 0, it is a static, non animated mesh. */
	virtual float GetAnimationSpeed() const
	{
		return mFramesPerSecond;
	}

	/** \param fps Frames per second to play the animation with. If the amount is 0, it is not animated.
	The actual speed is set in the scene node the mesh is instantiated in.*/
	virtual void SetAnimationSpeed(float fps)
	{
		mFramesPerSecond = fps;
	}

	//! Gets the frame count of the animated mesh.
	virtual unsigned int GetFrameCount() const;

private:

	float mFramesPerSecond;
	eastl::shared_ptr<MD3Mesh> mRootMesh;
};

#endif

