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

#include "GameEngine/GameEngine.h"
#include "XmlResource.h"

void XmlResourceExtraData::ParseXml(char* pRawBuffer)
{
	m_xmlDocument.Parse(pRawBuffer);
}

//! returns true if the file maybe is able to be loaded by this class
//! based on the file extension (e.g. ".xml")
bool XmlResourceLoader::IsALoadableFileExtension(const path& filename) const
{
	return Utils::HasFileExtension ( filename, "xml" );
}

bool XmlResourceLoader::LoadResource(void *rawBuffer, unsigned int rawSize, const shared_ptr<ResHandle>& handle)
{
    if (rawSize <= 0)
        return false;

    shared_ptr<XmlResourceExtraData> pExtraData(new XmlResourceExtraData());
	IReadFile* file = (IReadFile*)rawBuffer;
	u32 size = file->GetSize();
	eastl::vector<char> buffer(size+1);
	file->Read(buffer.data(), size);
	buffer[size] = 0;
    pExtraData->ParseXml(reinterpret_cast<char*> (buffer.data()));

    handle->SetExtra(shared_ptr<XmlResourceExtraData>(pExtraData));

    return true;
}


shared_ptr<BaseResourceLoader> CreateXmlResourceLoader()
{
    return shared_ptr<BaseResourceLoader>(new XmlResourceLoader());
}

XmlElement* XmlResourceLoader::LoadAndReturnRootXmlElement(const wchar_t* resourceString)
{
    Resource resource(resourceString);
	// this actually loads the XML file from the zip file
    const shared_ptr<ResHandle>& pResourceHandle = g_pGameApp->m_ResCache->GetHandle(&resource);
	if (pResourceHandle)
	{
		shared_ptr<XmlResourceExtraData> pExtraData = 
			static_pointer_cast<XmlResourceExtraData>(pResourceHandle->GetExtra());
		return pExtraData->GetRoot();
	}
	else return NULL;
}