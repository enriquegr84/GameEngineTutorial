// David Eberly, Geometric Tools, Redmond WA 98052
// Copyright (c) 1998-2017
// Distributed under the Boost Software License, Version 1.0.
// http://www.boost.org/LICENSE_1_0.txt
// http://www.geometrictools.com/License/Boost/LICENSE_1_0.txt
// File Version: 3.0.0 (2016/06/19)

#ifndef LOGGER_H
#define LOGGER_H

#include "Core/CoreStd.h"

#include <mutex>

class CORE_ITEM Logger
{
public:
	// Construction.  The Logger object is designed to exist only for a
	// single-line call.  A string is generated from the input parameters and
	// is used for reporting.
	Logger(char const* file, char const* function, int line, eastl::string const& message);
	Logger(char const* file, char const* function, int line, eastl::wstring const& message);

	// Notify current listeners about the logged information.
	void Assertion();
	void Error();
	void Warning();
	void Information();

	// Listeners subscribe to Logger to receive message strings.
	class Listener
	{
	public:
		enum
		{
			LISTEN_FOR_NOTHING = 0x00000000,
			LISTEN_FOR_ASSERTION = 0x00000001,
			LISTEN_FOR_ERROR = 0x00000002,
			LISTEN_FOR_WARNING = 0x00000004,
			LISTEN_FOR_INFORMATION = 0x00000008,
			LISTEN_FOR_ALL = 0xFFFFFFFF
		};

		// Construction and destruction.
		virtual ~Listener();
		Listener(int flags = LISTEN_FOR_NOTHING);

		// What the listener wants to hear.
		int GetFlags() const;

		// Handlers for the messages received from the logger.
		void Assertion(eastl::string const& message);
		void Error(eastl::string const& message);
		void Warning(eastl::string const& message);
		void Information(eastl::string const& message);

	private:
		virtual void Report(eastl::string const& message);

		int mFlags;
	};

	static void Subscribe(Listener* listener);
	static void Unsubscribe(Listener* listener);

private:
	eastl::string mMessage;

	static std::mutex msMutex;
	static eastl::set<Listener*> msListeners;
};


#if !defined(NO_LOGGER)

#define LogAssert(condition, message) \
    if (!(condition)) \
    { \
        Logger(__FILE__, __FUNCTION__, __LINE__, message).Assertion(); \
    }

#define LogError(message) \
    Logger(__FILE__, __FUNCTION__, __LINE__, message).Error()

#define LogWarning(message) \
    Logger(__FILE__, __FUNCTION__, __LINE__, message).Warning()

#define LogInformation(message) \
    Logger(__FILE__, __FUNCTION__, __LINE__, message).Information()

#else

// No logging of assertions, warnings, errors, or information.
#define LogAssert(condition, message)
#define LogError(message)
#define LogWarning(message)
#define LogInformation(message)

#endif

#endif