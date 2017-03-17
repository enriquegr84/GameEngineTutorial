 
//========================================================================
// RenderComponent.cpp : classes that define renderable components of actors like Meshes, Skyboxes, Lights, etc.
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

#include "Core/Logger/Logger.h"

const char* MeshRenderComponent::g_Name = "MeshRenderComponent";
const char* SphereRenderComponent::g_Name = "SphereRenderComponent";
const char* TeapotRenderComponent::g_Name = "TeapotRenderComponent";
const char* GridRenderComponent::g_Name = "GridRenderComponent";
const char* LightRenderComponent::g_Name = "LightRenderComponent";
const char* SkyRenderComponent::g_Name = "SkyRenderComponent";
const char* ParticleSystemRenderComponent::g_Name = "ParticleSystemRenderComponent";

//---------------------------------------------------------------------------------------------------------------------
// MeshRenderComponent
//---------------------------------------------------------------------------------------------------------------------
MeshRenderComponent::MeshRenderComponent(void)
{
    //m_segments = 50;
}

bool MeshRenderComponent::DelegateInit(XMLElement* pData)
{
    XMLElement* pMesh = pData->FirstChildElement("Mesh");

	m_meshTextureFile = pMesh->Attribute("texture_file");
	m_meshModelFile = pMesh->Attribute("model_file");
    //pMesh->Attribute("position", &segments);
	//pMesh->Attribute("rotation", &segments);
	//pMesh->Attribute("scale", &segments);

    return true;
}
/*
eastl::shared_ptr<SceneNode> MeshRenderComponent::CreateSceneNode(void)
{
    // get the transform component
    const eastl::shared_ptr<TransformComponent>& pTransformComponent(
		m_pOwner->GetComponent<TransformComponent>(TransformComponent::g_Name));
    if (!pTransformComponent)
    {
        // can't render without a transform
        return eastl::shared_ptr<SceneNode>();
    }

	GameApplication* gameApp = (GameApplication*)Application::App;
	const eastl::shared_ptr<ScreenElementScene>& pScene = gameApp->GetHumanView()->m_pScene;
	Transform transform = pTransformComponent->GetTransform();
	eastl::weak_ptr<BaseRenderComponent> wbrcp(this);

	if (g_pGameApp->GetRendererImpl() == ERT_DIRECT3D9)
	{
		eastl::shared_ptr<IAnimatedMesh> mesh = pScene->GetMesh(m_meshModelFile);
		
		// create an animated mesh scene node with specified mesh.
		eastl::shared_ptr<SceneNode> animatedMeshNode =
			pScene->AddAnimatedMeshSceneNode(m_pOwner->GetId(), wbrcp, 0, &transform, mesh);

		//To let the mesh look a little bit nicer, we change its material. We
		//disable lighting because we do not have a dynamic light in here, and
		//the mesh would be totally black otherwise. And last, we apply a
		//texture to the mesh. Without it the mesh would be drawn using only a
		//color.

		if (animatedMeshNode)
		{
			animatedMeshNode->SetMaterialFlag(EMF_LIGHTING, false);
			animatedMeshNode->SetMaterialTexture( 0, 
				pScene->GetRenderer()->GetTexture(m_meshTextureFile).get() );
		}

	    return animatedMeshNode;
	}
	else LogAssert(0 && "Unknown Renderer Implementation in MeshRenderComponent::CreateSceneNode");

	return eastl::shared_ptr<SceneNode>();
}
*/
void MeshRenderComponent::CreateInheritedXMLElements(XMLDocument doc, XMLElement* pBaseElement)
{
	LogError("MeshRenderComponent::GenerateSubclassXml() not implemented");
}

//---------------------------------------------------------------------------------------------------------------------
// SphereRenderComponent
//---------------------------------------------------------------------------------------------------------------------
SphereRenderComponent::SphereRenderComponent(void)
{
    m_segments = 50;
}

bool SphereRenderComponent::DelegateInit(XMLElement* pData)
{
    XMLElement* pMesh = pData->FirstChildElement("Sphere");
    int segments = 50;
	float radius = 1.0;
	radius = pMesh->FloatAttribute("radius", radius);
    segments = pMesh->IntAttribute("segments", segments);
	m_radius = (float)radius;
    m_segments = (unsigned int)segments;

    return true;
}
/*
eastl::shared_ptr<SceneNode> SphereRenderComponent::CreateSceneNode(void)
{
    // get the transform component
    const eastl::shared_ptr<TransformComponent>& pTransformComponent(
		m_pOwner->GetComponent<TransformComponent>(TransformComponent::g_Name));
    if (!pTransformComponent)
    {
        // can't render without a transform
        return eastl::shared_ptr<SceneNode>();
    }

	GameApplication* gameApp = (GameApplication*)Application::App;
	const eastl::shared_ptr<ScreenElementScene>& pScene = gameApp->GetHumanView()->m_pScene;
	Transform transform = pTransformComponent->GetTransform();
	eastl::weak_ptr<BaseRenderComponent> wbrcp(this);

	if (g_pGameApp->GetRendererImpl() == ERT_DIRECT3D9)
	{
		// create a sphere node with specified radius and poly count.
		eastl::shared_ptr<SceneNode> sphere =
			pScene->AddSphereSceneNode(m_pOwner->GetId(), wbrcp, 0, &transform, m_radius, m_segments);
		if (sphere)
		{
			sphere->SetPosition(transform.GetTranslation());
			sphere->SetMaterialTexture(0, gameApp->m_pRenderer->GetTexture("art/textures/wall.bmp").get());
			sphere->SetMaterialFlag(EMF_LIGHTING, false);
		}

	    return sphere;
	}
	else LogAssert(0 && "Unknown Renderer Implementation in SphereRenderComponent::CreateSceneNode");

	return eastl::shared_ptr<SceneNode>();
}
*/
void SphereRenderComponent::CreateInheritedXMLElements(XMLDocument doc, XMLElement* pBaseElement)
{
    XMLElement* pMesh = doc.NewElement("Sphere");
	pMesh->SetAttribute("radius", eastl::to_string(m_radius).c_str());
    pMesh->SetAttribute("segments", eastl::to_string(m_segments).c_str());
    pBaseElement->LinkEndChild(pBaseElement);
}

/*
//---------------------------------------------------------------------------------------------------------------------
// TeapotRenderComponent
//---------------------------------------------------------------------------------------------------------------------
eastl::shared_ptr<SceneNode> TeapotRenderComponent::CreateSceneNode(void)
{
    // get the transform component
    const eastl::shared_ptr<TransformComponent>& pTransformComponent(
		m_pOwner->GetComponent<TransformComponent>(TransformComponent::g_Name));
    if (pTransformComponent)
    {
		GameApplication* gameApp = (GameApplication*)Application::App;
		const eastl::shared_ptr<ScreenElementScene>& pScene = gameApp->GetHumanView()->m_pScene;
		Transform transform = pTransformComponent->GetTransform();
		eastl::weak_ptr<BaseRenderComponent> wbrcp(this);

		if (gameApp->GetRendererImpl() == ERT_DIRECT3D9)
		{
			// create a sphere node with specified radius and poly count.
			eastl::shared_ptr<SceneNode> sphere =
				pScene->AddSphereSceneNode(m_pOwner->GetId(), wbrcp, 0, &transform, 5, 64);
			if (sphere)
			{
				sphere->SetPosition(transform.GetTranslation());
				sphere->SetMaterialTexture(0, gameApp->m_pRenderer->GetTexture("art/textures/wall.bmp").get());
				sphere->SetMaterialFlag(EMF_LIGHTING, false);
			}

			return sphere;

			// add this mesh scene node.
			eastl::shared_ptr<SceneNode> node = 
				pScene->AddMeshSceneNode(m_pOwner->GetId(), wbrcp, 0, &transform, pScene->GetMesh("tiefite.3ds"));
			if (node)
			{
				node->SetRotation(transform.GetRotationDegrees());
				node->SetPosition(transform.GetTranslation()); // Put its feet on the floor.
				//node->SetScale(Vector3(5)); // Make it appear realistically scaled
			}

			return node;
		}
		else LogError("Unknown Renderer Implementation in TeapotRenderComponent");
    }

    return eastl::shared_ptr<SceneNode>();
}
*/

void TeapotRenderComponent::CreateInheritedXMLElements(XMLDocument, XMLElement *)
{
}

//---------------------------------------------------------------------------------------------------------------------
// GridRenderComponent
//---------------------------------------------------------------------------------------------------------------------
GridRenderComponent::GridRenderComponent(void)
{
    m_textureResource = "";
    m_squares = 0;
}

bool GridRenderComponent::DelegateInit(XMLElement* pData)
{
    XMLElement* pTexture = pData->FirstChildElement("Texture");
    if (pTexture)
	{
		m_textureResource = pTexture->FirstChild()->Value();
	}

    XMLElement* pDivision = pData->FirstChildElement("Division");
    if (pDivision)
	{
		m_squares = atoi(pDivision->FirstChild()->Value());
	}

    return true;
}

/*
eastl::shared_ptr<SceneNode> GridRenderComponent::CreateSceneNode(void)
{
    const eastl::shared_ptr<TransformComponent>& pTransformComponent(
		m_pOwner->GetComponent<TransformComponent>(TransformComponent::g_Name));
    if (pTransformComponent)
    {
		GameApplication* gameApp = (GameApplication*)Application::App;
		const eastl::shared_ptr<ScreenElementScene>& pScene = gameApp->GetHumanView()->m_pScene;
		Transform transform = pTransformComponent->GetTransform();
		eastl::weak_ptr<BaseRenderComponent> wbrcp(this);

		if (gameApp->GetRendererImpl() == ERT_DIRECT3D9)
		{
			// add this mesh scene node.
			eastl::shared_ptr<IMesh> tangentMesh(
				pScene->AddHillPlaneMesh("plane", Dimension2<f32>(40,40), 
					Dimension2<unsigned int>(sqrt((float)m_squares), sqrt((float)m_squares))));

			eastl::shared_ptr<SceneNode> plane =
				pScene->AddMeshSceneNode(m_pOwner->GetId(), wbrcp, 0, &transform, tangentMesh);
			
			if (plane)
			{
				plane->SetPosition(transform.GetTranslation());
				//plane->SetRotation(Vector3(270,0,0));

				plane->SetMaterialTexture(
					0, gameApp->m_pRenderer->GetTexture("art/textures/t351sml.jpg").get());
				plane->SetMaterialFlag(EMF_LIGHTING, false);
				plane->SetMaterialFlag(EMF_BACK_FACE_CULLING, true);
			}

			return plane;
		}
		else LogError("Unknown Renderer Implementation in GridRenderComponent");
    }

    return eastl::shared_ptr<SceneNode>();
}
*/
void GridRenderComponent::CreateInheritedXMLElements(XMLDocument doc, XMLElement *pBaseElement)
{
    XMLElement* pTextureNode = doc.NewElement("Texture");
    XMLText* pTextureText = doc.NewText(m_textureResource.c_str());
    pTextureNode->LinkEndChild(pTextureText);
    pBaseElement->LinkEndChild(pTextureNode);

    XMLElement* pDivisionNode = doc.NewElement("Division");
	XMLText* pDivisionText = doc.NewText(eastl::to_string(m_squares).c_str());
    pDivisionNode->LinkEndChild(pDivisionText);
    pBaseElement->LinkEndChild(pDivisionNode);
}

//---------------------------------------------------------------------------------------------------------------------
// LightRenderComponent
//---------------------------------------------------------------------------------------------------------------------
LightRenderComponent::LightRenderComponent(void)
:	m_animatorType("none"), m_addBillboard(false)
{

}

bool LightRenderComponent::DelegateInit(XMLElement* pData)
{
	double temp;
	XMLElement* pColor = pData->FirstChildElement("Color");
	if (pColor)
	{
		temp = pColor->DoubleAttribute("r", temp);
		//m_Props.m_DiffuseColor.r = temp;

		temp = pColor->DoubleAttribute("g", temp);
		//m_Props.m_DiffuseColor.g = temp;

		temp = pColor->DoubleAttribute("b", temp);
		//m_Props.m_DiffuseColor.b = temp;

		temp = pColor->DoubleAttribute("a", temp);
		//m_Props.m_DiffuseColor.a = temp;
	}

	XMLElement* pLight = pData->FirstChildElement("Light");
    XMLElement* pAttenuationNode = pLight->FirstChildElement("Attenuation");
    if (pAttenuationNode)
	{
		temp = pAttenuationNode->DoubleAttribute("const", temp);
		//m_Props.m_Attenuation.X = temp;

		temp = pAttenuationNode->DoubleAttribute("linear", temp);
		//m_Props.m_Attenuation.Y = temp;

		temp = pAttenuationNode->DoubleAttribute("exp", temp);
		//m_Props.m_Attenuation.Z = temp;
	}

    XMLElement* pShapeNode = pLight->FirstChildElement("Shape");
    if (pShapeNode)
	{
		temp = pShapeNode->DoubleAttribute("range", temp);
		//m_Props.m_Radius = temp;

		temp = pShapeNode->DoubleAttribute("falloff", temp);
		//m_Props.m_Falloff = temp;

		temp = pShapeNode->DoubleAttribute("outercone", temp);
		//m_Props.m_OuterCone = temp;

		temp = pShapeNode->DoubleAttribute("innercone", temp);
		//m_Props.m_InnerCone = temp;	
	}

	XMLElement* pAnimator = pData->FirstChildElement("Animator");
	if (pAnimator)
	{
		m_animatorType = pAnimator->Attribute("type");

		temp = pAnimator->DoubleAttribute("x", temp);
		//m_animatorCenter.X = temp;

		temp = pAnimator->DoubleAttribute("y", temp);
		//m_animatorCenter.Y = temp;

		temp = pAnimator->DoubleAttribute("z", temp);
		//m_animatorCenter.Z = temp;

		temp = pAnimator->DoubleAttribute("speed", temp);
		//m_animatorSpeed = temp;
	}

	XMLElement* pBillBoard = pData->FirstChildElement("Billboard");
	if (pBillBoard)
	{
		m_addBillboard=true;

		temp = pBillBoard->DoubleAttribute("x", temp);
		//m_billboardSize.Width = temp;

		temp = pBillBoard->DoubleAttribute("y", temp);
		//m_billboardSize.Height = temp;
	
		XMLElement* pMaterial = pBillBoard->FirstChildElement("Material");
		if (pMaterial)
		{
			m_billboardMaterial = pMaterial->Attribute("type");
			m_billboardTexture = pMaterial->Attribute("texture");
		}
	}

    return true;
}
/*
eastl::shared_ptr<SceneNode> LightRenderComponent::CreateSceneNode(void)
{
    const eastl::shared_ptr<TransformComponent>& pTransformComponent(
		m_pOwner->GetComponent<TransformComponent>(TransformComponent::g_Name));
    if (pTransformComponent)
    {
		GameApplication* gameApp = (GameApplication*)Application::App;
		const eastl::shared_ptr<ScreenElementScene>& pScene = gameApp->GetHumanView()->m_pScene;
		Transform transform = pTransformComponent->GetTransform();
		eastl::weak_ptr<BaseRenderComponent> wbrcp(this);

		if (gameApp->GetRendererImpl() == ERT_DIRECT3D9)
		{
			// Add light
			eastl::shared_ptr<LightSceneNode> light = pScene->AddLightSceneNode(
				m_pOwner->GetId(), wbrcp, 0, &transform, m_Props.m_DiffuseColor, m_Props.m_Radius);

			if (m_animatorType == "flycircle")
			{
				eastl::shared_ptr<SceneNodeAnimator> anim = 0;
				anim = pScene->CreateFlyCircleAnimator(m_animatorCenter, m_animatorSpeed);
				light->AddAnimator(anim);
			}

			// attach billboard to light
			if (m_addBillboard)
			{
				eastl::shared_ptr<BillboardSceneNode> bill = 
					pScene->AddBillboardSceneNode(gameApp->GetGameLogic()->GetNewActorID(),
						eastl::weak_ptr<BaseRenderComponent>(), light, &transform, m_billboardSize);

				bill->SetMaterialFlag( EMF_LIGHTING, false);
				bill->SetMaterialType( EMT_TRANSPARENT_ADD_COLOR );
				bill->SetMaterialTexture(0, pScene->GetRenderer()->GetTexture(m_billboardTexture).get());
			}
		}
	}
    return eastl::shared_ptr<SceneNode>();
}
*/
void LightRenderComponent::CreateInheritedXMLElements(XMLDocument doc, XMLElement *pBaseElement)
{
	/*
    XMLElement* pSceneNode = doc.NewElement("Light");

    // attenuation
    XMLElement* pAttenuation = doc.NewElement("Attenuation");
    pAttenuation->SetAttribute("const", eastl::string(m_Props.m_Attenuation[0]).c_str());
    pAttenuation->SetAttribute("linear", eastl::string(m_Props.m_Attenuation[1]).c_str());
    pAttenuation->SetAttribute("exp", eastl::string(m_Props.m_Attenuation[2]).c_str());
    pSceneNode->LinkEndChild(pAttenuation);

    // shape
    XMLElement* pShape = doc.NewElement("Shape");
    pShape->SetAttribute("range", eastl::string(m_Props.m_Range).c_str());
    pShape->SetAttribute("falloff", eastl::string(m_Props.m_Falloff).c_str());
    pShape->SetAttribute("theta", eastl::string(m_Props.m_Theta).c_str());
    pShape->SetAttribute("phi", eastl::string(m_Props.m_Phi).c_str());
    pSceneNode->LinkEndChild(pShape);

    pBaseElement->LinkEndChild(pSceneNode);
	*/
}


//---------------------------------------------------------------------------------------------------------------------
// ParticleSystemRenderComponent
//---------------------------------------------------------------------------------------------------------------------
ParticleSystemRenderComponent::ParticleSystemRenderComponent(void)
{

}

bool ParticleSystemRenderComponent::DelegateInit(XMLElement* pData)
{
    return true;
}
/*
eastl::shared_ptr<SceneNode> ParticleSystemRenderComponent::CreateSceneNode(void)
{
    const eastl::shared_ptr<TransformComponent>& pTransformComponent(
		m_pOwner->GetComponent<TransformComponent>(TransformComponent::g_Name));
    if (pTransformComponent)
    {
		GameApplication* gameApp = (GameApplication*)Application::App;
		const eastl::shared_ptr<ScreenElementScene>& pScene = gameApp->GetHumanView()->m_pScene;
		Transform transform = pTransformComponent->GetTransform();
		eastl::weak_ptr<BaseRenderComponent> wbrcp(this);

		if (gameApp->GetRendererImpl() == ERT_DIRECT3D9)
		{
			// create a particle system
			eastl::shared_ptr<ParticleSystemSceneNode> ps = pScene->AddParticleSystemSceneNode(
				m_pOwner->GetId(), wbrcp, 0, &transform, false);

			if (ps)
			{
				eastl::shared_ptr<IParticleEmitter> em(
					ps->CreateBoxEmitter(
						AABBox3<f32>(-7,0,-7,7,1,7), // emitter size
						Vector3(0.0f,0.06f,0.0f),   // initial direction
						80,100,                     // emit rate
						Color(0,255,255,255),       // darkest color
						Color(0,255,255,255),       // brightest color
						800,2000,0,                         // min and max age, angle
						Vector2<float>{10.f, 10.f},         // min size
						Vector2<float>{20.f, 20.f}));        // max size

				ps->SetEmitter(em); // this grabs the emitter
		
				eastl::shared_ptr<IParticleAffector> paf(ps->CreateFadeOutParticleAffector());

				ps->AddAffector(paf); // same goes for the affector
				ps->SetPosition(Vector3<float>());
				ps->SetScale(Vector3<float>{2, 2, 2});

				ps->SetMaterialFlag(EMF_LIGHTING, false);
				ps->SetMaterialFlag(EMF_ZWRITE_ENABLE, false);
				ps->SetMaterialTexture(0, pScene->GetRenderer()->GetTexture("art/demos/demos/fire.bmp").get());
				ps->SetMaterialType(EMT_TRANSPARENT_ADD_COLOR);
			}
		}
	}
    return eastl::shared_ptr<SceneNode>();
}
*/
void ParticleSystemRenderComponent::CreateInheritedXMLElements(XMLDocument doc, XMLElement *pBaseElement)
{

}


//---------------------------------------------------------------------------------------------------------------------
// SkyRenderComponent
//---------------------------------------------------------------------------------------------------------------------
SkyRenderComponent::SkyRenderComponent(void)
{
}

bool SkyRenderComponent::DelegateInit(XMLElement* pData)
{
    XMLElement* pTexture = pData->FirstChildElement("Texture");
    if (pTexture)
	{
		m_textureResource = pTexture->FirstChild()->Value();
	}
	return true;
}
/*
eastl::shared_ptr<SceneNode> SkyRenderComponent::CreateSceneNode(void)
{
	eastl::shared_ptr<SceneNode> sky;

	GameApplication* gameApp = (GameApplication*)Application::App;
	const eastl::shared_ptr<ScreenElementScene>& pScene = gameApp->GetHumanView()->m_pScene;
	eastl::weak_ptr<BaseRenderComponent> wbrcp(this);

	if (gameApp->GetRendererImpl() == ERT_DIRECT3D9)
	{
		// add skybox
		sky = pScene->AddSkyBoxSceneNode(
			m_pOwner->GetId(), wbrcp, 0, &g_IdentityMatrix4,
			gameApp->mRenderer->GetTexture(m_textureResource + "t.jpg"),
			gameApp->mRenderer->GetTexture(m_textureResource + "b.jpg"),
			gameApp->mRenderer->GetTexture(m_textureResource + "e.jpg"),
			gameApp->mRenderer->GetTexture(m_textureResource + "w.jpg"),
			gameApp->mRenderer->GetTexture(m_textureResource + "n.jpg"),
			gameApp->mRenderer->GetTexture(m_textureResource + "s.jpg"));

		return sky;
	}
	else LogError("Unknown Renderer Implementation in GridRenderComponent");

    return eastl::shared_ptr<SceneNode>();
}
*/
void SkyRenderComponent::CreateInheritedXMLElements(XMLDocument doc, XMLElement *pBaseElement)
{
    XMLElement* pTextureNode = doc.NewElement("Texture");
    XMLText* pTextureText = doc.NewText(m_textureResource.c_str());
    pTextureNode->LinkEndChild(pTextureText);
    pBaseElement->LinkEndChild(pTextureNode);
}
