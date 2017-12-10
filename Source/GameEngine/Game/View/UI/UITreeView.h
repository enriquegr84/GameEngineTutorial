// This file is part of the "Irrlicht Engine".
// written by Reinhard Ostermeier, reinhard@nospam.r-ostermeier.de

#ifndef UITREEVIEW_H
#define UITREEVIEW_H

#include "UIElement.h"

#include "Graphic/Effect/BasicEffect.h"
#include "Graphic/Scene/Hierarchy/Visual.h"

// forward declarations
class BaseUIFont;
class BaseUIScrollBar;
class BaseUITreeView;
class BaseUITreeViewNode;

//! Node for gui tree view
/** \par This element can create the following events of type EGUI_EVENT_TYPE:
\li UIEVT_TREEVIEW_NODE_EXPAND
\li UIEVT_TREEVIEW_NODE_COLLAPS
\li UIEVT_TREEVIEW_NODE_DESELECT
\li UIEVT_TREEVIEW_NODE_SELECT
*/
class BaseUITreeViewNode : public eastl::enable_shared_from_this<BaseUITreeViewNode>
{
public:
	//! returns the owner (tree view) of this node
	virtual eastl::shared_ptr<BaseUIElement> GetOwner() const = 0;

	//! Returns the parent node of this node.
	/** For the root node this will return 0. */
	virtual eastl::shared_ptr<BaseUITreeViewNode> GetParent() const = 0;

	//! returns the text of the node
	virtual const wchar_t* GetText() const = 0;

	//! sets the text of the node
	virtual void SetText(const wchar_t* text) = 0;

	//! returns the icon text of the node
	virtual const wchar_t* GetIcon() const = 0;

	//! sets the icon text of the node
	virtual void SetIcon(const wchar_t* icon) = 0;

	//! returns the image index of the node
	virtual unsigned int GetImageIndex() const = 0;

	//! sets the image index of the node
	virtual void SetImageIndex(unsigned int imageIndex) = 0;

	//! returns the image index of the node
	virtual unsigned int GetSelectedImageIndex() const = 0;

	//! sets the image index of the node
	virtual void SetSelectedImageIndex(unsigned int imageIndex) = 0;

	//! returns the user data (void*) of this node
	virtual void* GetData() const = 0;

	//! sets the user data (void*) of this node
	virtual void SetData(void* data) = 0;

	//! returns the user data2 (IReferenceCounted) of this node
	virtual void* GetData2() const = 0;

	//! sets the user data2 (IReferenceCounted) of this node
	virtual void SetData2(void* data) = 0;

	//! returns the child item count
	virtual unsigned int GetChildCount() const = 0;

	//! removes all children (recursive) from this node
	virtual void ClearChildren() = 0;

	//! returns true if this node has child nodes
	virtual bool HasChildren() const = 0;

	//! Adds a new node behind the last child node.
	/** \param text text of the new node
	\param icon icon text of the new node
	\param imageIndex index of the image for the new node (-1 = none)
	\param selectedImageIndex index of the selected image for the new node (-1 = same as imageIndex)
	\param data user data (void*) of the new node
	\param data2 user data2 (IReferenceCounted*) of the new node
	\return The new node
	*/
	virtual eastl::shared_ptr<BaseUITreeViewNode> AddChildBack(
		const wchar_t* text, const wchar_t* icon = 0, int imageIndex = -1, 
		int selectedImageIndex = -1, void* data = 0, void* data2 = 0) = 0;

	//! Adds a new node before the first child node.
	/** \param text text of the new node
	\param icon icon text of the new node
	\param imageIndex index of the image for the new node (-1 = none)
	\param selectedImageIndex index of the selected image for the new node (-1 = same as imageIndex)
	\param data user data (void*) of the new node
	\param data2 user data2 (IReferenceCounted*) of the new node
	\return The new node
	*/
	virtual eastl::shared_ptr<BaseUITreeViewNode> AddChildFront(
		const wchar_t* text, const wchar_t* icon = 0, int imageIndex = -1, 
		int selectedImageIndex = -1, void* data = 0, void* data2 = 0) = 0;

	//! Adds a new node behind the other node.
	/** The other node has also te be a child node from this node.
	\param other Node to insert after
	\param text text of the new node
	\param icon icon text of the new node
	\param imageIndex index of the image for the new node (-1 = none)
	\param selectedImageIndex index of the selected image for the new node (-1 = same as imageIndex)
	\param data user data (void*) of the new node
	\param data2 user data2 (IReferenceCounted*) of the new node
	\return The new node or 0 if other is no child node from this
	*/
	virtual eastl::shared_ptr<BaseUITreeViewNode> InsertChildAfter(
		eastl::shared_ptr<BaseUITreeViewNode> other, const wchar_t* text, const wchar_t* icon = 0,
		int imageIndex = -1, int selectedImageIndex = -1, void* data = 0, void* data2 = 0) = 0;

	//! Adds a new node before the other node.
	/** The other node has also te be a child node from this node.
	\param other Node to insert before
	\param text text of the new node
	\param icon icon text of the new node
	\param imageIndex index of the image for the new node (-1 = none)
	\param selectedImageIndex index of the selected image for the new node (-1 = same as imageIndex)
	\param data user data (void*) of the new node
	\param data2 user data2 (IReferenceCounted*) of the new node
	\return The new node or 0 if other is no child node from this
	*/
	virtual eastl::shared_ptr<BaseUITreeViewNode> InsertChildBefore(
		eastl::shared_ptr<BaseUITreeViewNode> other, const wchar_t* text, const wchar_t* icon = 0,
		int imageIndex = -1, int selectedImageIndex = -1, void* data = 0, void* data2 = 0) = 0;

	//! Return the first child node from this node.
	/** \return The first child node or 0 if this node has no children. */
	virtual eastl::shared_ptr<BaseUITreeViewNode> GetFrontChild() const = 0;

	//! Return the last child node from this node.
	/** \return The last child node or 0 if this node has no children. */
	virtual eastl::shared_ptr<BaseUITreeViewNode> GetBackChild() const = 0;

	//! Returns the previous sibling node from this node.
	/** \return The previous sibling node from this node or 0 if this is
	the first node from the parent node.
	*/
	virtual eastl::shared_ptr<BaseUITreeViewNode> GetPrevNode() const = 0;

	//! Returns the next sibling node from this node.
	/** \return The next sibling node from this node or 0 if this is
	the last node from the parent node.
	*/
	virtual eastl::shared_ptr<BaseUITreeViewNode> GetNextNode() const = 0;

	//! Returns the next visible (expanded, may be out of scrolling) node from this node.
	/** \return The next visible node from this node or 0 if this is
	the last visible node. */
	virtual eastl::shared_ptr<BaseUITreeViewNode> GetNextVisible() const = 0;

	//! Deletes a child node.
	/** \return Returns true if the node was found as a child and is deleted. */
	virtual bool DeleteChild(eastl::shared_ptr<BaseUITreeViewNode> child) = 0;

	//! Moves a child node one position up.
	/** \return True if the node was found as achild node and was not already the first child. */
	virtual bool MoveChildUp(eastl::shared_ptr<BaseUITreeViewNode> child) = 0;

	//! Moves a child node one position down.
	/** \return True if the node was found as achild node and was not already the last child. */
	virtual bool MoveChildDown(eastl::shared_ptr<BaseUITreeViewNode> child) = 0;

	//! Returns true if the node is expanded (children are visible).
	virtual bool GetExpanded() const = 0;

	//! Sets if the node is expanded.
	virtual void SetExpanded(bool expanded) = 0;

	//! Returns true if the node is currently selected.
	virtual bool GetSelected() const = 0;

	//! Sets this node as selected.
	virtual void SetSelected(bool selected) = 0;

	//! Returns true if this node is the root node.
	virtual bool IsRoot() const = 0;

	//! Returns the level of this node.
	/** The root node has level 0. Direct children of the root has level 1 ... */
	virtual int GetLevel() const = 0;

	//! Returns true if this node is visible (all parents are expanded).
	virtual bool IsVisible() const = 0;
};


//! Default tree view GUI element.
/** Displays a windows like tree buttons to expand/collaps the child
nodes of an node and optional tree lines. Each node consits of an
text, an icon text and a void pointer for user data. */
class BaseUITreeView : public BaseUIElement
{
public:
	//! constructor
	BaseUITreeView(int id, RectangleShape<2, int> rectangle)
		: BaseUIElement(UIET_TREE_VIEW, id, rectangle) {}

	//! initialize treeview
	virtual void OnInit(bool scrollBarVertical = true, bool scrollBarHorizontal = true) = 0;

	//! returns the root node (not visible) from the tree.
	virtual eastl::shared_ptr<BaseUITreeViewNode> GetRoot() const = 0;

	//! returns the selected node of the tree or 0 if none is selected
	virtual eastl::shared_ptr<BaseUITreeViewNode> GetSelected() const = 0;

	//! returns true if the tree lines are visible
	virtual bool GetLinesVisible() const = 0;

	//! sets if the tree lines are visible
	/** \param visible true for visible, false for invisible */
	virtual void SetLinesVisible(bool visible) = 0;

	//! Sets the font which should be used as icon font.
	/** This font is set to the Irrlicht engine built-in-font by
	default. Icons can be displayed in front of every list item.
	An icon is a string, displayed with the icon font. When using
	the build-in-font of the Irrlicht engine as icon font, the icon
	strings defined in GUIIcons.h can be used.
	*/
	virtual void SetIconFont(eastl::shared_ptr<BaseUIFont> font) = 0;

	//! Sets if the image is left of the icon. Default is true.
	virtual void SetImageLeftOfIcon(bool bLeftOf) = 0;

	//! Returns if the Image is left of the icon. Default is true.
	virtual bool GetImageLeftOfIcon() const = 0;

	//! Returns the node which is associated to the last event.
	/** This pointer is only valid inside the OnEvent call! */
	virtual eastl::shared_ptr<BaseUITreeViewNode> GetLastEventNode() const = 0;
};

//! Node for gui tree view
class UITreeViewNode : public BaseUITreeViewNode
{
friend class UITreeView;

public:
	//! constructor
	UITreeViewNode(eastl::shared_ptr<BaseUIElement> owner, eastl::shared_ptr<BaseUITreeViewNode> parent);

	//! destructor
	~UITreeViewNode();

	//! returns the owner (tree view) of this node
	virtual eastl::shared_ptr<BaseUIElement> GetOwner() const;

	//! Returns the parent node of this node.
	virtual eastl::shared_ptr<BaseUITreeViewNode> GetParent() const;

	//! returns the text of the node
	virtual const wchar_t* GetText() const { return mText.c_str(); }

	//! sets the text of the node
	virtual void SetText( const wchar_t* text );

	//! returns the icon text of the node
	virtual const wchar_t* GetIcon() const { return mIcon.c_str(); }

	//! sets the icon text of the node
	virtual void SetIcon( const wchar_t* icon );

	//! returns the image index of the node
	virtual unsigned int GetImageIndex() const { return mImageIndex; }

	//! sets the image index of the node
	virtual void SetImageIndex( unsigned int imageIndex ) { mImageIndex = imageIndex; }

	//! returns the image index of the node
	virtual unsigned int GetSelectedImageIndex() const { return mSelectedImageIndex; }

	//! sets the image index of the node
	virtual void SetSelectedImageIndex( unsigned int imageIndex ) { mSelectedImageIndex = imageIndex; }

	//! returns the user data (void*) of this node
	virtual void* GetData() const { return mData; }

	//! sets the user data (void*) of this node
	virtual void SetData( void* data ) { mData = data; }

	//! returns the user data2 (IReferenceCounted) of this node
	virtual void* GetData2() const { return mData2; }

	//! sets the user data2 (IReferenceCounted) of this node
	virtual void SetData2( void* data )
	{
		mData2 = data;
	}

	//! returns the child item count
	virtual unsigned int GetChildCount() const { return mChildren.size(); }

	//! removes all children (recursive) from this node
	virtual void ClearChildren();

	//! returns true if this node has child nodes
	virtual bool HasChildren() const { return !mChildren.empty(); }

	//! Adds a new node behind the last child node.
	//! \param text text of the new node
	//! \param icon icon text of the new node
	//! \param imageIndex index of the image for the new node (-1 = none)
	//! \param selectedImageIndex index of the selected image for the new node (-1 = same as imageIndex)
	//! \param data user data (void*) of the new node
	//! \param data2 user data2 (IReferenceCounted*) of the new node
	//! \return
	//! returns the new node
	virtual eastl::shared_ptr<BaseUITreeViewNode> AddChildBack(
			const wchar_t* text, const wchar_t* icon = 0, int imageIndex = -1, 
			int selectedImageIndex = -1, void* data = 0, void* data2 = 0);

	//! Adds a new node before the first child node.
	//! \param text text of the new node
	//! \param icon icon text of the new node
	//! \param imageIndex index of the image for the new node (-1 = none)
	//! \param selectedImageIndex index of the selected image for the new node (-1 = same as imageIndex)
	//! \param data user data (void*) of the new node
	//! \param data2 user data2 (IReferenceCounted*) of the new node
	//! \return
	//! returns the new node
	virtual eastl::shared_ptr<BaseUITreeViewNode> AddChildFront(
			const wchar_t* text, const wchar_t* icon = 0, int imageIndex = -1, 
			int selectedImageIndex = -1, void* data = 0, void* data2 = 0 );

	//! Adds a new node behind the other node.
	//! The other node has also te be a child node from this node.
	//! \param text text of the new node
	//! \param icon icon text of the new node
	//! \param imageIndex index of the image for the new node (-1 = none)
	//! \param selectedImageIndex index of the selected image for the new node (-1 = same as imageIndex)
	//! \param data user data (void*) of the new node
	//! \param data2 user data2 (IReferenceCounted*) of the new node
	//! \return
	//! returns the new node or 0 if other is no child node from this
	virtual eastl::shared_ptr<BaseUITreeViewNode> InsertChildAfter(
			eastl::shared_ptr<BaseUITreeViewNode> other, const wchar_t* text, const wchar_t* icon = 0,
			int imageIndex = -1, int selectedImageIndex = -1, void* data = 0, void* data2 = 0 );

	//! Adds a new node before the other node.
	//! The other node has also te be a child node from this node.
	//! \param text text of the new node
	//! \param icon icon text of the new node
	//! \param imageIndex index of the image for the new node (-1 = none)
	//! \param selectedImageIndex index of the selected image for the new node (-1 = same as imageIndex)
	//! \param data user data (void*) of the new node
	//! \param data2 user data2 (IReferenceCounted*) of the new node
	//! \return
	//! returns the new node or 0 if other is no child node from this
	virtual eastl::shared_ptr<BaseUITreeViewNode> InsertChildBefore(
			eastl::shared_ptr<BaseUITreeViewNode> other, const wchar_t* text, const wchar_t* icon = 0,
			int imageIndex = -1, int selectedImageIndex = -1, void* data = 0, void* data2 = 0 );

	//! Return the first child note from this node.
	virtual eastl::shared_ptr<BaseUITreeViewNode> GetFrontChild() const;

	//! Return the last child note from this node.
	virtual eastl::shared_ptr<BaseUITreeViewNode> GetBackChild() const;

	//! Returns the preverse sibling node from this node.
	virtual eastl::shared_ptr<BaseUITreeViewNode> GetPrevNode() const;

	//! Returns the next sibling node from this node.
	virtual eastl::shared_ptr<BaseUITreeViewNode> GetNextNode() const;

	//! Returns the next visible (expanded, may be out of scrolling) node from this node.
	virtual eastl::shared_ptr<BaseUITreeViewNode> GetNextVisible() const;

	//! Deletes a child node.
	virtual bool DeleteChild(eastl::shared_ptr<BaseUITreeViewNode> child );

	//! Moves a child node one position up.
	virtual bool MoveChildUp(eastl::shared_ptr<BaseUITreeViewNode> child );

	//! Moves a child node one position down.
	virtual bool MoveChildDown(eastl::shared_ptr<BaseUITreeViewNode> child );

	//! Returns true if the node is expanded (children are visible).
	virtual bool GetExpanded() const { return mExpanded; }

	//! Sets if the node is expanded.
	virtual void SetExpanded( bool expanded );

	//! Returns true if the node is currently selected.
	virtual bool GetSelected() const;

	//! Sets this node as selected.
	virtual void SetSelected( bool selected );

	//! Returns true if this node is the root node.
	virtual bool IsRoot() const;

	//! Returns the level of this node.
	virtual int GetLevel() const;

	//! Returns true if this node is visible (all parents are expanded).
	virtual bool IsVisible() const;

private:

	eastl::shared_ptr<BaseUIElement> mOwner;
	eastl::shared_ptr<BaseUITreeViewNode> mParent;

	eastl::wstring mText;
	eastl::wstring mIcon;
	int mImageIndex;
	int mSelectedImageIndex;
	void* mData;
	void* mData2;
	bool mExpanded;
	eastl::list<eastl::shared_ptr<BaseUITreeViewNode>> mChildren;
};


//! Default tree view GUI element.
class UITreeView : public BaseUITreeView
{
friend class UITreeViewNode;

public:
	//! constructor
	UITreeView(BaseUI* ui, int id, RectangleShape<2, int> rectangle, 
		bool clip = true, bool drawBack = false);

	//! destructor
	virtual ~UITreeView();

	//! initialize treeview
	virtual void OnInit(bool scrollBarVertical = true, bool scrollBarHorizontal = true);

	//! returns the root node (not visible) from the tree.
	virtual eastl::shared_ptr<BaseUITreeViewNode> GetRoot() const { return mRoot; }

	//! returns the selected node of the tree or 0 if none is selected
	virtual eastl::shared_ptr<BaseUITreeViewNode> GetSelected() const { return mSelected; }

	//! returns true if the tree lines are visible
	virtual bool GetLinesVisible() const { return mLinesVisible; }

	//! sets if the tree lines are visible
	virtual void SetLinesVisible( bool visible ) { mLinesVisible = visible; }

	//! called if an event happened.
	virtual bool OnEvent( const Event &event );

	//! draws the element and its children
	virtual void Draw();

	//! Sets the font which should be used as icon font. This font is set to the Irrlicht engine
	//! built-in-font by default. Icons can be displayed in front of every list item.
	//! An icon is a string, displayed with the icon font. When using the build-in-font of the
	//! Irrlicht engine as icon font, the icon strings defined in GUIIcons.h can be used.
	virtual void SetIconFont(eastl::shared_ptr<BaseUIFont> font);

	//! Sets if the image is left of the icon. Default is true.
	virtual void SetImageLeftOfIcon( bool bLeftOf ) { mImageLeftOfIcon = bLeftOf; }

	//! Returns if the Image is left of the icon. Default is true.
	virtual bool GetImageLeftOfIcon() const { return mImageLeftOfIcon; }

	//! Returns the node which is associated to the last event.
	virtual eastl::shared_ptr<BaseUITreeViewNode> GetLastEventNode() const { return mLastEventNode; }

private:
	//! calculates the heigth of an node and of all visible nodes.
	void RecalculateItemHeight();

	//! executes an mouse action (like selectNew of CGUIListBox)
	void MouseAction( int xpos, int ypos, bool onlyHover = false );

	eastl::shared_ptr<BaseUITreeViewNode> mRoot;
	eastl::shared_ptr<BaseUITreeViewNode> mSelected;

	BaseUI* mUI;

	eastl::shared_ptr<Visual> mVisual;
	eastl::shared_ptr<BasicEffect> mEffect;

	int mItemHeight;
	int mIndentWidth;
	int mTotalItemHeight;
	int mTotalItemWidth;
	eastl::shared_ptr<BaseUIFont> mFont;
	eastl::shared_ptr<BaseUIFont> mIconFont;
	eastl::shared_ptr<BaseUIScrollBar> mScrollBarH;
	eastl::shared_ptr<BaseUIScrollBar> mScrollBarV;
	eastl::shared_ptr<BaseUITreeViewNode> mLastEventNode;
	bool mLinesVisible;
	bool mSelecting;
	bool mClip;
	bool mDrawBack;
	bool mImageLeftOfIcon;
};

#endif