#pragma once

#include <stdint.h>

namespace Graphics
{
	class ValueNoise1D
	{
	public:
		ValueNoise1D();
		~ValueNoise1D();
		void Initialize(uint32_t width, uint32_t seed = 1);
		float Sample(float x);

	private:
		float* mValues;
		uint32_t mWidth;
	};

	class ValueNoise2D
	{
	public:
		ValueNoise2D();
		~ValueNoise2D();
		void Initialize(uint32_t width, uint32_t height, uint32_t seed = 1);
		float Sample(float x,float y);
		float Fbm(float x, float y, int octaves, float lacunariry = 2.0f, float gain = 0.5f);

	private:
		float* mValues;
		uint32_t mWidth;
		uint32_t mHeight;
	};
}