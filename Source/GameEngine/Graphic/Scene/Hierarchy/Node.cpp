// David Eberly, Geometric Tools, Redmond WA 98052
// Copyright (c) 1998-2017
// Distributed under the Boost Software License, Version 1.0.
// http://www.boost.org/LICENSE_1_0.txt
// http://www.geometrictools.com/License/Boost/LICENSE_1_0.txt
// File Version: 3.0.0 (2016/06/19)

#include "Node.h"

Node::~Node()
{
    for (auto& child : mChild)
    {
        if (child)
        {
            child->SetParent(nullptr);
            child = nullptr;
        }
    }
}

Node::Node()
{
}

int Node::GetNumChildren() const
{
    return static_cast<int>(mChild.size());
}

int Node::AttachChild(eastl::shared_ptr<Spatial> const& child)
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
    for (auto& current : mChild)
    {
        if (!current)
        {
            current = child;
            return i;
        }
        ++i;
    }

    // All slots are used, so append the child to the array.
    int const numChildren = static_cast<int>(mChild.size());
    mChild.push_back(child);
    return numChildren;
}

int Node::DetachChild(eastl::shared_ptr<Spatial> const& child)
{
    if (child)
    {
        int i = 0;
        for (auto& current : mChild)
        {
            if (current == child)
            {
                current->SetParent(nullptr);
                current = nullptr;
                return i;
            }
            ++i;
        }
    }
    return -1;
}

eastl::shared_ptr<Spatial> Node::DetachChildAt(int i)
{
    if (0 <= i && i < static_cast<int>(mChild.size()))
    {
		eastl::shared_ptr<Spatial> child = mChild[i];
        if (child)
        {
            child->SetParent(nullptr);
            mChild[i] = nullptr;
        }
        return child;
    }
    return nullptr;
}

void Node::DetachAllChildren()
{
    for (auto& current : mChild)
    {
        DetachChild(current);
    }
}

eastl::shared_ptr<Spatial> Node::SetChild(int i, eastl::shared_ptr<Spatial> const& child)
{
    if (child)
    {
        LogAssert(!child->GetParent(), "The child already has a parent.");
    }

    int const numChildren = static_cast<int>(mChild.size());
    if (0 <= i && i < numChildren)
    {
        // Remove the child currently in the slot.
		eastl::shared_ptr<Spatial> previousChild = mChild[i];
        if (previousChild)
        {
            previousChild->SetParent(nullptr);
        }

        // Insert the new child in the slot.
        if (child)
        {
            child->SetParent(this);
        }

        mChild[i] = child;
        return previousChild;
    }

    // The index is out of range, so append the child to the array.
    if (child)
    {
        child->SetParent(this);
    }
    mChild.push_back(child);
    return nullptr;
}

eastl::shared_ptr<Spatial> Node::GetChild(int i)
{
    if (0 <= i && i < static_cast<int>(mChild.size()))
    {
        return mChild[i];
    }
    return nullptr;
}

void Node::UpdateWorldData(double applicationTime)
{
    Spatial::UpdateWorldData(applicationTime);

    for (auto& child : mChild)
    {
        if (child)
        {
            child->Update(applicationTime, false);
        }
    }
}

void Node::UpdateWorldBound()
{
    if (!worldBoundIsCurrent)
    {
        // Start with an invalid bound.
        worldBound.SetCenter({ 0.0f, 0.0f, 0.0f, 1.0f });
        worldBound.SetRadius(0.0f);

        for (auto& child : mChild)
        {
            if (child)
            {
                // GrowToContain ignores invalid child bounds.  If the world
                // bound is invalid and a child bound is valid, the child
                // bound is copied to the world bound.  If the world bound and
                // child bound are valid, the smallest bound containing both
                // bounds is assigned to the world bound.
                worldBound.GrowToContain(child->worldBound);
            }
        }
    }
}

void Node::GetVisibleSet(Culler& culler, eastl::shared_ptr<Camera> const& camera, bool noCull)
{
    for (auto& child : mChild)
    {
        if (child)
        {
            child->OnGetVisibleSet(culler, camera, noCull);
        }
    }
}


////////////////////////////////////////////////////
// SceneNodeProperties Implementation
////////////////////////////////////////////////////

SceneNodeProperties::SceneNodeProperties(void)
{
	m_ActorId = INVALID_ACTOR_ID;
	m_Radius = 0;
	m_RenderPass = ERP_NONE;
	//m_AlphaType = AlphaOpaque;
	m_AutomaticCullingState = EAC_OFF;
	m_DebugDataVisible = EDS_OFF;
	m_Position = Vector3<float>();
	m_Rotation = Vector3<float>();
	m_Scale = Vector3<float>(1.f, 1.f, 1.f);
}


//
// SceneNodeProperties::Transform			- Chapter 16, page 528
//
void SceneNodeProperties::Transform(matrix4 *toWorld, matrix4 *fromWorld) const
{
	if (toWorld)
		*toWorld = m_ToWorld;

	if (fromWorld)
		*fromWorld = m_FromWorld;
}


////////////////////////////////////////////////////
// SceneNode Implementation
////////////////////////////////////////////////////

SceneNode::SceneNode(ActorId actorId, WeakBaseRenderComponentPtr renderComponent, E_RENDER_PASS renderPass,
	E_SCENE_NODE_TYPE nodeType, const matrix4 *to, const matrix4 *from)
	: m_pParent(0)
{
	m_Props.m_Id = -1;
	m_Props.m_ActorId = actorId;
	m_Props.m_Name = (renderComponent) ? renderComponent->GetName() : "SceneNode";
	m_Props.m_RenderPass = renderPass;
	//m_Props.m_AlphaType = AlphaOpaque;
	m_Props.m_Type = nodeType;
	m_RenderComponent = renderComponent;
	m_Props.m_IsVisible = true;
	SetTransform(to, from);

	m_Props.SetRadius(0);

	// [mrmike] - these lines were moved to VOnRestore() post press
	//Color color = (renderComponent) ? renderComponent->GetColor() : g_White;
	//m_Props.m_Material.SetDiffuse(color);
}


SceneNode::~SceneNode()
{
	// delete all children
	RemoveAll();

	// delete all animators
	m_Animators.clear();
}

//
// SceneNode::OnRestore					- Chapter 16, page 532
//
bool SceneNode::OnRestore(Scene *pScene)
{
	//Color color = (m_RenderComponent) ? m_RenderComponent->GetColor() : g_White;
	//m_Props.m_Material.SetDiffuse(color);

	// This is meant to be called from any class
	// that inherits from SceneNode and overloads
	// VOnRestore()

	SceneNodeList::iterator i = m_Children.begin();
	SceneNodeList::iterator end = m_Children.end();
	while (i != end)
	{
		(*i)->OnRestore(pScene);
		++i;
	}
	return true;
}


//
// SceneNode::OnLostDevice					- Chapter X, page Y
//
bool SceneNode::OnLostDevice(Scene *pScene)
{
	// This is meant to be called from any class
	// that inherits from SceneNode and overloads
	// VOnRestore()

	SceneNodeList::iterator i = m_Children.begin();
	SceneNodeList::iterator end = m_Children.end();
	while (i != end)
	{
		(*i)->OnLostDevice(pScene);
		++i;
	}
	return true;
}


//
// SceneNode::VOnUpdate					- Chapter 16, page 532
//
bool SceneNode::OnUpdate(Scene *pScene, unsigned long const elapsedMs)
{
	// This is meant to be called from any class
	// that inherits from SceneNode and overloads
	// do animations and other stuff.

	SceneNodeList::iterator i = m_Children.begin();
	SceneNodeList::iterator end = m_Children.end();

	while (i != end)
	{
		(*i)->OnUpdate(pScene, elapsedMs);
		++i;
	}
	return true;
}


//! OnAnimate() is called just before rendering the whole scene.
/** Nodes may calculate or store animations here, and may do other useful things,
depending on what they are. Also, OnAnimate() should be called for all
child scene nodes here. This method will be called once per frame, independent
of whether the scene node is visible or not.
\param timeMs Current time in milliseconds. */
bool SceneNode::OnAnimate(Scene* pScene, u32 timeMs)
{
	if (m_Props.IsVisible())
	{
		// animate this node with all animators

		SceneNodeAnimatorList::iterator ait = m_Animators.begin();
		while (ait != m_Animators.end())
		{
			// continue to the next node before calling animateNode()
			// so that the animator may remove itself from the scene
			// node without the iterator becoming invalid
			const shared_ptr<SceneNodeAnimator>& anim = *ait;
			++ait;
			anim->AnimateNode(this, timeMs);
		}

		// update absolute transformation
		UpdateAbsoluteTransformation();

		// perform the post render process on all children

		SceneNodeList::iterator it = m_Children.begin();
		for (; it != m_Children.end(); ++it)
			(*it)->OnAnimate(pScene, timeMs);
	}

	return true;
}

//! Updates the absolute position based on the relative and the parents position
/** Note: This does not recursively update the parents absolute positions, so if
you have a deeper hierarchy you might want to update the parents first.*/
void SceneNode::UpdateAbsoluteTransformation()
{

	if (m_pParent && m_pParent->Get()->GetType() != ESNT_ROOT)
	{
		SetTransform(
			&(m_pParent->Get()->ToWorld() * GetRelativeTransformation()));
	}
	else
		SetTransform(&GetRelativeTransformation());
}


//! Removes all children of this scene node
/** The scene nodes found in the children list are also dropped
and might be deleted if no other grab exists on them.
*/
void SceneNode::RemoveAll()
{
	SceneNodeList::iterator it = m_Children.begin();
	for (; it != m_Children.end(); ++it)
		(*it)->m_pParent = 0;

	m_Children.clear();
}


//! Removes this scene node from the scene
void SceneNode::Remove()
{
	if (m_pParent)
		m_pParent->RemoveChild(m_Props.ActorId());
}

//! Changes the parent of the scene node.
/** \param newParent The new parent to be used. */
void SceneNode::SetParent(const shared_ptr<SceneNode>& newParent)
{
	Remove();

	m_pParent = newParent;
}

//
// SceneNode::SetTransform					- Chapter 16, page 531
//
void SceneNode::SetTransform(const matrix4 *toWorld, const matrix4 *fromWorld)
{
	m_Props.m_ToWorld = *toWorld;
	if (!fromWorld)
	{
		m_Props.m_ToWorld.GetInverse(m_Props.m_FromWorld);
	}
	else
	{
		m_Props.m_FromWorld = *fromWorld;
	}
}

//
// SceneNode::PreRender					- Chapter 16, page 532
//
bool SceneNode::PreRender(Scene *pScene)
{
	// This was added post press! Is is always ok to read directly from the game logic.
	StrongActorPtr pActor =
		MakeStrongPtr(g_pGameApp->GetGameLogic()->GetActor(m_Props.m_ActorId));
	if (pActor)
	{
		const shared_ptr<TransformComponent>& pTc =
			MakeStrongPtr(pActor->GetComponent<TransformComponent>(TransformComponent::g_Name));
		if (pTc)
		{
			m_Props.m_ToWorld = pTc->GetTransform();
		}
	}

	// This is meant to be called from any class
	// that inherits from SceneNode and overloads
	// PreRender()

	SceneNodeList::iterator i = m_Children.begin();
	SceneNodeList::iterator end = m_Children.end();

	while (i != end)
	{
		(*i)->PreRender(pScene);
		++i;
	}

	return true;
}

//! Sort the nodes which are going to be rendered
void SceneNode::SortRenderList(Scene* pScene)
{
	//LIGHT NODES
	if (!pScene->GetLightManager())
	{
		SceneNodeRenderList& renderList = pScene->GetRenderList(ERP_LIGHT);

		// Sort the lights by distance from the camera
		Vector3<float> camWorldPos;
		if (pScene->GetActiveCamera())
			camWorldPos = pScene->GetActiveCamera()->GetPosition();

		eastl::vector<DistanceNodeEntry> SortedLights;
		for (int light = renderList.size() - 1; light >= 0; --light)
			SortedLights.push_back(DistanceNodeEntry(renderList[light], camWorldPos));

		eastl::sort(SortedLights.begin(), SortedLights.end());

		for (int light = (int)renderList.size() - 1; light >= 0; --light)
			renderList[light] = SortedLights[light].Node;
	}

	//SOLID NODES
	{
		SceneNodeRenderList& renderList = pScene->GetRenderList(ERP_SOLID);

		// Sort the solids by texture id
		eastl::vector<DefaultNodeEntry> SortedSolids;
		for (int solid = renderList.size() - 1; solid >= 0; --solid)
			SortedSolids.push_back(DefaultNodeEntry(renderList[solid]));

		eastl::sort(SortedSolids.begin(), SortedSolids.end());

		for (int solid = (int)renderList.size() - 1; solid >= 0; --solid)
			renderList[solid] = SortedSolids[solid].Node;
	}

	//TRANSPARENT NODES
	{
		SceneNodeRenderList& renderList = pScene->GetRenderList(ERP_TRANSPARENT);

		// Sort the transparent nodes by distance from the camera
		Vector3<float> camWorldPos;
		if (pScene->GetActiveCamera())
			camWorldPos = pScene->GetActiveCamera()->Get()->ToWorld().GetTranslation();

		eastl::vector<TransparentNodeEntry> SortedTransparent;
		for (int transparent = renderList.size() - 1; transparent >= 0; --transparent)
			SortedTransparent.push_back(TransparentNodeEntry(renderList[transparent], camWorldPos));

		eastl::sort(SortedTransparent.begin(), SortedTransparent.end());

		for (int transparent = (int)renderList.size() - 1; transparent >= 0; --transparent)
			renderList[transparent] = SortedTransparent[transparent].Node;
	}

	//TRANSPARENT EFFECT NODES
	{
		SceneNodeRenderList& renderList = pScene->GetRenderList(ERP_TRANSPARENT_EFFECT);

		// Sort the transparent nodes by distance from the camera
		Vector3<float> camWorldPos;
		if (pScene->GetActiveCamera())
			camWorldPos = pScene->GetActiveCamera()->Get()->ToWorld().GetTranslation();

		eastl::vector<TransparentNodeEntry> SortedTransparent;
		for (int transparent = renderList.size() - 1; transparent >= 0; --transparent)
			SortedTransparent.push_back(TransparentNodeEntry(renderList[transparent], camWorldPos));

		eastl::sort(SortedTransparent.begin(), SortedTransparent.end());

		for (int transparent = (int)renderList.size() - 1; transparent >= 0; --transparent)
			renderList[transparent] = SortedTransparent[transparent].Node;
	}
}

//
// SceneNode::PostRender					- Chapter 16, page 532
//
bool SceneNode::PostRender(Scene *pScene)
{
	return true;
}

//
// SceneNode::GetWorldPosition			- not described in the book
//
//   This was added post press to respect any SceneNode ancestors - you have to add all 
//   their positions together to get the world position of any SceneNode.
//
/*
const Vector3<float> SceneNode::GetWorldPosition() const
{
Vector3<float> pos = GetPosition();
if (m_pParent)
{
pos += m_pParent->GetWorldPosition();
}
return pos;
}
*/

/*
//
// SceneNode::Render					- Chapter 16, page Y
//
bool D3DSceneNode9::Render(Scene *pScene)
{
m_Props.GetMaterial().D3DUse9();

switch (m_Props.AlphaType())
{
case AlphaTexture:
// Nothing to do here....
break;

case AlphaMaterial:
DXUTGetD3D9Device()->SetRenderState( D3DRS_COLORVERTEX, true);
DXUTGetD3D9Device()->SetRenderState( D3DRS_DIFFUSEMATERIALSOURCE, D3DMCS_MATERIAL );
break;

case AlphaVertex:
GE_ASSERT(0 && _T("Not implemented!"));
break;
}

return true;
}
*/
//
// SceneNode::RenderChildren					- Chapter 16, page 533
//
bool SceneNode::RenderChildren(Scene *pScene)
{
	// Iterate through the children....
	SceneNodeList::iterator i = m_Children.begin();
	SceneNodeList::iterator end = m_Children.end();

	//LightManager* lightManager = pScene->GetLightManager();

	while (i != end)
	{
		SceneNode* node = (*i).get();
		if (node->PreRender(pScene) == true)
		{
			// You could short-circuit rendering
			// if an object returns E_FAIL from
			// PreRender()

			// Don't render this node if you can't see it
			bool CurrentRenderPass =
				pScene->GetCurrentRenderPass() == node->Get()->GetRenderPass();
			if (CurrentRenderPass && !pScene->IsCulled(node))
			{
				/*
				float alpha = node->Get()->m_Material.GetAlpha();

				if (alpha==fOPAQUE)
				{
				(*i)->VRender(pScene);
				}
				else if (alpha!=fTRANSPARENT)
				{
				// The object isn't totally transparent...
				AlphaSceneNode *asn = new AlphaSceneNode;
				GE_ASSERT(asn);
				asn->m_pNode = *i;
				asn->m_Concat = pScene->GetTopMatrix();

				Vec4 worldPos(asn->m_Concat.GetPosition());

				matrix4 fromWorld = pScene->GetActiveCamera()->VGet()->FromWorld();

				Vec4 screenPos = fromWorld.Xform(worldPos);

				asn->m_ScreenZ = screenPos.z;

				pScene->AddAlphaSceneNode(asn);
				}
				*/

				//if (lightManager)
				//	lightManager->OnNodePreRender(node);

				node->Render(pScene);
				node->RenderChildren(pScene);

				//if (lightManager)
				//	lightManager->OnNodePostRender(node);
			}
		}
		node->PostRender(pScene);
		++i;
	}

	return true;
}


//
// SceneNode::AddChild					- Chapter 16, page 535
//
bool SceneNode::AddChild(const shared_ptr<SceneNode>& ikid)
{
	m_Children.push_back(ikid);

	const shared_ptr<SceneNode>& kid = static_pointer_cast<SceneNode>(ikid);
	kid->SetParent(shared_from_this());

	// The radius of the sphere should be fixed right here
	Vector3<float> kidPos = kid->Get()->ToWorld().GetTranslation();

	//	Post-press fix. This was not correct! subtracting the parents's position from 
	//	the kidPos created a HUGE radius, depending on the location of the parent, which 
	//	could be anywhere in the game world.

	//Vector3<float> dir = kidPos - m_Props.ToWorld().GetPosition();
	//float newRadius = dir.Length() + kid->VGet()->Radius();

	float newRadius = kidPos.GetLength() + kid->Get()->GetRadius();

	if (newRadius > m_Props.m_Radius)
		m_Props.m_Radius = newRadius;

	return true;
}

//
// SceneNode::RemoveChild					- not in the book
//
//   If an actor is destroyed it should be removed from the scene graph.
//   Generally the HumanView will recieve a message saying the actor has been
//   destroyed, and it will then call RemoveChild which will traverse 
//   the scene graph to find the child that needs removing.
//
bool SceneNode::RemoveChild(ActorId id)
{
	for (SceneNodeList::iterator i = m_Children.begin(); i != m_Children.end(); ++i)
	{
		const SceneNodeProperties* pProps = (*i)->Get();
		if (pProps->ActorId() != INVALID_ACTOR_ID && id == pProps->ActorId())
		{
			i = m_Children.erase(i);	//this can be expensive for vectors
			return true;
		}
	}
	return false;
}

//
// SceneNode::GetChild
//
shared_ptr<SceneNode> SceneNode::GetChild(ActorId id)
{
	for (SceneNodeList::iterator i = m_Children.begin(); i != m_Children.end(); ++i)
	{
		const SceneNodeProperties* pProps = (*i)->Get();
		if (pProps->ActorId() != INVALID_ACTOR_ID && id == pProps->ActorId())
			return (*i);
	}
	return 0;
}


//
// SceneNode::AddAnimator
//
//! Adds an animator which should animate this node.
/** \param animator A pointer to the new animator.*/
bool SceneNode::AddAnimator(const shared_ptr<SceneNodeAnimator>& animator)
{
	m_Animators.push_back(animator);
	return true;
}

//
// SceneNode::RemoveAnimator
//
//   If an animator is destroyed it should be removed from the scene graph.
//   Generally the HumanView will recieve a message saying the animator has been
//   destroyed, and it will then call RemoveAnimator which will traverse 
//   the scene graph to find the animator that needs removing.
//
bool SceneNode::RemoveAnimator(const shared_ptr<SceneNodeAnimator>& animator)
{
	for (SceneNodeAnimatorList::iterator i = m_Animators.begin(); i != m_Animators.end(); ++i)
	{
		if (animator == (*i))
		{
			i = m_Animators.erase(i);	//this can be expensive for vectors
			return true;
		}
	}
	return false;
}


bool SceneNode::Pick(Scene *pScene, RayCast *raycast)
{
	for (SceneNodeList::const_iterator i = m_Children.begin(); i != m_Children.end(); ++i)
	{
		bool hr = (*i)->Pick(pScene, raycast);

		if (hr == false)
			return false;
	}

	return true;
}

// This was changed post press - it was convenient to be able to set alpha on a parent node and
// have it propagate to all its children, since materials are set in the SceneNodes's properties, 
// and not as a SceneNode that changes renderstate by itself.
/*
void SceneNode::SetAlpha(float alpha)
{
m_Props.SetAlpha(alpha);
for(SceneNodeList::const_iterator i=m_Children.begin(); i!=m_Children.end(); ++i)
{
shared_ptr<SceneNode> sceneNode = static_pointer_cast<SceneNode>(*i);
sceneNode->SetAlpha(alpha);
}
}
*/


////////////////////////////////////////////////////
// RootNode Implementation
////////////////////////////////////////////////////

//
// RootNode::RootNode					- Chapter 16, page 545
//
RootNode::RootNode()
	: SceneNode(INVALID_ACTOR_ID, WeakBaseRenderComponentPtr(),
		ERP_NONE, ESNT_ROOT, &g_IdentityMatrix4)
{

}

//
// RootNode::PreRender
//
bool RootNode::PreRender(Scene *pScene)
{
	const shared_ptr<IRenderer>& renderer = pScene->GetRenderer();

	// reset all transforms
	renderer->SetMaterial(Material());
	renderer->SetTransform(ETS_PROJECTION, g_IdentityMatrix4);
	renderer->SetTransform(ETS_VIEW, g_IdentityMatrix4);
	renderer->SetTransform(ETS_WORLD, g_IdentityMatrix4);
	for (u32 i = ETS_COUNT - 1; i >= ETS_TEXTURE_0; --i)
		renderer->SetTransform((E_TRANSFORMATION_STATE)i, g_IdentityMatrix4);
	renderer->SetAllowZWriteOnTransparent(true);

	// first scene node for prerendering should be the active camera
	// consistent Camera is needed for culling
	if (pScene->GetActiveCamera())
		pScene->GetActiveCamera()->Render(pScene);

	bool success = SceneNode::PreRender(pScene);

	if (success)
		SceneNode::SortRenderList(pScene);

	return success;
}

//
// RootNode::RenderChildren					- Chapter 16, page 547
//
bool RootNode::RenderChildren(Scene *pScene)
{
	// Iterate through the render children....
	for (int pass = 0; pass < ERP_LAST; pass++)
	{
		SceneNodeRenderList::iterator itNode = pScene->GetRenderList(pass).begin();
		SceneNodeRenderList::iterator end = pScene->GetRenderList(pass).end();

		pScene->GetRenderer()->GetOverrideMaterial().m_Enabled =
			((pScene->GetRenderer()->GetOverrideMaterial().m_EnablePasses & pass) != 0);

		if (pScene->GetLightManager())
			pScene->GetLightManager()->OnRenderPassPreRender((E_RENDER_PASS)pass);

		if (pass == ERP_LIGHT)
		{
			pScene->GetRenderer()->DeleteAllDynamicLights();
			pScene->GetRenderer()->SetAmbientLight(pScene->GetAmbientLight());

			u32 maxLights = pScene->GetRenderList(pass).size();
			if (!pScene->GetLightManager())
				end = itNode +
				min_(pScene->GetRenderer()->GetMaximalDynamicLightAmount(), maxLights);
		}

		// This code creates fine control of the render passes.
		for (; itNode != end; ++itNode)
		{
			if (pScene->GetLightManager())
				pScene->GetLightManager()->OnNodePreRender((*itNode));
			(*itNode)->Render(pScene);
			if (pScene->GetLightManager())
				pScene->GetLightManager()->OnNodePostRender((*itNode));

		}

		if (pass == ERP_SHADOW)
		{
			if (!pScene->GetRenderList(pass).empty())
				pScene->GetRenderer()->DrawStencilShadow(true,
					pScene->GetShadowColor(), pScene->GetShadowColor(),
					pScene->GetShadowColor(), pScene->GetShadowColor());
		}

		if (pScene->GetLightManager())
			pScene->GetLightManager()->OnRenderPassPostRender((E_RENDER_PASS)pass);
	}

	return true;
}

//
// RootNode::Render
//
bool RootNode::Render(Scene* pScene)
{

	return RenderChildren(pScene);
}

//
// RootNode::Render
//
bool RootNode::PostRender(Scene* pScene)
{
	pScene->ClearRenderList();
	pScene->ClearDeletionList();

	pScene->SetCurrentRenderPass(ERP_NONE);

	return SceneNode::PostRender(pScene);
}


//! returns the axis aligned bounding box of this node
const AABBox3<float>& RootNode::GetBoundingBox() const
{
	// should never be used.
	return *((AABBox3<float>*)0);
}
