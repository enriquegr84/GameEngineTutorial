// David Eberly, Geometric Tools, Redmond WA 98052
// Copyright (c) 1998-2017
// Distributed under the Boost Software License, Version 1.0.
// http://www.boost.org/LICENSE_1_0.txt
// http://www.geometrictools.com/License/Boost/LICENSE_1_0.txt
// File Version: 3.0.0 (2016/06/19)

#ifndef LOGTOFILE_H
#define LOGTOFILE_H

#include "Logger.h"

class CORE_ITEM LogToFile : public Logger::Listener
{
public:
    LogToFile(eastl::string const& filename, int flags);

private:
    virtual void Report(eastl::string const& message);

	eastl::string mFilename;
};

#endif