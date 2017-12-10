// Copyright (C) 2002-2012 Nikolaus Gebhardt
// This file is part of the "Irrlicht Engine".
// For conditions of distribution and use, see copyright notice in irrlicht.h

#include "MeshManipulator.h"

#include "Mesh.h"
#include "MeshBuffer.h"
#include "AnimatedMesh.h"

#include "OS/os.h"

static inline Vector3<float> GetAngleWeight(
	const Vector3<float>& v1, const Vector3<float>& v2, const Vector3<float>& v3)
{
	// Calculate this triangle's weight for each of its three vertices
	// start by calculating the lengths of its sides
	const float a = v2.GetDistanceFromSQ(v3);
	const float asqrt = sqrtf(a);
	const float b = v1.GetDistanceFromSQ(v3);
	const float bsqrt = sqrtf(b);
	const float c = v1.GetDistanceFromSQ(v2);
	const float csqrt = sqrtf(c);

	// use them to find the angle at each vertex
	return Vector3<float>(
		acosf((b + c - a) / (2.f * bsqrt * csqrt)),
		acosf((-b + c + a) / (2.f * asqrt * csqrt)),
		acosf((b - c + a) / (2.f * bsqrt * asqrt)));
}


//! Flips the direction of surfaces. Changes backfacing triangles to frontfacing
//! triangles and vice versa.
//! \param mesh: Mesh on which the operation is performed.
void MeshManipulator::FlipSurfaces(const IMesh* mesh) const
{
	if (!mesh)
		return;

	const unsigned int bcount = mesh->GetMeshBufferCount();
	for (unsigned int b=0; b<bcount; ++b)
	{
		const shared_ptr<IMeshBuffer>& buffer = mesh->GetMeshBuffer(b);
		const unsigned int idxcnt = buffer->GetIndexCount();
		if (buffer->GetIndexType() == EIT_16BIT)
		{
			u16* idx = buffer->GetIndices();
			for (unsigned int i=0; i<idxcnt; i+=3)
			{
				const u16 tmp = idx[i+1];
				idx[i+1] = idx[i+2];
				idx[i+2] = tmp;
			}
		}
		else
		{
			unsigned int* idx = reinterpret_cast<unsigned int*>(buffer->GetIndices());
			for (unsigned int i=0; i<idxcnt; i+=3)
			{
				const unsigned int tmp = idx[i+1];
				idx[i+1] = idx[i+2];
				idx[i+2] = tmp;
			}
		}
	}
}


namespace
{
template <typename T>
void RecalculateNormalsT(IMeshBuffer* buffer, bool smooth, bool angleWeighted)
{
	const unsigned int vtxcnt = buffer->GetVertexCount();
	const unsigned int idxcnt = buffer->GetIndexCount();
	const T* idx = reinterpret_cast<T*>(buffer->GetIndices());

	if (!smooth)
	{
		for (unsigned int i=0; i<idxcnt; i+=3)
		{
			const Vector3<float>& v1 = buffer->GetPosition(idx[i+0]);
			const Vector3<float>& v2 = buffer->GetPosition(idx[i+1]);
			const Vector3<float>& v3 = buffer->GetPosition(idx[i+2]);
			const Vector3<float> normal = Plane3<float>(v1, v2, v3).Normal;
			buffer->GetNormal(idx[i+0]) = normal;
			buffer->GetNormal(idx[i+1]) = normal;
			buffer->GetNormal(idx[i+2]) = normal;
		}
	}
	else
	{
		unsigned int i;

		for ( i = 0; i!= vtxcnt; ++i )
			buffer->GetNormal(i).set(0.f, 0.f, 0.f);

		for ( i=0; i<idxcnt; i+=3)
		{
			const Vector3<float>& v1 = buffer->GetPosition(idx[i+0]);
			const Vector3<float>& v2 = buffer->GetPosition(idx[i+1]);
			const Vector3<float>& v3 = buffer->GetPosition(idx[i+2]);
			const Vector3<float> normal = Plane3<float>(v1, v2, v3).Normal;

			Vector3<float> weight(1.f,1.f,1.f);
			if (angleWeighted)
				weight = GetAngleWeight(v1,v2,v3); // writing necessary for borland

			buffer->GetNormal(idx[i+0]) += weight.X*normal;
			buffer->GetNormal(idx[i+1]) += weight.Y*normal;
			buffer->GetNormal(idx[i+2]) += weight.Z*normal;
		}

		for ( i = 0; i!= vtxcnt; ++i )
			buffer->GetNormal(i).Normalize();
	}
}
}


//! Recalculates all normals of the mesh buffer.
/** \param buffer: Mesh buffer on which the operation is performed. */
void MeshManipulator::RecalculateNormals(IMeshBuffer* buffer, bool smooth, bool angleWeighted) const
{
	if (!buffer)
		return;

	if (buffer->GetIndexType()==EIT_16BIT)
		RecalculateNormalsT<u16>(buffer, smooth, angleWeighted);
	else
		RecalculateNormalsT<unsigned int>(buffer, smooth, angleWeighted);
}


//! Recalculates all normals of the mesh.
//! \param mesh: Mesh on which the operation is performed.
void MeshManipulator::RecalculateNormals(IMesh* mesh, bool smooth, bool angleWeighted) const
{
	if (!mesh)
		return;

	const unsigned int bcount = mesh->GetMeshBufferCount();
	for ( unsigned int b=0; b<bcount; ++b)
		RecalculateNormals(mesh->GetMeshBuffer(b).get(), smooth, angleWeighted);
}


namespace
{
void CalculateTangents(Vector3<float>& normal, Vector3<float>& tangent, Vector3<float>& binormal,
	const Vector3<float>& vt1, const Vector3<float>& vt2, const Vector3<float>& vt3, // vertices
	const Vector2f& tc1, const Vector2f& tc2, const Vector2f& tc3) // texture coords
{
	// choose one of them:
	//#define USE_NVIDIA_GLH_VERSION // use version used by nvidia in glh headers
	#define USE_GE_VERSION

#ifdef USE_GE_VERSION

	Vector3<float> v1 = vt1 - vt2;
	Vector3<float> v2 = vt3 - vt1;
	normal = v2.CrossProduct(v1);
	normal.Normalize();

	// binormal

	float deltaX1 = tc1.X - tc2.X;
	float deltaX2 = tc3.X - tc1.X;
	binormal = (v1 * deltaX2) - (v2 * deltaX1);
	binormal.Normalize();

	// tangent

	float deltaY1 = tc1.Y - tc2.Y;
	float deltaY2 = tc3.Y - tc1.Y;
	tangent = (v1 * deltaY2) - (v2 * deltaY1);
	tangent.Normalize();

	// adjust

	Vector3<float> txb = tangent.CrossProduct(binormal);
	if (txb.DotProduct(normal) < 0.0f)
	{
		tangent *= -1.0f;
		binormal *= -1.0f;
	}

#endif // USE_GE_VERSION

#ifdef USE_NVIDIA_GLH_VERSION

	tangent.set(0,0,0);
	binormal.set(0,0,0);

	Vector3<float> v1(vt2.X - vt1.X, tc2.X - tc1.X, tc2.Y - tc1.Y);
	Vector3<float> v2(vt3.X - vt1.X, tc3.X - tc1.X, tc3.Y - tc1.Y);

	Vector3<float> txb = v1.CrossProduct(v2);
	if ( !iszero ( txb.X ) )
	{
		tangent.X  = -txb.Y / txb.X;
		binormal.X = -txb.Z / txb.X;
	}

	v1.X = vt2.Y - vt1.Y;
	v2.X = vt3.Y - vt1.Y;
	txb = v1.CrossProduct(v2);

	if ( !Iszero ( txb.X ) )
	{
		tangent.Y  = -txb.Y / txb.X;
		binormal.Y = -txb.Z / txb.X;
	}

	v1.X = vt2.Z - vt1.Z;
	v2.X = vt3.Z - vt1.Z;
	txb = v1.CrossProduct(v2);

	if ( !Iszero ( txb.X ) )
	{
		tangent.Z  = -txb.Y / txb.X;
		binormal.Z = -txb.Z / txb.X;
	}

	tangent.Normalize();
	binormal.Normalize();

	normal = tangent.CrossProduct(binormal);
	normal.Normalize();

	binormal = tangent.CrossProduct(normal);
	binormal.Normalize();

	plane3d<float> pl(vt1, vt2, vt3);

	if(normal.DotProduct(pl.Normal) < 0.0f )
		normal *= -1.0f;

#endif // USE_NVIDIA_GLH_VERSION
}


//! Recalculates tangents for a tangent mesh buffer
template <typename T>
void RecalculateTangentsT(IMeshBuffer* buffer, bool recalculateNormals, bool smooth, bool angleWeighted)
{
	if (!buffer || (buffer->GetVertexType()!= EVT_TANGENTS))
		return;

	const unsigned int vtxCnt = buffer->GetVertexCount();
	const unsigned int idxCnt = buffer->GetIndexCount();

	T* idx = reinterpret_cast<T*>(buffer->GetIndices());
	VertexTangents* v =
		(VertexTangents*)buffer->GetVertices();

	if (smooth)
	{
		unsigned int i;

		for ( i = 0; i!= vtxCnt; ++i )
		{
			if (recalculateNormals)
				v[i].m_Normal.set( 0.f, 0.f, 0.f );
			v[i].m_Tangent.set( 0.f, 0.f, 0.f );
			v[i].m_Binormal.set( 0.f, 0.f, 0.f );
		}

		//Each vertex gets the sum of the tangents and binormals from the faces around it
		for ( i=0; i<idxCnt; i+=3)
		{
			// if this triangle is degenerate, skip it!
			if (v[idx[i+0]].m_Pos == v[idx[i+1]].m_Pos ||
				v[idx[i+0]].m_Pos == v[idx[i+2]].m_Pos ||
				v[idx[i+1]].m_Pos == v[idx[i+2]].m_Pos
				/*||
				v[idx[i+0]].m_TCoords == v[idx[i+1]].m_TCoords ||
				v[idx[i+0]].m_TCoords == v[idx[i+2]].m_TCoords ||
				v[idx[i+1]].m_TCoords == v[idx[i+2]].m_TCoords */
				)
				continue;

			//Angle-weighted normals look better, but are slightly more CPU intensive to calculate
			Vector3<float> weight(1.f,1.f,1.f);
			if (angleWeighted)
				weight = GetAngleWeight(v[i+0].m_Pos,v[i+1].m_Pos,v[i+2].m_Pos);	// writing  necessary for borland
			Vector3<float> localNormal;
			Vector3<float> localTangent;
			Vector3<float> localBinormal;

			CalculateTangents(
				localNormal,
				localTangent,
				localBinormal,
				v[idx[i+0]].m_Pos,
				v[idx[i+1]].m_Pos,
				v[idx[i+2]].m_Pos,
				v[idx[i+0]].m_TCoords,
				v[idx[i+1]].m_TCoords,
				v[idx[i+2]].m_TCoords);

			if (recalculateNormals)
				v[idx[i+0]].m_Normal += localNormal * weight.X;
			v[idx[i+0]].m_Tangent += localTangent * weight.X;
			v[idx[i+0]].m_Binormal += localBinormal * weight.X;

			CalculateTangents(
				localNormal,
				localTangent,
				localBinormal,
				v[idx[i+1]].m_Pos,
				v[idx[i+2]].m_Pos,
				v[idx[i+0]].m_Pos,
				v[idx[i+1]].m_TCoords,
				v[idx[i+2]].m_TCoords,
				v[idx[i+0]].m_TCoords);

			if (recalculateNormals)
				v[idx[i+1]].m_Normal += localNormal * weight.Y;
			v[idx[i+1]].m_Tangent += localTangent * weight.Y;
			v[idx[i+1]].m_Binormal += localBinormal * weight.Y;

			CalculateTangents(
				localNormal,
				localTangent,
				localBinormal,
				v[idx[i+2]].m_Pos,
				v[idx[i+0]].m_Pos,
				v[idx[i+1]].m_Pos,
				v[idx[i+2]].m_TCoords,
				v[idx[i+0]].m_TCoords,
				v[idx[i+1]].m_TCoords);

			if (recalculateNormals)
				v[idx[i+2]].m_Normal += localNormal * weight.Z;
			v[idx[i+2]].m_Tangent += localTangent * weight.Z;
			v[idx[i+2]].m_Binormal += localBinormal * weight.Z;
		}

		// Normalize the tangents and binormals
		if (recalculateNormals)
		{
			for ( i = 0; i!= vtxCnt; ++i )
				v[i].m_Normal.Normalize();
		}
		for ( i = 0; i!= vtxCnt; ++i )
		{
			v[i].m_Tangent.Normalize();
			v[i].m_Binormal.Normalize();
		}
	}
	else
	{
		Vector3<float> localNormal;
		for (unsigned int i=0; i<idxCnt; i+=3)
		{
			CalculateTangents(
				localNormal,
				v[idx[i+0]].m_Tangent,
				v[idx[i+0]].m_Binormal,
				v[idx[i+0]].m_Pos,
				v[idx[i+1]].m_Pos,
				v[idx[i+2]].m_Pos,
				v[idx[i+0]].m_TCoords,
				v[idx[i+1]].m_TCoords,
				v[idx[i+2]].m_TCoords);
			if (recalculateNormals)
				v[idx[i+0]].m_Normal=localNormal;

			CalculateTangents(
				localNormal,
				v[idx[i+1]].m_Tangent,
				v[idx[i+1]].m_Binormal,
				v[idx[i+1]].m_Pos,
				v[idx[i+2]].m_Pos,
				v[idx[i+0]].m_Pos,
				v[idx[i+1]].m_TCoords,
				v[idx[i+2]].m_TCoords,
				v[idx[i+0]].m_TCoords);
			if (recalculateNormals)
				v[idx[i+1]].m_Normal=localNormal;

			CalculateTangents(
				localNormal,
				v[idx[i+2]].m_Tangent,
				v[idx[i+2]].m_Binormal,
				v[idx[i+2]].m_Pos,
				v[idx[i+0]].m_Pos,
				v[idx[i+1]].m_Pos,
				v[idx[i+2]].m_TCoords,
				v[idx[i+0]].m_TCoords,
				v[idx[i+1]].m_TCoords);
			if (recalculateNormals)
				v[idx[i+2]].m_Normal=localNormal;
		}
	}
}
}

//! Recalculates tangents for a tangent mesh buffer
void MeshManipulator::RecalculateTangents(IMeshBuffer* buffer, bool recalculateNormals, bool smooth, bool angleWeighted) const
{
	if (buffer && (buffer->GetVertexType() == EVT_TANGENTS))
	{
		if (buffer->GetIndexType() == EIT_16BIT)
			RecalculateTangentsT<u16>(buffer, recalculateNormals, smooth, angleWeighted);
		else
			RecalculateTangentsT<unsigned int>(buffer, recalculateNormals, smooth, angleWeighted);
	}
}


//! Recalculates tangents for all tangent mesh buffers
void MeshManipulator::RecalculateTangents(IMesh* mesh, bool recalculateNormals, bool smooth, bool angleWeighted) const
{
	if (!mesh)
		return;

	const unsigned int meshBufferCount = mesh->GetMeshBufferCount();
	for (unsigned int b=0; b<meshBufferCount; ++b)
	{
		RecalculateTangents(mesh->GetMeshBuffer(b).get(), recalculateNormals, smooth, angleWeighted);
	}
}


namespace
{
//! Creates a planar texture mapping on the meshbuffer
template<typename T>
void MakePlanarTextureMappingT(IMeshBuffer* buffer, float resolution)
{
	unsigned int idxcnt = buffer->GetIndexCount();
	T* idx = reinterpret_cast<T*>(buffer->GetIndices());

	for (unsigned int i=0; i<idxcnt; i+=3)
	{
		Plane3f p(buffer->GetPosition(idx[i+0]), buffer->GetPosition(idx[i+1]), buffer->GetPosition(idx[i+2]));
		p.Normal.X = fabsf(p.Normal.X);
		p.Normal.Y = fabsf(p.Normal.Y);
		p.Normal.Z = fabsf(p.Normal.Z);
		// calculate planar mapping worldspace coordinates

		if (p.Normal.X > p.Normal.Y && p.Normal.X > p.Normal.Z)
		{
			for (unsigned int o=0; o!=3; ++o)
			{
				buffer->GetTCoords(idx[i+o]).X = buffer->GetPosition(idx[i+o]).Y * resolution;
				buffer->GetTCoords(idx[i+o]).Y = buffer->GetPosition(idx[i+o]).Z * resolution;
			}
		}
		else
		if (p.Normal.Y > p.Normal.X && p.Normal.Y > p.Normal.Z)
		{
			for (unsigned int o=0; o!=3; ++o)
			{
				buffer->GetTCoords(idx[i+o]).X = buffer->GetPosition(idx[i+o]).X * resolution;
				buffer->GetTCoords(idx[i+o]).Y = buffer->GetPosition(idx[i+o]).Z * resolution;
			}
		}
		else
		{
			for (unsigned int o=0; o!=3; ++o)
			{
				buffer->GetTCoords(idx[i+o]).X = buffer->GetPosition(idx[i+o]).X * resolution;
				buffer->GetTCoords(idx[i+o]).Y = buffer->GetPosition(idx[i+o]).Y * resolution;
			}
		}
	}
}
}


//! Creates a planar texture mapping on the meshbuffer
void MeshManipulator::MakePlanarTextureMapping(IMeshBuffer* buffer, float resolution) const
{
	if (!buffer)
		return;

	if (buffer->GetIndexType()==EIT_16BIT)
		MakePlanarTextureMappingT<u16>(buffer, resolution);
	else
		MakePlanarTextureMappingT<unsigned int>(buffer, resolution);
}


//! Creates a planar texture mapping on the mesh
void MeshManipulator::MakePlanarTextureMapping(IMesh* mesh, float resolution) const
{
	if (!mesh)
		return;

	const unsigned int bcount = mesh->GetMeshBufferCount();
	for ( unsigned int b=0; b<bcount; ++b)
	{
		MakePlanarTextureMapping(mesh->GetMeshBuffer(b).get(), resolution);
	}
}


namespace
{
//! Creates a planar texture mapping on the meshbuffer
template <typename T>
void MakePlanarTextureMappingT(IMeshBuffer* buffer, float resolutionS, float resolutionT, u8 axis, const Vector3<float>& offset)
{
	unsigned int idxcnt = buffer->GetIndexCount();
	T* idx = reinterpret_cast<T*>(buffer->GetIndices());

	for (unsigned int i=0; i<idxcnt; i+=3)
	{
		// calculate planar mapping worldspace coordinates
		if (axis==0)
		{
			for (unsigned int o=0; o!=3; ++o)
			{
				buffer->GetTCoords(idx[i+o]).X = 0.5f+(buffer->GetPosition(idx[i+o]).Z + offset.Z) * resolutionS;
				buffer->GetTCoords(idx[i+o]).Y = 0.5f-(buffer->GetPosition(idx[i+o]).Y + offset.Y) * resolutionT;
			}
		}
		else if (axis==1)
		{
			for (unsigned int o=0; o!=3; ++o)
			{
				buffer->GetTCoords(idx[i+o]).X = 0.5f+(buffer->GetPosition(idx[i+o]).X + offset.X) * resolutionS;
				buffer->GetTCoords(idx[i+o]).Y = 1.f-(buffer->GetPosition(idx[i+o]).Z + offset.Z) * resolutionT;
			}
		}
		else if (axis==2)
		{
			for (unsigned int o=0; o!=3; ++o)
			{
				buffer->GetTCoords(idx[i+o]).X = 0.5f+(buffer->GetPosition(idx[i+o]).X + offset.X) * resolutionS;
				buffer->GetTCoords(idx[i+o]).Y = 0.5f-(buffer->GetPosition(idx[i+o]).Y + offset.Y) * resolutionT;
			}
		}
	}
}
}


//! Creates a planar texture mapping on the meshbuffer
void MeshManipulator::MakePlanarTextureMapping(IMeshBuffer* buffer, float resolutionS, float resolutionT, 
	u8 axis, const Vector3<float>& offset) const
{
	if (!buffer)
		return;

	if (buffer->GetIndexType()==EIT_16BIT)
		MakePlanarTextureMappingT<u16>(buffer, resolutionS, resolutionT, axis, offset);
	else
		MakePlanarTextureMappingT<unsigned int>(buffer, resolutionS, resolutionT, axis, offset);
}


//! Creates a planar texture mapping on the mesh
void MeshManipulator::MakePlanarTextureMapping(IMesh* mesh, float resolutionS, float resolutionT, 
	u8 axis, const Vector3<float>& offset) const
{
	if (!mesh)
		return;

	const unsigned int bcount = mesh->GetMeshBufferCount();
	for ( unsigned int b=0; b<bcount; ++b)
	{
		MakePlanarTextureMapping(mesh->GetMeshBuffer(b).get(), resolutionS, resolutionT, axis, offset);
	}
}


//! Clones a static IMesh into a modifyable Mesh.
// not yet 32bit
IMesh* MeshManipulator::CreateMeshCopy(const IMesh* mesh) const
{
	if (!mesh)
		return 0;

	Mesh* clone = new Mesh();

	const unsigned int meshBufferCount = mesh->GetMeshBufferCount();

	for ( unsigned int b=0; b<meshBufferCount; ++b)
	{
		const shared_ptr<IMeshBuffer>& mb = mesh->GetMeshBuffer(b);
		switch(mb->GetVertexType())
		{
		case EVT_STANDARD:
			{
				MeshBufferVertex* buffer = new MeshBufferVertex();
				buffer->m_Material = mb->GetMaterial();
				const unsigned int vcount = mb->GetVertexCount();
				VertexBuffer* vertices = (VertexBuffer*)mb->GetVertices();
				for (unsigned int i=0; i < vcount; ++i)
					buffer->m_Vertices.push_back(vertices[i]);
				const unsigned int icount = mb->GetIndexCount();
				const u16* indices = mb->GetIndices();
				for (unsigned int i=0; i < icount; ++i)
					buffer->m_Indices.push_back(indices[i]);
				clone->AddMeshBuffer(shared_ptr<IMeshBuffer>(buffer));
			}
			break;
		case EVT_2TCOORDS:
			{
				MeshBufferLightMap* buffer = new MeshBufferLightMap();
				buffer->m_Material = mb->GetMaterial();
				const unsigned int vcount = mb->GetVertexCount();
				Vertex2TCoords* vertices = (Vertex2TCoords*)mb->GetVertices();
				for (unsigned int i=0; i < vcount; ++i)
					buffer->m_Vertices.push_back(vertices[i]);
				const unsigned int icount = mb->GetIndexCount();
				const u16* indices = mb->GetIndices();
				for (unsigned int i=0; i < icount; ++i)
					buffer->m_Indices.push_back(indices[i]);
				clone->AddMeshBuffer(shared_ptr<IMeshBuffer>(buffer));
			}
			break;
		case EVT_TANGENTS:
			{
				MeshBufferTangents* buffer = new MeshBufferTangents();
				buffer->m_Material = mb->GetMaterial();
				const unsigned int vcount = mb->GetVertexCount();
				VertexTangents* vertices = (VertexTangents*)mb->GetVertices();
				for (unsigned int i=0; i < vcount; ++i)
					buffer->m_Vertices.push_back(vertices[i]);
				const unsigned int icount = mb->GetIndexCount();
				const u16* indices = mb->GetIndices();
				for (unsigned int i=0; i < icount; ++i)
					buffer->m_Indices.push_back(indices[i]);
				clone->AddMeshBuffer(shared_ptr<IMeshBuffer>(buffer));
			}
			break;
		}// end switch

	}// end for all mesh buffers

	clone->m_BoundingBox = mesh->GetBoundingBox();
	return clone;
}


//! Creates a copy of the mesh, which will only consist of unique primitives
// not yet 32bit
IMesh* MeshManipulator::CreateMeshUniquePrimitives(const IMesh* mesh) const
{
	if (!mesh)
		return 0;

	Mesh* clone = new Mesh();

	const unsigned int meshBufferCount = mesh->GetMeshBufferCount();

	for ( unsigned int b=0; b<meshBufferCount; ++b)
	{
		const shared_ptr<IMeshBuffer>& mb = mesh->GetMeshBuffer(b);
		const int idxCnt = mb->GetIndexCount();
		const u16* idx = mb->GetIndices();

		switch(mb->GetVertexType())
		{
			case EVT_STANDARD:
			{
				MeshBufferVertex* buffer = new MeshBufferVertex();
				buffer->m_Material = mb->GetMaterial();

				VertexBuffer* v = (VertexBuffer*)mb->GetVertices();

				for (int i=0; i<idxCnt; i += 3)
				{
					buffer->m_Vertices.push_back( v[idx[i + 0 ]] );
					buffer->m_Vertices.push_back( v[idx[i + 1 ]] );
					buffer->m_Vertices.push_back( v[idx[i + 2 ]] );

					buffer->m_Indices.push_back( i + 0 );
					buffer->m_Indices.push_back( i + 1 );
					buffer->m_Indices.push_back( i + 2 );
				}

				buffer->SetBoundingBox(mb->GetBoundingBox());
				clone->AddMeshBuffer(shared_ptr<IMeshBuffer>(buffer));
			}
			break;

			case EVT_2TCOORDS:
			{
				MeshBufferLightMap* buffer = new MeshBufferLightMap();
				buffer->m_Material = mb->GetMaterial();

				Vertex2TCoords* v = (Vertex2TCoords*)mb->GetVertices();

				for (int i=0; i<idxCnt; i += 3)
				{
					buffer->m_Vertices.push_back( v[idx[i + 0 ]] );
					buffer->m_Vertices.push_back( v[idx[i + 1 ]] );
					buffer->m_Vertices.push_back( v[idx[i + 2 ]] );

					buffer->m_Indices.push_back( i + 0 );
					buffer->m_Indices.push_back( i + 1 );
					buffer->m_Indices.push_back( i + 2 );
				}
				buffer->SetBoundingBox(mb->GetBoundingBox());
				clone->AddMeshBuffer(shared_ptr<IMeshBuffer>(buffer));
			}
			break;

			case EVT_TANGENTS:
			{
				MeshBufferTangents* buffer = new MeshBufferTangents();
				buffer->m_Material = mb->GetMaterial();

				VertexTangents* v = (VertexTangents*)mb->GetVertices();

				for (int i=0; i<idxCnt; i += 3)
				{
					buffer->m_Vertices.push_back( v[idx[i + 0 ]] );
					buffer->m_Vertices.push_back( v[idx[i + 1 ]] );
					buffer->m_Vertices.push_back( v[idx[i + 2 ]] );

					buffer->m_Indices.push_back( i + 0 );
					buffer->m_Indices.push_back( i + 1 );
					buffer->m_Indices.push_back( i + 2 );
				}

				buffer->SetBoundingBox(mb->GetBoundingBox());
				clone->AddMeshBuffer(shared_ptr<IMeshBuffer>(buffer));
			}
			break;
		}// end switch

	}// end for all mesh buffers

	clone->m_BoundingBox = mesh->GetBoundingBox();
	return clone;
}


//! Creates a copy of a mesh, which will have identical vertices welded together
// not yet 32bit
IMesh* MeshManipulator::CreateMeshWelded(const IMesh* mesh, float tolerance) const
{

	Mesh* clone = new Mesh();
	clone->m_BoundingBox = mesh->GetBoundingBox();

	eastl::vector<u16> redirects;

	for (unsigned int b=0; b<mesh->GetMeshBufferCount(); ++b)
	{
		const shared_ptr<IMeshBuffer>& mb = mesh->GetMeshBuffer(b);

		const u16* indices = 0;
		unsigned int indexCount = 0;
		eastl::vector<u16>* outIdx = 0;

		switch(mb->GetVertexType())
		{
		case EVT_STANDARD:
		{
			MeshBufferVertex* buffer = new MeshBufferVertex();
			buffer->m_BoundingBox = mb->GetBoundingBox();
			buffer->m_Material = mb->GetMaterial();
			clone->AddMeshBuffer(shared_ptr<IMeshBuffer>(buffer));

			VertexBuffer* v = (VertexBuffer*)mb->GetVertices();
			unsigned int vertexCount = mb->GetVertexCount();

			indices = mb->GetIndices();
			indexCount = mb->GetIndexCount();
			outIdx = &buffer->m_Indices;

			for (unsigned int i=0; i < vertexCount; ++i)
			{
				bool found = false;
				for (unsigned int j=0; j < i; ++j)
				{
					if ( v[i].m_Pos.equal( v[j].m_Pos, tolerance) &&
						 v[i].m_Normal.equal( v[j].m_Normal, tolerance) &&
						 v[i].m_TCoords.equal( v[j].m_TCoords ) &&
						(v[i].m_Color == v[j].m_Color) )
					{
						redirects[i] = redirects[j];
						found = true;
						break;
					}
				}
				if (!found)
				{
					redirects[i] = buffer->m_Vertices.size();
					buffer->m_Vertices.push_back(v[i]);
				}
			}

			break;
		}
		case EVT_2TCOORDS:
		{
			MeshBufferLightMap* buffer = new MeshBufferLightMap();
			buffer->m_BoundingBox = mb->GetBoundingBox();
			buffer->m_Material = mb->GetMaterial();
			clone->AddMeshBuffer(shared_ptr<IMeshBuffer>(buffer));

			Vertex2TCoords* v = (Vertex2TCoords*)mb->GetVertices();
			unsigned int vertexCount = mb->GetVertexCount();

			indices = mb->GetIndices();
			indexCount = mb->GetIndexCount();
			outIdx = &buffer->m_Indices;

			for (unsigned int i=0; i < vertexCount; ++i)
			{
				bool found = false;
				for (unsigned int j=0; j < i; ++j)
				{
					if ( v[i].m_Pos.equal( v[j].m_Pos, tolerance) &&
						 v[i].m_Normal.equal( v[j].m_Normal, tolerance) &&
						 v[i].m_TCoords.equal( v[j].m_TCoords ) &&
						 v[i].m_TCoords2.equal( v[j].m_TCoords2 ) &&
						(v[i].m_Color == v[j].m_Color) )
					{
						redirects[i] = redirects[j];
						found = true;
						break;
					}
				}
				if (!found)
				{
					redirects[i] = buffer->m_Vertices.size();
					buffer->m_Vertices.push_back(v[i]);
				}
			}
			break;
		}
		case EVT_TANGENTS:
		{
			MeshBufferTangents* buffer = new MeshBufferTangents();
			buffer->m_BoundingBox = mb->GetBoundingBox();
			buffer->m_Material = mb->GetMaterial();
			clone->AddMeshBuffer(shared_ptr<IMeshBuffer>(buffer));

			VertexTangents* v = (VertexTangents*)mb->GetVertices();
			unsigned int vertexCount = mb->GetVertexCount();

			indices = mb->GetIndices();
			indexCount = mb->GetIndexCount();
			outIdx = &buffer->m_Indices;

			for (unsigned int i=0; i < vertexCount; ++i)
			{
				bool found = false;
				for (unsigned int j=0; j < i; ++j)
				{
					if ( v[i].m_Pos.equal( v[j].m_Pos, tolerance) &&
						 v[i].m_Normal.equal( v[j].m_Normal, tolerance) &&
						 v[i].m_TCoords.equal( v[j].m_TCoords ) &&
						 v[i].m_Tangent.equal( v[j].m_Tangent, tolerance ) &&
						 v[i].m_Binormal.equal( v[j].m_Binormal, tolerance ) &&
						(v[i].m_Color == v[j].m_Color) )
					{
						redirects[i] = redirects[j];
						found = true;
						break;
					}
				}
				if (!found)
				{
					redirects[i] = buffer->m_Vertices.size();
					buffer->m_Vertices.push_back(v[i]);
				}
			}
			break;
		}
		default:
			GE_ERROR("Cannot create welded mesh, vertex type unsupported");
			break;
		}

		// write the buffer's index list
		eastl::vector<u16> &Indices = *outIdx;
		for (unsigned int i=0; i<indexCount; ++i)
		{
			Indices[i] = redirects[ indices[i] ];
		}
	}
	return clone;
}


//! Creates a copy of the mesh, which will only consist of VertexTangents vertices.
// not yet 32bit
IMesh* MeshManipulator::CreateMeshWithTangents(const IMesh* mesh, 
	bool recalculateNormals, bool smooth, bool angleWeighted, bool calculateTangents) const
{
	if (!mesh)
		return 0;

	// copy mesh and fill data into SMeshBufferTangents

	Mesh* clone = new Mesh();
	const unsigned int meshBufferCount = mesh->GetMeshBufferCount();

	for (unsigned int b=0; b<meshBufferCount; ++b)
	{
		shared_ptr<IMeshBuffer> original = mesh->GetMeshBuffer(b);
		const unsigned int idxCnt = original->GetIndexCount();
		const u16* idx = original->GetIndices();

		MeshBufferTangents* buffer = new MeshBufferTangents();

		buffer->m_Material = original->GetMaterial();
		eastl::map<VertexTangents, int> vertMap;
		int vertLocation;

		// copy vertices

		const E_VERTEX_TYPE vType = original->GetVertexType();
		VertexTangents vNew;
		for (unsigned int i=0; i<idxCnt; ++i)
		{
			switch(vType)
			{
				case EVT_STANDARD:
				{
					const VertexBuffer* v =
						(const VertexBuffer*)original->GetVertices();
					vNew = VertexTangents(v[idx[i]].m_Pos, v[idx[i]].m_Normal, 
						v[idx[i]].m_Color, v[idx[i]].m_TCoords);
				}
				break;
				case EVT_2TCOORDS:
				{
					const Vertex2TCoords* v =
						(const Vertex2TCoords*)original->GetVertices();
					vNew = VertexTangents(v[idx[i]].m_Pos, v[idx[i]].m_Normal, 
						v[idx[i]].m_Color, v[idx[i]].m_TCoords);
				}
				break;
				case EVT_TANGENTS:
				{
					const VertexTangents* v =
						(const VertexTangents*)original->GetVertices();
					vNew = v[idx[i]];
				}
				break;
			}
			eastl::map<VertexTangents, int>::iterator itNode = vertMap.find(vNew);
			if (itNode == vertMap.end())
			{
				vertLocation = buffer->m_Vertices.size();
				buffer->m_Vertices.push_back(vNew);
				vertMap.insert(eastl::pair<VertexTangents, int>(vNew, vertLocation));		
			}
			else vertLocation = itNode->second;

			// create new indices
			buffer->m_Indices.push_back(vertLocation);
		}
		buffer->RecalculateBoundingBox();

		// add new buffer
		clone->AddMeshBuffer(shared_ptr<IMeshBuffer>(buffer));
	}

	clone->RecalculateBoundingBox();
	if (calculateTangents)
		RecalculateTangents(clone, recalculateNormals, smooth, angleWeighted);

	return clone;
}


//! Creates a copy of the mesh, which will only consist of Vertex2TCoords vertices.
// not yet 32bit
IMesh* MeshManipulator::CreateMeshWith2TCoords(const IMesh* mesh) const
{
	if (!mesh)
		return 0;

	// copy mesh and fill data into SMeshBufferLightMap

	Mesh* clone = new Mesh();
	const unsigned int meshBufferCount = mesh->GetMeshBufferCount();

	for (unsigned int b=0; b<meshBufferCount; ++b)
	{
		shared_ptr<IMeshBuffer> original = mesh->GetMeshBuffer(b);
		const unsigned int idxCnt = original->GetIndexCount();
		const u16* idx = original->GetIndices();

		MeshBufferLightMap* buffer = new MeshBufferLightMap();
		buffer->m_Material = original->GetMaterial();

		eastl::map<Vertex2TCoords, int> vertMap;
		int vertLocation;

		// copy vertices

		const E_VERTEX_TYPE vType = original->GetVertexType();
		Vertex2TCoords vNew;
		for (unsigned int i=0; i<idxCnt; ++i)
		{
			switch(vType)
			{
				case EVT_STANDARD:
				{
					const VertexBuffer* v = (const VertexBuffer*)original->GetVertices();
					vNew = Vertex2TCoords(v[idx[i]].m_Pos, v[idx[i]].m_Normal, 
						v[idx[i]].m_Color, v[idx[i]].m_TCoords, v[idx[i]].m_TCoords);
				}
				break;
				case EVT_2TCOORDS:
				{
					const Vertex2TCoords* v =
						(const Vertex2TCoords*)original->GetVertices();
					vNew = v[idx[i]];
				}
				break;
				case EVT_TANGENTS:
				{
					const VertexTangents* v =
						(const VertexTangents*)original->GetVertices();
					vNew = Vertex2TCoords(v[idx[i]].m_Pos, v[idx[i]].m_Normal, 
						v[idx[i]].m_Color, v[idx[i]].m_TCoords, v[idx[i]].m_TCoords);
				}
				break;
			}
			eastl::map<Vertex2TCoords, int>::iterator itNode = vertMap.find(vNew);
			if (itNode == vertMap.end())
			{
				vertLocation = buffer->m_Vertices.size();
				buffer->m_Vertices.push_back(vNew);
				vertMap.insert(eastl::pair<Vertex2TCoords, int>(vNew, vertLocation));
			}
			else vertLocation = itNode->second;

			// create new indices
			buffer->m_Indices.push_back(vertLocation);
		}
		buffer->RecalculateBoundingBox();

		// add new buffer
		clone->AddMeshBuffer(shared_ptr<IMeshBuffer>(buffer));
	}

	clone->RecalculateBoundingBox();
	return clone;
}


//! Creates a copy of the mesh, which will only consist of VertexBuffer vertices.
// not yet 32bit
IMesh* MeshManipulator::CreateMeshWith1TCoords(const IMesh* mesh) const
{
	if (!mesh)
		return 0;

	// copy mesh and fill data into SMeshBuffer
	Mesh* clone = new Mesh();
	const unsigned int meshBufferCount = mesh->GetMeshBufferCount();

	for (unsigned int b=0; b<meshBufferCount; ++b)
	{
		shared_ptr<IMeshBuffer> original = mesh->GetMeshBuffer(b);
		const unsigned int idxCnt = original->GetIndexCount();
		const u16* idx = original->GetIndices();

		MeshBufferVertex* buffer = new MeshBufferVertex();
		buffer->m_Material = original->GetMaterial();

		eastl::map<VertexBuffer, int> vertMap;
		int vertLocation;

		// copy vertices
		const E_VERTEX_TYPE vType = original->GetVertexType();
		VertexBuffer vNew;
		for (unsigned int i=0; i<idxCnt; ++i)
		{
			switch(vType)
			{
				case EVT_STANDARD:
				{
					VertexBuffer* v = (VertexBuffer*)original->GetVertices();
					vNew = v[idx[i]];
				}
				break;

				case EVT_2TCOORDS:
				{
					Vertex2TCoords* v = (Vertex2TCoords*)original->GetVertices();
					vNew = VertexBuffer(v[idx[i]].m_Pos, v[idx[i]].m_Normal, v[idx[i]].m_Color, 
						v[idx[i]].m_TCoords);
				}
				break;

				case EVT_TANGENTS:
				{
					VertexTangents* v = (VertexTangents*)original->GetVertices();
					vNew = VertexBuffer(v[idx[i]].m_Pos, v[idx[i]].m_Normal, v[idx[i]].m_Color, 
						v[idx[i]].m_TCoords);
				}
				break;
			}
			eastl::map<VertexBuffer, int>::iterator itNode = vertMap.find(vNew);
			if (itNode == vertMap.end())
			{
				vertLocation = buffer->m_Vertices.size();
				buffer->m_Vertices.push_back(vNew);
				vertMap.insert(eastl::pair<VertexBuffer, int>(vNew, vertLocation));
			}
			else vertLocation = itNode->second;

			// create new indices
			buffer->m_Indices.push_back(vertLocation);
		}
		buffer->RecalculateBoundingBox();
		// add new buffer
		clone->AddMeshBuffer(shared_ptr<IMeshBuffer>(buffer));
	}

	clone->RecalculateBoundingBox();
	return clone;
}


//! Returns amount of polygons in mesh.
int MeshManipulator::GetPolyCount(IMesh* mesh) const
{
	if (!mesh)
		return 0;

	int trianglecount = 0;

	for (unsigned int g=0; g<mesh->GetMeshBufferCount(); ++g)
		trianglecount += mesh->GetMeshBuffer(g)->GetIndexCount() / 3;

	return trianglecount;
}


//! Returns amount of polygons in mesh.
int MeshManipulator::GetPolyCount(IAnimatedMesh* mesh) const
{
	if (mesh && mesh->GetFrameCount() != 0)
		return GetPolyCount(mesh->GetMesh(0).get());

	return 0;
}


//! create a new AnimatedMesh and adds the mesh to it
IAnimatedMesh* MeshManipulator::CreateAnimatedMesh(const shared_ptr<IMesh>& mesh, E_ANIMATED_MESH_TYPE type) const
{
	return new AnimatedMesh(mesh, type);
}

namespace
{

struct vcache
{
	eastl::vector<unsigned int> tris;
	float score;
	s16 cachepos;
	u16 NumActiveTris;
};

struct tcache
{
	u16 ind[3];
	float score;
	bool drawn;
};

const u16 cachesize = 32;

float FindVertexScore(vcache *v)
{
	const float CacheDecayPower = 1.5f;
	const float LastTriScore = 0.75f;
	const float ValenceBoostScale = 2.0f;
	const float ValenceBoostPower = 0.5f;
	const float MaxSizeVertexCache = 32.0f;

	if (v->NumActiveTris == 0)
	{
		// No tri needs this vertex!
		return -1.0f;
	}

	float Score = 0.0f;
	int CachePosition = v->cachepos;
	if (CachePosition < 0)
	{
		// Vertex is not in FIFO cache - no score.
	}
	else
	{
		if (CachePosition < 3)
		{
			// This vertex was used in the last triangle,
			// so it has a fixed score.
			Score = LastTriScore;
		}
		else
		{
			// Points for being high in the cache.
			const float Scaler = 1.0f / (MaxSizeVertexCache - 3);
			Score = 1.0f - (CachePosition - 3) * Scaler;
			Score = powf(Score, CacheDecayPower);
		}
	}

	// Bonus points for having a low number of tris still to
	// use the vert, so we get rid of lone verts quickly.
	float ValenceBoost = powf(v->NumActiveTris,
				-ValenceBoostPower);
	Score += ValenceBoostScale * ValenceBoost;

	return Score;
}

/*
	A specialized LRU cache for the Forsyth algorithm.
*/

class f_lru
{

public:
	f_lru(vcache *v, tcache *t): vc(v), tc(t)
	{
		for (u16 i = 0; i < cachesize; i++)
		{
			cache[i] = -1;
		}
	}

	// Adds this vertex index and returns the highest-scoring triangle index
	unsigned int add(u16 vert, bool updatetris = false)
	{
		bool found = false;

		// Mark existing pos as empty
		for (u16 i = 0; i < cachesize; i++)
		{
			if (cache[i] == vert)
			{
				// Move everything down
				for (u16 j = i; j; j--)
				{
					cache[j] = cache[j - 1];
				}

				found = true;
				break;
			}
		}

		if (!found)
		{
			if (cache[cachesize-1] != -1)
				vc[cache[cachesize-1]].cachepos = -1;

			// Move everything down
			for (u16 i = cachesize - 1; i; i--)
			{
				cache[i] = cache[i - 1];
			}
		}

		cache[0] = vert;

		unsigned int highest = 0;
		float hiscore = 0;

		if (updatetris)
		{
			// Update cache positions
			for (u16 i = 0; i < cachesize; i++)
			{
				if (cache[i] == -1)
					break;

				vc[cache[i]].cachepos = i;
				vc[cache[i]].score = FindVertexScore(&vc[cache[i]]);
			}

			// Update triangle scores
			for (u16 i = 0; i < cachesize; i++)
			{
				if (cache[i] == -1)
					break;

				const u16 trisize = vc[cache[i]].tris.size();
				for (u16 t = 0; t < trisize; t++)
				{
					tcache *tri = &tc[vc[cache[i]].tris[t]];

					tri->score =
						vc[tri->ind[0]].score +
						vc[tri->ind[1]].score +
						vc[tri->ind[2]].score;

					if (tri->score > hiscore)
					{
						hiscore = tri->score;
						highest = vc[cache[i]].tris[t];
					}
				}
			}
		}

		return highest;
	}

private:
	int cache[cachesize];
	vcache *vc;
	tcache *tc;
};

} // end anonymous namespace

/**
Vertex cache optimization according to the Forsyth paper:
http://home.comcast.net/~tom_forsyth/papers/fast_vert_cache_opt.html

The function is thread-safe (read: you can optimize several meshes in different threads)

\param mesh Source mesh for the operation.  */
IMesh* MeshManipulator::CreateForsythOptimizedMesh(const IMesh* mesh) const
{
	if (!mesh)
		return 0;

	Mesh *newmesh = new Mesh();
	newmesh->m_BoundingBox = mesh->GetBoundingBox();

	const unsigned int mbcount = mesh->GetMeshBufferCount();

	for (unsigned int b = 0; b < mbcount; ++b)
	{
		const shared_ptr<IMeshBuffer>& mb = mesh->GetMeshBuffer(b);

		if (mb->GetIndexType() != EIT_16BIT)
		{
			GE_ERROR("Cannot optimize a mesh with 32bit indices");
			SAFE_DELETE(newmesh);
			return 0;
		}

		const unsigned int icount = mb->GetIndexCount();
		const unsigned int tcount = icount / 3;
		const unsigned int vcount = mb->GetVertexCount();
		const u16 *ind = mb->GetIndices();

		vcache *vc = new vcache[vcount];
		tcache *tc = new tcache[tcount];

		f_lru lru(vc, tc);

		// init
		for (u16 i = 0; i < vcount; i++)
		{
			vc[i].score = 0;
			vc[i].cachepos = -1;
			vc[i].NumActiveTris = 0;
		}

		// First pass: count how many times a vert is used
		for (unsigned int i = 0; i < icount; i += 3)
		{
			vc[ind[i]].NumActiveTris++;
			vc[ind[i + 1]].NumActiveTris++;
			vc[ind[i + 2]].NumActiveTris++;

			const unsigned int tri_ind = i/3;
			tc[tri_ind].ind[0] = ind[i];
			tc[tri_ind].ind[1] = ind[i + 1];
			tc[tri_ind].ind[2] = ind[i + 2];
		}

		// Second pass: list of each triangle
		for (unsigned int i = 0; i < tcount; i++)
		{
			vc[tc[i].ind[0]].tris.push_back(i);
			vc[tc[i].ind[1]].tris.push_back(i);
			vc[tc[i].ind[2]].tris.push_back(i);

			tc[i].drawn = false;
		}

		// Give initial scores
		for (u16 i = 0; i < vcount; i++)
		{
			vc[i].score = FindVertexScore(&vc[i]);
		}
		for (unsigned int i = 0; i < tcount; i++)
		{
			tc[i].score =
					vc[tc[i].ind[0]].score +
					vc[tc[i].ind[1]].score +
					vc[tc[i].ind[2]].score;
		}

		switch(mb->GetVertexType())
		{
			case EVT_STANDARD:
			{
				VertexBuffer *v = (VertexBuffer *) mb->GetVertices();

				MeshBufferVertex* buf = new MeshBufferVertex();

				buf->m_Material = mb->GetMaterial();

				eastl::map<const VertexBuffer, const u16> sind; // search index for fast operation
				typedef eastl::map<const VertexBuffer, const u16>::iterator itsind;

				// Main algorithm
				unsigned int highest = 0;
				unsigned int drawcalls = 0;
				for (;;)
				{
					if (tc[highest].drawn)
					{
						bool found = false;
						float hiscore = 0;
						for (unsigned int t = 0; t < tcount; t++)
						{
							if (!tc[t].drawn)
							{
								if (tc[t].score > hiscore)
								{
									highest = t;
									hiscore = tc[t].score;
									found = true;
								}
							}
						}
						if (!found)
							break;
					}

					// Output the best triangle
					u16 newind = buf->m_Vertices.size();

					itsind s = sind.find(v[tc[highest].ind[0]]);

					if (s == sind.end())
					{
						buf->m_Vertices.push_back(v[tc[highest].ind[0]]);
						buf->m_Indices.push_back(newind);
						sind.insert(eastl::pair<const VertexBuffer, const u16>(v[tc[highest].ind[0]], newind));
						newind++;
					}
					else buf->m_Indices.push_back(s->second);

					s = sind.find(v[tc[highest].ind[1]]);

					if (s == sind.end())
					{
						buf->m_Vertices.push_back(v[tc[highest].ind[1]]);
						buf->m_Indices.push_back(newind);
						sind.insert(eastl::pair<const VertexBuffer, const u16>(v[tc[highest].ind[1]], newind));
						newind++;
					}
					else buf->m_Indices.push_back(s->second);

					s = sind.find(v[tc[highest].ind[2]]);

					if (s == sind.end())
					{
						buf->m_Vertices.push_back(v[tc[highest].ind[2]]);
						buf->m_Indices.push_back(newind);
						sind.insert(eastl::pair<const VertexBuffer, const u16>(v[tc[highest].ind[2]], newind));
					}
					else buf->m_Indices.push_back(s->second);

					vc[tc[highest].ind[0]].NumActiveTris--;
					vc[tc[highest].ind[1]].NumActiveTris--;
					vc[tc[highest].ind[2]].NumActiveTris--;

					tc[highest].drawn = true;

					for (u16 j = 0; j < 3; j++)
					{
						vcache *vert = &vc[tc[highest].ind[j]];
						eastl::vector<unsigned int>::iterator itVector = vert->tris.begin();
						for (; itVector < vert->tris.end(); itVector++)
						{
							if (highest == (*itVector))
							{
								vert->tris.erase(itVector);
								break;
							}
						}
					}

					lru.add(tc[highest].ind[0]);
					lru.add(tc[highest].ind[1]);
					highest = lru.add(tc[highest].ind[2], true);
					drawcalls++;
				}

				buf->SetBoundingBox(mb->GetBoundingBox());
				newmesh->AddMeshBuffer(shared_ptr<IMeshBuffer>(buf));
			}
			break;
			case EVT_2TCOORDS:
			{
				Vertex2TCoords *v = (Vertex2TCoords *) mb->GetVertices();

				MeshBufferLightMap* buf = new MeshBufferLightMap();
				buf->m_Material = mb->GetMaterial();

				eastl::map<const Vertex2TCoords, const u16> sind; // search index for fast operation
				typedef eastl::map<const Vertex2TCoords, const u16>::iterator itsind;

				// Main algorithm
				unsigned int highest = 0;
				unsigned int drawcalls = 0;
				for (;;)
				{
					if (tc[highest].drawn)
					{
						bool found = false;
						float hiscore = 0;
						for (unsigned int t = 0; t < tcount; t++)
						{
							if (!tc[t].drawn)
							{
								if (tc[t].score > hiscore)
								{
									highest = t;
									hiscore = tc[t].score;
									found = true;
								}
							}
						}
						if (!found)
							break;
					}

					// Output the best triangle
					u16 newind = buf->m_Vertices.size();

					itsind s = sind.find(v[tc[highest].ind[0]]);

					if (s == sind.end())
					{
						buf->m_Vertices.push_back(v[tc[highest].ind[0]]);
						buf->m_Indices.push_back(newind);
						sind.insert(eastl::pair<const Vertex2TCoords, const u16>(v[tc[highest].ind[0]], newind));
						newind++;
					}
					else buf->m_Indices.push_back(s->second);

					s = sind.find(v[tc[highest].ind[1]]);

					if (s == sind.end())
					{
						buf->m_Vertices.push_back(v[tc[highest].ind[1]]);
						buf->m_Indices.push_back(newind);
						sind.insert(eastl::pair<const Vertex2TCoords, const u16>(v[tc[highest].ind[1]], newind));
						newind++;
					}
					else buf->m_Indices.push_back(s->second);

					s = sind.find(v[tc[highest].ind[2]]);

					if (s == sind.end())
					{
						buf->m_Vertices.push_back(v[tc[highest].ind[2]]);
						buf->m_Indices.push_back(newind);
						sind.insert(eastl::pair<const Vertex2TCoords, const u16>(v[tc[highest].ind[2]], newind));
					}
					else buf->m_Indices.push_back(s->second);

					vc[tc[highest].ind[0]].NumActiveTris--;
					vc[tc[highest].ind[1]].NumActiveTris--;
					vc[tc[highest].ind[2]].NumActiveTris--;

					tc[highest].drawn = true;

					for (u16 j = 0; j < 3; j++)
					{
						vcache *vert = &vc[tc[highest].ind[j]];
						eastl::vector<unsigned int>::iterator itVector = vert->tris.begin();
						for (; itVector < vert->tris.end(); itVector++)
						{
							if (highest == (*itVector))
							{
								vert->tris.erase(itVector);
								break;
							}
						}
					}

					lru.add(tc[highest].ind[0]);
					lru.add(tc[highest].ind[1]);
					highest = lru.add(tc[highest].ind[2]);
					drawcalls++;
				}

				buf->SetBoundingBox(mb->GetBoundingBox());
				newmesh->AddMeshBuffer(shared_ptr<IMeshBuffer>(buf));

			}
			break;
			case EVT_TANGENTS:
			{
				VertexTangents *v = (VertexTangents *) mb->GetVertices();

				MeshBufferTangents* buf = new MeshBufferTangents();
				buf->m_Material = mb->GetMaterial();

				eastl::map<const VertexTangents, const u16> sind; // search index for fast operation
				typedef eastl::map<const VertexTangents, const u16>::iterator itsind;

				// Main algorithm
				unsigned int highest = 0;
				unsigned int drawcalls = 0;
				for (;;)
				{
					if (tc[highest].drawn)
					{
						bool found = false;
						float hiscore = 0;
						for (unsigned int t = 0; t < tcount; t++)
						{
							if (!tc[t].drawn)
							{
								if (tc[t].score > hiscore)
								{
									highest = t;
									hiscore = tc[t].score;
									found = true;
								}
							}
						}
						if (!found)
							break;
					}

					// Output the best triangle
					u16 newind = buf->m_Vertices.size();

					itsind s = sind.find(v[tc[highest].ind[0]]);

					if (s == sind.end())
					{
						buf->m_Vertices.push_back(v[tc[highest].ind[0]]);
						buf->m_Indices.push_back(newind);
						sind.insert(eastl::pair<const VertexTangents, const u16>(v[tc[highest].ind[0]], newind));
						newind++;
					}
					else buf->m_Indices.push_back(s->second);

					s = sind.find(v[tc[highest].ind[1]]);

					if (s == sind.end())
					{
						buf->m_Vertices.push_back(v[tc[highest].ind[1]]);
						buf->m_Indices.push_back(newind);
						sind.insert(eastl::pair<const VertexTangents, const u16>(v[tc[highest].ind[1]], newind));
						newind++;
					}
					else buf->m_Indices.push_back(s->second);

					s = sind.find(v[tc[highest].ind[2]]);

					if (s == sind.end())
					{
						buf->m_Vertices.push_back(v[tc[highest].ind[2]]);
						buf->m_Indices.push_back(newind);
						sind.insert(eastl::pair<const VertexTangents, const u16>(v[tc[highest].ind[2]], newind));
					}
					else buf->m_Indices.push_back(s->second);

					vc[tc[highest].ind[0]].NumActiveTris--;
					vc[tc[highest].ind[1]].NumActiveTris--;
					vc[tc[highest].ind[2]].NumActiveTris--;

					tc[highest].drawn = true;

					for (u16 j = 0; j < 3; j++)
					{
						vcache *vert = &vc[tc[highest].ind[j]];
						eastl::vector<unsigned int>::iterator itVector = vert->tris.begin();
						for (; itVector < vert->tris.end(); itVector++)
						{
							if (highest == (*itVector))
							{
								vert->tris.erase(itVector);
								break;
							}
						}
					}

					lru.add(tc[highest].ind[0]);
					lru.add(tc[highest].ind[1]);
					highest = lru.add(tc[highest].ind[2]);
					drawcalls++;
				}

				buf->SetBoundingBox(mb->GetBoundingBox());
				newmesh->AddMeshBuffer(shared_ptr<IMeshBuffer>(buf));
			}
			break;
		}

		SAFE_DELETE_ARRAY(vc);
		SAFE_DELETE_ARRAY(tc);

	} // for each meshbuffer

	return newmesh;
}