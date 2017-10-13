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
#include "Graphic/Image/ImageResource.h"

//! constructor
UIFont::UIFont(BaseUI* ui, eastl::wstring fileName, const eastl::shared_ptr<Font> font)
:	mUI(ui), mFont(font)
{
	#ifdef _DEBUG
	//setDebugName("UIFont");
	#endif

	if (mUI)
	{
		mSpriteBank = mUI->GetSpriteBank(fileName);
		if (!mSpriteBank)	// could be default-font which has no file
			mSpriteBank = mUI->AddEmptySpriteBank(fileName);
	}
}

//! constructor
UIFont::UIFont(BaseUI* ui, eastl::wstring fileName)
	: mUI(ui), mFont(0)
{
	#ifdef _DEBUG
		//setDebugName("UIFont");
	#endif

	if (mUI)
	{
		mSpriteBank = mUI->GetSpriteBank(fileName);
		if (!mSpriteBank)	// could be default-font which has no file
			mSpriteBank = mUI->AddEmptySpriteBank(fileName);
	}
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
	if (!Renderer::Get())
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


//! loads a font file, native file needed, for texture parsing
bool UIFont::Load(const eastl::wstring& filename)
{
	BaseResource resource(filename);
	eastl::shared_ptr<ResHandle> fontResource = ResCache::Get()->GetHandle(&resource);
	if (fontResource->GetExtra()->ToString() == L"ImageResourceExtraData")
	{
		eastl::shared_ptr<ImageResourceExtraData> extra = 
			eastl::static_pointer_cast<ImageResourceExtraData>(fontResource->GetExtra());
		extra->GetImage()->AutogenerateMipmaps();

		return LoadTexture(extra->GetImage().get(), filename);
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
	if (!image || !Renderer::Get())
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


//! draws some text and clips it to the specified rectangle if wanted
void UIFont::Draw(const eastl::wstring& text, const RectangleBase<2, int>& position,
	eastl::array<float, 4> const& color, bool hcenter, bool vcenter, const RectangleBase<2, int>* clip)
{
	if (!Renderer::Get())
		return;

	Vector2<int> offset;
	offset[0] = position.center[0] - (position.extent[0] / 2);
	offset[1] = position.center[1];

	if (hcenter)
		offset[0] = position.center[0];

	if (vcenter)
		offset[1] = position.center[1] + (int)round(position.extent[1] / 2.f);

	Renderer::Get()->Draw(offset[0], offset[1], color, text);
}