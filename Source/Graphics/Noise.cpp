#include "Noise.h"
#include <stdlib.h>
#include <iostream>
#include "glm/glm.hpp"

#define SAFE_RELEASE(p)		if(p){ delete p;		p=nullptr; }
#define SAFE_RELEASE_ARR(a) if(a){ delete[] a;		a=nullptr; }

inline float smoothstep(float a, float b, float t)
{
	return glm::mix(a, b, t * t * (3.0f - 2.0f * t));
}

//////////////////////////////////
// Value Noise 1D/////////////////
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

//////////////////////////////////
// Value Noise 2D/////////////////
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
	/*
		c00	|		c10
		----p----------
		    |
			|
		c01	|		c11
	*/
	// Integer part
	int ix = int(x);
	int iy = int(y);

	// Define min and max (y)
	int minx = ix % mWidth;
	int maxx = (ix + 1) % mWidth;

	// Define min and max (y)
	int miny = iy % mHeight;
	int maxy = (iy + 1) % mHeight;

	// Take samples at the corners
	float s00 = mValues[minx + miny * mWidth];
	float s10 = mValues[maxx + miny * mWidth];
	float s01 = mValues[minx + maxy * mWidth];
	float s11 = mValues[maxx + maxy * mWidth];

	// Bi-linear interpolation
	float t = x - (float)ix;
	float b0 = smoothstep(s00, s10, t);
	float b1 = smoothstep(s01, s11, t);
	return smoothstep(b0, b1, y - float(iy));
}

float Graphics::ValueNoise2D::Fbm(float x, float y, int octaves, float lacunariry /*= 2.0f*/, float gain /*= 0.5f*/)
{
	float sum = 0.0f;
	float a = 1.0f;
	float f = 1.0f;
	float tot = 0.0f;
	for (int i = 0; i < octaves; i++)
	{
		tot += a;
		sum += Sample(x * f, y * f) * a;
		f *= lacunariry;
		a *= gain;
	}
	float ret = sum / tot;
	assert(ret >= 0.0f && ret <= 1.0f);
	return ret;
}


//////////////////////////////////
// Value Noise 3D/////////////////
Graphics::ValueNoise3D::ValueNoise3D():
	mValues(nullptr),
	mWidth(0),
	mHeight(0),
	mDepth(0)
{
}

Graphics::ValueNoise3D::~ValueNoise3D()
{
	SAFE_RELEASE_ARR(mValues);
}

void Graphics::ValueNoise3D::Initialize(uint32_t width, uint32_t height, uint32_t depth, uint32_t seed /*=1*/)
{
	srand(seed);
	mWidth = width;
	mHeight = height;
	mDepth = depth;
	mValues = new float[width * height * depth];
	for (int i = 0; i < width * height * depth; i++)
	{
		mValues[i] = glm::clamp((float)rand() / (float)RAND_MAX, 0.0f, 1.0f);
		assert(mValues[i] <= 1.0f && mValues[i] >= 0.0f);
	}
}

#pragma  optimize("",off)
float Graphics::ValueNoise3D::Sample(float x, float y,float z)
{
	/*
		Basically we have to blend between two slices

			c001-------c101
			/|          /|
		   / | 	       / |
		  /	 |		  /  |
		c000--------c100 |
		 |   |       |   |
		 |	c011-----|-c111
		 |  /		 | /
		 | /         |/
		c010--------c110
	*/

	int ix = int(x);
	int iy = int(y);
	int iz = int(z);

	// Define our cube
	int minx = ix % mWidth;
	int maxx = (ix + 1) % mWidth;

	int miny = iy % mHeight;
	int maxy = (iy + 1) % mHeight;

	int minz = iz % mDepth;
	int maxz = (iz + 1) % mDepth;

	// Calculate the current 3D slice offset
	size_t sliceSize = mWidth * mHeight;
	size_t slizeOff = minz * sliceSize;

	// 1st slice
	float c000 = mValues[slizeOff + (minx + miny * mWidth)];
	float c100 = mValues[slizeOff + (maxx + miny * mWidth)];
	float c010 = mValues[slizeOff + (minx + maxy * mWidth)];
	float c110 = mValues[slizeOff + (maxx + maxy * mWidth)];

	// 2nd slice
	slizeOff += sliceSize; 
	float c001 = mValues[slizeOff + (minx + miny * mWidth)];
	float c101 = mValues[slizeOff + (maxx + miny * mWidth)];
	float c011 = mValues[slizeOff + (minx + maxy * mWidth)];
	float c111 = mValues[slizeOff + (maxx + maxy * mWidth)];

	// Blend each slice and then blend using 'z' between the slices:
	float slice0 = 0.0f;
	{
		float t = x - (float)ix;
		float b0 = smoothstep(c000, c100, t);
		float b1 = smoothstep(c010, c110, t);
		slice0 = smoothstep(b0, b1, y - float(iy));
	}
	float slice1 = 0.0f;
	{
		float t = x - (float)ix;
		float b0 = smoothstep(c001, c101, t);
		float b1 = smoothstep(c011, c111, t);
		slice1 = smoothstep(b0, b1, y - float(iy));
	}
	float res = smoothstep(slice0, slice1, z - float(iz));

	assert(res >= 0.0f && res <= 1.0f);
	return res;
}

float Graphics::ValueNoise3D::Fbm(float x, float y,float z, int octaves, float lacunariry /*= 2.0f*/, float gain /*= 0.5f*/)
{
	float sum = 0.0f;
	float a = 1.0f;
	float f = 1.0f;
	float tot = 0.0f;
	for (int i = 0; i < octaves; i++)
	{
		tot += a;
		sum += Sample(x * f, y * f,z * f) * a;
		f *= lacunariry;
		a *= gain;
	}
	float ret = sum / tot;
	assert(ret >= 0.0f && ret <= 1.0f);
	return ret;
}
