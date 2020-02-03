#include "Light.h"

using namespace World;

Light::Light() :
	 mType(LightType::Point)
	,mColor(1.0f)
	,mIntensity(1.0f)
	,mRadius(5.0f)
{
}

Light::~Light()
{
}

void Light::SetLightType(LightType::T type)
{
	mType = type;
}

Light::LightType::T Light::GetLightType() const
{
	return mType;
}

void Light::SetColor(glm::vec3 color)
{
	mColor = color;
}

glm::vec3 Light::GetColor() const
{
	return mColor;
}

void Light::SetIntensity(float intensity)
{
	mIntensity = intensity;
}

float Light::GetIntensity() const
{
	return mIntensity;
}

void Light::SetRadius(float radius)
{
	if (mType == LightType::Point)
	{
		mRadius = radius;
	}
}

float World::Light::GetRadius() const
{
	if (mType == LightType::Point)
	{
		return mRadius;
	}
	return -1.0f;
}
