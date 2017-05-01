// Copyright (C) 2002-2012 Nikolaus Gebhardt
// This file is part of the "Irrlicht Engine".
// For conditions of distribution and use, see copyright notice in irrlicht.h

#ifndef DEFAULTUIELEMENTFACTORY_H
#define DEFAULTUIELEMENTFACTORY_H

#include "GameEngineStd.h"

#include "UIElementFactory.h"
#include "UserInterface.h"

//! This interface makes it possible to dynamically create gui elements.
class DefaultUIElementFactory : public UIElementFactory
{
public:

	DefaultUIElementFactory( BaseUI* );

	//! Adds an element to the gui environment based on its type id.
	/** \param type: Type of the element to add.
	\param parent: Parent scene node of the new element. A value of 0 adds it to the root.
	\return Returns pointer to the new element or 0 if unsuccessful. */
	virtual eastl::shared_ptr<BaseUIElement> AddUIElement(
		UIElementType type, const eastl::shared_ptr<BaseUIElement>& parent=0);

	//! Returns the amount of GUI element types this factory is able to create.
	virtual int GetCreatableUIElementTypeCount() const;

private:

	BaseUI* UI;
};

#endif
