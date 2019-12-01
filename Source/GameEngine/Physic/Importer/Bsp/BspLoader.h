/*
===========================================================================
Copyright (C) 1999-2005 Id Software, Inc.

This file is part of Quake III Arena source code.

Quake III Arena source code is free software; you can redistribute it
and/or modify it under the terms of the GNU bteral Public License as
published by the Free Software Foundation; either version 2 of the License,
or (at your option) any later version.

Quake III Arena source code is distributed in the hope that it will be
useful, but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU bteral Public License for more details.

You should have received a copy of the GNU bteral Public License
along with Foobar; if not, write to the Free Software
Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
===========================================================================
*/

#ifndef BSPLOADER_H
#define BSPLOADER_H

#include "GameEngineStd.h"

#define	BSPMAXTOKEN	1024
#define	BSPMAX_KEY				32
#define	BSPMAX_VALUE			1024

#define	BSPCONTENTS_SOLID			1		// an eye is never valid in a solid
#define	BSPCONTENTS_LAVA			8
#define	BSPCONTENTS_SLIME			16
#define	BSPCONTENTS_WATER			32
#define	BSPCONTENTS_FOG				64

#define BSPCONTENTS_NOTTEAM1		0x0080
#define BSPCONTENTS_NOTTEAM2		0x0100
#define BSPCONTENTS_NOBOTCLIP		0x0200

#define	BSPCONTENTS_AREAPORTAL		0x8000

#define	BSPCONTENTS_PLAYERCLIP		0x10000
#define	BSPCONTENTS_MONSTERCLIP		0x20000
//bot specific contents types
#define	BSPCONTENTS_TELEPORTER		0x40000
#define	BSPCONTENTS_JUMPPAD			0x80000
#define BSPCONTENTS_CLUSTERPORTAL	0x100000
#define BSPCONTENTS_DONOTENTER		0x200000
#define BSPCONTENTS_BOTCLIP			0x400000
#define BSPCONTENTS_MOVER			0x800000

#define	BSPCONTENTS_ORIGIN			0x1000000	// removed before bsping an entity

#define	BSPCONTENTS_BODY			0x2000000	// should never be on a brush, only in game
#define	BSPCONTENTS_CORPSE			0x4000000
#define	BSPCONTENTS_DETAIL			0x8000000	// brushes not used for the bsp
#define	BSPCONTENTS_STRUCTURAL		0x10000000	// brushes used for the bsp
#define	BSPCONTENTS_TRANSLUCENT		0x20000000	// don't consume surface fragments inside
#define	BSPCONTENTS_TRIGGER			0x40000000
#define	BSPCONTENTS_NODROP			0x80000000	// don't leave bodies or items (death fog, lava)

#define	BSPSURF_NODAMAGE			0x1		// never give falling damage
#define	BSPSURF_SLICK				0x2		// effects game physics
#define	BSPSURF_SKY					0x4		// lighting from environment map
#define	BSPSURF_LADDER				0x8
#define	BSPSURF_NOIMPACT			0x10	// don't make missile explosions
#define	BSPSURF_NOMARKS				0x20	// don't leave missile marks
#define	BSPSURF_FLESH				0x40	// make flesh sounds and effects
#define	BSPSURF_NODRAW				0x80	// don't generate a drawsurface at all
#define	BSPSURF_HINT				0x100	// make a primary bsp splitter
#define	BSPSURF_SKIP				0x200	// completely ignore, allowing non-closed brushes
#define	BSPSURF_NOLIGHTMAP			0x400	// surface doesn't need a lightmap
#define	BSPSURF_POINTLIGHT			0x800	// generate lighting info at vertexes
#define	BSPSURF_METALSTEPS			0x1000	// clanking footsteps
#define	BSPSURF_NOSTEPS				0x2000	// no footstep sounds
#define	BSPSURF_NONSOLID			0x4000	// don't collide against curves with this set
#define	BSPSURF_LIGHTFILTER			0x8000	// act as a light filter during q3map -light
#define	BSPSURF_ALPHASHADOW			0x10000	// do per-pixel light shadow casting in q3map
#define	BSPSURF_NODLIGHT			0x20000	// don't dlight even if solid (solid lava, skies)
#define BSPSURF_DUST				0x40000 // leave a dust trail when walking on this surface


#define	BSPLUMP_ENTITIES		0
#define	BSPLUMP_SHADERS		1
#define	BSPLUMP_PLANES			2
#define	BSPLUMP_NODES			3
#define	BSPLUMP_LEAFS			4
#define	BSPLUMP_LEAFSURFACES	5
#define	BSPLUMP_LEAFBRUSHES	6
#define	LUMP_MODELS			7
#define	LUMP_BRUSHES		8
#define	LUMP_BRUSHSIDES		9
#define	LUMP_DRAWVERTS		10
#define	LUMP_DRAWINDEXES	11
#define	LUMP_SURFACES		13
#define	LUMP_LIGHTMAPS		14
#define	LUMP_LIGHTGRID		15
#define	LUMP_VISIBILITY		16
#define	HEADER_LUMPS		17
#define	MAX_QPATH		64

typedef struct {
	int fileofs, filelen;
} BSPLump;

typedef float BSPVector2[2];
typedef float BSPVector3[3];

typedef struct {
	int ident;
	int version;
	
	BSPLump lumps[HEADER_LUMPS];
} BSPHeader;

typedef struct {
	float mins[3], maxs[3];
	int firstSurface, numSurfaces;
	int firstBrush, numBrushes;
} BSPModel;

typedef struct {
	char shader[MAX_QPATH];
	int surfaceFlags;
	int contentFlags;
} BSPShader;

typedef struct {
	float normal[3];
	float dist;
} BSPPlane;

typedef struct {
	int planeNum;
	int children[2];
	int mins[3];
	int maxs[3];
} BSPNode;

typedef struct {
	int cluster;	
	int area;
	
	int mins[3];	
	int maxs[3];
	
	int firstLeafSurface;
	int numLeafSurfaces;
	
	int firstLeafBrush;
	int numLeafBrushes;
} BSPLeaf;

typedef struct {
	int planeNum;	
	int shaderNum;
} BSPBrushSide;

typedef struct {
	int firstSide;
	int numSides;
	int shaderNum;	
} BSPBrush;

typedef struct {
	BSPVector3	xyz;
	float		st[2];
	float		lightmap[2];
	BSPVector3	normal;
	unsigned char color[4];
} BSPVertice;

typedef struct BSPPair {
	struct BSPPair* next;
	char* key;
	char* value;
} BSPKeyValuePair;

typedef struct {
	BSPVector3 origin;
	struct bspbrush_s* brushes;
	struct parseMesh_s* patches;
	int firstDrawSurf;
	BSPKeyValuePair* epairs;
} BSPEntity;

typedef enum {
	MST_BAD,
	MST_PLANAR,
	MST_PATCH,
	MST_TRIANGLE_SOUP,
	MST_FLARE
} BSPMapSurface;

typedef struct {
	int shaderNum;
	int fogNum;
	int surfaceType;
	
	int firstVert;
	int numVerts;
	
	int firstIndex;
	int numIndexes;
	
	int lightmapNum;
	int lightmapX, lightmapY;
	int lightmapWidth, lightmapHeight;
	
	BSPVector3 lightmapOrigin;
	BSPVector3 lightmapVecs[3];
	
	int patchWidth;
	int patchHeight;
} BSPSurface;


struct vec2f
{
	//! Creates a quadratically interpolated vector between this and two other vectors.
	/** \param v2 Second vector to interpolate with.
	\param v3 Third vector to interpolate with (maximum at 1.0f)
	\param d Interpolation value between 0.0f (all this vector) and 1.0f (all the 3rd vector).
	Note that this is the opposite direction of interpolation to getInterpolated() and interpolate()
	\return An interpolated vector. This vector is not modified. */
	vec2f getInterpolatedQuadratic(const vec2f& v2, const vec2f& v3, float d) const
	{
		// this*(1-d)*(1-d) + 2 * v2 * (1-d) + v3 * d * d;
		const float inv = 1.f - d;
		const float mul0 = inv * inv;
		const float mul1 = 2.f * d * inv;
		const float mul2 = d * d;

		return vec2f((x * mul0 + v2.x * mul1 + v3.x * mul2),
			(y * mul0 + v2.y * mul1 + v3.y * mul2));
	}

	//! Default constructor (null vector).
	vec2f() : x(0.f), y(0.f) {}
	//! Constructor with the same value for all elements
	vec2f(float n) : x(n), y(n) {}
	//! Constructor with three different values
	vec2f(float nx, float ny) : x(nx), y(ny) {}

	float x, y;
};

struct vec3f
{
	//! Normalizes the vector.
	/** In case of the 0 vector the result is still 0, otherwise
	the length of the vector will be 1.
	\return Reference to this vector after normalization. */
	vec3f& normalize()
	{
		float length = x * x + y * y + z * z;
		if (length == 0) // this check isn't an optimization but prevents getting NAN in the sqrt.
			return *this;
		length = 1.f / sqrt(length);

		x = (x * length);
		y = (y * length);
		z = (z * length);
		return *this;
	}

	//! Creates a quadratically interpolated vector between this and two other vectors.
	/** \param v2 Second vector to interpolate with.
	\param v3 Third vector to interpolate with (maximum at 1.0f)
	\param d Interpolation value between 0.0f (all this vector) and 1.0f (all the 3rd vector).
	Note that this is the opposite direction of interpolation to getInterpolated() and interpolate()
	\return An interpolated vector. This vector is not modified. */
	vec3f getInterpolatedQuadratic(const vec3f& v2, const vec3f& v3, float d) const
	{
		// this*(1-d)*(1-d) + 2 * v2 * (1-d) + v3 * d * d;
		const float inv = 1.f - d;
		const float mul0 = inv * inv;
		const float mul1 = 2.f * d * inv;
		const float mul2 = d * d;

		return vec3f((x * mul0 + v2.x * mul1 + v3.x * mul2),
			(y * mul0 + v2.y * mul1 + v3.y * mul2),
			(z * mul0 + v2.z * mul1 + v3.z * mul2));
	}

	//! Default constructor (null vector).
	vec3f() : x(0.f), y(0.f), z(0.f) {}
	//! Constructor with the same value for all elements
	vec3f(float n) : x(n), y(n), z(n) {}
	//! Constructor with three different values
	vec3f(float nx, float ny, float nz) : x(nx), y(ny), z(nz) {}

	float x, y, z;
};

//! struct representing a 32 bit ARGB color.
/** The color values for alpha, red, green, and blue are
stored in a single u32. So all four values may be between 0 and 255.
Alpha is opacity, so 0 is fully transparent, 255 is fully opaque (solid).
This class is used by most parts of the Engine
to specify a color. Another way is using the class SColorf, which
stores the color values in 4 floats.
This class must consist of only one u32 and must not use virtual functions.
*/
class color
{
public:

	//! Constructor of the Color. Does nothing.
	/** The color value is not initialized to save time. */
	color() {}

	//! Constructs the color from 4 values representing the alpha, red, green and blue component.
	/** Must be values between 0 and 255. */
	color(unsigned int a, unsigned int r, unsigned int g, unsigned int b)
		: data(((a & 0xff) << 24) | ((r & 0xff) << 16) | ((g & 0xff) << 8) | (b & 0xff)) {}

	//! Constructs the color from a 32 bit value. Could be another color.
	color(unsigned int clr) : data(clr) {}

	//! Returns the alpha component of the color.
	/** The alpha component defines how opaque a color is.
	\return The alpha value of the color. 0 is fully transparent, 255 is fully opaque. */
	unsigned int getAlpha() const { return data >> 24; }

	//! Returns the red component of the color.
	/** \return Value between 0 and 255, specifying how red the color is.
	0 means no red, 255 means full red. */
	unsigned int getRed() const { return (data >> 16) & 0xff; }

	//! Returns the green component of the color.
	/** \return Value between 0 and 255, specifying how green the color is.
	0 means no green, 255 means full green. */
	unsigned int getGreen() const { return (data >> 8) & 0xff; }

	//! Returns the blue component of the color.
	/** \return Value between 0 and 255, specifying how blue the color is.
	0 means no blue, 255 means full blue. */
	unsigned int getBlue() const { return data & 0xff; }

	//! Sets all four components of the color at once.
	/** Constructs the color from 4 values representing the alpha,
	red, green and blue components of the color. Must be values
	between 0 and 255.
	\param a: Alpha component of the color. The alpha component
	defines how transparent a color should be. Has to be a value
	between 0 and 255. 255 means not transparent (opaque), 0 means
	fully transparent.
	\param r: Sets the red component of the Color. Has to be a
	value between 0 and 255. 0 means no red, 255 means full red.
	\param g: Sets the green component of the Color. Has to be a
	value between 0 and 255. 0 means no green, 255 means full
	green.
	\param b: Sets the blue component of the Color. Has to be a
	value between 0 and 255. 0 means no blue, 255 means full blue. */
	void set(unsigned int a, unsigned int r, unsigned int g, unsigned int b)
	{
		data = (((a & 0xff) << 24) | ((r & 0xff) << 16) | ((g & 0xff) << 8) | (b & 0xff));
	}
	void set(unsigned int col) { data = col; }

	//! Compares the color to another color.
	/** \return True if the colors are the same, and false if not. */
	bool operator==(const color& other) const { return other.data == data; }

	//! Compares the color to another color.
	/** \return True if the colors are different, and false if they are the same. */
	bool operator!=(const color& other) const { return other.data != data; }

	//! comparison operator
	/** \return True if this color is smaller than the other one */
	bool operator<(const color& other) const { return (data < other.data); }

	//! Adds two colors, result is clamped to 0..255 values
	/** \param other Color to add to this color
	\return Addition of the two colors, clamped to 0..255 values */
	color operator+(const color& other) const
	{
		return color(
			eastl::min(getAlpha() + other.getAlpha(), 255u),
			eastl::min(getRed() + other.getRed(), 255u),
			eastl::min(getGreen() + other.getGreen(), 255u),
			eastl::min(getBlue() + other.getBlue(), 255u));
	}

	//! color in A8R8G8B8 Format
	unsigned int data;
};

struct colorf
{
	//! Converts this color to a SColor without floats.
	color toSColor() const
	{
		return color((unsigned int)round(a*255.0f),
			(unsigned int)round(r*255.0f),
			(unsigned int)round(g*255.0f),
			(unsigned int)round(b*255.0f));
	}

	//! Sets all four color components to new values at once.
	/** \param aa: Alpha component. Should be a value between 0.0f meaning
	fully transparent and 1.0f, meaning opaque.
	\param rr: Red color component. Should be a value between 0.0f meaning
	no red and 1.0f, meaning full red.
	\param gg: Green color component. Should be a value between 0.0f meaning
	no green and 1.0f, meaning full green.
	\param bb: Blue color component. Should be a value between 0.0f meaning
	no blue and 1.0f, meaning full blue. */
	void set(float aa, float rr, float gg, float bb) { a = aa; r = rr; g = gg; b = bb; }

	//! Returns interpolated color. ( quadratic )
	/** \param c1: first color to interpolate with
	\param c2: second color to interpolate with
	\param d: value between 0.0f and 1.0f. */
	colorf getInterpolatedQuadratic(const colorf& c1, const colorf& c2, float d) const
	{
		d = eastl::min(eastl::max(d, 0.f), 1.f);
		// this*(1-d)*(1-d) + 2 * c1 * (1-d) + c2 * d * d;
		const float inv = 1.f - d;
		const float mul0 = inv * inv;
		const float mul1 = 2.f * d * inv;
		const float mul2 = d * d;

		return colorf(r * mul0 + c1.r * mul1 + c2.r * mul2,
			g * mul0 + c1.g * mul1 + c2.g * mul2,
			b * mul0 + c1.b * mul1 + c2.b * mul2,
			a * mul0 + c1.a * mul1 + c2.a * mul2);
	}

	//! Default constructor for SColorf.
	/** Sets red, green and blue to 0.0f and alpha to 1.0f. */
	colorf() : r(0.0f), g(0.0f), b(0.0f), a(1.0f) {}

	//! Constructs a color from up to four color values: red, green, blue, and alpha.
	/** \param r: Red color component. Should be a value between
	0.0f meaning no red and 1.0f, meaning full red.
	\param g: Green color component. Should be a value between 0.0f
	meaning no green and 1.0f, meaning full green.
	\param b: Blue color component. Should be a value between 0.0f
	meaning no blue and 1.0f, meaning full blue.
	\param a: Alpha color component of the color. The alpha
	component defines how transparent a color should be. Has to be
	a value between 0.0f and 1.0f, 1.0f means not transparent
	(opaque), 0.0f means fully transparent. */
	colorf(float r, float g, float b, float a = 1.0f) : r(r), g(g), b(b), a(a) {}

	float r, g, b, a;
};

struct S3DVertex2TCoords
{
	vec3f vPosition;    ///< Position of vertex
	vec2f vTexCoord;    ///< (u,v) Texturecoordinate of detailtexture
	vec2f vLightmap;    ///< (u,v) Texturecoordinate of lightmap
	vec3f vNormal;      ///< vertex normale
	colorf vColor;      ///< Color in RGBA

	S3DVertex2TCoords() {}
	S3DVertex2TCoords(const vec3f& pos, const vec3f& normal, colorf color,
		const vec2f& texCoords, const vec2f& lightMap)
		: vPosition(pos), vNormal(normal), vColor(color), vTexCoord(texCoords), vLightmap(lightMap) {}

	S3DVertex2TCoords getInterpolatedQuadratic(const S3DVertex2TCoords& v2,
		const S3DVertex2TCoords& v3, const float d) const
	{
		return S3DVertex2TCoords(
			vPosition.getInterpolatedQuadratic(v2.vPosition, v3.vPosition, d),
			vNormal.getInterpolatedQuadratic(v2.vNormal, v3.vNormal, d),
			vColor.getInterpolatedQuadratic(v2.vColor, v3.vColor, (float)d),
			vTexCoord.getInterpolatedQuadratic(v2.vTexCoord, v3.vTexCoord, d),
			vLightmap.getInterpolatedQuadratic(v2.vLightmap, v3.vLightmap, d));
	}
};

inline void copy(S3DVertex2TCoords * dest, const BSPVertice * source)
{
	dest->vPosition.x = source->xyz[0];
	dest->vPosition.y = source->xyz[1];
	dest->vPosition.z = source->xyz[2];

	dest->vNormal.x = source->normal[0];
	dest->vNormal.y = source->normal[1];
	dest->vNormal.z = source->normal[2];
	dest->vNormal.normalize();

	dest->vTexCoord.x = source->st[0];
	dest->vTexCoord.y = source->st[1];
	dest->vLightmap.x = source->lightmap[0];
	dest->vLightmap.y = source->lightmap[1];

	unsigned char a = source->color[3];
	unsigned char r = eastl::min(source->color[0] * 4, 255);
	unsigned char g = eastl::min(source->color[1] * 4, 255);
	unsigned char b = eastl::min(source->color[2] * 4, 255);

	dest->vColor.set(a * 1.f / 255.f, r * 1.f / 255.f,
		g * 1.f / 255.f, b * 1.f / 255.f);
}

struct SBezier
{
	//! Vertices of this buffer
	eastl::vector<S3DVertex2TCoords> vertices;
	//! Indices into the vertices of this buffer.
	eastl::vector<unsigned int> indices;
	//controls
	S3DVertex2TCoords control[9];

	void tesselate(int level)
	{
		//Calculate how many vertices across/down there are
		int j, k;

		column[0].resize(level + 1);
		column[1].resize(level + 1);
		column[2].resize(level + 1);

		const float w = 0.f + (1.f / (float)level);

		//Tesselate along the columns
		for (j = 0; j <= level; ++j)
		{
			const float f = w * (float)j;

			column[0][j] = control[0].getInterpolatedQuadratic(control[3], control[6], f);
			column[1][j] = control[1].getInterpolatedQuadratic(control[4], control[7], f);
			column[2][j] = control[2].getInterpolatedQuadratic(control[5], control[8], f);
		}

		const unsigned int idx = vertices.size();
		//Tesselate across the rows to get final vertices
		S3DVertex2TCoords v;
		for (j = 0; j <= level; ++j)
		{
			for (k = 0; k <= level; ++k)
			{
				v = column[0][j].getInterpolatedQuadratic(column[1][j], column[2][j], w * (float)k);
				vertices.push_back(v);
			}
		}

		// connect
		for (j = 0; j < level; ++j)
		{
			for (k = 0; k < level; ++k)
			{
				const int inx = idx + (k * (level + 1)) + j;

				indices.push_back(inx + 0);
				indices.push_back(inx + (level + 1) + 0);
				indices.push_back(inx + (level + 1) + 1);

				indices.push_back(inx + 0);
				indices.push_back(inx + (level + 1) + 1);
				indices.push_back(inx + 1);
			}
		}
	}

private:
	int	level;

	eastl::vector<S3DVertex2TCoords> column[3];

};

///GPL code from IdSofware to parse a Quake 3 BSP file
///check that your platform define __BIG_ENDIAN__ correctly (in BspLoader.cpp)
class BspLoader
{
	int mEndianness;

public:

	BspLoader();

	bool LoadBSPFile(void* memoryBuffer);

	const char* GetValueForKey(const BSPEntity *ent, const char *key) const;

	bool GetVectorForKey(const BSPEntity *ent, const char *key, BSPVector3 vec);

	float GetFloatForKey(const BSPEntity *ent, const char *key);

	void ParseEntities(void);

	bool FindVectorByName(float* outvec, const char* name);

	const BSPEntity * GetEntityByValue(const char* name, const char* value);

protected:

	void ParseFromMemory(char *buffer, int size);

	bool IsEndOfScript(bool crossline);

	bool GetToken(bool crossline);

	char* CopyString(const char *s);

	void StripTrailing(char *e);

	BSPKeyValuePair* ParseEpair(void);

	bool ParseEntity(void);

	short IsLittleShort(short l);
	int IsLittleLong(int l);
	float IsLittleFloat(float l);

	int IsBigLong(int l);
	short IsBigShort(short l);
	float IsBigFloat(float l);

	void SwapBlock(int *block, int sizeOfBlock);

	int CopyLump(BSPHeader	*header, int lump, void *dest, int size);

	void SwapBSPFile(void);

public: //easier for conversion

	int mNumEntities;
	eastl::vector<BSPEntity> mEntities;

	int mNumModels;
	eastl::vector<BSPModel> mDModels;

	int mNumShaders;
	eastl::vector<BSPShader> mDShaders;

	int mEntDataSize;
	eastl::vector<char> mDentData;

	int mNumLeafs;
	eastl::vector<BSPLeaf> mDLeafs;

	int mNumPlanes;
	eastl::vector<BSPPlane> mDPlanes;

	int mNumNodes;
	eastl::vector<BSPNode> mDNodes;

	int mNumLeafSurfaces;
	eastl::vector<int> mDLeafSurfaces;

	int mNumLeafBrushes;
	eastl::vector<int> mDLeafBrushes;

	int mNumBrushes;
	eastl::vector<BSPBrush> mDBrushes;

	int mNumBrushsides;
	eastl::vector<BSPBrushSide> mDBrushsides;

	int mNumLightBytes;
	eastl::vector<unsigned char> mLightBytes;

	int mNumGridPoints;
	eastl::vector<unsigned char> mGridData;

	int mNumVisBytes;
	eastl::vector<unsigned char> mVisBytes;

	int mNumDrawIndexes;
	eastl::vector<int> mDrawIndexes;

	int mNumDrawSurfaces;
	eastl::vector<BSPSurface> mDrawSurfaces;

	int mNumDrawVertices;
	eastl::vector<BSPVertice> mDrawVertices;

	enum
	{
		BSP_LITTLE_ENDIAN = 0,
		BSP_BIG_ENDIAN = 1
	};

	//returns machines big endian / little endian
	int GetMachineEndianness();

	inline int MachineEndianness()
	{
		return mEndianness;
	}
};

#endif //BSPLOADER_H
