// David Eberly, Geometric Tools, Redmond WA 98052
// Copyright (c) 1998-2018
// Distributed under the Boost Software License, Version 1.0.
// http://www.boost.org/LICENSE_1_0.txt
// http://www.geometrictools.com/License/Boost/LICENSE_1_0.txt
// File Version: 3.0.0 (2016/06/19)

#include "ViewVolumeNode.h"

#include "Graphic/Renderer/Renderer.h"
#include "Graphic/Scene/Scene.h"

ViewVolumeNode::ViewVolumeNode(const ActorId actorid, eastl::shared_ptr<ViewVolume> const& viewVolume)
    : Node(actorid, WeakBaseRenderComponentPtr(), RP_NONE, NT_LIGHT),  mOnUpdate([](ViewVolumeNode*){})
{
    SetViewVolume(viewVolume);
}

//! prerender
bool ViewVolumeNode::PreRender(Scene *pScene)
{
	if (IsVisible())
	{
		// because this node supports rendering of mixed mode meshes consisting of
		// transparent and solid material at the same time, we need to go through all
		// materials, check of what type they are and register this node for the right
		// render pass according to that.
		int transparentCount = 0;
		int solidCount = 0;

		// count transparent and solid materials in this scene node
		if (GetMaterial().IsTransparent())
			++transparentCount;
		else
			++solidCount;

		// register according to material types counted
		if (!pScene->IsCulled(this))
		{
			if (solidCount)
				pScene->AddToRenderQueue(RP_SOLID, shared_from_this());

			if (transparentCount)
				pScene->AddToRenderQueue(RP_TRANSPARENT, shared_from_this());
		}
	}

	return Node::PreRender(pScene);
}


//! render
bool ViewVolumeNode::Render(Scene *pScene)
{
	// do nothing
	return Node::Render(pScene);
}

void ViewVolumeNode::SetViewVolume(eastl::shared_ptr<ViewVolume> const& viewVolume)
{
    mViewVolume = viewVolume;

    if (mViewVolume)
    {
        Matrix4x4<float> rotate;
#if defined(GE_USE_MAT_VEC)
        rotate.SetCol(0, mViewVolume->GetDVector());
        rotate.SetCol(1, mViewVolume->GetUVector());
        rotate.SetCol(2, mViewVolume->GetRVector());
        rotate.SetCol(3, { 0.0f, 0.0f, 0.0f, 1.0f });
#else
        rotate.SetRow(0, mViewVolume->GetDVector());
        rotate.SetRow(1, mViewVolume->GetUVector());
        rotate.SetRow(2, mViewVolume->GetRVector());
        rotate.SetRow(3, { 0.0f, 0.0f, 0.0f, 1.0f });
#endif
        GetRelativeTransform().SetTranslation(mViewVolume->GetPosition());
		GetRelativeTransform().SetRotation(rotate);
        Update();
    }
}

void ViewVolumeNode::UpdateWorldData(double applicationTIme)
{
    Node::UpdateWorldData(applicationTIme);

    if (mViewVolume)
    {
        Vector4<float> position = GetAbsoluteTransform().GetTranslationW1();

        Matrix4x4<float> const& rotate = GetAbsoluteTransform().GetHMatrix();
#if defined(GE_USE_MAT_VEC)
        Vector4<float> dVector = rotate.GetCol(0);
        Vector4<float> uVector = rotate.GetCol(1);
        Vector4<float> rVector = rotate.GetCol(2);
#else
        Vector4<float> dVector = rotate.GetRow(0);
        Vector4<float> uVector = rotate.GetRow(1);
        Vector4<float> rVector = rotate.GetRow(2);
#endif
        mViewVolume->SetFrame(position, dVector, uVector, rVector);
        mOnUpdate(this);
    }
}
