// David Eberly, Geometric Tools, Redmond WA 98052
// Copyright (c) 1998-2017
// Distributed under the Boost Software License, Version 1.0.
// http://www.boost.org/LICENSE_1_0.txt
// http://www.geometrictools.com/License/Boost/LICENSE_1_0.txt
// File Version: 3.0.0 (2016/06/19)

#ifndef KEYFRAMECONTROLLER_H
#define KEYFRAMECONTROLLER_H

#include "TransformController.h"

#include "Mathematic/Algebra/Quaternion.h"

class GRAPHIC_ITEM KeyframeController : public TransformController
{
public:
    // Construction and destruction.  If the translations, rotations, and
    // scales all share the same keyframe times, then numCommonTimes is
    // set to a positive number.  Each remaining number is numCommonTimes
    // when the channel exists or zero when it does not.  If the keyframe
    // times are not shared, then numCommonTimes must be set to zero and
    // the remaining numbers set to the appropriate values--positive when
    // the channel exists or zero otherwise.
    //
    // The Transform input initializes the controlled object's local
    // transform.  The previous behavior of this class was to fill in only
    // those transformation channels represented by the key frames, which
    // relied implicitly on the Spatial mObject to have its other channels
    // set appropriately by the application.  Now KeyframeController sets
    // *all* the channels.
    virtual ~KeyframeController();
    KeyframeController(int numCommonTimes, int numTranslations,
        int numRotations, int numScales, Transform const& localTransform);

    // Member access.  After calling the constructor, you must set the data
    // using these functions.
    inline int GetNumCommonTimes() const;
    inline float* GetCommonTimes();

    inline int GetNumTranslations() const;
    inline float* GetTranslationTimes();
    inline Vector4<float>* GetTranslations();

    inline int GetNumRotations() const;
    inline float* GetRotationTimes();
    inline Quaternion<float>* GetRotations();

    inline int GetNumScales() const;
    inline float* GetScaleTimes();
    inline float* GetScales();

    // The animation update.
    virtual bool Update();

protected:
    // Support for looking up keyframes given the specified time.
    static void GetKeyInfo(float ctrlTime, int numTimes, float* times,
        int& lastIndex, float& normTime, int& i0, int& i1);

    Vector4<float> GetTranslate(float normTime, int i0, int i1);
    Matrix4x4<float> GetRotate(float normTime, int i0, int i1);
    float GetScale(float normTime, int i0, int i1);

    // This array is used only when times are shared by translations,
    // rotations, and scales.
    int mNumCommonTimes;
    eastl::vector<float> mCommonTimes;

    int mNumTranslations;
	eastl::vector<float> mTranslationTimes;
	eastl::vector<Vector4<float>> mTranslations;

    int mNumRotations;
	eastl::vector<float> mRotationTimes;
	eastl::vector<Quaternion<float>> mRotations;

    int mNumScales;
	eastl::vector<float> mScaleTimes;
	eastl::vector<float> mScales;

    // Cached indices for the last found pair of keys used for interpolation.
    // For a sequence of times, this guarantees an O(1) lookup.
    int mTLastIndex, mRLastIndex, mSLastIndex, mCLastIndex;
};


inline int KeyframeController::GetNumCommonTimes() const
{
    return mNumCommonTimes;
}

inline float* KeyframeController::GetCommonTimes()
{
    return mCommonTimes.data();
}

inline int KeyframeController::GetNumTranslations() const
{
    return mNumTranslations;
}

inline float* KeyframeController::GetTranslationTimes()
{
    return mTranslationTimes.data();
}

inline Vector4<float>* KeyframeController::GetTranslations()
{
    return mTranslations.data();
}

inline int KeyframeController::GetNumRotations() const
{
    return mNumRotations;
}

inline float* KeyframeController::GetRotationTimes()
{
    return mRotationTimes.data();
}

inline Quaternion<float>* KeyframeController::GetRotations()
{
    return mRotations.data();
}

inline int KeyframeController::GetNumScales() const
{
    return mNumScales;
}

inline float* KeyframeController::GetScaleTimes()
{
    return mScaleTimes.data();
}

inline float* KeyframeController::GetScales()
{
    return mScales.data();
}

#endif