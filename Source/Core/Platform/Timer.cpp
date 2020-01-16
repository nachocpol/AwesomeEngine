#include "Timer.h"

using namespace Core;

#if defined(PLATFORM_WINDOWS)
	#include <Windows.h>
	static int64_t gSysFrequency = -1;
#else
	#error Timer not implemented for this platform!
#endif

Timer::Timer():
	 mStarted(false)
	,mStart(0)
{
#if defined(PLATFORM_WINDOWS)
	if (gSysFrequency == -1)
	{
		LARGE_INTEGER freq;
		QueryPerformanceFrequency(&freq);
		gSysFrequency = freq.QuadPart;
	}
#endif
}

Timer::~Timer()
{
}

void Timer::Start()
{
#if defined(PLATFORM_WINDOWS)
	// According with mcdoc, from WinXP this should never return false/fail
	LARGE_INTEGER startStamp;
	QueryPerformanceCounter(&startStamp);
	mStart = startStamp.QuadPart;
#endif
}

float Timer::Stop()
{
#if defined(PLATFORM_WINDOWS)
	LARGE_INTEGER endStamp;
	LARGE_INTEGER elapsed;
	QueryPerformanceCounter(&endStamp);

	elapsed.QuadPart = endStamp.QuadPart - mStart;
	elapsed.QuadPart *= 1000000;
	elapsed.QuadPart /= gSysFrequency;

	// from microseconds to ms
	return (float)elapsed.QuadPart / 1000.0f;
#endif
}
