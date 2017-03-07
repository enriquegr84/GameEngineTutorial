// Copyright (C) 2002-2012 Nikolaus Gebhardt
// This file is part of the "Irrlicht Engine".
// For conditions of distribution and use, see copyright notice in irrlicht.h

#include "UIRoot.h"

#include "UserInterface/UserInterface.h"

//! constructor
UIRoot::UIRoot(BaseUI* ui, EUI_ELEMENT_TYPE type, s32 id, Rect<s32> rectangle)
:	UIElement(type, id, rectangle), UserInterface(ui)
{
	#ifdef _DEBUG
	//setDebugName("UIRoot");
	#endif

}

//! called if an event happened.
bool UIRoot::OnEvent(const Event& evt)
{
	bool ret = false;
	if (UserInterface
		&& (evt.m_EventType != EET_MOUSE_INPUT_EVENT)
		&& (evt.m_EventType != EET_KEY_INPUT_EVENT)
		&& (evt.m_EventType != EET_UI_EVENT || evt.m_UIEvent.m_Caller != this))
	{
		ret = UserInterface->OnEvent(evt);
	}

	return ret;
}

