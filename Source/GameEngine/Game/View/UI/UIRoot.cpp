// Copyright (C) 2002-2012 Nikolaus Gebhardt
// This file is part of the "Irrlicht Engine".
// For conditions of distribution and use, see copyright notice in irrlicht.h

#include "UIRoot.h"
#include "UserInterface.h"

//! constructor
UIRoot::UIRoot(BaseUI* ui, UIElementType type, int id, RectangleShape<2, int> rectangle)
:	BaseUIElement(type, id, rectangle), mUI(ui)
{
}

//! called if an event happened.
bool UIRoot::OnEvent(const Event& evt)
{
	bool ret = false;
	if (mUI && (evt.mEventType != ET_MOUSE_INPUT_EVENT)
		&& (evt.mEventType != ET_KEY_INPUT_EVENT)
		&& (evt.mEventType != ET_UI_EVENT || evt.mUIEvent.mCaller != this))
	{
		ret = mUI->OnEvent(evt);
	}

	return ret;
}

