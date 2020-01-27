#include "Camera.h"

#include "glm/ext.hpp"

using namespace World;

Camera::Camera()
{
}

Camera::~Camera()
{

}

void Camera::Update(float deltaTime)
{

}

void Camera::ConfigureProjection(float aspect, float vfov, float near, float far)
{
	mProjection = glm::perspective(glm::radians(vfov), aspect, near, far);
}
