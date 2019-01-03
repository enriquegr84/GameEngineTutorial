// Copyright (C) 2002-2012 Nikolaus Gebhardt / Thomas Alten
// This file is part of the "Irrlicht Engine".
// For conditions of distribution and use, see copyright notice in irrlicht.h

#ifndef MESHMD3_H
#define MESHMD3_H

#include "Graphic/Scene/Element/Mesh/Mesh.h"

#include "Mathematic/Algebra/Rotation.h"

// include this file right before the data structures to be 1-aligned
// and add to each structure the PACK_STRUCT define just like this:
// struct mystruct
// {
//	...
// } PACK_STRUCT;
// Always declare unpack right after the last type declared
// like this, and do not put any other types with different alignment
// in between!

// byte-align structures
#if defined(_MSC_VER) || defined(__BORLANDC__) || defined (__BCPLUSPLUS__)
#	pragma warning(disable: 4103)
#	pragma pack( push, packing )
#	pragma pack( 1 )
#	define PACK_STRUCT
#elif defined( __DMC__ )
#	pragma pack( push, 1 )
#	define PACK_STRUCT
#elif defined( __GNUC__ )
// Using pragma pack might work with earlier gcc versions already, but
// it started to be necessary with gcc 4.7 on mingw unless compiled with -mno-ms-bitfields.
// And I found some hints on the web that older gcc versions on the other hand had sometimes
// trouble with pragma pack while they worked with __attribute__((packed)).
#	if (__GNUC__ > 4 ) || ((__GNUC__ == 4 ) && (__GNUC_MINOR__ >= 7))
#		pragma pack( push, packing )
#		pragma pack( 1 )
#		define PACK_STRUCT
#	else
#		define PACK_STRUCT	__attribute__((packed))
#endif
#else
#	error compiler not supported
#endif

// master scale factor for all vertices in a MD3 model
#define MD3_XYZ_SCALE        (1.0f/64.0f)

//! this holds the header info of the MD3 file
struct MD3Header
{
	char headerID[4];	//id of file, always "IDP3"
	unsigned int	version;	//this is a version number, always 15
	char fileName[68];	//sometimes left Blank... 65 chars, 32bit aligned == 68 chars
	int	numFrames;	//number of KeyFrames
	unsigned int	numTags;	//number of 'tags' per frame
	unsigned int	numMeshes;	//number of meshes/skins
	unsigned int	numMaxSkins;	//maximum number of unique skins used in md3 file. artefact md2
	unsigned int	frameStart;	//starting position of frame-structur
	unsigned int	tagStart;	//starting position of tag-structures
	unsigned int	tagEnd;		//ending position of tag-structures/starting position of mesh-structures
	unsigned int	fileSize;
} PACK_STRUCT;

//! this holds the header info of an MD3 mesh section
struct MD3MeshHeader
{
	char meshID[4];		//id, must be IDP3
	char meshName[68];	//name of mesh 65 chars, 32 bit aligned == 68 chars

	unsigned int numFrames;		//number of meshframes in mesh
	unsigned int numShader;		//number of skins in mesh
	unsigned int numVertices;	//number of vertices
	unsigned int numTriangles;	//number of Triangles

	unsigned int offsetTriangles;	//starting position of Triangle data, relative to start of Mesh_Header
	unsigned int offsetShaders;	//size of header
	unsigned int offsetSt;		//starting position of texvector data, relative to start of Mesh_Header
	unsigned int vertexStart;	//starting position of vertex data,relative to start of Mesh_Header
	int offsetEnd;
} PACK_STRUCT;

//! Compressed Vertex Data
struct MD3Vertex
{
	short position[3];
	unsigned short normal;
} PACK_STRUCT;

//! Texture Coordinate
struct MD3TexCoord
{
	float u;
	float v;
} PACK_STRUCT;

//! Triangle Index
struct MD3Face
{
	int index[3];
} PACK_STRUCT;

//! An attachment point for another MD3 model.
struct MD3Tag
{
	char name[64];		//name of 'tag' as it's usually called in the md3 files 
						//try to see it as a sub-mesh/seperate mesh-part.
	float position[3];	//relative position of tag
	float rotation[9];	//3x3 rotation direction of tag
} PACK_STRUCT;

// Default alignment
#if defined(_MSC_VER) || defined(__BORLANDC__) || defined (__BCPLUSPLUS__)
#	pragma pack( pop, packing )
#elif defined (__DMC__)
#	pragma pack( pop )
#elif defined( __GNUC__ )
#   if (__GNUC__ > 4 ) || ((__GNUC__ == 4 ) && (__GNUC_MINOR__ >= 7))
#	    pragma pack( pop, packing )
#   endif
#endif

#undef PACK_STRUCT

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

	LEGS_WALKCR = 6,		// The legs's crouching walk animation
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
	ANIMATION_COUNT = 25
};

struct AnimationData
{
	AnimationData() : mAnimationType(0), 
		mBeginFrame(0), mEndFrame(0), mLoopFrame(0), mFramesPerSecond(0.f)
	{

	}

	int mBeginFrame;
	int mEndFrame;
	int mLoopFrame;
	float mFramesPerSecond;
	int mAnimationType;
};

//! Holding Frame Data for a Mesh
struct MD3MeshBuffer
{
	MD3MeshHeader mMeshHeader;

	eastl::vector<MD3Face> mFaces;
	eastl::vector<MD3TexCoord> mTexCoords;
	eastl::vector<Vector3<float>> mNormals;
	eastl::vector<Vector3<float>> mPositions;
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

	// default constructor
	MD3QuaternionTag() {}

	// construct for searching
	MD3QuaternionTag(const eastl::string& name)
		: mName(name) 
	{
	
	}

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

// Forward declarations
class MD3Mesh;

typedef eastl::vector<eastl::shared_ptr<MD3Mesh>> MD3MeshList;

//! md3 mesh data
class MD3Mesh : public eastl::enable_shared_from_this<MD3Mesh>
{
public:
	MD3Mesh() : mInterPolShift(0), mLoopMode(0), mParent(nullptr),
		mNumTags(0), mNumFrames(0), mCurrentFrame(0), mCurrentAnimation(0)
	{

	}

	MD3Mesh(eastl::string name) : 
		mName(name), mInterPolShift(0), mLoopMode(0), mParent(nullptr),
		mNumTags(0), mNumFrames(0), mCurrentFrame(0), mCurrentAnimation(0)
	{

	}

	virtual ~MD3Mesh()
	{
		mBufferInterpol.clear();

		// delete all children
		DetachAllChildren();
	}

	void SetInterpolationShift(unsigned int shift, unsigned int loopMode);

	// MD3 Mesh
	eastl::string GetName() { return mName; }

	eastl::shared_ptr<MD3Mesh> GetRootMesh();
	eastl::shared_ptr<MD3Mesh> GetMesh(eastl::string meshName);
	void GetMeshes(eastl::vector<eastl::shared_ptr<MD3Mesh>>& meshes);

	bool UpdateMesh(int frame, int detailLevel, int startFrameLoop, int endFrameLoop);
	eastl::shared_ptr<MD3Mesh> CreateMesh(eastl::string parentMesh, eastl::string newMesh);

	eastl::shared_ptr<MD3Mesh> GetParent() { return mParent; }
	void SetParent(eastl::shared_ptr<MD3Mesh> parent){ mParent = parent; }

	int AttachChild(eastl::shared_ptr<MD3Mesh> const& child);
	int DetachChild(eastl::shared_ptr<MD3Mesh> const& child);
	eastl::shared_ptr<MD3Mesh> DetachChildAt(int i);
	void DetachAllChildren();

	MD3MeshList& GetChildren() { return mChildren; }

	//! returns amount of mesh buffers.
	unsigned int GetMeshBufferCount() const;

	//! returns pointer to a mesh buffer
	eastl::shared_ptr<BaseMeshBuffer> GetMeshBuffer(unsigned int nr) const;

	//! Returns pointer to a mesh buffer which fits a material
	eastl::shared_ptr<BaseMeshBuffer> GetMeshBuffer(const Material &material) const;

	//! Adds a new meshbuffer to the mesh, access it as last one
	void AddMeshBuffer(BaseMeshBuffer* meshBuffer);

	eastl::shared_ptr<MeshBuffer> CreateMeshBuffer(const eastl::shared_ptr<MD3MeshBuffer>& source);

	void BuildFrameNr(bool loop, unsigned int timeMs);
	void BuildVertexArray(unsigned int meshId, unsigned int frameA, unsigned int frameB, float interpolate);
	void BuildTagArray(unsigned int frameA, unsigned int frameB, float interpolate);

	//! tags
	bool IsTagMesh();
	MD3QuaternionTag& GetTagInterpolation();
	eastl::shared_ptr<MD3Mesh> GetTagMesh(eastl::string tagName);

	//! animations
	float GetCurrentFrame() { return mCurrentFrame; }
	void SetCurrentFrame(float currentFrame) { mCurrentFrame = currentFrame; }
	void SetCurrentAnimation( unsigned int currentAnim) { mCurrentAnimation = currentAnim; }
	unsigned int GetCurrentAnimation() { return mCurrentAnimation; }
	unsigned int GetAnimationCount();
	AnimationData& GetAnimation(unsigned int nr);
	void AddAnimation(AnimationData& animation);

	//! model
	bool LoadModel(eastl::wstring& path);

protected:

	inline unsigned int Conditional(const int condition, const unsigned int a, const unsigned int b)
	{
		return ((-condition >> 31) & (a ^ b)) ^ b;
	}

	// -------------------------------------------------------------------------------
	/** @brief
	*  @param iNormal Input normal vector in latitude/longitude form
	*  @param afOut Pointer to an array of three floats to receive the result
	*/
	inline void LatLngNormalToVec3(short iNormal, float* afOut)
	{
		float lat = (float)((iNormal >> 8u) & 0xff);
		float lng = (float)((iNormal & 0xff));
		const float invVal(float(1.0) / float(128.0));
		lat *= float(3.141926) * invVal;
		lng *= float(3.141926) * invVal;

		afOut[0] = cos(lat) * sin(lng);
		afOut[1] = sin(lat) * sin(lng);
		afOut[2] = cos(lng);
	}

private:

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
	MD3MeshList mChildren;
	eastl::shared_ptr<MD3Mesh> mParent;

	int mNumFrames;
	float mCurrentFrame;
	unsigned int mCurrentAnimation;
	eastl::vector<AnimationData> mAnimations;

	unsigned int mLoopMode;
	unsigned int mInterPolShift;	// The next frame of animation to interpolate too

	//! tag data
	unsigned int mNumTags;
	MD3QuaternionTagList mTags;
	MD3QuaternionTag mTagInterpol;

	eastl::vector<eastl::shared_ptr<MD3MeshBuffer>> mBuffer;
	eastl::vector<eastl::shared_ptr<MeshBuffer>> mBufferInterpol;
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

