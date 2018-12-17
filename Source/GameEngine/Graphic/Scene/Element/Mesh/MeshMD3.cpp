// Copyright (C) 2002-2012 Nikolaus Gebhardt / Fabio Concas / Thomas Alten
// This file is part of the "Irrlicht Engine".
// For conditions of distribution and use, see copyright notice in irrlicht.h

#include "MeshMD3.h"

#include "Core/OS/OS.h"
#include "Core/IO/Filesystem.h"
#include "Core/Utility/StringUtil.h"

void MD3Mesh::SetInterpolationShift(unsigned int shift, unsigned int loopMode)
{
	mInterPolShift = shift;
	mLoopMode = loopMode;
}

//! returns amount of mesh buffers.
unsigned int MD3Mesh::GetMeshBufferCount() const
{
	return mBufferInterpol.size();
}

//! returns pointer to a mesh buffer
eastl::shared_ptr<BaseMeshBuffer> MD3Mesh::GetMeshBuffer(unsigned int nr) const
{
	if (nr < mBufferInterpol.size())
		return mBufferInterpol[nr];

	return 0;
}

//! Returns pointer to a mesh buffer which fits a material
eastl::shared_ptr<BaseMeshBuffer> MD3Mesh::GetMeshBuffer(const Material &material) const
{
	for (unsigned int i = 0; i<mBufferInterpol.size(); ++i)
		if (&material == mBufferInterpol[i]->GetMaterial().get())
			return mBufferInterpol[i];

	return 0;
}

//! Adds a new meshbuffer to the mesh, access it as last one
void MD3Mesh::AddMeshBuffer(BaseMeshBuffer* meshBuffer)
{
	eastl::shared_ptr<MeshBuffer> buffer((MeshBuffer*)meshBuffer);
	mBufferInterpol.push_back(buffer);
}

//! create a Irrlicht MeshBuffer for a MD3 MeshBuffer
eastl::shared_ptr<MeshBuffer> MD3Mesh::CreateMeshBuffer(const eastl::shared_ptr<MD3MeshBuffer>& source)
{
	VertexFormat vformat;
	vformat.Bind(VA_POSITION, DF_R32G32B32_FLOAT, 0);
	vformat.Bind(VA_TEXCOORD, DF_R32G32_FLOAT, 0);
	vformat.Bind(VA_NORMAL, DF_R32G32B32_FLOAT, 0);

	eastl::shared_ptr<MeshBuffer> dest = eastl::make_shared<MeshBuffer>(vformat, 
		source->mMeshHeader.numVertices, source->mMeshHeader.numTriangles, sizeof(unsigned int));

	// fill in buffer info
	dest->SetName(ToWideString(source->mMeshHeader.meshName));
	for (unsigned int i = 0; i < source->mMeshHeader.numVertices; ++i)
	{
		dest->Position(i) = source->mPositions[i];
		dest->Normal(i) = source->mNormals[i];
		dest->TCoord(0, i) = Vector2<float>{ source->mTexCoords[i].u, source->mTexCoords[i].v };
	}

	// Fill in all triangles
	unsigned int iCurrent = 0;
	unsigned int* destIndices = dest->GetIndice()->Get<unsigned int>();
	for (unsigned int i = 0; i < source->mMeshHeader.numTriangles; ++i)
	{
		destIndices[iCurrent + 0] = source->mFaces[i].index[0];
		destIndices[iCurrent + 1] = source->mFaces[i].index[1];
		destIndices[iCurrent + 2] = source->mFaces[i].index[2];
		iCurrent += 3;
	}

	return dest;
}

int MD3Mesh::AttachChild(eastl::shared_ptr<MD3Mesh> const& child)
{
	if (!child)
	{
		LogError("You cannot attach null children to a node.");
		return -1;
	}

	if (child->GetParent())
	{
		LogError("The child already has a parent.");
		return -1;
	}

	child->SetParent(shared_from_this());

	// Insert the child in the first available slot (if any).
	int i = 0;
	for (auto& current : mChildren)
	{
		if (!current)
		{
			current = child;
			return i;
		}
		++i;
	}

	// All slots are used, so append the child to the array.
	int const numChildren = static_cast<int>(mChildren.size());
	mChildren.push_back(child);
	return numChildren;
}

int MD3Mesh::DetachChild(eastl::shared_ptr<MD3Mesh> const& child)
{
	if (child)
	{
		int i = 0;
		for (MD3MeshList::iterator it = mChildren.begin(); it != mChildren.end(); ++it)
		{
			if ((*it) == child)
			{
				(*it)->SetParent(nullptr);
				mChildren.erase(it);
				return i;
			}
			++i;
		}
	}
	return -1;
}

eastl::shared_ptr<MD3Mesh> MD3Mesh::DetachChildAt(int i)
{
	if (0 <= i && i < static_cast<int>(mChildren.size()))
	{
		MD3MeshList::iterator itChild = mChildren.begin() + i;
		if (itChild != mChildren.end())
		{
			(*itChild)->SetParent(nullptr);
			mChildren.erase(itChild);
		}
		return (*itChild);
	}
	return nullptr;
}

void MD3Mesh::DetachAllChildren()
{
	for (MD3MeshList::iterator it = mChildren.begin(); it != mChildren.end(); ++it)
		(*it)->SetParent(nullptr);
	mChildren.clear();
}

eastl::shared_ptr<MD3Mesh> MD3Mesh::GetTagMesh(eastl::string tagName)
{
	if (mTagInterpol.mName == tagName)
		return shared_from_this();

	for (unsigned int n = 0; n < mChildren.size(); n++)
	{
		eastl::shared_ptr<MD3Mesh> const mesh = mChildren[n]->GetTagMesh(tagName);
		if (mesh) return mesh;
	}

	return nullptr;
}

bool MD3Mesh::IsTagMesh()
{
	return mTags.Size();
}

//! Returns the animated tag
MD3QuaternionTag& MD3Mesh::GetTagInterpolation()
{
	return mTagInterpol;
}

AnimationData& MD3Mesh::GetAnimation(unsigned int nr)
{
	return mAnimations[nr];
}

void MD3Mesh::AddAnimation(AnimationData& animation)
{
	mAnimations.push_back(animation);
}

unsigned int MD3Mesh::GetAnimationCount()
{
	return mAnimations.size();
}

eastl::shared_ptr<MD3Mesh> MD3Mesh::GetRootMesh()
{
	if (mParent == nullptr)
		return shared_from_this();

	return mParent->GetRootMesh();
}

eastl::shared_ptr<MD3Mesh> MD3Mesh::GetMesh(eastl::string meshName)
{
	if (mName == meshName)
		return shared_from_this();

	for (unsigned int n = 0; n < mChildren.size(); n++)
	{
		eastl::shared_ptr<MD3Mesh> const mesh = mChildren[n]->GetMesh(meshName);
		if (mesh) return mesh;
	}

	return nullptr;
}

void MD3Mesh::GetMeshes(eastl::vector<eastl::shared_ptr<MD3Mesh>>& meshes)
{
	meshes.push_back(shared_from_this());
	for (unsigned int n = 0; n < mChildren.size(); n++)
		mChildren[n]->GetMeshes(meshes);
}


eastl::shared_ptr<MD3Mesh> MD3Mesh::CreateMesh(eastl::string parentName, eastl::string name)
{
	eastl::shared_ptr<MD3Mesh> rootMesh = GetRootMesh();
	eastl::shared_ptr<MD3Mesh> mesh = rootMesh->GetMesh(name);
	if (mesh) return mesh;

	if (!parentName.empty())
	{
		eastl::shared_ptr<MD3Mesh> meshParent = GetMesh(parentName);
		eastl::shared_ptr<MD3Mesh> meshChild(new MD3Mesh(name));
		meshParent->AttachChild(meshChild);
		return meshChild;
	}

	return shared_from_this();
}


//! update mesh based on a detail level. 0 is the lowest, 255 the highest detail.
bool MD3Mesh::UpdateMesh(int frame, int detailLevel, int startFrameLoop, int endFrameLoop)
{
	if (0 == mBufferInterpol.size())
		return false;

	//! check if we have the mesh in our private cache
	CacheAnimationInfo candidate(frame, startFrameLoop, endFrameLoop);
	if (candidate == mCurrent)
		return true;

	startFrameLoop = eastl::max(0, startFrameLoop >> mInterPolShift);
	endFrameLoop = Conditional(
		endFrameLoop < 0, mNumFrames - 1, endFrameLoop >> mInterPolShift);

	const unsigned int mask = 1 << mInterPolShift;

	int frameA;
	int frameB;
	float interpolation;
	if (mLoopMode)
	{
		// correct frame to "pixel center"
		frame -= mask >> 1;

		// interpolation
		interpolation = float(frame & (mask - 1)) * (1.0f / (float(mask)));

		// wrap anim
		frame >>= mInterPolShift;
		frameA = Conditional(frame < startFrameLoop, endFrameLoop, frame);
		frameB = Conditional(frameA + 1 > endFrameLoop, startFrameLoop, frameA + 1);
	}
	else
	{
		// correct frame to "pixel center"
		frame -= mask >> 1;

		interpolation = float(frame & (mask - 1)) * (1.0f / (float(mask)));

		// clamp anim
		frame >>= mInterPolShift;

		frameA = frame <= startFrameLoop ?
			startFrameLoop : (frame >= endFrameLoop ? endFrameLoop : frame);
		frameB = eastl::min(frameA + 1, endFrameLoop);
	}

	// build current vertex
	for (unsigned int i = 0; i != mBufferInterpol.size(); ++i)
		BuildVertexArray(i, frameA, frameB, interpolation);

	// build current tags
	BuildTagArray(frameA, frameB, interpolation);

	mCurrent = candidate;
	return true;
}

//! build final mesh's vertices from frames frameA and frameB with linear interpolation.
void MD3Mesh::BuildVertexArray(unsigned int meshId,
	unsigned int frameA, unsigned int frameB, float interpolate)
{
	const unsigned int frameOffsetA = frameA * mBufferInterpol[meshId]->GetVertice()->GetNumElements();
	const unsigned int frameOffsetB = frameB * mBufferInterpol[meshId]->GetVertice()->GetNumElements();

	for (unsigned int i = 0; i != mBuffer[meshId]->mMeshHeader.numTriangles; ++i)
	{
		for (unsigned int idx = 0; idx < 3; idx++)
		{
			int index = mBuffer[meshId]->mFaces[i].index[idx];

			const Vector3<float> &vA = mBuffer[meshId]->mPositions[frameOffsetA + index];
			const Vector3<float> &vB = mBuffer[meshId]->mPositions[frameOffsetB + index];

			// position
			mBufferInterpol[meshId]->Position(index)[0] = vA[0] + interpolate * (vB[0] - vA[0]);
			mBufferInterpol[meshId]->Position(index)[1] = vA[1] + interpolate * (vB[1] - vA[1]);
			mBufferInterpol[meshId]->Position(index)[2] = vA[2] + interpolate * (vB[2] - vA[2]);

			// normal
			const Vector3<float> &nA = mBuffer[meshId]->mNormals[frameOffsetA + index];
			const Vector3<float> &nB = mBuffer[meshId]->mNormals[frameOffsetB + index];

			mBufferInterpol[meshId]->Normal(index)[0] = nA[0] + interpolate * (nB[0] - nA[0]);
			mBufferInterpol[meshId]->Normal(index)[1] = nA[1] + interpolate * (nB[1] - nA[1]);
			mBufferInterpol[meshId]->Normal(index)[2] = nA[2] + interpolate * (nB[2] - nA[2]);
		}
	}

	//dest->recalculateBoundingBox();
}

//! build final mesh's tag from frames frameA and frameB with linear interpolation.
void MD3Mesh::BuildTagArray(unsigned int frameA, unsigned int frameB, float interpolate)
{
	for (unsigned int i = 0; i < GetChildren().size(); i++)
	{
		eastl::shared_ptr<MD3Mesh> mesh = GetChildren()[i];
		if (mesh->IsTagMesh())
		{
			const unsigned int frameOffsetA = frameA * mesh->mNumTags;
			const unsigned int frameOffsetB = frameB * mesh->mNumTags;

			const MD3QuaternionTag &qA = mesh->mTags[frameOffsetA + i];
			const MD3QuaternionTag &qB = mesh->mTags[frameOffsetB + i];

			MD3QuaternionTag &q = mesh->GetTagInterpolation();
			// rotation
			q.mRotation = Slerp(interpolate, qA.mRotation, qB.mRotation);

			// position
			q.mPosition[0] = qA.mPosition[0] + interpolate * (qB.mPosition[0] - qA.mPosition[0]);
			q.mPosition[1] = qA.mPosition[1] + interpolate * (qB.mPosition[1] - qA.mPosition[1]);
			q.mPosition[2] = qA.mPosition[2] + interpolate * (qB.mPosition[2] - qA.mPosition[2]);
		}
	}
}

//! Get CurrentFrameNr and update transiting settings
void MD3Mesh::BuildFrameNr(bool loop, unsigned int elapsedTimeMs)
{
	if (mAnimations.size())
	{
		if (mAnimations[mCurrentAnimation].mBeginFrame == mAnimations[mCurrentAnimation].mEndFrame)
		{
			//Support for non animated meshes
			mCurrentFrame = mAnimations[mCurrentAnimation].mBeginFrame;
		}
		else if (loop)
		{
			// play animation looped
			mCurrentFrame += elapsedTimeMs * mAnimations[mCurrentAnimation].mFramesPerSecond * 0.001f;

			// We have no interpolation between EndFrame and StartFrame,
			// the last frame must be identical to first one with our current solution.
			if (mAnimations[mCurrentAnimation].mFramesPerSecond > 0.f) //forwards...
			{
				if (mCurrentFrame > mAnimations[mCurrentAnimation].mEndFrame)
				{
					mCurrentFrame = mAnimations[mCurrentAnimation].mBeginFrame;
				}
			}
			else //backwards...
			{
				if (mCurrentFrame < mAnimations[mCurrentAnimation].mBeginFrame)
				{
					mCurrentFrame = mAnimations[mCurrentAnimation].mEndFrame;
				}
			}
		}
		else
		{
			// play animation non looped
			mCurrentFrame += elapsedTimeMs * mAnimations[mCurrentAnimation].mFramesPerSecond * 0.001f;

			if (mAnimations[mCurrentAnimation].mFramesPerSecond > 0.f) //forwards...
			{
				if (mCurrentFrame > mAnimations[mCurrentAnimation].mEndFrame)
				{
					mCurrentFrame = mAnimations[mCurrentAnimation].mEndFrame;
				}
			}
			else //backwards...
			{
				if (mCurrentFrame < mAnimations[mCurrentAnimation].mBeginFrame)
				{
					mCurrentFrame = mAnimations[mCurrentAnimation].mBeginFrame;
				}
			}
		}
	}
}

/*!
load model
*/
bool MD3Mesh::LoadModel(eastl::wstring& path)
{
	BaseReadFile* file = FileSystem::Get()->CreateReadFile(path);
	if (!file)
		return false;

	MD3Header header;
	//! Check MD3Header
	{
		file->Read(&header, sizeof(MD3Header));

		if (strncmp(header.headerID, "IDP3", 4))
		{
			LogError("MD3 Loader: invalid header");
			return false;
		}
	}

	LogInformation(L"Loading Quake3 model file " + path);

	//! Tag Data
	mNumFrames = header.numFrames;
	const unsigned int totalTags = header.numTags * header.numFrames;

	MD3Tag import;

	file->Seek(header.tagStart);
	MD3QuaternionTagList tags;
	for (unsigned int i = 0; i != totalTags; ++i)
	{
		file->Read(&import, sizeof(import));

		MD3QuaternionTag exp(import.name);

		//! position
		exp.mPosition[0] = import.position[0];
		exp.mPosition[1] = import.position[1];
		exp.mPosition[2] = import.position[2];

		//! construct quaternion from a RH 3x3 Matrix
		Matrix4x4<float> rotation = Matrix4x4<float>::Identity();
		rotation.SetRow(0, Vector4<float>{
			import.rotation[0], import.rotation[3], import.rotation[6], 0.f});
		rotation.SetRow(1, Vector4<float>{
			import.rotation[1], import.rotation[4], import.rotation[7], 0.f});
		rotation.SetRow(2, Vector4<float>{
			import.rotation[2], import.rotation[5], import.rotation[8], 0.f});
		exp.mRotation = Rotation<4, float>(rotation);

		tags.Pushback(exp);
	}

	// Init Tag Interpolation
	eastl::shared_ptr<MD3Mesh> rootMesh = GetRootMesh();
	for (unsigned int i = 0; i != header.numTags; i++)
	{
		eastl::shared_ptr<MD3Mesh> meshMD3 = rootMesh->GetTagMesh(tags[i].mName);
		if (!meshMD3)
			meshMD3 = CreateMesh(mName, tags[i].mName);

		if (!meshMD3->IsTagMesh())
		{
			meshMD3->mTags = tags;
			meshMD3->mNumFrames = header.numFrames;
			meshMD3->mNumTags = header.numTags;
		}
	}

	//! Meshes
	unsigned int offset = header.tagEnd;
	for (unsigned int i = 0; i != header.numMeshes; ++i)
	{
		//! construct a new mesh buffer
		eastl::shared_ptr<MD3MeshBuffer> buf(new MD3MeshBuffer());

		// !read mesh header info
		MD3MeshHeader &meshHeader = buf->mMeshHeader;

		//! read mesh info
		file->Seek(offset);
		file->Read(&meshHeader, sizeof(MD3MeshHeader));

		//! prepare memory
		eastl::vector<MD3Vertex> vertices = eastl::vector<MD3Vertex>(meshHeader.numVertices * meshHeader.numFrames);
		buf->mTexCoords = eastl::vector<MD3TexCoord>(meshHeader.numVertices);
		buf->mFaces = eastl::vector<MD3Face>(meshHeader.numTriangles);

		//! read vertices
		file->Seek(offset + meshHeader.vertexStart);
		file->Read(vertices.data(), header.numFrames * meshHeader.numVertices * sizeof(MD3Vertex));

		//! read texture coordinates
		file->Seek(offset + meshHeader.offsetSt);
		file->Read(buf->mTexCoords.data(), meshHeader.numVertices * sizeof(MD3TexCoord));

		//! read indices
		file->Seek(offset + buf->mMeshHeader.offsetTriangles);
		file->Read(buf->mFaces.data(), meshHeader.numTriangles * sizeof(MD3Face));

		//! prepare memory
		buf->mNormals = eastl::vector<Vector3<float>>(meshHeader.numVertices * meshHeader.numFrames);
		buf->mPositions = eastl::vector<Vector3<float>>(meshHeader.numVertices * meshHeader.numFrames);

		// Fill in all triangles
		const float scale = (1.f / 64.f);
		for (unsigned int i = 0; i < meshHeader.numVertices * meshHeader.numFrames; ++i)
		{
			// Read vertices
			Vector3<float>& position = buf->mPositions[i];
			position[0] = vertices[i].position[0] * scale;
			position[1] = vertices[i].position[1] * scale;
			position[2] = vertices[i].position[2] * scale;

			// Convert the normal vector to uncompressed float3 format
			Vector3<float>& normal = buf->mNormals[i];
			LatLngNormalToVec3(vertices[i].normal, (float*)&normal);
		}

		//! store meshBuffer
		mBuffer.push_back(buf);
		mBufferInterpol.push_back(CreateMeshBuffer(buf));

		offset += meshHeader.offsetEnd;
	}

	return true;
}

//! Constructor
AnimateMeshMD3::AnimateMeshMD3() //: mFPS(25.f)
{

}

//! Destructor
AnimateMeshMD3::~AnimateMeshMD3()
{

}

//! returns amount of mesh buffers.
unsigned int AnimateMeshMD3::GetMeshBufferCount() const
{
	return mRootMesh->GetMeshBufferCount();
}

//! returns pointer to a mesh buffer
eastl::shared_ptr<BaseMeshBuffer> AnimateMeshMD3::GetMeshBuffer(unsigned int nr) const
{
	return mRootMesh->GetMeshBuffer(nr);
}

//! Returns pointer to a mesh buffer which fits a material
eastl::shared_ptr<BaseMeshBuffer> AnimateMeshMD3::GetMeshBuffer(const Material &material) const
{
	eastl::shared_ptr<BaseMeshBuffer> meshBuffer = mRootMesh->GetMeshBuffer(material);
	if (meshBuffer)
		return meshBuffer;

	return 0;
}

//! Adds a new meshbuffer to the mesh, access it as last one
void AnimateMeshMD3::AddMeshBuffer(BaseMeshBuffer* meshBuffer)
{
	mRootMesh->AddMeshBuffer(meshBuffer);
}

//! Returns the animated mesh based on a detail level. 0 is the lowest, 255 the highest detail.
eastl::shared_ptr<BaseMesh> AnimateMeshMD3::GetMesh(
	int frame, int detailLevel, int startFrameLoop, int endFrameLoop)
{
	return shared_from_this();
}


//! Gets the frame count of the animated mesh.
unsigned int AnimateMeshMD3::GetFrameCount() const
{
	return 1;
}

//! Returns the type of the animated mesh.
MeshType AnimateMeshMD3::GetMeshType() const
{
	return MT_ANIMATED;
}