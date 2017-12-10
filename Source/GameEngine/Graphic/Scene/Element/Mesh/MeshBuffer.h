// Copyright (C) 2002-2012 Nikolaus Gebhardt
// This file is part of the "Irrlicht Engine".
// For conditions of distribution and use, see copyright notice in irrlicht.h

#ifndef _MESHBUFFER_H_INCLUDED_
#define _MESHBUFFER_H_INCLUDED_

#include "IMeshBuffer.h"

//! Template implementation of the IMeshBuffer interface
template <class T>
class MeshBuffer : public IMeshBuffer
{
public:
	//! Default constructor for empty meshbuffer
	MeshBuffer()
	:m_ChangedIDVertex(1), m_ChangedIDIndex(1), m_MappingHintVertex(EHM_NEVER), m_MappingHintIndex(EHM_NEVER)
	{
		#ifdef _DEBUG
		//setDebugName("MeshBuffer");
		#endif
	}


	//! Get material of this meshbuffer
	/** \return Material of this buffer */
	virtual const Material& GetMaterial() const
	{
		return m_Material;
	}


	//! Get material of this meshbuffer
	/** \return Material of this buffer */
	virtual Material& GetMaterial()
	{
		return m_Material;
	}


	//! Get pointer to vertices
	/** \return Pointer to vertices. */
	virtual const void* GetVertices() const
	{
		return m_Vertices.data();
	}


	//! Get pointer to vertices
	/** \return Pointer to vertices. */
	virtual void* GetVertices()
	{
		return m_Vertices.data();
	}


	//! Get number of vertices
	/** \return Number of vertices. */
	virtual unsigned int GetVertexCount() const
	{
		return m_Vertices.size();
	}

	//! Get type of index data which is stored in this meshbuffer.
	/** \return Index type of this buffer. */
	virtual E_INDEX_TYPE GetIndexType() const
	{
		return EIT_16BIT;
	}

	//! Get pointer to indices
	/** \return Pointer to indices. */
	virtual const u16* GetIndices() const
	{
		return m_Indices.data();
	}


	//! Get pointer to indices
	/** \return Pointer to indices. */
	virtual u16* GetIndices()
	{
		return m_Indices.data();
	}


	//! Get number of indices
	/** \return Number of indices. */
	virtual unsigned int GetIndexCount() const
	{
		return m_Indices.size();
	}


	//! Get the axis aligned bounding box
	/** \return Axis aligned bounding box of this buffer. */
	virtual const AABBox3<float>& GetBoundingBox() const
	{
		return m_BoundingBox;
	}


	//! Set the axis aligned bounding box
	/** \param box New axis aligned bounding box for this buffer. */
	//! set user axis aligned bounding box
	virtual void SetBoundingBox(const AABBox3f& box)
	{
		m_BoundingBox = box;
	}


	//! Recalculate the bounding box.
	/** should be called if the mesh changed. */
	virtual void RecalculateBoundingBox()
	{
		if (m_Vertices.empty())
			m_BoundingBox.Reset(0,0,0);
		else
		{
			m_BoundingBox.Reset(m_Vertices[0].m_Pos);
			for (unsigned int i=1; i<m_Vertices.size(); ++i)
				m_BoundingBox.AddInternalPoint(m_Vertices[i].m_Pos);
		}
	}


	//! Get type of vertex data stored in this buffer.
	/** \return Type of vertex data. */
	virtual E_VERTEX_TYPE GetVertexType() const
	{
		return T().GetType();
	}

	//! returns position of vertex i
	virtual const Vector3<float>& GetPosition(unsigned int i) const
	{
		return m_Vertices[i].m_Pos;
	}

	//! returns position of vertex i
	virtual Vector3<float>& GetPosition(unsigned int i)
	{
		return m_Vertices[i].m_Pos;
	}

	//! returns normal of vertex i
	virtual const Vector3<float>& GetNormal(unsigned int i) const
	{
		return m_Vertices[i].m_Normal;
	}

	//! returns normal of vertex i
	virtual Vector3<float>& GetNormal(unsigned int i)
	{
		return m_Vertices[i].m_Normal;
	}

	//! returns texture coord of vertex i
	virtual const Vector2f& GetTCoords(unsigned int i) const
	{
		return m_Vertices[i].m_TCoords;
	}

	//! returns texture coord of vertex i
	virtual Vector2f& GetTCoords(unsigned int i)
	{
		return m_Vertices[i].m_TCoords;
	}


	//! Append the vertices and indices to the current buffer
	/** Only works for compatible types, i.e. either the same type
	or the main buffer is of standard type. Otherwise, behavior is
	undefined.
	*/
	virtual void Append(const void* const vertices, unsigned int numVertices, const u16* const indices, unsigned int numIndices)
	{
		if (vertices == GetVertices())
			return;

		const unsigned int vertexCount = GetVertexCount();
		unsigned int i;

		for (i=0; i<numVertices; ++i)
		{
			m_Vertices.push_back(reinterpret_cast<const T*>(vertices)[i]);
			m_BoundingBox.AddInternalPoint(reinterpret_cast<const T*>(vertices)[i].m_Pos);
		}

		for (i=0; i<numIndices; ++i)
		{
			m_Indices.push_back(indices[i]+vertexCount);
		}
	}


	//! Append the meshbuffer to the current buffer
	/** Only works for compatible types, i.e. either the same type
	or the main buffer is of standard type. Otherwise, behavior is
	undefined.
	\param other Meshbuffer to be appended to this one.
	*/
	virtual void Append(const IMeshBuffer* const other)
	{
		/*
		if (this==other)
			return;

		const unsigned int vertexCount = getVertexCount();
		unsigned int i;

		for (i=0; i<other->getVertexCount(); ++i)
		{
			Vertices.push_back(reinterpret_cast<const T*>(other->getVertices())[i]);
		}

		for (i=0; i<other->getIndexCount(); ++i)
		{
			Indices.push_back(other->getIndices()[i]+vertexCount);
		}
		BoundingBox.addInternalBox(other->VGetBoundingBox());
		*/
	}


	//! get the current hardware mapping hint
	virtual E_HARDWARE_MAPPING GetHardwareMappingHintVertex() const
	{
		return m_MappingHintVertex;
	}

	//! get the current hardware mapping hint
	virtual E_HARDWARE_MAPPING GetHardwareMappingHintIndex() const
	{
		return m_MappingHintIndex;
	}

	//! set the hardware mapping hint, for driver
	virtual void SetHardwareMappingHint( E_HARDWARE_MAPPING NewMappingHint, E_BUFFER_TYPE Buffer=EBT_VERTEX_AND_INDEX )
	{
		if (Buffer==EBT_VERTEX_AND_INDEX || Buffer==EBT_VERTEX)
			m_MappingHintVertex=NewMappingHint;
		if (Buffer==EBT_VERTEX_AND_INDEX || Buffer==EBT_INDEX)
			m_MappingHintIndex=NewMappingHint;
	}


	//! flags the mesh as changed, reloads hardware buffers
	virtual void SetDirty(E_BUFFER_TYPE Buffer=EBT_VERTEX_AND_INDEX)
	{
		if (Buffer==EBT_VERTEX_AND_INDEX || Buffer==EBT_VERTEX)
			++m_ChangedIDVertex;
		if (Buffer==EBT_VERTEX_AND_INDEX || Buffer==EBT_INDEX)
			++m_ChangedIDIndex;
	}

	//! Get the currently used ID for identification of changes.
	/** This shouldn't be used for anything outside the VideoDriver. */
	virtual unsigned int GetChangedIDVertex() const {return m_ChangedIDVertex;}

	//! Get the currently used ID for identification of changes.
	/** This shouldn't be used for anything outside the VideoDriver. */
	virtual unsigned int GetChangedIDIndex() const {return m_ChangedIDIndex;}

	unsigned int m_ChangedIDVertex;
	unsigned int m_ChangedIDIndex;

	//! hardware mapping hint
	E_HARDWARE_MAPPING m_MappingHintVertex;
	E_HARDWARE_MAPPING m_MappingHintIndex;

	//! Material for this meshbuffer.
	Material m_Material;
	//! Vertices of this buffer
	eastl::vector<T> m_Vertices;
	//! Indices into the vertices of this buffer.
	eastl::vector<u16> m_Indices;
	//! Bounding box of this meshbuffer.
	AABBox3<float> m_BoundingBox;
};

//! Standard meshbuffer
typedef MeshBuffer<VertexBuffer> MeshBufferVertex;
//! Meshbuffer with two texture coords per vertex, e.g. for lightmaps
typedef MeshBuffer<Vertex2TCoords> MeshBufferLightMap;
//! Meshbuffer with vertices having tangents stored, e.g. for normal mapping
typedef MeshBuffer<VertexTangents> MeshBufferTangents;


#endif


