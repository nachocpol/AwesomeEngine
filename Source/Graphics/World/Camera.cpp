#include "Camera.h"
#include "Core/Platform/InputManager.h"

#include "glm/ext.hpp"

using namespace World;
using namespace Core;

// set defauls! !!!!
Camera::Camera():
	 mYaw(0.0f)
	,mPitch(0.0f)
{

}

Camera::~Camera()
{

}

void Camera::Update(float deltaTime)
{
	// Actor update? Use actor up right front etc....

	InputManager* input = InputManager::GetInstance();
	
	// Update position
	float hackSpeed = 0.005f * deltaTime;
	glm::vec3 position = GetPosition();
	if (input->IsKeyPressed('a'))
	{
		position += mRight * hackSpeed;
	}
	if (input->IsKeyPressed('d'))
	{
		position -= mRight * hackSpeed;
	}
	if (input->IsKeyPressed('w'))
	{
		position += mFront * hackSpeed;
	}
	if (input->IsKeyPressed('s'))
	{
		position -= mFront * hackSpeed;
	}
	if (input->IsKeyPressed('q'))
	{
		position += mUp * hackSpeed;
	}
	if (input->IsKeyPressed('e'))
	{
		position -= mUp * hackSpeed;
	}
	SetPosition(position);

	// Mouse look:
	glm::vec2 mousePos = input->GetMousePos();
	glm::vec2 mouseOff = glm::vec2(0.0f, 0.0f);
	if (input->IsMouseButtonPressed(MouseButton::Right))
	{
		mouseOff = mousePos - mLastMousePos;
	}
	mLastMousePos = mousePos;

	mouseOff *= 0.25f;
	mYaw -= mouseOff.x;
	mPitch -= mouseOff.y;
	mPitch = glm::clamp(mPitch, -89.0f, 89.0f);
	
	mFront.x = cos(glm::radians(mYaw)) * cos(glm::radians(mPitch));
	mFront.y = sin(glm::radians(mPitch));
	mFront.z = sin(glm::radians(mYaw)) * cos(glm::radians(mPitch));
	mFront = glm::normalize(mFront);
	
	mRight = glm::normalize(glm::cross(mFront, glm::vec3(0.0f, 1.0f, 0.0f)));
	mUp = glm::normalize(glm::cross(mRight,mFront));

	mViewTransform = glm::lookAtLH(GetPosition(), GetPosition() + mFront, mUp);
}

void Camera::ConfigureProjection(float aspect, float vfov, float near, float far)
{
	mProjection = glm::perspectiveLH(glm::radians(vfov), aspect, near, far);

	mProjectionProps.Aspect = aspect;
	mProjectionProps.VFov = vfov;
	mProjectionProps.Near = near;
	mProjectionProps.Far = far;
}

glm::mat4 World::Camera::GetProjection() const
{
	return mProjection;
}

glm::mat4 World::Camera::GetInvViewTransform() const
{
	// This is inverse as in we convert from world to view space.
	// Note that we don't call glm::inverse anywhere, as it is implicit inside the lookAt!
	return mViewTransform;
}

Camera::ProjectionProps World::Camera::GetProjectionProps() const
{
	return mProjectionProps;
}
