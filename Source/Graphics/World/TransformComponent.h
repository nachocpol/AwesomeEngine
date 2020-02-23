#pragma once

#include "Component.h"
#include "glm/glm.hpp"

namespace World
{
	class TransformComponent : public Component
	{
	public:
		TransformComponent();
		void Update(float deltaTime);

		glm::vec3 GetPosition()const;
		void SetPosition(const glm::vec3& position);
		void SetPosition(const float& x, const float& y, const float& z);
		void Translate(const glm::vec3& delta);

		glm::vec3 GetRotation()const;
		virtual void SetRotation(const glm::vec3& rotation);
		void SetRotation(const float& x, const float& y, const float& z);
		void Rotate(float x, float y, float z);

		glm::vec3 GetScale()const;
		virtual void SetScale(const glm::vec3& scale);
		void SetScale(const float& x, const float& y, const float& z);

		glm::mat4 GetWorldTransform()const;

	private:
		glm::vec3 mPosition;
		glm::vec3 mRotation;
		glm::vec3 mScale;
		glm::mat4 mWorldTransform;
	};
}