#include "Noise.h"
#include <stdlib.h>
#include <iostream>
#include "glm/glm.hpp"
#include "glm/gtc/random.hpp"

#define SAFE_RELEASE(p)		if(p){ delete p;		p=nullptr; }
#define SAFE_RELEASE_ARR(a) if(a){ delete[] a;		a=nullptr; }

inline float smoothstep(float a, float b, float t)
{
	return glm::mix(a, b, t * t * (3.0f - 2.0f * t));
}

inline float quintic(float a, float b, float t)
{
	return glm::mix(a, b, t * t * t * (t * (t * 6.0f - 15.0f) + 10.0f));
}

glm::vec2 rand2(glm::vec2 p)
{
	return glm::fract(glm::sin(glm::vec2(glm::dot(p, glm::vec2(127.1f, 311.7f)), glm::dot(p, glm::vec2(269.5f, 183.3f))))*43758.5453f);
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

	int ix = floor(x);
	int iy = floor(y);
	int iz = floor(z);

	int32_t slizeSize = mWidth * mHeight;
	int32_t slizeOff = slizeSize * (iz % (mDepth - 1));

	float c000 = mValues[(((ix + 0) % mWidth) + ((iy + 0) % mHeight) * mWidth) + slizeOff];
	float c100 = mValues[(((ix + 1) % mWidth) + ((iy + 0) % mHeight) * mWidth) + slizeOff];
	float c010 = mValues[(((ix + 0) % mWidth) + ((iy + 1) % mHeight) * mWidth) + slizeOff];
	float c110 = mValues[(((ix + 1) % mWidth) + ((iy + 1) % mHeight) * mWidth) + slizeOff];

	slizeOff += slizeSize;
	float c001 = mValues[(((ix + 0) % mWidth) + ((iy + 0) % mHeight) * mWidth) + slizeOff];
	float c101 = mValues[(((ix + 1) % mWidth) + ((iy + 0) % mHeight) * mWidth) + slizeOff];
	float c011 = mValues[(((ix + 0) % mWidth) + ((iy + 1) % mHeight) * mWidth) + slizeOff];
	float c111 = mValues[(((ix + 1) % mWidth) + ((iy + 1) % mHeight) * mWidth) + slizeOff];

	float tx = x - float(ix);
	float ty = y - float(iy);
	float tz = z - float(iz);

	float s0 = smoothstep(smoothstep(c000, c100, tx), smoothstep(c010, c110, tx), ty);
	float s1 = smoothstep(smoothstep(c001, c101, tx), smoothstep(c011, c111, tx), ty);
	float res = smoothstep(s0, s1, tz);

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

//////////////////////////////////
// Gradient Noise 2D//////////////
Graphics::GradientNoise2D::GradientNoise2D():
	mValues(nullptr),
	mWidth(0),
	mHeight(0)
{
}

Graphics::GradientNoise2D::~GradientNoise2D()
{
	SAFE_RELEASE_ARR(mValues);
}


void Graphics::GradientNoise2D::Initialize(uint32_t width, uint32_t height, uint32_t seed)
{
	mWidth = width;
	mHeight = height;
	srand(seed);
	mValues = new glm::vec2[width * height];
	for (int i = 0; i < width*height; i++)
	{
		glm::vec2 cur(0.0f);
		cur.x = glm::clamp((float)rand() / (float)RAND_MAX, 0.0f, 1.0f) * 2.0f - 1.0f;
		cur.y = glm::clamp((float)rand() / (float)RAND_MAX, 0.0f, 1.0f) * 2.0f - 1.0f;
		mValues[i] = glm::normalize(cur);
	}
}

float Graphics::GradientNoise2D::Sample(float x, float y)
{
	/*
		c00	|		c10
		----p----------
		    |
			|
		c01	|		c11
	*/
	// Integer part
	int ix = floor(x);
	int iy = floor(y);

	// Define min and max (y)
	int minx = ix % mWidth;
	int maxx = (ix + 1) % mWidth;

	// Define min and max (y)
	int miny = iy % mHeight;
	int maxy = (iy + 1) % mHeight;

	float tx = x - (float)ix;
	float ty = y - (float)iy;

	// Setup vectors (corner to point)
	float x0 = tx, x1 = tx - 1.0f;
	float y0 = ty, y1 = ty - 1.0f;
	glm::vec2 c00p = glm::vec2(x0,y0);
	glm::vec2 c10p = glm::vec2(x1,y0);
	glm::vec2 c01p = glm::vec2(x0,y1);
	glm::vec2 c11p = glm::vec2(x1,y1);

	// Take samples at the corners
	glm::vec2 s00 = mValues[minx + miny * mWidth];
	glm::vec2 s10 = mValues[maxx + miny * mWidth];
	glm::vec2 s01 = mValues[minx + maxy * mWidth];
	glm::vec2 s11 = mValues[maxx + maxy * mWidth];

	// Gradients
	float g00 = glm::dot(s00, c00p);
	float g10 = glm::dot(s10, c10p);
	float g01 = glm::dot(s01, c01p);
	float g11 = glm::dot(s11, c11p);

	// Bi-linear interpolation
	float b0 = smoothstep(g00, g10, tx);
	float b1 = smoothstep(g01, g11, tx);
	return smoothstep(b0, b1, ty) * 0.5f + 0.5f;
}

float Graphics::GradientNoise2D::Fbm(float x, float y, int octaves, float lacunariry, float gain)
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
// Gradient Noise 3D//////////////
Graphics::GradientNoise3D::GradientNoise3D() :
	mValues(nullptr),
	mWidth(0),
	mHeight(0),
	mDepth(0)
{
}

Graphics::GradientNoise3D::~GradientNoise3D()
{
	SAFE_RELEASE_ARR(mValues);
}


void Graphics::GradientNoise3D::Initialize(uint32_t width, uint32_t height, uint32_t depth, uint32_t seed)
{
	mWidth = width;
	mHeight = height;
	mDepth = depth;
	srand(seed);
	mValues = new glm::vec3[width * height * depth];
	for (int i = 0; i < width*height*depth; i++)
	{
		glm::vec3 cur(0.0f);
		cur.x = glm::clamp((float)rand() / (float)RAND_MAX, 0.0f, 1.0f) * 2.0f - 1.0f;
		cur.y = glm::clamp((float)rand() / (float)RAND_MAX, 0.0f, 1.0f) * 2.0f - 1.0f;
		cur.z = glm::clamp((float)rand() / (float)RAND_MAX, 0.0f, 1.0f) * 2.0f - 1.0f;
		mValues[i] = glm::normalize(cur);
	}
}

float Graphics::GradientNoise3D::Sample(float x, float y, float z)
{
	// Integer part
	int ix = floor(x);
	int iy = floor(y);
	int iz = floor(z);

	// Define min and max (y)
	int minx = ix % mWidth;
	int maxx = (ix + 1) % mWidth;

	// Define min and max (y)
	int miny = iy % mHeight;
	int maxy = (iy + 1) % mHeight;

	// Define min and max (z)
	int minz = iz % mDepth;
	int maxz = (iz + 1) % mDepth;

	float tx = x - (float)ix;
	float ty = y - (float)iy;
	float tz = z - (float)iz;

	// Setup vectors (corner to point)
	float x0 = tx, x1 = tx - 1.0f;
	float y0 = ty, y1 = ty - 1.0f;
	float z0 = tz, z1 = tz - 1.0f;
	glm::vec3 c000p = glm::vec3(x0, y0, z0);
	glm::vec3 c100p = glm::vec3(x1, y0, z0);
	glm::vec3 c010p = glm::vec3(x0, y1, z0);
	glm::vec3 c110p = glm::vec3(x1, y1, z0);
			
	glm::vec3 c001p = glm::vec3(x0, y0, z1);
	glm::vec3 c101p = glm::vec3(x1, y0, z1);
	glm::vec3 c011p = glm::vec3(x0, y1, z1);
	glm::vec3 c111p = glm::vec3(x1, y1, z1);

	// Take samples at the corners
	size_t sliceSize = mWidth * mHeight;
	size_t slizeOff = minz * sliceSize;

	glm::vec3 s000 = mValues[slizeOff + (minx + miny * mWidth)];
	glm::vec3 s100 = mValues[slizeOff + (maxx + miny * mWidth)];
	glm::vec3 s010 = mValues[slizeOff + (minx + maxy * mWidth)];
	glm::vec3 s110 = mValues[slizeOff + (maxx + maxy * mWidth)];
	
	slizeOff += sliceSize;
	glm::vec3 s001 = mValues[slizeOff + (minx + miny * mWidth)];
	glm::vec3 s101 = mValues[slizeOff + (maxx + miny * mWidth)];
	glm::vec3 s011 = mValues[slizeOff + (minx + maxy * mWidth)];
	glm::vec3 s111 = mValues[slizeOff + (maxx + maxy * mWidth)];

	// Gradients
	float g000 = glm::dot(s000, c000p);
	float g100 = glm::dot(s100, c100p);
	float g010 = glm::dot(s010, c010p);
	float g110 = glm::dot(s110, c110p);

	float g001 = glm::dot(s001, c001p);
	float g101 = glm::dot(s101, c101p);
	float g011 = glm::dot(s011, c011p);
	float g111 = glm::dot(s111, c111p);

	// Tri-linear interpolation
	float b00 = smoothstep(g000, g100, tx);
	float b10 = smoothstep(g010, g110, tx);

	float b01 = smoothstep(g001, g101, tx);
	float b11 = smoothstep(g011, g111, tx);

	float b0 = smoothstep(b00, b10, ty);
	float b1 = smoothstep(b01, b11, ty);

	return smoothstep(b0,b1,tz) * 0.5f + 0.5f;
}

float Graphics::GradientNoise3D::Fbm(float x, float y,float z, int octaves, float lacunariry, float gain)
{
	float sum = 0.0f;
	float a = 1.0f;
	float f = 1.0f;
	float tot = 0.0f;
	for (int i = 0; i < octaves; i++)
	{
		tot += a;
		sum += Sample(x * f, y * f, z * f) * a;
		f *= lacunariry;
		a *= gain;
	}
	float ret = sum / tot;
	assert(ret >= 0.0f && ret <= 1.0f);
	return ret;
}

//////////////////////////////////
// Worley Noise 2D//////////////
Graphics::WorleyNoise2D::WorleyNoise2D():
	mWidth(0),
	mHeight(0)
{
}

Graphics::WorleyNoise2D::~WorleyNoise2D()
{
}

void Graphics::WorleyNoise2D::Initialize(uint32_t width, uint32_t height, uint32_t seed)
{
	mWidth = width;
	mHeight = height;
}

float Graphics::WorleyNoise2D::Sample(float x, float y)
{
	// Scale it 
	x *= (float)mWidth;
	y *= (float)mHeight;

	int ix = floor(x);
	int iy = floor(y);
	
	float fx = x - float(ix);
	float fy = y - float(iy);

	float d = 9999999.0f;
	for (int i = -1; i <= 1; i++)
	{
		for (int j = -1; j <= 1; j++)
		{
			int tmpi = i == -1 ? mWidth - 1 : i;
			int tmpy = y == -1 ? mHeight - 1 : y;
			glm::vec2 p = glm::vec2((ix + i) % mWidth, (iy + j) % mHeight);
			glm::vec2 feature = rand2(p);
			glm::vec2 diff = p - glm::vec2(fx, fy);
			float dist = glm::length(diff);
			d = glm::min(d, dist);
		}
	}

	return d;
}


