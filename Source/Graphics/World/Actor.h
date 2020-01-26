#pragma once

#include "glm/glm.hpp"
#include <vector>

namespace World
{
	class SceneGraph;
	class Actor
	{
		friend SceneGraph;
	public:
		Actor();
		~Actor();

		glm::vec3 GetPosition()const;
		void SetPosition(const glm::vec3& position);
		void SetPosition(const float& x, const float& y, const float& z);

		glm::vec3 GetRotation()const;
		void SetRotation(const glm::vec3& rotation);
		void SetRotation(const float& x, const float& y, const float& z);

		glm::vec3 GetScale()const;
		void SetScale(const glm::vec3& scale);
		void SetScale(const float& x, const float& y, const float& z);

		uint32_t GetNumChilds()const;
		Actor* GetChild(uint32_t index);

		virtual void Update(float deltaTime);

	private:
		void AddChild(Actor* child);

		glm::vec3 mPosition;
		glm::vec3 mRotation;
		glm::vec3 mScale;
		Actor* mParent;
		std::vector<Actor*> mChilds;
	};
}