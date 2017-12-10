// Copyright (C) 2002-2012 Nikolaus Gebhardt
// This file is part of the "Irrlicht Engine".
// For conditions of distribution and use, see copyright notice in irrlicht.h

#include "SceneNodeAnimatorFactory.h"
#include "SceneNodeAnimatorDelete.h"
#include "SceneNodeAnimatorTexture.h"
#include "SceneNodeAnimatorRotation.h"
#include "SceneNodeAnimatorFlyCircle.h"
#include "SceneNodeAnimatorFlyStraight.h"
#include "SceneNodeAnimatorFollowSpline.h"

#include "Scenes/SceneNodes.h"
#include "OS/os.h"

//! Names for scene node types
const c8* const SceneNodeAnimatorTypeNames[] =
{
	"flyCircle",
	"flyStraight",
	"followSpline",
	"rotation",
	"texture",
	"deletion",
	0
};


SceneNodeAnimatorFactory::SceneNodeAnimatorFactory(const shared_ptr<ICursorControl>& cursor)
: m_CursorControl(cursor)
{
	#ifdef _DEBUG
	//setDebugName("CDefaultSceneNodeAnimatorFactory");
	#endif

	// don't grab the scene manager here to prevent cyclic references
}


SceneNodeAnimatorFactory::~SceneNodeAnimatorFactory()
{

}


//! creates a scene node animator based on its type id
SceneNodeAnimator* SceneNodeAnimatorFactory::CreateSceneNodeAnimator(E_SCENE_NODE_ANIMATOR_TYPE type, Scene* pScene)
{
	SceneNodeAnimator* anim = 0;

	switch(type)
	{
		case ESNAT_FLY_CIRCLE:
			{
				float speed=0.001f;
				float radius=10.f;
				Vector3<float> direction(0.f, 1.f, 0.f);
				Vector3<float> center(0.f,0.f,0.f);
				float startPosition = 0.f;
				float radiusEllipsoid = 0.f;
				const float orbitDurationMs = (DEGTORAD * 360.f) / speed;
				const unsigned int effectiveTime = Timer::GetTime() + (unsigned int)(orbitDurationMs * startPosition);

				anim = new SceneNodeAnimatorFlyCircle(effectiveTime, center, radius, speed, direction, radiusEllipsoid);
			}
			break;
		case ESNAT_FLY_STRAIGHT:
			{
				anim = new SceneNodeAnimatorFlyStraight(
					Vector3<float>(100,0,60), Vector3<float>(-100,0,60), 3500, true, Timer::GetTime(), false );
			}
			break;
		case ESNAT_FOLLOW_SPLINE:
			{
				eastl::vector<Vector3<float>> points;
				points.push_back(Vector3<float>(0,0,0));
				points.push_back(Vector3<float>(10,5,10));
				anim = new SceneNodeAnimatorFollowSpline(0, points);
			}
			break;
		case ESNAT_ROTATION:
			{
				anim = new SceneNodeAnimatorRotation(Timer::GetTime(), Vector3<float>(0.8f,0.f,0.8f));
			}
			break;
		case ESNAT_TEXTURE:
			{
				eastl::vector<ITexture*> textures;
				anim = new SceneNodeAnimatorTexture(textures, 250, true, Timer::GetTime());
			}
			break;
		case ESNAT_DELETION:
			{
				anim = new SceneNodeAnimatorDelete(5000);
			}
			break;
		default:
			break;
	}

	return anim;
}


//! creates a scene node animator based on its type name
SceneNodeAnimator* SceneNodeAnimatorFactory::CreateSceneNodeAnimator(const c8* typeName, Scene* pScene)
{
	return CreateSceneNodeAnimator( GetTypeFromName(typeName), pScene );
}


//! returns amount of scene node animator types this factory is able to create
unsigned int SceneNodeAnimatorFactory::GetCreatableSceneNodeAnimatorTypeCount() const
{
	return ESNAT_COUNT;
}


//! returns type of a createable scene node animator type
E_SCENE_NODE_ANIMATOR_TYPE SceneNodeAnimatorFactory::GetCreateableSceneNodeAnimatorType(unsigned int idx) const
{
	if (idx<ESNAT_COUNT)
		return (E_SCENE_NODE_ANIMATOR_TYPE)idx;
	else
		return ESNAT_UNKNOWN;
}


//! returns type name of a createable scene node animator type
const c8* SceneNodeAnimatorFactory::GetCreateableSceneNodeAnimatorTypeName(unsigned int idx) const
{
	if (idx<ESNAT_COUNT)
		return SceneNodeAnimatorTypeNames[idx];
	else
		return 0;
}

//! returns type name of a createable scene node animator type
const c8* SceneNodeAnimatorFactory::GetCreateableSceneNodeAnimatorTypeName(E_SCENE_NODE_ANIMATOR_TYPE type) const
{
	// for this factory: index == type
	if (type<ESNAT_COUNT)
		return SceneNodeAnimatorTypeNames[type];
	else
		return 0;
}

E_SCENE_NODE_ANIMATOR_TYPE SceneNodeAnimatorFactory::GetTypeFromName(const c8* name) const
{
	for ( unsigned int i=0; SceneNodeAnimatorTypeNames[i]; ++i)
		if (!strcmp(name, SceneNodeAnimatorTypeNames[i]) )
			return (E_SCENE_NODE_ANIMATOR_TYPE)i;

	return ESNAT_UNKNOWN;
}

