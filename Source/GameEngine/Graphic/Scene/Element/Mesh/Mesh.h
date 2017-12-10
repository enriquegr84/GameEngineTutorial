//========================================================================
// File: Mesh.h - classes to render meshes in D3D9 and D3D11
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

#ifndef _MESH_H_INCLUDED_
#define _MESH_H_INCLUDED_

#include "GameEngineStd.h"

//#include <SDKMesh.h>

#include "IMesh.h"
#include "IMeshBuffer.h"

#include "Utilities/Geometry.h"
#include "ResourceCache/ResCache.h"


//! Simple implementation of the IMesh interface.
class Mesh : public IMesh
{
public:
	//! constructor
	Mesh()
	{
		#ifdef _DEBUG
		//setDebugName("SMesh");
		#endif
	}

	//! destructor
	virtual ~Mesh()
	{
	}

	//! clean mesh
	virtual void Clear()
	{
		m_MeshBuffers.clear();
		m_BoundingBox.Reset ( 0.f, 0.f, 0.f );
	}


	//! returns amount of mesh buffers.
	virtual u32 GetMeshBufferCount() const
	{
		return m_MeshBuffers.size();
	}

	//! Get pointer to a mesh buffer.
	virtual shared_ptr<IMeshBuffer> GetMeshBuffer(u32 nr) const
	{
		return m_MeshBuffers[nr];
	}

	//! Get pointer to a mesh buffer which fits a material
	virtual shared_ptr<IMeshBuffer> GetMeshBuffer( const Material &material) const
	{
		for (int i = (int)m_MeshBuffers.size()-1; i >= 0; --i)
		{
			if ( material == m_MeshBuffers[i]->GetMaterial())
				return m_MeshBuffers[i];
		}

		return NULL;
	}

	//! returns an axis aligned bounding box
	virtual const AABBox3<float>& GetBoundingBox() const
	{
		return m_BoundingBox;
	}

	//! set user axis aligned bounding box
	virtual void SetBoundingBox( const AABBox3f& box)
	{
		m_BoundingBox = box;
	}

	//! recalculates the bounding box
	void RecalculateBoundingBox()
	{
		if (m_MeshBuffers.size())
		{
			m_BoundingBox = m_MeshBuffers[0]->GetBoundingBox();
			for (u32 i=1; i< m_MeshBuffers.size(); ++i)
				m_BoundingBox.AddInternalBox(m_MeshBuffers[i]->GetBoundingBox());
		}
		else
			m_BoundingBox.Reset(0.0f, 0.0f, 0.0f);
	}

	//! adds a MeshBuffer
	/** The bounding box is not updated automatically. */
	void AddMeshBuffer(const shared_ptr<IMeshBuffer>& buf)
	{
		if (buf)
			m_MeshBuffers.push_back(buf);
	}

	//! sets a flag of all contained materials to a new value
	virtual void SetMaterialFlag(E_MATERIAL_FLAG flag, bool newvalue)
	{
		for (u32 i=0; i< m_MeshBuffers.size(); ++i)
			m_MeshBuffers[i]->GetMaterial().SetFlag(flag, newvalue);
	}

	//! set the hardware mapping hint, for driver
	virtual void SetHardwareMappingHint( E_HARDWARE_MAPPING newMappingHint, E_BUFFER_TYPE buffer=EBT_VERTEX_AND_INDEX )
	{
		for (u32 i=0; i< m_MeshBuffers.size(); ++i)
			m_MeshBuffers[i]->SetHardwareMappingHint(newMappingHint, buffer);
	}

	//! flags the meshbuffer as changed, reloads hardware buffers
	virtual void SetDirty(E_BUFFER_TYPE buffer=EBT_VERTEX_AND_INDEX)
	{
		for (u32 i=0; i< m_MeshBuffers.size(); ++i)
			m_MeshBuffers[i]->SetDirty(buffer);
	}

	//! The meshbuffers of this mesh
	eastl::vector<shared_ptr<IMeshBuffer>> m_MeshBuffers;

	//! The bounding box of this mesh
	AABBox3<float> m_BoundingBox;
};

#endif