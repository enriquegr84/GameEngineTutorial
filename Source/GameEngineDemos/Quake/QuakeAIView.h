//========================================================================
// QuakeAIView.h : AI Controller class
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

#ifndef QUAKEAIVIEW_H
#define QUAKEAIVIEW_H

#include "QuakeStd.h"

#include "Core/Event/EventManager.h"

#include "Game/View/GameView.h"

#include "AI/Pathing.h"

class QuakeAIView : public BaseGameView 
{

public:
	QuakeAIView();
	virtual ~QuakeAIView();

	virtual bool OnRestore() { return true; }
	virtual void OnRender(double time, float elapsedTime) {}
	virtual bool OnLostDevice() { return true; }
	virtual GameViewType GetType() { return GV_AI; }
	virtual GameViewId GetId() const { return mViewId; }
	virtual ActorId GetActorId() const { return mPlayerId; }
	virtual void OnAttach(GameViewId vid, ActorId actorId);
	virtual bool OnMsgProc( const Event& event ) {	return false; }
	virtual void OnUpdate(unsigned int timeMs, unsigned long deltaMs);

protected:

	void Stationary(unsigned long deltaMs);
	void Movement(unsigned long deltaMs);
	void Smooth(unsigned long deltaMs);
	void Cliff();

	// Movement Controls
	int mOrientation;
	unsigned long mStationaryTime;

	// Orientation Controls
	float mYaw;
	float mYawSmooth;
	float mPitch;
	float mPitchTarget;
	float mPitchOnDown;
	float mYawOnDown;

	// Speed Controls
	float mMaxFallSpeed;
	float mMaxJumpSpeed;
	float mMaxRotateSpeed;
	float mFallSpeed;
	float mJumpSpeed;
	float mJumpMoveSpeed;
	float mMoveSpeed;
	float mRotateSpeed;

	GameViewId	mViewId;
	ActorId mPlayerId;

	Transform mAbsoluteTransform;

private:

	void RegisterAllDelegates(void);
	void RemoveAllDelegates(void);

	float mCurrentActionTime;
	unsigned int mCurrentAction;

	ActorId mCurrentActor;
	PathingArcVec mCurrentPlan;

	PathingNode* mGoalNode;
	PathingArc* mCurrentArc;
	PathingNode* mCurrentNode;
	eastl::shared_ptr<PathingGraph> mPathingGraph;
};

#endif