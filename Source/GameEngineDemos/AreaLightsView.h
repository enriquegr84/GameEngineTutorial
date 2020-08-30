//========================================================================
// DemosView.h : source file for the sample game
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

#ifndef AREALIGHTSVIEW_H
#define AREALIGHTSVIEW_H


#include "Core/Event/EventManager.h"

#include "Game/View/HumanView.h"

//! Macro for save Dropping an Element
#define DropElement(x)	if (x) { x->Remove(); x = 0; }


class AreaLightsHUD : public BaseUI
{

public:
	AreaLightsHUD();
	virtual ~AreaLightsHUD();

	// IScreenElement Implementation
	virtual bool OnInit();
	virtual bool OnRestore();
	virtual bool OnLostDevice() { return true; }

	virtual void OnUpdate(int deltaMilliseconds) { }

	//! draws all gui elements
	virtual bool OnRender(double time, float elapsedTime);

	virtual bool OnEvent(const Event& evt);
	virtual bool OnMsgProc(const Event& evt);

	virtual int GetZOrder() const { return 1; }
	virtual void SetZOrder(int const zOrder) { }

};

class AreaLightsHumanView : public HumanView
{
	friend class AreaLightsHUD;

protected:
	bool  mShowUI;					// If true, it renders the UI control text
	DebugMode mDebugMode;
    eastl::string mGameplayText;

	ActorId mLightID;

	eastl::shared_ptr<AreaLightsHUD> mAreaLightsHUD;

public:
	AreaLightsHumanView();
	virtual ~AreaLightsHumanView();

	virtual bool OnMsgProc( const Event& event );	
	virtual void RenderText();	
	virtual void OnUpdate(unsigned int timeMs, unsigned long deltaMs);
	virtual void OnAttach(GameViewId vid, ActorId aid);

	virtual bool LoadGameDelegate(tinyxml2::XMLElement* pLevelData) override;
};

#endif