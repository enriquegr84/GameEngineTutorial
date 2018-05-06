// Copyright (C) 2002-2012 Nikolaus Gebhardt
// This file is part of the "Irrlicht Engine".
// For conditions of distribution and use, see copyright notice in irrlicht.h

#include "ShadowVolumeNode.h"
#include "CameraNode.h"

#include "Graphic/Renderer/Renderer.h"

#include "Core/OS/OS.h"

#include "Graphic/Scene/Scene.h"


//! constructor
ShadowVolumeNode::ShadowVolumeNode(const ActorId actorId, PVWUpdater& updater, 
	WeakBaseRenderComponentPtr renderComponent, const eastl::shared_ptr<BaseMesh>& shadowMesh, 
	bool zfailmethod, float infinity)
:	Node(actorId, renderComponent, RP_SHADOW, NT_SHADOW_VOLUME), mShadowMesh(0), mIndexCount(0), 
	mVertexCount(0), mShadowVolumesUsed(0), mInfinity(infinity), mUseZFailMethod(zfailmethod)
{
	mPVWUpdater = updater;
	SetShadowMesh(shadowMesh);
	//SetAutomaticCulling(AC_OFF);
}

void ShadowVolumeNode::SetShadowMesh(const eastl::shared_ptr<BaseMesh>& mesh)
{
	if (mShadowMesh == mesh)
		return;

	mShadowMesh = mesh;

	//if (mShadowMesh)
	//	mBBox = mShadowMesh->GetBoundingBox();

	if (mShadowMesh)
	{
		MeshFactory mf;

		mVisuals.clear();
		for (unsigned int i = 0; i<mShadowMesh->GetMeshBufferCount(); ++i)
		{
			const eastl::shared_ptr<MeshBuffer<float>>& meshBuffer = mShadowMesh->GetMeshBuffer(i);
			if (meshBuffer)
			{
				eastl::shared_ptr<Visual> visual = mf.CreateMesh(meshBuffer->mMesh.get());
				eastl::shared_ptr<PointLightTextureEffect> effect = eastl::make_shared<PointLightTextureEffect>(
					ProgramFactory::Get(), mPVWUpdater.GetUpdater(), meshBuffer->GetMaterial(), eastl::make_shared<Light>(),
					eastl::make_shared<LightCameraGeometry>(), eastl::make_shared<Texture2>(), SamplerState::MIN_L_MAG_L_MIP_L,
					SamplerState::WRAP, SamplerState::WRAP);
				visual->SetEffect(effect);
				mVisuals.push_back(visual);
				mPVWUpdater.Subscribe(visual->GetAbsoluteTransform(), effect->GetPVWMatrixConstant());
			}
		}
	}
}



void ShadowVolumeNode::CreateShadowVolume(const Vector3<float>& light, bool isDirectional)
{
	ShadowVolume* svp = 0;
	AlignedBox3<float>* bb = 0;

	// builds the shadow volume and adds it to the shadow volume list.
	if (mShadowVolumes.size() > mShadowVolumesUsed)
	{
		// get the next unused buffer

		svp = &mShadowVolumes[mShadowVolumesUsed];
		bb = &mShadowBBox[mShadowVolumesUsed];
	}
	else
	{
		mShadowVolumes.push_back(ShadowVolume());
		svp = &mShadowVolumes.back();

		mShadowBBox.push_back(AlignedBox3<float>());
		bb = &mShadowBBox.back();
	}
	++mShadowVolumesUsed;

	// We use triangle lists
	unsigned int numEdges = 0;

	numEdges=CreateEdgesAndCaps(light, svp, bb);

	// for all edges add the near->far quads
	for (unsigned int i=0; i<numEdges; ++i)
	{
		const Vector3<float> &v1 = mVertices[mEdges[2*i+0]];
		const Vector3<float> &v2 = mVertices[mEdges[2*i+1]];
		const Vector3<float> v3(Normalize(v1+(v1 - light))*mInfinity);
		const Vector3<float> v4(Normalize(v2+(v2 - light))*mInfinity);

		// Add a quad (two triangles) to the vertex list
		svp->push_back(v1);
		svp->push_back(v2);
		svp->push_back(v3);

		svp->push_back(v2);
		svp->push_back(v4);
		svp->push_back(v3);
	}
}


#define _USE_ADJACENCY
#define _USE_REVERSE_EXTRUDED

unsigned int ShadowVolumeNode::CreateEdgesAndCaps(const Vector3<float>& light, ShadowVolume* svp, AlignedBox3<float>* bb)
{
	unsigned int numEdges=0;
	const unsigned int faceCount = mIndexCount / 3;
	/*
	if(faceCount >= 1)
		bb->Reset(mVertices[mIndices[0]]);
	else
		bb->Reset(0,0,0);
	*/
	// Check every face if it is front or back facing the light.
	for (unsigned int i=0; i<faceCount; ++i)
	{
		const Vector3<float> v0 = mVertices[mIndices[3*i+0]];
		const Vector3<float> v1 = mVertices[mIndices[3*i+1]];
		const Vector3<float> v2 = mVertices[mIndices[3*i+2]];

#ifdef _USE_REVERSE_EXTRUDED
		//! Test if the triangle would be front or backfacing from any point.
		Vector3<float> normal = Cross(v1 - v0, v2 - v2);
		Normalize(normal);
		mFaceData[i] = Dot(normal, light) <= 0.0f;
#else
		//! Test if the triangle would be front or backfacing from any point.
		Vector3<float> normal = Cross(v1 - v2, v2 - v0);
		Normalize(normal);
		mFaceData[i] = Dot(normal, light) <= 0.0f;
#endif

		if (mUseZFailMethod && mFaceData[i])
		{
			// add front cap from light-facing faces
			svp->push_back(v2);
			svp->push_back(v1);
			svp->push_back(v0);

			// add back cap
			const Vector3<float> i0 = Normalize(v0+(v0-light))*mInfinity;
			const Vector3<float> i1 = Normalize(v1+(v1-light))*mInfinity;
			const Vector3<float> i2 = Normalize(v2+(v2-light))*mInfinity;

			svp->push_back(i0);
			svp->push_back(i1);
			svp->push_back(i2);
			/*
			bb->AddInternalPoint(i0);
			bb->AddInternalPoint(i1);
			bb->AddInternalPoint(i2);
			*/
		}
	}

	// Create edges
	for (unsigned int i=0; i<faceCount; ++i)
	{
		// check all front facing faces
		if (mFaceData[i] == true)
		{
			const unsigned int wFace0 = mIndices[3*i+0];
			const unsigned int wFace1 = mIndices[3*i+1];
			const unsigned int wFace2 = mIndices[3*i+2];

			const unsigned int adj0 = mAdjacency[3*i+0];
			const unsigned int adj1 = mAdjacency[3*i+1];
			const unsigned int adj2 = mAdjacency[3*i+2];

			// add edges if face is adjacent to back-facing face
			// or if no adjacent face was found
#ifdef _USE_ADJACENCY
			if (adj0 == i || mFaceData[adj0] == false)
#endif
			{
				// add edge v0-v1
				mEdges[2*numEdges+0] = wFace0;
				mEdges[2*numEdges+1] = wFace1;
				++numEdges;
			}

#ifdef _USE_ADJACENCY
			if (adj1 == i || mFaceData[adj1] == false)
#endif
			{
				// add edge v1-v2
				mEdges[2*numEdges+0] = wFace1;
				mEdges[2*numEdges+1] = wFace2;
				++numEdges;
			}

#ifdef _USE_ADJACENCY
			if (adj2 == i || mFaceData[adj2] == false)
#endif
			{
				// add edge v2-v0
				mEdges[2*numEdges+0] = wFace2;
				mEdges[2*numEdges+1] = wFace0;
				++numEdges;
			}
		}
	}
	return numEdges;
}

void ShadowVolumeNode::UpdateShadowVolumes(Scene *pScene)
{
	const unsigned int oldIndexCount = mIndexCount;
	const unsigned int oldVertexCount = mVertexCount;

	const BaseMesh* const mesh = mShadowMesh.get();
	if (!mesh) return;

	// create as much shadow volumes as there are lights but
	// do not ignore the max light settings.
	/*
	const unsigned int lightCount = Renderer::Get()->GetDynamicLightCount();
	if (!lightCount)
		return;
	*/
	// calculate total amount of vertices and indices

	mVertexCount = 0;
	mIndexCount = 0;
	mShadowVolumesUsed = 0;

	unsigned int i;
	unsigned int totalVertices = 0;
	unsigned int totalIndices = 0;
	const unsigned int bufcnt = mesh->GetMeshBufferCount();

	for (i=0; i<bufcnt; ++i)
	{
		const MeshBuffer<float>* buf = mesh->GetMeshBuffer(i).get();
		totalIndices += buf->GetNumElements();
		totalVertices += buf->GetNumElements();
	}

	// copy mesh
	for (unsigned int i = 0; i<mesh->GetMeshBufferCount(); ++i)
	{
		unsigned int const* index = mVisuals[i]->GetIndexBuffer()->Get<unsigned int>();
		unsigned int numElements = mVisuals[i]->GetIndexBuffer()->GetNumElements();
		for (unsigned int i = 0; i < numElements; ++i, ++index)
			mIndices[mIndexCount++] = *index + mVertexCount;

		struct Vertex
		{
			Vector3<float> position;
		};
		Vertex* vertex = mVisuals[i]->GetVertexBuffer()->Get<Vertex>();
		numElements = mVisuals[i]->GetVertexBuffer()->GetNumElements();
		for (unsigned int i = 0; i < numElements; ++i, ++index)
			mVertices[mVertexCount++] = vertex[i].position;
	}

	// recalculate adjacency if necessary
	if (oldVertexCount != mVertexCount || oldIndexCount != mIndexCount)
		CalculateAdjacency();

	//Matrix4x4<float> toWorld, fromWorld;
	//GetParent()->GetAbsoluteTransform(&toWorld, &fromWorld);
	//toWorld.MakeInverse();

	const Vector3<float> parentpos = GetParent()->GetAbsoluteTransform().GetTranslation();

	// TODO: Only correct for point lights.
	/*
	for (i=0; i<lightCount; ++i)
	{
		const Light& dl = Renderer::Get()->GetDynamicLight(i);
		Vector3<float> lpos = dl.mLighting->mPosition;
		if (dl.mLighting->mCastShadows &&
			fabs(Length(lpos - parentpos)) <= (dl.mLighting->mRadius*dl.mLighting->mRadius*4.0f))
		{
			toWorld.TransformVect(lpos);
			CreateShadowVolume(lpos);
		}
	}
	*/
}

//! pre render method
bool ShadowVolumeNode::PreRender(Scene* pScene)
{
	if (IsVisible())
	{
		// register according to material types counted
		if (!pScene->IsCulled(this))
			pScene->AddToRenderQueue(RP_SHADOW, shared_from_this());
	}

	return Node::PreRender(pScene);
}

//! renders the node.
bool ShadowVolumeNode::Render(Scene* pScene)
{
	if (!mShadowVolumesUsed || !Renderer::Get())
		return false;

	//Renderer::Get()->SetTransform(TS_WORLD, toWorld);

	for (unsigned int i=0; i<mShadowVolumesUsed; ++i)
	{
		bool drawShadow = true;

		if (mUseZFailMethod && pScene->GetActiveCamera())
		{
			// Disable shadows drawing, when back cap is behind of ZFar plane.

			const float* frustum = pScene->GetActiveCamera()->Get()->GetFrustum();

			//Matrix4x4<float> invTrans(toWorld, Matrix4x4<float>::EM4CONST_INVERSE);
			//frust.Transform(invTrans);

			Vector3<float> edges[8];
			//mShadowBBox[i].GetEdges(edges);

			Vector3<float> largestEdge = edges[0];
			float maxDistance = Length(pScene->GetActiveCamera()->GetAbsoluteTransform().GetTranslation() - edges[0]);
			float curDistance = 0.f;

			for(int j = 1; j < 8; ++j)
			{
				curDistance = Length(pScene->GetActiveCamera()->GetAbsoluteTransform().GetTranslation()  - edges[j]);
				if(curDistance > maxDistance)
				{
					maxDistance = curDistance;
					largestEdge = edges[j];
				}
			}
			/*
			if (!(frustum[Camera::VF_DMAX].ClassifyPointRelation(largestEdge) != ISREL3D_FRONT))
				drawShadow = false;
			*/
		}
		/*
		if(!drawShadow)
		{
			eastl::vector<Vector3<float>> triangles;
			Renderer::Get()->DrawStencilShadowVolume(triangles, mUseZFailMethod, DebugDataVisible());
		}
		else Renderer::Get()->DrawStencilShadowVolume(mShadowVolumes[i], mUseZFailMethod, DebugDataVisible());
		*/
	}
	return true;
}

//! Generates adjacency information based on mesh indices.
void ShadowVolumeNode::CalculateAdjacency()
{
	// go through all faces and fetch their three neighbours
	for (unsigned int f=0; f<mIndexCount; f+=3)
	{
		for (unsigned int edge = 0; edge<3; ++edge)
		{
			const Vector3<float>& v1 = mVertices[mIndices[f+edge]];
			const Vector3<float>& v2 = mVertices[mIndices[f+((edge+1)%3)]];

			// now we search an_O_ther _F_ace with these two
			// vertices, which is not the current face.
			unsigned int of;

			for (of=0; of<mIndexCount; of+=3)
			{
				// only other faces
				if (of != f)
				{
					bool cnt1 = false;
					bool cnt2 = false;

					for (int e=0; e<3; ++e)
					{
						if (v1 == mVertices[mIndices[of+e]])
							cnt1=true;

						if (v2 == mVertices[mIndices[of+e]])
							cnt2=true;
					}
					// one match for each vertex, i.e. edge is the same
					if (cnt1 && cnt2)
						break;
				}
			}

			// no adjacent edges -> store face number, else store adjacent face
			if (of >= mIndexCount)
				mAdjacency[f + edge] = f/3;
			else
				mAdjacency[f + edge] = of/3;
		}
	}
}