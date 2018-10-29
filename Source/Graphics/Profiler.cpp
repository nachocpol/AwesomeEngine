#include "Profiler.h"

namespace Graphics
{
	Graphics::Profiler::Profiler()
	{
	}
	
	Graphics::Profiler::~Profiler()
	{
	}
	
	Profiler* Graphics::Profiler::GetInstance()
	{
		static Profiler* sInstance = nullptr;
		if (!sInstance)
		{
			sInstance = new Profiler();
		}
		return sInstance;
	}

	void Profiler::Init(GraphicsInterface* graphics)
	{
		mGraphics = graphics;
	}

	void Profiler::Begin(const char* name)
	{
		auto entry = mBeginQueries.find(name);
		if (entry == mBeginQueries.end())
		{
			ProfilingEvent ev;
			ev.Query = mGraphics->CreateQuery(GPUQueryType::Timestamp);
			mBeginQueries[name] = ev;
		}
		mGraphics->EndQuery(mBeginQueries[name].Query, GPUQueryType::Timestamp);
	}

	void Profiler::End(const char* name)
	{
		auto entry = mEndQueries.find(name);
		if (entry == mEndQueries.end())
		{
			ProfilingEvent ev;
			ev.Query = mGraphics->CreateQuery(GPUQueryType::Timestamp);
			mEndQueries[name] = ev;
		}
		mGraphics->EndQuery(mEndQueries[name].Query, GPUQueryType::Timestamp);
	}
}
