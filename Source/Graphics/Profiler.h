#pragma once
#include "Graphics/GraphicsInterface.h"
#include <map>

namespace Graphics
{
	struct ProfilingEvent
	{
		GPUQueryHandle Query;
	};

	class Profiler
	{
	public:
		~Profiler();
		static Profiler* GetInstance();
		void Init(GraphicsInterface* graphics);
		void Begin(const char* name);
		void End(const char* name);

	private:
		Profiler();
		GraphicsInterface* mGraphics;

		std::map<const char*, ProfilingEvent> mBeginQueries;
		std::map<const char*, ProfilingEvent> mEndQueries;
	};
}