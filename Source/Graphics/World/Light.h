#pragma once

#include "Actor.h"

namespace World
{
	class Light : public Actor
	{
	public:
		Light();
		~Light();
		
		struct LightType
		{
			enum T
			{
				Point,
				Directional,
				COUNT
			};
		};

		void SetLightType(LightType::T type);
		LightType::T GetLightType()const;

		void SetColor(glm::vec3 color);
		glm::vec3 GetColor()const;

		void SetIntensity(float intensity);
		float GetIntensity()const;

		void SetRadius(float radius);
		float GetRadius()const;

		Type::T GetActorType() const override { return Type::Camera; };

	private:
		// Generic:
		LightType::T mType;
		glm::vec3 mColor;
		float mIntensity;

		// Point only:
		float mRadius;
	};
}