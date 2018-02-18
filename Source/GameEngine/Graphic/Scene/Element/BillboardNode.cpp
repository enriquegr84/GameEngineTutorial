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
BillboardNode::BillboardNode(const ActorId actorId, PVWUpdater& updater, WeakBaseRenderComponentPtr renderComponent,
	const Vector2<float>& size, eastl::array<float, 4> colorTop, eastl::array<float, 4> colorBottom)
	: Node(actorId, renderComponent, RP_TRANSPARENT, NT_BILLBOARD)
{
#ifdef _DEBUG
	//setDebugName("BillboardNode");
#endif
	mPVWUpdater = updater;

	VertexFormat vformat;
	vformat.Bind(VA_POSITION, DF_R32G32B32A32_FLOAT, 0);
	vformat.Bind(VA_NORMAL, DF_R32G32B32A32_FLOAT, 0);
	vformat.Bind(VA_TEXCOORD, DF_R32G32_FLOAT, 0);
	vformat.Bind(VA_COLOR, DF_R32G32B32A32_FLOAT, 0);

	eastl::shared_ptr<VertexBuffer> vertices = eastl::make_shared<VertexBuffer>(vformat, 4);
	eastl::shared_ptr<IndexBuffer> indices = eastl::make_shared<IndexBuffer>(IP_TRISTRIP, 2);
	eastl::shared_ptr<AmbientLightEffect> effect = eastl::make_shared<AmbientLightEffect>(
		ProgramFactory::Get(), mPVWUpdater.GetUpdater(), eastl::make_shared<Material>(), eastl::make_shared<Light>());

	SetSize(size);

	struct Vertex
	{
		Vector3<float> position;
		Vector3<float> normal;
		Vector2<float> tcoord;
		Vector4<float> color;
	};
	Vertex* vertex = vertices->Get<Vertex>();
	vertex[0].tcoord = { 1.0f, 1.0f };
	vertex[0].color = colorBottom;

	vertex[1].tcoord = { 1.0f, 0.0f };
	vertex[1].color = colorTop;

	vertex[2].tcoord = { 0.0f, 0.0f };
	vertex[2].color = colorTop;

	vertex[3].tcoord = { 0.0f, 1.0f };
	vertex[3].color = colorBottom;

	mVisual = eastl::make_shared<Visual>(vertices, indices, effect);
	mPVWUpdater.Subscribe(mVisual->GetAbsoluteTransform(), effect->GetPVWMatrixConstant());
}

void BillboardNode::UpdateWorldData(double applicationTime)
{
    // Compute the billboard's world transforms based on its parent's world
    // transform and its local transforms.  Notice that you should not call
    // Node::UpdateWorldData since that function updates its children.  The
    // children of a BillboardNode cannot be updated until the billboard is
    // aligned with the camera.
    Spatial::UpdateWorldData(applicationTime);

    if (mPVWUpdater.GetCamera())
    {
        // Inverse-transform the camera to the model space of the billboard.
        Matrix4x4<float> const& inverse = mWorldTransform.GetHInverse();
#if defined(GE_USE_MAT_VEC)
        Vector4<float> modelPos = inverse * mPVWUpdater.GetCamera()->GetPosition();
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
        Matrix4x4<float> orient = Rotation<4, float>(AxisAngle<4, float>(
            Vector4<float>::Unit(1), angle));
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
            child->Update(applicationTime, false);
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
		{
			eastl::shared_ptr<AmbientLightEffect> effect =
				eastl::static_pointer_cast<AmbientLightEffect>(mVisual->GetEffect());

			if (effect->GetMaterial()->IsTransparent())
				++transparentCount;
			else
				++solidCount;
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
{
	const eastl::shared_ptr<CameraNode>& cameraNode = pScene->GetActiveCamera();

	if (!cameraNode || !Renderer::Get())
		return false;

	// make billboard look to camera
	Vector4<float> pos = mWorldTransform.GetTranslationW1();

	Vector4<float> campos = cameraNode->Get()->GetPosition();
	Vector4<float> target = 
		cameraNode->Get()->GetPosition() + cameraNode->Get()->GetDVector();

	if (cameraNode->GetTarget())
		target = cameraNode->GetTarget()->GetAbsoluteTransform().GetTranslationW1();

	Vector4<float> up = cameraNode->Get()->GetUVector();
	Vector4<float> view = Normalize(target - campos);

	Vector4<float> horizontal = Cross(up, view);
	if (Length(horizontal) == 0)
	{
		horizontal = { up[1], up[0], up[2], up[3] };
	}

	horizontal = Normalize(horizontal);
	Vector4<float> topHorizontal = horizontal * 0.5f * mTopEdgeWidth;
	horizontal *= 0.5f * mSize[0];

	// pointing down!
	Vector4<float> vertical = Cross(horizontal, view);
	vertical = Normalize(vertical);
	vertical *= 0.5f * mSize[1];

	view *= -1.0f;

	struct Vertex
	{
		Vector3<float> position;
		Vector3<float> normal;
		Vector2<float> tcoord;
		Vector4<float> color;
	};
	Vertex* vertex = mVisual->GetVertexBuffer()->Get<Vertex>();
	for (int i = 0; i<4; ++i)
		vertex[i].normal = { view[0], view[1], view[2] };

	/* Vertices are:
	2--1
	|\ |
	| \|
	3--0
	*/
	Vector4<float> vertexPos = pos + horizontal + vertical;
	vertex[0].position = { vertexPos[0], vertexPos[1], vertexPos[2] };
	vertexPos = pos + topHorizontal - vertical;
	vertex[1].position = { vertexPos[0], vertexPos[1], vertexPos[2] };
	vertexPos = pos - topHorizontal - vertical;
	vertex[2].position = { vertexPos[0], vertexPos[1], vertexPos[2] };
	vertexPos = pos - horizontal + vertical;
	vertex[3].position = { vertexPos[0], vertexPos[1], vertexPos[2] };

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

	eastl::shared_ptr<AmbientLightEffect> effect =
		eastl::static_pointer_cast<AmbientLightEffect>(mVisual->GetEffect());
	effect->SetMaterial(material);
	*/

	Renderer::Get()->Draw(mVisual);

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


eastl::shared_ptr<Material> const& BillboardNode::GetMaterial(unsigned int i)
{
	eastl::shared_ptr<AmbientLightEffect> effect =
		eastl::static_pointer_cast<AmbientLightEffect>(mVisual->GetEffect());
	return effect->GetMaterial();
}


//! returns amount of materials used by this scene node.
unsigned int BillboardNode::GetMaterialCount() const
{
	return 1;
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


//! Set the color of all vertices of the billboard
//! \param overallColor: the color to set
void BillboardNode::SetColor(const eastl::array<float, 4>& overallColor)
{
	struct Vertex
	{
		Vector3<float> position;
		Vector3<float> normal;
		Vector2<float> tcoord;
		Vector4<float> color;
	};
	Vertex* vertex = mVisual->GetVertexBuffer()->Get<Vertex>();
	for (unsigned int idx = 0; idx < 4; ++idx)
		vertex[idx].color = overallColor;
}


//! Set the color of the top and bottom vertices of the billboard
//! \param topColor: the color to set the top vertices
//! \param bottomColor: the color to set the bottom vertices
void BillboardNode::SetColor(const eastl::array<float, 4>& topColor, const eastl::array<float, 4>& bottomColor)
{
	struct Vertex
	{
		Vector3<float> position;
		Vector3<float> normal;
		Vector2<float> tcoord;
		Vector4<float> color;
	};
	Vertex* vertex = mVisual->GetVertexBuffer()->Get<Vertex>();
	vertex[0].color = bottomColor;
	vertex[1].color = topColor;
	vertex[2].color = topColor;
	vertex[3].color = bottomColor;
}


//! Gets the color of the top and bottom vertices of the billboard
//! \param[out] topColor: stores the color of the top vertices
//! \param[out] bottomColor: stores the color of the bottom vertices
void BillboardNode::GetColor(eastl::array<float, 4>& topColor, eastl::array<float, 4>& bottomColor) const
{
	struct Vertex
	{
		Vector3<float> position;
		Vector3<float> normal;
		Vector2<float> tcoord;
		Vector4<float> color;
	};
	Vertex* vertex = mVisual->GetVertexBuffer()->Get<Vertex>();
	bottomColor = { vertex[0].color[0], vertex[0].color[1], vertex[0].color[2], vertex[0].color[3] };
	topColor = { vertex[1].color[0], vertex[1].color[1], vertex[1].color[2], vertex[1].color[3] };
}

