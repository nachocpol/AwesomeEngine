#include "Noise.h"
#include <stdlib.h>
#include "glm/glm.hpp"

#define SAFE_RELEASE(p)		if(p){ delete p;		p=nullptr; }
#define SAFE_RELEASE_ARR(a) if(a){ delete[] a;		a=nullptr; }

/////////////////
// Value Noise 1D
Graphics::ValueNoise1D::ValueNoise1D():
	mValues(nullptr),
	mWidth(0)
{
}

Graphics::ValueNoise1D::~ValueNoise1D()
{
	SAFE_RELEASE_ARR(mValues);
}

void Graphics::ValueNoise1D::Initialize(uint32_t width, uint32_t seed /*=1*/)
{
	srand(seed);
	mWidth = width;
	mValues = new float[width];
	for (int i = 0; i < width; i++)
	{
		mValues[i] = glm::clamp((float)rand() / (float)RAND_MAX, 0.0f, 1.0f);
		assert(mValues[i] <= 1.0f && mValues[i] >= 0.0f);
	}
}

float Graphics::ValueNoise1D::Sample(float x)
{
	int ipart = (int)x;
	int minIdx = ipart % mWidth;
	int maxIdx = (ipart + 1) % mWidth;
	assert(maxIdx <= mWidth - 1);
	return glm::mix(mValues[minIdx], mValues[maxIdx], x - (float)ipart);
}

/////////////////
// Value Noise 2D
Graphics::ValueNoise2D::ValueNoise2D() :
	mValues(nullptr),
	mWidth(0),
	mHeight(0)
{
}

Graphics::ValueNoise2D::~ValueNoise2D()
{
	SAFE_RELEASE_ARR(mValues);
}

void Graphics::ValueNoise2D::Initialize(uint32_t width, uint32_t height, uint32_t seed /*=1*/)
{
	srand(seed);
	mWidth = width;
	mHeight = height;
	mValues = new float[width * height];
	for (int i = 0; i < width * height; i++)
	{
		mValues[i] = glm::clamp((float)rand() / (float)RAND_MAX, 0.0f, 1.0f);
		assert(mValues[i] <= 1.0f && mValues[i] >= 0.0f);
	}
}

float Graphics::ValueNoise2D::Sample(float x, float y)
{
	return 0.0f;
}
