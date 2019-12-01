// This file is part of the "Irrlicht Engine".
// written by Reinhard Ostermeier, reinhard@nospam.r-ostermeier.de
// expanded by burningwater

#include "UITreeView.h"

#include "Graphic/Renderer/Renderer.h"
#include "Core/OS/OS.h"

UITreeViewNode::UITreeViewNode(eastl::shared_ptr<BaseUIElement> owner, eastl::shared_ptr<BaseUITreeViewNode> parent )
	: mOwner(owner), mParent(parent), mImageIndex(-1), mSelectedImageIndex(-1), mData(0), mData2(0), mExpanded(false)
{

}

UITreeViewNode::~UITreeViewNode()
{
	if( mOwner)
	{
		BaseUITreeView* owner = reinterpret_cast<BaseUITreeView*>(mOwner.get());
		if (shared_from_this() == owner->GetSelected())
			SetSelected( false );
	}

	ClearChildren();
}

eastl::shared_ptr<BaseUIElement> UITreeViewNode::GetOwner() const
{
	return mOwner;
}

eastl::shared_ptr<BaseUITreeViewNode> UITreeViewNode::GetParent() const
{
	return mParent;
}

void UITreeViewNode::SetText( const wchar_t* text )
{
	mText = text;
}

void UITreeViewNode::SetIcon( const wchar_t* icon )
{
	mIcon = icon;
}

void UITreeViewNode::ClearChildren()
{
	mChildren.clear();
}

eastl::shared_ptr<BaseUITreeViewNode> UITreeViewNode::AddChildBack(
	const wchar_t* text, const wchar_t* icon /*= 0*/,
	int imageIndex /*= -1*/, int selectedImageIndex /*= -1*/,
	void* data /*= 0*/, void* data2 /*= 0*/ )
{
	eastl::shared_ptr<UITreeViewNode> newChild(new UITreeViewNode( mOwner, shared_from_this() ));

	mChildren.push_back( newChild );
	newChild->mText = text;
	newChild->mIcon = icon;
	newChild->mImageIndex = imageIndex;
	newChild->mSelectedImageIndex = selectedImageIndex;
	newChild->mData = data;
	newChild->mData2 = data2;

	return newChild;
}

eastl::shared_ptr<BaseUITreeViewNode> UITreeViewNode::AddChildFront(
	const wchar_t* text, const wchar_t* icon /*= 0*/,
	int imageIndex /*= -1*/, int selectedImageIndex /*= -1*/,
	void* data /*= 0*/, void* data2 /*= 0*/ )
{
	eastl::shared_ptr<UITreeViewNode> newChild(new UITreeViewNode( mOwner, shared_from_this() ));

	mChildren.push_front( newChild );
	newChild->mText = text;
	newChild->mIcon = icon;
	newChild->mImageIndex = imageIndex;
	newChild->mSelectedImageIndex = selectedImageIndex;
	newChild->mData = data;
	newChild->mData2 = data2;

	return newChild;
}

eastl::shared_ptr<BaseUITreeViewNode> UITreeViewNode::InsertChildAfter(
	eastl::shared_ptr<BaseUITreeViewNode> other,
	const wchar_t* text, const wchar_t* icon /*= 0*/,
	int imageIndex /*= -1*/, int selectedImageIndex /*= -1*/,
	void* data /*= 0*/, void* data2/* = 0*/ )
{
	eastl::shared_ptr<UITreeViewNode> newChild = 0;

	for( auto itOther = mChildren.begin(); itOther != mChildren.end(); itOther++ )
	{
		if( other == *itOther )
		{
			newChild.reset(new UITreeViewNode( mOwner, shared_from_this() ));
			newChild->mText = text;
			newChild->mIcon = icon;
			newChild->mImageIndex = imageIndex;
			newChild->mSelectedImageIndex = selectedImageIndex;
			newChild->mData = data;
			newChild->mData2 = data2;

			mChildren.insert( itOther, newChild );
			break;
		}
	}
	return newChild;
}

eastl::shared_ptr<BaseUITreeViewNode> UITreeViewNode::InsertChildBefore(
	eastl::shared_ptr<BaseUITreeViewNode> other,
	const wchar_t* text, const wchar_t* icon /*= 0*/,
	int imageIndex /*= -1*/, int selectedImageIndex /*= -1*/,
	void* data /*= 0*/, void* data2/* = 0*/ )
{
	eastl::shared_ptr<UITreeViewNode> newChild = 0;

	for( auto itOther = mChildren.begin(); itOther != mChildren.end(); itOther++ )
	{
		if( other == *itOther )
		{
			newChild.reset(new UITreeViewNode(mOwner, shared_from_this()));
			newChild->mText = text;
			newChild->mIcon = icon;
			newChild->mImageIndex = imageIndex;
			newChild->mSelectedImageIndex = selectedImageIndex;
			newChild->mData = data;
			newChild->mData2 = data2;

			mChildren.insert( itOther, newChild );
			break;
		}
	}
	return newChild;
}

eastl::shared_ptr<BaseUITreeViewNode> UITreeViewNode::GetFrontChild() const
{
	if( mChildren.empty() )
		return nullptr;
	else
		return mChildren.front();
}

eastl::shared_ptr<BaseUITreeViewNode> UITreeViewNode::GetBackChild() const
{
	if( mChildren.empty() )
		return nullptr;
	else
		return mChildren.back();
}

eastl::shared_ptr<BaseUITreeViewNode> UITreeViewNode::GetPrevNode() const
{
	eastl::list<eastl::shared_ptr<BaseUITreeViewNode>>::iterator itOther;
	eastl::shared_ptr<BaseUITreeViewNode> other = 0;

	if(mParent)
	{
		UITreeViewNode* parent = reinterpret_cast<UITreeViewNode*>(mParent.get());
		for( auto itThis = parent->mChildren.begin(); itThis != parent->mChildren.end(); itThis++ )
		{
			if( shared_from_this() == *itThis )
			{
				if( itThis != parent->mChildren.begin() )
				{
					other = *itOther;
				}
				break;
			}
			itOther = itThis;
		}
	}
	return other;
}

eastl::shared_ptr<BaseUITreeViewNode> UITreeViewNode::GetNextNode() const
{
	eastl::shared_ptr<BaseUITreeViewNode> other = 0;

	if( mParent )
	{
		UITreeViewNode* parent = reinterpret_cast<UITreeViewNode*>(mParent.get());
		for (auto itThis = parent->mChildren.begin(); itThis != parent->mChildren.end(); itThis++)
		{
			if( shared_from_this() == *itThis )
			{
				if( *itThis != parent->mChildren.back() )
				{
					other = *( ++itThis );
				}
				break;
			}
		}
	}
	return other;
}

eastl::shared_ptr<BaseUITreeViewNode> UITreeViewNode::GetNextVisible() const
{
	eastl::shared_ptr<BaseUITreeViewNode> next = 0;
	eastl::shared_ptr<const BaseUITreeViewNode> node = shared_from_this();

	if( node->GetExpanded() && node->HasChildren() )
		next = node->GetFrontChild();
	else
		next = node->GetNextNode();

	while( !next && node->GetParent() )
	{
		next = node->GetParent()->GetNextNode();
		if( !next ) node = node->GetParent();
	}

	return next;
}

bool UITreeViewNode::DeleteChild( eastl::shared_ptr<BaseUITreeViewNode> child )
{
	bool deleted = false;

	for( auto itChild = mChildren.begin(); itChild != mChildren.end(); itChild++ )
	{
		if( child == *itChild )
		{
			mChildren.erase( itChild );
			deleted = true;
			break;
		}
	}
	return deleted;
}

bool UITreeViewNode::MoveChildUp( eastl::shared_ptr<BaseUITreeViewNode> child )
{
	eastl::list<eastl::shared_ptr<BaseUITreeViewNode>>::iterator itOther;
	eastl::shared_ptr<BaseUITreeViewNode> nodeTmp;
	bool moved = false;

	for( auto itChild = mChildren.begin(); itChild != mChildren.end(); itChild++ )
	{
		if( child == *itChild )
		{
			if( itChild != mChildren.begin() )
			{
				nodeTmp = *itChild;
				*itChild = *itOther;
				*itOther = nodeTmp;
				moved = true;
			}
			break;
		}
		itOther = itChild;
	}
	return moved;
}

bool UITreeViewNode::MoveChildDown(eastl::shared_ptr<BaseUITreeViewNode> child )
{
	eastl::list<eastl::shared_ptr<BaseUITreeViewNode>>::iterator itOther;
	eastl::shared_ptr<BaseUITreeViewNode> nodeTmp;
	bool moved = false;

	for( auto itChild = mChildren.begin(); itChild != mChildren.end(); itChild++ )
	{
		if( child == *itChild )
		{
			if( *itChild != mChildren.back() )
			{
				itOther = itChild;
				++itOther;
				nodeTmp = *itChild;
				*itChild = *itOther;
				*itOther = nodeTmp;
				moved = true;
			}
			break;
		}
	}
	return moved;
}

void UITreeViewNode::SetExpanded( bool expanded )
{
	mExpanded = expanded;
}

void UITreeViewNode::SetSelected( bool selected )
{
	if( mOwner )
	{
		UITreeView* owner = reinterpret_cast<UITreeView*>(mOwner.get());
		if (selected)
		{
			owner->mSelected = shared_from_this();
		}
		else
		{
			if (owner->mSelected == shared_from_this())
				owner->mSelected.reset();
		}
	}
}

bool UITreeViewNode::GetSelected() const
{
	if( mOwner )
	{
		UITreeView* owner = reinterpret_cast<UITreeView*>(mOwner.get());
		return owner->mSelected == shared_from_this();
	}
	else return false;
}

bool UITreeViewNode::IsRoot() const
{
	if (mOwner)
	{
		UITreeView* owner = reinterpret_cast<UITreeView*>(mOwner.get());
		return ( shared_from_this() == owner->mRoot );
	}
	else return false;
}

int UITreeViewNode::GetLevel() const
{
	if( mParent )
	{
		return mParent->GetLevel() + 1;
	}
	else
	{
		return 0;
	}
}

bool UITreeViewNode::IsVisible() const
{
	if( mParent )
	{
		return mParent->GetExpanded() && mParent->IsVisible();
	}
	else
	{
		return true;
	}
}


//! constructor
UITreeView::UITreeView(BaseUI* ui, int id, RectangleShape<2, int> rectangle, bool clip, bool drawBack)
	: BaseUITreeView( id, rectangle ), mUI(ui), mRoot(0), mSelected(0), mItemHeight( 0 ), mIndentWidth( 0 ), 
	mTotalItemHeight( 0 ), mTotalItemWidth ( 0 ), mFont( 0 ), mIconFont( 0 ), mScrollBarH( 0 ), mScrollBarV( 0 ), 
	mLastEventNode( 0 ), mLinesVisible( true ), mSelecting( false ), mClip( clip ), mDrawBack( drawBack ), mImageLeftOfIcon( true )
{
	// Create a vertex buffer for a single triangle.
	VertexFormat vformat;
	vformat.Bind(VA_POSITION, DF_R32G32B32_FLOAT, 0);
	vformat.Bind(VA_COLOR, DF_R32G32B32A32_FLOAT, 0);

	eastl::vector<eastl::string> path;
#if defined(_OPENGL_)
	path.push_back("Effects/ColorEffectVS.glsl");
	path.push_back("Effects/ColorEffectPS.glsl");
#else
	path.push_back("Effects/ColorEffectVS.hlsl");
	path.push_back("Effects/ColorEffectPS.hlsl");
#endif
	mEffect = eastl::make_shared<ColorEffect>(ProgramFactory::Get(), path);

	eastl::shared_ptr<VertexBuffer> vbuffer = eastl::make_shared<VertexBuffer>(vformat, 4);
	eastl::shared_ptr<IndexBuffer> ibuffer = eastl::make_shared<IndexBuffer>(IP_TRISTRIP, 2);
	vbuffer->SetUsage(Resource::DYNAMIC_UPDATE);

	// Create the geometric object for drawing.
	mVisual = eastl::make_shared<Visual>(vbuffer, ibuffer, mEffect);
}


//! destructor
UITreeView::~UITreeView()
{

}


//! initialize combobox
void UITreeView::OnInit(bool scrollBarVertical, bool scrollBarHorizontal)
{
	const eastl::shared_ptr<BaseUISkin>& skin = mUI->GetSkin();
	int s = skin->GetSize(DS_SCROLLBAR_SIZE);

	if (scrollBarVertical)
	{
		RectangleShape<2, int> rectangle;
		rectangle.mCenter[0] = (mRelativeRect.mExtent[0] - s) / 2;
		rectangle.mCenter[1] = (mRelativeRect.mExtent[1] - scrollBarHorizontal ? s : 0) / 2;
		rectangle.mExtent[0] = s;
		rectangle.mExtent[1] = mRelativeRect.mExtent[1] - scrollBarHorizontal ? s : 0;

		mScrollBarV.reset(new UIScrollBar(mUI, 0, rectangle, false));
		mScrollBarV->OnInit(!mClip);
		mScrollBarV->SetSubElement(true);
		mScrollBarV->SetPos(0);
	}

	if (scrollBarHorizontal)
	{
		RectangleShape<2, int> rectangle;
		rectangle.mCenter[0] = (mRelativeRect.mExtent[0] - s) / 2;
		rectangle.mCenter[1] = mRelativeRect.mExtent[1] - (s / 2);
		rectangle.mExtent[0] = mRelativeRect.mExtent[0] - s;
		rectangle.mExtent[1] = s;

		mScrollBarH.reset(new UIScrollBar(mUI, 0, rectangle, false));
		mScrollBarH->OnInit(!mClip);
		mScrollBarH->SetSubElement(true);
		mScrollBarH->SetPos(0);
	}

	mRoot.reset(new UITreeViewNode(shared_from_this(), 0));
	UITreeViewNode* root = reinterpret_cast<UITreeViewNode*>(mRoot.get());
	root->mExpanded = true;

	RecalculateItemHeight();
}


void UITreeView::RecalculateItemHeight()
{
	const eastl::shared_ptr<BaseUISkin>& skin = mUI->GetSkin();

	eastl::shared_ptr<BaseUITreeViewNode> node;

	if( mFont != skin->GetFont() )
	{
		mFont = skin->GetFont();
		mItemHeight = 0;

		if( mFont )
			mItemHeight = mFont->GetDimension( L"A" )[1] + 4;

		if( mIconFont )
		{
			int height = mIconFont->GetDimension( L" " )[1];
			if( height > mItemHeight )
				mItemHeight = height;
		}
	}

	mIndentWidth = mItemHeight;
	if( mIndentWidth < 9 )
	{
		mIndentWidth = 9;
	}
	else if( mIndentWidth > 15 )
	{
		mIndentWidth = 15;
	}
	else
	{
		if( ( ( mIndentWidth >> 1 ) << 1 ) - mIndentWidth == 0 )
		{
			--mIndentWidth;
		}
	}

	mTotalItemHeight = 0;
	mTotalItemWidth = mAbsoluteRect.mExtent[0] * 2;
	node = mRoot->GetFrontChild();
	while( node )
	{
		mTotalItemHeight += mItemHeight;
		node = node->GetNextVisible();
	}

	if ( mScrollBarV )
		mScrollBarV->SetMax( eastl::max(0,mTotalItemHeight - mAbsoluteRect.mExtent[1]));

	if ( mScrollBarH )
		mScrollBarH->SetMax( eastl::max(0, mTotalItemWidth - mAbsoluteRect.mExtent[0]));

}

//! called if an event happened.
bool UITreeView::OnEvent( const Event &event )
{
	if ( IsEnabled() )
	{
		switch( event.mEventType )
		{
			case ET_UI_EVENT:
				switch( event.mUIEvent.mEventType )
				{
					case UIEVT_SCROLL_BAR_CHANGED:
						if( event.mUIEvent.mCaller == mScrollBarV.get() || event.mUIEvent.mCaller == mScrollBarH.get() )
						{
							//int pos = ( ( gui::IGUIScrollBar* )event.GUIEvent.Caller )->getPos();
							return true;
						}
						break;
					case UIEVT_ELEMENT_FOCUS_LOST:
						{
							mSelecting = false;
							return false;
						}
						break;
					default:
						break;
				}
				break;
			case ET_MOUSE_INPUT_EVENT:
				{
					Vector2<int> p{ event.mMouseInput.X, event.mMouseInput.Y };

					switch( event.mMouseInput.mEvent )
					{
						case MIE_MOUSE_WHEEL:
							if ( mScrollBarV )
								mScrollBarV->SetPos( mScrollBarV->GetPos() + (event.mMouseInput.mWheel < 0 ? -1 : 1) * -10 );
							return true;
							break;

						case MIE_LMOUSE_PRESSED_DOWN:

							if (mUI->HasFocus(shared_from_this()) && !IsPointInside(p) )
							{
								mUI->RemoveFocus(shared_from_this());
								return false;
							}

							if( mUI->HasFocus(shared_from_this()) &&
								((mScrollBarV && mScrollBarV->GetAbsolutePosition().IsPointInside(p) && mScrollBarV->OnEvent( event ) ) ||
								(mScrollBarH && mScrollBarH->GetAbsolutePosition().IsPointInside(p) && mScrollBarH->OnEvent( event ) )))
							{
								return true;
							}

							mSelecting = true;
							mUI->SetFocus( shared_from_this() );
							return true;
							break;

						case MIE_LMOUSE_LEFT_UP:
							if( mUI->HasFocus(shared_from_this()) &&
								((mScrollBarV && mScrollBarV->GetAbsolutePosition().IsPointInside(p) && mScrollBarV->OnEvent( event ) ) ||
								(mScrollBarH && mScrollBarH->GetAbsolutePosition().IsPointInside(p) && mScrollBarH->OnEvent( event ) )))
							{
								return true;
							}

							mSelecting = false;
							mUI->RemoveFocus(shared_from_this());
							MouseAction( event.mMouseInput.X, event.mMouseInput.Y );
							return true;
							break;

						case MIE_MOUSE_MOVED:
							if( mSelecting )
							{
								if(GetAbsolutePosition().IsPointInside(p) )
								{
									MouseAction( event.mMouseInput.X, event.mMouseInput.Y, true );
									return true;
								}
							}
							break;
						default:
							break;
					}
				}
				break;
			default:
				break;
		}
	}

	return mParent ? mParent->OnEvent( event ) : false;
}

/*!
*/
void UITreeView::MouseAction( int xpos, int ypos, bool onlyHover /*= false*/ )
{
	eastl::shared_ptr<BaseUITreeViewNode> oldSelected = mSelected;
	eastl::shared_ptr<BaseUITreeViewNode> hitNode = 0;
	int selIdx=-1;
	int n;
	eastl::shared_ptr<BaseUITreeViewNode> node;

	Event event;
	event.mEventType = ET_UI_EVENT;
	event.mUIEvent.mCaller = this;
	event.mUIEvent.mElement = 0;

	xpos -= mAbsoluteRect.mCenter[0] - mAbsoluteRect.mExtent[0] / 2;
	ypos -= mAbsoluteRect.mCenter[1] - mAbsoluteRect.mExtent[1] / 2;

	// find new selected item.
	if( mItemHeight != 0 && mScrollBarV )
	{
		selIdx = ( ( ypos - 1 ) + mScrollBarV->GetPos() ) / mItemHeight;
	}

	hitNode = 0;
	node = mRoot->GetFrontChild();
	n = 0;
	while( node )
	{
		if( selIdx == n )
		{
			hitNode = node;
			break;
		}
		node = node->GetNextVisible();
		++n;
	}

	if( hitNode && xpos > hitNode->GetLevel() * mIndentWidth )
	{
		mSelected = hitNode;
	}

	if( hitNode && !onlyHover
		&& xpos < hitNode->GetLevel() * mIndentWidth
		&& xpos > ( hitNode->GetLevel() - 1 ) * mIndentWidth
		&& hitNode->HasChildren() )
	{
		hitNode->SetExpanded( !hitNode->GetExpanded() );

		// post expand/collaps news
		if( hitNode->GetExpanded() )
		{
			event.mUIEvent.mEventType = UIEVT_TREEVIEW_NODE_EXPAND;
		}
		else
		{
			event.mUIEvent.mEventType = UIEVT_TREEVIEW_NODE_COLLAPS;
		}
		mLastEventNode = hitNode;
		mParent->OnEvent( event );
		mLastEventNode = 0;
	}

	if( mSelected && !mSelected->IsVisible() )
	{
		mSelected = 0;
	}

	// post selection news

	if(mParent && !onlyHover && mSelected != oldSelected )
	{
		if( oldSelected )
		{
			event.mUIEvent.mEventType = UIEVT_TREEVIEW_NODE_DESELECT;
			mLastEventNode = oldSelected;
			mParent->OnEvent( event );
			mLastEventNode = 0;
		}
		if( mSelected )
		{
			event.mUIEvent.mEventType = UIEVT_TREEVIEW_NODE_SELECT;
			mLastEventNode = mSelected;
			mParent->OnEvent( event );
			mLastEventNode = 0;
		}
	}
}


//! draws the element and its children
void UITreeView::Draw()
{
	if( !IsVisible() )
	{
		return;
	}

	RecalculateItemHeight(); // if the font changed

	const eastl::shared_ptr<BaseUISkin>& skin = mUI->GetSkin();

	RectangleShape<2, int>* clipRect = 0;
	if( mClip )
		clipRect = &mAbsoluteClippingRect;

	// draw background
	RectangleShape<2, int> frameRect(mAbsoluteRect);

	if( mDrawBack )
		skin->Draw2DRectangle( shared_from_this(), skin->GetColor(DC_3D_HIGH_LIGHT), mVisual, frameRect, clipRect );

	// draw items

	RectangleShape<2, int> clientClip( mAbsoluteRect );

	if (mScrollBarV)
	{
		clientClip.mCenter[0] -= skin->GetSize(DS_SCROLLBAR_SIZE) / 2;
		clientClip.mExtent[0] -= skin->GetSize(DS_SCROLLBAR_SIZE);
	}
	if ( mScrollBarH )
	{
		clientClip.mCenter[1] -= skin->GetSize(DS_SCROLLBAR_SIZE) / 2;
		clientClip.mExtent[1] -= skin->GetSize(DS_SCROLLBAR_SIZE);
	}

	//if( clipRect )
	//	clientClip.clipAgainst( *clipRect );

	frameRect = mAbsoluteRect;
	frameRect.mExtent[0] -= skin->GetSize( DS_SCROLLBAR_SIZE );
	frameRect.mExtent[1] += mItemHeight;

	if ( mScrollBarV )
	{
		frameRect.mExtent[1] -= 2 * mScrollBarV->GetPos();
	}

	if ( mScrollBarH )
	{
		frameRect.mExtent[0] -= 2 * mScrollBarH->GetPos();
	}

	eastl::shared_ptr<BaseUITreeViewNode> node = mRoot->GetFrontChild();
	while( node )
	{
		frameRect.mExtent[0] = mAbsoluteRect.mExtent[0] + 1 + node->GetLevel() * mIndentWidth;

		if (frameRect.mCenter[1] + (int)round(frameRect.mExtent[1] / 2.f) >= 
			mAbsoluteRect.mCenter[1] - (mAbsoluteRect.mCenter[1] / 2) && 
			frameRect.mCenter[1] - (frameRect.mCenter[1] / 2) <= 
			mAbsoluteRect.mCenter[1] + (int)round(mAbsoluteRect.mExtent[1] / 2.f))
		{
			if( node == mSelected )
				skin->Draw2DRectangle( 
					shared_from_this(), skin->GetColor( DC_HIGH_LIGHT ), mVisual, frameRect, &clientClip );

			RectangleShape<2, int> textRect = frameRect;

			if( mFont )
			{
				UIDefaultColor textCol = DC_GRAY_TEXT;
				if ( IsEnabled() )
					textCol = ( node == mSelected ) ? DC_HIGH_LIGHT_TEXT : DC_BUTTON_TEXT;

				int iconWidth = 0;
				for( int n = 0; n < 2; ++n )
				{
					int index = node->GetImageIndex();
					if( ( index >= 0 )
						&& ( ( mImageLeftOfIcon && n == 0 )
						|| ( !mImageLeftOfIcon && n == 1 ) ) )
					{
						index = node->GetSelectedImageIndex();
						if( node != mSelected || index < 0 )
							index = node->GetImageIndex();

						/*
						ImageList->Draw(
							index,
							core::position2d<int>(
							textRect.UpperLeftCorner.X,
							textRect.UpperLeftCorner.Y + ( ( textRect.getHeight() - ImageList->getImageSize().Height ) >> 1 ) ),
							&clientClip );
						iconWidth += ImageList->GetImageSize().Width + 3;
						textRect.UpperLeftCorner.X += ImageList->GetImageSize().Width + 3;
						*/
					}
					else if( ( mIconFont && reinterpret_cast<UITreeViewNode*>( node.get() )->mIcon.size() )
						&& ( ( mImageLeftOfIcon && n == 1 )
						|| ( !mImageLeftOfIcon && n == 0 ) ) )
					{
						mIconFont->Draw( node->GetIcon(), textRect, skin->GetColor(textCol), false, true, &clientClip );
						iconWidth += mIconFont->GetDimension(node->GetIcon())[0] + 3;
						textRect.mCenter[0] += (mIconFont->GetDimension(node->GetIcon())[0] + 3) / 2;
						textRect.mExtent[0] -= mIconFont->GetDimension(node->GetIcon())[0] + 3;
					}
				}

				mFont->Draw( node->GetText(), textRect, skin->GetColor(textCol), false, true, &clientClip );
				
				textRect.mCenter[0] -= iconWidth / 2;
				textRect.mExtent[0] += iconWidth;
			}
		}
		frameRect.mCenter[1] += mItemHeight;

		node = node->GetNextVisible();
	}

	BaseUIElement::Draw();
}

//! Sets the font which should be used as icon font. This font is set to the engine
//! built-in-font by default. Icons can be displayed in front of every list item.
//! An icon is a string, displayed with the icon font. When using the build-in-font of the
//! engine as icon font, the icon strings defined in GUIIcons.h can be used.
void UITreeView::SetIconFont(eastl::shared_ptr<BaseUIFont> font)
{
	mIconFont = font;

	int	height;

	if( mIconFont )
	{
		height = mIconFont->GetDimension( L" " )[1];
		if( height > mItemHeight )
			mItemHeight = height;
	}
}