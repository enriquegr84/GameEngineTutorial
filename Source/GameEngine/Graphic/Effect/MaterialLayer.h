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

class GRAPHIC_ITEM MaterialLayer
{
public:
    // Construction.
	MaterialLayer() : 
		mTexture(0),
		mModeU(SamplerState::WRAP),
		mModeV(SamplerState::WRAP),
		mFilter(SamplerState::MIN_P_MAG_P_MIP_L),
		mLODBias(false)
	{

	}

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
		mModeU = other.mModeU;
		mModeV = other.mModeV;
		mFilter = other.mFilter;
		mLODBias = other.mLODBias;

		return *this;
	}

	//! Inequality operator
	/** \param b Layer to compare to.
	\return True if layers are different, else false. */
	inline bool operator!=(const MaterialLayer& other) const
	{
		bool different =
			mTexture != other.mTexture ||
			mModeU != other.mModeU ||
			mModeV != other.mModeV ||
			mFilter != other.mFilter ||
			mLODBias != other.mLODBias;
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
	eastl::shared_ptr<Texture2> mTexture;

	//! Modes for texture coordinates.
	SamplerState::Mode mModeU;
	SamplerState::Mode mModeV;

	//! Filter state codification
	SamplerState::Filter mFilter;

	//! Bias for the mipmap choosing decision.
	/** This value can make the textures more or less blurry than with the
	default value of 0. The value (divided by 8.f) is added to the mipmap level
	chosen initially, and thus takes a smaller mipmap for a region
	if the value is positive. */
	bool mLODBias;

private:
	friend class Material;
};

#endif
