// Copyright (C) 2002-2012 Nikolaus Gebhardt
// This file is part of the "Irrlicht Engine".
// For conditions of distribution and use, see copyright notice in irrlicht.h

#ifndef _IMESHLOADER_H_INCLUDED_
#define _IMESHLOADER_H_INCLUDED_

#include "Utilities/path.h"

#include "GameEngine\interfaces.h"
#include "ResourceCache\ResCache.h"
#include "IAnimatedMesh.h"

class MeshResourceExtraData : public IResourceExtraData
{

public:
    virtual stringc ToString() { return "MeshResourceExtraData"; }
	virtual IAnimatedMesh* GetMesh() { return m_pAnimatedMesh; }
	virtual void SetMesh(IAnimatedMesh* aMesh) { m_pAnimatedMesh = aMesh; }
	virtual ~MeshResourceExtraData() { SAFE_DELETE(m_pAnimatedMesh); }

protected:
	IAnimatedMesh* m_pAnimatedMesh;

};


////////////////////////////////////////////////////
//
// D3DMeshNode9 Description
//
//    Attaches a ID3DXMesh Mesh object to the GameEngine scene graph
//    This is a creature of D3D9, and not described in the 4th edition.
//    In the 3rd edition this was called D3DMeshNode
//
////////////////////////////////////////////////////
/*
class D3DMeshNode9 : public D3DSceneNode9
{
protected:
	ID3DXMesh *m_pMesh;
	stringc m_XFileName;

public:
	D3DMeshNode9(const ActorId actorId, 
		WeakBaseRenderComponentPtr renderComponent,
		ID3DXMesh *Mesh, 
		RenderPass renderPass, 
		const matrix4 *t);

	D3DMeshNode9(const ActorId actorId, 
		WeakBaseRenderComponentPtr renderComponent,
		stringc xFileName, 
		RenderPass renderPass,  
		const matrix4 *t); 

	virtual ~D3DMeshNode9() { SAFE_RELEASE(m_pMesh); }
	bool VRender(Scene *pScene);
	virtual bool VOnRestore(Scene *pScene);
	virtual bool VPick(Scene *pScene, RayCast *pRayCast);
	
	float CalcBoundingSphere();
};


////////////////////////////////////////////////////
//
// D3DShaderMeshNode9 Description
//
//    Attaches a ID3DXMesh Mesh object to the GameEngine scene graph and renders it with a shader.
//    This is a creature of D3D9, and not described in the 4th edition.
//    In the 3rd edition this was called D3DShaderMeshNode
//
////////////////////////////////////////////////////

class D3DShaderMeshNode9 : public D3DMeshNode9
{
protected:
	ID3DXEffect *m_pEffect;
	stringc m_fxFileName;

public:
	D3DShaderMeshNode9(const ActorId actorId, 
		WeakBaseRenderComponentPtr renderComponent,
		ID3DXMesh *Mesh, 
		stringc fxFileName,
		RenderPass renderPass, 
		const matrix4 *t); 

	D3DShaderMeshNode9(const ActorId actorId,
		WeakBaseRenderComponentPtr renderComponent,
		stringc xFileName, 
		stringc fxFileName,
		RenderPass renderPass, 
		const matrix4 *t); 

	virtual ~D3DShaderMeshNode9() { SAFE_RELEASE(m_pEffect); }
	virtual bool VOnRestore(Scene *pScene);
	virtual bool VOnLostDevice(Scene *pScene);
	bool VRender(Scene *pScene);
};

////////////////////////////////////////////////////
//
// class TeapotMeshNode9				- Chapter X, page Y
//
////////////////////////////////////////////////////

class D3DTeapotMeshNode9 : public D3DShaderMeshNode9
{
public:
	D3DTeapotMeshNode9(const ActorId actorId, WeakBaseRenderComponentPtr renderComponent, stringc fxFileName, RenderPass renderPass, const matrix4 *t);
	virtual bool VOnRestore(Scene *pScene);
};


class D3DShaderMeshNode11 : public SceneNode
{
public:
	D3DShaderMeshNode11(const ActorId actorId, 
		WeakBaseRenderComponentPtr renderComponent,
		stringc sdkMeshFileName, 
		RenderPass renderPass, 
		const matrix4 *t);

	virtual bool VOnRestore(Scene *pScene);
	virtual bool VOnLostDevice(Scene *pScene) { return S_OK; }
	virtual bool VRender(Scene *pScene);
	virtual bool VPick(Scene *pScene, RayCast *pRayCast);

protected:
	stringc					m_sdkMeshFileName;

	GameEngine_Hlsl_VertexShader		m_VertexShader;
	GameEngine_Hlsl_PixelShader		m_PixelShader;

	float CalcBoundingSphere(CDXUTSDKMesh *mesh11);			// this was added post press.
};


class D3DTeapotMeshNode11 : public D3DShaderMeshNode11
{
public:
	D3DTeapotMeshNode11(const ActorId actorId, WeakBaseRenderComponentPtr renderComponent, RenderPass renderPass,const matrix4 *t)
		: D3DShaderMeshNode11(actorId, renderComponent, "art\\teapot.sdkmesh", renderPass, t) { }
	bool VIsVisible() { return true; }
};
*/
#endif

