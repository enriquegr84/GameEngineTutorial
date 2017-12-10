// Copyright (C) 2002-2012 Nikolaus Gebhardt
// This file is part of the "Irrlicht Engine".
// For conditions of distribution and use, see copyright notice in irrlicht.h

#include "EmptyNode.h"

#include "Graphic/Scene/Hierarchy/Node.h"
#include "Graphic/Scene/Scene.h"

//! constructor
EmptyNode::EmptyNode(const ActorId actorId, WeakBaseRenderComponentPtr renderComponent)
:	Node(actorId, renderComponent, ERP_TRANSPARENT, ESNT_UNKNOWN)
{
	#ifdef _DEBUG
	//setDebugName("EmptySceneNode");
	#endif

	Get()->SetAutomaticCulling(EAC_OFF);
}


//! prerender
bool EmptyNode::PreRender(Scene *pScene)
{
	if (mProps.IsVisible())
	{
		// because this node supports rendering of mixed mode meshes consisting of
		// transparent and solid material at the same time, we need to go through all
		// materials, check of what type they are and register this node for the right
		// render pass according to that.
		const eastl::shared_ptr<Renderer>& renderer = pScene->GetRenderer();

		int transparentCount = 0;
		int solidCount = 0;

		// count transparent and solid materials in this scene node
		for (unsigned int i=0; i< GetMaterialCount(); ++i)
		{
			const eastl::shared_ptr<MaterialRenderer>& rnd =
				renderer->GetMaterialRenderer(GetMaterial(i).MaterialType);

			if (rnd && rnd->IsTransparent())
				++transparentCount;
			else
				++solidCount;
		}

		// register according to material types counted
		if (!pScene->IsCulled(this))
		{
			if (solidCount)
				pScene->AddToRenderQueue(ERP_SOLID, eastl::shared_from_this());

			if (transparentCount)
				pScene->AddToRenderQueue(ERP_TRANSPARENT, eastl::shared_from_this());
		}
	}

	return Node::PreRender(pScene);
}


//! render
bool EmptyNode::Render(Scene *pScene)
{
	// do nothing
	return Node::Render(pScene);
}


//! returns the axis aligned bounding box of this node
const AABBox3<float>& EmptyNode::GetBoundingBox() const
{
	return mBBox;
}