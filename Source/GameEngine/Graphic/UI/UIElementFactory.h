// Copyright (C) 2002-2012 Nikolaus Gebhardt
// This file is part of the "Irrlicht Engine".
// For conditions of distribution and use, see copyright notice in irrlicht.h

#ifndef UIELEMENTFACTORY_H
#define UIELEMENTFACTORY_H

#include "GameEngineStd.h"

#include "UserInterface.h"

//! Interface making it possible to dynamically create UI elements
/** To be able to add custom elements to Irrlicht and to make it possible for the
scene manager to save and load them, simply implement this interface and register it
in your gui environment via UIEnvironment::RegisterUIElementFactory.
Note: When implementing your own element factory, don't call UIEnvironment::grab() to
increase the reference counter of the environment. This is not necessary because the
it will grab() the factory anyway, and otherwise cyclic references will be created.
*/
class UIElementFactory
{
public:

	//! adds an element to the gui environment based on its type id
	/** \param type: Type of the element to add.
	\param parent: Parent scene node of the new element, can be null to add to the root.
	\return Pointer to the new element or null if not successful. */
	virtual eastl::shared_ptr<BaseUIElement> AddUIElement(
		UIElementType type, const eastl::shared_ptr<BaseUIElement>& parent=0) = 0;

	//! Get amount of UI element types this factory is able to create
	virtual int GetCreatableUIElementTypeCount() const = 0;
};


//! This interface makes it possible to dynamically create gui elements.
class DefaultUIElementFactory : public UIElementFactory
{
public:

	DefaultUIElementFactory(BaseUI*);

	//! Adds an element to the gui environment based on its type id.
	/** \param type: Type of the element to add.
	\param parent: Parent scene node of the new element. A value of 0 adds it to the root.
	\return Returns pointer to the new element or 0 if unsuccessful. */
	virtual eastl::shared_ptr<BaseUIElement> AddUIElement(
		UIElementType type, const eastl::shared_ptr<BaseUIElement>& parent = 0);

	//! Returns the amount of GUI element types this factory is able to create.
	virtual int GetCreatableUIElementTypeCount() const;

private:

	BaseUI* UI;
};

#endif