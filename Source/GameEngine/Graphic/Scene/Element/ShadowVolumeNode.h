// Copyright (C) 2002-2012 Nikolaus Gebhardt
// This file is part of the "Irrlicht Engine".
// For conditions of distribution and use, see copyright notice in irrlicht.h

#ifndef SHADOWVOLUMENODE_H
#define SHADOWVOLUMENODE_H

#include "Graphic/Scene/Hierarchy/Node.h"

//! Scene node for rendering a shadow volume into a stencil buffer.
class ShadowVolumeNode : public Node
{
public:

	//! constructor
	ShadowVolumeNode( const ActorId actorId, PVWUpdater* updater, WeakBaseRenderComponentPtr renderComponent,
		const eastl::shared_ptr<BaseMesh>& shadowMesh, bool zfailmethod=true, float infinity=10000.0f );

	//! destructor
	~ShadowVolumeNode(){ }

	//! Sets the mesh from which the shadow volume should be generated.
	/** To optimize shadow rendering, use a simpler mesh for shadows.
	*/
	void SetShadowMesh(const eastl::shared_ptr<BaseMesh>& mesh);

	//! Render events
	virtual bool PreRender(Scene *pScene);
	virtual bool Render(Scene *pScene);

	//! Updates the shadow volumes for current light positions.
	/** Called each render cycle from Animated Mesh SceneNode render method. */
	void UpdateShadowVolumes(Scene *pScene);


private:

	typedef eastl::vector<Vector3<float>> ShadowVolume;

	void CreateShadowVolume(const Vector3<float>& pos, bool isDirectional=false);
	unsigned int CreateEdgesAndCaps(const Vector3<float>& light, ShadowVolume* svp, AlignedBox3<float>* bb);

	//! Generates adjacency information based on mesh indices.
	void CalculateAdjacency();

	// a shadow volume for every light
	eastl::vector<ShadowVolume> mShadowVolumes;

	// a back cap bounding box for every light
	eastl::vector<AlignedBox3<float>> mShadowBBox;

	eastl::vector<Vector3<float>> mVertices;
	eastl::vector<unsigned int> mIndices;
	eastl::vector<unsigned int> mAdjacency;
	eastl::vector<unsigned int> mEdges;
	// tells if face is front facing
	eastl::vector<bool> mFaceData;

	eastl::vector<eastl::shared_ptr<Visual>> mVisuals;
	eastl::shared_ptr<BaseMesh> mShadowMesh;

	unsigned int mIndexCount;
	unsigned int mVertexCount;
	unsigned int mShadowVolumesUsed;

	float mInfinity;

	bool mUseZFailMethod;
};

#endif
