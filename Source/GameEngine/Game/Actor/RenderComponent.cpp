 
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
const char* CubeRenderComponent::Name = "CubeRenderComponent";
const char* GridRenderComponent::Name = "GridRenderComponent";
const char* LightRenderComponent::Name = "LightRenderComponent";
const char* SkyRenderComponent::Name = "SkyRenderComponent";
const char* ParticleEffectRenderComponent::Name = "ParticleEffectRenderComponent";

//---------------------------------------------------------------------------------------------------------------------
// MeshRenderComponent
//---------------------------------------------------------------------------------------------------------------------
MeshRenderComponent::MeshRenderComponent(void)
{
}

bool MeshRenderComponent::DelegateInit(tinyxml2::XMLElement* pData)
{
	tinyxml2::XMLElement* pMesh = pData->FirstChildElement("Mesh");
	if (pMesh)
	{
		eastl::string meshes = pMesh->FirstChild()->Value();
		meshes.erase(eastl::remove(meshes.begin(), meshes.end(), '\r'), meshes.end());
		meshes.erase(eastl::remove(meshes.begin(), meshes.end(), '\n'), meshes.end());
		meshes.erase(eastl::remove(meshes.begin(), meshes.end(), '\t'), meshes.end());
		size_t meshBegin = 0, meshEnd = 0;
		do
		{
			meshEnd = meshes.find(',', meshBegin);
			mMeshes.push_back(meshes.substr(meshBegin, meshEnd));

			meshBegin = meshEnd + 1;
		} 
		while (meshEnd != eastl::string::npos);
	}

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

		eastl::shared_ptr<BaseMesh> mesh;
		for (eastl::string meshName : mMeshes)
		{
			eastl::shared_ptr<ResHandle>& resHandle =
				ResCache::Get()->GetHandle(&BaseResource(ToWideString(meshName.c_str())));
			if (resHandle)
			{
				const eastl::shared_ptr<MeshResourceExtraData>& extra =
					eastl::static_pointer_cast<MeshResourceExtraData>(resHandle->GetExtra());
				
				eastl::shared_ptr<AnimateMeshMD3> meshMD3 = 
					eastl::dynamic_shared_pointer_cast<AnimateMeshMD3>(mesh);
				if (meshMD3)
				{
					AnimateMeshMD3* animMeshMD3 = dynamic_cast<AnimateMeshMD3*>(extra->GetMesh());
					if (!animMeshMD3->GetMD3Mesh()->GetParent())
						meshMD3->GetMD3Mesh()->AttachChild(animMeshMD3->GetMD3Mesh());
				}
				else mesh = eastl::shared_ptr<BaseMesh>(extra->GetMesh());
			}
		}

		eastl::shared_ptr<Node> meshNode = nullptr;
		if (mesh->GetMeshType() == MT_STANDARD)
		{
			// create an mesh scene node with specified mesh.
			meshNode = pScene->AddMeshNode(wbrcp, 0, mesh, mOwner->GetId());
			if (meshNode)
				meshNode->GetRelativeTransform() = transform;
		}
		else
		{
			// create an animated mesh scene node with specified animated mesh.
			meshNode = pScene->AddAnimatedMeshNode(
				wbrcp, 0, eastl::dynamic_shared_pointer_cast<BaseAnimatedMesh>(mesh), mOwner->GetId());
			if (meshNode)
				meshNode->GetRelativeTransform() = transform;
		}
		return meshNode;
	}
	return eastl::shared_ptr<Node>();
}

void MeshRenderComponent::CreateInheritedXMLElements(
	tinyxml2::XMLDocument doc, tinyxml2::XMLElement* pBaseElement)
{
	// shape
	tinyxml2::XMLElement* pMesh = doc.NewElement("Mesh");
	for (eastl::string mesh : mMeshes)
	{
		tinyxml2::XMLText* pMeshText = doc.NewText(mesh.c_str());
		pBaseElement->LinkEndChild(pMeshText);
	}
}

//---------------------------------------------------------------------------------------------------------------------
// SphereRenderComponent
//---------------------------------------------------------------------------------------------------------------------
SphereRenderComponent::SphereRenderComponent(void)
{
    mSegments = 0;
	mMaterialType = 0;
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

	tinyxml2::XMLElement* pMaterial = pData->FirstChildElement("Material");
	if (pMaterial)
	{
		unsigned int type = 0;
		mMaterialType = pMaterial->IntAttribute("type", type);
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

		eastl::shared_ptr<ResHandle>& resHandle =
			ResCache::Get()->GetHandle(&BaseResource(ToWideString(mTextureResource.c_str())));
		if (resHandle)
		{
			const eastl::shared_ptr<ImageResourceExtraData>& extra =
				eastl::static_pointer_cast<ImageResourceExtraData>(resHandle->GetExtra());
			extra->GetImage()->AutogenerateMipmaps();

			// create a sphere node with specified radius and poly count.
			eastl::shared_ptr<Node> sphereNode =
				pScene->AddSphereNode(wbrcp, nullptr, extra->GetImage(), mRadius, mSegments, mOwner->GetId());
			if (sphereNode)
			{
				sphereNode->GetRelativeTransform() = transform;

				if (mMaterialType == MaterialType::MT_TRANSPARENT)
				{
					for (unsigned int i = 0; i < sphereNode->GetMaterialCount(); ++i)
					{
						eastl::shared_ptr<Material> material = sphereNode->GetMaterial(i);
						material->mBlendTarget.enable = true;
						material->mBlendTarget.srcColor = BlendState::BM_ONE;
						material->mBlendTarget.dstColor = BlendState::BM_INV_SRC_COLOR;
						material->mBlendTarget.srcAlpha = BlendState::BM_SRC_ALPHA;
						material->mBlendTarget.dstAlpha = BlendState::BM_INV_SRC_ALPHA;

						material->mDepthBuffer = true;
						material->mDepthMask = DepthStencilState::MASK_ZERO;

						material->mFillMode = RasterizerState::FILL_SOLID;
						material->mCullMode = RasterizerState::CULL_NONE;
					}
				}

				for (unsigned int i = 0; i < sphereNode->GetMaterialCount(); ++i)
					sphereNode->GetMaterial(i)->mLighting = false;
				sphereNode->SetMaterialTexture(0, extra->GetImage());
				sphereNode->SetMaterialType((MaterialType)mMaterialType);
			}
			return sphereNode;
		}
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

	tinyxml2::XMLElement* pMaterial = doc.NewElement("Material");
	pMaterial->SetAttribute("type", eastl::to_string(mMaterialType).c_str());
	pBaseElement->LinkEndChild(pMaterial);
}

//---------------------------------------------------------------------------------------------------------------------
// CubeRenderComponent
//---------------------------------------------------------------------------------------------------------------------
CubeRenderComponent::CubeRenderComponent(void)
{
	mTextureResource = "";
	mMaterialType = 0;
	mSize = 0;
}

bool CubeRenderComponent::DelegateInit(tinyxml2::XMLElement* pData)
{
	tinyxml2::XMLElement* pCube = pData->FirstChildElement("Cube");
	if (pCube)
	{
		mSize = 1.f;
		mSize = pCube->FloatAttribute("size", mSize);
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

	tinyxml2::XMLElement* pMaterial = pData->FirstChildElement("Material");
	if (pMaterial)
	{
		unsigned int type = 0;
		mMaterialType = pMaterial->IntAttribute("type", type);
	}

	return true;
}

eastl::shared_ptr<Node> CubeRenderComponent::CreateSceneNode(void)
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

		eastl::shared_ptr<ResHandle>& resHandle =
			ResCache::Get()->GetHandle(&BaseResource(ToWideString(mTextureResource.c_str())));
		if (resHandle)
		{
			const eastl::shared_ptr<ImageResourceExtraData>& extra =
				eastl::static_pointer_cast<ImageResourceExtraData>(resHandle->GetExtra());
			extra->GetImage()->AutogenerateMipmaps();

			// create an animated mesh scene node with specified mesh.
			eastl::shared_ptr<Node> cubeNode = pScene->AddCubeNode(wbrcp, 0, 
				extra->GetImage(), mTextureScale[0], mTextureScale[1], mSize, mOwner->GetId());

			//To let the mesh look a little bit nicer, we change its material. We
			//disable lighting because we do not have a dynamic light in here, and
			//the mesh would be totally black otherwise. And last, we apply a
			//texture to the mesh. Without it the mesh would be drawn using only a
			//color.
			if (cubeNode)
			{
				cubeNode->GetRelativeTransform() = transform;

				if (mMaterialType == MaterialType::MT_TRANSPARENT)
				{
					for (unsigned int i = 0; i<cubeNode->GetMaterialCount(); ++i)
					{
						eastl::shared_ptr<Material> material = cubeNode->GetMaterial(i);
						material->mBlendTarget.enable = true;
						material->mBlendTarget.srcColor = BlendState::BM_ONE;
						material->mBlendTarget.dstColor = BlendState::BM_INV_SRC_COLOR;
						material->mBlendTarget.srcAlpha = BlendState::BM_SRC_ALPHA;
						material->mBlendTarget.dstAlpha = BlendState::BM_INV_SRC_ALPHA;

						material->mDepthBuffer = true;
						material->mDepthMask = DepthStencilState::MASK_ZERO;

						material->mFillMode = RasterizerState::FILL_SOLID;
						material->mCullMode = RasterizerState::CULL_NONE;
					}
				}

				for (unsigned int i = 0; i<cubeNode->GetMaterialCount(); ++i)
					cubeNode->GetMaterial(i)->mLighting = false;
				cubeNode->SetMaterialTexture(0, extra->GetImage());
				cubeNode->SetMaterialType((MaterialType)mMaterialType);
			}

			return cubeNode;
		}
	}

	return eastl::shared_ptr<Node>();
}

void CubeRenderComponent::CreateInheritedXMLElements(
	tinyxml2::XMLDocument doc, tinyxml2::XMLElement *pBaseElement)
{
	tinyxml2::XMLElement* pCube = doc.NewElement("Cube");
	pCube->SetAttribute("size", eastl::to_string(mSize).c_str());
	pBaseElement->LinkEndChild(pCube);

	tinyxml2::XMLElement* pTexture = doc.NewElement("Texture");
	pTexture->SetAttribute("file", mTextureResource.c_str());
	pTexture->SetAttribute("x", eastl::to_string(mTextureScale[0]).c_str());
	pTexture->SetAttribute("y", eastl::to_string(mTextureScale[1]).c_str());
	pBaseElement->LinkEndChild(pTexture);

	tinyxml2::XMLElement* pMaterial = doc.NewElement("Material");
	pMaterial->SetAttribute("type", eastl::to_string(mMaterialType).c_str());
	pBaseElement->LinkEndChild(pMaterial);
}

//---------------------------------------------------------------------------------------------------------------------
// GridRenderComponent
//---------------------------------------------------------------------------------------------------------------------
GridRenderComponent::GridRenderComponent(void)
{
    mTextureResource = "";
	mMaterialType = 0;
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

	tinyxml2::XMLElement* pMaterial = pData->FirstChildElement("Material");
	if (pMaterial)
	{
		unsigned int type = 0;
		mMaterialType = pMaterial->IntAttribute("type", type);
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

				if (mMaterialType == MaterialType::MT_TRANSPARENT)
				{
					for (unsigned int i = 0; i<gridNode->GetMaterialCount(); ++i)
					{
						eastl::shared_ptr<Material> material = gridNode->GetMaterial(i);
						material->mBlendTarget.enable = true;
						material->mBlendTarget.srcColor = BlendState::BM_ONE;
						material->mBlendTarget.dstColor = BlendState::BM_INV_SRC_COLOR;
						material->mBlendTarget.srcAlpha = BlendState::BM_SRC_ALPHA;
						material->mBlendTarget.dstAlpha = BlendState::BM_INV_SRC_ALPHA;

						material->mDepthBuffer = true;
						material->mDepthMask = DepthStencilState::MASK_ZERO;

						material->mFillMode = RasterizerState::FILL_SOLID;
						material->mCullMode = RasterizerState::CULL_NONE;
					}
				}

				for (unsigned int i = 0; i<gridNode->GetMaterialCount(); ++i)
					gridNode->GetMaterial(i)->mLighting = false;
				gridNode->SetMaterialTexture(0, extra->GetImage());
				gridNode->SetMaterialType((MaterialType)mMaterialType);
			}

			return gridNode;
		}
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

	tinyxml2::XMLElement* pMaterial = doc.NewElement("Material");
	pMaterial->SetAttribute("type", eastl::to_string(mMaterialType).c_str());
	pBaseElement->LinkEndChild(pMaterial);

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
{
	mAnimatorType = -1;
	mTextureResource = "";

#if defined(_OPENGL_)
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

	tinyxml2::XMLElement* pLighting = pLight->FirstChildElement("Lighting");
	if (pLighting)
	{
		int type = 0;
		type = pLighting->IntAttribute("type", type);
		mLightData->mLighting->mType = (LightType)type;
	}

	tinyxml2::XMLElement* pTexture = pLight->FirstChildElement("Texture");
	if (pTexture)
	{
		float x = 1.f;
		float y = 1.f;
		x = pTexture->FloatAttribute("x", x);
		y = pTexture->FloatAttribute("y", y);
		mTextureScale = Vector2<float>{ x, y };
		mTextureResource = pTexture->Attribute("file");
	}

	tinyxml2::XMLElement* pDirection = pLight->FirstChildElement("Direction");
	if (pDirection)
	{
		temp = pDirection->FloatAttribute("x", temp);
		mLightData->mLighting->mDirection[0] = temp;

		temp = pDirection->FloatAttribute("y", temp);
		mLightData->mLighting->mDirection[1] = temp;

		temp = pDirection->FloatAttribute("z", temp);
		mLightData->mLighting->mDirection[2] = temp;
	}

	tinyxml2::XMLElement* pAmbient = pLight->FirstChildElement("Ambient");
	if (pAmbient)
	{
		temp = pAmbient->FloatAttribute("r", temp);
		mLightData->mLighting->mAmbient[0] = temp;

		temp = pAmbient->FloatAttribute("g", temp);
		mLightData->mLighting->mAmbient[1] = temp;

		temp = pAmbient->FloatAttribute("b", temp);
		mLightData->mLighting->mAmbient[2] = temp;

		temp = pAmbient->FloatAttribute("a", temp);
		mLightData->mLighting->mAmbient[3] = temp;
	}

	tinyxml2::XMLElement* pDiffuse = pLight->FirstChildElement("Diffuse");
	if (pDiffuse)
	{
		temp = pDiffuse->FloatAttribute("r", temp);
		mLightData->mLighting->mDiffuse[0] = temp;

		temp = pDiffuse->FloatAttribute("g", temp);
		mLightData->mLighting->mDiffuse[1] = temp;

		temp = pDiffuse->FloatAttribute("b", temp);
		mLightData->mLighting->mDiffuse[2] = temp;

		temp = pDiffuse->FloatAttribute("a", temp);
		mLightData->mLighting->mDiffuse[3] = temp;
	}

	tinyxml2::XMLElement* pSpecular = pLight->FirstChildElement("Specular");
	if (pSpecular)
	{
		temp = pSpecular->FloatAttribute("r", temp);
		mLightData->mLighting->mSpecular[0] = temp;

		temp = pSpecular->FloatAttribute("g", temp);
		mLightData->mLighting->mSpecular[1] = temp;

		temp = pSpecular->FloatAttribute("b", temp);
		mLightData->mLighting->mSpecular[2] = temp;

		temp = pSpecular->FloatAttribute("a", temp);
		mLightData->mLighting->mSpecular[3] = temp;
	}

	tinyxml2::XMLElement* pAttenuation = pLight->FirstChildElement("Attenuation");
    if (pAttenuation)
	{
		temp = pAttenuation->FloatAttribute("const", temp);
		mLightData->mLighting->mAttenuation[0] = temp;

		temp = pAttenuation->FloatAttribute("linear", temp);
		mLightData->mLighting->mAttenuation[1] = temp;

		temp = pAttenuation->FloatAttribute("exp", temp);
		mLightData->mLighting->mAttenuation[2] = temp;

		temp = pAttenuation->FloatAttribute("int", temp);
		mLightData->mLighting->mAttenuation[3] = temp;
	}

	tinyxml2::XMLElement* pSpotCutoff = pLight->FirstChildElement("SpotCutoff");
    if (pSpotCutoff)
	{
		temp = pSpotCutoff->FloatAttribute("angle", temp);
		mLightData->mLighting->mSpotCutoff[0] = temp;
		mLightData->mLighting->mSpotCutoff[1] = cos(temp);
		mLightData->mLighting->mSpotCutoff[2] = sin(temp);

		temp = pSpotCutoff->FloatAttribute("exponent", temp);
		mLightData->mLighting->mSpotCutoff[3] = temp;
	}

	tinyxml2::XMLElement* pAnimator = pData->FirstChildElement("Animator");
	tinyxml2::XMLElement* pAnimation = pAnimator->FirstChildElement("Animation");
	if (pAnimation)
	{
		unsigned int type = 0;
		mAnimatorType = pAnimation->IntAttribute("type", type);

		float radius = 0;
		mAnimatorRadius = pAnimation->FloatAttribute("radius", radius);
	}

	tinyxml2::XMLElement* pCenter = pAnimator->FirstChildElement("Center");
	if (pCenter)
	{
		temp = pCenter->FloatAttribute("x", temp);
		mAnimatorCenter[0] = temp;

		temp = pCenter->FloatAttribute("y", temp);
		mAnimatorCenter[1] = temp;

		temp = pCenter->FloatAttribute("z", temp);
		mAnimatorCenter[2] = temp;
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

		eastl::shared_ptr<ResHandle>& resHandle =
			ResCache::Get()->GetHandle(&BaseResource(ToWideString(mTextureResource.c_str())));
		if (resHandle)
		{
			const eastl::shared_ptr<ImageResourceExtraData>& extra =
				eastl::static_pointer_cast<ImageResourceExtraData>(resHandle->GetExtra());
			extra->GetImage()->AutogenerateMipmaps();

			// Add light
			eastl::shared_ptr<Node> lightNode =
				pScene->AddLightNode(wbrcp, 0, extra->GetImage(), mLightData, mTextureScale, mOwner->GetId());
			if (lightNode)
			{
				lightNode->GetRelativeTransform() = transform;

				switch(mAnimatorType)
				{
					case NAT_FLY_CIRCLE:
					{
						eastl::shared_ptr<NodeAnimator> anim = 0;
						anim = pScene->CreateFlyCircleAnimator(mAnimatorCenter, mAnimatorRadius);
						lightNode->AttachAnimator(anim);
						break;
					}

					default:
						break;
				}

				for (unsigned int i = 0; i < lightNode->GetMaterialCount(); ++i)
					lightNode->GetMaterial(i)->mLighting = false;
				lightNode->SetMaterialType(MT_TRANSPARENT);

				for (unsigned int i = 0; i < lightNode->GetMaterialCount(); ++i)
				{
					eastl::shared_ptr<Material> material = lightNode->GetMaterial(i);
					material->mBlendTarget.enable = true;
					material->mBlendTarget.srcColor = BlendState::BM_ONE;
					material->mBlendTarget.dstColor = BlendState::BM_INV_SRC_COLOR;
					material->mBlendTarget.srcAlpha = BlendState::BM_SRC_ALPHA;
					material->mBlendTarget.dstAlpha = BlendState::BM_INV_SRC_ALPHA;

					material->mDepthBuffer = true;
					material->mDepthMask = DepthStencilState::MASK_ZERO;

					material->mFillMode = RasterizerState::FILL_SOLID;
					material->mCullMode = RasterizerState::CULL_NONE;
				}
			}
			return lightNode;
		}
	}
	return eastl::shared_ptr<Node>();
}

void LightRenderComponent::CreateInheritedXMLElements(
	tinyxml2::XMLDocument doc, tinyxml2::XMLElement *pBaseElement)
{
	tinyxml2::XMLElement* pLightElement = doc.NewElement("Light");

	// lighting
	tinyxml2::XMLElement* pLighting = doc.NewElement("Lighting");
	pLighting->SetAttribute("type", eastl::to_string(mLightData->mLighting->mType).c_str());
	pLightElement->LinkEndChild(pLighting);

	// texture
	tinyxml2::XMLElement* pTexture = doc.NewElement("Texture");
	pTexture->SetAttribute("file", mTextureResource.c_str());
	pTexture->SetAttribute("x", eastl::to_string(mTextureScale[0]).c_str());
	pTexture->SetAttribute("y", eastl::to_string(mTextureScale[1]).c_str());
	pLightElement->LinkEndChild(pTexture);

	// Ambient
	tinyxml2::XMLElement* pDirection = doc.NewElement("Direction");
	pDirection->SetAttribute("x", eastl::to_string(mLightData->mLighting->mDirection[0]).c_str());
	pDirection->SetAttribute("y", eastl::to_string(mLightData->mLighting->mDirection[1]).c_str());
	pDirection->SetAttribute("z", eastl::to_string(mLightData->mLighting->mDirection[2]).c_str());
	pLightElement->LinkEndChild(pDirection);

	// Ambient
	tinyxml2::XMLElement* pAmbient = doc.NewElement("Ambient");
	pAmbient->SetAttribute("r", eastl::to_string(mLightData->mLighting->mAmbient[0]).c_str());
	pAmbient->SetAttribute("g", eastl::to_string(mLightData->mLighting->mAmbient[1]).c_str());
	pAmbient->SetAttribute("b", eastl::to_string(mLightData->mLighting->mAmbient[2]).c_str());
	pAmbient->SetAttribute("a", eastl::to_string(mLightData->mLighting->mAmbient[3]).c_str());
	pLightElement->LinkEndChild(pAmbient);

	// Diffuse
	tinyxml2::XMLElement* pDiffuse = doc.NewElement("Diffuse");
	pDiffuse->SetAttribute("r", eastl::to_string(mLightData->mLighting->mDiffuse[0]).c_str());
	pDiffuse->SetAttribute("g", eastl::to_string(mLightData->mLighting->mDiffuse[1]).c_str());
	pDiffuse->SetAttribute("b", eastl::to_string(mLightData->mLighting->mDiffuse[2]).c_str());
	pDiffuse->SetAttribute("a", eastl::to_string(mLightData->mLighting->mDiffuse[3]).c_str());
	pLightElement->LinkEndChild(pDiffuse);

	// Specular
	tinyxml2::XMLElement* pSpecular = doc.NewElement("Specular");
	pSpecular->SetAttribute("r", eastl::to_string(mLightData->mLighting->mSpecular[0]).c_str());
	pSpecular->SetAttribute("g", eastl::to_string(mLightData->mLighting->mSpecular[1]).c_str());
	pSpecular->SetAttribute("b", eastl::to_string(mLightData->mLighting->mSpecular[2]).c_str());
	pSpecular->SetAttribute("a", eastl::to_string(mLightData->mLighting->mSpecular[3]).c_str());
	pLightElement->LinkEndChild(pSpecular);

    // attenuation
	tinyxml2::XMLElement* pAttenuation = doc.NewElement("Attenuation");
    pAttenuation->SetAttribute("const", eastl::to_string(mLightData->mLighting->mAttenuation[0]).c_str());
    pAttenuation->SetAttribute("linear", eastl::to_string(mLightData->mLighting->mAttenuation[1]).c_str());
    pAttenuation->SetAttribute("exp", eastl::to_string(mLightData->mLighting->mAttenuation[2]).c_str());
	pAttenuation->SetAttribute("int", eastl::to_string(mLightData->mLighting->mAttenuation[3]).c_str());
    pLightElement->LinkEndChild(pAttenuation);

    // spotcutoff
	tinyxml2::XMLElement* pSpotCutoff = doc.NewElement("SpotCutoff");
	pSpotCutoff->SetAttribute("angle", eastl::to_string(mLightData->mLighting->mSpotCutoff[0]).c_str());
	pSpotCutoff->SetAttribute("exponent", eastl::to_string(mLightData->mLighting->mSpotCutoff[3]).c_str());
    pLightElement->LinkEndChild(pSpotCutoff);

	pBaseElement->LinkEndChild(pLightElement);

	tinyxml2::XMLElement* pAnimatorElement = doc.NewElement("Animator");

	// animation
	tinyxml2::XMLElement* pAnimation = doc.NewElement("Animation");
	pAnimation->SetAttribute("type", eastl::to_string(mAnimatorType).c_str());
	pAnimation->SetAttribute("radius", eastl::to_string(mAnimatorRadius).c_str());
	pAnimatorElement->LinkEndChild(pAnimation);

	tinyxml2::XMLElement* pCenter = doc.NewElement("Center");
	pCenter->SetAttribute("x", eastl::to_string(mAnimatorCenter[0]).c_str());
	pCenter->SetAttribute("y", eastl::to_string(mAnimatorCenter[1]).c_str());
	pCenter->SetAttribute("z", eastl::to_string(mAnimatorCenter[2]).c_str());
	pAnimatorElement->LinkEndChild(pCenter);

	pBaseElement->LinkEndChild(pAnimatorElement);
}


//---------------------------------------------------------------------------------------------------------------------
// ParticleEffectRenderComponent
//---------------------------------------------------------------------------------------------------------------------
ParticleEffectRenderComponent::ParticleEffectRenderComponent(void)
{
	mTextureResource = "";
	mMaterialType = 0;
	mEmitterType = PET_NONE;
	mAffectorType = PAT_NONE;
}

bool ParticleEffectRenderComponent::DelegateInit(tinyxml2::XMLElement* pData)
{
	tinyxml2::XMLElement* pTexture = pData->FirstChildElement("Texture");
	if (pTexture)
	{
		mTextureResource = pTexture->Attribute("file");
	}

	tinyxml2::XMLElement* pMaterial = pData->FirstChildElement("Material");
	if (pMaterial)
	{
		unsigned int type = 0;
		mMaterialType = pMaterial->IntAttribute("type", type);
	}

	tinyxml2::XMLElement* pEmitterType = pData->FirstChildElement("EmitterType");
	if (pEmitterType)
	{
		mEmitterType = (ParticleEmitterType)pEmitterType->IntAttribute("type");
	}

	tinyxml2::XMLElement* pAffector = pData->FirstChildElement("AffectorType");
	if (pAffector)
	{
		mAffectorType = (ParticleAffectorType)pAffector->IntAttribute("type");
	}

	tinyxml2::XMLElement* pEmitter = pData->FirstChildElement("Emitter");
	if (pEmitter)
	{
		float minx = -7.0f;
		float miny = -7.0f;
		float minz = 0.0f;

		float maxx = 7.0f;
		float maxy = 7.0f;
		float maxz = 1.0f;

		minx = pEmitter->FloatAttribute("minx", minx);
		miny = pEmitter->FloatAttribute("miny", miny);
		minz = pEmitter->FloatAttribute("minz", minz);
		maxx = pEmitter->FloatAttribute("maxx", maxx);
		maxy = pEmitter->FloatAttribute("maxy", maxy);
		maxz = pEmitter->FloatAttribute("maxz", maxz);

		mEmitter = AlignedBox3<float>(
			Vector3<float>{minx, miny, minz}, 
			Vector3<float>{maxx, maxy, maxz} );
	}

	tinyxml2::XMLElement* pDirection = pData->FirstChildElement("Direction");
	if (pDirection)
	{
		float x = 0.0f;
		float y = 0.0f;
		float z = 0.06f;
		x = pDirection->FloatAttribute("x", x);
		y = pDirection->FloatAttribute("y", y);
		z = pDirection->FloatAttribute("z", z);

		mDirection = Vector3<float>{ x, y, z };
	}

	tinyxml2::XMLElement* pEmitRate = pData->FirstChildElement("EmitRate");
	if (pEmitRate)
	{
		int min = 80;
		int max = 100;
		min = pEmitRate->IntAttribute("min", min);
		max = pEmitRate->IntAttribute("max", max);

		mMinParticlesPerSecond = min;
		mMaxParticlesPerSecond = max;
	}

	tinyxml2::XMLElement* pColorMinimum = pData->FirstChildElement("ColorMinimum");
	if (pColorMinimum)
	{
		float temp = 0;

		temp = pColorMinimum->FloatAttribute("r", temp);
		mMinStartColor[0] = temp;

		temp = pColorMinimum->FloatAttribute("g", temp);
		mMinStartColor[1] = temp;

		temp = pColorMinimum->FloatAttribute("b", temp);
		mMinStartColor[2] = temp;

		temp = pColorMinimum->FloatAttribute("a", temp);
		mMinStartColor[3] = temp;
	}

	tinyxml2::XMLElement* pColorMaximum = pData->FirstChildElement("ColorMaximum");
	if (pColorMaximum)
	{
		float temp = 0;

		temp = pColorMaximum->FloatAttribute("r", temp);
		mMaxStartColor[0] = temp;

		temp = pColorMaximum->FloatAttribute("g", temp);
		mMaxStartColor[1] = temp;

		temp = pColorMaximum->FloatAttribute("b", temp);
		mMaxStartColor[2] = temp;

		temp = pColorMaximum->FloatAttribute("a", temp);
		mMaxStartColor[3] = temp;
	}

	tinyxml2::XMLElement* pLifeTime = pData->FirstChildElement("LifeTime");
	if (pEmitRate)
	{
		int min = 800;
		int max = 2000;
		min = pLifeTime->IntAttribute("min", min);
		max = pLifeTime->IntAttribute("max", max);

		mMinLifeTime = min;
		mMaxLifeTime = max;
	}

	tinyxml2::XMLElement* pAngleMaximum = pData->FirstChildElement("AngleMaximum");
	if (pAngleMaximum)
	{
		int angle = 0;
		angle = pAngleMaximum->IntAttribute("angle", angle);

		mMaxAngle = angle;
	}

	tinyxml2::XMLElement* pSizeMinimum = pData->FirstChildElement("SizeMinimum");
	if (pSizeMinimum)
	{
		float x = 10.0f;
		float y = 10.0f;
		x = pSizeMinimum->FloatAttribute("x", x);
		y = pSizeMinimum->FloatAttribute("y", y);

		mMinStartSize = Vector2<float>{ x, y };
	}

	tinyxml2::XMLElement* pSizeMaximum = pData->FirstChildElement("SizeMaximum");
	if (pSizeMaximum)
	{
		float x = 20.0f;
		float y = 20.0f;
		x = pSizeMaximum->FloatAttribute("x", x);
		y = pSizeMaximum->FloatAttribute("y", y);

		mMaxStartSize = Vector2<float>{ x, y };
	}

	return true;
}

eastl::shared_ptr<Node> ParticleEffectRenderComponent::CreateSceneNode(void)
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

		// create a particle system
		eastl::shared_ptr<Node> node = 
			pScene->AddParticleSystemNode(wbrcp, 0, mOwner->GetId(), false);

		if (node)
		{
			eastl::shared_ptr<ParticleSystemNode> particleSystem = 
				eastl::dynamic_shared_pointer_cast<ParticleSystemNode>(node);
			particleSystem->GetRelativeTransform() = transform;

			switch (mEmitterType)
			{
				case PET_POINT:
				{
					eastl::shared_ptr<BaseParticleEmitter> em(particleSystem->CreatePointEmitter(
						mDirection, mMinParticlesPerSecond, mMaxParticlesPerSecond,
						mMinStartColor, mMaxStartColor, mMinLifeTime, mMaxLifeTime,
						mMaxAngle, mMinStartSize, mMaxStartSize));
					particleSystem->SetEmitter(em); // this grabs the emitter
					break;
				}
				case PET_BOX:
				{
					eastl::shared_ptr<BaseParticleEmitter> em(particleSystem->CreateBoxEmitter(
						mEmitter, mDirection, mMinParticlesPerSecond, mMaxParticlesPerSecond,
						mMinStartColor, mMaxStartColor, mMinLifeTime, mMaxLifeTime,
						mMaxAngle, mMinStartSize, mMaxStartSize));
					particleSystem->SetEmitter(em); // this grabs the emitter
					break;
				}
				case PET_CYLINDER:
				{
					eastl::shared_ptr<BaseParticleEmitter> em(particleSystem->CreateCylinderEmitter(
						Vector3<float>(), 0.0f, Vector3<float>(), 0.0f, true,
						mDirection, mMinParticlesPerSecond, mMaxParticlesPerSecond,
						mMinStartColor, mMaxStartColor, mMinLifeTime, mMaxLifeTime,
						mMaxAngle, mMinStartSize, mMaxStartSize));
					particleSystem->SetEmitter(em); // this grabs the emitter
					break;
				}
				case PET_MESH:
				{
					eastl::shared_ptr<BaseParticleEmitter> em(particleSystem->CreateMeshEmitter(
						eastl::shared_ptr<BaseMesh>(), false, mDirection, 0, 0, false,
						mMinParticlesPerSecond, mMaxParticlesPerSecond,
						mMinStartColor, mMaxStartColor, mMinLifeTime, mMaxLifeTime,
						mMaxAngle, mMinStartSize, mMaxStartSize));
					particleSystem->SetEmitter(em); // this grabs the emitter;
					break;
				}
				case PET_RING:
				{
					eastl::shared_ptr<BaseParticleEmitter> em(particleSystem->CreateRingEmitter(
						Vector3<float>(), 0.0f, 0.0f, mDirection, mMinParticlesPerSecond, mMaxParticlesPerSecond,
						mMinStartColor, mMaxStartColor, mMinLifeTime, mMaxLifeTime,
						mMaxAngle, mMinStartSize, mMaxStartSize));
					particleSystem->SetEmitter(em); // this grabs the emitter
					break;
				}
				case PET_SPHERE:
				{
					eastl::shared_ptr<BaseParticleEmitter> em(particleSystem->CreateSphereEmitter(
						Vector3<float>(), 0.0f, mDirection, mMinParticlesPerSecond, mMaxParticlesPerSecond,
						mMinStartColor, mMaxStartColor, mMinLifeTime, mMaxLifeTime,
						mMaxAngle, mMinStartSize, mMaxStartSize));
					particleSystem->SetEmitter(em); // this grabs the emitter
					break;
				}
				default:
					break;
			}

			switch (mAffectorType)
			{
				case PAT_ATTRACT:
				{
					eastl::shared_ptr<BaseParticleAffector> particleAffector(
						particleSystem->CreateAttractionAffector());
					particleSystem->AddAffector(particleAffector);
					break;
				}
				case PAT_FADE_OUT:
				{
					eastl::shared_ptr<BaseParticleAffector> particleAffector(
						particleSystem->CreateFadeOutParticleAffector());
					particleSystem->AddAffector(particleAffector);
					break;
				}
				case PAT_GRAVITY:
				{
					eastl::shared_ptr<BaseParticleAffector> particleAffector(
						particleSystem->CreateGravityAffector());
					particleSystem->AddAffector(particleAffector);
					break;
				}
				case PAT_ROTATE:
				{
					eastl::shared_ptr<BaseParticleAffector> particleAffector(
						particleSystem->CreateRotationAffector());
					particleSystem->AddAffector(particleAffector);
					break;
				}
				case PAT_SCALE:
				{
					eastl::shared_ptr<BaseParticleAffector> particleAffector(
						particleSystem->CreateScaleParticleAffector());
					particleSystem->AddAffector(particleAffector);
					break;
				}
				default:
					break;
			}

			eastl::shared_ptr<ResHandle>& resHandle =
				ResCache::Get()->GetHandle(&BaseResource(ToWideString(mTextureResource.c_str())));
			if (resHandle)
			{
				const eastl::shared_ptr<ImageResourceExtraData>& extra =
					eastl::static_pointer_cast<ImageResourceExtraData>(resHandle->GetExtra());
				extra->GetImage()->AutogenerateMipmaps();

				for (unsigned int i = 0; i<particleSystem->GetMaterialCount(); ++i)
					particleSystem->GetMaterial(i)->mLighting = false;
				if (mMaterialType == MaterialType::MT_TRANSPARENT)
				{
					for (unsigned int i = 0; i<particleSystem->GetMaterialCount(); ++i)
					{
						eastl::shared_ptr<Material> material = particleSystem->GetMaterial(i);

						material->mBlendTarget.enable = true;
						material->mBlendTarget.srcColor = BlendState::BM_ONE;
						material->mBlendTarget.dstColor = BlendState::BM_INV_SRC_COLOR;
						material->mBlendTarget.srcAlpha = BlendState::BM_SRC_ALPHA;
						material->mBlendTarget.dstAlpha = BlendState::BM_INV_SRC_ALPHA;

						material->mDepthBuffer = true;
						material->mDepthMask = DepthStencilState::MASK_ZERO;

						material->mFillMode = RasterizerState::FILL_SOLID;
						material->mCullMode = RasterizerState::CULL_NONE;
					}
				}

				particleSystem->SetMaterialTexture(0, extra->GetImage());
				particleSystem->SetMaterialType((MaterialType)mMaterialType);
			}

			particleSystem->SetEffect(0);
		}
		return node;
	}

	return eastl::shared_ptr<Node>();
}

void ParticleEffectRenderComponent::CreateInheritedXMLElements(
	tinyxml2::XMLDocument doc, tinyxml2::XMLElement *pBaseElement)
{
	tinyxml2::XMLElement* pTexture = doc.NewElement("Texture");
	pTexture->SetAttribute("file", mTextureResource.c_str());
	pBaseElement->LinkEndChild(pTexture);

	tinyxml2::XMLElement* pMaterial = doc.NewElement("Material");
	pMaterial->SetAttribute("type", eastl::to_string(mMaterialType).c_str());
	pBaseElement->LinkEndChild(pMaterial);

	tinyxml2::XMLElement* pAffector = doc.NewElement("Affector");
	pAffector->SetAttribute("type", eastl::to_string(mAffectorType).c_str());
	pBaseElement->LinkEndChild(pAffector);

	tinyxml2::XMLElement* pEmitter = doc.NewElement("Emitter");
	pEmitter->SetAttribute("minx", eastl::to_string(mEmitter.mMin[0]).c_str());
	pEmitter->SetAttribute("miny", eastl::to_string(mEmitter.mMin[1]).c_str());
	pEmitter->SetAttribute("minz", eastl::to_string(mEmitter.mMin[2]).c_str());
	pEmitter->SetAttribute("maxx", eastl::to_string(mEmitter.mMax[0]).c_str());
	pEmitter->SetAttribute("maxy", eastl::to_string(mEmitter.mMax[1]).c_str());
	pEmitter->SetAttribute("maxz", eastl::to_string(mEmitter.mMax[2]).c_str());
	pBaseElement->LinkEndChild(pEmitter);

	tinyxml2::XMLElement* pDirection = doc.NewElement("Direction");
	pDirection->SetAttribute("x", eastl::to_string(mDirection[0]).c_str());
	pDirection->SetAttribute("y", eastl::to_string(mDirection[1]).c_str());
	pDirection->SetAttribute("z", eastl::to_string(mDirection[2]).c_str());
	pBaseElement->LinkEndChild(pDirection);

	tinyxml2::XMLElement* pEmitRate = doc.NewElement("EmitRate");
	pEmitRate->SetAttribute("min", eastl::to_string(mMinParticlesPerSecond).c_str());
	pEmitRate->SetAttribute("max", eastl::to_string(mMaxParticlesPerSecond).c_str());
	pBaseElement->LinkEndChild(pEmitRate);

	tinyxml2::XMLElement* pColorMinimum = doc.NewElement("ColorMinimum");
	pColorMinimum->SetAttribute("r", eastl::to_string(mMinStartColor[0]).c_str());
	pColorMinimum->SetAttribute("g", eastl::to_string(mMinStartColor[1]).c_str());
	pColorMinimum->SetAttribute("b", eastl::to_string(mMinStartColor[2]).c_str());
	pColorMinimum->SetAttribute("a", eastl::to_string(mMinStartColor[3]).c_str());
	pBaseElement->LinkEndChild(pColorMinimum);

	tinyxml2::XMLElement* pColorMaximum = doc.NewElement("ColorMaximum");
	pColorMaximum->SetAttribute("r", eastl::to_string(mMaxStartColor[0]).c_str());
	pColorMaximum->SetAttribute("g", eastl::to_string(mMaxStartColor[1]).c_str());
	pColorMaximum->SetAttribute("b", eastl::to_string(mMaxStartColor[2]).c_str());
	pColorMaximum->SetAttribute("a", eastl::to_string(mMaxStartColor[3]).c_str());
	pBaseElement->LinkEndChild(pColorMaximum);

	tinyxml2::XMLElement* pLifeTime = doc.NewElement("LifeTime");
	pLifeTime->SetAttribute("min", eastl::to_string(mMinLifeTime).c_str());
	pLifeTime->SetAttribute("max", eastl::to_string(mMaxLifeTime).c_str());
	pBaseElement->LinkEndChild(pLifeTime);

	tinyxml2::XMLElement* pAngleMaximum = doc.NewElement("AngleMaximum");
	pAngleMaximum->SetAttribute("angle", eastl::to_string(mMaxAngle).c_str());
	pBaseElement->LinkEndChild(pAngleMaximum);

	tinyxml2::XMLElement* pSizeMinimum = doc.NewElement("SizeMinimum");
	pSizeMinimum->SetAttribute("x", eastl::to_string(mMinStartSize[0]).c_str());
	pSizeMinimum->SetAttribute("y", eastl::to_string(mMinStartSize[1]).c_str());
	pBaseElement->LinkEndChild(pSizeMinimum);

	tinyxml2::XMLElement* pSizeMaximum = doc.NewElement("SizeMaximum");
	pSizeMaximum->SetAttribute("x", eastl::to_string(mMaxStartSize[0]).c_str());
	pSizeMaximum->SetAttribute("y", eastl::to_string(mMaxStartSize[1]).c_str());
	pBaseElement->LinkEndChild(pSizeMaximum);
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
