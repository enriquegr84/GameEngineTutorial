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
	MaterialLayer() : mTexture(0)
	{
		mSamplerState = eastl::make_shared<SamplerState>();
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
	eastl::shared_ptr<Texture2> mTexture;
	eastl::shared_ptr<SamplerState> mSamplerState;

private:
	friend class Material;
};

#endif
