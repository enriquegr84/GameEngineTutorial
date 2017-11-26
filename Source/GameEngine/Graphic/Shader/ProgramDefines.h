// David Eberly, Geometric Tools, Redmond WA 98052
// Copyright (c) 1998-2017
// Distributed under the Boost Software License, Version 1.0.
// http://www.boost.org/LICENSE_1_0.txt
// http://www.geometrictools.com/License/Boost/LICENSE_1_0.txt
// File Version: 3.0.0 (2016/06/19)

#ifndef PROGRAMDEFINES_H
#define PROGRAMDEFINES_H

#include "GameEngineStd.h"

#include <EASTL/string.h>
#include <EASTL/vector.h>

// Construct definitions used by shader compilers.

class GRAPHIC_ITEM ProgramDefines
{
public:
    // Construction.
    ProgramDefines();

    // Set a definition.  Each is stored as a eastl::pair of eastl::string.  The
    // firstelement of the pair is the 'name' and the second element of the
    // pair is the string representation of 'value'.
    inline void Set(eastl::string const& name, int value);
    inline void Set(eastl::string const& name, unsigned int value);
    inline void Set(eastl::string const& name, float value);
    inline void Set(eastl::string const& name, double value);
    inline void Set(eastl::string const& name, eastl::string const& value);
    inline eastl::vector<eastl::pair<eastl::string, eastl::string>> const& Get() const;

    // Remove definitions, which allows the ProgramDefines object to be
    // shared in a scope.
    void Remove(eastl::string const& name);
    void Clear();

private:
    void Update(eastl::string const& name, eastl::string const& value);

	eastl::vector<eastl::pair<eastl::string, eastl::string>> mDefinitions;
};


inline void ProgramDefines::Set(eastl::string const& name, int value)
{
    Update(name, eastl::to_string(value));
}

inline void ProgramDefines::Set(eastl::string const& name, unsigned int value)
{
    Update(name, eastl::to_string(value));
}

inline void ProgramDefines::Set(eastl::string const& name, float value)
{
    Update(name, eastl::to_string(value));
}

inline void ProgramDefines::Set(eastl::string const& name, double value)
{
    Update(name, eastl::to_string(value));
}

inline void ProgramDefines::Set(eastl::string const& name, eastl::string const& value)
{
    Update(name, value);
}

inline eastl::vector<eastl::pair<eastl::string, eastl::string>> const&
ProgramDefines::Get() const
{
    return mDefinitions;
}

#endif
