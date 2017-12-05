// David Eberly, Geometric Tools, Redmond WA 98052
// Copyright (c) 1998-2017
// Distributed under the Boost Software License, Version 1.0.
// http://www.boost.org/LICENSE_1_0.txt
// http://www.geometrictools.com/License/Boost/LICENSE_1_0.txt
// File Version: 3.0.0 (2016/06/19)

#include "Light.h"

//----------------------------------------------------------------------------
Light::Light(bool isPerspective, bool isDepthRangeZeroOne)
    : ViewVolume(isPerspective, isDepthRangeZeroOne)
{

}

/*
LightNode::LightNode(const ActorId actorId, WeakBaseRenderComponentPtr renderComponent, const LightProperties &props, const Mat4x4 *t)
	: SceneNode(actorId, renderComponent, RenderPass_NotRendered, t)
{
	m_LightProps = props;
}

HRESULT D3DLightNode9::VOnRestore(Scene *pScene)
{
	ZeroMemory(&m_d3dLight9, sizeof(D3DLIGHT9));
	m_d3dLight9.Type = D3DLIGHT_DIRECTIONAL;

	// These parameters are constant for the list after the scene is loaded
	m_d3dLight9.Range = m_LightProps.m_Range;
	m_d3dLight9.Falloff = m_LightProps.m_Falloff;
	m_d3dLight9.Attenuation0 = m_LightProps.m_Attenuation[0];
	m_d3dLight9.Attenuation1 = m_LightProps.m_Attenuation[0];
	m_d3dLight9.Attenuation2 = m_LightProps.m_Attenuation[0];
	m_d3dLight9.Theta = m_LightProps.m_Theta;
	m_d3dLight9.Phi = m_LightProps.m_Phi;

	return S_OK;
}


HRESULT D3DLightNode9::VOnUpdate(Scene *, DWORD const elapsedMs)
{
	// light color can change anytime! Check the BaseRenderComponent!
	LightRenderComponent* lrc = static_cast<LightRenderComponent*>(m_RenderComponent);
	m_Props.GetMaterial().SetDiffuse(lrc->GetColor());

	m_d3dLight9.Diffuse = m_Props.GetMaterial().GetDiffuse();
	float power;
	Color spec;
	m_Props.GetMaterial().GetSpecular(spec, power);
	m_d3dLight9.Specular = spec;
	m_d3dLight9.Ambient = m_Props.GetMaterial().GetAmbient();

	m_d3dLight9.Position = GetPosition();
	m_d3dLight9.Direction = GetDirection();
	return S_OK;
}

HRESULT D3DLightNode11::VOnUpdate(Scene *, DWORD const elapsedMs)
{
	// light color can change anytime! Check the BaseRenderComponent!
	LightRenderComponent* lrc = static_cast<LightRenderComponent*>(m_RenderComponent);
	m_Props.GetMaterial().SetDiffuse(lrc->GetColor());
	return S_OK;
}
*/
//----------------------------------------------------------------------------
