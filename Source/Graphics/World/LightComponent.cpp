#include "LightComponent.h"

using namespace World;

LightComponent::LightComponent() :
	 mType(LightType::Point)
	,mColor(1.0f)
	,mIntensity(1.0f)
	,mRadius(5.0f)
{
}

LightComponent::~LightComponent()
{
}

void LightComponent::Update(float deltaTime)
{
}

void LightComponent::SetLightType(LightType::T type)
{
	mType = type;
}

LightComponent::LightType::T LightComponent::GetLightType() const
{
	return mType;
}

void LightComponent::SetColor(glm::vec3 color)
{
	mColor = color;
}

glm::vec3 LightComponent::GetColor() const
{
	return mColor;
}

void LightComponent::SetIntensity(float intensity)
{
	mIntensity = intensity;
}

float LightComponent::GetIntensity() const
{
	return mIntensity;
}

void LightComponent::SetRadius(float radius)
{
	if (mType == LightType::Point)
	{
		mRadius = radius;
	}
}

float LightComponent::GetRadius() const
{
	if (mType == LightType::Point)
	{
		return mRadius;
	}
	return -1.0f;
}
