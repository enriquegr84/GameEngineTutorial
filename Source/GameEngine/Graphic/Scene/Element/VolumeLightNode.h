// Copyright (C) 2002-2012 Nikolaus Gebhardt
// This file is part of the "Irrlicht Engine".
// For conditions of distribution and use, see copyright notice in irrlicht.h
//
// created by Dean Wadsworth aka Varmint Dec 31 2007

#ifndef VOLUMELIGHTNODE_H
#define VOLUMELIGHTNODE_H

#include "Graphic/Scene/Hierarchy/Node.h"

class GRAPHIC_ITEM VolumeLightNode : public Node
{
public:

	//! constructor
	VolumeLightNode(const ActorId actorId, PVWUpdater* updater, 
		WeakBaseRenderComponentPtr renderComponent,
		const unsigned int subdivU = 32, const unsigned int subdivV = 32,
		const eastl::array<float, 4> foot = eastl::array<float, 4>{51.f, 0.f, 230.f, 180.f},
		const eastl::array<float, 4> tail = eastl::array<float, 4>{0.f, 0.f, 0.f, 0.f});

	~VolumeLightNode();

	//! Renders event
	virtual bool PreRender(Scene *pScene);
	virtual bool Render(Scene *pScene);

	//! returns the material based on the zero based index i.
	virtual eastl::shared_ptr<Material> const& GetMaterial(unsigned int i);

	//! returns amount of materials used by this scene node.
	virtual unsigned int GetMaterialCount() const;

	//! Returns type of the scene node
	virtual NodeType GetType() const { return NT_VOLUME_LIGHT; }

	//! Sets the number of segments across the U axis
	void SetSubDivideU(const unsigned int inU);

	//! Sets the number of segments across the V axis
	void SetSubDivideV(const unsigned int inV);

	//! Returns the number of segments across the U axis
	unsigned int GetSubDivideU() const { return mSubdivideU; }

	//! Returns the number of segments across the V axis
	unsigned int GetSubDivideV() const { return mSubdivideV; }

	//! Sets the color of the base of the light
	void SetFootColor(const eastl::array<float, 4> inColor);

	//! Sets the color of the tip of the light
	void SetTailColor(const eastl::array<float, 4> inColor);

	//! Returns the color of the base of the light
	eastl::array<float, 4> GetFootColor() const { return mFootColor; }

	//! Returns the color of the tip of the light
	eastl::array<float, 4> GetTailColor() const { return mTailColor; }

private:

	void ConstructLight();

	eastl::shared_ptr<BaseMesh> mMesh;

	float  mLPDistance;		// Distance to hypothetical lightsource point -- affects fov angle

	unsigned int  mSubdivideU;		// Number of subdivisions in U and V space.
	unsigned int  mSubdivideV;		// Controls the number of "slices" in the volume.
	// NOTE : Total number of polygons = 2 + ((SubdivideU + 1) + (SubdivideV + 1)) * 2
	// Each slice being a quad plus the rectangular plane at the bottom.

	eastl::array<float, 4> mFootColor;	// Color at the source
	eastl::array<float, 4> mTailColor;	// Color at the end.

	// LightDimensions.Y Distance of shooting -- Length of beams
	// LightDimensions.X and LightDimensions.Z determine the
	// size/dimension of the plane
	Vector3<float> mLightDimensions;
};

#endif

