// Copyright (C) 2002-2012 Nikolaus Gebhardt
// This file is part of the "Irrlicht Engine".
// For conditions of distribution and use, see copyright notice in irrlicht.h

#ifndef _SHADOWVOLUMESCENENODE_H_INCLUDED_
#define _SHADOWVOLUMESCENENODE_H_INCLUDED_

#include "Scenes/SceneNodes.h"

#include "Scenes/Mesh/IMesh.h"

//! Scene node for rendering a shadow volume into a stencil buffer.
class ShadowVolumeSceneNode : public SceneNode
{
public:

	//! constructor
	ShadowVolumeSceneNode( const ActorId actorId, WeakBaseRenderComponentPtr renderComponent, 
		matrix4 const *t, const shared_ptr<IMesh>& shadowMesh, bool zfailmethod=true, f32 infinity=10000.0f );

	//! destructor
	virtual ~ShadowVolumeSceneNode(){ }

	//! Sets the mesh from which the shadow volume should be generated.
	/** To optimize shadow rendering, use a simpler mesh for shadows.
	*/
	virtual void SetShadowMesh(const shared_ptr<IMesh>& mesh);

	//! Render events
	virtual bool PreRender(Scene *pScene);
	virtual bool Render(Scene *pScene);

	//! Updates the shadow volumes for current light positions.
	/** Called each render cycle from Animated Mesh SceneNode render method. */
	virtual void UpdateShadowVolumes(Scene *pScene);

	//! returns the axis aligned bounding box of this node
	virtual const AABBox3<f32>& GetBoundingBox() const;


private:

	typedef eastl::vector<Vector3f> ShadowVolume;

	void CreateShadowVolume(const Vector3f& pos, bool isDirectional=false);
	u32 CreateEdgesAndCaps(const Vector3f& light, ShadowVolume* svp, AABBox3<f32>* bb);

	//! Generates adjacency information based on mesh indices.
	void CalculateAdjacency();

	AABBox3<f32> m_BBox;

	// a shadow volume for every light
	eastl::vector<ShadowVolume> m_ShadowVolumes;

	// a back cap bounding box for every light
	eastl::vector<AABBox3<f32> > m_ShadowBBox;

	eastl::vector<Vector3f> m_Vertices;
	eastl::vector<u16> m_Indices;
	eastl::vector<u16> m_Adjacency;
	eastl::vector<u16> m_Edges;
	// tells if face is front facing
	eastl::vector<bool> m_FaceData;

	shared_ptr<IMesh> m_ShadowMesh;

	u32 m_IndexCount;
	u32 m_VertexCount;
	u32 m_ShadowVolumesUsed;

	f32 m_Infinity;

	bool m_UseZFailMethod;
};

#endif
