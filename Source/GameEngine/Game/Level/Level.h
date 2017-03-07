//  SuperTuxKart - a fun racing game with go-kart
//
//  Copyright (C) 2004-2013 Steve Baker <sjbaker1@airmail.net>
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

#ifndef _LEVEL_H_INCLUDED_
#define _LEVEL_H_INCLUDED_

#include "GameEngineStd.h"
#include "GameEngine/interfaces.h"

#include "Scenes/SceneNodes.h"

#include "Utilities/translation.h"
/**
  * \defgroup demos
  * Contains information about demos
  * objects.
  */

struct Subtitle
{
    int m_from, m_to;
    eastl::wstring m_text;

    Subtitle(int from, int to, eastl::wstring text)
    {
        m_from = from;
        m_to = to;
        m_text = text;
    }
    int getFrom() const { return m_from; }
    int getTo()   const { return m_to;   }
    const eastl::wstring& getText() const { return m_text; }
};

/**
  * \ingroup levels
  */
class Level
{
private:

#ifdef DEBUG
    unsigned int             m_magic_number;
#endif

	eastl::string					m_ident;
    eastl::string					m_screenshot;
	double                  m_gravity;

    eastl::vector<Subtitle> m_subtitles;

    /** The full filename of the config (xml) file. */
    eastl::string					m_filename;

    /** The base dir of all files of this level. */
    eastl::string					m_root;
    eastl::vector<eastl::string>	m_groups;

    /** The version of this level. A certain version will only support
     *  certain levels versions. */
    int                      m_version;

    /** Whether this is an "internal" level. If so it won't be offered
      * in the level selection screen
      */
    bool                     m_internal;

    /** Whether this level should be available in reverse version */
    bool                     m_reverse_available;

    bool                     m_is_cutscene;

    bool m_clouds;

    bool m_bloom;
    double m_bloom_threshold;

    bool m_lensflare;
    bool m_godrays;
    bool m_shadows;

    double m_displacement_speed;
    double m_caustics_speed;

    /** The levels for color correction 
     * m_color_inlevel(black, gamma, white)
     * m_color_outlevel(black, white)*/
    Vector3f m_color_inlevel;
    Vector2f m_color_outlevel;

    /** True if this level supports using smoothed normals. */
    bool                m_smooth_normals;

    /** A simple class to keep information about a level mode. */
    class LevelMode
    {
    public:
        eastl::string m_name;        /**< Name / description of this mode. */
        eastl::string m_quad_name;   /**< Name of the quad file to use.    */
        eastl::string m_graph_name;  /**< Name of the graph file to use.   */
        eastl::string m_scene;       /**< Name of the scene file to use.   */

#ifdef DEBUG
        unsigned int m_magic_number;
#endif

        /** Default constructor, sets default names for all fields. */
        LevelMode() 
		:	m_name("default"), m_quad_name("quads.xml"),
			m_graph_name("graph.xml"), m_scene("scene.xml")
        {
#ifdef DEBUG
            m_magic_number = 0x46825179;
#endif
        }

        ~LevelMode()
        {
#ifdef DEBUG
            GE_ASSERT(m_magic_number == 0x46825179);
            m_magic_number = 0xDEADBEEF;
#endif
        }

    };   // LevelMode

    /** List of all modes for a level. */
    eastl::vector<LevelMode> m_all_modes;

    /** Name of the level to display. */
    eastl::string				m_name;

    void loadLevelInfo();
    bool loadMainLevel(const XmlElement* pNode);

public:

    bool reverseAvailable() { return m_reverse_available; }
    void handleAnimatedTextures(SceneNode *node, const XmlElement* xml);

    static const float NOHIT;

                       Level             (const eastl::string& filename);
                      ~Level             ();
    void               cleanup           ();

    void               update(float dt);
    void               reset();

    void               loadLevelModel  (bool reverse_track = false,
                                        unsigned int mode_id=0);
    //bool findGround(AbstractKart *kart);

    eastl::vector< eastl::vector<float> > buildHeightMap();
    // ------------------------------------------------------------------------
    /*void               loadLevelModel  (	World* parent,
											bool reverse_track = false,
											unsigned int mode_id=0);*/
    // ------------------------------------------------------------------------
    //void               addMusic          (MusicInformation* mi)
	//											{m_music.push_back(mi);     }
    // ------------------------------------------------------------------------
    double             getGravity        () const {return m_gravity;          }
    // ------------------------------------------------------------------------
    /** Returns the version of the .level file. */
    int                getVersion        () const {return m_version;          }
    // ------------------------------------------------------------------------
    /** Returns the length of the main driveline. */
    //float              getDemoLength    () const
    //                                 {return QuadGraph::get()->getLapLength();}
    // ------------------------------------------------------------------------
    /** Returns a unique identifier for this level (the directory name). */
    const eastl::string& getIdent          () const {return m_ident;            }
    // ------------------------------------------------------------------------
    /** Returns the name of the level, which is e.g. displayed on the screen.
        \note this is the LTR name, invoke fribidi as needed. */
    const eastl::wstring getName               () const
                             {return translations->w_gettext(m_name.c_str()); }
    // ------------------------------------------------------------------------
    /** Returns all groups this level belongs to. */
	const eastl::vector<eastl::string>& getGroups         () const {return m_groups;}
    // ------------------------------------------------------------------------
    /** Returns the filename of this level. */
    const eastl::string& getFilename       () const	{	return m_filename;      }
    // ------------------------------------------------------------------------
    /** Returns the name of the designer. */
    //const eastl::wstring& getDesigner     () const	{	return m_designer;		}
};   // class Level

#endif
