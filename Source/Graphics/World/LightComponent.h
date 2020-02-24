#pragma once

#include "Component.h"
#include "glm/glm.hpp"

namespace World
{
	class LightComponent : public Component
	{
	public:
		LightComponent();
		~LightComponent();
		
		struct LightType
		{
			enum T
			{
				Point,
				Directional,
				COUNT
			};
		};

		void Update(float deltaTime);

		void SetLightType(LightType::T type);
		LightType::T GetLightType()const;

		void SetColor(glm::vec3 color);
		glm::vec3 GetColor()const;

		void SetIntensity(float intensity);
		float GetIntensity()const;

		void SetRadius(float radius);
		float GetRadius()const;

	private:
		// Generic:
		LightType::T mType;
		glm::vec3 mColor;
		float mIntensity;

		// Point only:
		float mRadius;
	};
}