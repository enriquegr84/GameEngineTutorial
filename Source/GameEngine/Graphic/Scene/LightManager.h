// David Eberly, Geometric Tools, Redmond WA 98052
// Copyright (c) 1998-2017
// Distributed under the Boost Software License, Version 1.0.
// http://www.boost.org/LICENSE_1_0.txt
// http://www.geometrictools.com/License/Boost/LICENSE_1_0.txt
// File Version: 3.0.0 (2016/06/19)

#ifndef LIGHTMANAGER_H
#define LIGHTMANAGER_H

#include "Scene.h"

typedef eastl::list<eastl::shared_ptr<Light> > Lights;

class LightManager
{
	friend class Scene;

protected:
	/*
	void CreateScene();
	void UseLightType(int type);
	void UpdateConstants();
	
	eastl::shared_ptr<RasterizerState> mWireState;
	*/
	enum { LDIR, LPNT, LSPT, LNUM };
	enum { GPLN, GSPH, GNUM };
	enum { SVTX, SPXL, SNUM };
	eastl::shared_ptr<LightingEffect> mEffect[LNUM][GNUM][SNUM];
	eastl::shared_ptr<Visual> mPlane[SNUM], mSphere[SNUM];
	Vector4<float> mLightWorldPosition[2], mLightWorldDirection;
	eastl::string mCaption[LNUM];
	int mType;

	Lights mLights;
	/*
	Vector4 mLightDir[MAXIMUM_LIGHTS_SUPPORTED];
	Color mLightDiffuse[MAXIMUM_LIGHTS_SUPPORTED];
	Vector4 mLightAmbient;
	*/

public:

	LightManager();
	/*
	virtual void OnIdle() override;
	virtual bool OnCharPress(unsigned char key, int x, int y) override;
	*/
	void CalculateLighting(Scene *pScene);
	void CalculateLighting(Lighting* pLighting, Node *pNode);
	int GetLightCount(const Node *node) { return mLights.size(); }
	/*
	const Vector4* GetLightAmbient(const Node *node) { return &mLightAmbient; }
	const Vector4* GetLightDirection(const Node *node) { return mLightDir; }
	const Color* GetLightDiffuse(const Node *node) { return mLightDiffuse; }
	*/
};

#endif
