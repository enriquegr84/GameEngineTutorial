//========================================================================
// HumanView.cpp - Implements the class HumanView, which provides a Human interface into the game
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

#include "UserInterface.h"

#include "Core/IO/XmlResource.h"
#include "Core/OS/Os.h"

#include "Graphic/Image/ImageResource.h"

//! constructor
BaseUI::BaseUI()
	: mHovered(0), mHoveredNoSubelement(0), mFocus(0), 
	mLastHoveredMousePos{ 0,0 }, mCurrentSkin(0)
{
	Renderer* renderer = Renderer::Get();
	Vector2<unsigned int> screenSize(renderer->GetScreenSize());
	RectangleShape<2, int> screenRectangle;
	screenRectangle.mCenter[0] = screenSize[0] / 2;
	screenRectangle.mCenter[1] = screenSize[1] / 2;
	screenRectangle.mExtent[0] = (int)screenSize[0];
	screenRectangle.mExtent[1] = (int)screenSize[1];

	mRoot = eastl::shared_ptr<BaseUIElement>(
		new UIRoot(this, UIET_ROOT, 0, screenRectangle));

	// environment is root tab group
	mRoot->SetTabGroup(true);
}


//! destructor
BaseUI::~BaseUI()
{

}

bool BaseUI::OnInit()
{
	//LoadBuiltInFont
	eastl::vector<eastl::string> path;
#if defined(_OPENGL_)
	path.push_back("Effects/TextEffectVS.glsl");
	path.push_back("Effects/TextEffectPS.glsl");
#else
	path.push_back("Effects/TextEffectVS.hlsl");
	path.push_back("Effects/TextEffectPS.hlsl");
#endif
	eastl::shared_ptr<Font> builtInFont = eastl::make_shared<FontArialW400H18>(ProgramFactory::Get(), path, 256);
	Renderer::Get()->SetDefaultFont(builtInFont);
	mFonts[L"DefaultFont"] = eastl::shared_ptr<BaseUIFont>(new UIFont(this, L"DefaultFont", builtInFont));

	const eastl::shared_ptr<BaseUISkin>& skin = CreateSkin( STT_WINDOWS_CLASSIC );
	SetSkin(skin);

	//set tooltip default
	mToolTip.mLastTime = 0;
	mToolTip.mEnterTime = 0;
	mToolTip.mLaunchTime = 1000;
	mToolTip.mRelaunchTime = 500;
	mToolTip.mElement = 
		AddStaticText(L"", RectangleShape<2, int>(), true, true, mRoot, -1, true);

	eastl::shared_ptr<ResHandle>& resHandle = 
		ResCache::Get()->GetHandle(&BaseResource(mCurrentSkin->GetIcon(DI_WINDOW_MAXIMIZE)));
	if (resHandle)
	{
		const eastl::shared_ptr<ImageResourceExtraData>& extra =
			eastl::static_pointer_cast<ImageResourceExtraData>(resHandle->GetExtra());
		extra->GetImage()->AutogenerateMipmaps();
		extra->GetImage()->SetName(mCurrentSkin->GetIcon(DI_WINDOW_MAXIMIZE));
		mCurrentSkin->GetSpriteBank()->AddTextureAsSprite(extra->GetImage());
	}

	resHandle = ResCache::Get()->GetHandle(&BaseResource(mCurrentSkin->GetIcon(DI_WINDOW_RESTORE)));
	if (resHandle)
	{
		const eastl::shared_ptr<ImageResourceExtraData>& extra =
			eastl::static_pointer_cast<ImageResourceExtraData>(resHandle->GetExtra());
		extra->GetImage()->AutogenerateMipmaps();
		extra->GetImage()->SetName(mCurrentSkin->GetIcon(DI_WINDOW_RESTORE));
		mCurrentSkin->GetSpriteBank()->AddTextureAsSprite(extra->GetImage());
	}

	resHandle = ResCache::Get()->GetHandle(&BaseResource(mCurrentSkin->GetIcon(DI_WINDOW_CLOSE)));
	if (resHandle)
	{
		const eastl::shared_ptr<ImageResourceExtraData>& extra =
			eastl::static_pointer_cast<ImageResourceExtraData>(resHandle->GetExtra());
		extra->GetImage()->AutogenerateMipmaps();
		extra->GetImage()->SetName(mCurrentSkin->GetIcon(DI_WINDOW_CLOSE));
		mCurrentSkin->GetSpriteBank()->AddTextureAsSprite(extra->GetImage());
	}

	resHandle = ResCache::Get()->GetHandle(&BaseResource(mCurrentSkin->GetIcon(DI_WINDOW_MINIMIZE)));
	if (resHandle)
	{
		const eastl::shared_ptr<ImageResourceExtraData>& extra =
			eastl::static_pointer_cast<ImageResourceExtraData>(resHandle->GetExtra());
		extra->GetImage()->AutogenerateMipmaps();
		extra->GetImage()->SetName(mCurrentSkin->GetIcon(DI_WINDOW_MINIMIZE));
		mCurrentSkin->GetSpriteBank()->AddTextureAsSprite(extra->GetImage());
	}

	resHandle = ResCache::Get()->GetHandle(&BaseResource(mCurrentSkin->GetIcon(DI_WINDOW_RESIZE)));
	if (resHandle)
	{
		const eastl::shared_ptr<ImageResourceExtraData>& extra =
			eastl::static_pointer_cast<ImageResourceExtraData>(resHandle->GetExtra());
		extra->GetImage()->AutogenerateMipmaps();
		extra->GetImage()->SetName(mCurrentSkin->GetIcon(DI_WINDOW_RESIZE));
		mCurrentSkin->GetSpriteBank()->AddTextureAsSprite(extra->GetImage());
	}

	resHandle = ResCache::Get()->GetHandle(&BaseResource(mCurrentSkin->GetIcon(DI_CURSOR_UP)));
	if (resHandle)
	{
		const eastl::shared_ptr<ImageResourceExtraData>& extra =
			eastl::static_pointer_cast<ImageResourceExtraData>(resHandle->GetExtra());
		extra->GetImage()->AutogenerateMipmaps();
		extra->GetImage()->SetName(mCurrentSkin->GetIcon(DI_CURSOR_UP));
		mCurrentSkin->GetSpriteBank()->AddTextureAsSprite(extra->GetImage());
	}

	resHandle = ResCache::Get()->GetHandle(&BaseResource(mCurrentSkin->GetIcon(DI_CURSOR_DOWN)));
	if (resHandle)
	{
		const eastl::shared_ptr<ImageResourceExtraData>& extra =
			eastl::static_pointer_cast<ImageResourceExtraData>(resHandle->GetExtra());
		extra->GetImage()->AutogenerateMipmaps();
		extra->GetImage()->SetName(mCurrentSkin->GetIcon(DI_CURSOR_DOWN));
		mCurrentSkin->GetSpriteBank()->AddTextureAsSprite(extra->GetImage());
	}

	resHandle = ResCache::Get()->GetHandle(&BaseResource(mCurrentSkin->GetIcon(DI_CURSOR_LEFT)));
	if (resHandle)
	{
		const eastl::shared_ptr<ImageResourceExtraData>& extra =
			eastl::static_pointer_cast<ImageResourceExtraData>(resHandle->GetExtra());
		extra->GetImage()->AutogenerateMipmaps();
		extra->GetImage()->SetName(mCurrentSkin->GetIcon(DI_CURSOR_LEFT));
		mCurrentSkin->GetSpriteBank()->AddTextureAsSprite(extra->GetImage());
	}
	
	resHandle = ResCache::Get()->GetHandle(&BaseResource(mCurrentSkin->GetIcon(DI_CURSOR_RIGHT)));
	if (resHandle)
	{
		const eastl::shared_ptr<ImageResourceExtraData>& extra =
			eastl::static_pointer_cast<ImageResourceExtraData>(resHandle->GetExtra());
		extra->GetImage()->AutogenerateMipmaps();
		extra->GetImage()->SetName(mCurrentSkin->GetIcon(DI_CURSOR_RIGHT));
		mCurrentSkin->GetSpriteBank()->AddTextureAsSprite(extra->GetImage());
	}

	resHandle = ResCache::Get()->GetHandle(&BaseResource(mCurrentSkin->GetIcon(DI_MENU_MORE)));
	if (resHandle)
	{
		const eastl::shared_ptr<ImageResourceExtraData>& extra =
			eastl::static_pointer_cast<ImageResourceExtraData>(resHandle->GetExtra());
		extra->GetImage()->AutogenerateMipmaps();
		extra->GetImage()->SetName(mCurrentSkin->GetIcon(DI_MENU_MORE));
		mCurrentSkin->GetSpriteBank()->AddTextureAsSprite(extra->GetImage());
	}

	resHandle = ResCache::Get()->GetHandle(&BaseResource(mCurrentSkin->GetIcon(DI_CHECKBOX_CHECKED)));
	if (resHandle)
	{
		const eastl::shared_ptr<ImageResourceExtraData>& extra =
			eastl::static_pointer_cast<ImageResourceExtraData>(resHandle->GetExtra());
		extra->GetImage()->AutogenerateMipmaps();
		extra->GetImage()->SetName(mCurrentSkin->GetIcon(DI_CHECKBOX_CHECKED));
		mCurrentSkin->GetSpriteBank()->AddTextureAsSprite(extra->GetImage());
	}

	resHandle = ResCache::Get()->GetHandle(&BaseResource(mCurrentSkin->GetIcon(DI_CHECKBOX_UNCHECKED)));
	if (resHandle)
	{
		const eastl::shared_ptr<ImageResourceExtraData>& extra =
			eastl::static_pointer_cast<ImageResourceExtraData>(resHandle->GetExtra());
		extra->GetImage()->AutogenerateMipmaps();
		extra->GetImage()->SetName(mCurrentSkin->GetIcon(DI_CHECKBOX_UNCHECKED));
		mCurrentSkin->GetSpriteBank()->AddTextureAsSprite(extra->GetImage());
	}

	resHandle = ResCache::Get()->GetHandle(&BaseResource(mCurrentSkin->GetIcon(DI_DROP_DOWN)));
	if (resHandle)
	{
		const eastl::shared_ptr<ImageResourceExtraData>& extra =
			eastl::static_pointer_cast<ImageResourceExtraData>(resHandle->GetExtra());
		extra->GetImage()->AutogenerateMipmaps();
		extra->GetImage()->SetName(mCurrentSkin->GetIcon(DI_DROP_DOWN));
		mCurrentSkin->GetSpriteBank()->AddTextureAsSprite(extra->GetImage());
	}

	resHandle = ResCache::Get()->GetHandle(&BaseResource(mCurrentSkin->GetIcon(DI_SMALL_CURSOR_UP)));
	if (resHandle)
	{
		const eastl::shared_ptr<ImageResourceExtraData>& extra =
			eastl::static_pointer_cast<ImageResourceExtraData>(resHandle->GetExtra());
		extra->GetImage()->AutogenerateMipmaps();
		extra->GetImage()->SetName(mCurrentSkin->GetIcon(DI_SMALL_CURSOR_UP));
		mCurrentSkin->GetSpriteBank()->AddTextureAsSprite(extra->GetImage());
	}

	resHandle = ResCache::Get()->GetHandle(&BaseResource(mCurrentSkin->GetIcon(DI_SMALL_CURSOR_DOWN)));
	if (resHandle)
	{
		const eastl::shared_ptr<ImageResourceExtraData>& extra =
			eastl::static_pointer_cast<ImageResourceExtraData>(resHandle->GetExtra());
		extra->GetImage()->AutogenerateMipmaps();
		extra->GetImage()->SetName(mCurrentSkin->GetIcon(DI_SMALL_CURSOR_DOWN));
		mCurrentSkin->GetSpriteBank()->AddTextureAsSprite(extra->GetImage());
	}

	resHandle = ResCache::Get()->GetHandle(&BaseResource(mCurrentSkin->GetIcon(DI_RADIO_BUTTON_CHECKED)));
	if (resHandle)
	{
		const eastl::shared_ptr<ImageResourceExtraData>& extra =
			eastl::static_pointer_cast<ImageResourceExtraData>(resHandle->GetExtra());
		extra->GetImage()->AutogenerateMipmaps();
		extra->GetImage()->SetName(mCurrentSkin->GetIcon(DI_RADIO_BUTTON_CHECKED));
		mCurrentSkin->GetSpriteBank()->AddTextureAsSprite(extra->GetImage());
	}

	resHandle = ResCache::Get()->GetHandle(&BaseResource(mCurrentSkin->GetIcon(DI_MORE_LEFT)));
	if (resHandle)
	{
		const eastl::shared_ptr<ImageResourceExtraData>& extra =
			eastl::static_pointer_cast<ImageResourceExtraData>(resHandle->GetExtra());
		extra->GetImage()->AutogenerateMipmaps();
		extra->GetImage()->SetName(mCurrentSkin->GetIcon(DI_MORE_LEFT));
		mCurrentSkin->GetSpriteBank()->AddTextureAsSprite(extra->GetImage());
	}

	resHandle = ResCache::Get()->GetHandle(&BaseResource(mCurrentSkin->GetIcon(DI_MORE_RIGHT)));
	if (resHandle)
	{
		const eastl::shared_ptr<ImageResourceExtraData>& extra =
			eastl::static_pointer_cast<ImageResourceExtraData>(resHandle->GetExtra());
		extra->GetImage()->AutogenerateMipmaps();
		extra->GetImage()->SetName(mCurrentSkin->GetIcon(DI_MORE_RIGHT));
		mCurrentSkin->GetSpriteBank()->AddTextureAsSprite(extra->GetImage());
	}

	resHandle = ResCache::Get()->GetHandle(&BaseResource(mCurrentSkin->GetIcon(DI_MORE_UP)));
	if (resHandle)
	{
		const eastl::shared_ptr<ImageResourceExtraData>& extra =
			eastl::static_pointer_cast<ImageResourceExtraData>(resHandle->GetExtra());
		extra->GetImage()->AutogenerateMipmaps();
		extra->GetImage()->SetName(mCurrentSkin->GetIcon(DI_MORE_UP));
		mCurrentSkin->GetSpriteBank()->AddTextureAsSprite(extra->GetImage());
	}

	resHandle = ResCache::Get()->GetHandle(&BaseResource(mCurrentSkin->GetIcon(DI_MORE_DOWN)));
	if (resHandle)
	{
		const eastl::shared_ptr<ImageResourceExtraData>& extra =
			eastl::static_pointer_cast<ImageResourceExtraData>(resHandle->GetExtra());
		extra->GetImage()->AutogenerateMipmaps();
		extra->GetImage()->SetName(mCurrentSkin->GetIcon(DI_MORE_DOWN));
		mCurrentSkin->GetSpriteBank()->AddTextureAsSprite(extra->GetImage());
	}

	resHandle = ResCache::Get()->GetHandle(&BaseResource(mCurrentSkin->GetIcon(DI_EXPAND)));
	if (resHandle)
	{
		const eastl::shared_ptr<ImageResourceExtraData>& extra =
			eastl::static_pointer_cast<ImageResourceExtraData>(resHandle->GetExtra());
		extra->GetImage()->AutogenerateMipmaps();
		extra->GetImage()->SetName(mCurrentSkin->GetIcon(DI_EXPAND));
		mCurrentSkin->GetSpriteBank()->AddTextureAsSprite(extra->GetImage());
	}

	resHandle = ResCache::Get()->GetHandle(&BaseResource(mCurrentSkin->GetIcon(DI_COLLAPSE)));
	if (resHandle)
	{
		const eastl::shared_ptr<ImageResourceExtraData>& extra =
			eastl::static_pointer_cast<ImageResourceExtraData>(resHandle->GetExtra());
		extra->GetImage()->AutogenerateMipmaps();
		extra->GetImage()->SetName(mCurrentSkin->GetIcon(DI_COLLAPSE));
		mCurrentSkin->GetSpriteBank()->AddTextureAsSprite(extra->GetImage());
	}

	return true;
}

//! draws all gui elements
bool BaseUI::OnRender(double time, float elapsedTime)
{
	Renderer* renderer = Renderer::Get();
	Vector2<unsigned int> screenSize(renderer->GetScreenSize());
	if ((mRoot->mAbsoluteRect.mCenter[0] + (int)round(mRoot->mAbsoluteRect.mExtent[0] / 2.f)) != screenSize[0] ||
		(mRoot->mAbsoluteRect.mCenter[1] + (int)round(mRoot->mAbsoluteRect.mExtent[1] / 2.f)) != screenSize[1])
	{
		// resize gui environment
		Vector2<int> center(mRoot->mDesiredRect.mCenter);
		mRoot->mDesiredRect.mCenter[0] = (int)screenSize[0] - (int)round(mRoot->mDesiredRect.mExtent[0] / 2.f);
		mRoot->mDesiredRect.mCenter[1] = (int)screenSize[1] - (int)round(mRoot->mDesiredRect.mExtent[1] / 2.f);
		mRoot->mDesiredRect.mExtent[0] = 2 * ((int)screenSize[0] - center[0]);
		mRoot->mDesiredRect.mExtent[1] = 2 * ((int)screenSize[1] - center[1]);
		mRoot->mAbsoluteClippingRect = mRoot->mDesiredRect;
		mRoot->mAbsoluteRect = mRoot->mDesiredRect;
		mRoot->UpdateAbsolutePosition();
	}

	// make sure tooltip is always on top
	if (mToolTip.mElement->IsVisible())
		mRoot->BringToFront(mToolTip.mElement);

	mRoot->Draw();

	return OnPostRender ( Timer::GetTime () );
}


//! clear all UI elements
void BaseUI::Clear()
{
	// Remove the focus
	if (mFocus)
		mFocus = 0;

	if (mHovered && mHovered != mRoot)
		mHovered = 0;
	
	if ( mHoveredNoSubelement && mHoveredNoSubelement != mRoot)
		mHoveredNoSubelement = 0;

	// get the root's children in case the root changes in future
	const eastl::list<eastl::shared_ptr<BaseUIElement>>& children =
		GetRootUIElement()->GetChildren();

	while (!children.empty())
		children.back()->Remove();
	/*
	// delete all sprite banks
	for (int i = 0; i < mBanks.size(); ++i)
		if (mBanks[i].mBank)
			delete mBanks[i].mBank;

	// delete all fonts
	for (int i = 0; i < mFonts.size(); ++i)
		delete mFonts[i]->mFont;

	// remove all factories
	for (int i = 0; i < UIElementFactoryList.size(); ++i)
		delete UIElementFactoryList[i];
	*/
}

//
bool BaseUI::OnPostRender( unsigned int time )
{
	// launch tooltip
	if (!mToolTip.mElement->IsVisible() &&
		mHoveredNoSubelement && mHoveredNoSubelement != mRoot &&
		(time - mToolTip.mEnterTime >= mToolTip.mLaunchTime || 
		(time - mToolTip.mLastTime >= mToolTip.mRelaunchTime && 
		time - mToolTip.mLastTime < mToolTip.mLaunchTime)) &&
		mHoveredNoSubelement->GetToolTipText().size() &&
		GetSkin() && GetSkin()->GetFont(DF_TOOLTIP))
	{
		RectangleShape<2, int> pos;

		Vector2<int> dim = GetSkin()->GetFont(DF_TOOLTIP)->GetDimension(
			mHoveredNoSubelement->GetToolTipText().c_str());
		dim[0] += GetSkin()->GetSize(DS_TEXT_DISTANCE_X) * 2;
		dim[1] += GetSkin()->GetSize(DS_TEXT_DISTANCE_Y) * 2;

		pos.mCenter[0] = mLastHoveredMousePos[0] + (dim[0] / 2);
		pos.mExtent[0] = dim[0];
		pos.mCenter[1] = mLastHoveredMousePos[1] - (dim[1] / 2);
		pos.mExtent[1] = dim[1] - 2;

		mToolTip.mElement->SetVisible(true);
		mToolTip.mElement->SetRelativePosition(pos);
		mToolTip.mElement->SetTextAlignment(UIA_UPPERLEFT, UIA_CENTER);
		mToolTip.mElement->SetText(mHoveredNoSubelement->GetToolTipText().c_str());
		mToolTip.mElement->SetOverrideColor(GetSkin()->GetColor(DC_TOOLTIP));
		mToolTip.mElement->SetBackgroundColor(GetSkin()->GetColor(DC_TOOLTIP_BACKGROUND));
		mToolTip.mElement->SetOverrideFont(GetSkin()->GetFont(DF_TOOLTIP));
		mToolTip.mElement->SetSubElement(true);
	}

	// (IsVisible() check only because we might use visibility for ToolTip one day)
	if (mToolTip.mElement && mToolTip.mElement->IsVisible())
	{
		mToolTip.mLastTime = time;

		// got invisible or removed in the meantime?
		if (!mHoveredNoSubelement ||
			!mHoveredNoSubelement->IsVisible() ||
			!mHoveredNoSubelement->GetParent())	// got invisible or removed in the meantime?
		{
			mToolTip.mElement->SetVisible(false);
		}
	}

	mRoot->OnPostRender ( time );
	return true;
}


//
void BaseUI::UpdateHoveredElement(Vector2<int> mousePos)
{
	eastl::shared_ptr<BaseUIElement> lastHovered = mHovered;
	eastl::shared_ptr<BaseUIElement> lastHoveredNoSubelement = mHoveredNoSubelement;
	mLastHoveredMousePos = mousePos;

	mHovered = mRoot->GetElementFromPoint(mousePos);

	if (mToolTip.mElement && mHovered == mToolTip.mElement)
	{
		/*
		// When the mouse is over the ToolTip we remove that so it will be re-created at a new position.
		// Note that ToolTip.EnterTime does not get changed here, so it will be re-created at once.
		mToolTip.mElement->SetVisible(false);

		// Get the real Hovered
		mHovered = mRoot->GetElementFromPoint(mousePos);
		*/
		mHovered = lastHovered;
	}

	// for tooltips we want the element itself and not some of it's subelements
	mHoveredNoSubelement = mHovered;
	while ( mHoveredNoSubelement && mHoveredNoSubelement->IsSubElement() )
	{
		mHoveredNoSubelement = mHoveredNoSubelement->GetParent();
	}

	if (mHovered != lastHovered)
	{
		Event ev;
		ev.mEventType = ET_UI_EVENT;

		if (lastHovered)
		{
			ev.mUIEvent.mCaller = lastHovered.get();
			ev.mUIEvent.mElement = 0;
			ev.mUIEvent.mEventType = UIEVT_ELEMENT_LEFT;
			lastHovered->OnEvent(ev);
		}

		if ( mHovered )
		{
			ev.mUIEvent.mCaller  = mHovered.get();
			ev.mUIEvent.mElement = mHovered.get();
			ev.mUIEvent.mEventType = UIEVT_ELEMENT_HOVERED;
			mHovered->OnEvent(ev);
		}
	}

	if (lastHoveredNoSubelement != mHoveredNoSubelement)
	{
		if (mToolTip.mElement)
			mToolTip.mElement->SetVisible(false);

		if (mHoveredNoSubelement)
		{
			unsigned int now = Timer::GetTime();
			mToolTip.mEnterTime = now;
		}
	}
}

//! posts an input event to the environment
bool BaseUI::OnMsgProc(const Event& ev)
{
	switch(ev.mEventType)
	{
		case ET_UI_EVENT:
			// hey, why is the user sending gui events..?
			break;
		case ET_MOUSE_INPUT_EVENT:

			UpdateHoveredElement(Vector2<int>{ev.mMouseInput.X, ev.mMouseInput.Y});

			if (ev.mMouseInput.mEvent == MIE_LMOUSE_PRESSED_DOWN)
				if ( (mHovered && mHovered != mFocus) || !mFocus )
			{
				SetFocus(mHovered);
			}

			// sending input to focus
			if (mFocus && mFocus->OnEvent(ev))
				return true;

			// focus could have died in last call
			if (!mFocus && mHovered)
				return mHovered->OnEvent(ev);

			break;
		case ET_KEY_INPUT_EVENT:
			{
				if (mFocus && mFocus->OnEvent(ev))
					return true;

				// For keys we handle the event before changing focus to give elements 
				// the chance for catching the TAB. Send focus changing event
				if (ev.mEventType == ET_KEY_INPUT_EVENT &&
					ev.mKeyInput.mPressedDown &&
					ev.mKeyInput.mKey == KEY_TAB)
				{
					const eastl::shared_ptr<BaseUIElement>& next =
						GetNextElement(ev.mKeyInput.mShift, ev.mKeyInput.mControl);
					if (next && next != mFocus)
					{
						if (SetFocus(next))
							return true;
					}
				}

			}
			break;
		default:
			break;
	} // end switch

	return false;
}


//! sets the focus to an element
bool BaseUI::SetFocus(eastl::shared_ptr<BaseUIElement> element)
{
	if (mFocus == element)
		return false;

	// UI Environment should not get the focus
	if (element == mRoot)
		element = 0;

	// focus may change or be removed in this call
	eastl::shared_ptr<BaseUIElement> currentFocus = 0;
	if (mFocus)
	{
		currentFocus = mFocus;
		Event ev;
		ev.mEventType = ET_UI_EVENT;
		ev.mUIEvent.mCaller = mFocus.get();
		ev.mUIEvent.mElement = element.get();
		ev.mUIEvent.mEventType = UIEVT_ELEMENT_FOCUS_LOST;
		if (mFocus->OnEvent(ev))
			return false;

		currentFocus = 0;
	}

	if (element)
	{
		currentFocus = mFocus;

		// send Focused event
		Event ev;
		ev.mEventType = ET_UI_EVENT;
		ev.mUIEvent.mCaller = element.get();
		ev.mUIEvent.mElement = mFocus.get();
		ev.mUIEvent.mEventType = UIEVT_ELEMENT_FOCUSED;
		if (element->OnEvent(ev))
			return false;
	}

	// element is the new focus so it doesn't have to be dropped
	mFocus = element;

	return true;
}


//! returns the element with the focus
const eastl::shared_ptr<BaseUIElement>& BaseUI::GetFocus() const
{
	return mFocus;
}

//! returns the element last known to be under the mouse cursor
const eastl::shared_ptr<BaseUIElement>& BaseUI::GetHovered() const
{
	return mHovered;
}


//! removes the focus from an element
bool BaseUI::RemoveFocus(const eastl::shared_ptr<BaseUIElement>& element)
{
	if (mFocus && mFocus==element)
	{
		Event ev;
		ev.mEventType = ET_UI_EVENT;
		ev.mUIEvent.mCaller = mFocus.get();
		ev.mUIEvent.mElement = 0;
		ev.mUIEvent.mEventType = UIEVT_ELEMENT_FOCUS_LOST;
		if (mFocus->OnEvent(ev))
			return false;
	}

	if (mFocus)
		mFocus = 0;

	return true;
}


//! Returns whether the element has focus
bool BaseUI::HasFocus(const eastl::shared_ptr<BaseUIElement>& element, bool checkSubElements) const
{
	if (element == mFocus)
		return true;

	if ( !checkSubElements || !element )
		return false;

	eastl::shared_ptr<BaseUIElement> f = mFocus;
	while ( f && f->IsSubElement() )
	{
		f = f->GetParent();
		if ( f == element )
			return true;
	}
	return false;
}


//! returns the current gui skin
const eastl::shared_ptr<BaseUISkin>& BaseUI::GetSkin() const
{
	return mCurrentSkin;
}


//! Sets a new UI Skin
void BaseUI::SetSkin(const eastl::shared_ptr<BaseUISkin>& skin)
{
	mCurrentSkin = skin;
}


//! Creates a new UI Skin based on a template.
// \return Returns a pointer to the created skin.
//	If you no longer need the skin, you should call BaseUISkin::drop().
//	See IReferenceCounted::drop() for more information.
eastl::shared_ptr<BaseUISkin> BaseUI::CreateSkin(UISkinThemeType type)
{

	eastl::shared_ptr<UISkin> skin(new UISkin(this, type));
	/*
	To make the font a little bit nicer, we load an external font
	and set it as the new default font in the skin.
	*/
	eastl::shared_ptr<BaseUIFont> font(GetFont(L"DefaultFont"));
	if (font) skin->SetFont(font);

	BaseUIFontBitmap* bitfont = 0;
	if (font && font->GetType() == FT_BITMAP)
		bitfont = (BaseUIFontBitmap*)font.get();

	skin->SetFont(font);

	eastl::shared_ptr<BaseUISpriteBank> bank = 0;
	if (bitfont)
		bank = bitfont->GetSpriteBank();
	skin->SetSpriteBank(bank);

	return skin;
}

//! returns the font
eastl::shared_ptr<BaseUIFont> BaseUI::GetFont(const eastl::wstring& fileName)
{
	auto itFont = mFonts.find(fileName);
	if (itFont != mFonts.end()) return mFonts[fileName];

	tinyxml2::XMLElement* pRoot = XmlResourceLoader::LoadAndReturnRootXMLElement(fileName.c_str());
    // font doesn't exist, attempt to load it
	if (!pRoot)
    {
        LogError(L"Failed to find resource file: " + fileName);
        return nullptr;
    }
	
	bool found=false;
	// this is an XML font, but we need to know what type
	UIFontType t = FT_CUSTOM;
	// Loop through each child element and load the component
    for (tinyxml2::XMLElement* pNode = pRoot->FirstChildElement(); pNode; pNode = pNode->NextSiblingElement())
    {
		if (eastl::string("font").compare(eastl::string(pNode->Value())) == 0)
		{
			eastl::string type(pNode->Attribute("type"));
			if (eastl::string("vector").compare(type) == 0)
			{
				t = FT_VECTOR;
				found=true;
			}
			else if (eastl::string("bitmap").compare(type) == 0)
			{
				t = FT_BITMAP;
				found=true;
			}
			else found=true;
		}
        unsigned char flags = 0;
      
    }

	eastl::shared_ptr<UIFont> font=0;
	if (t==FT_BITMAP)
	{
		font = eastl::shared_ptr<UIFont>(new UIFont(this, fileName));
		// change working directory, for loading textures
		eastl::wstring workingDir = FileSystem::Get()->GetWorkingDirectory();
		FileSystem::Get()->ChangeWorkingDirectoryTo(FileSystem::Get()->GetFileDir(fileName));

		// load the font
		if (!((UIFont*)font.get())->Load(fileName.c_str()))
			font = 0;

		// change working dir back again
		FileSystem::Get()->ChangeWorkingDirectoryTo( workingDir );
	}
	else if (t==FT_VECTOR)
	{
		// todo: vector fonts
		LogError(L"Unable to load font, XML vector fonts are not supported yet " + fileName);

		//UIFontVector* fontVector = new UIFontVector(Driver);
		//font = eastl::shared_ptr<BaseUIFont>(font);
		//if (!font->Load(pRoot))
	}

	if (!font)
	{
		font = eastl::shared_ptr<UIFont>(new UIFont(this, fileName));
		if (!font->Load(fileName))
		{
			font = 0;
			return nullptr;
		}
	}

	// add to fonts.
	mFonts[fileName] = font;
	return font;
}


//! add an externally loaded font
const eastl::shared_ptr<BaseUIFont>& BaseUI::AddFont(
	const eastl::wstring& name, const eastl::shared_ptr<BaseUIFont>& font)
{
	if (font)
	{
		auto itFont = mFonts.find(name);
		if (itFont == mFonts.end()) mFonts[name] = font;
	}

	return font;
}

//! remove loaded font
void BaseUI::RemoveFont(const eastl::shared_ptr<BaseUIFont>& font)
{
	if ( !font )
		return;

	auto itFont = mFonts.begin();
	for (; itFont != mFonts.end(); itFont++)
	{
		if ((*itFont).second == font)
		{
			mFonts.erase(itFont);
			return;
		}
	}
}

//! returns default font
eastl::shared_ptr<BaseUIFont> BaseUI::GetBuiltInFont()
{
	if (mFonts.empty())
		return nullptr;

	return mFonts[L"DefaultFont"];
}

eastl::shared_ptr<BaseUISpriteBank> BaseUI::GetSpriteBank(const eastl::wstring& fileName)
{
	auto itBank = mBanks.find(fileName);
	if (itBank != mBanks.end()) return mBanks[fileName];

	// todo: load it!
	/*
	tinyxml2::XMLElement* pRoot = XmlResourceLoader::LoadAndReturnRootXMLElement(fileName.c_str());
	// font doesn't exist, attempt to load it
	if (!pRoot)
	{
		LogError(L"Failed to find resource file: " + fileName);
		return nullptr;
	}
	*/
	return nullptr;
}


eastl::shared_ptr<BaseUISpriteBank> BaseUI::AddEmptySpriteBank(const eastl::wstring& fileName)
{
	// no duplicate names allowed
	auto itBank = mBanks.find(fileName);
	if (itBank != mBanks.end()) return mBanks[fileName];

	mBanks[fileName] = eastl::shared_ptr<UISpriteBank>(new UISpriteBank(this));
	return mBanks[fileName];
}

//! Returns the default element factory which can create all built in elements
eastl::shared_ptr<UIElementFactory> BaseUI::GetDefaultUIElementFactory()
{
	// gui factory
	if (!GetUIElementFactory(0))
	{
		eastl::shared_ptr<UIElementFactory> factory(new DefaultUIElementFactory(this));
		RegisterUIElementFactory(factory);
	}
	return GetUIElementFactory(0);
}


//! Adds an element factory to the gui environment.
// Use this to extend the gui environment with new element types which it should be
// able to create automaticly, for example when loading data from xml files.
void BaseUI::RegisterUIElementFactory(const eastl::shared_ptr<UIElementFactory>& factoryToAdd)
{
	if (factoryToAdd)
		UIElementFactoryList.push_back(factoryToAdd);
}


//! Returns amount of registered scene node factories.
unsigned int BaseUI::GetRegisteredUIElementFactoryCount() const
{
	return UIElementFactoryList.size();
}


//! Returns a scene node factory by index
eastl::shared_ptr<UIElementFactory> BaseUI::GetUIElementFactory(unsigned int index) const
{
	if (index < UIElementFactoryList.size())
		return UIElementFactoryList[index];
	else
		return nullptr;
}


//! Returns the root gui element.
const eastl::shared_ptr<BaseUIElement>& BaseUI::GetRootUIElement()
{
	return mRoot;
}


//! Returns the next element in the tab group starting at the Focused element
eastl::shared_ptr<BaseUIElement> BaseUI::GetNextElement(bool reverse, bool group)
{
	// start the search at the root of the current tab group
	eastl::shared_ptr<BaseUIElement> startPos = mFocus ? mFocus->GetTabGroup() : 0;
	int startOrder = -1;

	// if we're searching for a group
	if (group && startPos)
	{
		startOrder = startPos->GetTabOrder();
	}
	else
	if (!group && mFocus && !mFocus->IsTabGroup())
	{
		startOrder = mFocus->GetTabOrder();
		if (startOrder == -1)
		{
			// this element is not part of the tab cycle,
			// but its parent might be...
			eastl::shared_ptr<BaseUIElement> el = mFocus;
			while (el && el->GetParent() && startOrder == -1)
			{
				el = el->GetParent();
				startOrder = el->GetTabOrder();
			}

		}
	}

	if (group || !startPos)
		startPos = mRoot; // start at the root

	// find the element
	eastl::shared_ptr<BaseUIElement> closest = 0;
	eastl::shared_ptr<BaseUIElement> first = 0;
	startPos->GetNextElement(startOrder, reverse, group, first, closest);

	if (closest)
		return closest; // we found an element
	else if (first)
		return first; // go to the end or the start
	else if (group)
		return mRoot; // no group found? root group
	else
		return nullptr;
}

//! adds a UI Element using its name
eastl::shared_ptr<BaseUIElement> BaseUI::AddUIElement(UIElementType elementType,
	const eastl::shared_ptr<BaseUIElement>& parent)
{
	eastl::shared_ptr<BaseUIElement> node=0;

	for (int i=UIElementFactoryList.size()-1; i>=0 && !node; --i)
		node = UIElementFactoryList[i]->AddUIElement(elementType, parent ? parent : mRoot);

	return node;
}

//! adds a button. The returned pointer must not be dropped.
eastl::shared_ptr<BaseUIButton> BaseUI::AddButton(const RectangleShape<2, int>& rectangle,
	const eastl::shared_ptr<BaseUIElement>& parent, int id, const wchar_t* text, const wchar_t *tooltiptext)
{
	eastl::shared_ptr<BaseUIButton> button(new UIButton(this, id, rectangle));
	button->SetParent(parent ? parent : mRoot);
	button->OnInit();
	if (text)
		button->SetText(text);

	if (tooltiptext)
		button->SetToolTipText(tooltiptext);

	return button;
}


//! adds a static text. The returned pointer must not be dropped.
eastl::shared_ptr<BaseUIStaticText> BaseUI::AddStaticText(const wchar_t* text, const RectangleShape<2, int>& rectangle,
	bool border, bool wordWrap, const eastl::shared_ptr<BaseUIElement>& parent, int id, bool background)
{
	eastl::shared_ptr<BaseUIStaticText> staticText(new UIStaticText(this, id, text, border, rectangle, background));
	staticText->SetParent(parent ? parent : mRoot);
	staticText->SetWordWrap(wordWrap);
	return staticText;
}


//! adds a window. The returned pointer must not be dropped.
eastl::shared_ptr<BaseUIWindow> BaseUI::AddWindow(const RectangleShape<2, int>& rectangle, bool modal,
	const wchar_t* text, const eastl::shared_ptr<BaseUIElement>& parent, int id)
{
	eastl::shared_ptr<BaseUIWindow> win(new UIWindow(this, id, rectangle));
	win->SetParent(parent ? parent : mRoot);
	win->OnInit();
	if (text)
		win->SetText(text);

	if (modal)
	{
		// Careful, don't just set the modal as parent above. That will mess up the focus 
		// (and is hard to change because we have to be very careful not to get virtual function 
		//	call, like OnEvent, in the window.
		/*
		eastl::shared_ptr<BaseUIModalScreen> modalScreen(new UIModalScreen(this, parent ? parent : Root, -1));
		modalScreen->SetParent(parent ? parent : mRoot);
		modalScreen->AddChild(win);
		*/
	}

	return win;
}


//! adds a scrollbar. The returned pointer must not be dropped.
eastl::shared_ptr<BaseUIScrollBar> BaseUI::AddScrollBar(bool horizontal, const RectangleShape<2, int>& rectangle, 
	const eastl::shared_ptr<BaseUIElement>& parent, int id)
{
	eastl::shared_ptr<BaseUIScrollBar> bar(new UIScrollBar(this, id, rectangle, horizontal));
	bar->SetParent(parent ? parent : mRoot);
	bar->OnInit();
	return bar;
}


//! Adds an image element.
eastl::shared_ptr<BaseUIImage> BaseUI::AddImage(eastl::shared_ptr<Texture2> image, Vector2<int> pos,
	bool useAlphaChannel, const eastl::shared_ptr<BaseUIElement>& parent , int id, const wchar_t* text)
{
	Vector2<int> size;
	if (image)
	{
		size[0] = image->GetDimension(0);
		size[1] = image->GetDimension(1);
	}

	RectangleShape<2, int> rectangle;
	rectangle.mCenter[0] = pos[0] + (size[0] / 2);
	rectangle.mCenter[1] = pos[1] + (size[1] / 2);
	rectangle.mExtent[0] = size[0];
	rectangle.mExtent[1] = size[1];
	eastl::shared_ptr<BaseUIImage> img(new UIImage(this, id, rectangle));
	img->SetParent(parent ? parent : mRoot);

	if (text)
		img->SetText(text);

	if (useAlphaChannel)
		img->SetUseAlphaChannel(true);

	if (image)
		img->SetImage(image);

	return img;
}


//! adds an image. The returned pointer must not be dropped.
eastl::shared_ptr<BaseUIImage> BaseUI::AddImage(const RectangleShape<2, int>& rectangle, 
	const eastl::shared_ptr<BaseUIElement>& parent, int id, const wchar_t* text, bool useAlphaChannel)
{
	eastl::shared_ptr<BaseUIImage> img(new UIImage(this, id, rectangle));
	img->SetParent(parent ? parent : mRoot);

	if (text)
		img->SetText(text);

	if (useAlphaChannel)
		img->SetUseAlphaChannel(true);

	return img;
}


//! adds a checkbox
eastl::shared_ptr<BaseUICheckBox> BaseUI::AddCheckBox(bool checked, const RectangleShape<2, int>& rectangle, 
	const eastl::shared_ptr<BaseUIElement>& parent, int id, const wchar_t* text)
{
	eastl::shared_ptr<BaseUICheckBox> check(new UICheckBox(this, id, rectangle, checked));
	check->SetParent(parent ? parent : mRoot);
	check->OnInit();

	if (text)
		check->SetText(text);

	return check;
}


//! adds a list box
eastl::shared_ptr<BaseUIListBox> BaseUI::AddListBox(const RectangleShape<2, int>& rectangle,
	const eastl::shared_ptr<BaseUIElement>& parent, int id, bool drawBackground)
{
	eastl::shared_ptr<BaseUIListBox> listBox(new UIListBox(this, id, rectangle, true, drawBackground, false));
	listBox->SetParent(parent ? parent : mRoot);
	listBox->OnInit();

	if (mCurrentSkin && mCurrentSkin->GetSpriteBank())
	{
		listBox->SetSpriteBank(mCurrentSkin->GetSpriteBank());
	}
	else if (GetBuiltInFont() && GetBuiltInFont()->GetType() == FT_BITMAP)
	{
		listBox->SetSpriteBank(((BaseUIFontBitmap*)GetBuiltInFont().get())->GetSpriteBank());
	}

	return listBox;
}

//! adds a tree view
eastl::shared_ptr<BaseUITreeView> BaseUI::AddTreeView(const RectangleShape<2, int>& rectangle,
	const eastl::shared_ptr<BaseUIElement>& parent, int id, bool drawBackground, bool scrollBarVertical, bool scrollBarHorizontal)
{
	eastl::shared_ptr<BaseUITreeView> treeView(new UITreeView(this, id, rectangle, true, drawBackground));
	treeView->SetParent(parent ? parent : mRoot);
	treeView->OnInit(scrollBarVertical, scrollBarHorizontal);

	treeView->SetIconFont(GetBuiltInFont());
	return treeView;
}

//! Adds an edit box. The returned pointer must not be dropped.
eastl::shared_ptr<BaseUIEditBox> BaseUI::AddEditBox(const wchar_t* text,
	const RectangleShape<2, int>& rectangle, bool border, 
	const eastl::shared_ptr<BaseUIElement>& parent, int id)
{
	eastl::shared_ptr<BaseUIEditBox> editBox(new UIEditBox(text, border, this, id, rectangle));
	editBox->SetParent(parent ? parent : mRoot);
	editBox->OnInit();

	return editBox;
}

//! Adds a combo box to the environment.
eastl::shared_ptr<BaseUIComboBox> BaseUI::AddComboBox(const RectangleShape<2, int>& rectangle,
	const eastl::shared_ptr<BaseUIElement>& parent, int id)
{
	eastl::shared_ptr<BaseUIComboBox> comboBox(new UIComboBox(this, id, rectangle));
	comboBox->SetParent(parent ? parent : mRoot);
	comboBox->OnInit();

	return comboBox;
}