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

		glm::mat4 GetProjection()const;
		glm::mat4 GetViewTransform()const;

		Type::T GetActorType() const { return Type::Camera; }

	private:
		glm::mat4 mProjection;
		glm::mat4 mViewTransform;

		glm::vec2 mLastMousePos;
		float mYaw;
		float mPitch;
		glm::vec3 mFront;
		glm::vec3 mRight;
		glm::vec3 mUp;
	};
}