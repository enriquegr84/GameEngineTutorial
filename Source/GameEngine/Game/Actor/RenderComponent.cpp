 
//========================================================================
// RenderComponent.cpp : classes that define renderable components of actors like Meshes, Skies, Lights, etc.
// Author: David "Rez" Graham
//
// Part of the GameEngine Application
//
// GameEngine is the sample application that encapsulates much of the source code
// discussed in "Game Coding Complete - 4th Edition" by Mike McShaffry and David
// "Rez" Graham, published by Charles River Media. 
// ISBN-10: 1133776574 | ISBN-13: 978-1133776574
//
// If this source code has found it's way to you, and you think it has helped you
// in any way, do the authors a favor and buy a new copy of the book - there are 
// detailed explanations in it that compliment this code well. Buy a copy at Amazon.com
// by clicking here: 
//    http://www.amazon.com/gp/product/1133776574/ref=olp_product_details?ie=UTF8&me=&seller=
//
// There's a companion web site at http://www.mcshaffry.com/GameCode/
// 
// The source code is managed and maintained through Google Code: 
//    http://code.google.com/p/GameEngine/
//
// (c) Copyright 2012 Michael L. McShaffry and David Graham
//
// This program is free software; you can redistribute it and/or
// modify it under the terms of the GNU Lesser GPL v3
// as published by the Free Software Foundation.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See 
// http://www.gnu.org/licenses/lgpl-3.0.txt for more details.
//
// You should have received a copy of the GNU Lesser GPL v3
// along with this program; if not, write to the Free Software
// Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
//
//========================================================================

#include "RenderComponent.h"

#include "Graphic/Graphic.h"
#include "Core/Logger/Logger.h"

#include "Game/Actor/Actor.h"
#include "Game/Actor/TransformComponent.h"

#include "Application/GameApplication.h"

const char* MeshRenderComponent::Name = "MeshRenderComponent";
const char* SphereRenderComponent::Name = "SphereRenderComponent";
const char* GridRenderComponent::Name = "GridRenderComponent";
const char* LightRenderComponent::Name = "LightRenderComponent";
const char* SkyRenderComponent::Name = "SkyRenderComponent";
const char* ParticleSystemRenderComponent::Name = "ParticleSystemRenderComponent";

//---------------------------------------------------------------------------------------------------------------------
// MeshRenderComponent
//---------------------------------------------------------------------------------------------------------------------
MeshRenderComponent::MeshRenderComponent(void)
{

}

bool MeshRenderComponent::DelegateInit(tinyxml2::XMLElement* pData)
{
	tinyxml2::XMLElement* pMesh = pData->FirstChildElement("Mesh");

	mMeshModelFile = pMesh->Attribute("model_file");
	mMeshTextureFile = pMesh->Attribute("texture_file");
    //pMesh->Attribute("position", &segments);
	//pMesh->Attribute("rotation", &segments);
	//pMesh->Attribute("scale", &segments);

    return true;
}

eastl::shared_ptr<Node> MeshRenderComponent::CreateSceneNode(void)
{
    // get the transform component
    const eastl::shared_ptr<TransformComponent>& pTransformComponent(
		mOwner->GetComponent<TransformComponent>(TransformComponent::Name).lock());
	if (pTransformComponent)
	{
		GameApplication* gameApp = (GameApplication*)Application::App;
		const eastl::shared_ptr<ScreenElementScene>& pScene = gameApp->GetHumanView()->mScene;
		Transform transform = pTransformComponent->GetTransform();
		WeakBaseRenderComponentPtr wbrcp(
			eastl::dynamic_shared_pointer_cast<BaseRenderComponent>(shared_from_this()));

		if (gameApp->mOption.mRendererType == RT_DIRECT3D11)
		{
			eastl::shared_ptr<ResHandle>& resHandle =
				ResCache::Get()->GetHandle(&BaseResource(ToWideString(mMeshModelFile.c_str())));
			if (resHandle)
			{
				const eastl::shared_ptr<MeshResourceExtraData>& extra =
					eastl::static_pointer_cast<MeshResourceExtraData>(resHandle->GetExtra());
				eastl::shared_ptr<BaseAnimatedMesh> animatedMesh(extra->GetMesh());
				// create an animated mesh scene node with specified mesh.
				eastl::shared_ptr<Node> animatedMeshNode =
					pScene->AddAnimatedMeshNode(wbrcp, 0, animatedMesh, mOwner->GetId());

				//To let the mesh look a little bit nicer, we change its material. We
				//disable lighting because we do not have a dynamic light in here, and
				//the mesh would be totally black otherwise. And last, we apply a
				//texture to the mesh. Without it the mesh would be drawn using only a
				//color.

				if (animatedMeshNode)
				{
					animatedMeshNode->GetRelativeTransform() = transform;

					resHandle =
						ResCache::Get()->GetHandle(&BaseResource(ToWideString(mMeshTextureFile.c_str())));
					if (resHandle)
					{
						const eastl::shared_ptr<ImageResourceExtraData>& extra =
							eastl::static_pointer_cast<ImageResourceExtraData>(resHandle->GetExtra());

						animatedMeshNode->SetMaterialFlag(MF_LIGHTING, false);
						animatedMeshNode->SetMaterialTexture(0, extra->GetImage().get());
					}
				}

				return animatedMeshNode;
			}
		}
		else LogAssert(nullptr, "Unknown Renderer Implementation in MeshRenderComponent::CreateSceneNode");
	}
	return eastl::shared_ptr<Node>();
}

void MeshRenderComponent::CreateInheritedXMLElements(
	tinyxml2::XMLDocument doc, tinyxml2::XMLElement* pBaseElement)
{
	LogError("MeshRenderComponent::GenerateSubclassXml() not implemented");
}

//---------------------------------------------------------------------------------------------------------------------
// SphereRenderComponent
//---------------------------------------------------------------------------------------------------------------------
SphereRenderComponent::SphereRenderComponent(void)
{
    mSegments = 0;
	mTextureResource = "";
}

bool SphereRenderComponent::DelegateInit(tinyxml2::XMLElement* pData)
{
	tinyxml2::XMLElement* pSphere = pData->FirstChildElement("Sphere");
	if (pSphere)
	{
		int segments = 50;
		float radius = 1.0;
		radius = pSphere->FloatAttribute("radius", radius);
		segments = pSphere->IntAttribute("segments", segments);
		mRadius = (float)radius;
		mSegments = (unsigned int)segments;
	}

	tinyxml2::XMLElement* pTexture = pData->FirstChildElement("Texture");
	if (pTexture)
	{
		float x = 1.f;
		float y = 1.f;
		x = pTexture->FloatAttribute("x", x);
		y = pTexture->FloatAttribute("y", y);
		mTextureScale = Vector2<float>{ x, y };
		mTextureResource = pTexture->Attribute("file");
	}

    return true;
}

eastl::shared_ptr<Node> SphereRenderComponent::CreateSceneNode(void)
{
    // get the transform component
    const eastl::shared_ptr<TransformComponent>& pTransformComponent(
		mOwner->GetComponent<TransformComponent>(TransformComponent::Name).lock());
	if (pTransformComponent)
	{
		GameApplication* gameApp = (GameApplication*)Application::App;
		const eastl::shared_ptr<ScreenElementScene>& pScene = gameApp->GetHumanView()->mScene;
		Transform transform = pTransformComponent->GetTransform();
		WeakBaseRenderComponentPtr wbrcp(
			eastl::dynamic_shared_pointer_cast<BaseRenderComponent>(shared_from_this()));

		if (gameApp->mOption.mRendererType == RT_DIRECT3D11)
		{
			// create a sphere node with specified radius and poly count.
			eastl::shared_ptr<Node> sphere =
				pScene->AddSphereNode(wbrcp, nullptr, mRadius, mSegments, mOwner->GetId());
			if (sphere)
			{
				sphere->GetRelativeTransform() = transform;

				eastl::shared_ptr<ResHandle>& resHandle =
					ResCache::Get()->GetHandle(&BaseResource(ToWideString(mTextureResource.c_str())));
				if (resHandle)
				{
					const eastl::shared_ptr<ImageResourceExtraData>& extra =
						eastl::static_pointer_cast<ImageResourceExtraData>(resHandle->GetExtra());

					sphere->SetMaterialTexture(0, extra->GetImage().get());
					sphere->SetMaterialFlag(MF_LIGHTING, false);
				}
			}

			return sphere;
		}
		else LogAssert(nullptr, "Unknown Renderer Implementation in SphereRenderComponent::CreateSceneNode");
	}
	return eastl::shared_ptr<Node>();
}

void SphereRenderComponent::CreateInheritedXMLElements(
	tinyxml2::XMLDocument doc, tinyxml2::XMLElement* pBaseElement)
{
	tinyxml2::XMLElement* pMesh = doc.NewElement("Sphere");
	pMesh->SetAttribute("radius", eastl::to_string(mRadius).c_str());
    pMesh->SetAttribute("segments", eastl::to_string(mSegments).c_str());
    pBaseElement->LinkEndChild(pBaseElement);

	tinyxml2::XMLElement* pTexture = doc.NewElement("Texture");
	pTexture->SetAttribute("file", mTextureResource.c_str());
	pTexture->SetAttribute("x", eastl::to_string(mTextureScale[0]).c_str());
	pTexture->SetAttribute("y", eastl::to_string(mTextureScale[1]).c_str());
	pBaseElement->LinkEndChild(pTexture);
}

//---------------------------------------------------------------------------------------------------------------------
// GridRenderComponent
//---------------------------------------------------------------------------------------------------------------------
GridRenderComponent::GridRenderComponent(void)
{
    mTextureResource = "";
}

bool GridRenderComponent::DelegateInit(tinyxml2::XMLElement* pData)
{
	tinyxml2::XMLElement* pTexture = pData->FirstChildElement("Texture");
    if (pTexture)
	{
		float x = 1.f;
		float y = 1.f;
		x = pTexture->FloatAttribute("x", x);
		y = pTexture->FloatAttribute("y", y);
		mTextureScale = Vector2<float>{ x, y };
		mTextureResource = pTexture->Attribute("file");
	}

	tinyxml2::XMLElement* pSegment = pData->FirstChildElement("Segment");
    if (pSegment)
	{
		int x = 16;
		int y = 16;
		x = pSegment->IntAttribute("x", x);
		y = pSegment->IntAttribute("y", y);
		mSegments = Vector2<int>{ x, y };
	}

	tinyxml2::XMLElement* pExtent = pData->FirstChildElement("Extent");
	if (pExtent)
	{
		float x = 16.f;
		float y = 16.f;
		x = pExtent->FloatAttribute("x", x);
		y = pExtent->FloatAttribute("y", y);
		mExtent = Vector2<float>{ x, y };
	}

    return true;
}

eastl::shared_ptr<Node> GridRenderComponent::CreateSceneNode(void)
{
    const eastl::shared_ptr<TransformComponent>& pTransformComponent(
		mOwner->GetComponent<TransformComponent>(TransformComponent::Name).lock());
    if (pTransformComponent)
    {
		GameApplication* gameApp = (GameApplication*)Application::App;
		const eastl::shared_ptr<ScreenElementScene>& pScene = gameApp->GetHumanView()->mScene;
		Transform transform = pTransformComponent->GetTransform();
		WeakBaseRenderComponentPtr wbrcp(
			eastl::dynamic_shared_pointer_cast<BaseRenderComponent>(shared_from_this()));

		if (gameApp->mOption.mRendererType == RT_DIRECT3D11)
		{
			eastl::shared_ptr<ResHandle>& resHandle =
				ResCache::Get()->GetHandle(&BaseResource(ToWideString(mTextureResource.c_str())));
			if (resHandle)
			{
				const eastl::shared_ptr<ImageResourceExtraData>& extra =
					eastl::static_pointer_cast<ImageResourceExtraData>(resHandle->GetExtra());
				extra->GetImage()->AutogenerateMipmaps();

				// create an animated mesh scene node with specified mesh.
				eastl::shared_ptr<Node> gridNode = pScene->AddRectangleNode(
					wbrcp, 0, extra->GetImage(), mTextureScale[0], mTextureScale[1],
					mExtent[0], mExtent[1], mSegments[0], mSegments[1], mOwner->GetId());

				//To let the mesh look a little bit nicer, we change its material. We
				//disable lighting because we do not have a dynamic light in here, and
				//the mesh would be totally black otherwise. And last, we apply a
				//texture to the mesh. Without it the mesh would be drawn using only a
				//color.
				if (gridNode)
				{
					gridNode->GetRelativeTransform() = transform;

					resHandle =
						ResCache::Get()->GetHandle(&BaseResource(ToWideString(mTextureResource.c_str())));
					if (resHandle)
					{
						const eastl::shared_ptr<ImageResourceExtraData>& extra =
							eastl::static_pointer_cast<ImageResourceExtraData>(resHandle->GetExtra());

						gridNode->SetMaterialFlag(MF_LIGHTING, false);
						gridNode->SetMaterialTexture(0, extra->GetImage().get());
					}

					resHandle =
						ResCache::Get()->GetHandle(&BaseResource(L"Art/t351sml.bmp"));
					if (resHandle)
					{
						const eastl::shared_ptr<ImageResourceExtraData>& extra =
							eastl::static_pointer_cast<ImageResourceExtraData>(resHandle->GetExtra());

						gridNode->SetMaterialTexture(0, extra->GetImage().get());
						gridNode->SetMaterialFlag(MF_LIGHTING, false);
						gridNode->SetMaterialFlag(MF_BACK_FACE_CULLING, true);
					}

				}
				return gridNode;
			}
		}
		else LogError("Unknown Renderer Implementation in GridRenderComponent");
    }

	return eastl::shared_ptr<Node>();
}

void GridRenderComponent::CreateInheritedXMLElements(
	tinyxml2::XMLDocument doc, tinyxml2::XMLElement *pBaseElement)
{
	tinyxml2::XMLElement* pTexture = doc.NewElement("Texture");
	pTexture->SetAttribute("file", mTextureResource.c_str());
	pTexture->SetAttribute("x", eastl::to_string(mTextureScale[0]).c_str());
	pTexture->SetAttribute("y", eastl::to_string(mTextureScale[1]).c_str());
    pBaseElement->LinkEndChild(pTexture);

	tinyxml2::XMLElement* pSegment = doc.NewElement("Segment");
	pSegment->SetAttribute("x", eastl::to_string(mSegments[0]).c_str());
	pSegment->SetAttribute("y", eastl::to_string(mSegments[1]).c_str());
	pBaseElement->LinkEndChild(pSegment);

	tinyxml2::XMLElement* pExtent = doc.NewElement("Extent");
	pExtent->SetAttribute("x", eastl::to_string(mExtent[0]).c_str());
	pExtent->SetAttribute("y", eastl::to_string(mExtent[1]).c_str());
	pBaseElement->LinkEndChild(pExtent);
}

//---------------------------------------------------------------------------------------------------------------------
// LightRenderComponent
//---------------------------------------------------------------------------------------------------------------------
LightRenderComponent::LightRenderComponent(void)
: mAnimatorType("none"), mAddBillboard(false)
{
#if defined(GE_DEV_OPENGL)
	mLightData = eastl::make_shared<Light>(true, false);
#else
	mLightData = eastl::make_shared<Light>(true, true);
#endif
	mLightData->mLighting = eastl::make_shared<Lighting>();
}

bool LightRenderComponent::DelegateInit(tinyxml2::XMLElement* pData)
{
	float temp = 0;

	tinyxml2::XMLElement* pLight = pData->FirstChildElement("Light");
	tinyxml2::XMLElement* pAmbientNode = pLight->FirstChildElement("Ambient");
	if (pAmbientNode)
	{
		temp = pAmbientNode->FloatAttribute("r", temp);
		mLightData->mLighting->mAmbient[0] = temp;

		temp = pAmbientNode->FloatAttribute("g", temp);
		mLightData->mLighting->mAmbient[1] = temp;

		temp = pAmbientNode->FloatAttribute("b", temp);
		mLightData->mLighting->mAmbient[2] = temp;

		temp = pAmbientNode->FloatAttribute("a", temp);
		mLightData->mLighting->mAmbient[3] = temp;
	}

	tinyxml2::XMLElement* pDiffuseNode = pLight->FirstChildElement("Diffuse");
	if (pDiffuseNode)
	{
		temp = pDiffuseNode->FloatAttribute("r", temp);
		mLightData->mLighting->mDiffuse[0] = temp;

		temp = pDiffuseNode->FloatAttribute("g", temp);
		mLightData->mLighting->mDiffuse[1] = temp;

		temp = pDiffuseNode->FloatAttribute("b", temp);
		mLightData->mLighting->mDiffuse[2] = temp;

		temp = pDiffuseNode->FloatAttribute("a", temp);
		mLightData->mLighting->mDiffuse[3] = temp;
	}

	tinyxml2::XMLElement* pSpecularNode = pLight->FirstChildElement("Specular");
	if (pSpecularNode)
	{
		temp = pSpecularNode->FloatAttribute("r", temp);
		mLightData->mLighting->mSpecular[0] = temp;

		temp = pSpecularNode->FloatAttribute("g", temp);
		mLightData->mLighting->mSpecular[1] = temp;

		temp = pSpecularNode->FloatAttribute("b", temp);
		mLightData->mLighting->mSpecular[2] = temp;

		temp = pSpecularNode->FloatAttribute("a", temp);
		mLightData->mLighting->mSpecular[3] = temp;
	}

	tinyxml2::XMLElement* pAttenuationNode = pLight->FirstChildElement("Attenuation");
    if (pAttenuationNode)
	{
		temp = pAttenuationNode->FloatAttribute("const", temp);
		mLightData->mLighting->mAttenuation[0] = temp;

		temp = pAttenuationNode->FloatAttribute("linear", temp);
		mLightData->mLighting->mAttenuation[1] = temp;

		temp = pAttenuationNode->FloatAttribute("exp", temp);
		mLightData->mLighting->mAttenuation[2] = temp;
	}

	tinyxml2::XMLElement* pShapeNode = pLight->FirstChildElement("Shape");
    if (pShapeNode)
	{
		temp = pShapeNode->FloatAttribute("type", temp);
		mLightData->mLighting->mType = static_cast<LightType>((int)temp);

		temp = pShapeNode->FloatAttribute("range", temp);
		mLightData->mLighting->mRadius = temp;

		temp = pShapeNode->FloatAttribute("falloff", temp);
		mLightData->mLighting->mFalloff = temp;

		temp = pShapeNode->FloatAttribute("outercone", temp);
		mLightData->mLighting->mOuterCone = temp;

		temp = pShapeNode->FloatAttribute("innercone", temp);
		mLightData->mLighting->mInnerCone = temp;
	}

	tinyxml2::XMLElement* pAnimator = pData->FirstChildElement("Animator");
	if (pAnimator)
	{
		mAnimatorType = pAnimator->Attribute("type");

		temp = pAnimator->FloatAttribute("x", temp);
		//mAnimatorCenter.X = temp;

		temp = pAnimator->FloatAttribute("y", temp);
		//mAnimatorCenter.Y = temp;

		temp = pAnimator->FloatAttribute("z", temp);
		//mAnimatorCenter.Z = temp;

		temp = pAnimator->FloatAttribute("speed", temp);
		//mAnimatorSpeed = temp;
	}

	tinyxml2::XMLElement* pBillBoard = pData->FirstChildElement("Billboard");
	if (pBillBoard)
	{
		mAddBillboard=true;

		temp = pBillBoard->FloatAttribute("x", temp);
		//mBillboardSize.Width = temp;

		temp = pBillBoard->FloatAttribute("y", temp);
		//mBillboardSize.Height = temp;
	
		tinyxml2::XMLElement* pMaterial = pBillBoard->FirstChildElement("Material");
		if (pMaterial)
		{
			mBillboardMaterial = pMaterial->Attribute("type");
			mBillboardTexture = pMaterial->Attribute("texture");
		}
	}

    return true;
}

eastl::shared_ptr<Node> LightRenderComponent::CreateSceneNode(void)
{
    const eastl::shared_ptr<TransformComponent>& pTransformComponent(
		mOwner->GetComponent<TransformComponent>(TransformComponent::Name).lock());
    if (pTransformComponent)
    {
		GameApplication* gameApp = (GameApplication*)Application::App;
		const eastl::shared_ptr<ScreenElementScene>& pScene = gameApp->GetHumanView()->mScene;
		Transform transform = pTransformComponent->GetTransform();
		WeakBaseRenderComponentPtr wbrcp(
			eastl::dynamic_shared_pointer_cast<BaseRenderComponent>(shared_from_this()));

		if (gameApp->mOption.mRendererType == RT_DIRECT3D11)
		{
			// Add light
			eastl::shared_ptr<Node> light = 
				pScene->AddLightNode(wbrcp, 0, mLightData, mOwner->GetId());

			light->GetRelativeTransform() = transform;

			if (mAnimatorType == "flycircle")
			{
				eastl::shared_ptr<NodeAnimator> anim = 0;
				anim = pScene->CreateFlyCircleAnimator(mAnimatorCenter, mAnimatorSpeed);
				light->AttachAnimator(anim);
			}

			// attach billboard to light
			if (mAddBillboard)
			{
				eastl::shared_ptr<ResHandle>& resHandle =
					ResCache::Get()->GetHandle(&BaseResource(ToWideString(mBillboardTexture.c_str())));
				if (resHandle)
				{
					const eastl::shared_ptr<ImageResourceExtraData>& extra =
						eastl::static_pointer_cast<ImageResourceExtraData>(resHandle->GetExtra());

					eastl::shared_ptr<Node> bill = pScene->AddBillboardNode(eastl::weak_ptr<BaseRenderComponent>(), 
						light, extra->GetImage(), mBillboardSize, INVALID_ACTOR_ID);
					if (bill)
					{
						bill->GetRelativeTransform() = transform;

						bill->SetMaterialFlag(MF_LIGHTING, false);
						bill->SetMaterialType(MT_TRANSPARENT_ADD_COLOR);
						bill->SetMaterialTexture(0, extra->GetImage().get());
					}
				}
			}

			return light;
		}
		else LogError("Unknown Renderer Implementation in LightRenderComponent");
	}
	return eastl::shared_ptr<Node>();
}

void LightRenderComponent::CreateInheritedXMLElements(
	tinyxml2::XMLDocument doc, tinyxml2::XMLElement *pBaseElement)
{

	tinyxml2::XMLElement* pSceneNode = doc.NewElement("Light");

	// Ambient
	tinyxml2::XMLElement* pAmbient = doc.NewElement("Ambient");
	pAmbient->SetAttribute("r", eastl::to_string(mLightData->mLighting->mAmbient[0]).c_str());
	pAmbient->SetAttribute("g", eastl::to_string(mLightData->mLighting->mAmbient[1]).c_str());
	pAmbient->SetAttribute("b", eastl::to_string(mLightData->mLighting->mAmbient[2]).c_str());
	pAmbient->SetAttribute("a", eastl::to_string(mLightData->mLighting->mAmbient[3]).c_str());
	pSceneNode->LinkEndChild(pAmbient);

	// Diffuse
	tinyxml2::XMLElement* pDiffuse = doc.NewElement("Diffuse");
	pDiffuse->SetAttribute("r", eastl::to_string(mLightData->mLighting->mDiffuse[0]).c_str());
	pDiffuse->SetAttribute("g", eastl::to_string(mLightData->mLighting->mDiffuse[1]).c_str());
	pDiffuse->SetAttribute("b", eastl::to_string(mLightData->mLighting->mDiffuse[2]).c_str());
	pDiffuse->SetAttribute("a", eastl::to_string(mLightData->mLighting->mDiffuse[3]).c_str());
	pSceneNode->LinkEndChild(pDiffuse);

	// Specular
	tinyxml2::XMLElement* pSpecular = doc.NewElement("Specular");
	pSpecular->SetAttribute("r", eastl::to_string(mLightData->mLighting->mSpecular[0]).c_str());
	pSpecular->SetAttribute("g", eastl::to_string(mLightData->mLighting->mSpecular[1]).c_str());
	pSpecular->SetAttribute("b", eastl::to_string(mLightData->mLighting->mSpecular[2]).c_str());
	pSpecular->SetAttribute("a", eastl::to_string(mLightData->mLighting->mSpecular[3]).c_str());
	pSceneNode->LinkEndChild(pSpecular);

    // attenuation
	tinyxml2::XMLElement* pAttenuation = doc.NewElement("Attenuation");
    pAttenuation->SetAttribute("const", eastl::to_string(mLightData->mLighting->mAttenuation[0]).c_str());
    pAttenuation->SetAttribute("linear", eastl::to_string(mLightData->mLighting->mAttenuation[1]).c_str());
    pAttenuation->SetAttribute("exp", eastl::to_string(mLightData->mLighting->mAttenuation[2]).c_str());
    pSceneNode->LinkEndChild(pAttenuation);

    // shape
	tinyxml2::XMLElement* pShape = doc.NewElement("Shape");
    pShape->SetAttribute("range", eastl::to_string(mLightData->mLighting->mRadius).c_str());
    pShape->SetAttribute("falloff", eastl::to_string(mLightData->mLighting->mFalloff).c_str());
    pShape->SetAttribute("theta", eastl::to_string(mLightData->mLighting->mInnerCone).c_str());
    pShape->SetAttribute("phi", eastl::to_string(mLightData->mLighting->mOuterCone).c_str());
    pSceneNode->LinkEndChild(pShape);

    pBaseElement->LinkEndChild(pSceneNode);

}


//---------------------------------------------------------------------------------------------------------------------
// ParticleSystemRenderComponent
//---------------------------------------------------------------------------------------------------------------------
ParticleSystemRenderComponent::ParticleSystemRenderComponent(void)
{

}

bool ParticleSystemRenderComponent::DelegateInit(tinyxml2::XMLElement* pData)
{
    return true;
}

eastl::shared_ptr<Node> ParticleSystemRenderComponent::CreateSceneNode(void)
{
    const eastl::shared_ptr<TransformComponent>& pTransformComponent(
		mOwner->GetComponent<TransformComponent>(TransformComponent::Name).lock());
    if (pTransformComponent)
    {
		GameApplication* gameApp = (GameApplication*)Application::App;
		const eastl::shared_ptr<ScreenElementScene>& pScene = gameApp->GetHumanView()->mScene;
		Transform transform = pTransformComponent->GetTransform();
		WeakBaseRenderComponentPtr wbrcp(
			eastl::dynamic_shared_pointer_cast<BaseRenderComponent>(shared_from_this()));

		if (gameApp->mOption.mRendererType == RT_DIRECT3D11)
		{
			// create a particle system
			eastl::shared_ptr<Node> node = 
				pScene->AddParticleSystemNode(wbrcp, 0, mOwner->GetId(), false);

			if (node)
			{
				eastl::shared_ptr<ParticleSystemNode> particleSystem = 
					eastl::dynamic_shared_pointer_cast<ParticleSystemNode>(node);
				particleSystem->GetRelativeTransform() = transform;

				eastl::shared_ptr<BaseParticleEmitter> em(
					particleSystem->CreateBoxEmitter(
						AlignedBox3<float>(),//{-7, 0, -7, 7, 1, 7}, // emitter size
						Vector3<float>{ 0.0f, 0.06f, 0.0f },   // initial direction
						80,100,                     // emit rate
						eastl::array<float, 4>{0, 255, 255, 255},       // darkest color
						eastl::array<float, 4>{0, 255, 255, 255},       // brightest color
						800,2000,0,                         // min and max age, angle
						Vector2<float>{10.f, 10.f},         // min size
						Vector2<float>{20.f, 20.f}));        // max size

				particleSystem->SetEmitter(em); // this grabs the emitter
		
				eastl::shared_ptr<BaseParticleAffector> particleAffector(
					particleSystem->CreateFadeOutParticleAffector());
				particleSystem->AddAffector(particleAffector); // same goes for the affector

				eastl::shared_ptr<ResHandle>& resHandle =
					ResCache::Get()->GetHandle(&BaseResource(L"Art/fire.bmp"));
				if (resHandle)
				{
					const eastl::shared_ptr<ImageResourceExtraData>& extra =
						eastl::static_pointer_cast<ImageResourceExtraData>(resHandle->GetExtra());

					particleSystem->SetMaterialFlag(MF_LIGHTING, false);
					particleSystem->SetMaterialFlag(MF_ZWRITE_ENABLE, false);
					particleSystem->SetMaterialTexture(0, extra->GetImage().get());
					particleSystem->SetMaterialType(MT_TRANSPARENT_ADD_COLOR);
				}
			}
			return node;
		}
		else LogError("Unknown Renderer Implementation in ParticleSystemRenderComponent");
	}
	return eastl::shared_ptr<Node>();
}

void ParticleSystemRenderComponent::CreateInheritedXMLElements(
	tinyxml2::XMLDocument doc, tinyxml2::XMLElement *pBaseElement)
{

}


//---------------------------------------------------------------------------------------------------------------------
// SkyRenderComponent
//---------------------------------------------------------------------------------------------------------------------
SkyRenderComponent::SkyRenderComponent(void)
{
	mHoriRes = 16;
	mVertRes = 8;
	mTexturePercentage = 0.9f;
	mSpherePercentage = 2.0f;
	mRadius = 1000.f;
	mTextureResource = "";
}

bool SkyRenderComponent::DelegateInit(tinyxml2::XMLElement* pData)
{
	tinyxml2::XMLElement* pTexture = pData->FirstChildElement("Texture");
	if (pTexture)
	{
		unsigned int horiRes = 16;
		unsigned int vertRes = 8;
		float texturePercentage = 0.9f;
		float spherePercentage = 2.0f;
		float radius = 1000.f;
		horiRes = pTexture->UnsignedAttribute("horiRes", horiRes);
		vertRes = pTexture->UnsignedAttribute("vertRes", vertRes);
		texturePercentage = pTexture->FloatAttribute("texturePercentage", texturePercentage);
		spherePercentage = pTexture->FloatAttribute("spherePercentage", spherePercentage);
		radius = pTexture->FloatAttribute("radius", radius);

		mHoriRes = horiRes;
		mVertRes = vertRes;
		mTexturePercentage = texturePercentage;
		mSpherePercentage = spherePercentage;
		mRadius = radius;
		mTextureResource = pTexture->Attribute("file");
	}
	return true;
}

eastl::shared_ptr<Node> SkyRenderComponent::CreateSceneNode(void)
{
	GameApplication* gameApp = (GameApplication*)Application::App;
	const eastl::shared_ptr<ScreenElementScene>& pScene = gameApp->GetHumanView()->mScene;
	WeakBaseRenderComponentPtr wbrcp(
		eastl::dynamic_shared_pointer_cast<BaseRenderComponent>(shared_from_this()));

	if (gameApp->mOption.mRendererType == RT_DIRECT3D11)
	{
		eastl::shared_ptr<Texture2> skyDome;
		eastl::shared_ptr<ResHandle>& resHandle = ResCache::Get()->GetHandle(
			&BaseResource(ToWideString(eastl::string(mTextureResource).c_str())));
		if (resHandle)
		{
			const eastl::shared_ptr<ImageResourceExtraData>& extra =
				eastl::static_pointer_cast<ImageResourceExtraData>(resHandle->GetExtra());
			skyDome = extra->GetImage();
		}

		// add skydome
		eastl::shared_ptr<Node> sky = pScene->AddSkyDomeNode(wbrcp, 0,
			skyDome, mHoriRes, mVertRes, mTexturePercentage, mSpherePercentage, mRadius, mOwner->GetId());
		return sky;
	}
	else LogError("Unknown Renderer Implementation in SkyRenderComponent");

	return eastl::shared_ptr<Node>();
}

void SkyRenderComponent::CreateInheritedXMLElements(
	tinyxml2::XMLDocument doc, tinyxml2::XMLElement *pBaseElement)
{
	tinyxml2::XMLElement* pTexture = doc.NewElement("Texture");
	pTexture->SetAttribute("file", mTextureResource.c_str());
	pTexture->SetAttribute("horiRes", eastl::to_string(mHoriRes).c_str());
	pTexture->SetAttribute("vertRes", eastl::to_string(mVertRes).c_str());
	pTexture->SetAttribute("texturePercentage", eastl::to_string(mTexturePercentage).c_str());
	pTexture->SetAttribute("spherePercentage", eastl::to_string(mSpherePercentage).c_str());
	pTexture->SetAttribute("radius", eastl::to_string(mRadius).c_str());
	pBaseElement->LinkEndChild(pTexture);
}
