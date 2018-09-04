// Copyright (C) 2002-2012 Nikolaus Gebhardt
// This file is part of the "Irrlicht Engine".
// For conditions of distribution and use, see copyright notice in irrlicht.h

#ifndef SKYDOMENODE_H
#define SKYDOMENODE_H

#include "Graphic/Scene/Hierarchy/Node.h"

/* horiRes and vertRes:
Controls the number of faces along the horizontal axis (30 is a good value)
and the number of faces along the vertical axis (8 is a good value).

texturePercentage:
Only the top texturePercentage of the image is used, e.g. 0.8 uses the top 80% of the image,
1.0 uses the entire image. This is useful as some landscape images have a small banner
at the bottom that you don't want.

spherePercentage:
This controls how far around the sphere the sky dome goes. For value 1.0 you get exactly the upper
hemisphere, for 1.1 you get slightly more, and for 2.0 you get a full sphere. It is sometimes useful
to use a value slightly bigger than 1 to avoid a gap between some ground place and the sky. This
parameters stretches the image to fit the chosen "sphere-size".
*/

// Skydome, rendered with zbuffer turned off, before all other nodes.
class SkyDomeNode : public Node
{
public:

	//! constructor
	SkyDomeNode(const ActorId actorId, PVWUpdater* updater, WeakBaseRenderComponentPtr renderComponent,
			const eastl::shared_ptr<Texture2>& sky, unsigned int horiRes, unsigned int vertRes,
			float texturePercentage, float spherePercentage, float radius);

	//! Render events
	virtual bool PreRender(Scene *pScene);
	virtual bool Render(Scene *pScene);

	//! returns the material based on the zero based index i. To get the amount
	//! of materials used by this scene node, use getMaterialCount().
	//! This function is needed for inserting the node into the scene hirachy on a
	//! optimal position for minimizing renderstate changes, but can also be used
	//! to directly modify the material of a scene node.
	virtual eastl::shared_ptr<Material> const& GetMaterial(unsigned int i);

	//! returns amount of materials used by this scene node.
	virtual unsigned int GetMaterialCount() const;

	//! Returns type of the scene node
	virtual NodeType GetType() const { return NT_SKY; }

private:

	void GenerateMesh(const eastl::shared_ptr<Texture2>& sky);

	eastl::shared_ptr<Visual> mVisual;

	unsigned int mHorizontalResolution, mVerticalResolution;
	float mTexturePercentage, mSpherePercentage, mRadius;
};


#endif

