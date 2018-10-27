// Copyright (C) 2002-2012 Nikolaus Gebhardt
// This file is part of the "Irrlicht Engine".
// For conditions of distribution and use, see copyright notice in irrlicht.h

#include "LightNode.h"

#include "Graphic/Renderer/Renderer.h"
#include "Graphic/Effect/Material.h"

#include "Core/OS/OS.h"

#include "Graphic/Scene/Scene.h"

#include "Application/GameApplication.h"

//! constructor
LightNode::LightNode(const ActorId actorId, PVWUpdater* updater, 
	WeakBaseRenderComponentPtr renderComponent, 
	const eastl::shared_ptr<Texture2>& texture, 
	const Vector2<float>& size, const eastl::shared_ptr<Light>& light)
	:	Node(actorId, renderComponent, NT_LIGHT), mLight(light)
{
	SetSize(size);

	mPVWUpdater = updater;

	mBlendState = eastl::make_shared<BlendState>();
	mDepthStencilState = eastl::make_shared<DepthStencilState>();

	struct Vertex
	{
		Vector3<float> position;
		Vector4<float> color;
		Vector2<float> tcoord;
	};
	VertexFormat vformat;
	vformat.Bind(VA_POSITION, DF_R32G32B32_FLOAT, 0);
	vformat.Bind(VA_COLOR, DF_R32G32B32A32_FLOAT, 0);
	vformat.Bind(VA_TEXCOORD, DF_R32G32_FLOAT, 0);

	MeshBuffer* meshBuffer = new MeshBuffer(vformat, 4, 2, sizeof(unsigned int));
	
	// A point light illuminates the target.  Create a semitransparent
	// material for the patch.
	eastl::shared_ptr<Material> material = eastl::make_shared<Material>();
	material->mEmissive = { 0.0f, 0.0f, 0.0f, 1.0f };
	material->mAmbient = mLight->mLighting->mAmbient;
	material->mDiffuse = mLight->mLighting->mDiffuse;
	material->mSpecular = mLight->mLighting->mSpecular;
	for (unsigned int i = 0; i < GetMaterialCount(); ++i)
		meshBuffer->GetMaterial() = material;
	mMeshBuffer.reset(meshBuffer);
	SetMaterialTexture(0, texture);

	// fill vertices
	for (unsigned int i = 0; i<mMeshBuffer->GetVertice()->GetNumElements(); i += 4)
	{
		mMeshBuffer->TCoord(0, 0 + i) = Vector2<float>{ 0.0f, 0.0f };
		mMeshBuffer->TCoord(0, 1 + i) = Vector2<float>{ 0.0f, 1.0f };
		mMeshBuffer->TCoord(0, 2 + i) = Vector2<float>{ 1.0f, 1.0f };
		mMeshBuffer->TCoord(0, 3 + i) = Vector2<float>{ 1.0f, 0.0f };
	}

	// fill indices
	int vertices = 0;
	for (unsigned int i = 0; i<mMeshBuffer->GetIndice()->GetNumPrimitives(); i += 2, vertices += 4)
	{
		mMeshBuffer->GetIndice()->SetTriangle(i,
			(unsigned int)0 + vertices, (unsigned int)2 + vertices, (unsigned int)1 + vertices);
		mMeshBuffer->GetIndice()->SetTriangle(i + 1,
			(unsigned int)0 + vertices, (unsigned int)3 + vertices, (unsigned int)2 + vertices);
	}

	eastl::string path = FileSystem::Get()->GetPath("Effects/Texture2ColorEffect.hlsl");
	mEffect = eastl::make_shared<Texture2ColorEffect>(
		ProgramFactory::Get(), path, mMeshBuffer->GetMaterial()->GetTexture(0),
		SamplerState::MIN_L_MAG_L_MIP_P, SamplerState::WRAP, SamplerState::WRAP);
	mVisual = eastl::make_shared<Visual>(
		mMeshBuffer->GetVertice(), mMeshBuffer->GetIndice(), mEffect);
	mVisual->UpdateModelBound();

	DoLightRecalc();
}

//! prerender
bool LightNode::PreRender(Scene *pScene)
{
	DoLightRecalc();

	if (IsVisible())
		pScene->AddToRenderQueue(RP_LIGHT, shared_from_this());

	return Node::PreRender(pScene);
}

//
// LightSceneNode::Render
//
bool LightNode::Render(Scene *pScene)
{
	const eastl::shared_ptr<CameraNode>& cameraNode = pScene->GetActiveCamera();

	if (!cameraNode || !Renderer::Get())
		return false;

	/* Vertices are:
	2--1
	|\ |
	| \|
	3--0
	*/

	Vector3<float> scale = GetAbsoluteTransform().GetScale();
	Vector3<float> position = GetAbsoluteTransform().GetTranslation();

	float f = 0.5f * mSize[0];
	const Vector4<float> horizontal = cameraNode->Get()->GetRVector() * f;

	f = 0.5f * mSize[1];
	const Vector4<float> vertical = cameraNode->Get()->GetUVector() * f;

	mMeshBuffer->Position(0) = position + HProject( horizontal + vertical) * scale;
	mMeshBuffer->Color(0, 0) = mMeshBuffer->GetMaterial()->mDiffuse;
	mMeshBuffer->Position(1) = position + HProject( horizontal - vertical) * scale;
	mMeshBuffer->Color(0, 1) = mMeshBuffer->GetMaterial()->mDiffuse;
	mMeshBuffer->Position(2) = position + HProject( -horizontal - vertical) * scale;
	mMeshBuffer->Color(0, 2) = mMeshBuffer->GetMaterial()->mDiffuse;
	mMeshBuffer->Position(3) = position + HProject( -horizontal + vertical) * scale;
	mMeshBuffer->Color(0, 3) = mMeshBuffer->GetMaterial()->mDiffuse;

	eastl::shared_ptr<ConstantBuffer> cbuffer;
	cbuffer = mEffect->GetVertexShader()->Get<ConstantBuffer>("PVWMatrix");
	*cbuffer->Get<Matrix4x4<float>>() = cameraNode->Get()->GetProjectionViewMatrix();

	for (unsigned int i = 0; i < GetMaterialCount(); ++i)
	{
		GetMaterial(i)->Update(mBlendState);
		GetMaterial(i)->Update(mDepthStencilState);
	}

	Renderer::Get()->SetBlendState(mBlendState);
	Renderer::Get()->SetDepthStencilState(mDepthStencilState);

	Renderer* renderer = Renderer::Get();
	renderer->Update(mMeshBuffer->GetVertice());
	renderer->Update(cbuffer);
	renderer->Draw(mVisual);

	Renderer::Get()->SetDefaultBlendState();
	Renderer::Get()->SetDefaultDepthStencilState();

	/*
	if (DebugDataVisible() & DS_BBOX )
	{
		Renderer::Get()->SetTransform(TS_WORLD, toWorld);
		Material m;
		m.mLighting = false;
		Renderer::Get()->SetMaterial(m);

		switch ( mLightData.mType )
		{
			case LT_POINT:
			case LT_SPOT:
				Renderer::Get()->Draw3DBox(mBBox, mLightData.mLighting->mDiffuse.ToColor());
				break;

			case LT_DIRECTIONAL:
				Renderer::Get()->Draw3DLine(Vector3<float>{0.f, 0.f, 0.f},
						mLightData.mLighting->mDirection * mLightData.mRadius,
						mLightData.mLighting.>mDiffuse.ToColor());
				break;
			default:
				break;
		}
	}
	*/

	return true;
}

//! sets the size of the billboard
void LightNode::SetSize(const Vector2<float>& size)
{
	mSize = size;
}

//! gets the size of the light node
const Vector2<float>& LightNode::GetSize() const
{
	return mSize;
}

//! Gets the light type.
/** \return The current light type. */
LightType LightNode::GetLightType() const
{
	return mLight->mLighting->mType;
}

//! Gets the light.
/** \return The current light. */
eastl::shared_ptr<Light> const& LightNode::GetLight() const 
{ 
	return mLight; 
}

void LightNode::DoLightRecalc()
{
	if ((mLight->mLighting->mType == LT_SPOT) ||
		(mLight->mLighting->mType == LT_DIRECTIONAL))
	{
		/*
		Vector4<float> lightDirection = Vector4<float>::Unit(1);
		GetAbsoluteTransform().GetRotation().Transformation(Vector4<float>::Unit(1), lightDirection);
		mLight->mLighting->mDirection = HProject(lightDirection);
		Normalize(mLight->mLighting->mDirection);
		*/
	}
	if ((mLight->mLighting->mType == LT_SPOT) ||
		(mLight->mLighting->mType == LT_POINT))
	{
		mLight->mLighting->mPosition = GetAbsoluteTransform().GetTranslation();
		//mBBox.MaxEdge.set( r, r, r );
		//mBBox.MinEdge.set( -r, -r, -r );
		//Get()->SetAutomaticCulling( AC_BOX );
		//Get()->SetAutomaticCulling( AC_OFF );
	}
	if (mLight->mLighting->mType == LT_DIRECTIONAL)
	{
		//mBBox.Reset( 0, 0, 0 );
		//Get()->SetAutomaticCulling( AC_OFF );
	}
}

//! Returns the visual based on the zero based index i. To get the amount 
//! of visuals used by this scene node, use GetVisualCount(). 
//! This function is needed for inserting the node into the scene hierarchy 
//! at an optimal position for minimizing renderstate changes, but can also 
//! be used to directly modify the visual of a scene node.
eastl::shared_ptr<Visual> const& LightNode::GetVisual(unsigned int i)
{
	return mVisual;
}

//! return amount of visuals of this scene node.
unsigned int LightNode::GetVisualCount() const
{
	return 1;
}

//! returns the material based on the zero based index i. To get the amount
//! of materials used by this scene node, use GetMaterialCount().
//! This function is needed for inserting the node into the scene hirachy on a
//! optimal position for minimizing renderstate changes, but can also be used
//! to directly modify the material of a scene node.
eastl::shared_ptr<Material> const& LightNode::GetMaterial(unsigned int i)
{
	return mMeshBuffer->GetMaterial();
}

//! returns amount of materials used by this scene node.
unsigned int LightNode::GetMaterialCount() const
{
	return 1;
}

//! Sets the texture of the specified layer in all materials of this scene node to the new texture.
/** \param textureLayer Layer of texture to be set. Must be a value smaller than MATERIAL_MAX_TEXTURES.
\param texture New texture to be used. */
void LightNode::SetMaterialTexture(unsigned int textureLayer, eastl::shared_ptr<Texture2> texture)
{
	if (textureLayer >= MATERIAL_MAX_TEXTURES)
		return;

	for (unsigned int i = 0; i<GetMaterialCount(); ++i)
		GetMaterial(i)->SetTexture(textureLayer, texture);
}

//! Sets the material type of all materials in this scene node to a new material type.
/** \param newType New type of material to be set. */
void LightNode::SetMaterialType(MaterialType newType)
{
	for (unsigned int i = 0; i<GetMaterialCount(); ++i)
		GetMaterial(i)->mType = newType;
}