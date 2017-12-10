// David Eberly, Geometric Tools, Redmond WA 98052
// Copyright (c) 1998-2017
// Distributed under the Boost Software License, Version 1.0.
// http://www.boost.org/LICENSE_1_0.txt
// http://www.geometrictools.com/License/Boost/LICENSE_1_0.txt
// File Version: 3.0.0 (2016/06/19)

#ifndef BILLBOARDNODE_H
#define BILLBOARDNODE_H

#include "Graphic/Scene/Hierarchy/Node.h"

//! Scene node which is a billboard. A billboard is like a 3d sprite: A 2d element,
//! which always looks to the camera.
class GRAPHIC_ITEM BillboardNode : public Node
{
public:
    // The model space of the billboard has an up vector of (0,1,0) that is
    // chosen to be the billboard's axis of rotation.

    // Construction.
    BillboardNode(eastl::shared_ptr<Camera> const& camera);

    // The camera to which the billboard is aligned.
    inline void AlignTo(eastl::shared_ptr<Camera> const& camera);


	//! Constructor
	BillboardNode(const ActorId actorId, WeakBaseRenderComponentPtr renderComponent, const Vector2<float>& size,
		eastl::array<float, 4> const colorTop = eastl::array<float, 4>{255.f, 255.f, 255.f, 255.f},
		eastl::array<float, 4> const colorBottom = eastl::array<float, 4>{255.f, 255.f, 255.f, 255.f});

	//! Renders event
	bool PreRender(Scene *pScene);
	bool Render(Scene *pScene);

	//! returns the axis aligned bounding box of this node
	const AABBox3<float>& GetBoundingBox() const;

	//! sets the size of the billboard
	void SetSize(const Vector2<float>& size);

	//! Sets the widths of the top and bottom edges of the billboard independently.
	void SetSize(float height, float bottomEdgeWidth, float topEdgeWidth);

	//! gets the size of the billboard
	const Vector2<float>& GetSize() const;

	//! Gets the widths of the top and bottom edges of the billboard.
	void GetSize(float& height, float& bottomEdgeWidth, float& topEdgeWidth) const;

	Material& GetMaterial(unsigned int i);

	//! returns amount of materials used by this scene node.
	 unsigned int GetMaterialCount() const;

	//! Set the color of all vertices of the billboard
	//! \param overallColor: the color to set
	void SetColor(const eastl::array<float, 4>& overallColor);

	//! Set the color of the top and bottom vertices of the billboard
	//! \param topColor: the color to set the top vertices
	//! \param bottomColor: the color to set the bottom vertices
	void SetColor(const eastl::array<float, 4>& topColor, const eastl::array<float, 4>& bottomColor);

	//! Gets the color of the top and bottom vertices of the billboard
	//! \param[out] topColor: stores the color of the top vertices
	//! \param[out] bottomColor: stores the color of the bottom vertices
	void GetColor(eastl::array<float, 4>& topColor, eastl::array<float, 4>& bottomColor) const;

	//! Returns type of the scene node
	E_SCENE_NODE_TYPE GetType() const { return ESNT_BILLBOARD; }

protected:
    // Support for the geometric update.
    virtual void UpdateWorldData(double applicationTime);

	eastl::shared_ptr<Camera> mCamera;

private:

	//! Size.Width is the bottom edge width
	Vector2<float> mSize;
	float mTopEdgeWidth;
	AABBox3<float> mBBox;
	Material mMaterial;

	VertexBuffer mVertices[4];
	unsigned int mIndices[6];
};


inline void BillboardNode::AlignTo(eastl::shared_ptr<Camera> const& camera)
{
    mCamera = camera;
}


#endif