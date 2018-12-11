// Copyright (C) 2002-2012 Nikolaus Gebhardt / Fabio Concas / Thomas Alten
// This file is part of the "Irrlicht Engine".
// For conditions of distribution and use, see copyright notice in irrlicht.h

#include "MeshMD3.h"

#include "Core/OS/OS.h"

#include "Core/Utility/StringUtil.h"

void MD3Mesh::SetInterpolationShift(unsigned int shift, unsigned int loopMode)
{
	mInterPolShift = shift;
	mLoopMode = loopMode;
}

//! returns amount of mesh buffers.
unsigned int MD3Mesh::GetMeshBufferCount() const
{
	return mMeshInterPol.size();
}

//! returns pointer to a mesh buffer
eastl::shared_ptr<BaseMeshBuffer> MD3Mesh::GetMeshBuffer(unsigned int nr) const
{
	if (nr < mMeshInterPol.size())
		return mMeshInterPol[nr];

	return 0;
}

//! Returns pointer to a mesh buffer which fits a material
eastl::shared_ptr<BaseMeshBuffer> MD3Mesh::GetMeshBuffer(const Material &material) const
{
	for (unsigned int i = 0; i<mMeshInterPol.size(); ++i)
		if (&material == mMeshInterPol[i]->GetMaterial().get())
			return mMeshInterPol[i];

	return 0;
}

//! Adds a new meshbuffer to the mesh, access it as last one
void MD3Mesh::AddMeshBuffer(BaseMeshBuffer* meshBuffer)
{
	eastl::shared_ptr<MeshBuffer> buffer((MeshBuffer*)meshBuffer);

	mMesh.push_back(buffer);
	mMeshInterPol.push_back(CreateMeshBuffer(buffer));
}


void MD3Mesh::DetachAllChildren()
{
	eastl::vector<eastl::shared_ptr<MD3Mesh>>::const_iterator it;
	for (it = mChildren.begin(); it != mChildren.end(); ++it)
		(*it)->mParent = nullptr;
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
	if (0 == mMeshInterPol.size())
		return false;

	//! check if we have the mesh in our private cache
	CacheAnimationInfo candidate(frame, startFrameLoop, endFrameLoop);
	if (candidate == mCurrent)
		return true;

	startFrameLoop = eastl::max(0, startFrameLoop >> mInterPolShift);
	endFrameLoop = Conditional(endFrameLoop < 0, mNumFrames - 1, endFrameLoop >> mInterPolShift);

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
	for (unsigned int i = 0; i != mMesh.size(); ++i)
	{
		BuildVertexArray(frameA, frameB, iPol, mMesh[i], mMeshInterPol[i]);
	}

	// build current tags
	BuildTagArray(frameA, frameB, iPol);

	mCurrent = candidate;
	return true;
}

//! create a Irrlicht MeshBuffer for a MD3 MeshBuffer
eastl::shared_ptr<MeshBuffer> MD3Mesh::CreateMeshBuffer(const eastl::shared_ptr<MeshBuffer>& source)
{
	eastl::shared_ptr<MeshBuffer> dest = eastl::make_shared<MeshBuffer>(source->GetVertice()->GetFormat(),
		source->GetVertice()->GetNumElements(), source->GetIndice()->GetNumPrimitives(), sizeof(unsigned int));

	// fill in static face info
	unsigned int numIndices = source->GetIndice()->GetNumElements();
	unsigned int* sourceIndices = source->GetIndice()->Get<unsigned int>();
	unsigned int* destIndices = dest->GetIndice()->Get<unsigned int>();
	for (unsigned int i = 0; i < numIndices; i += 3)
	{
		destIndices[i + 0] = sourceIndices[i + 0];
		destIndices[i + 1] = sourceIndices[i + 1];
		destIndices[i + 2] = sourceIndices[i + 2];
	}

	// fill in static vertex info
	unsigned int numVertices = source->GetVertice()->GetNumElements();
	for (unsigned int i = 0; i != numVertices; ++i)
		dest->TCoord(0, i) = source->TCoord(0, i);
	dest->GetMaterial() = source->GetMaterial();

	return dest;
}

//! build final mesh's vertices from frames frameA and frameB with linear interpolation.
void MD3Mesh::BuildVertexArray(unsigned int frameA, unsigned int frameB, float interpolate,
	eastl::shared_ptr<MeshBuffer>& source, eastl::shared_ptr<MeshBuffer>& dest)
{
	const unsigned int frameOffsetA = frameA * source->GetVertice()->GetNumElements();
	const unsigned int frameOffsetB = frameB * source->GetVertice()->GetNumElements();
	const float scale = (1.f / 64.f);

	for (unsigned int i = 0; i != source->GetVertice()->GetNumElements(); ++i)
	{
		const Vector3<float> &vA = source->Position(frameOffsetA + i);
		const Vector3<float> &vB = source->Position(frameOffsetB + i);

		const Vector3<float> &nA = source->Normal(frameOffsetA + i);
		const Vector3<float> &nB = source->Normal(frameOffsetB + i);

		// position
		dest->Position(i)[0] = scale * (vA[0] + interpolate * (vB[0] - vA[0]));
		dest->Position(i)[1] = scale * (vA[2] + interpolate * (vB[2] - vA[2]));
		dest->Position(i)[2] = scale * (vA[1] + interpolate * (vB[1] - vA[1]));

		// normal
		const Vector3<float> nMD3A(GetMD3Normal(nA[0], nA[1]));
		const Vector3<float> nMD3B(GetMD3Normal(nB[0], nB[1]));

		dest->Normal(i)[0] = nMD3A[0] + interpolate * (nMD3B[0] - nMD3A[0]);
		dest->Normal(i)[1] = nMD3A[2] + interpolate * (nMD3B[2] - nMD3A[2]);
		dest->Normal(i)[2] = nMD3A[1] + interpolate * (nMD3B[1] - nMD3A[1]);
	}

	//dest->recalculateBoundingBox();
}

//! build final mesh's tag from frames frameA and frameB with linear interpolation.
void MD3Mesh::BuildTagArray(unsigned int frameA, unsigned int frameB, float interpolate)
{
	const unsigned int frameOffsetA = frameA * mNumTags;
	const unsigned int frameOffsetB = frameB * mNumTags;

	for (int i = 0; i != mNumTags; ++i)
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
load tag
*/
bool MD3Mesh::LoadTag(MD3Tag& import)
{
	MD3QuaternionTag exp(import.mName);

	//! position
	exp.mPosition[0] = import.mPosition[0];
	exp.mPosition[1] = import.mPosition[2];
	exp.mPosition[2] = import.mPosition[1];

	//! construct quaternion from a RH 3x3 Matrix
	exp.mRotation.Set(import.mRotationMatrix[7],
		0.f,
		-import.mRotationMatrix[6],
		1 + import.mRotationMatrix[8]);
	Normalize(exp.mRotation);

	mTag.Pushback(exp);
	mTagInterPol.Pushback(exp);
	return true;
}

//! Returns the animated tag list based on a detail level. 0 is the lowest, 255 the highest detail.
MD3QuaternionTagList* MD3Mesh::GetTagList(
	int frame, int detailLevel, int startFrameLoop, int endFrameLoop)
{
	if (0 == mMeshInterPol.size())
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
	eastl::shared_ptr<MD3Mesh> md3Mesh =
		mRootMesh->GetMesh(ToString(meshBuffer->GetName().c_str()));

	if (md3Mesh)
		md3Mesh->AddMeshBuffer(meshBuffer);
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