// Copyright (C) 2002-2012 Nikolaus Gebhardt
// This file is part of the "Irrlicht Engine".
// For conditions of distribution and use, see copyright notice in irrlicht.h

#include "SceneNodeAnimatorTexture.h"

#include "GameEngine/GameEngine.h"
#include "OS/os.h"

#include "Graphics/ITexture.h"

//! constructor
SceneNodeAnimatorTexture::SceneNodeAnimatorTexture(
	const eastl::vector<ITexture*>& textures, int timePerFrame, bool loop, unsigned int now)
: m_FinishTime(0), m_HasFinished(false), m_TimePerFrame(timePerFrame), m_StartTime(now), m_Loop(loop)
{
	#ifdef _DEBUG
	//setDebugName("SceneNodeAnimatorTexture");
	#endif

	for (unsigned int i=0; i<textures.size(); ++i)
		m_Textures.push_back(textures[i]);

	m_FinishTime = now + (timePerFrame * m_Textures.size());
}


//! destructor
SceneNodeAnimatorTexture::~SceneNodeAnimatorTexture()
{
	ClearTextures();
}


void SceneNodeAnimatorTexture::ClearTextures()
{
	for (unsigned int i=0; i<m_Textures.size(); ++i)
		if (m_Textures[i])
			m_Textures[i] = 0;

	m_Textures.clear();
}


//! animates a scene node
void SceneNodeAnimatorTexture::AnimateNode(SceneNode* node, unsigned int timeMs)
{
	if(!node)
		return;

	if (m_Textures.size())
	{
		const unsigned int t = (timeMs-m_StartTime);

		unsigned int idx = 0;
		if (!m_Loop && timeMs >= m_FinishTime)
		{
			idx = m_Textures.size() - 1;
			m_HasFinished = true;
		}
		else
		{
			idx = (t/m_TimePerFrame) % m_Textures.size();
		}

		if (idx < m_Textures.size())
			node->SetMaterialTexture(0, m_Textures[idx]);
	}
}


SceneNodeAnimator* SceneNodeAnimatorTexture::CreateClone(SceneNode* node)
{
	SceneNodeAnimatorTexture * newAnimator =
		new SceneNodeAnimatorTexture(m_Textures, m_TimePerFrame, m_Loop, m_StartTime);

	return newAnimator;
}

