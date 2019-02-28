//
//  This program is free software; you can redistribute it and/or
//  modify it under the terms of the GNU General Public License
//  as published by the Free Software Foundation; either version 3
//  of the License, or (at your option) any later version.
//
//  This program is distributed in the hope that it will be useful,
//  but WITHOUT ANY WARRANTY; without even the implied warranty of
//  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//  GNU General Public License for more details.
//
//  You should have received a copy of the GNU General Public License
//  along with this program; if not, write to the Free Software
//  Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.

#include "Level.h"

#include "Core/IO/FileSystem.h"
#include "Core/IO/XmlResource.h"

#include "Core/Utility/StringUtil.h"

// ----------------------------------------------------------------------------
Level::Level(const eastl::wstring& filename) : mID(filename), mFileName(filename)
{
	if (filename.rfind('.') != eastl::string::npos)
	{
		eastl::wstring destFileName(filename.substr(0, filename.rfind('.')).c_str());
		FileSystem* fileSystem = FileSystem::Get();
		fileSystem->GetFileDir(destFileName);
	}

	LoadLevelInfo();
}   // Level

//-----------------------------------------------------------------------------
/** Destructor, removes quad data structures etc. */
Level::~Level()
{

}   // ~Level

//-----------------------------------------------------------------------------
/** Prepates the level for a new game. This function must be called after all
 *  models are created, since the check objects allocate data structures
 *  depending on the number of models.
 */
void Level::Reset()
{

}   // reset

//-----------------------------------------------------------------------------
void Level::LoadLevelInfo()
{
	tinyxml2::XMLElement* pRoot = XmlResourceLoader::LoadAndReturnRootXMLElement(mFileName.c_str());

	if (!pRoot)
	{
		LogError(eastl::wstring(L"Can't load level '") + mFileName.c_str() + eastl::wstring(L"', no level element."));
		return;
	}

	mName = ToWideString(pRoot->Attribute("name"));
}   // loadLevelInfo

// ----------------------------------------------------------------------------
/** Loads the main model (i.e. all other objects contained in the
 *  scene might use raycast on this track model to determine the actual
 *  height of the terrain.
 */
bool Level::LoadMainLevel(const tinyxml2::XMLElement* pRoot)
{

    return true;
} 

// ----------------------------------------------------------------------------
/** Update, called once per frame.
 *  \param dt Timestep.
 */
void Level::Update(float dt)
{

}   // update