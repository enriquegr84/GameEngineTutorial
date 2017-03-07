//  SuperTuxKart - a fun racing game with go-kart
//
//  Copyright (C) 2004-2013  Steve Baker <sjbaker1@airmail.net>
//  Copyright (C) 2009-2013  Joerg Henrichs, Steve Baker
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

#include "GameEngine/GameEngine.h"

#include "Scenes/Scene.h"
#include "ResourceCache/XmlResource.h"
//#include "IBillboardTextSceneNode.h"

/*
#include "addons/addon.hpp"
#include "audio/music_manager.hpp"
#include "challenges/challenge.hpp"
#include "challenges/unlock_manager.hpp"
#include "config/stk_config.hpp"
#include "config/user_config.hpp"
#include "graphics/camera.hpp"
#include "graphics/BatchingMesh.hpp"
#include "graphics/irr_driver.hpp"
#include "graphics/lod_node.hpp"
#include "graphics/material_manager.hpp"
#include "graphics/mesh_tools.hpp"
#include "graphics/moving_texture.hpp"
#include "graphics/particle_emitter.hpp"
#include "graphics/particle_kind.hpp"
#include "graphics/particle_kind_manager.hpp"
#include "guiengine/scalable_font.hpp"
#include "io/file_manager.hpp"
#include "io/xml_node.hpp"
#include "items/item.hpp"
#include "items/item_manager.hpp"
#include "karts/abstract_kart.hpp"
#include "karts/kart_properties.hpp"
#include "modes/linear_world.hpp"
#include "modes/easter_egg_hunt.hpp"
#include "modes/world.hpp"
#include "physics/physical_object.hpp"
#include "physics/physics.hpp"
#include "physics/triangle_mesh.hpp"
#include "race/race_manager.hpp"
#include "tracks/bezier_curve.hpp"
#include "tracks/check_manager.hpp"
#include "tracks/lod_node_loader.hpp"
#include "tracks/track_manager.hpp"
#include "tracks/quad_graph.hpp"
#include "tracks/quad_set.hpp"
#include "tracks/track_object_manager.hpp"
#include "utils/constants.hpp"
#include "utils/log.hpp"
#include "utils/string_utils.hpp"
#include "utils/translation.hpp"

#include <ISceneManager.h>
#include <IMeshSceneNode.h>
#include <IMeshManipulator.h>
#include <ILightSceneNode.h>
#include <IMeshCache.h>
*/

const float Level::NOHIT           = -99999.9f;

// ----------------------------------------------------------------------------
Level::Level(const eastl::string& filename)
{
#ifdef DEBUG
    m_magic_number          = 0x17AC3802;
#endif

    m_filename              = filename;
	path	destFileName;
	Utils::RemoveFilenameExtension(destFileName, filename);
    m_root                  =
		g_pGameApp->m_pFileSystem->GetFileDir(destFileName);
	m_ident                 = Utils::GetFileBasename(m_root);
    // If this is an addon track, add "addon_" to the identifier - just in
    // case that an addon track has the same directory name (and therefore
    // identifier) as an included level.
    //if(Addon::isAddon(filename))
     //   m_ident = Addon::createAddonId(m_ident);

    // The directory should always have a '/' at the end, but getBasename
    // above returns "" if a "/" is at the end, so we add the "/" here.
    m_root                 += "/";
    //m_designer              = "";
    m_screenshot            = "";
    m_version               = 0;
    m_internal              = false;
    //m_enable_auto_rescue    = true;  // Below set to false in arenas
    //m_enable_push_back      = true;
    m_reverse_available     = false;
    m_is_cutscene           = false;

    loadLevelInfo();
}   // Level

//-----------------------------------------------------------------------------
/** Destructor, removes quad data structures etc. */
Level::~Level()
{
    // Note that the music information in m_music is globally managed
    // by the music_manager, and is freed there. So no need to free it
    // here (esp. since various level might share the same music).
#ifdef DEBUG
    assert(m_magic_number == 0x17AC3802);
    m_magic_number = 0xDEADBEEF;
#endif
}   // ~Level

//-----------------------------------------------------------------------------
/** Prepates the level for a new game. This function must be called after all
 *  models are created, since the check objects allocate data structures
 *  depending on the number of models.
 */
void Level::reset()
{
    //m_ambient_color = m_default_ambient_color;
    //CheckManager::get()->reset(*this);
    //ItemManager::get()->reset();
    //m_level_object_manager->reset();
}   // reset

//-----------------------------------------------------------------------------
void Level::loadLevelInfo()
{
	m_gravity               = 9.80665f;
	m_smooth_normals        = false;
	m_bloom                 = true;
    m_bloom_threshold       = 0.75f;
    m_color_inlevel         = Vector3f(0.0,1.0, 255.0);
    m_color_outlevel        = Vector2f(0.0, 255.0);
    m_clouds                = false;
    m_lensflare             = false;
    m_godrays               = false;
    m_displacement_speed    = 1.0f;
    m_caustics_speed        = 1.0f;
    m_shadows               = true;

	XmlElement* pRoot = XmlResourceLoader::LoadAndReturnRootXmlElement(
		eastl::wstring(m_filename.c_str()).c_str());

	if(!pRoot || !pRoot->FirstChildElement("level"))
    {
        std::ostringstream o;
		o << "Can't load track '" << m_filename.c_str() << "', no track element.";
        throw std::runtime_error(o.str());
    }
	m_name = pRoot->Attribute("name");

	eastl::string designer = pRoot->Attribute("designer");
    //m_designer = StringUtils::DecodeFromHtmlEntities(designer);
	
	pRoot->Attribute("version",			&m_version);
    //eastl::vector<eastl::string> filenames;
    //pRoot->Attribute("music",                 &filenames);
    //GetMusicInformation(filenames, m_music);
    m_screenshot = pRoot->Attribute("screenshot");
    pRoot->Attribute("gravity",               &m_gravity);
    //pRoot->Attribute("soccer",                &m_is_soccer);
    //pRoot->Attribute("arena",                 &m_is_arena);
	StringUtils::StringToBool(pRoot->Attribute("cutscene"), &m_is_cutscene);
    //pRoot->Attribute("groups",                &m_groups);
	StringUtils::StringToBool(pRoot->Attribute("internal"), &m_internal);
	StringUtils::StringToBool(pRoot->Attribute("reverse"), &m_reverse_available);
    //pRoot->Attribute("push-back",             &m_enable_push_back);

	StringUtils::StringToBool(pRoot->Attribute("clouds"), &m_clouds);
	StringUtils::StringToBool(pRoot->Attribute("bloom"), &m_bloom);
	pRoot->Attribute("bloom-threshold",		&m_bloom_threshold);
	StringUtils::StringToBool(pRoot->Attribute("lens-flare"), &m_lensflare);
	StringUtils::StringToBool(pRoot->Attribute("shadows"), &m_shadows);
	StringUtils::StringToBool(pRoot->Attribute("god-rays"), &m_godrays);

    pRoot->Attribute("displacement-speed",    &m_displacement_speed);
    pRoot->Attribute("caustics-speed",        &m_caustics_speed);

	StringUtils::StringToVector3(pRoot->Attribute("color-level-in"), &m_color_inlevel);
	StringUtils::StringToVector2(pRoot->Attribute("color-level-out"), &m_color_outlevel);

	StringUtils::StringToBool(pRoot->Attribute("smooth-normals"), &m_smooth_normals);

	// Loop through each child element
    for (XmlElement* pNode = pRoot->FirstChildElement(); pNode; 
		pNode = pNode->NextSiblingElement())
    {
		LevelMode lm;
		lm.m_name = pNode->Attribute("name");
		lm.m_quad_name = pNode->Attribute("quads");
		lm.m_graph_name = pNode->Attribute("graph");
		lm.m_scene = pNode->Attribute("scene");
		m_all_modes.push_back(lm);
    }

    // If no mode is specified, add a default mode.
    if(m_all_modes.size()==0)
    {
        LevelMode lm;
        m_all_modes.push_back(lm);
    }

    if(m_groups.size()==0) m_groups.push_back(DEFAULT_GROUP_NAME);

    // Set the correct paths
    //m_screenshot = m_root+m_screenshot;

}   // loadLevelInfo

// ----------------------------------------------------------------------------
/** Loads the main track model (i.e. all other objects contained in the
 *  scene might use raycast on this track model to determine the actual
 *  height of the terrain.
 */
bool Level::loadMainLevel(const XmlElement* pRoot)
{

    return true;
}   // loadMainTrack

// ----------------------------------------------------------------------------
/** Update, called once per frame.
 *  \param dt Timestep.
 */
void Level::update(float dt)
{
	/*
    m_level_object_manager->update(dt);

    for(unsigned int i=0; i<m_animated_textures.size(); i++)
    {
        m_animated_textures[i]->update(dt);
    }
    CheckManager::get()->update(dt);
    ItemManager::get()->update(dt);
	*/
}   // update