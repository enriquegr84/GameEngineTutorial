// Copyright (C) 2002-2012 Nikolaus Gebhardt
// This file is part of the "Irrlicht Engine".
// For conditions of distribution and use, see copyright notice in irrlicht.h

#include "UIFont.h"
//#include "UISpriteBank.h"

#include "UserInterface.h"

#include "Core/OS/OS.h"
#include "Core/IO/ReadFile.h"
#include "Core/IO/XmlResource.h"

#include "Graphic/Renderer/Renderer.h"
//#include "Graphic/Image/ImageLoader.h"

//! constructor
UIFont::UIFont(BaseUI* ui, const eastl::wstring& filename)
:	mSpriteBank(0), mWrongCharacter(0), mMaxHeight(0), 
	mGlobalKerningWidth(0), mGlobalKerningHeight(0), mUI(ui)
{
	#ifdef _DEBUG
	//setDebugName("GUIFont");
	#endif
	/*
	if (mUI)
	{
		mSpriteBank = mUI->GetSpriteBank(filename);
		if (!mSpriteBank)	// could be default-font which has no file
			mSpriteBank = mUI->AddEmptySpriteBank(filename);
	}
	else mSpriteBank.reset(new UISpriteBank(NULL));
	*/
	SetInvisibleCharacters ( L" " );
}


//! destructor
UIFont::~UIFont()
{
	// TODO: spritebank still exists in gui-environment and should be Removed here when it's
	// reference-count is 1. Just can't do that from here at the moment.
	// But spritebank would not be able to drop textures anyway because those are in texture-cache
	// where they can't be Removed unless materials start reference-couting 'em.
}


//! loads a font file from xml
bool UIFont::Load(XMLElement* pRoot)
{
	if (!mSpriteBank || !Renderer::Get())
		return false;

	//mSpriteBank->Clear();

	LogAssert(pRoot, "Root element is null");
	/*
	// load all materials
	XMLElement* pParentNode = pRoot->FirstChildElement("Texture");
	if (pParentNode)
	{
		XMLElement* pNode = pParentNode->FirstChildElement(); 
		
		int index = 0;

		eastl::string fn(pNode->Attribute("filename"));
		pNode = pNode->NextSiblingElement();
		pNode->Attribute("index", &index);
		pNode = pNode->NextSiblingElement();
		eastl::wstring alpha(pNode->Attribute("hasAlpha"));

		// add a texture
		while (index+1 > (int)mSpriteBank->GetTextureCount())
			mSpriteBank->AddTexture(0);

		// disable mipmaps+filtering
		bool mipmap = Renderer::Get()->GetTextureCreationFlag(TCF_CREATE_MIP_MAPS);
		Renderer::Get()->SetTextureCreationFlag(TCF_CREATE_MIP_MAPS, false);
		
		mSpriteBank->SetTexture(index, Renderer::Get()->GetTexture(fn) );

		// set previous mip-map+filter state
		Renderer::Get()->SetTextureCreationFlag(TCF_CREATE_MIP_MAPS, mipmap);

		// couldn't load texture, abort.
		if (!mSpriteBank->GetTexture(index))
		{
			LogError("Unable to load all textures in the font, aborting");
			return false;
		}
		else
		{
			// colorkey texture rather than alpha channel?
			if (alpha == eastl::wstring("false"))
				Renderer::Get()->MakeColorKeyTexture(
					mSpriteBank->GetTexture(index), Vector2<int>{0, 0});
		}
	}
	else
	{
		pParentNode = pRoot->FirstChildElement("c");
		if (pParentNode)
		{
			XMLElement* pNode = pParentNode->FirstChildElement(); 

			// adding a character to this font
			FontArea a;
			UISpriteFrame f;
			UISprite s;
			RectangleBase<2, int> rectangle;
			int texno;

			pNode->Attribute("u", &a.underhang);
			pNode = pNode->NextSiblingElement();
			pNode->Attribute("o", &a.overhang);
			pNode = pNode->NextSiblingElement();
			pNode->Attribute("i", &texno);
			pNode = pNode->NextSiblingElement();

			a.spriteno = mSpriteBank->GetSprites().size();

			// parse rectangle
			eastl::vector<wchar_t> chArray;

			eastl::string rectstr(pNode->Attribute("r"));
			pNode = pNode->NextSiblingElement();
			pNode->Attribute("c", (int *)&chArray[0]);
			wchar_t ch = chArray[0];

			const char *c = rectstr.c_str();
			int val;
			val = 0;
			while (*c >= '0' && *c <= '9')
			{
				val *= 10;
				val += *c - '0';
				c++;
			}
			rectangle.UpperLeftCorner.X = val;
			while (*c == L' ' || *c == L',') c++;

			val = 0;
			while (*c >= '0' && *c <= '9')
			{
				val *= 10;
				val += *c - '0';
				c++;
			}
			rectangle.UpperLeftCorner.Y = val;
			while (*c == L' ' || *c == L',') c++;

			val = 0;
			while (*c >= '0' && *c <= '9')
			{
				val *= 10;
				val += *c - '0';
				c++;
			}
			rectangle.LowerRightCorner.X = val;
			while (*c == L' ' || *c == L',') c++;

			val = 0;
			while (*c >= '0' && *c <= '9')
			{
				val *= 10;
				val += *c - '0';
				c++;
			}
			rectangle.LowerRightCorner.Y = val;

			mCharacterMap.insert(eastl::make_pair(ch,mAreas.size()));

			// make frame
			f.rectNumber = mSpriteBank->GetPositions().size();
			f.textureNumber = texno;

			// add frame to sprite
			s.Frames.push_back(f);
			s.frameTime = 0;

			// add rectangle to sprite bank
			mSpriteBank->GetPositions().push_back(rectangle);
			a.width = rectangle.GetWidth();

			// add sprite to sprite bank
			mSpriteBank->GetSprites().push_back(s);

			// add character to font
			mAreas.push_back(a);
		}
	}

	// set bad character
	mWrongCharacter = GetAreaFromCharacter(L' ');

	SetMaxHeight();
	*/
	return true;
}


void UIFont::SetMaxHeight()
{
	if ( !mSpriteBank )
		return;
	/*
	mMaxHeight = 0;
	int t;

	eastl::vector< RectangleBase<2, int> >& p = mSpriteBank->GetPositions();

	for (unsigned int i=0; i<p.size(); ++i)
	{
		t = p[i].extent[1];
		if (t>mMaxHeight)
			mMaxHeight = t;
	}
	*/
}


//! loads a font file, native file needed, for texture parsing
bool UIFont::Load(const eastl::wstring& filename)
{
	BaseResource resource(filename);
	eastl::shared_ptr<ResHandle> fontResource = ResCache::Get()->GetHandle(&resource);
	if (fontResource->GetExtra()->ToString() == L"ImageResourceExtraData")
	{
		/*
		eastl::shared_ptr<ImageResourceExtraData> extra = 
			eastl::static_pointer_cast<ImageResourceExtraData>(fontResource->GetExtra());
		return LoadTexture(extra->GetImage(), filename);
		*/
	}
	else if (fontResource->GetExtra()->ToString() == L"XmlResourceExtraData")
	{
		eastl::shared_ptr<XmlResourceExtraData> extra = 
			eastl::static_pointer_cast<XmlResourceExtraData>(fontResource->GetExtra());
		return Load(extra->GetRoot());
	}
	return false;
}


//! load & prepare font from ITexture
bool UIFont::LoadTexture(Texture* image, const eastl::wstring& name)
{
	if (!image || !mSpriteBank || !Renderer::Get())
		return false;

	bool ret = false;
	/*
	int lowerRightPositions = 0;

	Texture* tmpImage(image);
	bool deleteTmpImage=false;
	switch(image->GetColorFormat())
	{
		case CF_R5G6B5:
			tmpImage = Renderer::Get()->CreateImage(CF_A1R5G5B5,image->GetDimension());
			image->CopyTo(tmpImage);
			deleteTmpImage=true;
			break;
		case CF_A1R5G5B5:
		case CF_A8R8G8B8:
			break;
		case CF_R8G8B8:
			tmpImage = Renderer::Get()->CreateImage(CF_A8R8G8B8,image->GetDimension());
			image->CopyTo(tmpImage);
			deleteTmpImage=true;
			break;
		default:
			LogError("Unknown texture format provided for GUIFont::loadTexture");
			return false;
	}
	ReadPositions(tmpImage, lowerRightPositions);

	mWrongCharacter = GetAreaFromCharacter(L' ');

	// output warnings
	if (!lowerRightPositions || !mSpriteBank->GetSprites().size())
		LogError("	Either no upper or lower corner pixels in the font file. \
					If this font was made using the new font tool, please load \
					the XML file instead. If not, the font may be corrupted.");
	else if (lowerRightPositions != (int)mSpriteBank->GetPositions().size())
		LogError("	The amount of upper corner pixels and the lower corner pixels \
					is not equal, font file may be corrupted.");

	bool ret = ( !mSpriteBank->GetSprites().empty() && lowerRightPositions );

	if ( ret )
	{
		bool flag[2];
		flag[0] = Renderer::Get()->GetTextureCreationFlag ( TCF_ALLOW_NON_POWER_2 );
		flag[1] = Renderer::Get()->GetTextureCreationFlag ( TCF_CREATE_MIP_MAPS );

		Renderer::Get()->SetTextureCreationFlag(TCF_ALLOW_NON_POWER_2, true);
		Renderer::Get()->SetTextureCreationFlag(TCF_CREATE_MIP_MAPS, false );

		mSpriteBank->AddTexture(renderer->AddTexture(name, tmpImage));

		Renderer::Get()->SetTextureCreationFlag(TCF_ALLOW_NON_POWER_2, flag[0] );
		Renderer::Get()->SetTextureCreationFlag(TCF_CREATE_MIP_MAPS, flag[1] );
	}
	if (deleteTmpImage)
		delete tmpImage;

	SetMaxHeight();
	*/
	return ret;
}


void UIFont::ReadPositions(Texture* image, int& lowerRightPositions)
{
	if (!mSpriteBank )
		return;
	/*
	const Vector2<unsigned int> size = image->GetDimension();

	eastl::array<float, 4> colorTopLeft = image->GetPixel(0,0);
	colorTopLeft.SetAlpha(1.f);
	image->SetPixel(0,0,colorTopLeft);
	Color colorLowerRight = image->GetPixel(1,0);
	Color colorBackGround = image->GetPixel(2,0);
	Color colorBackGroundTransparent = 0;

	image->SetPixel(1,0,colorBackGround);

	// start parsing

	Vector2<int> pos{ 0,0 };
	for (pos.Y=0; pos[1]<(int)size.Height; ++pos.Y)
	{
		for (pos.X=0; pos[0]<(int)size.Width; ++pos.X)
		{
			const eastl::array<float, 4> c = image->GetPixel(pos.X, pos.Y);
			if (c == colorTopLeft)
			{
				image->SetPixel(pos.X, pos.Y, colorBackGroundTransparent);
				mSpriteBank->GetPositions().push_back(RectangleBase<2, int>(pos, pos));
			}
			else
			if (c == colorLowerRight)
			{
				// too many lower right points
				if (mSpriteBank->GetPositions().size()<=(unsigned int)lowerRightPositions)
				{
					lowerRightPositions = 0;
					return;
				}

				image->SetPixel(pos.X, pos.Y, colorBackGroundTransparent);
				mSpriteBank->GetPositions()[lowerRightPositions].LowerRightCorner = pos;
				// add frame to sprite bank
				UISpriteFrame f;
				f.rectNumber = lowerRightPositions;
				f.textureNumber = 0;
				UISprite s;
				s.Frames.push_back(f);
				s.frameTime = 0;
				mSpriteBank->GetSprites().push_back(s);
				// add character to font
				FontArea a;
				a.overhang = 0;
				a.underhang = 0;
				a.spriteno = lowerRightPositions;
				a.width = mSpriteBank->GetPositions()[lowerRightPositions].GetWidth();
				mAreas.push_back(a);
				// map letter to character
				wchar_t ch = (wchar_t)(lowerRightPositions + 32);
				mCharacterMap[ch] = lowerRightPositions;

				++lowerRightPositions;
			}
			else
			if (c == colorBackGround)
				image->SetPixel(pos.X, pos.Y, colorBackGroundTransparent);
		}
	}
	*/
}


//! set an Pixel Offset on drawing ( scale position on width )
void UIFont::SetKerningWidth(int kerning)
{
	mGlobalKerningWidth = kerning;
}


//! set an Pixel Offset on drawing ( scale position on width )
int UIFont::GetKerningWidth(const wchar_t* thisLetter, const wchar_t* previousLetter)
{
	int ret = mGlobalKerningWidth;

	if (thisLetter)
	{
		ret += mAreas[GetAreaFromCharacter(*thisLetter)].overhang;

		if (previousLetter)
		{
			ret += mAreas[GetAreaFromCharacter(*previousLetter)].underhang;
		}
	}

	return ret;
}


//! set an Pixel Offset on drawing ( scale position on height )
void UIFont::SetKerningHeight(int kerning)
{
	mGlobalKerningHeight = kerning;
}


//! set an Pixel Offset on drawing ( scale position on height )
int UIFont::GetKerningHeight ()
{
	return mGlobalKerningHeight;
}


//! returns the sprite number from a given character
unsigned int UIFont::GetSpriteNoFromChar(const wchar_t *c)
{
	return mAreas[GetAreaFromCharacter(*c)].spriteno;
}


int UIFont::GetAreaFromCharacter(const wchar_t c)
{
	eastl::map<wchar_t, int>::iterator itChar = mCharacterMap.find(c);

	if (itChar != mCharacterMap.end())
		return itChar->second;
	else
		return mWrongCharacter;
}

void UIFont::SetInvisibleCharacters( const wchar_t *s )
{
	mInvisible = s;
}


//! returns the dimension of text
Vector2<unsigned int> UIFont::GetDimension(const wchar_t* text)
{
	Vector2<unsigned int> dim{ 0, 0 };
	Vector2<unsigned int> thisLine{ 0, (unsigned int)mMaxHeight };

	for (const wchar_t* p = text; *p; ++p)
	{
		bool lineBreak=false;
		if (*p == L'\r') // Mac or Windows breaks
		{
			lineBreak = true;
			if (p[1] == L'\n') // Windows breaks
				++p;
		}
		else if (*p == L'\n') // Unix breaks
		{
			lineBreak = true;
		}
		if (lineBreak)
		{
			dim[1] += thisLine[1];
			if (dim[0] < thisLine[0])
				dim[0] = thisLine[0];
			thisLine[0] = 0;
			continue;
		}

		const FontArea &area = mAreas[GetAreaFromCharacter(*p)];

		thisLine[0] += area.underhang;
		thisLine[0] += area.width + area.overhang + mGlobalKerningWidth;
	}

	dim[1] += thisLine[1];
	if (dim[0] < thisLine[0])
		dim[0] = thisLine[0];

	return dim;
}

//! draws some text and clips it to the specified rectangle if wanted
void UIFont::Draw(const eastl::wstring& text, const RectangleBase<2, int>& position,
	eastl::array<float, 4> color, bool hcenter, bool vcenter, const RectangleBase<2, int>* clip)
{
	if (!Renderer::Get() || !mSpriteBank)
		return;

	// NOTE: don't make this u32 or the >> later on can fail when the dimension width is < position width
	Vector2<unsigned int> textDimension;
	Vector2<int> offset = position.center;

	if (hcenter || vcenter || clip)
		textDimension = GetDimension(text.c_str());

	if (hcenter)
		offset[0] += (position.extent[0] - textDimension[0]) >> 1;

	if (vcenter)
		offset[1] += (position.extent[1] - textDimension[1]) >> 1;

	if (clip)
	{
		RectangleBase<2, int> clippedRect;
		clippedRect.center = offset;
		clippedRect.extent[0] = textDimension[0];
		clippedRect.extent[1] = textDimension[1];
		/*
		clippedRect.ClipAgainst(*clip);
		if (!clippedRect.IsValid())
			return;
		*/
	}

	eastl::vector<unsigned int> indices(text.size());
	eastl::vector<Vector2<int>> offsets(text.size());

	for(unsigned int i = 0;i < text.size();i++)
	{
		wchar_t c = text[i];

		bool lineBreak=false;
		if ( c == L'\r') // Mac or Windows breaks
		{
			lineBreak = true;
			if ( text[i + 1] == L'\n') // Windows breaks
				c = text[++i];
		}
		else if ( c == L'\n') // Unix breaks
		{
			lineBreak = true;
		}

		if (lineBreak)
		{
			offset[1] += mMaxHeight;
			offset[0] = position.center[0];

			if ( hcenter )
			{
				offset[0] += (position.extent[0] - textDimension[0]) >> 1;
			}
			continue;
		}

		FontArea& area = mAreas[GetAreaFromCharacter(c)];

		offset[0] += area.underhang;
		/*
		if ( mInvisible.findFirst ( c ) < 0 )
		{
			indices.push_back(area.spriteno);
			offsets.push_back(offset);
		}
		*/

		offset[0] += area.width + area.overhang + mGlobalKerningWidth;
	}

	//mSpriteBank->Draw2DSpriteBatch(indices, offsets, clip, color);
}


//! Calculates the index of the character in the text which is on a specific position.
int UIFont::GetCharacterFromPos(const wchar_t* text, int pixelX)
{
	int x = 0;
	int idx = 0;

	while (text[idx])
	{
		const FontArea& a = mAreas[GetAreaFromCharacter(text[idx])];

		x += a.width + a.overhang + a.underhang + mGlobalKerningWidth;

		if (x >= pixelX)
			return idx;

		++idx;
	}

	return -1;
}


const eastl::shared_ptr<BaseUISpriteBank>& UIFont::GetSpriteBank() const
{
	return mSpriteBank;
}