#include "WindowsTime.h"
#include <Windows.h>
#include <iostream>

namespace Graphics { namespace Platform { namespace Windows {

	WindowsTimer::WindowsTimer():
		mStarted(false),
		mStart(0),
		mFreq(0)
	{
		// This can be done once!
		LARGE_INTEGER freq;
		QueryPerformanceFrequency(&freq);
		mFreq = freq.QuadPart;
	}

	WindowsTimer::~WindowsTimer()
	{
	}

	void WindowsTimer::Start()
	{
		// According with mcdoc, from WinXP this should never return false/fail
		LARGE_INTEGER startStamp;
		QueryPerformanceCounter(&startStamp);
		mStart = startStamp.QuadPart;
	}

	float WindowsTimer::Stop()
	{
		LARGE_INTEGER endStamp;
		LARGE_INTEGER elapsed;
		QueryPerformanceCounter(&endStamp);

		elapsed.QuadPart = endStamp.QuadPart - mStart;
		elapsed.QuadPart *= 1000000;
		elapsed.QuadPart /= mFreq;

		// from microseconds to ms
		return (float)elapsed.QuadPart / 1000.0f;
	}

}}}