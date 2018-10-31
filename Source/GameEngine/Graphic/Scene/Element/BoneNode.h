// Copyright (C) 2002-2012 Nikolaus Gebhardt
// This file is part of the "Irrlicht Engine".
// For conditions of distribution and use, see copyright notice in irrlicht.h

#ifndef BONENODE_H
#define BONENODE_H

#include "Graphic/Scene/Hierarchy/Node.h"

// Used with SkinnedMesh and AnimatedMeshNode, for boned meshes

//! Enumeration for different bone animation modes
enum BoneAnimationMode
{
	//! The bone is usually animated, unless it's parent is not animated
	BAM_AUTOMATIC = 0,

	//! The bone is animated by the skin, if it's parent is not animated then animation will resume from this bone onward
	BAM_ANIMATED,

	//! The bone is not animated by the skin
	BAM_UNANIMATED,

	//! Not an animation mode, just here to count the available modes
	BAM_COUNT
};

enum BoneSkinningSpace
{
	//! local skinning, standard
	BSS_LOCAL = 0,

	//! global skinning
	BSS_GLOBAL,

	BSS_COUNT
};

class BoneNode : public Node
{
public:

	//! constructor
	BoneNode(const ActorId actorId, PVWUpdater* updater, WeakBaseRenderComponentPtr renderComponent,
			unsigned int boneIndex=0, const char* boneName=0);

	//! Returns the index of the bone
	virtual unsigned int GetBoneIndex() const;

	//! Sets the animation mode of the bone. Returns true if successful.
	virtual bool SetAnimationMode(BoneAnimationMode mode);

	//! Gets the current animation mode of the bone
	virtual BoneAnimationMode GetAnimationMode() const;

	virtual void UpdateAbsoluteTransformationChildren();

	//! How the relative transformation of the bone is used
	virtual void SetSkinningSpace(BoneSkinningSpace space )
	{
		mSkinningSpace=space;
	}

	virtual BoneSkinningSpace GetSkinningSpace() const
	{
		return mSkinningSpace;
	}

	int mPositionHint;
	int mScaleHint;
	int mRotationHint;

private:
	void UpdateAbsoluteTransformationChildren(eastl::shared_ptr<Node> node);

	unsigned int mBoneIndex;

	BoneAnimationMode mAnimationMode;
	BoneSkinningSpace mSkinningSpace;
};

#endif

