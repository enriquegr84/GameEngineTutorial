//========================================================================
// BaseScriptComponent.cpp - Component for exposing actors to Lua
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

 
#include "BaseScriptComponent.h"
#include "Scripting/LUAScripting/LuaStateManager.h"

// component interfaces
#include "TransformComponent.h"
#include "PhysicsComponent.h"
#include "RenderComponentInterface.h"

// This is the name of the metatable where all the function definitions exported to Lua will live.
static const char* METATABLE_NAME = "BaseScriptComponentMetaTable";
const char* BaseScriptComponent::g_Name = "BaseScriptComponent";

BaseScriptComponent::BaseScriptComponent(void)
{
	m_scriptObject.AssignNil(LuaStateManager::Get()->GetLuaState());
    m_scriptDestructor.AssignNil(LuaStateManager::Get()->GetLuaState());
}

BaseScriptComponent::~BaseScriptComponent(void)
{
    // call the script destructor if there is one
    if (m_scriptDestructor.IsFunction())
    {
        LuaPlus::LuaFunction<void> func(m_scriptDestructor);
        func(m_scriptObject);
    }

    // clear out the script object
	m_scriptObject.AssignNil(LuaStateManager::Get()->GetLuaState());
	
    // if we were given a path for this script object, set it to nil
	if (!m_scriptObjectName.empty())
	{
		m_scriptObjectName += " = nil;";
		LuaStateManager::Get()->ExecuteString(m_scriptObjectName.c_str());
	}
}

bool BaseScriptComponent::Init(XmlElement* pData)
{
    LuaStateManager* pStateMgr = LuaStateManager::Get();
    LogAssert(pStateMgr);

    // load the <ScriptObject> tag and validate it
    XmlElement* pScriptObjectElement = pData->FirstChildElement("ScriptObject");
    if (!pScriptObjectElement)
    {
        LogError("No <ScriptObject> tag in XML. This won't be a very useful script component.");
        return true;  // technically it succeeded even though it won't be accessible
    }

    // read all the attributes
    const char* temp = NULL;
    temp = pScriptObjectElement->Attribute("var");
    if (temp)
        m_scriptObjectName = temp;

    temp = pScriptObjectElement->Attribute("constructor");
    if (temp)
        m_constructorName = temp;

    temp = pScriptObjectElement->Attribute("destructor");
    if (temp)
        m_destructorName = temp;

    // Having a var attribute will export the instance of this object to that name.
    if (!m_scriptObjectName.empty())
    {
        m_scriptObject = pStateMgr->CreatePath(m_scriptObjectName.c_str());

        if (!m_scriptObject.IsNil())
        {
            CreateScriptObject();
        }
	}

    //	The scriptConstructor attribute will also cause a Lua object to be created if 
	//	one wasn't created in the previous step. The scriptConstructor string is treated 
	//	as a function of the form f(scriptObject) and is called.
    if (!m_constructorName.empty())
    {
        m_scriptConstructor = 
			pStateMgr->GetGlobalVars().Lookup(m_constructorName.c_str());
        if (m_scriptConstructor.IsFunction())
        {
			//	m_scriptObject could be nil if there was no scriptObject attribute. 
			//	If this is the case, the Lua object is created here.
			if (m_scriptObject.IsNil())
			{
				m_scriptObject.AssignNewTable(pStateMgr->GetLuaState());
				CreateScriptObject();
			}
		}
    }

    //	The scriptDestructor attribute is treated as a function in the form of 
	//	f(scriptObject) and is called when the C++ ScriptObject instance is destroyed.
    if (!m_destructorName.empty())
    {
        m_scriptDestructor = 
			pStateMgr->GetGlobalVars().Lookup(m_destructorName.c_str());
	}

    // read the <ScriptData> tag
    XmlElement* pScriptDataElement = pData->FirstChildElement("ScriptData");
    if (pScriptDataElement)
    {
        if (m_scriptObject.IsNil())
        {
            LogError("m_scriptObject cannot be nil when ScriptData has been defined");
            return false;
        }

        for (XmlAttribute* pAttribute = pScriptDataElement->FirstAttribute(); 
			pAttribute != NULL; pAttribute = pAttribute->Next())
        {
            m_scriptObject.SetString(pAttribute->Name(), pAttribute->Value());
        }
    }

	return true;
}

void BaseScriptComponent::PostInit(void)
{
    // call the script constructor if one exists
    if (m_scriptConstructor.IsFunction())
    {
        LuaPlus::LuaFunction<bool> func(m_scriptConstructor);
        func(m_scriptObject);
    }
}

XmlElement* BaseScriptComponent::GenerateXml(void)
{
    XmlElement* pBaseElement = new XmlElement(GetName());

    // ScriptObject
    XmlElement* pScriptObjectElement = new XmlElement("ScriptObject");
    if (!m_scriptObjectName.empty())
        pScriptObjectElement->SetAttribute("var", m_scriptObjectName.c_str());
    if (!m_constructorName.empty())
        pScriptObjectElement->SetAttribute("constructor", m_constructorName.c_str());
    if (!m_destructorName.empty())
        pScriptObjectElement->SetAttribute("destructor", m_destructorName.c_str());
    pBaseElement->LinkEndChild(pScriptObjectElement);

    return pBaseElement;
}

void BaseScriptComponent::CreateScriptObject(void)
{
    LuaStateManager* pStateMgr = LuaStateManager::Get();
    LogAssert(pStateMgr);
	LogAssert(!m_scriptObject.IsNil());
	
	LuaPlus::LuaObject metaTableObj = pStateMgr->GetGlobalVars().Lookup(METATABLE_NAME);
	LogAssert(!metaTableObj.IsNil());
	
	LuaPlus::LuaObject boxedPtr = pStateMgr->GetLuaState()->BoxPointer(this);
	boxedPtr.SetMetaTable(metaTableObj);
	m_scriptObject.SetLightUserData("__object", this);
	m_scriptObject.SetMetaTable(metaTableObj);
}

void BaseScriptComponent::RegisterScriptFunctions(void)
{
	// create the metatable
	LuaPlus::LuaObject metaTableObj = LuaStateManager::Get()->GetGlobalVars().CreateTable(METATABLE_NAME);
	metaTableObj.SetObject("__index", metaTableObj);

	// transform component functions
	metaTableObj.RegisterObjectDirect("GetActorId",		        (BaseScriptComponent*)0, &BaseScriptComponent::GetActorId);
	metaTableObj.RegisterObjectDirect("GetPos",				    (BaseScriptComponent*)0, &BaseScriptComponent::GetPos);
    metaTableObj.RegisterObjectDirect("SetPos",				    (BaseScriptComponent*)0, &BaseScriptComponent::SetPos);
	metaTableObj.RegisterObjectDirect("GetLookAt",	            (BaseScriptComponent*)0, &BaseScriptComponent::GetLookAt);
	metaTableObj.RegisterObjectDirect("GetYOrientationDegrees", (BaseScriptComponent*)0, &BaseScriptComponent::GetYOrientationDegrees);
	metaTableObj.RegisterObjectDirect("RotateY",	            (BaseScriptComponent*)0, &BaseScriptComponent::RotateY);
    metaTableObj.RegisterObjectDirect("Stop",	                (BaseScriptComponent*)0, &BaseScriptComponent::Stop);

	metaTableObj.RegisterObjectDirect("SetPosition",	        (BaseScriptComponent*)0, &BaseScriptComponent::SetPosition);
}

void BaseScriptComponent::UnregisterScriptFunctions(void)
{
	LuaPlus::LuaObject metaTableObj = LuaStateManager::Get()->GetGlobalVars().Lookup(METATABLE_NAME);
	if (!metaTableObj.IsNil())
		metaTableObj.AssignNil(LuaStateManager::Get()->GetLuaState());
}

LuaPlus::LuaObject BaseScriptComponent::GetActorId(void)
{
	LogInformation("ObjectSystem", eastl::string("BaseScriptComponent::GetEntityId() return ") + eastl::string(m_pOwner->GetId()) + eastl::string(" on C++ side"));
	//LogInformation("ObjectSystem", eastl::string("BaseScriptComponent::GetEntityId() return 0x") + eastl::string(m_pOwner->GetId(), 16) + eastl::string(" on C++ side"));

	// [rez] Wtf??  Returning an unsigned int here will produce really odd results in Release mode (debug is 
	// fine).  The number will sometimes get truncated, so 0xb1437643 might become 0xb1437600.  Even though the 
	// C++ side is getting the right number, it gets mangled on the way out to Lua.  The somewhat hacky fix here 
	// is to return a LuaPlus::LuaObject, which seems to work just fine.  I'm not sure if this will cause any 
    // efficiency issues, but I'm guessing the LuaPlus internals do something like this anyway to marshal the 
    // data back to Lua land, so it's probably okay.
	LuaPlus::LuaObject ret;
	ret.AssignInteger(LuaStateManager::Get()->GetLuaState(), m_pOwner->GetId());
	return ret;

	//return m_pOwner->GetId();
}

LuaPlus::LuaObject BaseScriptComponent::GetPos(void)
{
    LuaPlus::LuaObject ret;

    eastl::shared_ptr<TransformComponent> pTransformComponent = eastl::make_shared(m_pOwner->GetComponent<TransformComponent>(TransformComponent::g_Name));
    if (pTransformComponent)
        LuaStateManager::Get()->ConvertVector3ToTable(pTransformComponent->GetPosition(), ret);
    else
        ret.AssignNil(LuaStateManager::Get()->GetLuaState());

    return ret;
}

void BaseScriptComponent::SetPos(LuaPlus::LuaObject newPos)
{
    eastl::shared_ptr<TransformComponent> pTransformComponent = eastl::make_shared(m_pOwner->GetComponent<TransformComponent>(TransformComponent::g_Name));
    if (pTransformComponent)
    {
        Vector3 pos;
        LuaStateManager::Get()->ConvertTableToVector3(newPos, pos);
        pTransformComponent->SetPosition(pos);
    }
    else
    {
        LogError(eastl::string("Attempting to call SetPos() on an actor with no physcial component; ActorId: ") + eastl::string(m_pOwner->GetId()));
    }
}


LuaPlus::LuaObject BaseScriptComponent::GetLookAt(void) const
{
    LuaPlus::LuaObject ret;

    shared_ptr<TransformComponent> pTransformComponent = eastl::make_shared(m_pOwner->GetComponent<TransformComponent>(TransformComponent::g_Name));
    if (pTransformComponent)
        LuaStateManager::Get()->ConvertVector3ToTable(pTransformComponent->GetLookAt(), ret);
    else
        ret.AssignNil(LuaStateManager::Get()->GetLuaState());

    return ret;
}

float BaseScriptComponent::GetYOrientationDegrees(void) const
{
    shared_ptr<TransformComponent> pTransformComponent = eastl::make_shared(m_pOwner->GetComponent<TransformComponent>(TransformComponent::g_Name));
    if (pTransformComponent)
    {
        return  RADTODEG * pTransformComponent->GetLookAt().Y;
    }
    else
    {
        LogError("Attempting to call GetYOrientationDegrees() on actor with no physical component");
        return 0;
    }
}

void BaseScriptComponent::RotateY(float angleRadians)
{
    shared_ptr<PhysicsComponent> pPhysicalComponent = eastl::make_shared(m_pOwner->GetComponent<PhysicsComponent>(PhysicsComponent::g_Name));
    if (pPhysicalComponent)
        pPhysicalComponent->RotateY(angleRadians);
}


void BaseScriptComponent::SetPosition(float x, float y, float z)
{
    shared_ptr<PhysicsComponent> pPhysicalComponent = eastl::make_shared(m_pOwner->GetComponent<PhysicsComponent>(PhysicsComponent::g_Name));
    if (pPhysicalComponent)
        pPhysicalComponent->SetPosition(x, y, z);
}

void BaseScriptComponent::Stop(void)
{
    shared_ptr<PhysicsComponent> pPhysicalComponent = eastl::make_shared(m_pOwner->GetComponent<PhysicsComponent>(PhysicsComponent::g_Name));
    if (pPhysicalComponent)
        pPhysicalComponent->Stop();
}