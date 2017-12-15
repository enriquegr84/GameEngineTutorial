// Copyright (C) 2002-2012 Nikolaus Gebhardt
// This file is part of the "Irrlicht Engine".
// For conditions of distribution and use, see copyright notice in irrlicht.h

#include "SkyBoxNode.h"
#include "CameraNode.h"

#include "Core/OS/OS.h"

#include "Graphic/Scene/Scene.h"

//! constructor
SkyBoxNode::SkyBoxNode(const ActorId actorId, WeakBaseRenderComponentPtr renderComponent, 
	const eastl::shared_ptr<Texture2>& top, const eastl::shared_ptr<Texture2>& bottom, const eastl::shared_ptr<Texture2>& left,
	const eastl::shared_ptr<Texture2>& right, const eastl::shared_ptr<Texture2>& front, const eastl::shared_ptr<Texture2>& back)
:	Node(actorId, renderComponent, ERP_SKY_BOX, ESNT_SKY_BOX)
{
	#ifdef _DEBUG
	//setDebugName("SkyBoxSceneNode");
	#endif

	Get()->SetAutomaticCulling(EAC_OFF);
	mBBox.MaxEdge.set(0,0,0);
	mBBox.MinEdge.set(0,0,0);

	// create indices

	mIndices[0] = 0;
	mIndices[1] = 1;
	mIndices[2] = 2;
	mIndices[3] = 3;

	// create material

	Material mat;
	mat.Lighting = false;
	mat.ZBuffer = ECFN_DISABLED;
	mat.ZWriteEnable = false;
	mat.AntiAliasing=0;
	mat.TextureLayer[0].TextureWrapU = ETC_CLAMP_TO_EDGE;
	mat.TextureLayer[0].TextureWrapV = ETC_CLAMP_TO_EDGE;

	/* Hey, I am no artist, but look at that
	   cool ASCII art I made! ;)

       -111         111
          /6--------/5        y
         /  |      / |        ^  z
        /   |   11-1 |        | /
  -11-1 3---------2  |        |/
        |   7- - -| -4 1-11    *---->x
        | -1-11   |  /       3-------|2
        |/        | /         |    //|
        0---------1/          |  //  |
     -1-1-1     1-1-1         |//    |
	                     0--------1
	*/

	eastl::shared_ptr<Texture2> tex = front;
	if (!tex) tex = left;
	if (!tex) tex = back;
	if (!tex) tex = right;
	if (!tex) tex = top;
	if (!tex) tex = bottom;

	const float onepixel = tex?(1.0f / (tex->GetSize().Width * 1.5f)) : 0.0f;
	const float t = 1.0f - onepixel;
	const float o = 0.0f + onepixel;

	// create front side

	mMaterials[0] = mat;
	mMaterials[0].SetTexture(0, front.get());
	mVertices[0] = VertexBuffer(-1, -1, -1, 0, 0, 1, eastl::array<float, 4>{255.f, 255.f, 255.f, 255.f}, t, t);
	mVertices[1] = VertexBuffer(1, -1, -1, 0, 0, 1, eastl::array<float, 4>{255.f, 255.f, 255.f, 255.f}, o, t);
	mVertices[2] = VertexBuffer(1, 1, -1, 0, 0, 1, eastl::array<float, 4>{255.f, 255.f, 255.f, 255.f}, o, o);
	mVertices[3] = VertexBuffer(-1, 1, -1, 0, 0, 1, eastl::array<float, 4>{255.f, 255.f, 255.f, 255.f}, t, o);

	// create left side

	mMaterials[1] = mat;
	mMaterials[1].SetTexture(0, left.get());
	mVertices[4] = VertexBuffer( 1,-1,-1, -1,0,0, eastl::array<float, 4>{255.f, 255.f, 255.f, 255.f}, t, t);
	mVertices[5] = VertexBuffer( 1,-1, 1, -1,0,0, eastl::array<float, 4>{255.f, 255.f, 255.f, 255.f}, o, t);
	mVertices[6] = VertexBuffer( 1, 1, 1, -1,0,0, eastl::array<float, 4>{255.f, 255.f, 255.f, 255.f}, o, o);
	mVertices[7] = VertexBuffer( 1, 1,-1, -1,0,0, eastl::array<float, 4>{255.f, 255.f, 255.f, 255.f}, t, o);

	// create back side

	mMaterials[2] = mat;
	mMaterials[2].SetTexture(0, back.get());
	mVertices[8]  = VertexBuffer( 1,-1, 1, 0,0,-1, eastl::array<float, 4>{255.f, 255.f, 255.f, 255.f}, t, t);
	mVertices[9]  = VertexBuffer(-1,-1, 1, 0,0,-1, eastl::array<float, 4>{255.f, 255.f, 255.f, 255.f}, o, t);
	mVertices[10] = VertexBuffer(-1, 1, 1, 0,0,-1, eastl::array<float, 4>{255.f, 255.f, 255.f, 255.f}, o, o);
	mVertices[11] = VertexBuffer( 1, 1, 1, 0,0,-1, eastl::array<float, 4>{255.f, 255.f, 255.f, 255.f}, t, o);

	// create right side

	mMaterials[3] = mat;
	mMaterials[3].SetTexture(0, right.get());
	mVertices[12] = VertexBuffer(-1,-1, 1, 1,0,0, eastl::array<float, 4>{255.f, 255.f, 255.f, 255.f}, t, t);
	mVertices[13] = VertexBuffer(-1,-1,-1, 1,0,0, eastl::array<float, 4>{255.f, 255.f, 255.f, 255.f}, o, t);
	mVertices[14] = VertexBuffer(-1, 1,-1, 1,0,0, eastl::array<float, 4>{255.f, 255.f, 255.f, 255.f}, o, o);
	mVertices[15] = VertexBuffer(-1, 1, 1, 1,0,0, eastl::array<float, 4>{255.f, 255.f, 255.f, 255.f}, t, o);

	// create top side

	mMaterials[4] = mat;
	mMaterials[4].SetTexture(0, top.get());
	mVertices[16] = VertexBuffer( 1, 1,-1, 0,-1,0, eastl::array<float, 4>{255.f, 255.f, 255.f, 255.f}, t, t);
	mVertices[17] = VertexBuffer( 1, 1, 1, 0,-1,0, eastl::array<float, 4>{255.f, 255.f, 255.f, 255.f}, o, t);
	mVertices[18] = VertexBuffer(-1, 1, 1, 0,-1,0, eastl::array<float, 4>{255.f, 255.f, 255.f, 255.f}, o, o);
	mVertices[19] = VertexBuffer(-1, 1,-1, 0,-1,0, eastl::array<float, 4>{255.f, 255.f, 255.f, 255.f}, t, o);

	// create bottom side

	mMaterials[5] = mat;
	mMaterials[5].SetTexture(0, bottom.get());
	mVertices[20] = VertexBuffer( 1,-1, 1, 0,1,0, eastl::array<float, 4>{255.f, 255.f, 255.f, 255.f}, o, o);
	mVertices[21] = VertexBuffer( 1,-1,-1, 0,1,0, eastl::array<float, 4>{255.f, 255.f, 255.f, 255.f}, t, o);
	mVertices[22] = VertexBuffer(-1,-1,-1, 0,1,0, eastl::array<float, 4>{255.f, 255.f, 255.f, 255.f}, t, t);
	mVertices[23] = VertexBuffer(-1,-1, 1, 0,1,0, eastl::array<float, 4>{255.f, 255.f, 255.f, 255.f}, o, t);
}

//! pre render method
bool SkyBoxNode::PreRender(Scene* pScene)
{
	if (Get()->IsVisible())
		pScene->AddToRenderQueue(ERP_SKY_BOX, shared_from_this());

	return Node::PreRender(pScene);
}

//! renders the node.
bool SkyBoxNode::Render(Scene* pScene)
{
	const eastl::shared_ptr<Renderer>& renderer = pScene->GetRenderer();
	const eastl::shared_ptr<CameraNode>& camera = pScene->GetActiveCamera();

	if (!camera || !renderer)
		return false;

	Matrix4x4<float> toWorld, fromWorld;
	mParent->Get()->Transform(&toWorld, &fromWorld);

	if ( !camera->IsOrthogonal() )
	{
		// draw perspective skybox

		Matrix4x4<float> translate(toWorld);
		translate.SetTranslation(camera->Get()->ToWorld().GetTranslation());

		// Draw the sky box between the near and far clip plane
		const float viewDistance = (camera->GetNearValue() + camera->GetFarValue()) * 0.5f;
		Matrix4x4<float> scale;
		scale.SetScale(Vector3<float>(viewDistance, viewDistance, viewDistance));

		renderer->SetTransform(ETS_WORLD, translate * scale);

		for (int i=0; i<6; ++i)
		{
			renderer->SetMaterial(mMaterials[i]);
			renderer->DrawIndexedTriangleFan(&mVertices[i*4], 4, mIndices, 2);
		}
	}
	else
	{
		// draw orthogonal skybox,
		// simply choose one texture and draw it as 2d picture.
		// there could be better ways to do this, but currently I think this is ok.

		Vector3<float> target =	
			camera->Get()->ToWorld().GetTranslation() + 
			camera->Get()->ToWorld().GetRotationDegrees().RotationToDirection();
		if (camera->GetTarget())
			target = camera->GetTarget()->Get()->ToWorld().GetTranslation();

		Vector3<float> lookVect = target - camera->Get()->ToWorld().GetTranslation();
		lookVect.Normalize();
		Vector3<float> absVect( eastl::abs(lookVect.X), eastl::abs(lookVect.Y), eastl::abs(lookVect.Z));

		int idx = 0;
		if ( absVect[0] >= absVect[1] && absVect[0] >= absVect[2] )
		{
			// x direction
			idx = lookVect[0] > 0 ? 0 : 2;
		}
		else
		if ( absVect[1] >= absVect[0] && absVect[1] >= absVect[2] )
		{
			// y direction
			idx = lookVect[1] > 0 ? 4 : 5;
		}
		else
		if ( absVect[2] >= absVect[0] && absVect[2] >= absVect[1] )
		{
			// z direction
			idx = lookVect[2] > 0 ? 1 : 3;
		}

		Texture2* tex(mMaterials[idx].GetTexture(0));

		if ( tex )
		{
			RectangleShape<int> rctDest(Vector2<int>(-1,0),
									Dimension2i(renderer->GetCurrentRenderTargetSize()));
			RectangleShape<int> rctSrc(Vector2<int>(0,0),
									Dimension2i(tex->GetSize()));

			renderer->Draw2DImage(tex, rctDest, rctSrc);
		}
	}

	return Node::Render(pScene);
}

//! returns the axis aligned bounding box of this node
const AlignedBox3<float>& SkyBoxNode::GetBoundingBox() const
{
	return mBBox;
}

//! returns the material based on the zero based index i. To get the amount
//! of materials used by this scene node, use getMaterialCount().
//! This function is needed for inserting the node into the scene hirachy on a
//! optimal position for minimizing renderstate changes, but can also be used
//! to directly modify the material of a scene node.
Material& SkyBoxNode::GetMaterial(unsigned int i)
{
	return mMaterials[i];
}

//! returns amount of materials used by this scene node.
unsigned int SkyBoxNode::GetMaterialCount() const
{
	return 6;
}