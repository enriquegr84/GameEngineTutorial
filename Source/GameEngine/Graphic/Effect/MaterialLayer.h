// David Eberly, Geometric Tools, Redmond WA 98052
// Copyright (c) 1998-2017
// Distributed under the Boost Software License, Version 1.0.
// http://www.boost.org/LICENSE_1_0.txt
// http://www.geometrictools.com/License/Boost/LICENSE_1_0.txt
// File Version: 3.0.0 (2016/06/19)

#ifndef MATERIAL_LAYER_H
#define MATERIAL_LAYER_H

#include "Graphic/GraphicStd.h"

#include "Graphic/State/SamplerState.h"

#include "Graphic/Resource/Texture/Texture2.h"

#include "Mathematic/Algebra/Matrix4x4.h"

//! Texture coord clamp mode outside [0.0, 1.0]
enum GRAPHIC_ITEM TextureClamp
{
	//! Texture repeats
	TC_REPEAT = 0,
	//! Texture is clamped to the last pixel
	TC_CLAMP,
	//! Texture is clamped to the edge pixel
	TC_CLAMP_TO_EDGE,
	//! Texture is clamped to the border pixel (if exists)
	TC_CLAMP_TO_BORDER,
	//! Texture is alternatingly mirrored (0..1..0..1..0..)
	TC_MIRROR,
	//! Texture is mirrored once and then clamped (0..1..0)
	TC_MIRROR_CLAMP,
	//! Texture is mirrored once and then clamped to edge
	TC_MIRROR_CLAMP_TO_EDGE,
	//! Texture is mirrored once and then clamped to border
	TC_MIRROR_CLAMP_TO_BORDER
};

class GRAPHIC_ITEM MaterialLayer
{
public:
    // Construction.
	MaterialLayer()
		: mTexture(0), mSamplerState(0)
	{}

	//! Copy constructor
	/** \param other Material layer to copy from. */
	MaterialLayer(const MaterialLayer& other)
	{
		*this = other;
	}

	//! Destructor
	~MaterialLayer()
	{

	}

	//! Assignment operator
	/** \param other Material layer to copy from.
	\return This material layer, updated. */
	MaterialLayer& operator=(const MaterialLayer& other)
	{
		// Check for self-assignment!
		if (this == &other)
			return *this;

		mTexture = other.mTexture;
		mSamplerState = other.mSamplerState;

		return *this;
	}

	//! Inequality operator
	/** \param b Layer to compare to.
	\return True if layers are different, else false. */
	inline bool operator!=(const MaterialLayer& other) const
	{
		bool different =
			mTexture != other.mTexture ||
			mSamplerState != other.mSamplerState;
		return different;
	}

	//! Equality operator
	/** \param b Layer to compare to.
	\return True if layers are equal, else false. */
	inline bool operator==(const MaterialLayer& other) const
	{
		return !(other != *this);
	}

	//! Texture
	Texture2* mTexture;

	SamplerState* mSamplerState;

private:
	friend class Material;
};

#endif
