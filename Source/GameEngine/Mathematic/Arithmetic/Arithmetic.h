// David Eberly, Geometric Tools, Redmond WA 98052
// Copyright (c) 1998-2017
// Distributed under the Boost Software License, Version 1.0.
// http://www.boost.org/LICENSE_1_0.txt
// http://www.geometrictools.com/License/Boost/LICENSE_1_0.txt
// File Version: 3.0.0 (2016/06/19)

#ifndef ARITHMETIC_H
#define ARITHMETIC_H

#include "Mathematic/MathematicStd.h"

#include "BSNumber.h"
#include "BSRational.h"
#include "IEEEBinary16.h"
#include "UIntegerAP32.h"
#include "UIntegerFP32.h"

class Arithmetic
{
public:
    // The tag-dispatch pattern is used for template-parameter-controlled
    // instantiation of mathematical functions. See Functions.h for
    // examples of how to use this type-trait system.

    enum Type
    {
        IS_INVALID,             // not an arithmetic type
        IS_FLOATING_POINT,      // 'float' or 'double' or 'long double'
        IS_FP16,                // IEEEBinary16
        IS_BINARY_SCIENTIFIC    // BSNumber or BSRational
    };

    typedef eastl::integral_constant<Type, IS_INVALID> IsInvalidType;
    typedef eastl::integral_constant<Type, IS_FLOATING_POINT> IsFPType;
    typedef eastl::integral_constant<Type, IS_FP16> IsFP16Type;
    typedef eastl::integral_constant<Type, IS_BINARY_SCIENTIFIC> IsBSType;

    template <typename T> struct WhichType : IsInvalidType{};
    template <typename U> struct WhichType<BSNumber<U>> : IsBSType{};
    template <typename U> struct WhichType<BSRational<U>> : IsBSType{};
};

template<> struct Arithmetic::WhichType<float> : IsFPType{};
template<> struct Arithmetic::WhichType<double> : IsFPType{};
template<> struct Arithmetic::WhichType<long double> : IsFPType{};
template<> struct Arithmetic::WhichType<IEEEBinary16> : IsFP16Type{};

#endif