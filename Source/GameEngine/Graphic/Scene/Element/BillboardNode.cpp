// David Eberly, Geometric Tools, Redmond WA 98052
// Copyright (c) 1998-2017
// Distributed under the Boost Software License, Version 1.0.
// http://www.boost.org/LICENSE_1_0.txt
// http://www.geometrictools.com/License/Boost/LICENSE_1_0.txt
// File Version: 3.0.0 (2016/06/19)

#include "BillboardNode.h"

#include "Mathematic/Algebra/Rotation.h"
#include "Mathematic/Function/Functions.h"

#include "Graphic/Renderer/Renderer.h"
#include "Graphic/Effect/Material.h"

#include "Graphic/Scene/Scene.h"

//! constructor
BillboardNode::BillboardNode(const ActorId actorId, 
	PVWUpdater* updater, WeakBaseRenderComponentPtr renderComponent, 
	const eastl::shared_ptr<Texture2>& texture, const Vector2<float>& size)
	: Node(actorId, renderComponent, NT_BILLBOARD)
{
	SetSize(size);

	mPVWUpdater = updater;

	VertexFormat vformat;
	vformat.Bind(VA_POSITION, DF_R32G32B32_FLOAT, 0);
	vformat.Bind(VA_NORMAL, DF_R32G32B32_FLOAT, 0);
	vformat.Bind(VA_TEXCOORD, DF_R32G32_FLOAT, 0);

	MeshFactory mf;
	mf.SetVertexFormat(vformat);
	mVisual = mf.CreateBox(size[0], 0.f, size[1]);
	eastl::shared_ptr<VertexBuffer> vbuffer = mVisual->GetVertexBuffer();
	vbuffer->SetUsage(Resource::DYNAMIC_UPDATE);

	// Create the visual effect. The world up-direction is (0,0,1).  Choose
	// the light to point down.
	mMaterial = eastl::make_shared<Material>();
	mMaterial->mEmissive = { 0.0f, 0.0f, 0.0f, 1.0f };
	mMaterial->mAmbient = { 0.5f, 0.5f, 0.5f, 1.0f };
	mMaterial->mDiffuse = { 0.5f, 0.5f, 0.5f, 1.0f };
	mMaterial->mSpecular = { 1.0f, 1.0f, 1.0f, 0.75f };

	eastl::shared_ptr<Lighting> lighting = eastl::make_shared<Lighting>();
	eastl::shared_ptr<LightCameraGeometry> geometry = eastl::make_shared<LightCameraGeometry>();

	eastl::string path = FileSystem::Get()->GetPath("Effects/PointLightTextureEffect.hlsl");
	eastl::shared_ptr<PointLightTextureEffect> effect = eastl::make_shared<PointLightTextureEffect>(
		ProgramFactory::Get(), mPVWUpdater->GetUpdater(), path, mMaterial, lighting, geometry, texture,
		SamplerState::MIN_L_MAG_L_MIP_L, SamplerState::WRAP, SamplerState::WRAP);
	mVisual->SetEffect(effect);
	mVisual->UpdateModelNormals();
	mPVWUpdater->Subscribe(mWorldTransform, effect->GetPVWMatrixConstant());
}

void BillboardNode::UpdateWorldData()
{
    // Compute the billboard's world transforms based on its parent's world
    // transform and its local transforms.  Notice that you should not call
    // Node::UpdateWorldData since that function updates its children.  The
    // children of a BillboardNode cannot be updated until the billboard is
    // aligned with the camera.
    Spatial::UpdateWorldData();

    if (mPVWUpdater->GetCamera())
    {
        // Inverse-transform the camera to the model space of the billboard.
        Matrix4x4<float> const& inverse = mWorldTransform.GetHInverse();
#if defined(GE_USE_MAT_VEC)
        Vector4<float> modelPos = inverse * mPVWUpdater->GetCamera()->GetPosition();
#else
        Vector4<float> modelPos = mPVWUpdater.GetCamera()->GetPosition() * inverse;
#endif

        // To align the billboard, the projection of the camera to the
        // xz-plane of the billboard's model space determines the angle of
        // rotation about the billboard's model y-axis.  If the projected
        // camera is on the model axis (x = 0 and z = 0), ATan2 returns zero
        // (rather than NaN), so there is no need to trap this degenerate
        // case and handle it separately.
        float angle = Function<float>::ATan2(modelPos[0], modelPos[2]);
        Matrix4x4<float> orient = Rotation<4, float>(
			AxisAngle<4, float>(Vector4<float>::Unit(2), angle));
#if defined(GE_USE_MAT_VEC)
        mWorldTransform.SetRotation(mWorldTransform.GetRotation() * orient);
#else
        mWorldTransform.SetRotation(orient * mWorldTransform.GetRotation());
#endif
    }

    // Update the children now that the billboard orientation is known.
    for (auto& child : mChildren)
    {
        if (child)
        {
            child->Update(false);
        }
    }
}

//! prerender
bool BillboardNode::PreRender(Scene *pScene)
{
	if (IsVisible())
	{
		// because this node supports rendering of mixed mode meshes consisting of
		// transparent and solid material at the same time, we need to go through all
		// materials, check of what type they are and register this node for the right
		// render pass according to that.

		int transparentCount = 0;
		int solidCount = 0;

		// count transparent and solid materials in this scene node
		for (unsigned int i = 0; i < GetMaterialCount(); ++i)
		{
			if (GetMaterial(i)->IsTransparent())
				++transparentCount;
			else
				++solidCount;

			if (solidCount && transparentCount)
				break;
		}

		// register according to material types counted
		if (!pScene->IsCulled(this))
		{
			if (solidCount)
				pScene->AddToRenderQueue(RP_SOLID, shared_from_this());

			if (transparentCount)
				pScene->AddToRenderQueue(RP_TRANSPARENT, shared_from_this());
		}
	}

	return Node::PreRender(pScene);
}

//
// BillboardNode::Render
//
bool BillboardNode::Render(Scene *pScene)
{/*
	const eastl::shared_ptr<CameraNode>& cameraNode = pScene->GetActiveCamera();

	if (!cameraNode || !Renderer::Get())
		return false;

	// make billboard look to camera
	Matrix4x4<float> rotation = mWorldTransform.GetRotation();
	Vector4<float> position = mWorldTransform.GetTranslationW1();

	Vector4<float> camPosition = cameraNode->Get()->GetPosition();
	Vector4<float> direction;
#if defined(GE_USE_MAT_VEC)
	direction = rotation.GetCol(1);
#else
	direction = rotation.GetRow(1);
#endif

	if (cameraNode->GetTarget())
	{
		direction = cameraNode->GetTarget()->GetAbsoluteTransform().GetTranslationW1() - camPosition;
		Normalize(direction);
	}

	Vector4<float> up = cameraNode->Get()->GetUVector();
	Vector4<float> horizontal = Cross(up, direction);
	if (Length(horizontal) == 0)
		horizontal = { up[1], up[0], up[2], up[3] };

	Normalize(horizontal);
	Vector4<float> topHorizontal = horizontal * 0.5f * mTopEdgeWidth;
	horizontal *= 0.5f * mSize[0];

	// pointing down!
	Vector4<float> vertical = Cross(horizontal, direction);
	Normalize(vertical);
	vertical *= 0.5f * mSize[1];

	direction *= -1.0f;

	struct Vertex
	{
		Vector3<float> position;
		Vector3<float> normal;
		Vector2<float> tcoord;
	};
	Vertex* vertex = mVisual->GetVertexBuffer()->Get<Vertex>();
	for (int i = 0; i<4; ++i)
		vertex[i].normal = { direction[0], direction[1], direction[2] };

	Vector4<float> vertexPos = position + horizontal + vertical;
	vertex[0].position = { vertexPos[0], vertexPos[1], vertexPos[2] };
	vertexPos = position + topHorizontal - vertical;
	vertex[1].position = { vertexPos[0], vertexPos[1], vertexPos[2] };
	vertexPos = position - topHorizontal - vertical;
	vertex[2].position = { vertexPos[0], vertexPos[1], vertexPos[2] };
	vertexPos = position - horizontal + vertical;
	vertex[3].position = { vertexPos[0], vertexPos[1], vertexPos[2] };
	/* Vertices are:
	2--1
	|\ |
	| \|
	3--0
	*/


	// draw
	/*
	if (DebugDataVisible() & DS_BBOX)
	{
		Renderer::Get()->SetTransform(TS_WORLD, toWorld);
		Material m;
		m.mLighting = false;
		Renderer::Get()->SetMaterial(m);
		Renderer::Get()->Draw3DBox(mBBox, eastl::array<float, 4>{0.f, 208.f, 195.f, 152.f});
	}

	Renderer::Get()->SetTransform(TS_WORLD, Matrix4x4<float>::Identity);
	Renderer::Get()->SetMaterial(mMaterial);
	Renderer::Get()->DrawIndexedTriangleList(mVertices, 4, mIndices, 2);
	*/
	/*
	Renderer::Get()->SetBlendState(mMaterial->mBlendState);
	Renderer::Get()->SetRasterizerState(mMaterial->mRasterizerState);
	Renderer::Get()->SetDepthStencilState(mMaterial->mDepthStencilState);
	*/
	eastl::shared_ptr<AmbientLightEffect> effect =
		eastl::static_pointer_cast<AmbientLightEffect>(mVisual->GetEffect());
	effect->SetMaterial(mMaterial);
	Renderer::Get()->Draw(mVisual);
	/*
	Renderer::Get()->SetDefaultDepthStencilState();
	Renderer::Get()->SetDefaultRasterizerState();
	Renderer::Get()->SetDefaultBlendState();
	*/
	return Node::Render(pScene);
}

//! sets the size of the billboard
void BillboardNode::SetSize(const Vector2<float>& size)
{
	mSize = size;

	if (Function<float>::Equals(mSize[0], 0.0f))
		mSize[0] = 1.0f;
	mTopEdgeWidth = mSize[0];

	if (Function<float>::Equals(mSize[1], 0.0f))
		mSize[1] = 1.0f;

	const float avg = (mSize[0] + mSize[1]) / 6;
	//mBBox.mMinEdge.set(-avg, -avg, -avg);
	//mBBox.mMaxEdge.set(avg, avg, avg);
}


void BillboardNode::SetSize(float height, float bottomEdgeWidth, float topEdgeWidth)
{
	mSize = { bottomEdgeWidth, height };
	mTopEdgeWidth = topEdgeWidth;

	if (Function<float>::Equals(mSize[1], 0.0f))
		mSize[1] = 1.0f;

	if (Function<float>::Equals(mSize[0], 0.f) &&
		Function<float>::Equals(mTopEdgeWidth, 0.f))
	{
		mSize[0] = 1.0f;
		mTopEdgeWidth = 1.0f;
	}

	const float avg = (eastl::max(mSize[0], mTopEdgeWidth) + mSize[1]) / 6;
	//mBBox.MinEdge.set(-avg, -avg, -avg);
	//mBBox.MaxEdge.set(avg, avg, avg);
}


//! gets the size of the billboard
const Vector2<float>& BillboardNode::GetSize() const
{
	return mSize;
}


//! Gets the widths of the top and bottom edges of the billboard.
void BillboardNode::GetSize(float& height, float& bottomEdgeWidth, float& topEdgeWidth) const
{
	height = mSize[1];
	bottomEdgeWidth = mSize[0];
	topEdgeWidth = mTopEdgeWidth;
}

//! Returns the visual based on the zero based index i. To get the amount 
//! of visuals used by this scene node, use GetVisualCount(). 
//! This function is needed for inserting the node into the scene hierarchy 
//! at an optimal position for minimizing renderstate changes, but can also 
//! be used to directly modify the visual of a scene node.
eastl::shared_ptr<Visual> const& BillboardNode::GetVisual(unsigned int i)
{
	return mVisual;
}

//! return amount of visuals of this scene node.
unsigned int BillboardNode::GetVisualCount() const
{
	return 1;
}

//! returns the material based on the zero based index i. To get the amount
//! of materials used by this scene node, use GetMaterialCount().
//! This function is needed for inserting the node into the scene hirachy on a
//! optimal position for minimizing renderstate changes, but can also be used
//! to directly modify the material of a scene node.
eastl::shared_ptr<Material> const& BillboardNode::GetMaterial(unsigned int i)
{
	return mMaterial;
}

//! returns amount of materials used by this scene node.
unsigned int BillboardNode::GetMaterialCount() const
{
	return 1;
}

//! Sets the texture of the specified layer in all materials of this scene node to the new texture.
/** \param textureLayer Layer of texture to be set. Must be a value smaller than MATERIAL_MAX_TEXTURES.
\param texture New texture to be used. */
void BillboardNode::SetMaterialTexture(unsigned int textureLayer, eastl::shared_ptr<Texture2> texture)
{
	if (textureLayer >= MATERIAL_MAX_TEXTURES)
		return;

	for (unsigned int i = 0; i<GetMaterialCount(); ++i)
		GetMaterial(i)->SetTexture(textureLayer, texture);
}

//! Sets the material type of all materials in this scene node to a new material type.
/** \param newType New type of material to be set. */
void BillboardNode::SetMaterialType(MaterialType newType)
{
	for (unsigned int i = 0; i<GetMaterialCount(); ++i)
		GetMaterial(i)->mType = newType;
}