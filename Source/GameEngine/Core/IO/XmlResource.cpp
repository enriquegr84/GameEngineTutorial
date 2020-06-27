//========================================================================
// XmlResource.cpp : API to use load Xml files from the Resource Cache
//
// Part of the GameEngine Application
//
// GameEngine is the sample application that encapsulates much of the source code
// discussed in "Game Coding Complete - 4th Edition" by Mike McShaffry and David
// "Rez" Graham, published by Charles River Media. 
// ISBN-10: 1133776574 | ISBN-13: 978-1133776574
//
// If this source code has found it's way to you, and you think it has helped you
// in any way, do the authors a favor and buy a new copy of the book - there are 
// detailed explanations in it that compliment this code well. Buy a copy at Amazon.com
// by clicking here: 
//    http://www.amazon.com/gp/product/1133776574/ref=olp_product_details?ie=UTF8&me=&seller=
//
// There's a companion web site at http://www.mcshaffry.com/GameCode/
// 
// The source code is managed and maintained through Google Code: 
//    http://code.google.com/p/GameEngine/
//
// (c) Copyright 2012 Michael L. McShaffry and David Graham
//
// This program is free software; you can redistribute it and/or
// modify it under the terms of the GNU Lesser GPL v3
// as published by the Free Software Foundation.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See 
// http://www.gnu.org/licenses/lgpl-3.0.txt for more details.
//
// You should have received a copy of the GNU Lesser GPL v3
// along with this program; if not, write to the Free Software
// Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
//
//========================================================================

#include "XmlResource.h"

void XmlResourceExtraData::ParseXml(char* pRawBuffer)
{
	mXmlDocument.Parse(pRawBuffer);
}

//! returns true if the file maybe is able to be loaded by this class
//! based on the file extension (e.g. ".xml")
bool XmlResourceLoader::IsALoadableFileExtension(const eastl::wstring& fileName) const
{
	if (fileName.find('.') != eastl::string::npos)
	{
		eastl::wstring fileExtension = fileName.substr(fileName.find('.') + 1);
		return fileExtension.compare(L"xml") == 0;
	}
	else return false;
}

bool XmlResourceLoader::LoadResource(
	void *rawBuffer, unsigned int rawSize, const eastl::shared_ptr<ResHandle>& handle)
{
    if (rawSize <= 0)
        return false;

    eastl::shared_ptr<XmlResourceExtraData> pExtraData(new XmlResourceExtraData());
	
	eastl::vector<char>* buffer = new eastl::vector<char>(rawSize + 1, 0);
	memcpy(buffer->data(), rawBuffer, rawSize * sizeof(char));
	delete[] rawBuffer;
	rawBuffer = buffer->data();

	pExtraData->ParseXml(reinterpret_cast<char*> (rawBuffer));

    handle->SetExtra(eastl::shared_ptr<XmlResourceExtraData>(pExtraData));

    return true;
}


eastl::shared_ptr<BaseResourceLoader> CreateXmlResourceLoader()
{
    return eastl::shared_ptr<BaseResourceLoader>(new XmlResourceLoader());
}

tinyxml2::XMLElement* XmlResourceLoader::LoadAndReturnRootXMLElement(const wchar_t* resourceName)
{
	BaseResource resource(resourceName);
	// this actually loads the XML file from the zip file
	const eastl::shared_ptr<ResHandle>& pResourceHandle = ResCache::Get()->GetHandle(&resource);
	if (pResourceHandle)
	{
		eastl::shared_ptr<XmlResourceExtraData> pExtraData = 
			eastl::static_pointer_cast<XmlResourceExtraData>(pResourceHandle->GetExtra());
		return pExtraData->GetRoot();
	}
	else return NULL;
}