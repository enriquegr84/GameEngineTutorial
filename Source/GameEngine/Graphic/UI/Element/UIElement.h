// Copyright (C) 2002-2012 Nikolaus Gebhardt
// This file is part of the "Irrlicht Engine".
// For conditions of distribution and use, see copyright notice in irrlicht.h

#ifndef UIELEMENT_H
#define UIELEMENT_H

#include "GameEngineStd.h"

#include "Application/System/EventSystem.h"

#include "Core/IO/FileSystem.h"

#include "Mathematic/Mathematic.h"

enum UIAlignment
{
	//! Aligned to parent's top or left side (default)
	UIA_UPPERLEFT = 0,
	//! Aligned to parent's bottom or right side
	UIA_LOWERRIGHT,
	//! Aligned to the center of parent
	UIA_CENTER,
	//! Stretched to fit parent
	UIA_SCALE
};

//! List of all basic GUI elements.
/** An IGUIElement returns this when calling IGUIElement::GetType(); */
enum UIElementType
{
	//! A button (IGUIButton)
	UIET_BUTTON = 0,

	//! A check box (IGUICheckBox)
	UIET_CHECK_BOX,

	//! A combo box (IGUIComboBox)
	UIET_COMBO_BOX,

	//! A context menu (IGUIContextMenu)
	UIET_CONTEXT_MENU,

	//! A menu (IGUIMenu)
	UIET_MENU,

	//! An edit box (IGUIEditBox)
	UIET_EDIT_BOX,

	//! A file open dialog (IGUIFileOpenDialog)
	UIET_FILE_OPEN_DIALOG,

	//! A color select open dialog (IGUIColorSelectDialog)
	UIET_COLOR_SELECT_DIALOG,

	//! A in/out fader (IGUIInOutFader)
	UIET_IN_OUT_FADER,

	//! An image (IGUIImage)
	UIET_IMAGE,

	//! A list box (IGUIListBox)
	UIET_LIST_BOX,

	//! A mesh viewer (IGUIMeshViewer)
	UIET_MESH_VIEWER,

	//! A message box (IGUIWindow)
	UIET_MESSAGE_BOX,

	//! A modal screen
	UIET_MODAL_SCREEN,

	//! A scroll bar (IGUIScrollBar)
	UIET_SCROLL_BAR,

	//! A spin box (IGUISpinBox)
	UIET_SPIN_BOX,

	//! A static text (IGUIStaticText)
	UIET_STATIC_TEXT,

	//! A tab (IGUITab)
	UIET_TAB,

	//! A tab control
	UIET_TAB_CONTROL,

	//! A Table
	UIET_TABLE,

	//! A tool bar (IGUIToolBar)
	UIET_TOOL_BAR,

	//! A Tree View
	UIET_TREE_VIEW,

	//! A window
	UIET_WINDOW,

	//! Unknown type.
	UIET_ELEMENT,

	//! The root of the GUI
	UIET_ROOT,

	//! Not an element, amount of elements in there
	UIET_COUNT,

	//! This enum is never used, it only forces the compiler to compile this enumeration to 32 bit.
	UIET_FORCE_32_BIT = 0x7fffffff

};

//! Base class of all UI elements.
class BaseUIElement : public eastl::enable_shared_from_this<BaseUIElement>
{
friend class BaseUI;

public:

	//! Constructor
	BaseUIElement(UIElementType type, int id, const RectangleShape<2, int>& rectangle)
	:	mParent(0), mRelativeRect(rectangle), mAbsoluteRect(rectangle), mAbsoluteClippingRect(rectangle), 
		mDesiredRect(rectangle), mMaxSize{ 0,0 }, mMinSize{ 1, 1 }, mVisible(true), mEnabled(true), mSubElement(false),
		mNoClip(false), mID(id), mTabStop(false), mTabOrder(-1), mTabGroup(false), mAlignLeft(UIA_UPPERLEFT), 
		mAlignRight(UIA_UPPERLEFT), mAlignTop(UIA_UPPERLEFT), mAlignBottom(UIA_UPPERLEFT), mType(type)
	{
		mChildren.clear();
	}

	//! Destructor
	virtual ~BaseUIElement()
	{
		// delete all children
		eastl::list<eastl::shared_ptr<BaseUIElement>>::iterator it = mChildren.begin();
		for (; it != mChildren.end(); ++it)
		{
			(*it)->mParent = 0;
		}
		mChildren.clear();
	}

	//! set the parent of the element
	void SetParent(const eastl::shared_ptr<BaseUIElement>& parent)
	{
		if (parent)
		{
			// if we were given a parent to attach to
			parent->AddChildToEnd(shared_from_this());
			RecalculateAbsolutePosition(true);
		}
	}

	//! Returns parent of this element.
	const eastl::shared_ptr<BaseUIElement>& GetParent() const
	{
		return mParent;
	}


	//! Returns the relative rectangle of this element.
	RectangleShape<2, int> GetRelativePosition() const
	{
		return mRelativeRect;
	}


	//! Sets the relative rectangle of this element.
	/** \param r The absolute position to set */
	void SetRelativePosition(const RectangleShape<2, int>& r)
	{
		if (mParent)
		{
			const RectangleShape<2, int>& rectangle = mParent->GetAbsolutePosition();
			Vector2<int> dimension(rectangle.mExtent);

			if (mAlignLeft == UIA_SCALE)
			{
				float corner = mScaleRect.mCenter[0] - (mScaleRect.mExtent[0] / 2);
				int scale = (mDesiredRect.mCenter[0] - (mDesiredRect.mExtent[0] / 2)) / dimension[0];
				mScaleRect.mExtent[0] = corner - scale;
				mScaleRect.mCenter[0] = corner + (mDesiredRect.mExtent[0] / 2);
			}
			if (mAlignRight == UIA_SCALE)
			{
				float corner = mScaleRect.mCenter[0] + (int)round(mScaleRect.mExtent[0] / 2.f);
				int scale = (mDesiredRect.mCenter[0] + (int)round(mDesiredRect.mExtent[0] / 2.f)) / dimension[0];
				mScaleRect.mExtent[0] = scale - corner;
				mScaleRect.mCenter[0] = corner - (int)round(mDesiredRect.mExtent[0] / 2.f);
			}
			if (mAlignTop == UIA_SCALE)
			{
				float corner = mScaleRect.mCenter[1] - (mScaleRect.mExtent[1] / 2);
				int scale = (mDesiredRect.mCenter[1] - (mDesiredRect.mExtent[1] / 2)) / dimension[1];
				mScaleRect.mExtent[1] = corner - scale;
				mScaleRect.mCenter[1] = corner + (mDesiredRect.mExtent[1] / 2);
			}
			if (mAlignBottom == UIA_SCALE)
			{
				float corner = mScaleRect.mCenter[1] + (int)round(mScaleRect.mExtent[1] / 2.f);
				int scale = (mDesiredRect.mCenter[1] + (int)round(mDesiredRect.mExtent[1] / 2.f)) / dimension[1];
				mScaleRect.mExtent[1] = scale - corner;
				mScaleRect.mCenter[1] = corner - (int)round(mDesiredRect.mExtent[1] / 2.f);
			}
		}

		mDesiredRect = r;
		UpdateAbsolutePosition();
	}

	//! Sets the relative rectangle of this element, maintaining its current width and height
	/** \param position The new relative position to set. Width and height will not be changed. */
	void SetRelativePosition(const Vector2<int>& position)
	{
		Vector2<int> center;
		center[0] = position[0] + (mRelativeRect.mExtent[0] / 2);
		center[1] = position[1] + (mRelativeRect.mExtent[1] / 2);
		const RectangleShape<2, int> rectangle(
			center, mRelativeRect.mAxis, mRelativeRect.mExtent);
		SetRelativePosition(rectangle);
	}


	//! Sets the relative rectangle of this element as a proportion of its parent's area.
	/** \note This method used to be 'void SetRelativePosition(const Rectangle<2, float>& r)'
	\param r  The rectangle to set, interpreted as a proportion of the parent's area.
	Meaningful values are in the range [0...1], unless you intend this element to spill
	outside its parent. */
	void SetRelativePositionProportional(const RectangleShape<2, float>& r)
	{
		if (!mParent)
			return;

		const RectangleShape<2, int>& rectangle = mParent->GetAbsolutePosition();
		Vector2<int> dimension(rectangle.mExtent);

		Vector2<int> upperLeftCorner{ 
			(int)floor(dimension[0] * (r.mCenter[0] - (r.mExtent[0] / 2))),
			(int)floor(dimension[1] * (r.mCenter[1] - (r.mExtent[1] / 2))) };
		Vector2<int> lowerRightCorner{
			(int)floor(dimension[0] * (r.mCenter[0] + (int)round(r.mExtent[0] / 2.f))),
			(int)floor(dimension[1] * (r.mCenter[1] + (int)round(r.mExtent[1] / 2.f))) };
		eastl::array<Vector2<int>, 2U> axis;
		for (int i = 0; i < r.mAxis.count; i++)
			axis[i] = Vector2<int>{ (int)r.mAxis[i][0], (int)r.mAxis[i][1] };
		Vector2<int> extent = lowerRightCorner - upperLeftCorner;
		Vector2<int> center = lowerRightCorner + (extent / 2);

		mDesiredRect = RectangleShape<2, int>(center, axis, extent);
		mScaleRect = r;

		UpdateAbsolutePosition();
	}


	//! Gets the absolute rectangle of this element
	RectangleShape<2, int> GetAbsolutePosition() const
	{
		return mAbsoluteRect;
	}


	//! Returns the visible area of the element.
	RectangleShape<2, int> GetAbsoluteClippingRect() const
	{
		return mAbsoluteClippingRect;
	}


	//! Sets whether the element will ignore its parent's clipping rectangle
	/** \param noClip If true, the element will not be clipped by its parent's clipping rectangle. */
	void SetNotClipped(bool noClip)
	{
		mNoClip = noClip;
		UpdateAbsolutePosition();
	}


	//! Gets whether the element will ignore its parent's clipping rectangle
	/** \return true if the element is not clipped by its parent's clipping rectangle. */
	bool IsNotClipped() const
	{
		return mNoClip;
	}


	//! Sets the maximum size allowed for this element
	/** If set to 0,0, there is no maximum size */
	void SetMaxSize(Vector2<unsigned int> size)
	{
		mMaxSize = size;
		UpdateAbsolutePosition();
	}


	//! Sets the minimum size allowed for this element
	void SetMinSize(Vector2<unsigned int> size)
	{
		mMinSize = size;
		if (mMinSize[0] < 1)
			mMinSize[0] = 1;
		if (mMinSize[1] < 1)
			mMinSize[1] = 1;
		UpdateAbsolutePosition();
	}


	//! The alignment defines how the borders of this element will be positioned when the parent element is resized.
	void SetAlignment(UIAlignment left, UIAlignment right, UIAlignment top, UIAlignment bottom)
	{
		mAlignLeft = left;
		mAlignRight = right;
		mAlignTop = top;
		mAlignBottom = bottom;

		if (mParent)
		{
			const RectangleShape<2, int>& rectangle = mParent->GetAbsolutePosition();
			Vector2<int> dimension(rectangle.mExtent);

			if (mAlignLeft == UIA_SCALE)
			{
				float corner = mScaleRect.mCenter[0] - (mScaleRect.mExtent[0] / 2);
				int scale = (mDesiredRect.mCenter[0] - (mDesiredRect.mExtent[0] / 2)) / dimension[0];
				mScaleRect.mExtent[0] = corner - scale;
				mScaleRect.mCenter[0] = corner + (mDesiredRect.mExtent[0] / 2);
			}
			if (mAlignRight == UIA_SCALE)
			{
				float corner = mScaleRect.mCenter[0] + (int)round(mScaleRect.mExtent[0] / 2.f);
				int scale = (mDesiredRect.mCenter[0] + (int)round(mDesiredRect.mExtent[0] / 2.f)) / dimension[0];
				mScaleRect.mExtent[0] = scale - corner;
				mScaleRect.mCenter[0] = corner - (int)round(mDesiredRect.mExtent[0] / 2.f);
			}
			if (mAlignTop == UIA_SCALE)
			{
				float corner = mScaleRect.mCenter[1] - (mScaleRect.mExtent[1] / 2);
				int scale = (mDesiredRect.mCenter[1] - (mDesiredRect.mExtent[1] / 2)) / dimension[1];
				mScaleRect.mExtent[1] = corner - scale;
				mScaleRect.mCenter[1] = corner + (mDesiredRect.mExtent[1] / 2);
			}
			if (mAlignBottom == UIA_SCALE)
			{
				float corner = mScaleRect.mCenter[1] + (int)round(mScaleRect.mExtent[1] / 2.f);
				int scale = (mDesiredRect.mCenter[1] + (int)round(mDesiredRect.mExtent[1] / 2.f)) / dimension[1];
				mScaleRect.mExtent[1] = scale - corner;
				mScaleRect.mCenter[1] = corner - (int)round(mDesiredRect.mExtent[1] / 2.f);
			}
		}
	}


	//! Updates the absolute position.
	virtual void UpdateAbsolutePosition()
	{
		RecalculateAbsolutePosition(false);

		// update all children
		eastl::list<eastl::shared_ptr<BaseUIElement>>::iterator it = mChildren.begin();
		for (; it != mChildren.end(); ++it)
		{
			(*it)->UpdateAbsolutePosition();
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
	eastl::shared_ptr<BaseUIElement> GetElementFromPoint(const Vector2<int>& point)
	{
		eastl::shared_ptr<BaseUIElement> target = 0;

		// we have to search from back to front, because later children
		// might be drawn over the top of earlier ones.

		eastl::list<eastl::shared_ptr<BaseUIElement>>::reverse_iterator it = mChildren.rbegin();
		if (IsVisible())
		{
			while(it != mChildren.rend())
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
		return (
			mAbsoluteRect.mCenter[0] - (mAbsoluteRect.mExtent[0] / 2) <= point[0] &&
			mAbsoluteRect.mCenter[1] - (mAbsoluteRect.mExtent[1] / 2) <= point[1] &&
			mAbsoluteRect.mCenter[0] + (int)round(mAbsoluteRect.mExtent[0] / 2.f) >= point[0] &&
			mAbsoluteRect.mCenter[1] + (int)round(mAbsoluteRect.mExtent[1] / 2.f) >= point[1] );
	}

	//! Adds a UI element as new child of this element.
	virtual void AddChild(const eastl::shared_ptr<BaseUIElement>& child)
	{
		AddChildToEnd(child);
		if (child)
			child->UpdateAbsolutePosition();
	}


	//! Removes a child.
	virtual void RemoveChild(const eastl::shared_ptr<BaseUIElement>& child)
	{
		eastl::list<eastl::shared_ptr<BaseUIElement>>::iterator it = mChildren.begin();
		for (; it != mChildren.end(); ++it)
		{
			if ((*it) == child)
			{
				(*it)->mParent = 0;
				mChildren.erase(it);
				return;
			}
		}
	}


	//! Removes this element from its parent.
	virtual void Remove()
	{
		if (mParent)
			mParent->RemoveChild(shared_from_this());
	}


	//! draws the element and its children.
	virtual void Draw( )
	{
		if ( IsVisible() )
		{
			eastl::list<eastl::shared_ptr<BaseUIElement>>::iterator it = mChildren.begin();
			for (; it != mChildren.end(); ++it)
				(*it)->Draw();
		}
	}


	//! animate the element and its children.
	virtual void OnPostRender(unsigned int timeMs)
	{
		if ( IsVisible() )
		{
			eastl::list<eastl::shared_ptr<BaseUIElement>>::iterator it = mChildren.begin();
			for (; it != mChildren.end(); ++it)
				(*it)->OnPostRender( timeMs );
		}
	}


	//! Moves this element.
	virtual void Move(Vector2<int> absoluteMovement)
	{
		Vector2<int> center(mDesiredRect.mCenter);
		center[0] += absoluteMovement[0];
		center[1] += absoluteMovement[1];
		const RectangleShape<2, int> rectangle(
			center, mDesiredRect.mAxis, mDesiredRect.mExtent);

		SetRelativePosition(rectangle);
	}


	//! Returns true if element is visible.
	virtual bool IsVisible() const
	{
		return mVisible;
	}

	//! Check whether the element is truly visible, taking into accounts its parents' visibility
	/** \return true if the element and all its parents are visible,
	false if this or any parent element is invisible. */
	virtual bool IsTrulyVisible() const
	{
		if(!mVisible)
			return false;

		if(!mParent)
			return true;

		return mParent->IsTrulyVisible();
	}

	//! Sets the visible state of this element.
	virtual void SetVisible(bool visible)
	{
		mVisible = visible;
	}


	//! Returns true if this element was created as part of its parent control
	virtual bool IsSubElement() const
	{
		return mSubElement;
	}


	//! Sets whether this control was created as part of its parent.
	/** For example, it is true when a scrollbar is part of a UIListBox.
	SubElements are not saved to disk when calling guiEnvironment->saveUI() */
	virtual void SetSubElement(bool subElement)
	{
		mSubElement = subElement;
	}


	//! If set to true, the focus will visit this element when using the tab key to cycle through elements.
	/** If this element is a tab group (see IsTabGroup/SetTabGroup) then
	ctrl+tab will be used instead. */
	void SetTabStop(bool enable)
	{
		mTabStop = enable;
	}


	//! Returns true if this element can be Focused by navigating with the tab key
	bool IsTabStop() const
	{
		return mTabStop;
	}


	//! Sets the priority of focus when using the tab key to navigate between a group of elements.
	/** See SetTabGroup, IsTabGroup and GetTabGroup for information on tab groups.
	Elements with a lower number are Focused first */
	void SetTabOrder(int index)
	{
		// negative = autonumber
		if (index < 0)
		{
			mTabOrder = 0;
			eastl::shared_ptr<BaseUIElement> el = GetTabGroup();
			while (mTabGroup && el && el->mParent)
				el = el->mParent;

			const eastl::shared_ptr<BaseUIElement>& first=0, closest=0;
			if (el)
			{
				// find the highest element number
				el->GetNextElement(-1, true, mTabGroup, first, closest, true);
				if (first)
					mTabOrder = first->GetTabOrder() + 1;
			}
		}
		else mTabOrder = index;
	}


	//! Returns the number in the tab order sequence
	int GetTabOrder() const
	{
		return mTabOrder;
	}


	//! Sets whether this element is a container for a group of elements which can be navigated using the tab key.
	/** For example, windows are tab groups.
	Groups can be navigated using ctrl+tab, providing IsTabStop is true. */
	void SetTabGroup(bool isGroup)
	{
		mTabGroup = isGroup;
	}


	//! Returns true if this element is a tab group.
	bool IsTabGroup() const
	{
		return mTabGroup;
	}


	//! Returns the container element which holds all elements in this element's tab group.
	eastl::shared_ptr<BaseUIElement> GetTabGroup()
	{
		eastl::shared_ptr<BaseUIElement> ret(shared_from_this());
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
		if ( IsSubElement() && mEnabled && GetParent() )
			return GetParent()->IsEnabled();

		return mEnabled;
	}


	//! Sets the enabled state of this element.
	virtual void SetEnabled(bool enabled)
	{
		mEnabled = enabled;
	}


	//! Sets the new caption of this element.
	virtual void SetText(const wchar_t* text)
	{
		mText = text;
	}


	//! Returns caption of this element.
	virtual const wchar_t* GetText() const
	{
		return mText.c_str();
	}


	//! Sets the new caption of this element.
	virtual void SetToolTipText(const wchar_t* text)
	{
		mToolTipText = text;
	}


	//! Returns caption of this element.
	virtual const eastl::wstring& GetToolTipText() const
	{
		return mToolTipText;
	}


	//! Returns id. Can be used to identify the element.
	virtual int GetID() const
	{
		return mID;
	}


	//! Sets the id of this element
	virtual void SetID(int id)
	{
		mID = id;
	}

	//! Called if an event happened.
	virtual bool OnEvent(const Event& event)
	{
		return mParent ? mParent->OnEvent(event) : false;
	}


	//! Brings a child to front
	/** \return True if successful, false if not. */
	virtual bool BringToFront(const eastl::shared_ptr<BaseUIElement>& element)
	{
		eastl::list<eastl::shared_ptr<BaseUIElement>>::iterator it = mChildren.begin();
		for (; it != mChildren.end(); ++it)
		{
			if (element == (*it))
			{
				mChildren.erase(it);
				mChildren.push_back(element);
				return true;
			}
		}

		return false;
	}


	//! Moves a child to the back, so it's siblings are drawn on top of it
	/** \return True if successful, false if not. */
	virtual bool SendToBack(const eastl::shared_ptr<BaseUIElement>& element)
	{
		eastl::list<eastl::shared_ptr<BaseUIElement>>::iterator it = mChildren.begin();
		if (element == (*it)) // already there
			return true;

		for (; it != mChildren.end(); ++it)
		{
			if (element == (*it))
			{
				mChildren.erase(it);
				mChildren.push_front(element);
				return true;
			}
		}

		return false;
	}

	//! Returns list with children of this element
	virtual const eastl::list<eastl::shared_ptr<BaseUIElement>>& GetChildren() const
	{
		return mChildren;
	}


	//! Finds the first element with the given id.
	/** \param id: Id to search for.
	\param searchchildren: Set this to true, if also children of this
	element may contain the element with the searched id and they
	should be searched too.
	\return Returns the first element with the given id. If no element
	with this id was found, 0 is returned. */
	virtual eastl::shared_ptr<BaseUIElement> GetElementFromId(int id, bool searchchildren=false) const
	{
		eastl::shared_ptr<BaseUIElement> e = 0;

		eastl::list<eastl::shared_ptr<BaseUIElement>>::const_iterator it = mChildren.begin();
		for (; it != mChildren.end(); ++it)
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
	bool IsMyChild(BaseUIElement* child) const
	{
		if (!child)
			return false;
		do
		{
			if (child->mParent)
				child = child->mParent.get();

		} while (child->mParent && child != this);

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
		eastl::shared_ptr<BaseUIElement> first, eastl::shared_ptr<BaseUIElement> closest,
		bool includeInvisible=false) const
	{
		// we'll stop searching if we find this number
		int wanted = startOrder + ( reverse ? -1 : 1 );
		if (wanted==-2)
			wanted = 1073741824; // maximum int

		eastl::list<eastl::shared_ptr<BaseUIElement>>::const_iterator it = mChildren.begin();

		int closestOrder, currentOrder;
		while(it != mChildren.end())
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
	UIElementType GetType() const
	{
		return mType;
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
	virtual bool HasType(UIElementType type) const
	{
		return type == mType;
	}

	//! Returns the name of the element.
	/** \return Name as character string. */
	virtual const char* GetName() const
	{
		return mName.c_str();
	}


	//! Sets the name of the element.
	/** \param name New name of the gui element. */
	virtual void SetName(const char* name)
	{
		mName = name;
	}


	//! Sets the name of the element.
	/** \param name New name of the gui element. */
	virtual void SetName(const eastl::string& name)
	{
		mName = name;
	}

protected:

	// not virtual because needed in constructor
	void AddChildToEnd(const eastl::shared_ptr<BaseUIElement>& child)
	{
		if (child)
		{
			child->Remove(); // Remove from old parent
			child->mLastParentRect = GetAbsolutePosition();
			child->mParent = shared_from_this();
			mChildren.push_back(child);
		}
	}

	// not virtual because needed in constructor
	void RecalculateAbsolutePosition(bool recursive)
	{
		RectangleShape<2, int> parentAbsolute;
		RectangleShape<2, int> parentAbsoluteClip;
		float fw=0.f, fh=0.f;

		if (mParent)
		{
			parentAbsolute = mParent->mAbsoluteRect;

			if (mNoClip)
			{
				eastl::shared_ptr<BaseUIElement> p(shared_from_this());
				while (p->mParent)
					p = p->mParent;
				parentAbsoluteClip = p->mAbsoluteClippingRect;
			}
			else
				parentAbsoluteClip = mParent->mAbsoluteClippingRect;
		}

		const int diffx = parentAbsolute.mExtent[0] - mLastParentRect.mExtent[0];
		const int diffy = parentAbsolute.mExtent[1] - mLastParentRect.mExtent[1];

		if (mAlignLeft == UIA_SCALE || mAlignRight == UIA_SCALE)
			fw = (float)parentAbsolute.mExtent[0];

		if (mAlignTop == UIA_SCALE || mAlignBottom == UIA_SCALE)
			fh = (float)parentAbsolute.mExtent[1];

		switch (mAlignLeft)
		{
			case UIA_UPPERLEFT:
				break;
			case UIA_LOWERRIGHT:
				mDesiredRect.mCenter[0] -= diffx / 2;
				mDesiredRect.mExtent[0] -= diffx;
				break;
			case UIA_CENTER:
				mDesiredRect.mCenter[0] -= diffx / 4;
				mDesiredRect.mExtent[0] -= diffx / 2;
				break;
			case UIA_SCALE:
				int corner = mDesiredRect.mCenter[0] - (mDesiredRect.mExtent[0] / 2);
				int scale = (int)round((mScaleRect.mCenter[0] - (mScaleRect.mExtent[0] / 2)) * fw);
				mDesiredRect.mExtent[0] = corner - scale;
				mDesiredRect.mCenter[0] = corner - (int)round(mDesiredRect.mExtent[0] / 2.f);
				break;
		}

		switch (mAlignRight)
		{
			case UIA_UPPERLEFT:
				break;
			case UIA_LOWERRIGHT:
				mDesiredRect.mCenter[0] += diffx / 2;
				mDesiredRect.mExtent[0] += diffx;
				break;
			case UIA_CENTER:
				mDesiredRect.mCenter[0] += diffx / 4;
				mDesiredRect.mExtent[0] += diffx / 2;
				break;
			case UIA_SCALE:
				int corner = mDesiredRect.mCenter[0] - (int)round(mDesiredRect.mExtent[0] / 2.f);
				int scale = (int)round((mScaleRect.mCenter[0] + (int)round(mScaleRect.mExtent[0] / 2.f)) * fw);
				mDesiredRect.mExtent[0] = scale - corner;
				mDesiredRect.mCenter[0] = corner + (mDesiredRect.mExtent[0] / 2);
				break;
		}

		switch (mAlignTop)
		{
			case UIA_UPPERLEFT:
				break;
			case UIA_LOWERRIGHT:
				mDesiredRect.mCenter[1] -= diffy / 2;
				mDesiredRect.mExtent[1] -= diffy;
				break;
			case UIA_CENTER:
				mDesiredRect.mCenter[1] -= diffy / 4;
				mDesiredRect.mExtent[1] -= diffy / 2;
				break;
			case UIA_SCALE:
				int corner = mDesiredRect.mCenter[1] - (mDesiredRect.mExtent[1] / 2);
				int scale = (int)round((mScaleRect.mCenter[1] - (mScaleRect.mExtent[1] / 2)) * fh);
				mDesiredRect.mExtent[1] = corner - scale;
				mDesiredRect.mCenter[1] = corner - (int)round(mDesiredRect.mExtent[1] / 2.f);
				break;
		}

		switch (mAlignBottom)
		{
			case UIA_UPPERLEFT:
				break;
			case UIA_LOWERRIGHT:
				mDesiredRect.mCenter[1] += diffy / 2;
				mDesiredRect.mExtent[1] += diffy;
				break;
			case UIA_CENTER:
				mDesiredRect.mCenter[1] += diffy / 4;
				mDesiredRect.mExtent[1] += diffy / 2;
				break;
			case UIA_SCALE:
				int corner = mDesiredRect.mCenter[1] - (int)round(mDesiredRect.mExtent[1] / 2.f);
				int scale = (int)round((mScaleRect.mCenter[1] + (int)round(mScaleRect.mExtent[1] / 2.f)) * fw);
				mDesiredRect.mExtent[1] = scale - corner;
				mDesiredRect.mCenter[1] = corner + (mDesiredRect.mExtent[1] / 2);
				break;
		}

		mRelativeRect = mDesiredRect;

		const int w = mRelativeRect.mExtent[0];
		const int h = mRelativeRect.mExtent[1];

		// make sure the desired rectangle is allowed
		if (w < (int)mMinSize[0])
		{
			mRelativeRect.mCenter[0] -= mRelativeRect.mExtent[0] / 2;
			mRelativeRect.mCenter[0] += mMinSize[0] / 2;
			mRelativeRect.mExtent[0] = mMinSize[0];
		}
		if (h < (int)mMinSize[1])
		{
			mRelativeRect.mCenter[1] -= mRelativeRect.mExtent[1] / 2;
			mRelativeRect.mCenter[1] += mMinSize[1] / 2;
			mRelativeRect.mExtent[1] = mMinSize[1];
		}
		if (mMaxSize[0] && w > (int)mMaxSize[0])
		{
			mRelativeRect.mCenter[0] -= mRelativeRect.mExtent[0] / 2;
			mRelativeRect.mCenter[0] += mMaxSize[0] / 2;
			mRelativeRect.mExtent[0] = mMaxSize[0];
		}
		if (mMaxSize[1] && h > (int)mMaxSize[1])
		{
			mRelativeRect.mCenter[1] -= mRelativeRect.mExtent[1] / 2;
			mRelativeRect.mCenter[1] += mMaxSize[1] / 2;
			mRelativeRect.mExtent[1] = mMaxSize[1];
		}

		mAbsoluteRect = mRelativeRect;
		mAbsoluteRect.mCenter += parentAbsolute.mCenter - (parentAbsolute.mExtent / 2);

		if (!mParent)
			parentAbsoluteClip = mAbsoluteRect;

		mAbsoluteClippingRect = mAbsoluteRect;
		mAbsoluteClippingRect.mExtent = parentAbsoluteClip.mExtent;

		mLastParentRect = parentAbsolute;

		if ( recursive )
		{
			// update all children
			eastl::list<eastl::shared_ptr<BaseUIElement>>::iterator it = mChildren.begin();
			for (; it != mChildren.end(); ++it)
			{
				(*it)->RecalculateAbsolutePosition(recursive);
			}
		}
	}

protected:

	//! List of all children of this element
	eastl::list<eastl::shared_ptr<BaseUIElement>> mChildren;

	//! Pointer to the parent
	eastl::shared_ptr<BaseUIElement> mParent;

	//! relative rect of element
	RectangleShape<2, int> mRelativeRect;

	//! absolute rect of element
	RectangleShape<2, int> mAbsoluteRect;

	//! absolute clipping rect of element
	RectangleShape<2, int> mAbsoluteClippingRect;

	//! the rectangle the element would prefer to be,
	//! if it was not constrained by parent or max/min size
	RectangleShape<2, int> mDesiredRect;

	//! for calculating the difference when resizing parent
	RectangleShape<2, int> mLastParentRect;

	//! relative scale of the element inside its parent
	RectangleShape<2, float> mScaleRect;

	//! maximum and minimum size of the element
	Vector2<unsigned int> mMaxSize, mMinSize;

	//! is visible?
	bool mVisible;

	//! is enabled?
	bool mEnabled;

	//! is a part of a larger whole and should not be serialized?
	bool mSubElement;

	//! does this element ignore its parent's clipping rectangle?
	bool mNoClip;

	//! caption
	eastl::wstring mText;

	//! tooltip
	eastl::wstring mToolTipText;

	//! users can set this for identificating the element by string
	eastl::string mName;

	//! users can set this for identificating the element by integer
	int mID;

	//! tab stop like in windows
	bool mTabStop;

	//! tab order
	int mTabOrder;

	//! tab groups are containers like windows, use ctrl+tab to navigate
	bool mTabGroup;

	//! tells the element how to act when its parent is resized
	UIAlignment mAlignLeft, mAlignRight, mAlignTop, mAlignBottom;

	//! type of element
	UIElementType mType;
};

#endif