// David Eberly, Geometric Tools, Redmond WA 98052
// Copyright (c) 1998-2017
// Distributed under the Boost Software License, Version 1.0.
// http://www.boost.org/LICENSE_1_0.txt
// http://www.geometrictools.com/License/Boost/LICENSE_1_0.txt
// File Version: 3.0.0 (2016/06/19)

#include "Node.h"

#include "Graphic/Scene/Scene.h"

#include "Game/Actor/Actor.h"
#include "Game/Actor/TransformComponent.h"

#include "Application/GameApplication.h"

////////////////////////////////////////////////////
// Node Implementation
////////////////////////////////////////////////////

Node::Node(int id, WeakBaseRenderComponentPtr renderComponent, NodeType nodeType)
	: Spatial(), mId(id), mDebugState(DM_OFF), mType(nodeType), mRenderComponent(renderComponent)
{
	mIsVisible = true;
}


Node::~Node()
{
	// delete all children
	DetachAllChildren();

	// delete all animators
	DetachAllAnimators();
}


int Node::GetNumChildren() const
{
    return static_cast<int>(mChildren.size());
}

int Node::AttachChild(eastl::shared_ptr<Node> const& child)
{
    if (!child)
    {
        LogError("You cannot attach null children to a node.");
        return -1;
    }

    if (child->GetParent())
    {
        LogError("The child already has a parent.");
        return -1;
    }

    child->SetParent(this);

    // Insert the child in the first available slot (if any).
    int i = 0;
    for (auto& current : mChildren)
    {
        if (!current)
        {
            current = child;
            return i;
        }
        ++i;
    }

    // All slots are used, so append the child to the array.
    int const numChildren = static_cast<int>(mChildren.size());
	mChildren.push_back(child);
    return numChildren;
}

int Node::DetachChild(eastl::shared_ptr<Node> const& child)
{
    if (child)
    {
        int i = 0;
		for (SceneNodeList::iterator it = mChildren.begin(); it != mChildren.end(); ++it)
		{
			if ((*it) == child)
			{
				(*it)->SetParent(nullptr);
				mChildren.erase(it);
				return i;
			}
			++i;
		}
    }
    return -1;
}

eastl::shared_ptr<Node> Node::DetachChildAt(int i)
{
	if (0 <= i && i < static_cast<int>(mChildren.size()))
	{
		SceneNodeList::iterator itChild = mChildren.begin() + i;
		if (itChild != mChildren.end())
		{
			(*itChild)->SetParent(nullptr);
			mChildren.erase(itChild);
		}
		return (*itChild);
	}
	return nullptr;
}

void Node::DetachAllChildren()
{
	for (SceneNodeList::iterator it = mChildren.begin(); it != mChildren.end(); ++it)
		(*it)->SetParent(nullptr);
	mChildren.clear();
}

eastl::shared_ptr<Node> Node::SetChild(int i, eastl::shared_ptr<Node> const& child)
{
    if (child)
    {
        LogAssert(!child->GetParent(), "The child already has a parent.");
    }

    int const numChildren = static_cast<int>(mChildren.size());
    if (0 <= i && i < numChildren)
    {
        // Remove the child currently in the slot.
		eastl::shared_ptr<Node> previousChild = mChildren[i];
        if (previousChild)
        {
            previousChild->SetParent(nullptr);
        }

        // Insert the new child in the slot.
        if (child)
        {
            child->SetParent(this);
        }

		mChildren[i] = child;
        return previousChild;
    }

    // The index is out of range, so append the child to the array.
    if (child)
    {
        child->SetParent(this);
    }
	mChildren.push_back(child);
    return nullptr;
}

eastl::shared_ptr<Node> Node::GetChild(int i)
{
    if (0 <= i && i < static_cast<int>(mChildren.size()))
    {
        return mChildren[i];
    }
    return nullptr;
}


int Node::AttachAnimator(eastl::shared_ptr<NodeAnimator> const& animator)
{
	if (!animator)
	{
		LogError("You cannot attach null animator.");
		return -1;
	}

	// Insert the animator in the first available slot (if any).
	int i = 0;
	for (auto& current : mAnimators)
	{
		if (!current)
		{
			current = animator;
			return i;
		}
		++i;
	}

	// All slots are used, so append the animator to the array.
	int const numAnimators = static_cast<int>(mAnimators.size());
	mAnimators.push_back(animator);
	return numAnimators;
}

int Node::DetachAnimator(eastl::shared_ptr<NodeAnimator> const& animator)
{
	if (animator)
	{
		int i = 0;
		for (SceneNodeAnimatorList::iterator it = mAnimators.begin(); it != mAnimators.end(); ++it)
		{
			if ((*it) == animator)
			{
				mAnimators.erase(it);
				return i;
			}
			++i;
		}
	}
	return -1;
}

eastl::shared_ptr<NodeAnimator> Node::DetachAnimatorAt(int i)
{
	if (0 <= i && i < static_cast<int>(mAnimators.size()))
	{
		SceneNodeAnimatorList::iterator itAnimator = mAnimators.begin() + i;
		if (itAnimator != mAnimators.end())
			mAnimators.erase(itAnimator);
		return (*itAnimator);
	}
	return nullptr;
}

void Node::DetachAllAnimators()
{
	mAnimators.clear();
}

eastl::shared_ptr<NodeAnimator> Node::SetAnimator(int i, eastl::shared_ptr<NodeAnimator> const& animator)
{
	int const numAnimators = static_cast<int>(mAnimators.size());
	if (0 <= i && i < numAnimators)
	{
		// Remove the animator currently in the slot.
		eastl::shared_ptr<NodeAnimator> previousAnimator = mAnimators[i];

		mAnimators[i] = animator;
		return previousAnimator;
	}

	// The index is out of range, so append the child to the array.
	mAnimators.push_back(animator);
	return nullptr;
}

eastl::shared_ptr<NodeAnimator> Node::GetAnimator(int i)
{
	if (0 <= i && i < static_cast<int>(mAnimators.size()))
	{
		return mAnimators[i];
	}
	return nullptr;
}

//! Enables or disables debug staet.
void Node::SetDebugState(unsigned int debug)
{
	mDebugState = debug;

	if (mDebugState & DM_WIREFRAME)
	{
		for (unsigned int i = 0; i < GetMaterialCount(); i++)
			GetMaterial(i)->mFillMode = RasterizerState::FILL_WIREFRAME;
	}
	else
	{
		for (unsigned int i = 0; i < GetMaterialCount(); i++)
			GetMaterial(i)->mFillMode = RasterizerState::FILL_SOLID;
	}
}


void Node::UpdateWorldData()
{
    Spatial::UpdateWorldData();

    for (auto& child : mChildren)
    {
        if (child)
        {
            child->UpdateWorldData();
        }
    }
}

void Node::UpdateWorldBound()
{
    // Start with visual bound.
    mWorldBound.SetCenter(Vector4<float>::Zero());
	mWorldBound.SetRadius(0.f);
	for (unsigned int v = 0; v < GetVisualCount(); v++)
	{
		BoundingSphere worldBound;
		BoundingSphere modelBound = GetVisual(v)->mModelBound;
		modelBound.TransformBy(mWorldTransform, worldBound);
		mWorldBound.GrowToContain(worldBound);
	}

    for (auto& child : mChildren)
    {
        if (child)
        {
            // GrowToContain ignores invalid child bounds.  If the world
            // bound is invalid and a child bound is valid, the child
            // bound is copied to the world bound.  If the world bound and
            // child bound are valid, the smallest bound containing both
            // bounds is assigned to the world bound.
			mWorldBound.GrowToContain(child->mWorldBound);
        }
    }
}

void Node::GetVisibleSet(Culler& culler, eastl::shared_ptr<Camera> const& camera, bool noCull)
{
    for (auto& child : mChildren)
    {
        if (child)
        {
            child->OnGetVisibleSet(culler, camera, noCull);
        }
    }
}


////////////////////////////////////////////////////
// Node Implementation
////////////////////////////////////////////////////

//
// Node::OnRestore					- Chapter 16, page 532
//
bool Node::OnRestore(Scene *pScene)
{
	//Color color = (mRenderComponent) ? mRenderComponent->GetColor() : gWhite;
	//mMaterial.SetDiffuse(color);

	// This is meant to be called from any class
	// that inherits from SceneNode and overloads
	// OnRestore()
	SceneNodeList::iterator i = mChildren.begin();
	SceneNodeList::iterator end = mChildren.end();
	while (i != end)
	{
		(*i)->OnRestore(pScene);
		++i;
	}
	return true;
}


//
// Node::OnLostDevice					- Chapter X, page Y
//
bool Node::OnLostDevice(Scene *pScene)
{
	// This is meant to be called from any class
	// that inherits from SceneNode and overloads
	// OnRestore()
	SceneNodeList::iterator i = mChildren.begin();
	SceneNodeList::iterator end = mChildren.end();
	while (i != end)
	{
		(*i)->OnLostDevice(pScene);
		++i;
	}
	return true;
}


//
// Node::OnUpdate					- Chapter 16, page 532
//
bool Node::OnUpdate(Scene *pScene, unsigned int timeMs, unsigned long const elapsedMs)
{
	// This is meant to be called from any class
	// that inherits from SceneNode and overloads
	// do animations and other stuff.
	SceneNodeList::iterator i = mChildren.begin();
	SceneNodeList::iterator end = mChildren.end();

	while (i != end)
	{
		(*i)->OnUpdate(pScene, timeMs, elapsedMs);
		++i;
	}

	// update spatial data
	Spatial::UpdateWorldData();

	// animate this node
	OnAnimate(pScene, timeMs);

	//update bounds
	UpdateWorldBound();

	return true;
}


//! OnAnimate() is called just before rendering the whole scene.
/** Nodes may calculate or store animations here, and may do other useful things,
depending on what they are. Also, OnAnimate() should be called for all
child scene nodes here. This method will be called once per frame, independent
of whether the scene node is visible or not.
\param timeMs Current time in milliseconds. */
bool Node::OnAnimate(Scene* pScene, unsigned int timeMs)
{
	if (IsVisible())
	{
		// animate this node with all animators

		SceneNodeAnimatorList::iterator ait = mAnimators.begin();
		while (ait != mAnimators.end())
		{
			// continue to the next node before calling animateNode()
			// so that the animator may remove itself from the scene
			// node without the iterator becoming invalid
			const eastl::shared_ptr<NodeAnimator>& anim = *ait;
			++ait;
			anim->AnimateNode(pScene, this, timeMs);
		}
	}

	return true;
}

//
// Node::PreRender					- Chapter 16, page 532
//
bool Node::PreRender(Scene *pScene)
{
	// This is meant to be called from any class
	// that inherits from SceneNode and overloads
	// PreRender()

	SceneNodeList::iterator i = mChildren.begin();
	SceneNodeList::iterator end = mChildren.end();

	while (i != end)
	{
		(*i)->PreRender(pScene);
		++i;
	}

	return true;
}

//! Sort the nodes which are going to be rendered
void Node::SortRenderList(Scene* pScene)
{
	//LIGHT NODES
	if (!pScene->GetLightManager())
	{
		SceneNodeRenderList& renderList = pScene->GetRenderList(RP_LIGHT);

		// Sort the lights by distance from the camera
		Vector4<float> camWorldPos;
		if (pScene->GetActiveCamera())
			camWorldPos = pScene->GetActiveCamera()->Get()->GetPosition();

		eastl::vector<DistanceNodeEntry> SortedLights;
		for (int light = renderList.size() - 1; light >= 0; --light)
			SortedLights.push_back(DistanceNodeEntry(renderList[light], camWorldPos));

		eastl::sort(SortedLights.begin(), SortedLights.end());

		for (int light = (int)renderList.size() - 1; light >= 0; --light)
			renderList[light] = SortedLights[light].mNode;
	}

	//SOLID NODES
	{
		SceneNodeRenderList& renderList = pScene->GetRenderList(RP_SOLID);

		// Sort the solids by texture id
		eastl::vector<DefaultNodeEntry> SortedSolids;
		for (int solid = renderList.size() - 1; solid >= 0; --solid)
			SortedSolids.push_back(DefaultNodeEntry(renderList[solid]));

		eastl::sort(SortedSolids.begin(), SortedSolids.end());

		for (int solid = (int)renderList.size() - 1; solid >= 0; --solid)
			renderList[solid] = SortedSolids[solid].mNode;
	}

	//TRANSPARENT NODES
	{
		SceneNodeRenderList& renderList = pScene->GetRenderList(RP_TRANSPARENT);

		// Sort the transparent nodes by distance from the camera
		Vector4<float> camWorldPos;
		if (pScene->GetActiveCamera())
			camWorldPos = pScene->GetActiveCamera()->Get()->GetPosition();

		eastl::vector<TransparentNodeEntry> SortedTransparent;
		for (int transparent = renderList.size() - 1; transparent >= 0; --transparent)
			SortedTransparent.push_back(TransparentNodeEntry(renderList[transparent], camWorldPos));

		eastl::sort(SortedTransparent.begin(), SortedTransparent.end());

		for (int transparent = (int)renderList.size() - 1; transparent >= 0; --transparent)
			renderList[transparent] = SortedTransparent[transparent].mNode;
	}

	//TRANSPARENT EFFECT NODES
	{
		SceneNodeRenderList& renderList = pScene->GetRenderList(RP_TRANSPARENT_EFFECT);

		// Sort the transparent nodes by distance from the camera
		Vector4<float> camWorldPos;
		if (pScene->GetActiveCamera())
			camWorldPos = pScene->GetActiveCamera()->Get()->GetPosition();

		eastl::vector<TransparentNodeEntry> SortedTransparent;
		for (int transparent = renderList.size() - 1; transparent >= 0; --transparent)
			SortedTransparent.push_back(TransparentNodeEntry(renderList[transparent], camWorldPos));

		eastl::sort(SortedTransparent.begin(), SortedTransparent.end());

		for (int transparent = (int)renderList.size() - 1; transparent >= 0; --transparent)
			renderList[transparent] = SortedTransparent[transparent].mNode;
	}
}

//
// Node::PostRender					- Chapter 16, page 532
//
bool Node::PostRender(Scene *pScene)
{
	return true;
}

//
// Node::RenderChildren					- Chapter 16, page 533
//
bool Node::RenderChildren(Scene *pScene)
{
	// Iterate through the children....
	SceneNodeList::iterator i = mChildren.begin();
	SceneNodeList::iterator end = mChildren.end();

	while (i != end)
	{
		Node* node = (*i).get();

		if (pScene->GetLightManager())
			pScene->GetLightManager()->OnNodePreRender(node);

		node->Render(pScene);
		node->RenderChildren(pScene);
				
		if (pScene->GetLightManager())
			pScene->GetLightManager()->OnNodePostRender(node);
	}

	return true;
}