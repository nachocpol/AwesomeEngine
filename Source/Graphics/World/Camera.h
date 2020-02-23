#pragma once

#include "glm/glm.hpp"
#include "Component.h"

namespace World
{
	class CameraComponent : public Component
	{
	public:
		struct ProjectionProps
		{
			float Aspect;
			float VFov;
			float Near;
			float Far;
		};

		CameraComponent();
		~CameraComponent();
		void Update(float deltaTime)override;
		void ConfigureProjection(float aspect, float vfov, float near, float far);

		glm::mat4 GetProjection()const;
		glm::mat4 GetInvViewTransform()const;
		ProjectionProps GetProjectionProps()const;

	private:
		glm::mat4 mProjection;
		ProjectionProps mProjectionProps;
		glm::mat4 mViewTransform;

		glm::vec2 mLastMousePos;
		float mYaw;
		float mPitch;
		glm::vec3 mFront;
		glm::vec3 mRight;
		glm::vec3 mUp;
	};
}