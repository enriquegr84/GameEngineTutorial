// David Eberly, Geometric Tools, Redmond WA 98052
// Copyright (c) 1998-2017
// Distributed under the Boost Software License, Version 1.0.
// http://www.boost.org/LICENSE_1_0.txt
// http://www.geometrictools.com/License/Boost/LICENSE_1_0.txt
// File Version: 3.0.0 (2016/06/19)

#include "LogToStringArray.h"

LogToStringArray::LogToStringArray(eastl::string const& name, int flags)
    :
    Logger::Listener(flags),
    mName(name)
{
}

eastl::string const& LogToStringArray::GetName() const
{
    return mName;
}

eastl::vector<eastl::string> const& LogToStringArray::GetMessages() const
{
    return mMessages;
}

eastl::vector<eastl::string>& LogToStringArray::GetMessages()
{
    return mMessages;
}

void LogToStringArray::Report(eastl::string const& message)
{
    mMessages.push_back(message);
}

