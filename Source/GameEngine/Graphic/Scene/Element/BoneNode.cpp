// Copyright (C) 2002-2012 Nikolaus Gebhardt
// This file is part of the "Irrlicht Engine".
// For conditions of distribution and use, see copyright notice in irrlicht.h

#include "BoneNode.h"

//! constructor
BoneNode::BoneNode(const ActorId actorId, PVWUpdater* updater, 
	WeakBaseRenderComponentPtr renderComponent, 
	unsigned int boneIndex, const char* boneName)
: Node(actorId, renderComponent, NT_ANIMATED_MESH),
	mBoneIndex(boneIndex), mAnimationMode(BAM_AUTOMATIC), mSkinningSpace(BSS_LOCAL),
	mPositionHint(-1), mScaleHint(-1), mRotationHint(-1)
{
	#ifdef _DEBUG
	//setDebugName("BoneSceneNode");
	#endif
	SetName(boneName);
}


//! Returns the index of the bone
unsigned int BoneNode::GetBoneIndex() const
{
	return mBoneIndex;
}


//! Sets the animation mode of the bone. Returns true if successful.
bool BoneNode::SetAnimationMode(BoneAnimationMode mode)
{
	mAnimationMode = mode;
	return true;
}


//! Gets the current animation mode of the bone
BoneAnimationMode BoneNode::GetAnimationMode() const
{
	return mAnimationMode;
}

void BoneNode::UpdateAbsoluteTransformationChildren(eastl::shared_ptr<Node> node)
{
	node->UpdateAbsoluteTransform();

	SceneNodeList::const_iterator it = node->GetChildren().begin();
	for (; it != node->GetChildren().end(); ++it)
		UpdateAbsoluteTransformationChildren((*it));
}


void BoneNode::UpdateAbsoluteTransformationChildren()
{
	UpdateAbsoluteTransformationChildren( shared_from_this() );
}