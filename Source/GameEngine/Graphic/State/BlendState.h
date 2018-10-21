// David Eberly, Geometric Tools, Redmond WA 98052
// Copyright (c) 1998-2017
// Distributed under the Boost Software License, Version 1.0.
// http://www.boost.org/LICENSE_1_0.txt
// http://www.geometrictools.com/License/Boost/LICENSE_1_0.txt
// File Version: 3.0.0 (2016/06/19)

#ifndef BLENDSTATE_H
#define BLENDSTATE_H

#include "Mathematic/Algebra/Vector4.h"
#include "DrawingState.h"

class GRAPHIC_ITEM BlendState : public DrawingState
{
public:
    enum GRAPHIC_ITEM Mode
    {
        BM_ZERO,
        BM_ONE,
        BM_SRC_COLOR,
        BM_INV_SRC_COLOR,
        BM_SRC_ALPHA,
        BM_INV_SRC_ALPHA,
        BM_DEST_ALPHA,
        BM_INV_DEST_ALPHA,
        BM_DEST_COLOR,
        BM_INV_DEST_COLOR,
        BM_SRC_ALPHA_SAT,
        BM_FACTOR,
        BM_INV_FACTOR,
        BM_SRC1_COLOR,
        BM_INV_SRC1_COLOR,
        BM_SRC1_ALPHA,
        BM_INV_SRC1_ALPHA
    };

    enum GRAPHIC_ITEM Operation
    {
        OP_ADD,
        OP_SUBTRACT,
        OP_REV_SUBTRACT,
        OP_MIN,
        OP_MAX
    };

    enum GRAPHIC_ITEM ColorWrite
    {
        CW_ENABLE_RED = 1,
        CW_ENABLE_GREEN = 2,
        CW_ENABLE_BLUE = 4,
        CW_ENABLE_ALPHA = 8,
        CW_ENABLE_ALL = 15
    };

    enum GRAPHIC_ITEM
    {
        NUM_TARGETS = 8
    };

    struct GRAPHIC_ITEM Target
    {
        bool enable;
        Mode srcColor;
        Mode dstColor;
        Operation opColor;
        Mode srcAlpha;
        Mode dstAlpha;
        Operation opAlpha;
        unsigned char mask;

		Target() // default
		{
			enable = false;
			srcColor = BM_ONE;
			dstColor = BM_ZERO;
			opColor = OP_ADD;
			srcAlpha = BM_ONE;
			dstAlpha = BM_ZERO;
			opAlpha = OP_ADD;
			mask = CW_ENABLE_ALL;
		}

		//! Inequality operator
		/** \param b Material to compare to.
		\return True if the materials differ, else false. */
		bool operator!=(const Target& other) const
		{
			bool different =
				enable != other.enable ||
				srcColor != other.srcColor ||
				dstColor != other.dstColor ||
				opColor != other.opColor ||
				srcAlpha != other.srcAlpha ||
				dstAlpha != other.dstAlpha ||
				opAlpha != other.opAlpha ||
				mask != other.mask;
			return different;
		}

		//! Equality operator
		/** \param b Material to compare to.
		\return True if the materials are equal, else false. */
		bool operator==(const Target& other) const
		{
			return !(other != *this);
		}

    };

    // Construction.
    BlendState();

    // Member access.  The members are intended to be write-once before
    // you create an associated graphics state.
    bool mEnableAlphaToCoverage;     // default: false
    bool mEnableIndependentBlend;    // default: false
    Target mTarget[NUM_TARGETS];
    Vector4<float> mBlendColor;      // default: (0,0,0,0)
    unsigned int mSampleMask;        // default: 0xFFFFFFFF
};

#endif