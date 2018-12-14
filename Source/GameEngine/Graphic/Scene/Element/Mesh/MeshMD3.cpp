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
	return mBufferInterPol.size();
}

//! returns pointer to a mesh buffer
eastl::shared_ptr<BaseMeshBuffer> MD3Mesh::GetMeshBuffer(unsigned int nr) const
{
	if (nr < mBufferInterPol.size())
		return mBufferInterPol[nr];

	return 0;
}

//! Returns pointer to a mesh buffer which fits a material
eastl::shared_ptr<BaseMeshBuffer> MD3Mesh::GetMeshBuffer(const Material &material) const
{
	for (unsigned int i = 0; i<mBufferInterPol.size(); ++i)
		if (&material == mBufferInterPol[i]->GetMaterial().get())
			return mBufferInterPol[i];

	return 0;
}

//! Adds a new meshbuffer to the mesh, access it as last one
void MD3Mesh::AddMeshBuffer(BaseMeshBuffer* meshBuffer)
{
	eastl::shared_ptr<MeshBuffer> buffer((MeshBuffer*)meshBuffer);
	mBufferInterPol.push_back(buffer);
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

void MD3Mesh::DetachAllChildren()
{
	mChildren.clear();
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


eastl::shared_ptr<MD3Mesh> MD3Mesh::CreateMesh(eastl::string parentMesh, eastl::string newMesh)
{
	if (!parentMesh.empty())
	{
		eastl::shared_ptr<MD3Mesh> meshParent = GetMesh(parentMesh);
		eastl::shared_ptr<MD3Mesh> newMesh(new MD3Mesh(newMesh));
		meshParent->mChildren.push_back(newMesh);
		return newMesh;
	}

	return shared_from_this();
}


//! update mesh based on a detail level. 0 is the lowest, 255 the highest detail.
bool MD3Mesh::UpdateMesh(int frame, int detailLevel, int startFrameLoop, int endFrameLoop)
{
	if (0 == mBufferInterPol.size())
		return false;

	//! check if we have the mesh in our private cache
	CacheAnimationInfo candidate(frame, startFrameLoop, endFrameLoop);
	if (candidate == mCurrent)
		return true;

	startFrameLoop = eastl::max(0, startFrameLoop >> mInterPolShift);
	endFrameLoop = Conditional(endFrameLoop < 0, mHeader.numFrames - 1, endFrameLoop >> mInterPolShift);

	const unsigned int mask = 1 << mInterPolShift;

	int frameA;
	int frameB;
	float iPol;
	if (mLoopMode)
	{
		// correct frame to "pixel center"
		frame -= mask >> 1;

		// interpolation
		iPol = float(frame & (mask - 1)) * (1.0f / (float(mask)));

		// wrap anim
		frame >>= mInterPolShift;
		frameA = Conditional(frame < startFrameLoop, endFrameLoop, frame);
		frameB = Conditional(frameA + 1 > endFrameLoop, startFrameLoop, frameA + 1);
	}
	else
	{
		// correct frame to "pixel center"
		frame -= mask >> 1;

		iPol = float(frame & (mask - 1)) * (1.0f / (float(mask)));

		// clamp anim
		frame >>= mInterPolShift;

		frameA = frame <= startFrameLoop ?
			startFrameLoop : (frame >= endFrameLoop ? endFrameLoop : frame);
		frameB = eastl::min(frameA + 1, endFrameLoop);
	}

	// build current vertex
	for (unsigned int i = 0; i != mBufferInterPol.size(); ++i)
		BuildVertexArray(i, frameA, frameB, iPol);

	// build current tags
	BuildTagArray(frameA, frameB, iPol);

	mCurrent = candidate;
	return true;
}

//! build final mesh's vertices from frames frameA and frameB with linear interpolation.
void MD3Mesh::BuildVertexArray(unsigned int meshId,
	unsigned int frameA, unsigned int frameB, float interpolate)
{
	const unsigned int frameOffsetA = frameA * mBufferInterPol[meshId]->GetVertice()->GetNumElements();
	const unsigned int frameOffsetB = frameB * mBufferInterPol[meshId]->GetVertice()->GetNumElements();

	for (unsigned int i = 0; i != mBuffer[meshId]->mMeshHeader.numTriangles; ++i)
	{
		for (unsigned int idx = 0; idx < 3; idx++)
		{
			int index = mBuffer[meshId]->mFaces[i].index[idx];

			const Vector3<float> &vA = mBuffer[meshId]->mPositions[frameOffsetA + index];
			const Vector3<float> &vB = mBuffer[meshId]->mPositions[frameOffsetB + index];

			// position
			mBufferInterPol[meshId]->Position(index)[0] = vA[0] + interpolate * (vB[0] - vA[0]);
			mBufferInterPol[meshId]->Position(index)[1] = vA[2] + interpolate * (vB[2] - vA[2]);
			mBufferInterPol[meshId]->Position(index)[2] = vA[1] + interpolate * (vB[1] - vA[1]);

			// normal
			const Vector3<float> &nA = mBuffer[meshId]->mNormals[frameOffsetA + index];
			const Vector3<float> &nB = mBuffer[meshId]->mNormals[frameOffsetB + index];

			mBufferInterPol[meshId]->Normal(index)[0] = nA[0] + interpolate * (nB[0] - nA[0]);
			mBufferInterPol[meshId]->Normal(index)[1] = nA[2] + interpolate * (nB[2] - nA[2]);
			mBufferInterPol[meshId]->Normal(index)[2] = nA[1] + interpolate * (nB[1] - nA[1]);
		}
	}

	//dest->recalculateBoundingBox();
}

//! build final mesh's tag from frames frameA and frameB with linear interpolation.
void MD3Mesh::BuildTagArray(unsigned int frameA, unsigned int frameB, float interpolate)
{
	const unsigned int frameOffsetA = frameA * mHeader.numTags;
	const unsigned int frameOffsetB = frameB * mHeader.numTags;

	for (unsigned int i = 0; i != mHeader.numTags; ++i)
	{
		MD3QuaternionTag &d = mTagInterPol[i];

		const MD3QuaternionTag &qA = mTag[frameOffsetA + i];
		const MD3QuaternionTag &qB = mTag[frameOffsetB + i];

		// rotation
		d.mRotation = Slerp(interpolate, qA.mRotation, qB.mRotation);

		// position
		d.mPosition[0] = qA.mPosition[0] + interpolate * (qB.mPosition[0] - qA.mPosition[0]);
		d.mPosition[1] = qA.mPosition[1] + interpolate * (qB.mPosition[1] - qA.mPosition[1]);
		d.mPosition[2] = qA.mPosition[2] + interpolate * (qB.mPosition[2] - qA.mPosition[2]);
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

	//! Check MD3Header
	{
		file->Read(&mHeader, sizeof(MD3Header));

		if (strncmp(mHeader.headerID, "IDP3", 4))
		{
			LogError("MD3 Loader: invalid header");
			return false;
		}
	}

	LogInformation(L"Loading Quake3 model file " + path);

	//! Tag Data
	const unsigned int totalTags = mHeader.numTags * mHeader.numFrames;

	MD3Tag import;

	file->Seek(mHeader.tagStart);
	for (unsigned int i = 0; i != totalTags; ++i)
	{
		file->Read(&import, sizeof(import));

		MD3QuaternionTag exp(import.name);

		//! position
		exp.mPosition[0] = import.position[0];
		exp.mPosition[1] = import.position[2];
		exp.mPosition[2] = import.position[1];

		//! construct quaternion from a RH 3x3 Matrix
		exp.mRotation.Set(import.rotationMatrix[7],
			0.f,
			-import.rotationMatrix[6],
			1 + import.rotationMatrix[8]);
		Normalize(exp.mRotation);

		mTag.Pushback(exp);
		mTagInterPol.Pushback(exp);
	}

	//! Meshes
	unsigned int offset = mHeader.tagEnd;
	for (unsigned int i = 0; i != mHeader.numMeshes; ++i)
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
		file->Read(vertices.data(), mHeader.numFrames * meshHeader.numVertices * sizeof(MD3Vertex));

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
		mBufferInterPol.push_back(CreateMeshBuffer(buf));

		offset += meshHeader.offsetEnd;
	}

	return true;
}

//! Returns the animated tag list based on a detail level. 0 is the lowest, 255 the highest detail.
MD3QuaternionTagList* MD3Mesh::GetTagList(
	int frame, int detailLevel, int startFrameLoop, int endFrameLoop)
{
	if (0 == mBufferInterPol.size())
		return 0;

	UpdateMesh(frame, detailLevel, startFrameLoop, endFrameLoop);
	return &mTagInterPol;
}

//! Constructor
AnimateMeshMD3::AnimateMeshMD3() //: mFPS(25.f)
{

}

//! Destructor
AnimateMeshMD3::~AnimateMeshMD3()
{

}

//! Returns the animated tag list based on a detail level. 0 is the lowest, 255 the highest detail.
MD3QuaternionTagList* AnimateMeshMD3::GetTagList(
	int frame, int detailLevel, int startFrameLoop, int endFrameLoop)
{
	MD3QuaternionTagList* meshTagList = mRootMesh->GetTagList(
		frame, detailLevel, startFrameLoop, endFrameLoop);

	return meshTagList;
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