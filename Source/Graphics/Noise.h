#pragma once

#include <stdint.h>
#include "glm/glm.hpp"

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
		float Fbm(float x, float y, int octaves, float lacunarity = 2.0f, float gain = 0.5f);

	private:
		float* mValues;
		uint32_t mWidth;
		uint32_t mHeight;
	};

	class ValueNoise3D
	{
	public:
		ValueNoise3D();
		~ValueNoise3D();
		void Initialize(uint32_t width, uint32_t height, uint32_t depth, uint32_t seed = 1);
		float Sample(float x, float y,float z);
		float Fbm(float x, float y,float z, int octaves, float lacunarity = 2.0f, float gain = 0.5f);

	private:
		float* mValues;
		uint32_t mWidth;
		uint32_t mHeight;
		uint32_t mDepth;
	};

	class GradientNoise2D
	{
	public:
		GradientNoise2D();
		~GradientNoise2D();
		void Initialize(uint32_t width, uint32_t height, uint32_t seed = 1);
		float Sample(float x, float y);
		float Fbm(float x, float y, int octaves, float lacunarity = 2.0f, float gain = 0.5f);

	private:
		glm::vec2* mValues;
		uint32_t mWidth;
		uint32_t mHeight;
	};

	class GradientNoise3D
	{
	public:
		GradientNoise3D();
		~GradientNoise3D();
		void Initialize(uint32_t width, uint32_t height, uint32_t depth, uint32_t seed = 1);
		float Sample(float x, float y,float z);
		float Fbm(float x, float y, float z, int octaves, float lacunarity = 2.0f, float gain = 0.5f);

	private:
		glm::vec3* mValues;
		uint32_t mWidth;
		uint32_t mHeight;
		uint32_t mDepth;
	};

	class WorleyNoise2D
	{
	public:
		WorleyNoise2D();
		~WorleyNoise2D();
		void Initialize(uint32_t width, uint32_t height,uint32_t seed = 1);
		float Sample(float x, float y);
		float Fbm(float x, float y, int octaves, float lacunarity = 2.0f, float gain = 0.5f);
	private:
		uint32_t mWidth;
		uint32_t mHeight;
	};

	class WorleyNoise3D
	{
	public:
		WorleyNoise3D();
		~WorleyNoise3D();
		void Initialize(uint32_t width, uint32_t height, uint32_t depth, uint32_t seed = 1);
		float Sample(float x, float y,float z);
		float Fbm(float x, float y, float z, int octaves, float lacunarity = 2.0f, float gain = 0.5f);
	private:
		uint32_t mWidth;
		uint32_t mHeight;
		uint32_t mDepth;
	};
}