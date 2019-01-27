#include "OS.h"

#if defined(_WINDOWS_API_) && defined(_MSC_VER) && (_MSC_VER > 1298)
	#include <stdlib.h>
	#define bswap_16(X) _byteswap_ushort(X)
	#define bswap_32(X) _byteswap_ulong(X)
#if (_MSC_VER >= 1400)
	#define localtime _localtime64_s
#endif
#endif

unsigned int ByteSwap::Byteswap(unsigned int num) {return bswap_32(num);}
signed int ByteSwap::Byteswap(signed int num) {return bswap_32(num);}
//float ByteSwap::ByteSwap(float num) {unsigned int tmp=IR(num); tmp=bswap_32(tmp); return (FR(tmp));}
// prevent accidental byte swapping of chars
char  ByteSwap::Byteswap(char num)  {return num;}


#if defined(_WINDOWS_API_)
// ----------------------------------------------------------------
// Windows specific functions
// ----------------------------------------------------------------
#include <time.h>

static LARGE_INTEGER HighPerformanceFreq;
static BOOL HighPerformanceTimerSupport = FALSE;
static BOOL MultiCore = FALSE;

void Timer::InitTimer(bool usePerformanceTimer)
{
	// workaround for hires timer on multiple core systems, bios bugs result in bad hires timers.
	SYSTEM_INFO sysinfo;
	GetSystemInfo(&sysinfo);
	MultiCore = (sysinfo.dwNumberOfProcessors > 1);

	//performance timer determines the frequency through a call to QueryPerformanceFrequency(),
	if (usePerformanceTimer)
		HighPerformanceTimerSupport = QueryPerformanceFrequency(&HighPerformanceFreq);
	else
		HighPerformanceTimerSupport = FALSE;
	InitVirtualTimer();
}


unsigned int Timer::GetRealTime()
{
	if (HighPerformanceTimerSupport)
	{
		// Avoid potential timing inaccuracies across multiple cores by
		// temporarily setting the affinity of this process to one core.
		DWORD_PTR affinityMask=0;
		if(MultiCore)
			affinityMask = SetThreadAffinityMask(GetCurrentThread(), 1);

		// To compute the elpased time of an iteration of the game loop, it is called the
		// QueryPerformanceCounter
		LARGE_INTEGER nTime;
		BOOL queriedOK = QueryPerformanceCounter(&nTime);

		// Restore the true affinity.
		if(MultiCore)
			(void)SetThreadAffinityMask(GetCurrentThread(), affinityMask);

		if(queriedOK)
			return unsigned int((nTime.QuadPart) * 1000 / HighPerformanceFreq.QuadPart);

	}

	return GetTickCount();
}

//! Get real time and date in calendar form
Timer::RealTimeDate Timer::GetRealTimeAndDate()
{
	time_t rawtime;
	time(&rawtime);

	struct tm * timeinfo = NULL;
	localtime(timeinfo, &rawtime);

	// init with all 0 to indicate error
	Timer::RealTimeDate date={0};
	// at least Windows returns NULL on some illegal dates
	if (timeinfo)
	{
		// set useful values if succeeded
		date.Hour=(unsigned int)timeinfo->tm_hour;
		date.Minute=(unsigned int)timeinfo->tm_min;
		date.Second=(unsigned int)timeinfo->tm_sec;
		date.Day=(unsigned int)timeinfo->tm_mday;
		date.Month=(unsigned int)timeinfo->tm_mon+1;
		date.Year=(unsigned int)timeinfo->tm_year+1900;
		date.Weekday=(Timer::EWeekday)timeinfo->tm_wday;
		date.Yearday=(unsigned int)timeinfo->tm_yday+1;
		date.IsDST=timeinfo->tm_isdst != 0;
	}
	return date;
}

#endif // windows

// ------------------------------------------------------------------------
/** Returns the number of seconds since 1.1.1970. This function is used
    *  to compare access times of files, e.g. news, addons data etc.
*/
long long Timer::GetTimeSinceEpoch()
{
#ifdef WIN32
    FILETIME ft;
    GetSystemTimeAsFileTime(&ft);
    long long t = ft.dwHighDateTime;
    t <<= 32;
    t /= 10;
    // The Unix epoch starts on Jan 1 1970.  Need to subtract
    // the difference in seconds from Jan 1 1601.
#       if defined(_MSC_VER) || defined(_MSC_EXTENSIONS)
#           define DELTA_EPOCH_IN_MICROSECS  11644473600000000Ui64
#       else
#           define DELTA_EPOCH_IN_MICROSECS  11644473600000000ULL
#       endif
    t -= DELTA_EPOCH_IN_MICROSECS;

    t |= ft.dwLowDateTime;
    // Convert to seconds since epoch
    t /= 1000000UL;
    return t;
#else
    struct timeval tv;
    gettimeofday(&tv, NULL);
    return tv.tv_sec;
#endif
};   // getTimeSinceEpoch

// our Randomizer is not really os specific, so we
// code one for all, which should work on every platform the same,
// which is desireable.

signed int Randomizer::seed = (int)time(0);

//! generates a pseudo random number
signed int Randomizer::Rand()
{
	// (a*seed)%m with Schrage's method
	seed = a * (seed%q) - r* (seed/q);
	if (seed<0)
		seed += m;

	return seed;
}

//! generates a pseudo random number
float Randomizer::FRand()
{
	return Rand()*(1.f/rMax);
}

signed int Randomizer::RandMax()
{
	return rMax;
}

//! resets the randomizer
void Randomizer::Reset(signed int value)
{
	seed = value;
}


// ------------------------------------------------------
/*
	Timer implementation, the mStartRealTime member stores the counter value for computer 
	the total elapsed time. the member mStaticTime is used for storing subsequent calls to 
	QueryPerformanceCounter() to compute the elpased time per frame.
*/

float Timer::mVirtualTimerSpeed = 1.0f;
signed int Timer::mVirtualTimerStopCounter = 0;
unsigned int Timer::mLastVirtualTime = 0;
unsigned int Timer::mStartRealTime = 0;
unsigned int Timer::mStaticTime = 0;

//! returns current virtual time
unsigned int Timer::GetTime()
{
	if (IsStopped())
		return mLastVirtualTime;

	return mLastVirtualTime + (unsigned int)((mStaticTime - mStartRealTime) * mVirtualTimerSpeed);
}

//! ticks, advances the virtual timer, encapsulates the elpased-time calculations
void Timer::Tick()
{
	mStaticTime = GetRealTime();
}

//! sets the current virtual time
void Timer::SetTime(unsigned int time)
{
	mStaticTime = GetRealTime();
	mLastVirtualTime = time;
	mStartRealTime = mStaticTime;
}

//! stops the virtual timer
void Timer::StopTimer()
{
	if (!IsStopped())
	{
		// stop the virtual timer
		mLastVirtualTime = GetTime();
	}

	--mVirtualTimerStopCounter;
}

//! starts the virtual timer
void Timer::StartTimer()
{
	++mVirtualTimerStopCounter;

	if (!IsStopped())
	{
		// restart virtual timer
		SetTime(mLastVirtualTime);
	}
}

//! sets the speed of the virtual timer
void Timer::SetSpeed(float speed)
{
	SetTime(GetTime());

	mVirtualTimerSpeed = speed;
	if (mVirtualTimerSpeed < 0.0f)
		mVirtualTimerSpeed = 0.0f;
}

//! gets the speed of the virtual timer
float Timer::GetSpeed()
{
	return mVirtualTimerSpeed;
}

//! returns if the timer currently is stopped
bool Timer::IsStopped()
{
	return mVirtualTimerStopCounter < 0;
}

void Timer::InitVirtualTimer()
{
	mStaticTime = GetRealTime();
	mStartRealTime = mStaticTime;
}