// Copyright (C) 2002-2012 Nikolaus Gebhardt
// This file is part of the "Irrlicht Engine".
// For conditions of distribution and use, see copyright notice in irrlicht.h

#ifndef UIELEMENT_H
#define UIELEMENT_H

#include "GameEngineStd.h"

#include "Application/System/EventSystem.h"

#include "Mathematic/Algebra/Vector2.h"
#include "Mathematic/Geometric/Rectangle.h"

class BaseUI;
class UISkin;
class UIFont;
class UIElementFactory;

enum EUI_ALIGNMENT
{
	//! Aligned to parent's top or left side (default)
	EUIA_UPPERLEFT = 0,
	//! Aligned to parent's bottom or right side
	EUIA_LOWERRIGHT,
	//! Aligned to the center of parent
	EUIA_CENTER,
	//! Stretched to fit parent
	EUIA_SCALE
};

//! List of all basic Irrlicht GUI elements.
/** An IGUIElement returns this when calling IGUIElement::GetType(); */
enum EUI_ELEMENT_TYPE
{
	//! A button (IGUIButton)
	EUIET_BUTTON = 0,

	//! A check box (IGUICheckBox)
	EUIET_CHECK_BOX,

	//! A combo box (IGUIComboBox)
	EUIET_COMBO_BOX,

	//! A context menu (IGUIContextMenu)
	EUIET_CONTEXT_MENU,

	//! A menu (IGUIMenu)
	EUIET_MENU,

	//! An edit box (IGUIEditBox)
	EUIET_EDIT_BOX,

	//! A file open dialog (IGUIFileOpenDialog)
	EUIET_FILE_OPEN_DIALOG,

	//! A color select open dialog (IGUIColorSelectDialog)
	EUIET_COLOR_SELECT_DIALOG,

	//! A in/out fader (IGUIInOutFader)
	EUIET_IN_OUT_FADER,

	//! An image (IGUIImage)
	EUIET_IMAGE,

	//! A list box (IGUIListBox)
	EUIET_LIST_BOX,

	//! A mesh viewer (IGUIMeshViewer)
	EUIET_MESH_VIEWER,

	//! A message box (IGUIWindow)
	EUIET_MESSAGE_BOX,

	//! A modal screen
	EUIET_MODAL_SCREEN,

	//! A scroll bar (IGUIScrollBar)
	EUIET_SCROLL_BAR,

	//! A spin box (IGUISpinBox)
	EUIET_SPIN_BOX,

	//! A static text (IGUIStaticText)
	EUIET_STATIC_TEXT,

	//! A tab (IGUITab)
	EUIET_TAB,

	//! A tab control
	EUIET_TAB_CONTROL,

	//! A Table
	EUIET_TABLE,

	//! A tool bar (IGUIToolBar)
	EUIET_TOOL_BAR,

	//! A Tree View
	EUIET_TREE_VIEW,

	//! A window
	EUIET_WINDOW,

	//! Unknown type.
	EUIET_ELEMENT,

	//! The root of the GUI
	EUIET_ROOT,

	//! Not an element, amount of elements in there
	EUIET_COUNT,

	//! This enum is never used, it only forces the compiler to compile this enumeration to 32 bit.
	EUIET_FORCE_32_BIT = 0x7fffffff

};

//! Base class of all UI elements.
class UIElement : public eastl::enable_shared_from_this<UIElement>
{
friend class BaseUI;

public:

	//! Constructor
	UIElement(EUI_ELEMENT_TYPE type, int id, const RectangleBase<2, int>& rectangle)
	:	Parent(0), RelativeRect(rectangle), AbsoluteRect(rectangle), AbsoluteClippingRect(rectangle), 
		DesiredRect(rectangle), MaxSize{ 0,0 }, MinSize{ 1, 1 }, Visible(true), Enabled(true), SubElement(false),
		NoClip(false), ID(id), TabStop(false), TabOrder(-1), TabGroup(false), AlignLeft(EUIA_UPPERLEFT), 
		AlignRight(EUIA_UPPERLEFT), AlignTop(EUIA_UPPERLEFT), AlignBottom(EUIA_UPPERLEFT), Type(type)
	{
		Children.clear();
	}

	//! Destructor
	virtual ~UIElement()
	{
		// delete all children
		eastl::list<eastl::shared_ptr<UIElement>>::iterator it = Children.begin();
		for (; it != Children.end(); ++it)
		{
			(*it)->Parent = 0;
		}
		Children.clear();
	}

	//! set the parent of the element
	void SetParent(const eastl::shared_ptr<UIElement>& parent)
	{
		if (parent)
		{
			// if we were given a parent to attach to
			parent->AddChildToEnd(shared_from_this());
			RecalculateAbsolutePosition(true);
		}
	}

	//! Returns parent of this element.
	const eastl::shared_ptr<UIElement>& GetParent() const
	{
		return Parent;
	}


	//! Returns the relative rectangle of this element.
	RectangleBase<2, int> GetRelativePosition() const
	{
		return RelativeRect;
	}


	//! Sets the relative rectangle of this element.
	/** \param r The absolute position to set */
	void SetRelativePosition(const RectangleBase<2, int>& r)
	{
		if (Parent)
		{
			const RectangleBase<2, int>& rectangle = Parent->GetAbsolutePosition();

			if (AlignLeft == EUIA_SCALE)
			{
				ScaleRect.center[0] = (float)r.center[0] / (float)rectangle.extent[0];
				ScaleRect.extent[0] = (float)rectangle.extent[0];
			}
			if (AlignRight == EUIA_SCALE)
			{
				ScaleRect.center[0] = (float)r.center[0] / (float)rectangle.extent[0];
				ScaleRect.extent[0] = (float)rectangle.extent[0];
			}
			if (AlignTop == EUIA_SCALE)
			{
				ScaleRect.center[1] = (float)r.center[1] / (float)rectangle.extent[1];
				ScaleRect.extent[1] = (float)rectangle.extent[1];
			}
			if (AlignBottom == EUIA_SCALE)
			{
				ScaleRect.center[1] = (float)r.center[1] / (float)rectangle.extent[1];
				ScaleRect.extent[1] = (float)rectangle.extent[1];
			}
		}

		DesiredRect = r;
		UpdateAbsoluteTransformation();
	}

	//! Sets the relative rectangle of this element, maintaining its current width and height
	/** \param position The new relative position to set. Width and height will not be changed. */
	void SetRelativePosition(const Vector2<int>& position)
	{
		Vector2<int> center(RelativeRect.center);
		center[0] += position[0];
		center[1] += position[1];
		const RectangleBase<2, int> rectangle(
			center, RelativeRect.axis, RelativeRect.extent);
		SetRelativePosition(rectangle);
	}


	//! Sets the relative rectangle of this element as a proportion of its parent's area.
	/** \note This method used to be 'void SetRelativePosition(const Rectangle<2, float>& r)'
	\param r  The rectangle to set, interpreted as a proportion of the parent's area.
	Meaningful values are in the range [0...1], unless you intend this element to spill
	outside its parent. */
	void SetRelativePositionProportional(const RectangleBase<2, float>& r)
	{
		if (!Parent)
			return;

		const RectangleBase<2, int>& rectangle = Parent->GetAbsolutePosition();
		Vector2<int> center{ 
			(int)floor(r.center[0] * rectangle.extent[0]), 
			(int)floor(r.center[1] * rectangle.extent[1]) };
		eastl::array<Vector2<int>, 2U> axis;
		for (int i = 0; i < r.axis.count; i++)
			axis[i] = Vector2<int>{ (int)r.axis[i][0], (int)r.axis[i][1] };
		Vector2<int> extent{ (int)r.extent[0], (int)r.extent[1] };

		DesiredRect = RectangleBase<2, int>(center, axis, extent);
		ScaleRect = r;

		UpdateAbsoluteTransformation();
	}


	//! Gets the absolute rectangle of this element
	RectangleBase<2, int> GetAbsolutePosition() const
	{
		return AbsoluteRect;
	}


	//! Returns the visible area of the element.
	RectangleBase<2, int> GetAbsoluteClippingRect() const
	{
		return AbsoluteClippingRect;
	}


	//! Sets whether the element will ignore its parent's clipping rectangle
	/** \param noClip If true, the element will not be clipped by its parent's clipping rectangle. */
	void SetNotClipped(bool noClip)
	{
		NoClip = noClip;
		UpdateAbsoluteTransformation();
	}


	//! Gets whether the element will ignore its parent's clipping rectangle
	/** \return true if the element is not clipped by its parent's clipping rectangle. */
	bool IsNotClipped() const
	{
		return NoClip;
	}


	//! Sets the maximum size allowed for this element
	/** If set to 0,0, there is no maximum size */
	void SetMaxSize(Vector2<unsigned int> size)
	{
		MaxSize = size;
		UpdateAbsoluteTransformation();
	}


	//! Sets the minimum size allowed for this element
	void SetMinSize(Vector2<unsigned int> size)
	{
		MinSize = size;
		if (MinSize[0] < 1)
			MinSize[0] = 1;
		if (MinSize[1] < 1)
			MinSize[1] = 1;
		UpdateAbsoluteTransformation();
	}


	//! The alignment defines how the borders of this element will be positioned when the parent element is resized.
	void SetAlignment(EUI_ALIGNMENT left, EUI_ALIGNMENT right, EUI_ALIGNMENT top, EUI_ALIGNMENT bottom)
	{
		AlignLeft = left;
		AlignRight = right;
		AlignTop = top;
		AlignBottom = bottom;

		if (Parent)
		{
			const RectangleBase<2, int>& rectangle = Parent->GetAbsolutePosition();

			if (AlignLeft == EUIA_SCALE)
			{
				ScaleRect.center[0] = (float)DesiredRect.center[0] / (float)rectangle.extent[0];
				ScaleRect.extent[0] = (float)rectangle.extent[0];
			}
			if (AlignRight == EUIA_SCALE)
			{
				ScaleRect.center[0] = (float)DesiredRect.center[0] / (float)rectangle.extent[0];
				ScaleRect.extent[0] = (float)rectangle.extent[0];
			}
			if (AlignTop == EUIA_SCALE)
			{
				ScaleRect.center[1] = (float)DesiredRect.center[1] / (float)rectangle.extent[1];
				ScaleRect.extent[1] = (float)rectangle.extent[1];
			}
			if (AlignBottom == EUIA_SCALE)
			{
				ScaleRect.center[1] = (float)DesiredRect.center[1] / (float)rectangle.extent[1];
				ScaleRect.extent[1] = (float)rectangle.extent[1];
			}
		}
	}


	//! Updates the absolute position.
	virtual void UpdateAbsoluteTransformation()
	{
		RecalculateAbsolutePosition(false);

		// update all children
		eastl::list<eastl::shared_ptr<UIElement>>::iterator it = Children.begin();
		for (; it != Children.end(); ++it)
		{
			(*it)->UpdateAbsoluteTransformation();
		}
	}


	//! Returns the topmost UI element at the specific position.
	/**
	This will check this UI element and all of its descendants, so it
	may return this UI element.  To check all UI elements, call this
	function on device->getUIEnvironment()->GetRootUIElement(). Note
	that the root element is the size of the screen, so doing so (with
	an on-screen point) will always return the root element if no other
	element is above it at that point.
	\param point: The point at which to find a UI element.
	\return The topmost UI element at that point, or 0 if there are
	no candidate elements at this point.
	*/
	eastl::shared_ptr<UIElement> GetElementFromPoint(const Vector2<int>& point)
	{
		eastl::shared_ptr<UIElement> target = 0;

		// we have to search from back to front, because later children
		// might be drawn over the top of earlier ones.

		eastl::list<eastl::shared_ptr<UIElement>>::reverse_iterator it = Children.rbegin();
		if (IsVisible())
		{
			while(it != Children.rend())
			{
				target = (*it)->GetElementFromPoint(point);
				if (target)
					return target;

				++it;
			}
		}

		if (IsVisible() && IsPointInside(point))
			target = shared_from_this();

		return target;
	}


	//! Returns true if a point is within this element.
	/** Elements with a shape other than a rectangle should override this method */
	virtual bool IsPointInside(const Vector2<int>& point) const
	{
		eastl::array<Vector2<int>, 4> vertex;
		AbsoluteClippingRect.GetVertices(vertex);
		return (
			vertex[0][0] >= point[0] && vertex[0][1] >= point[1] &&
			vertex[3][0] <= point[0] && vertex[3][1] <= point[1] );
	}


	//! Adds a UI element as new child of this element.
	virtual void AddChild(const eastl::shared_ptr<UIElement>& child)
	{
		AddChildToEnd(child);
		if (child)
			child->UpdateAbsoluteTransformation();
	}

	//! Removes a child.
	virtual void RemoveChild(const eastl::shared_ptr<UIElement>& child)
	{
		eastl::list<eastl::shared_ptr<UIElement>>::iterator it = Children.begin();
		for (; it != Children.end(); ++it)
		{
			if ((*it) == child)
			{
				(*it)->Parent = 0;
				Children.erase(it);
				return;
			}
		}
	}


	//! Removes this element from its parent.
	virtual void Remove()
	{
		if (Parent)
			Parent->RemoveChild(shared_from_this());
	}


	//! draws the element and its children.
	virtual void Draw( )
	{
		if ( IsVisible() )
		{
			eastl::list<eastl::shared_ptr<UIElement>>::iterator it = Children.begin();
			for (; it != Children.end(); ++it)
				(*it)->Draw();
		}
	}


	//! animate the element and its children.
	virtual void OnPostRender(unsigned int timeMs)
	{
		if ( IsVisible() )
		{
			eastl::list<eastl::shared_ptr<UIElement>>::iterator it = Children.begin();
			for (; it != Children.end(); ++it)
				(*it)->OnPostRender( timeMs );
		}
	}


	//! Moves this element.
	virtual void Move(Vector2<int> absoluteMovement)
	{
		Vector2<int> center(DesiredRect.center);
		center[0] += absoluteMovement[0];
		center[1] += absoluteMovement[1];
		const RectangleBase<2, int> rectangle(
			center, DesiredRect.axis, DesiredRect.extent);

		SetRelativePosition(rectangle);
	}


	//! Returns true if element is visible.
	virtual bool IsVisible() const
	{
		return Visible;
	}

	//! Check whether the element is truly visible, taking into accounts its parents' visibility
	/** \return true if the element and all its parents are visible,
	false if this or any parent element is invisible. */
	virtual bool IsTrulyVisible() const
	{
		if(!Visible)
			return false;

		if(!Parent)
			return true;

		return Parent->IsTrulyVisible();
	}

	//! Sets the visible state of this element.
	virtual void SetVisible(bool visible)
	{
		Visible = visible;
	}


	//! Returns true if this element was created as part of its parent control
	virtual bool IsSubElement() const
	{
		return SubElement;
	}


	//! Sets whether this control was created as part of its parent.
	/** For example, it is true when a scrollbar is part of a UIListBox.
	SubElements are not saved to disk when calling guiEnvironment->saveUI() */
	virtual void SetSubElement(bool subElement)
	{
		SubElement = subElement;
	}


	//! If set to true, the focus will visit this element when using the tab key to cycle through elements.
	/** If this element is a tab group (see IsTabGroup/SetTabGroup) then
	ctrl+tab will be used instead. */
	void SetTabStop(bool enable)
	{
		TabStop = enable;
	}


	//! Returns true if this element can be Focused by navigating with the tab key
	bool IsTabStop() const
	{
		return TabStop;
	}


	//! Sets the priority of focus when using the tab key to navigate between a group of elements.
	/** See SetTabGroup, IsTabGroup and GetTabGroup for information on tab groups.
	Elements with a lower number are Focused first */
	void SetTabOrder(int index)
	{
		// negative = autonumber
		if (index < 0)
		{
			TabOrder = 0;
			eastl::shared_ptr<UIElement> el = GetTabGroup();
			while (TabGroup && el && el->Parent)
				el = el->Parent;

			const eastl::shared_ptr<UIElement>& first=0, closest=0;
			if (el)
			{
				// find the highest element number
				el->GetNextElement(-1, true, TabGroup, first, closest, true);
				if (first)
					TabOrder = first->GetTabOrder() + 1;
			}
		}
		else  TabOrder = index;
	}


	//! Returns the number in the tab order sequence
	int GetTabOrder() const
	{
		return TabOrder;
	}


	//! Sets whether this element is a container for a group of elements which can be navigated using the tab key.
	/** For example, windows are tab groups.
	Groups can be navigated using ctrl+tab, providing IsTabStop is true. */
	void SetTabGroup(bool isGroup)
	{
		TabGroup = isGroup;
	}


	//! Returns true if this element is a tab group.
	bool IsTabGroup() const
	{
		return TabGroup;
	}


	//! Returns the container element which holds all elements in this element's tab group.
	eastl::shared_ptr<UIElement> GetTabGroup()
	{
		eastl::shared_ptr<UIElement> ret(shared_from_this());
		while (ret && !ret->IsTabGroup())
			ret = ret->GetParent();

		return ret;
	}


	//! Returns true if element is enabled
	/** Currently elements do _not_ care about parent-states.
		So if you want to affect childs you have to enable/disable them all.
		The only exception to this are sub-elements which also check their parent.
	*/
	virtual bool IsEnabled() const
	{
		if ( IsSubElement() && Enabled && GetParent() )
			return GetParent()->IsEnabled();

		return Enabled;
	}


	//! Sets the enabled state of this element.
	virtual void SetEnabled(bool enabled)
	{
		Enabled = enabled;
	}


	//! Sets the new caption of this element.
	virtual void SetText(const wchar_t* text)
	{
		Text = text;
	}


	//! Returns caption of this element.
	virtual const wchar_t* GetText() const
	{
		return Text.c_str();
	}


	//! Sets the new caption of this element.
	virtual void SetToolTipText(const wchar_t* text)
	{
		ToolTipText = text;
	}


	//! Returns caption of this element.
	virtual const eastl::wstring& GetToolTipText() const
	{
		return ToolTipText;
	}


	//! Returns id. Can be used to identify the element.
	virtual int GetID() const
	{
		return ID;
	}


	//! Sets the id of this element
	virtual void SetID(int id)
	{
		ID = id;
	}

	//! Called if an event happened.
	virtual bool OnEvent(const Event& event)
	{
		return Parent ? Parent->OnEvent(event) : false;
	}


	//! Brings a child to front
	/** \return True if successful, false if not. */
	virtual bool BringToFront(const eastl::shared_ptr<UIElement>& element)
	{
		eastl::list<eastl::shared_ptr<UIElement>>::iterator it = Children.begin();
		for (; it != Children.end(); ++it)
		{
			if (element == (*it))
			{
				Children.erase(it);
				Children.push_back(element);
				return true;
			}
		}

		return false;
	}


	//! Moves a child to the back, so it's siblings are drawn on top of it
	/** \return True if successful, false if not. */
	virtual bool SendToBack(const eastl::shared_ptr<UIElement>& element)
	{
		eastl::list<eastl::shared_ptr<UIElement>>::iterator it = Children.begin();
		if (element == (*it)) // already there
			return true;

		for (; it != Children.end(); ++it)
		{
			if (element == (*it))
			{
				Children.erase(it);
				Children.push_front(element);
				return true;
			}
		}

		return false;
	}

	//! Returns list with children of this element
	virtual const eastl::list<eastl::shared_ptr<UIElement>>& GetChildren() const
	{
		return Children;
	}


	//! Finds the first element with the given id.
	/** \param id: Id to search for.
	\param searchchildren: Set this to true, if also children of this
	element may contain the element with the searched id and they
	should be searched too.
	\return Returns the first element with the given id. If no element
	with this id was found, 0 is returned. */
	virtual eastl::shared_ptr<UIElement> GetElementFromId(int id, bool searchchildren=false) const
	{
		eastl::shared_ptr<UIElement> e = 0;

		eastl::list<eastl::shared_ptr<UIElement>>::const_iterator it = Children.begin();
		for (; it != Children.end(); ++it)
		{
			if ((*it)->GetID() == id)
				return (*it);

			if (searchchildren)
				e = (*it)->GetElementFromId(id, true);

			if (e)
				return e;
		}

		return e;
	}


	//! returns true if the given element is a child of this one.
	//! \param child: The child element to check
	bool IsMyChild(UIElement* child) const
	{
		if (!child)
			return false;
		do
		{
			if (child->Parent)
				child = child->Parent.get();

		} while (child->Parent && child != this);

		return child == this;
	}


	//! searches elements to find the closest next element to tab to
	/** \param startOrder: The TabOrder of the current element, -1 if none
	\param reverse: true if searching for a lower number
	\param group: true if searching for a higher one
	\param first: element with the highest/lowest known tab order depending on search direction
	\param closest: the closest match, depending on tab order and direction
	\param includeInvisible: includes invisible elements in the search (default=false)
	\return true if successfully found an element, false to continue searching/fail */
	bool GetNextElement(int startOrder, bool reverse, bool group,
		eastl::shared_ptr<UIElement> first, eastl::shared_ptr<UIElement> closest, 
		bool includeInvisible=false) const
	{
		// we'll stop searching if we find this number
		int wanted = startOrder + ( reverse ? -1 : 1 );
		if (wanted==-2)
			wanted = 1073741824; // maximum int

		eastl::list<eastl::shared_ptr<UIElement>>::const_iterator it = Children.begin();

		int closestOrder, currentOrder;
		while(it != Children.end())
		{
			// ignore invisible elements and their children
			if ( ( (*it)->IsVisible() || includeInvisible ) &&
				(group == true || (*it)->IsTabGroup() == false) )
			{
				// only check tab stops and those with the same group status
				if ((*it)->IsTabStop() && ((*it)->IsTabGroup() == group))
				{
					currentOrder = (*it)->GetTabOrder();

					// is this what we're looking for?
					if (currentOrder == wanted)
					{
						closest = (*it);
						return true;
					}

					// is it closer than the current closest?
					if (closest)
					{
						closestOrder = closest->GetTabOrder();
						if ( ( reverse && currentOrder > closestOrder && currentOrder < startOrder)
							||(!reverse && currentOrder < closestOrder && currentOrder > startOrder))
						{
							closest = (*it);
						}
					}
					else
					if ( (reverse && currentOrder < startOrder) || (!reverse && currentOrder > startOrder) )
					{
						closest = (*it);
					}

					// is it before the current first?
					if (first)
					{
						closestOrder = first->GetTabOrder();

						if ( (reverse && closestOrder < currentOrder) || (!reverse && closestOrder > currentOrder) )
						{
							first = (*it);
						}
					}
					else
					{
						first = (*it);
					}
				}
				// search within children
				if ((*it)->GetNextElement(startOrder, reverse, group, first, closest))
				{
					return true;
				}
			}
			++it;
		}
		return false;
	}


	//! Returns the type of the gui element.
	/** This is needed for the .NET wrapper but will be used
	later for serializing and deserializing.
	If you wrote your own UIElements, you need to set the type for your element as first parameter
	in the constructor of UIElement. For own (=unknown) elements, simply use EUIET_ELEMENT as type */
	EUI_ELEMENT_TYPE GetType() const
	{
		return Type;
	}

	//! Returns true if the gui element supports the given type.
	/** This is mostly used to check if you can cast a gui element to the class that goes with the type.
	Most gui elements will only support their own type, but if you derive your own classes from interfaces
	you can overload this function and add a check for the type of the base-class additionally.
	This allows for checks comparable to the dynamic_cast of c++ with enabled rtti.
	Note that you can't do that by calling BaseClass::HasType(type), but you have to do an explicit
	comparison check, because otherwise the base class usually just checks for the membervariable
	Type which contains the type of your derived class.
	*/
	virtual bool HasType(EUI_ELEMENT_TYPE type) const
	{
		return type == Type;
	}

	//! Returns the name of the element.
	/** \return Name as character string. */
	virtual const char* GetName() const
	{
		return Name.c_str();
	}


	//! Sets the name of the element.
	/** \param name New name of the gui element. */
	virtual void SetName(const char* name)
	{
		Name = name;
	}


	//! Sets the name of the element.
	/** \param name New name of the gui element. */
	virtual void SetName(const eastl::string& name)
	{
		Name = name;
	}

protected:

	// not virtual because needed in constructor
	void AddChildToEnd(const eastl::shared_ptr<UIElement>& child)
	{
		if (child)
		{
			child->Remove(); // Remove from old parent
			child->LastParentRect = GetAbsolutePosition();
			child->Parent = shared_from_this();
			Children.push_back(child);
		}
	}

	// not virtual because needed in constructor
	void RecalculateAbsolutePosition(bool recursive)
	{
		RectangleBase<2, int> parentAbsolute;
		RectangleBase<2, int> parentAbsoluteClip;
		float fw=0.f, fh=0.f;

		if (Parent)
		{
			parentAbsolute = Parent->AbsoluteRect;

			if (NoClip)
			{
				eastl::shared_ptr<UIElement> p(shared_from_this());
				while (p->Parent)
					p = p->Parent;
				parentAbsoluteClip = p->AbsoluteClippingRect;
			}
			else
				parentAbsoluteClip = Parent->AbsoluteClippingRect;
		}

		const int diffx = parentAbsolute.extent[0] - LastParentRect.extent[0];
		const int diffy = parentAbsolute.extent[1] - LastParentRect.extent[1];

		if (AlignLeft == EUIA_SCALE || AlignRight == EUIA_SCALE)
			fw = (float)parentAbsolute.extent[0];

		if (AlignTop == EUIA_SCALE || AlignBottom == EUIA_SCALE)
			fh = (float)parentAbsolute.extent[1];

		switch (AlignLeft)
		{
			case EUIA_UPPERLEFT:
				break;
			case EUIA_LOWERRIGHT:
				DesiredRect.center[0] += diffx;
				break;
			case EUIA_CENTER:
				DesiredRect.center[0] += diffx/2;
				break;
			case EUIA_SCALE:
				DesiredRect.center[0] = (int)round(ScaleRect.center[0] * fw);
				break;
		}

		switch (AlignRight)
		{
			case EUIA_UPPERLEFT:
				break;
			case EUIA_LOWERRIGHT:
				DesiredRect.center[0] += diffx;
				break;
			case EUIA_CENTER:
				DesiredRect.center[0] += diffx / 2;
				break;
			case EUIA_SCALE:
				DesiredRect.center[0] = (int)round(ScaleRect.center[0] * fw);
				break;
		}

		switch (AlignTop)
		{
			case EUIA_UPPERLEFT:
				break;
			case EUIA_LOWERRIGHT:
				DesiredRect.center[1] += diffy;
				break;
			case EUIA_CENTER:
				DesiredRect.center[1] += diffy / 2;
				break;
			case EUIA_SCALE:
				DesiredRect.center[1] = (int)round(ScaleRect.center[1] * fh);
				break;
		}

		switch (AlignBottom)
		{
			case EUIA_UPPERLEFT:
				break;
			case EUIA_LOWERRIGHT:
				DesiredRect.center[1] += diffy;
				break;
			case EUIA_CENTER:
				DesiredRect.center[1] += diffy / 2;
				break;
			case EUIA_SCALE:
				DesiredRect.center[1] = (int)round(ScaleRect.center[1] * fh);
				break;
		}

		RelativeRect = DesiredRect;

		const int w = RelativeRect.extent[0];
		const int h = RelativeRect.extent[1];

		// make sure the desired rectangle is allowed
		if (w < (int)MinSize[0])
			RelativeRect.center[0] = RelativeRect.center[0] + MinSize[0];
		if (h < (int)MinSize[1])
			RelativeRect.center[1] = RelativeRect.center[1] + MinSize[1];
		if (MaxSize[0] && w > (int)MaxSize[0])
			RelativeRect.center[0] = RelativeRect.center[0] + MaxSize[0];
		if (MaxSize[1] && h > (int)MaxSize[1])
			RelativeRect.center[1] = RelativeRect.center[1] + MaxSize[1];

		AbsoluteRect.center = RelativeRect.center + parentAbsolute.center;

		if (!Parent)
			parentAbsoluteClip = AbsoluteRect;

		AbsoluteClippingRect = AbsoluteRect;

		LastParentRect = parentAbsolute;

		if ( recursive )
		{
			// update all children
			eastl::list<eastl::shared_ptr<UIElement>>::iterator it = Children.begin();
			for (; it != Children.end(); ++it)
			{
				(*it)->RecalculateAbsolutePosition(recursive);
			}
		}
	}

protected:

	//! List of all children of this element
	eastl::list<eastl::shared_ptr<UIElement>> Children;

	//! Pointer to the parent
	eastl::shared_ptr<UIElement> Parent;

	//! relative rect of element
	RectangleBase<2, int> RelativeRect;

	//! absolute rect of element
	RectangleBase<2, int> AbsoluteRect;

	//! absolute clipping rect of element
	RectangleBase<2, int> AbsoluteClippingRect;

	//! the rectangle the element would prefer to be,
	//! if it was not constrained by parent or max/min size
	RectangleBase<2, int> DesiredRect;

	//! for calculating the difference when resizing parent
	RectangleBase<2, int> LastParentRect;

	//! relative scale of the element inside its parent
	RectangleBase<2, float> ScaleRect;

	//! maximum and minimum size of the element
	Vector2<unsigned int> MaxSize, MinSize;

	//! is visible?
	bool Visible;

	//! is enabled?
	bool Enabled;

	//! is a part of a larger whole and should not be serialized?
	bool SubElement;

	//! does this element ignore its parent's clipping rectangle?
	bool NoClip;

	//! caption
	eastl::wstring Text;

	//! tooltip
	eastl::wstring ToolTipText;

	//! users can set this for identificating the element by string
	eastl::string Name;

	//! users can set this for identificating the element by integer
	int ID;

	//! tab stop like in windows
	bool TabStop;

	//! tab order
	int TabOrder;

	//! tab groups are containers like windows, use ctrl+tab to navigate
	bool TabGroup;

	//! tells the element how to act when its parent is resized
	EUI_ALIGNMENT AlignLeft, AlignRight, AlignTop, AlignBottom;

	//! type of element
	EUI_ELEMENT_TYPE Type;
};

#endif