// Copyright (C) 2002-2012 Nikolaus Gebhardt
// This file is part of the "Irrlicht Engine".
// For conditions of distribution and use, see copyright notice in irrlicht.h

#ifndef UIROOT_H
#define UIROOT_H

#include "GameEngineStd.h"

#include "UIElement.h"

//! Root Element from all UIElements in the scene
class UIRoot : public BaseUIElement
{
public:

	//! constructor
	UIRoot(BaseUI* ui, UIElementType type, int id, RectangleBase<2, int> rectangle);

	//! called if an event happened.
	virtual bool OnEvent(const Event& event);

private:

	BaseUI* mUI;
};

#endif

