#pragma once

#include "Graphics/Platform/BaseTime.h"
#include <cstdint>

namespace Graphics { namespace Platform { namespace Windows {

	class WindowsTimer: public BaseTimer
	{
	public:
		WindowsTimer();
		~WindowsTimer();
		void Start()override;
		// Returns elapsed time from the start point. Time in ms
		float Stop()override;

	private:
		bool mStarted;
		int64_t mStart;
		int64_t mFreq;
	};
	
}}}