#pragma once
#include <stdint.h>

namespace Core 
{
	class Timer
	{
	public:
		Timer();
		~Timer();
		void Start();
		// Returns elapsed time from the start point. Time in ms
		float Stop();
	private:
		bool mStarted;
		int64_t mStart;
	};	
}