#pragma once

namespace Graphics { namespace Platform {

	class BaseTimer
	{
	public:
		BaseTimer() {}
		~BaseTimer() {}
		virtual void Start() = 0;
		// Returns elapsed time from the start point. Time in ms
		virtual float Stop() = 0;
	};
	
}}