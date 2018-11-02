// David Eberly, Geometric Tools, Redmond WA 98052
// Copyright (c) 1998-2017
// Distributed under the Boost Software License, Version 1.0.
// http://www.boost.org/LICENSE_1_0.txt
// http://www.geometrictools.com/License/Boost/LICENSE_1_0.txt
// File Version: 3.0.0 (2016/06/19)

#include "LogReporter.h"

LogReporter::~LogReporter()
{
    if (mLogToStdout)
    {
        Logger::Unsubscribe(mLogToStdout.get());
    }

    if (mLogToFile)
    {
        Logger::Unsubscribe(mLogToFile.get());
    }

#if defined(_WINDOWS_API_)
    if (mLogToOutputWindow)
    {
        Logger::Unsubscribe(mLogToOutputWindow.get());
    }

    if (mLogToMessageBox)
    {
        Logger::Unsubscribe(mLogToMessageBox.get());
    }
#endif
}

LogReporter::LogReporter(eastl::string const& logFile, int logFileFlags,
    int logStdoutFlags, int logMessageBoxFlags, int logOutputWindowFlags)
    :
    mLogToFile(nullptr),
    mLogToStdout(nullptr)
#if defined(_WINDOWS_API_)
    ,
    mLogToMessageBox(nullptr),
    mLogToOutputWindow(nullptr)
#endif
{
    if (logFileFlags != Logger::Listener::LISTEN_FOR_NOTHING)
    {
        mLogToFile = eastl::make_unique<LogToFile>(logFile, logFileFlags);
        Logger::Subscribe(mLogToFile.get());
    }

    if (logStdoutFlags != Logger::Listener::LISTEN_FOR_NOTHING)
    {
        mLogToStdout = eastl::make_unique<LogToStdout>(logStdoutFlags);
        Logger::Subscribe(mLogToStdout.get());
    }

#if defined(_WINDOWS_API_)
    if (logMessageBoxFlags != Logger::Listener::LISTEN_FOR_NOTHING)
    {
        mLogToMessageBox = eastl::make_unique<LogToMessageBox>(logMessageBoxFlags);
        Logger::Subscribe(mLogToMessageBox.get());
    }

    if (logOutputWindowFlags != Logger::Listener::LISTEN_FOR_NOTHING)
    {
        mLogToOutputWindow = eastl::make_unique<LogToOutputWindow>(logOutputWindowFlags);
        Logger::Subscribe(mLogToOutputWindow.get());
    }
#endif
}
