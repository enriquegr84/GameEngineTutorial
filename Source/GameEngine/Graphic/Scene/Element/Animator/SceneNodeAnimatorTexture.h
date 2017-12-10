// Copyright (C) 2002-2012 Nikolaus Gebhardt
// This file is part of the "Irrlicht Engine".
// For conditions of distribution and use, see copyright notice in irrlicht.h

#ifndef _SCENENODEANIMATORTEXTURE_H_INCLUDED_
#define _SCENENODEANIMATORTEXTURE_H_INCLUDED_

#include "Scenes/SceneNodes.h"

class SceneNodeAnimatorTexture : public SceneNodeAnimator
{
public:

	//! constructor
	SceneNodeAnimatorTexture(const eastl::vector<ITexture*>& textures,
			int timePerFrame, bool loop, unsigned int now);

	//! destructor
	virtual ~SceneNodeAnimatorTexture();

	//! animates a scene node
	virtual void AnimateNode(SceneNode* node, unsigned int timeMs);

	//! Returns type of the scene node animator
	virtual E_SCENE_NODE_ANIMATOR_TYPE GetType() const { return ESNAT_TEXTURE; }

	virtual bool HasFinished(void) const { return m_HasFinished; }

	//! Creates a clone of this animator.
	virtual SceneNodeAnimator* CreateClone(SceneNode* node);

protected:

	unsigned int		m_FinishTime;
	bool	m_HasFinished;

private:

	void ClearTextures();

	eastl::vector<ITexture*> m_Textures;
	unsigned int m_TimePerFrame;
	unsigned int m_StartTime;
	bool m_Loop;
};

#endif

