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

#include "BspLoader.h"

#include "Core/Logger/Logger.h"

typedef struct
{
	char	filename[1024];
	char    *buffer,*script_p,*end_p;
	int     line;
} BSPScript;

#define	MAX_INCLUDES	8
BSPScript	scriptstack[MAX_INCLUDES];
BSPScript	*script;
int			scriptline;

char    token[BSPMAXTOKEN];
bool endofscript;
bool tokenready;                     // only true if UnGetToken was just called

//
//loadBSPFile
//

int extrasize = 100;

BspLoader::BspLoader()
	:mNumEntities(0)
{
	mEndianness = GetMachineEndianness();
	if (mEndianness == BSP_BIG_ENDIAN)
	{
		LogInformation("Machine is BIG_ENDIAN");
	} else
	{
		LogInformation("Machine is Little Endian");
	}
}


bool BspLoader::LoadBSPFile( void* memoryBuffer) {
	
	BSPHeader *header = (BSPHeader*) memoryBuffer;

	// load the file header
	if (header)
	{
		// swap the header
		SwapBlock( (int *)header, sizeof(*header) );
		
		int length = (header->lumps[BSPLUMP_SHADERS].filelen) / sizeof(BSPShader);
		mDShaders.resize(length+extrasize);
		mNumShaders = CopyLump( header, BSPLUMP_SHADERS, &mDShaders[0], sizeof(BSPShader) );

		length = (header->lumps[LUMP_MODELS].filelen) / sizeof(BSPModel);
		mDModels.resize(length+extrasize);
		mNumModels = CopyLump( header, LUMP_MODELS, &mDModels[0], sizeof(BSPModel) );

		length = (header->lumps[BSPLUMP_PLANES].filelen) / sizeof(BSPPlane);
		mDPlanes.resize(length+extrasize);
		mNumPlanes = CopyLump( header, BSPLUMP_PLANES, &mDPlanes[0], sizeof(BSPPlane) );

		length = (header->lumps[BSPLUMP_LEAFS].filelen) / sizeof(BSPLeaf);
		mDLeafs.resize(length+extrasize);
		mNumLeafs = CopyLump( header, BSPLUMP_LEAFS, &mDLeafs[0], sizeof(BSPLeaf) );

		length = (header->lumps[BSPLUMP_NODES].filelen) / sizeof(BSPNode);
		mDNodes.resize(length+extrasize);
		mNumNodes = CopyLump( header, BSPLUMP_NODES, &mDNodes[0], sizeof(BSPNode) );

		length = (header->lumps[BSPLUMP_LEAFSURFACES].filelen) / sizeof(mDLeafSurfaces[0]);
		mDLeafSurfaces.resize(length+extrasize);
		mNumLeafSurfaces = CopyLump( header, BSPLUMP_LEAFSURFACES, &mDLeafSurfaces[0], sizeof(mDLeafSurfaces[0]) );

		length = (header->lumps[BSPLUMP_LEAFBRUSHES].filelen) / sizeof(mDLeafBrushes[0]) ;
		mDLeafBrushes.resize(length+extrasize);
		mNumLeafBrushes = CopyLump( header, BSPLUMP_LEAFBRUSHES, &mDLeafBrushes[0], sizeof(mDLeafBrushes[0]) );

		length = (header->lumps[LUMP_BRUSHES].filelen) / sizeof(BSPBrush);
		mDBrushes.resize(length+extrasize);
		mNumBrushes = CopyLump( header, LUMP_BRUSHES, &mDBrushes[0], sizeof(BSPBrush) );

		length = (header->lumps[LUMP_BRUSHSIDES].filelen) / sizeof(BSPBrushSide);
		mDBrushsides.resize(length+extrasize);
		mNumBrushsides = CopyLump( header, LUMP_BRUSHSIDES, &mDBrushsides[0], sizeof(BSPBrushSide) );

		length = (header->lumps[LUMP_SURFACES].filelen) / sizeof(BSPSurface);
		mDrawSurfaces.resize(length+extrasize);
		mNumDrawSurfaces = CopyLump( header, LUMP_SURFACES, &mDrawSurfaces[0], sizeof(BSPSurface) );

		length = (header->lumps[LUMP_DRAWVERTS].filelen) / sizeof(BSPVertice);
		mDrawVertices.resize(length + extrasize);
		mNumDrawVertices = CopyLump(header, LUMP_DRAWVERTS, &mDrawVertices[0], sizeof(BSPVertice));

		length = (header->lumps[LUMP_DRAWINDEXES].filelen) / sizeof(mDrawIndexes[0]);
		mDrawIndexes.resize(length+extrasize);
		mNumDrawIndexes = CopyLump( header, LUMP_DRAWINDEXES, &mDrawIndexes[0], sizeof(mDrawIndexes[0]) );

		length = (header->lumps[LUMP_VISIBILITY].filelen) / 1;
		mVisBytes.resize(length+extrasize);
		mNumVisBytes = CopyLump( header, LUMP_VISIBILITY, &mVisBytes[0], 1 );

		length = (header->lumps[LUMP_LIGHTMAPS].filelen) / 1;
		mLightBytes.resize(length+extrasize);
		mNumLightBytes = CopyLump( header, LUMP_LIGHTMAPS, &mLightBytes[0], 1 );

		length = (header->lumps[BSPLUMP_ENTITIES].filelen) / 1;
		mDentData.resize(length+extrasize);
		mEntDataSize = CopyLump( header, BSPLUMP_ENTITIES, &mDentData[0], 1);

		length = (header->lumps[LUMP_LIGHTGRID].filelen) / 1;
		mGridData.resize(length+extrasize);
		mNumGridPoints = CopyLump( header, LUMP_LIGHTGRID, &mGridData[0], 8 );

		// swap everything
		SwapBSPFile();

		return true;

	}
	return false;
}


const char* BspLoader::GetValueForKey( const  BSPEntity* ent, const char* key ) const 
{
	const BSPKeyValuePair* ep;
	
	for (ep=ent->epairs ; ep ; ep=ep->next) 
	{
		if (!strcmp(ep->key, key) )
		{
			return ep->value;
		}
	}
	return "";
}

float BspLoader::GetFloatForKey( const BSPEntity *ent, const char *key ) 
{
	const char	*k;
	
	k = GetValueForKey( ent, key );
	return float(atof(k));
}

bool BspLoader::GetVectorForKey( const BSPEntity *ent, const char *key, BSPVector3 vec ) 
{
	const char	*k;
	k = GetValueForKey (ent, key);
	if (strcmp(k, ""))
	{
		sscanf (k, "%f %f %f", &vec[0], &vec[1], &vec[2]);
		return true;
	}
	return false;
}

/*
==============
parseFromMemory
==============
*/
void BspLoader::ParseFromMemory (char *buffer, int size)
{
	script = scriptstack;
	script++;
	if (script == &scriptstack[MAX_INCLUDES])
	{
		//printf("script file exceeded MAX_INCLUDES");
	}
	strcpy (script->filename, "memory buffer" );

	script->buffer = buffer;
	script->line = 1;
	script->script_p = script->buffer;
	script->end_p = script->buffer + size;

	endofscript = false;
	tokenready = false;
}


bool BspLoader::IsEndOfScript (bool crossline)
{
	if (!crossline)
		//printf("Line %i is incomplete\n",scriptline);

	if (!strcmp (script->filename, "memory buffer"))
	{
		endofscript = true;
		return false;
	}

	//free (script->buffer);
	if (script == scriptstack+1)
	{
		endofscript = true;
		return false;
	}
	script--;
	scriptline = script->line;
	//printf ("returning to %s\n", script->filename);
	return GetToken (crossline);
}

/*
==============
getToken
==============
*/
bool BspLoader::GetToken (bool crossline)
{
	char    *token_p;

	if (tokenready) // is a token allready waiting?
	{
		tokenready = false;
		return true;
	}

	if (script->script_p >= script->end_p)
		return IsEndOfScript (crossline);

//
// skip space
//
skipspace:
	while (*script->script_p <= 32)
	{
		if (script->script_p >= script->end_p)
			return IsEndOfScript (crossline);
		if (*script->script_p++ == '\n')
		{
			if (!crossline)
			{
				//printf("Line %i is incomplete\n",scriptline);
			}
			scriptline = script->line++;
		}
	}

	if (script->script_p >= script->end_p)
		return IsEndOfScript (crossline);

	// ; # // comments
	if (*script->script_p == ';' || *script->script_p == '#'
		|| ( script->script_p[0] == '/' && script->script_p[1] == '/') )
	{
		if (!crossline)
		{
			//printf("Line %i is incomplete\n",scriptline);
		}
		while (*script->script_p++ != '\n')
			if (script->script_p >= script->end_p)
				return IsEndOfScript (crossline);
		scriptline = script->line++;
		goto skipspace;
	}

	// /* */ comments
	if (script->script_p[0] == '/' && script->script_p[1] == '*')
	{
		if (!crossline)
		{
			//printf("Line %i is incomplete\n",scriptline);
		}
		script->script_p+=2;
		while (script->script_p[0] != '*' && script->script_p[1] != '/')
		{
			if ( *script->script_p == '\n' ) {
				scriptline = script->line++;
			}
			script->script_p++;
			if (script->script_p >= script->end_p)
				return IsEndOfScript (crossline);
		}
		script->script_p += 2;
		goto skipspace;
	}

//
// copy token
//
	token_p = token;

	if (*script->script_p == '"')
	{
		// quoted token
		script->script_p++;
		while (*script->script_p != '"')
		{
			*token_p++ = *script->script_p++;
			if (script->script_p == script->end_p)
				break;
			if (token_p == &token[BSPMAXTOKEN])
			{
				//printf ("Token too large on line %i\n",scriptline);
			}
		}
		script->script_p++;
	}
	else	// regular token
	{
		while (*script->script_p > 32 && *script->script_p != ';')
		{
			*token_p++ = *script->script_p++;
			if (script->script_p == script->end_p)
				break;
			if (token_p == &token[BSPMAXTOKEN])
			{
				//printf ("Token too large on line %i\n",scriptline);
			}
		}
	}

	*token_p = 0;

	if (!strcmp (token, "$include"))
	{
		//getToken (false);
		//AddScriptToStack (token);
		return false;//getToken (crossline);
	}

	return true;
}

char *BspLoader::CopyString(const char *s)
{
	char	*b;
	b = (char*) malloc( strlen(s)+1);
	strcpy (b, s);
	return b;
}

void BspLoader::StripTrailing( char *e ) 
{
	char	*s;

	s = e + strlen(e)-1;
	while (s >= e && *s <= 32)
	{
		*s = 0;
		s--;
	}
}

/*
=================
parseEpair
=================
*/
BSPKeyValuePair *BspLoader::ParseEpair( void ) 
{
	BSPKeyValuePair	*e;

	e = (struct BSPPair*) malloc( sizeof(BSPKeyValuePair));
	memset( e, 0, sizeof(BSPKeyValuePair) );
	
	if ( strlen(token) >= BSPMAX_KEY-1 ) 
	{
		//printf ("ParseEpar: token too long");
	}
	e->key = CopyString( token );
	GetToken( false );
	if ( strlen(token) >= BSPMAX_VALUE-1 ) 
	{

		//printf ("ParseEpar: token too long");
	}
	e->value = CopyString( token );

	// strip trailing spaces that sometimes get accidentally
	// added in the editor
	StripTrailing( e->key );
	StripTrailing( e->value );

	return e;
}

/*
================
parseEntity
================
*/
bool BspLoader::ParseEntity( void ) 
{
	BSPKeyValuePair		*e;
	BSPEntity	*mapent;

	if ( !GetToken (true) ) {
		return false;
	}

	if ( strcmp (token, "{") ) {

		//printf ("parseEntity: { not found");
	}

	BSPEntity bla;
	bla.brushes = 0;
	bla.epairs = 0;
	bla.firstDrawSurf = 0;
	bla.origin[0] = 0.f;
	bla.origin[1] = 0.f;
	bla.origin[2] = 0.f;
	bla.patches = 0;

	mEntities.push_back(bla);
	mapent = &mEntities[mEntities.size()-1];
	mNumEntities++;

	do 
	{
		if ( !GetToken (true) ) 
		{
			//printf("parseEntity: EOF without closing brace");
		}
		if ( !strcmp (token, "}") ) 
		{
			break;
		}
		e = (struct BSPPair*)ParseEpair ();
		e->next = mapent->epairs;
		mapent->epairs = e;
	} 
	while (1);
	
	return true;
}

/*
================
parseEntities

Parses the dentdata string into entities
================
*/
void BspLoader::ParseEntities( void ) 
{
	mNumEntities = 0;
	mEntities.clear();

	ParseFromMemory( &mDentData[0], mEntDataSize );

	while ( ParseEntity () ) 
	{
	
	}	
}

int BspLoader::GetMachineEndianness()
{
   long int i = 1;
   const char *p = (const char *) &i;
   if (p[0] == 1)  // Lowest address contains the least significant byte
	   return BSP_LITTLE_ENDIAN;
   else
	   return BSP_BIG_ENDIAN;
}

short BspLoader::IsLittleShort (short l)
{
	if (MachineEndianness() == BSP_BIG_ENDIAN)
	{
		unsigned char    b1,b2;

		b1 = l&255;
		b2 = (l>>8)&255;

		return (b1<<8) + b2;
	}
	//little endian
	return l;
}

short BspLoader::IsBigShort (short l)
{
	if (MachineEndianness() == BSP_BIG_ENDIAN)
	{
		return l;
	}

	unsigned char   b1,b2;

	b1 = l&255;
	b2 = (l>>8)&255;

	return (b1<<8) + b2;
}

int BspLoader::IsLittleLong (int l)
{
	if (MachineEndianness() == BSP_BIG_ENDIAN)
	{
		unsigned char    b1,b2,b3,b4;

		b1 = l&255;
		b2 = (l>>8)&255;
		b3 = (l>>16)&255;
		b4 = (l>>24)&255;

		return ((int)b1<<24) + ((int)b2<<16) + ((int)b3<<8) + b4;
	}

	//little endian
	return l;
}

int BspLoader::IsBigLong (int l)
{
	if (MachineEndianness() == BSP_BIG_ENDIAN)
	{
		return l;
	}

	unsigned char    b1,b2,b3,b4;

	b1 = l&255;
	b2 = (l>>8)&255;
	b3 = (l>>16)&255;
	b4 = (l>>24)&255;

	return ((int)b1<<24) + ((int)b2<<16) + ((int)b3<<8) + b4;
}

float BspLoader::IsLittleFloat (float l)
{
	if (MachineEndianness() == BSP_BIG_ENDIAN)
	{
		union {unsigned char b[4]; float f;} in, out;
		
		in.f = l;
		out.b[0] = in.b[3];
		out.b[1] = in.b[2];
		out.b[2] = in.b[1];
		out.b[3] = in.b[0];
		
		return out.f;
	}

	//little endian
	return l;
}

float BspLoader::IsBigFloat (float l)
{
	if (MachineEndianness() == BSP_BIG_ENDIAN)
	{
		return l;
	}
	//little endian
	union {unsigned char b[4]; float f;} in, out;
	
	in.f = l;
	out.b[0] = in.b[3];
	out.b[1] = in.b[2];
	out.b[2] = in.b[1];
	out.b[3] = in.b[0];
	
	return out.f;
}

//
// swapBlock
// If all values are 32 bits, this can be used to swap everything
//

void BspLoader::SwapBlock( int *block, int sizeOfBlock ) 
{
	int	i;

	sizeOfBlock >>= 2;
	for ( i = 0 ; i < sizeOfBlock ; i++ ) 
	{
		block[i] = IsLittleLong( block[i] );
	}
}

//
// copyLump
//

int BspLoader::CopyLump( BSPHeader	*header, int lump, void *dest, int size ) 
{
	int	length, ofs;

	length = header->lumps[lump].filelen;
	ofs = header->lumps[lump].fileofs;
	
	//if ( length % size ) {
	//	printf ("loadBSPFile: odd lump size");
	//}

	memcpy( dest, (unsigned char *)header + ofs, length );

	return length / size;
}

//
// swapBSPFile
//

void BspLoader::SwapBSPFile( void ) 
{
	int i;
	
	// models	
	SwapBlock( (int *) &mDModels[0], mNumModels * sizeof( mDModels[0] ) );

	// shaders (don't swap the name)
	for ( i = 0 ; i < mNumShaders ; i++ ) 
	{
		mDShaders[i].contentFlags = IsLittleLong( mDShaders[i].contentFlags );
		mDShaders[i].surfaceFlags = IsLittleLong( mDShaders[i].surfaceFlags );
	}

	// planes
	SwapBlock( (int *)&mDPlanes[0], mNumPlanes * sizeof( mDPlanes[0] ) );
	
	// nodes
	SwapBlock( (int *)&mDNodes[0], mNumNodes * sizeof( mDNodes[0] ) );

	// leafs
	SwapBlock( (int *)&mDLeafs[0], mNumLeafs * sizeof( mDLeafs[0] ) );

	// leaffaces
	SwapBlock( (int *)&mDLeafSurfaces[0], mNumLeafSurfaces * sizeof( mDLeafSurfaces[0] ) );

	// leafbrushes
	SwapBlock( (int *)&mDLeafBrushes[0], mNumLeafBrushes * sizeof( mDLeafBrushes[0] ) );

	// brushes
	SwapBlock( (int *)&mDBrushes[0], mNumBrushes * sizeof( mDBrushes[0] ) );

	// brushsides
	SwapBlock( (int *)&mDBrushsides[0], mNumBrushsides * sizeof( mDBrushsides[0] ) );

	// vis
	((int *)&mVisBytes)[0] = IsLittleLong( ((int *)&mVisBytes)[0] );
	((int *)&mVisBytes)[1] = IsLittleLong( ((int *)&mVisBytes)[1] );


	// drawindexes
	SwapBlock( (int *)&mDrawIndexes[0], mNumDrawIndexes * sizeof( mDrawIndexes[0] ) );

	// drawsurfs
	SwapBlock( (int *)&mDrawSurfaces[0], mNumDrawSurfaces * sizeof( mDrawSurfaces[0] ) );

}

bool BspLoader::FindVectorByName(float* outvec,const char* name)
{
	const char *cl;
	BSPVector3 origin;
	
	bool found = false;

	for ( int i = 1; i < mNumEntities; i++ ) 
	{
		cl = GetValueForKey (&mEntities[i], "classname");
		if ( !strcmp( cl, "info_player_start" ) ) 
		{
			GetVectorForKey( &mEntities[i], "origin", origin );
			found = true;
			break;
		}
		if ( !strcmp( cl, "info_player_deathmatch" ) ) 
		{
			GetVectorForKey( &mEntities[i], "origin", origin );
			found = true;
			break;
		}
	}

	if (found)
	{
		outvec[0] = origin[0];
		outvec[1] = origin[1];
		outvec[2] = origin[2];
	}
	return found;
}

const BSPEntity * BspLoader::GetEntityByValue( const char* name, const char* value)
{
	const BSPEntity* entity = NULL;

	for ( int i = 1; i < mNumEntities; i++ ) 
	{
		const BSPEntity& ent = mEntities[i];

		const char* cl = GetValueForKey (&mEntities[i], name);
		if ( !strcmp( cl, value ) ) 
		{
			entity = &ent;
			break;
		}
	}
	return entity;
}