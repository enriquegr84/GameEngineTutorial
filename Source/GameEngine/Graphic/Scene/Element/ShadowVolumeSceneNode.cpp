// Copyright (C) 2002-2012 Nikolaus Gebhardt
// This file is part of the "Irrlicht Engine".
// For conditions of distribution and use, see copyright notice in irrlicht.h

#include "CameraSceneNode.h"
#include "ShadowVolumeSceneNode.h"

#include "Scenes/Scene.h"

#include "Graphics/IRenderer.h"
#include "Graphics/Light.h"

#include "Utilities/ViewFrustum.h"

#include "OS/os.h"


//! constructor
ShadowVolumeSceneNode::ShadowVolumeSceneNode(const ActorId actorId, WeakBaseRenderComponentPtr renderComponent, 
	matrix4 const *t, const shared_ptr<IMesh>& shadowMesh, bool zfailmethod, f32 infinity)
:	SceneNode(actorId, renderComponent, ERP_SHADOW, ESNT_SHADOW_VOLUME, t), m_ShadowMesh(0), m_IndexCount(0), 
	m_VertexCount(0), m_ShadowVolumesUsed(0), m_Infinity(infinity), m_UseZFailMethod(zfailmethod)
{
	SetShadowMesh(shadowMesh);
	
	Get()->SetAutomaticCulling(EAC_OFF);
}


void ShadowVolumeSceneNode::CreateShadowVolume(const Vector3f& light, bool isDirectional)
{
	ShadowVolume* svp = 0;
	AABBox3<f32>* bb = 0;

	// builds the shadow volume and adds it to the shadow volume list.

	if (m_ShadowVolumes.size() > m_ShadowVolumesUsed)
	{
		// get the next unused buffer

		svp = &m_ShadowVolumes[m_ShadowVolumesUsed];
		bb = &m_ShadowBBox[m_ShadowVolumesUsed];
	}
	else
	{
		m_ShadowVolumes.push_back(ShadowVolume());
		svp = &m_ShadowVolumes.back();

		m_ShadowBBox.push_back(AABBox3<f32>());
		bb = &m_ShadowBBox.back();
	}
	++m_ShadowVolumesUsed;

	// We use triangle lists
	u32 numEdges = 0;

	numEdges=CreateEdgesAndCaps(light, svp, bb);

	// for all edges add the near->far quads
	for (u32 i=0; i<numEdges; ++i)
	{
		const Vector3f &v1 = m_Vertices[m_Edges[2*i+0]];
		const Vector3f &v2 = m_Vertices[m_Edges[2*i+1]];
		const Vector3f v3(v1+(v1 - light).Normalize()*m_Infinity);
		const Vector3f v4(v2+(v2 - light).Normalize()*m_Infinity);

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

u32 ShadowVolumeSceneNode::CreateEdgesAndCaps(const Vector3f& light, ShadowVolume* svp, AABBox3<f32>* bb)
{
	u32 numEdges=0;
	const u32 faceCount = m_IndexCount / 3;

	if(faceCount >= 1)
		bb->Reset(m_Vertices[m_Indices[0]]);
	else
		bb->Reset(0,0,0);

	// Check every face if it is front or back facing the light.
	for (u32 i=0; i<faceCount; ++i)
	{
		const Vector3f v0 = m_Vertices[m_Indices[3*i+0]];
		const Vector3f v1 = m_Vertices[m_Indices[3*i+1]];
		const Vector3f v2 = m_Vertices[m_Indices[3*i+2]];

#ifdef _USE_REVERSE_EXTRUDED
		m_FaceData[i]=Triangle3f(v0,v1,v2).IsFrontFacing(light);
#else
		m_FaceData[i]=Triangle3f(v2,v1,v0).IsFrontFacing(light);
#endif

		if (m_UseZFailMethod && m_FaceData[i])
		{
			// add front cap from light-facing faces
			svp->push_back(v2);
			svp->push_back(v1);
			svp->push_back(v0);

			// add back cap
			const Vector3f i0 = v0+(v0-light).Normalize()*m_Infinity;
			const Vector3f i1 = v1+(v1-light).Normalize()*m_Infinity;
			const Vector3f i2 = v2+(v2-light).Normalize()*m_Infinity;

			svp->push_back(i0);
			svp->push_back(i1);
			svp->push_back(i2);

			bb->AddInternalPoint(i0);
			bb->AddInternalPoint(i1);
			bb->AddInternalPoint(i2);
		}
	}

	// Create edges
	for (u32 i=0; i<faceCount; ++i)
	{
		// check all front facing faces
		if (m_FaceData[i] == true)
		{
			const u16 wFace0 = m_Indices[3*i+0];
			const u16 wFace1 = m_Indices[3*i+1];
			const u16 wFace2 = m_Indices[3*i+2];

			const u16 adj0 = m_Adjacency[3*i+0];
			const u16 adj1 = m_Adjacency[3*i+1];
			const u16 adj2 = m_Adjacency[3*i+2];

			// add edges if face is adjacent to back-facing face
			// or if no adjacent face was found
#ifdef _USE_ADJACENCY
			if (adj0 == i || m_FaceData[adj0] == false)
#endif
			{
				// add edge v0-v1
				m_Edges[2*numEdges+0] = wFace0;
				m_Edges[2*numEdges+1] = wFace1;
				++numEdges;
			}

#ifdef _USE_ADJACENCY
			if (adj1 == i || m_FaceData[adj1] == false)
#endif
			{
				// add edge v1-v2
				m_Edges[2*numEdges+0] = wFace1;
				m_Edges[2*numEdges+1] = wFace2;
				++numEdges;
			}

#ifdef _USE_ADJACENCY
			if (adj2 == i || m_FaceData[adj2] == false)
#endif
			{
				// add edge v2-v0
				m_Edges[2*numEdges+0] = wFace2;
				m_Edges[2*numEdges+1] = wFace0;
				++numEdges;
			}
		}
	}
	return numEdges;
}


void ShadowVolumeSceneNode::SetShadowMesh(const shared_ptr<IMesh>& mesh)
{
	if (m_ShadowMesh == mesh)
		return;

	m_ShadowMesh = mesh;
	if (m_ShadowMesh)
	{
		m_BBox = m_ShadowMesh->GetBoundingBox();
	}
}


void ShadowVolumeSceneNode::UpdateShadowVolumes(Scene *pScene)
{
	const u32 oldIndexCount = m_IndexCount;
	const u32 oldVertexCount = m_VertexCount;

	const IMesh* const mesh = m_ShadowMesh.get();
	if (!mesh)
		return;

	// create as much shadow volumes as there are lights but
	// do not ignore the max light settings.
	const u32 lightCount = pScene->GetRenderer()->GetDynamicLightCount();
	if (!lightCount)
		return;

	// calculate total amount of vertices and indices

	m_VertexCount = 0;
	m_IndexCount = 0;
	m_ShadowVolumesUsed = 0;

	u32 i;
	u32 totalVertices = 0;
	u32 totalIndices = 0;
	const u32 bufcnt = mesh->GetMeshBufferCount();

	for (i=0; i<bufcnt; ++i)
	{
		const IMeshBuffer* buf = mesh->GetMeshBuffer(i).get();
		totalIndices += buf->GetIndexCount();
		totalVertices += buf->GetVertexCount();
	}

	// copy mesh
	for (i=0; i<bufcnt; ++i)
	{
		const IMeshBuffer* buf = mesh->GetMeshBuffer(i).get();

		const u16* idxp = buf->GetIndices();
		const u16* idxpend = idxp + buf->GetIndexCount();
		for (; idxp!=idxpend; ++idxp)
			m_Indices[m_IndexCount++] = *idxp + m_VertexCount;

		const u32 vtxcnt = buf->GetVertexCount();
		for (u32 j=0; j<vtxcnt; ++j)
			m_Vertices[m_VertexCount++] = buf->GetPosition(j);
	}

	// recalculate adjacency if necessary
	if (oldVertexCount != m_VertexCount || oldIndexCount != m_IndexCount)
		CalculateAdjacency();

	matrix4 toWorld, fromWorld;
	m_pParent->Get()->Transform(&toWorld, &fromWorld);
	toWorld.MakeInverse();

	const Vector3f parentpos = m_pParent->Get()->ToWorld().GetTranslation();

	// TODO: Only correct for point lights.
	for (i=0; i<lightCount; ++i)
	{
		const Light& dl = pScene->GetRenderer()->GetDynamicLight(i);
		Vector3f lpos = dl.m_Position;
		if (dl.m_CastShadows &&
			fabs((lpos - parentpos).GetLengthSQ()) <= (dl.m_Radius*dl.m_Radius*4.0f))
		{
			toWorld.TransformVect(lpos);
			CreateShadowVolume(lpos);
		}
	}
}

//! pre render method
bool ShadowVolumeSceneNode::PreRender(Scene* pScene)
{
	if (Get()->IsVisible())
	{
		// register according to material types counted
		if (!pScene->IsCulled(this))
			pScene->AddToRenderQueue(ERP_SHADOW, shared_from_this());
	}

	return SceneNode::PreRender(pScene);
}

//! renders the node.
bool ShadowVolumeSceneNode::Render(Scene* pScene)
{
	const shared_ptr<IRenderer>& renderer = pScene->GetRenderer();

	if (!m_ShadowVolumesUsed || !renderer)
		return false;

	matrix4 toWorld, fromWorld;
	m_pParent->Get()->Transform(&toWorld, &fromWorld);

	renderer->SetTransform(ETS_WORLD, toWorld);

	for (u32 i=0; i<m_ShadowVolumesUsed; ++i)
	{
		bool drawShadow = true;

		if (m_UseZFailMethod && pScene->GetActiveCamera())
		{
			// Disable shadows drawing, when back cap is behind of ZFar plane.

			ViewFrustum frust(pScene->GetActiveCamera()->GetViewFrustum());

			matrix4 invTrans(toWorld, matrix4::EM4CONST_INVERSE);
			frust.Transform(invTrans);

			Vector3f edges[8];
			m_ShadowBBox[i].GetEdges(edges);

			Vector3f largestEdge = edges[0];
			f32 maxDistance = 
				Vector3f(pScene->GetActiveCamera()->Get()->ToWorld().GetTranslation() - edges[0]).GetLength();
			f32 curDistance = 0.f;

			for(int j = 1; j < 8; ++j)
			{
				curDistance = 
					Vector3f(pScene->GetActiveCamera()->Get()->ToWorld().GetTranslation()  - edges[j]).GetLength();

				if(curDistance > maxDistance)
				{
					maxDistance = curDistance;
					largestEdge = edges[j];
				}
			}

			if (!(frust.Planes[ViewFrustum::VF_FAR_PLANE].ClassifyPointRelation(largestEdge) != ISREL3D_FRONT))
				drawShadow = false;
		}

		if(!drawShadow)
		{
			eastl::vector<Vector3f> triangles;
			renderer->DrawStencilShadowVolume(triangles, m_UseZFailMethod, m_Props.DebugDataVisible());			
		}
		else renderer->DrawStencilShadowVolume(m_ShadowVolumes[i], m_UseZFailMethod, m_Props.DebugDataVisible());
	}
	return true;
}


//! returns the axis aligned bounding box of this node
const AABBox3<f32>& ShadowVolumeSceneNode::GetBoundingBox() const
{
	return m_BBox;
}


//! Generates adjacency information based on mesh indices.
void ShadowVolumeSceneNode::CalculateAdjacency()
{
	// go through all faces and fetch their three neighbours
	for (u32 f=0; f<m_IndexCount; f+=3)
	{
		for (u32 edge = 0; edge<3; ++edge)
		{
			const Vector3f& v1 = m_Vertices[m_Indices[f+edge]];
			const Vector3f& v2 = m_Vertices[m_Indices[f+((edge+1)%3)]];

			// now we search an_O_ther _F_ace with these two
			// vertices, which is not the current face.
			u32 of;

			for (of=0; of<m_IndexCount; of+=3)
			{
				// only other faces
				if (of != f)
				{
					bool cnt1 = false;
					bool cnt2 = false;

					for (s32 e=0; e<3; ++e)
					{
						if (v1.equal(m_Vertices[m_Indices[of+e]]))
							cnt1=true;

						if (v2.equal(m_Vertices[m_Indices[of+e]]))
							cnt2=true;
					}
					// one match for each vertex, i.e. edge is the same
					if (cnt1 && cnt2)
						break;
				}
			}

			// no adjacent edges -> store face number, else store adjacent face
			if (of >= m_IndexCount)
				m_Adjacency[f + edge] = f/3;
			else
				m_Adjacency[f + edge] = of/3;
		}
	}
}