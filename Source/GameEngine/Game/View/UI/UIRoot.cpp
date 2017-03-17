// Copyright (C) 2002-2012 Nikolaus Gebhardt
// This file is part of the "Irrlicht Engine".
// For conditions of distribution and use, see copyright notice in irrlicht.h

#include "UIRoot.h"
#include "UserInterface.h"

//! constructor
UIRoot::UIRoot(BaseUI* ui, EUI_ELEMENT_TYPE type, int id, RectangleBase<2, int> rectangle)
:	UIElement(type, id, rectangle), UI(ui)
{
}

//! called if an event happened.
bool UIRoot::OnEvent(const Event& evt)
{
	bool ret = false;
	if (UI && (evt.mEventType != ET_MOUSE_INPUT_EVENT)
		&& (evt.mEventType != ET_KEY_INPUT_EVENT)
		&& (evt.mEventType != ET_UI_EVENT || evt.mUIEvent.mCaller != this))
	{
		ret = UI->OnEvent(evt);
	}

	return ret;
}

