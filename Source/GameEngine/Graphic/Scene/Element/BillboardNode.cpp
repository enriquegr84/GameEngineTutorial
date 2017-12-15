// David Eberly, Geometric Tools, Redmond WA 98052
// Copyright (c) 1998-2017
// Distributed under the Boost Software License, Version 1.0.
// http://www.boost.org/LICENSE_1_0.txt
// http://www.geometrictools.com/License/Boost/LICENSE_1_0.txt
// File Version: 3.0.0 (2016/06/19)

#include "BillboardNode.h"
#include "Mathematic/Algebra/Rotation.h"
#include "Mathematic/Function/Functions.h"

BillboardNode::BillboardNode(eastl::shared_ptr<Camera> const& camera)
    : mCamera(camera)
{
}

void BillboardNode::UpdateWorldData(double applicationTime)
{
    // Compute the billboard's world transforms based on its parent's world
    // transform and its local transforms.  Notice that you should not call
    // Node::UpdateWorldData since that function updates its children.  The
    // children of a BillboardNode cannot be updated until the billboard is
    // aligned with the camera.
    Spatial::UpdateWorldData(applicationTime);

    if (mCamera)
    {
        // Inverse-transform the camera to the model space of the billboard.
        Matrix4x4<float> const& inverse = worldTransform.GetHInverse();
#if defined(GE_USE_MAT_VEC)
        Vector4<float> modelPos = inverse * mCamera->GetPosition();
#else
        Vector4<float> modelPos = mCamera->GetPosition() * inverse;
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
        worldTransform.SetRotation(worldTransform.GetRotation() * orient);
#else
        worldTransform.SetRotation(orient * worldTransform.GetRotation());
#endif
    }

    // Update the children now that the billboard orientation is known.
    for (auto& child : mChild)
    {
        if (child)
        {
            child->Update(applicationTime, false);
        }
    }
}


//! constructor
BillboardNode::BillboardNode(const ActorId actorId, WeakBaseRenderComponentPtr renderComponent,
	const Vector2<float>& size, eastl::array<float, 4> colorTop, eastl::array<float, 4> colorBottom)
	: Node(actorId, renderComponent, ERP_TRANSPARENT, ESNT_BILLBOARD)
{
#ifdef _DEBUG
	//setDebugName("BillboardNode");
#endif

	SetSize(size);

	mIndices[0] = 0;
	mIndices[1] = 2;
	mIndices[2] = 1;
	mIndices[3] = 0;
	mIndices[4] = 3;
	mIndices[5] = 2;

	mVertices[0].mTCoords.set(1.0f, 1.0f);
	mVertices[0].mColor = colorBottom;

	mVertices[1].mTCoords.set(1.0f, 0.0f);
	mVertices[1].mColor = colorTop;

	mVertices[2].mTCoords.set(0.0f, 0.0f);
	mVertices[2].mColor = colorTop;

	mVertices[3].mTCoords.set(0.0f, 1.0f);
	mVertices[3].mColor = colorBottom;
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
		for (unsigned int i = 0; i<GetMaterialCount(); ++i)
		{
			if (GetMaterial(i).IsTransparent())
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
{
	Matrix4x4<float> toWorld, fromWorld;
	Get()->Transform(&toWorld, &fromWorld);

	const eastl::shared_ptr<Renderer>& renderer = pScene->GetRenderer();
	const eastl::shared_ptr<CameraNode>& camera = pScene->GetActiveCamera();

	if (!camera || !renderer)
		return false;

	// make billboard look to camera
	Vector3<float> pos = toWorld.GetTranslation();

	Vector3<float> campos = camera->Get()->ToWorld().GetTranslation();
	Vector3<float> target = camera->Get()->ToWorld().GetTranslation() +
		camera->Get()->ToWorld().GetRotationDegrees().RotationToDirection();
	if (camera->GetTarget())
		target = camera->GetTarget()->Get()->ToWorld().GetTranslation();

	Vector3<float> up = camera->GetUpVector();
	Vector3<float> view = target - campos;
	view.Normalize();

	Vector3<float> horizontal = up.CrossProduct(view);
	if (horizontal.GetLength() == 0)
	{
		horizontal.set(up.Y, up.X, up.Z);
	}
	horizontal.Normalize();
	Vector3<float> topHorizontal = horizontal * 0.5f * mTopEdgeWidth;
	horizontal *= 0.5f * mSize.Width;

	// pointing down!
	Vector3<float> vertical = horizontal.CrossProduct(view);
	vertical.Normalize();
	vertical *= 0.5f * mSize.Height;

	view *= -1.0f;

	for (int i = 0; i<4; ++i)
		mVertices[i].mNormal = view;

	/* Vertices are:
	2--1
	|\ |
	| \|
	3--0
	*/
	mVertices[0].mPos = pos + horizontal + vertical;
	mVertices[1].mPos = pos + topHorizontal - vertical;
	mVertices[2].mPos = pos - topHorizontal - vertical;
	mVertices[3].mPos = pos - horizontal + vertical;

	// draw
	if (mProps.DebugDataVisible() & EDS_BBOX)
	{
		renderer->SetTransform(ETS_WORLD, toWorld);
		Material m;
		m.Lighting = false;
		renderer->SetMaterial(m);
		renderer->Draw3DBox(mBBox, eastl::array<float, 4>{0.f, 208.f, 195.f, 152.f});
	}

	renderer->SetTransform(ETS_WORLD, Matrix4x4<float>::Identity);
	renderer->SetMaterial(mMaterial);
	renderer->DrawIndexedTriangleList(mVertices, 4, mIndices, 2);

	return Node::Render(pScene);
}


//! returns the axis aligned bounding box of this node
const AlignedBox3<float>& BillboardNode::GetBoundingBox() const
{
	return mBBox;
}


//! sets the size of the billboard
void BillboardNode::SetSize(const Vector2<float>& size)
{
	mSize = size;

	if (eastl::equals(mSize.Width, 0.0f))
		mSize.Width = 1.0f;
	mTopEdgeWidth = mSize.Width;

	if (eastl::equals(mSize.Height, 0.0f))
		mSize.Height = 1.0f;

	const float avg = (mSize.Width + mSize.Height) / 6;
	mBBox.MinEdge.set(-avg, -avg, -avg);
	mBBox.MaxEdge.set(avg, avg, avg);
}


void BillboardNode::SetSize(float height, float bottomEdgeWidth, float topEdgeWidth)
{
	mSize.set(bottomEdgeWidth, height);
	mTopEdgeWidth = topEdgeWidth;

	if (eastl::equals(mSize.Height, 0.0f))
		mSize.Height = 1.0f;

	if (eastl::equals(mSize.Width, 0.f) && 
		eastl::equals(mTopEdgeWidth, 0.f))
	{
		mSize.Width = 1.0f;
		mTopEdgeWidth = 1.0f;
	}

	const float avg = (eastl::max(mSize.Width, mTopEdgeWidth) + mSize.Height) / 6;
	mBBox.MinEdge.set(-avg, -avg, -avg);
	mBBox.MaxEdge.set(avg, avg, avg);
}


Material& BillboardNode::GetMaterial()
{
	return mMaterial;
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
	height = mSize.Height;
	bottomEdgeWidth = mSize.Width;
	topEdgeWidth = mTopEdgeWidth;
}


//! Set the color of all vertices of the billboard
//! \param overallColor: the color to set
void BillboardNode::SetColor(const eastl::array<float, 4>& overallColor)
{
	for (unsigned int vertex = 0; vertex < 4; ++vertex)
		mVertices[vertex].mColor = overallColor;
}


//! Set the color of the top and bottom vertices of the billboard
//! \param topColor: the color to set the top vertices
//! \param bottomColor: the color to set the bottom vertices
void BillboardNode::SetColor(const eastl::array<float, 4>& topColor, const eastl::array<float, 4>& bottomColor)
{
	mVertices[0].mColor = bottomColor;
	mVertices[1].mColor = topColor;
	mVertices[2].mColor = topColor;
	mVertices[3].mColor = bottomColor;
}


//! Gets the color of the top and bottom vertices of the billboard
//! \param[out] topColor: stores the color of the top vertices
//! \param[out] bottomColor: stores the color of the bottom vertices
void BillboardNode::GetColor(eastl::array<float, 4>& topColor, eastl::array<float, 4>& bottomColor) const
{
	bottomColor = mVertices[0].mColor;
	topColor = mVertices[1].mColor;
}

