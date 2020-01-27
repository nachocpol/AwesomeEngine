#pragma once

#include "Actor.h"

namespace World
{
	class Camera : public Actor
	{
	public:
		Camera();
		~Camera();
		void Update(float deltaTime)override;
		void ConfigureProjection(float aspect, float vfov, float near, float far);

		Type::T GetActorType() const { return Type::Camera; }

	private:
		glm::mat4 mProjection;
	};
}