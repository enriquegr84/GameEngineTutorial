// David Eberly, Geometric Tools, Redmond WA 98052
// Copyright (c) 1998-2017
// Distributed under the Boost Software License, Version 1.0.
// http://www.boost.org/LICENSE_1_0.txt
// http://www.geometrictools.com/License/Boost/LICENSE_1_0.txt
// File Version: 3.0.0 (2016/06/19)

#include "Logger.h"

Logger::Logger(char const* file, char const* function, int line, eastl::string const& message)
{
	mMessage =
		"File: " + eastl::string(file) + "\n" +
		"Func: " + eastl::string(function) + "\n" +
		"Line: " + eastl::to_string(line) + "\n" +
		message + "\n\n";
}

Logger::Logger(char const* file, char const* function, int line, eastl::wstring const& message)
{
	std::wstring messageOriginal(message.c_str());
	std::string messageConvertion(messageOriginal.begin(), messageOriginal.end());
	Logger(file, function, line, eastl::string(messageConvertion.c_str()));
}

void Logger::Assertion()
{
	msMutex.lock();
	for (auto listener : msListeners)
	{
		if (listener->GetFlags() & Listener::LISTEN_FOR_ASSERTION)
		{
			listener->Assertion(mMessage);
		}
	}
	msMutex.unlock();
}

void Logger::Error()
{
	msMutex.lock();
	for (auto listener : msListeners)
	{
		if (listener->GetFlags() & Listener::LISTEN_FOR_ERROR)
		{
			listener->Error(mMessage);
		}
	}
	msMutex.unlock();
}

void Logger::Warning()
{
	msMutex.lock();
	for (auto listener : msListeners)
	{
		if (listener->GetFlags() & Listener::LISTEN_FOR_WARNING)
		{
			listener->Warning(mMessage);
		}
	}
	msMutex.unlock();
}

void Logger::Information()
{
	msMutex.lock();
	for (auto listener : msListeners)
	{
		if (listener->GetFlags() & Listener::LISTEN_FOR_INFORMATION)
		{
			listener->Information(mMessage);
		}
	}
	msMutex.unlock();
}

void Logger::Subscribe(Listener* listener)
{
	msMutex.lock();
	msListeners.insert(listener);
	msMutex.unlock();
}

void Logger::Unsubscribe(Listener* listener)
{
	msMutex.lock();
	msListeners.erase(listener);
	msMutex.unlock();
}


// Logger::Listener
Logger::Listener::~Listener()
{
}

Logger::Listener::Listener(int flags)
	:
	mFlags(flags)
{
}

int Logger::Listener::GetFlags() const
{
	return mFlags;
}

void Logger::Listener::Assertion(eastl::string const& message)
{
	Report("\nGE ASSERTION:\n" + message);
}

void Logger::Listener::Error(eastl::string const& message)
{
	Report("\nGE ERROR:\n" + message);
}

void Logger::Listener::Warning(eastl::string const& message)
{
	Report("\nGE WARNING:\n" + message);
}

void Logger::Listener::Information(eastl::string const& message)
{
	Report("\nGE INFORMATION:\n" + message);
}

void Logger::Listener::Report(eastl::string const&)
{
	// Stub for derived classes.
}


std::mutex Logger::msMutex;
eastl::set<Logger::Listener*> Logger::msListeners;
