//========================================================================
// PhysicsDebugDrawer.cpp - implements a physics debug drawer in DX9 
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

#include "PhysicDebugDrawer.h"

#include "Application/GameApplication.h"

#include "Graphic/Renderer/Renderer.h"
#include "Core/Logger/Logger.h"

BulletDebugDrawer::BulletDebugDrawer()
{
	eastl::string path = FileSystem::Get()->GetPath("Effects/ColorEffect.hlsl");
	mEffect = eastl::make_shared<ColorEffect>(ProgramFactory::Get(), path);
}

void BulletDebugDrawer::drawContactPoint(
	const btVector3& PointOnB, const btVector3& normalOnB, 
	btScalar distance, int lifeTime, const btVector3& color)
{
	// draw a line to represent the normal. This only lasts one frame, and is hard to see.
	// it might help to linger this drawn object onscreen for a while to make it more noticeable
	
	btVector3 const startPoint = PointOnB;
	btVector3 const endPoint = PointOnB + normalOnB * distance;
	
	drawLine( startPoint, endPoint, color );
}

void BulletDebugDrawer::reportErrorWarning(const char* warningString)
{
    LogWarning(warningString);
}

void BulletDebugDrawer::draw3dText(const btVector3& location, const char* text)
{
	// FUTURE WORK - BulletDebugDrawer::draw3dText needs an implementation
}

void BulletDebugDrawer::setDebugMode(int debugMode)
{
    mDebugModes = (DebugDrawModes) debugMode;
}

int BulletDebugDrawer::getDebugMode() const
{
    return mDebugModes;
}

void BulletDebugDrawer::ReadOptions(tinyxml2::XMLElement *pRoot)
{
	int debugModes = btIDebugDraw::DBG_NoDebug;
	tinyxml2::XMLElement *pNode = pRoot->FirstChildElement("PhysicsDebug");
	if (pNode)
	{
		if (pNode->Attribute("DrawWireFrame"))
		{
	        eastl::string attribute(pNode->Attribute("DrawWireFrame"));
			if (attribute == "true") debugModes |= btIDebugDraw::DBG_DrawWireframe;
		}

		if (pNode->Attribute("DrawAabb"))
		{
	        eastl::string attribute(pNode->Attribute("DrawAabb"));
			if (attribute == "true") debugModes |= btIDebugDraw::DBG_DrawAabb;
		}

		if (pNode->Attribute("DrawFeaturesText"))
		{
	        eastl::string attribute(pNode->Attribute("DrawFeaturesText"));
			if (attribute == "true") debugModes |= btIDebugDraw::DBG_DrawFeaturesText;
		}

		if (pNode->Attribute("DrawContactPoints"))
		{
	        eastl::string attribute(pNode->Attribute("DrawContactPoints"));
			if (attribute == "true") debugModes |= btIDebugDraw::DBG_DrawContactPoints;
		}

		if (pNode->Attribute("NoDeactivation"))
		{
	        eastl::string attribute(pNode->Attribute("NoDeactivation"));
			if (attribute == "true") debugModes |= btIDebugDraw::DBG_NoDeactivation;
		}

		if (pNode->Attribute("NoHelpText"))
		{
	        eastl::string attribute(pNode->Attribute("NoHelpText"));
			if (attribute == "true") debugModes |= btIDebugDraw::DBG_NoHelpText;
		}

		if (pNode->Attribute("DrawText"))
		{
	        eastl::string attribute(pNode->Attribute("DrawText"));
			if (attribute == "true") debugModes |= btIDebugDraw::DBG_DrawText;
		}

		if (pNode->Attribute("ProfileTimings"))
		{
	        eastl::string attribute(pNode->Attribute("ProfileTimings"));
			if (attribute == "true") debugModes |= btIDebugDraw::DBG_ProfileTimings;
		}

		if (pNode->Attribute("EnableSatComparison"))
		{
	        eastl::string attribute(pNode->Attribute("EnableSatComparison"));
			if (attribute == "true") debugModes |= btIDebugDraw::DBG_EnableSatComparison;
		}

		if (pNode->Attribute("DisableBulletLCP"))
		{
	        eastl::string attribute(pNode->Attribute("DisableBulletLCP"));
			if (attribute == "true") debugModes |= btIDebugDraw::DBG_DisableBulletLCP;
		}

		if (pNode->Attribute("EnableCCD"))
		{
	        eastl::string attribute(pNode->Attribute("EnableCCD"));
			if (attribute == "true") debugModes |= btIDebugDraw::DBG_EnableCCD;
		}

		if (pNode->Attribute("DrawConstraints"))
		{
	        eastl::string attribute(pNode->Attribute("DrawConstraints"));
			if (attribute == "true") debugModes |= btIDebugDraw::DBG_DrawConstraints;
		}

		if (pNode->Attribute("DrawConstraintLimits"))
		{
	        eastl::string attribute(pNode->Attribute("DrawConstraintLimits"));
			if (attribute == "true") debugModes |= btIDebugDraw::DBG_DrawConstraintLimits;
		}

		if (pNode->Attribute("FastWireframe"))
		{
	        eastl::string attribute(pNode->Attribute("FastWireframe"));
			if (attribute == "true") debugModes |= btIDebugDraw::DBG_FastWireframe;
		}

		if (debugModes != btIDebugDraw::DBG_NoDebug)
		{
			setDebugMode(debugModes);
		}
	}

}

void BulletDebugDrawer::drawLine(
	const btVector3& from, const btVector3& to, const btVector3& lineColor)
{
	Vector3<float> vFrom, vTo;
	vFrom[0] = from.x();
	vFrom[1] = from.y();
	vFrom[2] = from.z();

	vTo[0] = to.x();
	vTo[1] = to.y();
	vTo[2] = to.z();

	Vector4<float> color{ lineColor.x(), lineColor.y(), lineColor.z(), 1.f };
	
	Vertex* vertexFrom = new Vertex();
	vertexFrom->position = vFrom;
	vertexFrom->color = color;

	Vertex* vertexTo = new Vertex();
	vertexTo->position = vTo;
	vertexTo->color = color;
	mVertices[vertexFrom] = vertexTo;
}

void BulletDebugDrawer::Render()
{
	VertexFormat vformat;
	vformat.Bind(VA_POSITION, DF_R32G32B32_FLOAT, 0);
	vformat.Bind(VA_COLOR, DF_R32G32B32A32_FLOAT, 0);
	eastl::shared_ptr<VertexBuffer> vbuffer =
		eastl::make_shared<VertexBuffer>(vformat, mVertices.size()*2);
	vbuffer->SetUsage(Resource::DYNAMIC_UPDATE);
	eastl::shared_ptr<IndexBuffer> ibuffer =
		eastl::make_shared<IndexBuffer>(IP_POLYSEGMENT_DISJOINT, mVertices.size());
	eastl::shared_ptr<Visual> visual = eastl::make_shared<Visual>(vbuffer, ibuffer, mEffect);

	int vertexCount = 0;
	Vertex* vertex = vbuffer->Get<Vertex>();
	eastl::map<Vertex*, Vertex*>::iterator itVertex = mVertices.begin();
	for (; itVertex != mVertices.end(); ++itVertex)
	{
		vertex[vertexCount].position = (*itVertex).first->position;
		vertex[vertexCount].color = (*itVertex).first->color;
		vertex[vertexCount+1].position = (*itVertex).second->position;
		vertex[vertexCount+1].color = (*itVertex).second->color;
		vertexCount += 2;
	}

	GameApplication* gameApp = (GameApplication*)Application::App;
	const eastl::shared_ptr<ScreenElementScene>& pScene = gameApp->GetHumanView()->mScene;

	eastl::shared_ptr<ConstantBuffer> cbuffer;
	cbuffer = mEffect->GetVertexShader()->Get<ConstantBuffer>("PVWMatrix");
	*cbuffer->Get<Matrix4x4<float>>() = pScene->GetActiveCamera()->Get()->GetProjectionViewMatrix();

	Renderer* renderer = Renderer::Get();
	renderer->Update(cbuffer);
	renderer->Update(vbuffer);
	renderer->Draw(visual);
}

void BulletDebugDrawer::Clear()
{
	eastl::map<Vertex*, Vertex*>::iterator itVertex = mVertices.begin();
	for (; itVertex != mVertices.end(); ++itVertex)
	{
		delete (*itVertex).first;
		delete (*itVertex).second;
	}
	mVertices.clear();
}