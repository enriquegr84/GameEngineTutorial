// Copyright (C) 2002-2012 Nikolaus Gebhardt
// This file is part of the "Irrlicht Engine".
// For conditions of distribution and use, see copyright notice in irrlicht.h

#include "DefaultUIElementFactory.h"

DefaultUIElementFactory::DefaultUIElementFactory(BaseUI* ui)
:	UI(ui)
{
	// don't grab the gui environment here to prevent cyclic references
}


//! adds an element to the env based on its type id
eastl::shared_ptr<UIElement> DefaultUIElementFactory::AddUIElement(
	EUI_ELEMENT_TYPE type, const eastl::shared_ptr<UIElement>& parent)
{
	return eastl::shared_ptr<UIElement>();
}

//! Returns the amount of element types this factory is able to create.
int DefaultUIElementFactory::GetCreatableUIElementTypeCount() const
{
	return EUIET_COUNT;
}