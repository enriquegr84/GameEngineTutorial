// Copyright (C) 2002-2012 Nikolaus Gebhardt
// This file is part of the "Irrlicht Engine".
// For conditions of distribution and use, see copyright notice in irrlicht.h

#include "UIEditBox.h"

#include "Core/OS/OS.h"

#include "Graphic/Renderer/Renderer.h"
#include "Graphic/Image/ImageResource.h"

//! constructor
UIEditBox::UIEditBox(const wchar_t* text, bool border, BaseUI* ui, int id, RectangleShape<2, int> rectangle)
	: BaseUIEditBox(ui, id, rectangle), mUI(ui), mMouseMarking(false),
	mBorder(border), mBackground(true), mOverrideColorEnabled(false), mMarkBegin(0), mMarkEnd(0),
	mOverrideColor(eastl::array<float, 4>{255 / 255.f, 255 / 255.f, 255 / 255.f, 101 / 255.f}),
	mOverrideFont(0), mLastBreakFont(0), mBlinkStartTime(0), mCursorPos(0), mHScrollPos(0), 
	mVScrollPos(0), mMax(0), mWordWrap(false), mMultiLine(false), mAutoScroll(true), 
	mPasswordBox(false), mPasswordChar(L'*'), mHAlign(UIA_UPPERLEFT), mVAlign(UIA_CENTER), 
	mFrameRect(rectangle)
{
	mText = text;

	eastl::shared_ptr<ResHandle>& resHandle =
		ResCache::Get()->GetHandle(&BaseResource(L"Art/UserControl/appbar.empty.png"));
	if (resHandle)
	{
		const eastl::shared_ptr<ImageResourceExtraData>& extra =
			eastl::static_pointer_cast<ImageResourceExtraData>(resHandle->GetExtra());
		extra->GetImage()->AutogenerateMipmaps();

		// Create a vertex buffer for a two-triangles square. The PNG is stored
		// in left-handed coordinates. The texture coordinates are chosen to
		// reflect the texture in the y-direction.
		VertexFormat vformat;
		vformat.Bind(VA_POSITION, DF_R32G32B32_FLOAT, 0);
		vformat.Bind(VA_TEXCOORD, DF_R32G32_FLOAT, 0);

		eastl::shared_ptr<VertexBuffer> vbuffer = eastl::make_shared<VertexBuffer>(vformat, 4);
		eastl::shared_ptr<IndexBuffer> ibuffer = eastl::make_shared<IndexBuffer>(IP_TRISTRIP, 2);
		vbuffer->SetUsage(Resource::DYNAMIC_UPDATE);

		// Create an effect for the vertex and pixel shaders. The texture is
		// bilinearly filtered and the texture coordinates are clamped to [0,1]^2.
		eastl::vector<eastl::string> path;
#if defined(_OPENGL_)
		path.push_back("Effects/Texture2EffectVS.glsl");
		path.push_back("Effects/Texture2EffectPS.glsl");
#else
		path.push_back("Effects/Texture2EffectVS.hlsl");
		path.push_back("Effects/Texture2EffectPS.hlsl");
#endif
		mEffect = eastl::make_shared<Texture2Effect>(ProgramFactory::Get(), path, extra->GetImage(),
			SamplerState::MIN_L_MAG_L_MIP_P, SamplerState::CLAMP, SamplerState::CLAMP);

		// Create the geometric object for drawing.
		mVisual = eastl::make_shared<Visual>(vbuffer, ibuffer, mEffect);
	}

	// Create a vertex buffer for a single triangle.
	VertexFormat vformat;
	vformat.Bind(VA_POSITION, DF_R32G32B32_FLOAT, 0);
	vformat.Bind(VA_COLOR, DF_R32G32B32A32_FLOAT, 0);

	eastl::shared_ptr<VertexBuffer> vbuffer = eastl::make_shared<VertexBuffer>(vformat, 4);
	eastl::shared_ptr<IndexBuffer> ibuffer = eastl::make_shared<IndexBuffer>(IP_TRISTRIP, 2);
	vbuffer->SetUsage(Resource::DYNAMIC_UPDATE);

	eastl::vector<eastl::string> path;
#if defined(_OPENGL_)
	path.push_back("Effects/ColorEffectVS.glsl");
	path.push_back("Effects/ColorEffectPS.glsl");
#else
	path.push_back("Effects/ColorEffectVS.hlsl");
	path.push_back("Effects/ColorEffectPS.hlsl");
#endif
	mEffectHighlight = eastl::make_shared<ColorEffect>(ProgramFactory::Get(), path);
	mVisualHighlight = eastl::make_shared<Visual>(vbuffer, ibuffer, mEffectHighlight);
}


//! destructor
UIEditBox::~UIEditBox()
{

}

//! initialize editbox
void UIEditBox::OnInit()
{
	// this element can be tabbed to
	SetTabStop(true);
	SetTabOrder(-1);

	CalculateFrameRect();
	BreakText();

	CalculateScrollPos();
}


//! Sets another skin independent font.
void UIEditBox::SetOverrideFont(const eastl::shared_ptr<BaseUIFont>& font)
{
	if (mOverrideFont == font)
		return;

	mOverrideFont = font;
	BreakText();
}

//! Gets the override font (if any)
const eastl::shared_ptr<BaseUIFont>& UIEditBox::GetOverrideFont() const
{
	return mOverrideFont;
}

//! Get the font which is used right now for drawing
const eastl::shared_ptr<BaseUIFont>& UIEditBox::GetActiveFont() const
{
	if ( mOverrideFont )
		return mOverrideFont;

	const eastl::shared_ptr<BaseUISkin>& skin = mUI->GetSkin();
	if (skin)
		return skin->GetFont();

	return nullptr;
}

//! Sets another color for the text.
void UIEditBox::SetOverrideColor(eastl::array<float, 4> color)
{
	mOverrideColor = color;
	mOverrideColorEnabled = true;
}


eastl::array<float, 4> UIEditBox::GetOverrideColor() const
{
	return mOverrideColor;
}


//! Turns the border on or off
void UIEditBox::SetDrawBorder(bool border)
{
	mBorder = border;
}

//! Sets whether to draw the background
void UIEditBox::SetDrawBackground(bool draw)
{
	mBackground = draw;
}

//! Sets if the text should use the overide color or the color in the gui skin.
void UIEditBox::EnableOverrideColor(bool enable)
{
	mOverrideColorEnabled = enable;
}

bool UIEditBox::IsOverrideColorEnabled() const
{
	return mOverrideColorEnabled;
}

//! Enables or disables word wrap
void UIEditBox::SetWordWrap(bool enable)
{
	mWordWrap = enable;
	BreakText();
}

void UIEditBox::UpdateAbsolutePosition()
{
	RectangleShape<2, int> oldAbsoluteRect(mAbsoluteRect);
	BaseUIElement::UpdateAbsolutePosition();

	if ( oldAbsoluteRect != mAbsoluteRect )
	{
		CalculateFrameRect();
		BreakText();
		CalculateScrollPos();
	}
}


//! Checks if word wrap is enabled
bool UIEditBox::IsWordWrapEnabled() const
{
	return mWordWrap;
}


//! Enables or disables newlines.
void UIEditBox::SetMultiLine(bool enable)
{
	mMultiLine = enable;
	BreakText();
}


//! Checks if multi line editing is enabled
bool UIEditBox::IsMultiLineEnabled() const
{
	return mMultiLine;
}


void UIEditBox::SetPasswordBox(bool passwordBox, wchar_t passwordChar)
{
	mPasswordBox = passwordBox;
	if (mPasswordBox)
	{
		mPasswordChar = passwordChar;
		SetMultiLine(false);
		SetWordWrap(false);
		mBrokenText.clear();
	}
}


bool UIEditBox::IsPasswordBox() const
{
	return mPasswordBox;
}


//! Sets text justification
void UIEditBox::SetTextAlignment(UIAlignment horizontal, UIAlignment vertical)
{
	mHAlign = horizontal;
	mVAlign = vertical;
}


//! called if an event happened.
bool UIEditBox::OnEvent(const Event& event)
{
	if (IsEnabled())
	{
		switch(event.mEventType)
		{
		case ET_UI_EVENT:
			if (event.mUIEvent.mEventType == UIEVT_ELEMENT_FOCUS_LOST)
			{
				if (event.mUIEvent.mCaller == this)
				{
					mMouseMarking = false;
					SetTextMarkers(0,0);
				}
			}
			break;
		case ET_KEY_INPUT_EVENT:
			if (ProcessKey(event))
				return true;
			break;
		case ET_MOUSE_INPUT_EVENT:
			if (ProcessMouse(event))
				return true;
			break;
		default:
			break;
		}
	}

	return BaseUIElement::OnEvent(event);
}


bool UIEditBox::ProcessKey(const Event& event)
{
	if (!event.mKeyInput.mPressedDown)
		return false;

	bool textChanged = false;
	int newMarkBegin = mMarkBegin;
	int newMarkEnd = mMarkEnd;

	// control shortcut handling
	if (event.mKeyInput.mControl)
	{
		// german backlash '\' entered with control + '?'
		if (event.mKeyInput.mChar == '\\' )
		{
			InputChar(event.mKeyInput.mChar);
			return true;
		}

		switch(event.mKeyInput.mKey)
		{
			case KEY_KEY_A:
				// select all
				newMarkBegin = 0;
				newMarkEnd = mText.size();
				break;
			case KEY_KEY_C:
				// copy to clipboard
				if (!mPasswordBox && mMarkBegin != mMarkEnd)
				{
					const int realmbgn = mMarkBegin < mMarkEnd ? mMarkBegin : mMarkEnd;
					const int realmend = mMarkBegin < mMarkEnd ? mMarkEnd : mMarkBegin;

					eastl::wstring s;
					s = mText.substr(realmbgn, realmend - realmbgn);
					//Operator->copyToClipboard(s.c_str());
				}
				break;
			case KEY_KEY_X:
				// cut to the clipboard
				if (!mPasswordBox && mMarkBegin != mMarkEnd)
				{
					const int realmbgn = mMarkBegin < mMarkEnd ? mMarkBegin : mMarkEnd;
					const int realmend = mMarkBegin < mMarkEnd ? mMarkEnd : mMarkBegin;

					// copy
					eastl::wstring s;
					s = mText.substr(realmbgn, realmend - realmbgn);
					//Operator->copyToClipboard(s.c_str());

					if (IsEnabled())
					{
						// delete
						eastl::wstring s;
						s = mText.substr(0, realmbgn);
						s.append( mText.substr(realmend, mText.size() - realmend) );
						mText = s;

						mCursorPos = realmbgn;
						newMarkBegin = 0;
						newMarkEnd = 0;
						textChanged = true;
					}
				}
				break;
			case KEY_KEY_V:
			{
				if (!IsEnabled())
					break;

				// paste from the clipboard
				const int realmbgn = mMarkBegin < mMarkEnd ? mMarkBegin : mMarkEnd;
				const int realmend = mMarkBegin < mMarkEnd ? mMarkEnd : mMarkBegin;

				// add new character
				const char* p = nullptr; // = Operator->getTextFromClipboard();
				if (p)
				{
					// TODO: we should have such a function in core::string
					size_t lenOld = strlen(p);
					wchar_t *ws = new wchar_t[lenOld + 1];
					size_t len = mbstowcs(ws, p, lenOld);
					ws[len] = 0;
					eastl::wstring widep(ws);
					delete[] ws;

					if (mMarkBegin == mMarkEnd)
					{
						// insert text
						eastl::wstring s = mText.substr(0, mCursorPos);
						s.append(widep);
						s.append(mText.substr(mCursorPos, mText.size() - mCursorPos));

						if (!mMax || s.size() <= mMax) // thx to Fish FH for fix
						{
							mText = s;
							s = widep;
							mCursorPos += s.size();
						}
					}
					else
					{
						// replace text
						eastl::wstring s = mText.substr(0, realmbgn);
						s.append(widep);
						s.append(mText.substr(realmend, mText.size() - realmend));

						if (!mMax || s.size() <= mMax)  // thx to Fish FH for fix
						{
							mText = s;
							s = widep;
							mCursorPos = realmbgn + s.size();
						}
					}

					newMarkBegin = 0;
					newMarkEnd = 0;
					textChanged = true;
				}
				break;
			}
			case KEY_HOME:
				// move/highlight to start of text
				if (event.mKeyInput.mShift)
				{
					newMarkEnd = mCursorPos;
					newMarkBegin = 0;
					mCursorPos = 0;
				}
				else
				{
					mCursorPos = 0;
					newMarkBegin = 0;
					newMarkEnd = 0;
				}
				break;
			case KEY_END:
				// move/highlight to end of text
				if (event.mKeyInput.mShift)
				{
					newMarkBegin = mCursorPos;
					newMarkEnd = mText.size();
					mCursorPos = 0;
				}
				else
				{
					mCursorPos = mText.size();
					newMarkBegin = 0;
					newMarkEnd = 0;
				}
				break;
			default:
				return false;
		}
	}
	// default keyboard handling
	else
	{
		switch (event.mKeyInput.mKey)
		{
			case KEY_END:
			{
				int p = mText.size();
				if (mWordWrap || mMultiLine)
				{
					p = GetLineFromPos(mCursorPos);
					p = mBrokenTextPositions[p] + mBrokenText[p].size();
					if (p > 0 && (mText[p - 1] == L'\r' || mText[p - 1] == L'\n'))
						p -= 1;
				}

				if (event.mKeyInput.mShift)
				{
					if (mMarkBegin == mMarkEnd)
						newMarkBegin = mCursorPos;

					newMarkEnd = p;
				}
				else
				{
					newMarkBegin = 0;
					newMarkEnd = 0;
				}
				mCursorPos = p;
				mBlinkStartTime = Timer::GetTime();
			}
			break;
			case KEY_HOME:
			{
				int p = 0;
				if (mWordWrap || mMultiLine)
				{
					p = GetLineFromPos(mCursorPos);
					p = mBrokenTextPositions[p];
				}

				if (event.mKeyInput.mShift)
				{
					if (mMarkBegin == mMarkEnd)
						newMarkBegin = mCursorPos;
					newMarkEnd = p;
				}
				else
				{
					newMarkBegin = 0;
					newMarkEnd = 0;
				}
				mCursorPos = p;
				mBlinkStartTime = Timer::GetTime();
			}
			break;
			case KEY_RETURN:
				if (mMultiLine)
				{
					InputChar(L'\n');
				}
				else
				{
					CalculateScrollPos();
					SendUIEvent(UIEVT_EDITBOX_ENTER);
				}
				return true;
			case KEY_LEFT:
			{
				if (event.mKeyInput.mShift)
				{
					if (mCursorPos > 0)
					{
						if (mMarkBegin == mMarkEnd)
							newMarkBegin = mCursorPos;

						newMarkEnd = mCursorPos - 1;
					}
				}
				else
				{
					newMarkBegin = 0;
					newMarkEnd = 0;
				}

				if (mCursorPos > 0) mCursorPos--;
				mBlinkStartTime = Timer::GetTime();
				break;
			}

			case KEY_RIGHT:
			{
				if (event.mKeyInput.mShift)
				{
					if ((int)mText.size() > mCursorPos)
					{
						if (mMarkBegin == mMarkEnd)
							newMarkBegin = mCursorPos;

						newMarkEnd = mCursorPos + 1;
					}
				}
				else
				{
					newMarkBegin = 0;
					newMarkEnd = 0;
				}

				if ((int)mText.size() > mCursorPos) mCursorPos++;
				mBlinkStartTime = Timer::GetTime();
				break;
			}
			case KEY_UP:
				if (mMultiLine || (mWordWrap && mBrokenText.size() > 1))
				{
					int lineNo = GetLineFromPos(mCursorPos);
					int mb = (mMarkBegin == mMarkEnd) ? mCursorPos : (mMarkBegin > mMarkEnd ? mMarkBegin : mMarkEnd);
					if (lineNo > 0)
					{
						int cp = mCursorPos - mBrokenTextPositions[lineNo];
						if ((int)mBrokenText[lineNo - 1].size() < cp)
						{
							mCursorPos =
								mBrokenTextPositions[lineNo - 1] +
								eastl::max((unsigned int)1, (unsigned int)mBrokenText[lineNo - 1].size()) - 1;
						}
						else mCursorPos = mBrokenTextPositions[lineNo - 1] + cp;
					}

					if (event.mKeyInput.mShift)
					{
						newMarkBegin = mb;
						newMarkEnd = mCursorPos;
					}
					else
					{
						newMarkBegin = 0;
						newMarkEnd = 0;
					}
				}
				else
				{
					return false;
				}
				break;
			case KEY_DOWN:
				if (mMultiLine || (mWordWrap && mBrokenText.size() > 1))
				{
					int lineNo = GetLineFromPos(mCursorPos);
					int mb = (mMarkBegin == mMarkEnd) ? mCursorPos : (mMarkBegin < mMarkEnd ? mMarkBegin : mMarkEnd);
					if (lineNo < (int)mBrokenText.size() - 1)
					{
						int cp = mCursorPos - mBrokenTextPositions[lineNo];
						if ((int)mBrokenText[lineNo + 1].size() < cp)
						{
							mCursorPos = mBrokenTextPositions[lineNo + 1] +
								eastl::max((unsigned int)1, (unsigned int)mBrokenText[lineNo + 1].size()) - 1;
						}
						else mCursorPos = mBrokenTextPositions[lineNo + 1] + cp;
					}

					if (event.mKeyInput.mShift)
					{
						newMarkBegin = mb;
						newMarkEnd = mCursorPos;
					}
					else
					{
						newMarkBegin = 0;
						newMarkEnd = 0;
					}
				}
				else
				{
					return false;
				}
				break;

			case KEY_BACK:
				if (!IsEnabled())
					break;

				if (mText.size())
				{
					eastl::wstring s;

					if (mMarkBegin != mMarkEnd)
					{
						// delete marked text
						const int realmbgn = mMarkBegin < mMarkEnd ? mMarkBegin : mMarkEnd;
						const int realmend = mMarkBegin < mMarkEnd ? mMarkEnd : mMarkBegin;

						s = mText.substr(0, realmbgn);
						s.append(mText.substr(realmend, mText.size() - realmend));
						mText = s;

						mCursorPos = realmbgn;
					}
					else
					{
						// delete text behind cursor
						if (mCursorPos>0)
							s = mText.substr(0, mCursorPos - 1);
						else
							s = L"";
						s.append(mText.substr(mCursorPos, mText.size() - mCursorPos));
						mText = s;

						--mCursorPos;
					}

					if (mCursorPos < 0)
						mCursorPos = 0;
					mBlinkStartTime = Timer::GetTime();
					newMarkBegin = 0;
					newMarkEnd = 0;
					textChanged = true;
				}
				break;
			case KEY_DELETE:
				if (!IsEnabled())
					break;

				if (mText.size() != 0)
				{
					eastl::wstring s;

					if (mMarkBegin != mMarkEnd)
					{
						// delete marked text
						const int realmbgn = mMarkBegin < mMarkEnd ? mMarkBegin : mMarkEnd;
						const int realmend = mMarkBegin < mMarkEnd ? mMarkEnd : mMarkBegin;

						s = mText.substr(0, realmbgn);
						s.append(mText.substr(realmend, mText.size() - realmend));
						mText = s;

						mCursorPos = realmbgn;
					}
					else
					{
						// delete text before cursor
						s = mText.substr(0, mCursorPos);
						s.append(mText.substr(mCursorPos + 1, mText.size() - mCursorPos - 1));
						mText = s;
					}

					if (mCursorPos > (int)mText.size())
						mCursorPos = (int)mText.size();

					mBlinkStartTime = Timer::GetTime();
					newMarkBegin = 0;
					newMarkEnd = 0;
					textChanged = true;
				}
				break;

			case KEY_ESCAPE:
			case KEY_TAB:
			case KEY_SHIFT:
			case KEY_F1:
			case KEY_F2:
			case KEY_F3:
			case KEY_F4:
			case KEY_F5:
			case KEY_F6:
			case KEY_F7:
			case KEY_F8:
			case KEY_F9:
			case KEY_F10:
			case KEY_F11:
			case KEY_F12:
			case KEY_F13:
			case KEY_F14:
			case KEY_F15:
			case KEY_F16:
			case KEY_F17:
			case KEY_F18:
			case KEY_F19:
			case KEY_F20:
			case KEY_F21:
			case KEY_F22:
			case KEY_F23:
			case KEY_F24:
				// ignore these keys
				return false;

			default:
				InputChar(event.mKeyInput.mChar);
				return true;
		}
	}

	// Set new text markers
	SetTextMarkers( newMarkBegin, newMarkEnd );

	// break the text if it has changed
	if (textChanged)
	{
		BreakText();
		CalculateScrollPos();
		SendUIEvent(UIEVT_EDITBOX_CHANGED);
	}
	else CalculateScrollPos();

	return true;
}


void UIEditBox::InputChar(wchar_t c)
{
	if (!IsEnabled())
		return;

	if (c != 0)
	{
		if (mText.size() < mMax || mMax == 0)
		{
			eastl::wstring s;

			if (mMarkBegin != mMarkEnd)
			{
				// replace marked text
				const int realmbgn = mMarkBegin < mMarkEnd ? mMarkBegin : mMarkEnd;
				const int realmend = mMarkBegin < mMarkEnd ? mMarkEnd : mMarkBegin;

				s = mText.substr(0, realmbgn);
				s.append(1, c);
				s.append(mText.substr(realmend, mText.size() - realmend));
				mText = s;
				mCursorPos = realmbgn + 1;
			}
			else
			{
				// add new character
				s = mText.substr(0, mCursorPos);
				s.append(1, c);
				s.append(mText.substr(mCursorPos, mText.size() - mCursorPos));
				mText = s;
				++mCursorPos;
			}

			mBlinkStartTime = Timer::GetTime();
			SetTextMarkers(0, 0);
		}
	}
	BreakText();
	CalculateScrollPos();
	SendUIEvent(UIEVT_EDITBOX_CHANGED);
}


//! draws the element and its children
void UIEditBox::Draw()
{
	if (!IsVisible())
		return;

	const bool focus = mUI->HasFocus(shared_from_this());

	eastl::shared_ptr<BaseUISkin> skin = mUI->GetSkin();
	if (!skin)
		return;

	UIDefaultColor bgCol = DC_GRAY_EDITABLE;
	if ( IsEnabled() )
		bgCol = focus ? DC_FOCUSED_EDITABLE : DC_EDITABLE;

	// draw the border
	skin->Draw2DTexture(shared_from_this(), mVisual, mAbsoluteRect, mAbsoluteClippingRect.mExtent / 2);

	if (mBorder)
		CalculateFrameRect();

	RectangleShape<2, int> clipRect = mAbsoluteClippingRect;

	// draw the text
	const eastl::shared_ptr<BaseUIFont>& font = GetActiveFont();

	int cursorLine = 0;

	if (font)
	{
		if (mLastBreakFont != font)
			BreakText();

		// calculate cursor pos
		eastl::wstring *txtLine = &mText;
		int startPos = 0;

		eastl::wstring s, s2;

		// get mark position
		const bool ml = (!mPasswordBox && (mWordWrap || mMultiLine));
		const int realmbgn = mMarkBegin < mMarkEnd ? mMarkBegin : mMarkEnd;
		const int realmend = mMarkBegin < mMarkEnd ? mMarkEnd : mMarkBegin;
		const int hlineStart = ml ? GetLineFromPos(realmbgn) : 0;
		const int hlineCount = ml ? GetLineFromPos(realmend) - hlineStart + 1 : 1;
		const int lineCount = ml ? mBrokenText.size() : 1;

		// Save the override color information.
		// Then, alter it if the edit box is disabled.
		const bool prevOver = mOverrideColorEnabled;
		const eastl::array<float, 4> prevColor = mOverrideColor;

		if (mText.size())
		{
			if (!IsEnabled() && !mOverrideColorEnabled)
			{
				mOverrideColorEnabled = true;
				mOverrideColor = skin->GetColor(DC_GRAY_TEXT);
			}

			for (int i=0; i < lineCount; ++i)
			{
				SetTextRect(i);

				// clipping test - don't draw anything outside the visible area
				//RectangleShape<2, int> c = clipRect;
				//c.ClipAgainst(mCurrentTextRect);
				//if (!c.IsValid())
				//	continue;

				// get current line
				if (mPasswordBox)
				{
					if (mBrokenText.size() != 1)
					{
						mBrokenText.clear();
						mBrokenText.push_back(eastl::wstring());
					}
					if (mBrokenText[0].size() != mText.size())
					{
						mBrokenText[0] = mText;
						for (unsigned int q = 0; q < mText.size(); ++q)
						{
							mBrokenText[0] [q] = mPasswordChar;
						}
					}
					txtLine = &mBrokenText[0];
					startPos = 0;
				}
				else
				{
					txtLine = ml ? &mBrokenText[i] : &mText;
					startPos = ml ? mBrokenTextPositions[i] : 0;
				}

				int cursorWidth = font->GetDimension(L"_")[0];
				int txtWidth = font->GetDimension(txtLine->c_str())[0];
				const bool hasBrokenText = mMultiLine || mWordWrap;
				if (mCurrentTextRect.mExtent[0] <= txtWidth + cursorWidth)
				{
					for (unsigned int txtPosition = 1; txtPosition <= (unsigned int)txtLine->size(); txtPosition++)
					{
						if (mCurrentTextRect.mExtent[0] <= font->GetDimension(txtLine->substr(mHScrollPos, txtPosition).c_str())[0] + cursorWidth)
						{
							txtWidth = hasBrokenText ? txtPosition - mBrokenTextPositions[cursorLine] : txtPosition;
							break;
						}
					}
				}

				// draw normal text
				font->Draw(txtLine->substr(mHScrollPos, txtWidth).c_str(), mCurrentTextRect,
					mOverrideColorEnabled ? mOverrideColor : skin->GetColor(DC_BUTTON_TEXT),
					mHAlign == UIA_CENTER, mVAlign == UIA_CENTER, &clipRect);

				// draw mark and marked text
				if (focus && mMarkBegin != mMarkEnd && 
					i >= hlineStart && i < hlineStart + hlineCount)
				{
					int mbegin = 0, mend = 0;
					int lineStartPos = 0, lineEndPos = txtLine->size();

					if (i == hlineStart)
					{
						// highlight start is on this line
						s = txtLine->substr(0, realmbgn - startPos);
						mbegin = font->GetDimension(s.c_str())[0];

						lineStartPos = realmbgn - startPos;
					}
					if (i == hlineStart + hlineCount - 1)
					{
						// highlight end is on this line
						s2 = txtLine->substr(0, realmend - startPos);
						mend = font->GetDimension(s2.c_str())[0];

						lineEndPos = s2.size();
					}
					else mend = font->GetDimension(txtLine->c_str())[0];

					RectangleShape<2, int> highlightTextRect = mFrameRect;
					highlightTextRect.mCenter[0] -= highlightTextRect.mExtent[0] / 2;
					highlightTextRect.mCenter[0] -= font->GetDimension(txtLine->substr(0, mHScrollPos).c_str())[0];
					highlightTextRect.mCenter[0] += mbegin + (mend - mbegin) / 2;
					highlightTextRect.mExtent[0] = mend - mbegin;

					// draw mark
					skin->Draw2DRectangle(shared_from_this(), skin->GetColor(DC_HIGH_LIGHT), 
						mVisualHighlight, highlightTextRect, &clipRect);

					// draw marked text
					s = txtLine->substr(lineStartPos, lineEndPos - lineStartPos);

					if (s.size())
						font->Draw(s.c_str(), highlightTextRect,
							mOverrideColorEnabled ? mOverrideColor : skin->GetColor(DC_HIGH_LIGHT_TEXT),
							mHAlign == UIA_CENTER, mVAlign == UIA_CENTER, &clipRect);
				}
			}

			// Return the override color information to its previous settings.
			mOverrideColorEnabled = prevOver;
			mOverrideColor = prevColor;
		}

		// draw cursor
		if ( IsEnabled() )
		{
			const bool hasBrokenText = mMultiLine || mWordWrap;
			if (hasBrokenText)
			{
				cursorLine = GetLineFromPos(mCursorPos);
				txtLine = &mBrokenText[cursorLine];
				startPos = mBrokenTextPositions[cursorLine];
			}
			s = txtLine->substr(mHScrollPos, mCursorPos - mHScrollPos - startPos);

			if (focus && (Timer::GetTime() - mBlinkStartTime) % 700 < 350)
			{
				SetTextRect(cursorLine);
				mCurrentTextRect.mCenter[0] += font->GetDimension(s.c_str())[0];

				font->Draw(L"_", mCurrentTextRect,
					mOverrideColorEnabled ? mOverrideColor : skin->GetColor(DC_BUTTON_TEXT),
					mHAlign == UIA_CENTER, mVAlign == UIA_CENTER, &clipRect);
			}
		}
	}

	// draw children
	BaseUIElement::Draw();
}


//! Sets the new caption of this element.
void UIEditBox::SetText(const wchar_t* text)
{
	mText = text;
	if (mCursorPos > (int)mText.size())
		mCursorPos = (int)mText.size();
	mHScrollPos = 0;
	BreakText();
}


//! Enables or disables automatic scrolling with cursor position
//! \param enable: If set to true, the text will move around with the cursor position
void UIEditBox::SetAutoScroll(bool enable)
{
	mAutoScroll = enable;
}


//! Checks to see if automatic scrolling is enabled
//! \return true if automatic scrolling is enabled, false if not
bool UIEditBox::IsAutoScrollEnabled() const
{
	return mAutoScroll;
}


//! Gets the area of the text in the edit box
//! \return Returns the size in pixels of the text
Vector2<int> UIEditBox::GetTextDimension()
{
	RectangleShape<2, int> ret;

	SetTextRect(0);
	ret = mCurrentTextRect;

	for (unsigned int i=1; i < mBrokenText.size(); ++i)
	{
		SetTextRect(i);
		if (mCurrentTextRect.mExtent[0] > ret.mExtent[0])
		{
			ret.mCenter[0] = mCurrentTextRect.mCenter[0];
			ret.mExtent[0] = mCurrentTextRect.mExtent[0];
		}
		if (mCurrentTextRect.mExtent[1] > ret.mExtent[1])
		{
			ret.mCenter[1] = mCurrentTextRect.mCenter[1];
			ret.mExtent[1] = mCurrentTextRect.mExtent[1];
		}
	}

	return ret.mExtent;
}


//! Sets the maximum amount of characters which may be entered in the box.
//! \param max: Maximum amount of characters. If 0, the character amount is
//! infinity.
void UIEditBox::SetMax(unsigned int max)
{
	mMax = max;

	if (mText.size() > mMax && mMax != 0)
		mText = mText.substr(0, mMax);
}


//! Returns maximum amount of characters, previously set by setMax();
unsigned int UIEditBox::GetMax() const
{
	return mMax;
}


bool UIEditBox::ProcessMouse(const Event& event)
{
	switch(event.mMouseInput.mEvent)
	{
		case MIE_LMOUSE_LEFT_UP:
			if (mUI->HasFocus(shared_from_this()))
			{
				mCursorPos = GetCursorPos(event.mMouseInput.X, event.mMouseInput.Y);
				if (mMouseMarking)
					SetTextMarkers( mMarkBegin, mCursorPos );

				mMouseMarking = false;
				CalculateScrollPos();
				return true;
			}
			break;
		case MIE_MOUSE_MOVED:
			{
				if (mMouseMarking)
				{
					mCursorPos = GetCursorPos(event.mMouseInput.X, event.mMouseInput.Y);
					SetTextMarkers( mMarkBegin, mCursorPos );
					CalculateScrollPos();
					return true;
				}
			}
			break;
		case MIE_LMOUSE_PRESSED_DOWN:
			if (!mUI->HasFocus(shared_from_this()))
			{
				mBlinkStartTime = Timer::GetTime();
				mMouseMarking = true;
				mCursorPos = GetCursorPos(event.mMouseInput.X, event.mMouseInput.Y);
				SetTextMarkers(mCursorPos, mCursorPos );
				CalculateScrollPos();
				return true;
			}
			else
			{
				if (!mAbsoluteClippingRect.IsPointInside(
					Vector2<int>{event.mMouseInput.X, event.mMouseInput.Y}))
				{
					return false;
				}
				else
				{
					// move cursor
					mCursorPos = GetCursorPos(event.mMouseInput.X, event.mMouseInput.Y);

					int newMarkBegin = mMarkBegin;
					if (!mMouseMarking)
						newMarkBegin = mCursorPos;

					mMouseMarking = true;
					SetTextMarkers( newMarkBegin, mCursorPos);
					CalculateScrollPos();
					return true;
				}
			}
		default:
			break;
	}

	return false;
}


int UIEditBox::GetCursorPos(int x, int y)
{
	const eastl::shared_ptr<BaseUIFont>& font = GetActiveFont();

	const unsigned int lineCount = (mWordWrap || mMultiLine) ? mBrokenText.size() : 1;

	eastl::wstring *txtLine=0;
	int startPos=0;
	x+=3;

	for (unsigned int i=0; i < lineCount; ++i)
	{
		SetTextRect(i);
		if (i == 0 && y < mCurrentTextRect.mCenter[1] - (mCurrentTextRect.mExtent[1] / 2))
			y = mCurrentTextRect.mCenter[1] - (mCurrentTextRect.mExtent[1] / 2);
		if (i == lineCount - 1 && y > mCurrentTextRect.mCenter[1] + (int)round(mCurrentTextRect.mExtent[1] / 2.f))
			y = mCurrentTextRect.mCenter[1] + (int)round(mCurrentTextRect.mExtent[1] / 2.f);

		// is it inside this region?
		if (y >= mCurrentTextRect.mCenter[1] - (mCurrentTextRect.mExtent[1] / 2) && 
			y <= mCurrentTextRect.mCenter[1] + (int)round(mCurrentTextRect.mExtent[1] / 2.f))
		{
			// we've found the clicked line
			txtLine = (mWordWrap || mMultiLine) ? &mBrokenText[i] : &mText;
			startPos = (mWordWrap || mMultiLine) ? mBrokenTextPositions[i] : 0;
			break;
		}
	}

	if (x <  mCurrentTextRect.mCenter[0] - (mCurrentTextRect.mExtent[0] / 2))
		x = mCurrentTextRect.mCenter[0] - (mCurrentTextRect.mExtent[0] / 2);

	if ( !txtLine )
		return 0;

	const wchar_t* txt = txtLine->c_str();
	int pixel = x - (mCurrentTextRect.mCenter[0] - (mCurrentTextRect.mExtent[0] / 2));

	// click was on or left of the line
	if (pixel <= font->GetDimension(txt)[0])
		for (unsigned int txtPosition = 1; txtPosition <= (unsigned int)txtLine->size(); txtPosition++)
			if (pixel <= font->GetDimension(txtLine->substr(mHScrollPos, txtPosition).c_str())[0])
				return txtPosition + startPos + mHScrollPos;

	// click was off the right edge of the line, go to end.
	return txtLine->size() + startPos + mHScrollPos;
}


//! Breaks the single text line.
void UIEditBox::BreakText()
{
	if ((!mWordWrap && !mMultiLine))
		return;

	mBrokenText.clear(); // need to reallocate :/
	mBrokenTextPositions.resize(0);

	const eastl::shared_ptr<BaseUIFont>& font = GetActiveFont();
	if (!font)
		return;

	mLastBreakFont = font;

	eastl::wstring line;
	eastl::wstring word;
	eastl::wstring whitespace;
	int lastLineStart = 0;
	int size = mText.size();
	int length = 0;
	int elWidth = mRelativeRect.mExtent[0] - 6;
	wchar_t c;

	for (int i=0; i<size; ++i)
	{
		c = mText[i];
		bool lineBreak = false;

		if (c == L'\r') // Mac or Windows breaks
		{
			lineBreak = true;
			c = 0;
			if (mText[i+1] == L'\n') // Windows breaks
			{
				// TODO: I (Michael) think that we shouldn't change the text given by the user for whatever reason.
				// Instead rework the cursor positioning to be able to handle this (but not in stable release
				// branch as users might already expect this behavior).
				mText.erase(i+1);
				--size;
				if ( mCursorPos > i )
					--mCursorPos;
			}
		}
		else if (c == L'\n') // Unix breaks
		{
			lineBreak = true;
			c = 0;
		}

		// don't break if we're not a multi-line edit box
		if (!mMultiLine)
			lineBreak = false;

		if (c == L' ' || c == 0 || i == (size-1))
		{
			// here comes the next whitespace, look if
			// we can break the last word to the next line
			// We also break whitespace, otherwise cursor would vanish beside the right border.
			int whitelgth = font->GetDimension(whitespace.c_str())[0];
			int worldlgth = font->GetDimension(word.c_str())[0];

			if (mWordWrap && length + worldlgth + whitelgth > elWidth && line.size() > 0)
			{
				// break to next line
				length = worldlgth;
				mBrokenText.push_back(line);
				mBrokenTextPositions.push_back(lastLineStart);
				lastLineStart = i - word.size();
				line = word;
			}
			else
			{
				// add word to line
				line += whitespace;
				line += word;
				length += whitelgth + worldlgth;
			}

			word = L"";
			whitespace = L"";

			if ( c )
				whitespace += c;

			// compute line break
			if (lineBreak)
			{
				line += whitespace;
				line += word;
				mBrokenText.push_back(line);
				mBrokenTextPositions.push_back(lastLineStart);
				lastLineStart = i+1;
				line = L"";
				word = L"";
				whitespace = L"";
				length = 0;
			}
		}
		else
		{
			// yippee this is a word..
			word += c;
		}
	}

	line += whitespace;
	line += word;
	mBrokenText.push_back(line);
	mBrokenTextPositions.push_back(lastLineStart);
}

// TODO: that function does interpret VAlign according to line-index (indexed line is placed on top-center-bottom)
// but HAlign according to line-width (pixels) and not by row.
// Intuitively I suppose HAlign handling is better as VScrollPos should handle the line-scrolling.
// But please no one change this without also rewriting (and this time fucking testing!!!) 
// autoscrolling (I noticed this when fixing the old autoscrolling).
void UIEditBox::SetTextRect(int line)
{
	if ( line < 0 )
		return;

	const eastl::shared_ptr<BaseUIFont>& font = GetActiveFont();
	if (!font)
		return;

	Vector2<int> d;

	// get text dimension
	const unsigned int lineCount = (mWordWrap || mMultiLine) ? mBrokenText.size() : 1;

	if (mWordWrap || mMultiLine)
	{
		d = font->GetDimension(mBrokenText[line].c_str());
	}
	else
	{
		d = font->GetDimension(mText.c_str());
		d[1] = mAbsoluteRect.mExtent[1];
	}

	// justification
	mCurrentTextRect = mFrameRect;

	mCurrentTextRect.mCenter[0] += 4;
	mCurrentTextRect.mExtent[1] = d[1];
}


int UIEditBox::GetLineFromPos(int pos)
{
	if (!mWordWrap && !mMultiLine)
		return 0;

	int i=0;
	while (i < (int)mBrokenTextPositions.size())
	{
		if (mBrokenTextPositions[i] > pos)
			return i-1;
		++i;
	}
	return mBrokenTextPositions.size() - 1;
}

// calculate autoscroll
void UIEditBox::CalculateScrollPos()
{
	if (!mAutoScroll)
		return;

	const eastl::shared_ptr<BaseUISkin>& skin = mUI->GetSkin();
	if (!skin)
		return;

	const eastl::shared_ptr<BaseUIFont>& font = mOverrideFont ? mOverrideFont : skin->GetFont();
	if (!font)
		return;

	int cursLine = GetLineFromPos(mCursorPos);
	if ( cursLine < 0 )
		return;

	SetTextRect(cursLine);
	const bool hasBrokenText = mMultiLine || mWordWrap;

	// Check horizonal scrolling
	// NOTE: Calculations different to vertical scrolling because setTextRect 
	// interprets VAlign relative to line but HAlign not relative to row
	{
		// get cursor position
		const eastl::shared_ptr<BaseUIFont>& font = GetActiveFont();
		if (!font)
			return;

		// get cursor area
		int cursorWidth = font->GetDimension(L"_")[0];
		eastl::wstring *txtLine = hasBrokenText ? &mBrokenText[cursLine] : &mText;
		int cPos = hasBrokenText ? mCursorPos - mBrokenTextPositions[cursLine] : mCursorPos;	// column
		mHScrollPos = mHScrollPos > cPos ? cPos : mHScrollPos;
		int cStart = font->GetDimension(txtLine->substr(mHScrollPos, cPos - mHScrollPos).c_str())[0]; // pixels from text-start
		int cEnd = cStart + cursorWidth;

		if (mCurrentTextRect.mCenter[0] - (mCurrentTextRect.mExtent[0] / 2) + cStart < 
			mFrameRect.mCenter[0] - (mFrameRect.mExtent[0] / 2))
		{
			// cursor to the left of the clipping area
			int scrollPos = 0;
			if (mCurrentTextRect.mExtent[0] <= font->GetDimension(txtLine->c_str())[0] + cursorWidth)
				scrollPos = cPos;

			mHScrollPos = scrollPos;
			SetTextRect(cursLine);

			// TODO: should show more characters to the left when we're scrolling left
			//	and the cursor reaches the border.
		}
		else if (mCurrentTextRect.mCenter[0] - (mCurrentTextRect.mExtent[0] / 2) + cEnd > 
			mFrameRect.mCenter[0] + (int)round(mFrameRect.mExtent[0] / 2.f))
		{
			// cursor to the right of the clipping area
			int scrollPos = 0;
			if (mCurrentTextRect.mExtent[0] <= font->GetDimension(txtLine->c_str())[0] + cursorWidth)
				for (scrollPos = mHScrollPos; scrollPos < (int)txtLine->size(); scrollPos++)
					if (mCurrentTextRect.mExtent[0] > font->GetDimension(txtLine->substr(scrollPos, cPos - scrollPos).c_str())[0] + cursorWidth)
						break;

			mHScrollPos = scrollPos;
			SetTextRect(cursLine);
		}
		else
		{
			// TODO: Needs a clean left and right gap removal depending on HAlign, similar to vertical 
			// scrolling tests for top/bottom. This check just fixes the case where it was most noticable 
			// (text smaller than clipping area).
			int scrollPos = mHScrollPos;
			if (mCurrentTextRect.mExtent[0] <= cEnd)
				for (scrollPos = mHScrollPos+1; scrollPos < (int)txtLine->size(); scrollPos++)
					if (mCurrentTextRect.mExtent[0] > font->GetDimension(txtLine->substr(scrollPos, cPos - scrollPos).c_str())[0] + cursorWidth)
						break;

			mHScrollPos = scrollPos;
			SetTextRect(cursLine);
		}
	}

	// calculate vertical scrolling
	if (hasBrokenText)
	{
		int lineHeight = font->GetDimension(L"A")[1];
		// only up to 1 line fits?
		if ( lineHeight >= mFrameRect.mExtent[1] )
		{
			mVScrollPos = 0;
			SetTextRect(cursLine);
			int unscrolledPos = mCurrentTextRect.mCenter[1] - (mCurrentTextRect.mExtent[1] / 2);
			int pivot = mFrameRect.mCenter[1] - (mFrameRect.mExtent[1] / 2);
			switch (mVAlign)
			{
				case UIA_CENTER:
					pivot += mFrameRect.mExtent[1] / 2;
					unscrolledPos += lineHeight / 2;
					break;
				case UIA_LOWERRIGHT:
					pivot += mFrameRect.mExtent[1];
					unscrolledPos += lineHeight;
					break;
				default:
					break;
			}
			mVScrollPos = unscrolledPos-pivot;
			SetTextRect(cursLine);
		}
		else
		{
			// First 2 checks are necessary when people delete lines
			SetTextRect(0);
			if (mCurrentTextRect.mCenter[1] - (mCurrentTextRect.mExtent[1] / 2) > 
				mFrameRect.mCenter[1] - (mFrameRect.mExtent[1] / 2) &&
				mVAlign != UIA_LOWERRIGHT)
			{
				// first line is leaving a gap on top
				mVScrollPos = 0;
			}
			else if (mVAlign != UIA_UPPERLEFT)
			{
				unsigned int lastLine = mBrokenTextPositions.empty() ? 0 : mBrokenTextPositions.size()-1;
				SetTextRect(lastLine);
				if ( mCurrentTextRect.mCenter[1] + (int)round(mCurrentTextRect.mExtent[1] / 2.f) < 
					 mFrameRect.mCenter[1] + (int)round(mFrameRect.mExtent[1] / 2.f) )
				{
					// last line is leaving a gap on bottom
					mVScrollPos -= mCurrentTextRect.mExtent[1];
				}
			}

			SetTextRect(cursLine);
			if (mCurrentTextRect.mCenter[1] - (mCurrentTextRect.mExtent[1] / 2) <
				mFrameRect.mCenter[1] - (mFrameRect.mExtent[1] / 2))
			{
				// text above valid area
				mVScrollPos -= mCurrentTextRect.mExtent[1];
				SetTextRect(cursLine);
			}
			if (mCurrentTextRect.mCenter[1] + (int)round(mCurrentTextRect.mExtent[1] / 2.f) >
				mFrameRect.mCenter[1] + (int)round(mFrameRect.mExtent[1] / 2.f))
			{
				// text below valid area
				mVScrollPos += mCurrentTextRect.mExtent[1];
				SetTextRect(cursLine);
			}
		}
	}
}

void UIEditBox::CalculateFrameRect()
{
	mFrameRect = mAbsoluteRect;

	const eastl::shared_ptr<BaseUISkin>& skin = mUI->GetSkin();
	if (!skin)
		return;

	if (mBorder)
	{
		mFrameRect.mExtent[0] += 2 * (skin->GetSize(DS_TEXT_DISTANCE_X)+1);
		mFrameRect.mExtent[1] += 2 * (skin->GetSize(DS_TEXT_DISTANCE_Y)+1);
	}
}

//! set text markers
void UIEditBox::SetTextMarkers(int begin, int end)
{
	if ( begin != mMarkBegin || end != mMarkEnd )
	{
		mMarkBegin = begin;
		mMarkEnd = end;
		SendUIEvent(UIEVT_EDITBOX_MARKING_CHANGED);
	}
}

//! send some gui event to parent
void UIEditBox::SendUIEvent(UIEventType type)
{
	if ( mParent )
	{
		Event e;
		e.mEventType = ET_UI_EVENT;
		e.mUIEvent.mCaller = this;
		e.mUIEvent.mElement = 0;
		e.mUIEvent.mEventType = type;

		mParent->OnEvent(e);
	}
}