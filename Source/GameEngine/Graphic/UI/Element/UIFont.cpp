// Copyright (C) 2002-2012 Nikolaus Gebhardt
// This file is part of the "Irrlicht Engine".
// For conditions of distribution and use, see copyright notice in irrlicht.h

#include "UIFont.h"

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


// Font widht and height info
Vector2<int> UIFont::GetDimension(eastl::wstring const& message) const
{
	return mFont->GetDimension(message);
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
		return true;
	}
	else if (fontResource->GetExtra()->ToString() == L"XmlResourceExtraData")
	{
		eastl::shared_ptr<XmlResourceExtraData> extra = 
			eastl::static_pointer_cast<XmlResourceExtraData>(fontResource->GetExtra());
		return true;
	}
	return false;
}


//! draws some text and clips it to the specified rectangle if wanted
void UIFont::Draw(const eastl::wstring& text, const RectangleShape<2, int>& position,
	eastl::array<float, 4> const& color, bool hcenter, bool vcenter, const RectangleShape<2, int>* clip)
{
	if (!Renderer::Get())
		return;

	Vector2<int> offset = position.center - (position.extent / 2);

	if (hcenter)
		offset[0] = position.center[0] + (int)round(position.extent[0] / 2.f);

	if (vcenter)
		offset[1] = position.center[1] + (int)round(position.extent[1] / 2.f);

	Renderer::Get()->Draw(offset[0], offset[1], color, text);
}